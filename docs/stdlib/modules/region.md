# std::region

`std::region` is the preferred user-facing spelling for Ari's bulk allocation
lifetime model. A `Region` owns a bounded allocation area, values allocated
from it live until the region is reset or destroyed, and higher-level handles
such as `String` and `Vec[T]` can grow through an `Allocator` recovered from
their backing storage.

The implementation is now a small owned wrapper around the existing `Zone`
runtime. That split is deliberate: `Region` is the public lifetime object,
`Allocator` is the public growth capability, and `Zone` remains the low-level
runtime and compatibility layer.

## API

```ari
Region

region::create(capacity: i64) -> own Region
region::default_capacity() -> i64
region::allocator(ref mut Region) -> Allocator
region::as_zone(ref mut Region) -> ref mut Zone

region::alloc(ref mut Region, bytes: i64, align: i64) -> ptr u8
region::alloc_array<T>(ref mut Region, count: i64) -> ptr T
region::new<T>(ref mut Region, value: T) -> ptr T
region::string(ref mut Region, bytes: Slice[u8]) -> String
region::vec<T>(ref mut Region, capacity: i64) -> Vec[T]
region::boxed<T>(ref mut Region, value: T) -> Box[T]
region::cstring(ref mut Region, bytes: Slice[u8]) -> Result[CString, Error]
region::promote<T>(ref mut target, source: ptr T) -> ptr T

region::capacity(ref mut Region) -> i64
region::used(ref mut Region) -> i64
region::remaining(ref mut Region) -> i64
region::can_alloc(ref mut Region, bytes: i64) -> bool
region::can_alloc_array<T>(ref mut Region, count: i64) -> bool

region::reset(ref mut Region) -> void
region::destroy(region: own Region) -> void
region { statements... }
region(capacity) { statements... }

Region::allocator() -> Allocator
Region::alloc(bytes: i64, align: i64) -> ptr u8
Region::alloc_array<T>(count: i64) -> ptr T
Region::new<T>(value: T) -> ptr T
Region::string(bytes: Slice[u8]) -> String
Region::vec<T>(capacity: i64) -> Vec[T]
Region::boxed<T>(value: T) -> Box[T]
Region::cstring(bytes: Slice[u8]) -> Result[CString, Error]
Region::promote<T>(source: ptr T) -> ptr T
Region::capacity() -> i64
Region::used() -> i64
Region::remaining() -> i64
Region::can_alloc(bytes: i64) -> bool
Region::can_alloc_array<T>(count: i64) -> bool
Region::reset() -> void
```

## When To Use Region

Use `Region` when code chooses an allocation lifetime:

```ari
var region = region::create(4096);
let text = region.string("hello");
var values = region.vec<i64>(4);
values.push(10);
let boxed = region.boxed<i64>(42);
region::destroy(region);
```

For short-lived scratch work, prefer a lexical region block:

```ari
region {
  let text = string::from("hello");
  let rendered = format!("value={}", text);
}

region(65536) {
  var values = vec::new<i64>(8);
  values.push(1);
  values.push(2);
}
```

`region { ... }` creates a hidden `own Region` with
`region::default_capacity()`, makes it the current allocation source for the
body, and destroys it on every exit from the block. `region(capacity) { ... }`
uses the checked `i64` capacity expression instead. The hidden region can
satisfy legacy `ref mut Zone` parameters through the same narrow
`region::as_zone` bridge used for explicit `ref mut region` arguments, so older
zone-backed APIs become pleasant without adding a global heap.

Values allocated inside a lexical region block are temporary. The checker
rejects returning them, storing them into outer bindings, or otherwise letting
them outlive the block. Allocate into a named caller-owned `Region` when the
result must survive.

The method form is equivalent and is the preferred spelling when the region is
already named:

```ari
var region = region::create(4096);
let data = region.alloc_array<i64>(4);
ptr_store(data, 10);
let spare = region.remaining();
region.reset();
region::destroy(region);
```

Use `Allocator` when code already has a region-backed handle and only needs to
allocate more storage from the same backing region:

```ari
var region = region::create(4096);
var values = region.vec<i64>(2);
let allocator = std::allocator::of(ref values);
let scratch = allocator.alloc_array<i64>(4);
region::destroy(region);
```

Use `std::allocator::from_region(ref mut region)` when helper code should be
able to allocate but should not control the region's lifecycle:

```ari
fn write_scratch(allocator: std::allocator::Allocator) -> ptr u8 {
  return allocator.alloc(32, 1);
}

var region = region::create(4096);
let bytes = write_scratch(std::allocator::from_region(ref mut region));
region::destroy(region);
```

Use `std::zone` only when interacting with older APIs, low-level runtime
tests, or implementation details such as `ZoneMetadata`. `region::as_zone` is
the narrow compatibility operation for existing functions that still take
`ref mut Zone`; new user-facing APIs should prefer `Region` or `Allocator`.

The compiler applies that compatibility operation for ordinary calls. If a
legacy API expects `ref mut Zone`, a caller may pass `ref mut region` directly:

```ari
var region = region::create(4096);

let text = std::string::from_slice_in(ref mut region, "hello");
var values = std::vec::new<i64>(ref mut region, 2);
values.push_in(ref mut region, 10);
let spare = std::zone::remaining(ref mut region);

region::destroy(region);
```

The bridge is intentionally narrow. The argument must be an explicit mutable
borrow of `Region`; immutable borrows and owned region values are not silently
converted. That keeps allocation authority visible while avoiding the old
`std::region::as_zone(ref mut region)` spelling in routine code.

The convenience methods are deliberately small. They cover the common standard
handles that otherwise force users to spell `region::as_zone`: owned text,
vectors, boxes, and C strings. Once a handle is created from a `Region`, its
growth methods recover the same allocation source, so `values.push(...)` or
`text.push(...)` can grow without storing a region field in the handle.

## Capacity And Failure

Regions are bounded. `capacity` is the logical payload budget selected at
creation time, `used` is the logical payload allocated so far, and `remaining`
is `capacity - used`. These counters are for planning and diagnostics; they do
not include runtime allocation-header padding or alignment slack.

`can_alloc` and `can_alloc_array<T>` are preflight helpers. They return `false`
for negative sizes and avoid multiplication overflow for arrays, but they do
not reserve memory. Another allocation can still consume the region before a
later allocation.

Raw `alloc` and `alloc_array<T>` return uninitialized memory. Prefer
`region::new<T>`, `Box`, `String`, `Vec[T]`, or another owning handle when the
code wants constructed values.

## Lifetime Rules

`region::reset(ref mut region)` invalidates all pointers and handles allocated
from the region while keeping the region itself alive. `region::destroy(region)`
consumes the owner and releases the backing storage. The checker tracks many
direct region-backed pointers and stdlib handles and rejects obvious
use-after-reset or use-after-destroy cases.

`Region` is still explicit allocation. It is not a process-global heap and it
does not make raw memory fully safe. Pass a region or allocator capability to
the code that allocates, and keep escaping values in a longer-lived region when
needed.

## Relationship To Zone

Today:

- `Region` is an owned wrapper over a private `Zone` field.
- `region { ... }` lowers to a hidden `Region`, a current allocation source,
  and an inserted `region::destroy` cleanup at block exit.
- `region::*` functions and `Region` methods delegate to `zone::*`.
- `region::as_zone` is exposed as the concrete compatibility bridge, and the
  compiler inserts it when `ref mut Region` is passed to an old
  `ref mut Zone` parameter.
- `Allocator` wraps the current zone-backed allocation metadata.
- the compiler tracks `Region` as an allocation source and rejects handles
  used after `region.reset()` or `region::destroy(region)`.

Direction:

- write new user-facing examples with `Region` or `region::*`
- use `region { ... }` for short-lived scratch work
- keep `Allocator` as the growth capability for containers and formatters
- keep `Zone`/`ZoneMetadata` as low-level compatibility names until the
  compiler and stdlib can migrate old APIs without breaking existing programs
- keep `zone { ... }` as compatibility syntax until older examples migrate

## Tests

Focused coverage:

- `tests/cases/standard-library/ok/zone/std-region-capability.ari`
- `tests/cases/standard-library/ok/zone/std-region-zone-bridge.ari`
- `tests/cases/memory/ok/region-current-block.ari`
- `tests/cases/memory/errors/region-current-block-escape.ari`
- `tests/cases/memory/errors/region-zone-bridge-immutable.ari`

Related compatibility coverage:

- `tests/cases/standard-library/ok/zone/std-allocator-capability.ari`
- `tests/cases/standard-library/ok/zone/std-zone-alloc-array.ari`
- `tests/cases/standard-library/ok/zone/std-zone-backed.ari`
