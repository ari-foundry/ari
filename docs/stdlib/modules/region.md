# std::region

`std::region` is the preferred user-facing spelling for Ari's bulk allocation
lifetime model. A `Region` owns a bounded allocation area, values allocated
from it live until the region is reset or destroyed, and higher-level handles
such as `String` and `Vec[T]` can grow through an `Allocator` recovered from
their backing storage.

The current implementation is an alias over the existing `Zone` runtime. That
is intentional: `Zone` remains the low-level implementation and compatibility
name, while new user-facing docs and APIs should explain the model as
`Region` ownership plus `Allocator` capability.

## API

```ari
Region

region::create(capacity: i64) -> own Region
region::default_capacity() -> i64
region::allocator(ref mut Region) -> Allocator

region::alloc(ref mut Region, bytes: i64, align: i64) -> ptr u8
region::alloc_array<T>(ref mut Region, count: i64) -> ptr T
region::new<T>(ref mut Region, value: T) -> ptr T
region::promote<T>(ref mut target, source: ptr T) -> ptr T

region::capacity(ref mut Region) -> i64
region::used(ref mut Region) -> i64
region::remaining(ref mut Region) -> i64
region::can_alloc(ref mut Region, bytes: i64) -> bool
region::can_alloc_array<T>(ref mut Region, count: i64) -> bool

region::reset(ref mut Region) -> void
region::destroy(region: own Region) -> void
```

## When To Use Region

Use `Region` when code chooses an allocation lifetime:

```ari
var region = region::create(4096);
let text = std::string::from(ref mut region, "hello");
region::destroy(region);
```

Use `Allocator` when code already has a region-backed handle and only needs to
allocate more storage from the same backing region:

```ari
var region = region::create(4096);
var values = std::vec::new<i64>(ref mut region, 2);
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
tests, or implementation details such as `ZoneMetadata`. New docs should not
require ordinary users to understand allocation-header recovery.

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

- `Region` is a public alias for `Zone`.
- `region::*` functions delegate to `zone::*`.
- `Allocator` wraps the current zone-backed allocation metadata.

Direction:

- write new user-facing examples with `Region` or `region::*`
- keep `Allocator` as the growth capability for containers and formatters
- keep `Zone`/`ZoneMetadata` as low-level compatibility names until the
  compiler and stdlib can migrate old APIs without breaking existing programs
- future syntax can make short-lived regions prettier without adding a hidden
  ambient heap

## Tests

Focused coverage:

- `tests/cases/standard-library/ok/zone/std-region-capability.ari`

Related compatibility coverage:

- `tests/cases/standard-library/ok/zone/std-allocator-capability.ari`
- `tests/cases/standard-library/ok/zone/std-zone-alloc-array.ari`
- `tests/cases/standard-library/ok/zone/std-zone-backed.ari`
