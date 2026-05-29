# std::zone

`std::zone` is Ari's low-level explicit region-lifecycle module. It exists so
code can allocate memory without hiding ownership behind a process-wide heap,
and it is still the current runtime implementation behind the user-facing
`std::region` API.

For new user-facing code, prefer `std::region::Region` and `region::*` when
choosing an allocation lifetime, and prefer `std::allocator::Allocator` when a
handle only needs to grow from existing region-backed storage. Keep
`std::zone` for compatibility, raw allocation tests, and implementation details
such as metadata recovery.

When older `std::zone` functions are still useful, callers can usually keep the
newer `Region` spelling. Passing `ref mut Region` to a parameter typed
`ref mut Zone` is lowered through `std::region::as_zone` by the compiler:

```ari
var region = std::region::create(4096);
let bytes = std::zone::alloc(ref mut region, 64, 1);
let spare = std::zone::remaining(ref mut region);
std::region::destroy(region);
```

This is a compatibility bridge, not a new ambient allocator. The call site
still has to borrow the region mutably.

`ZoneMetadata` and `ZoneBacked` remain available under `std::zone` as
compatibility and implementation bridges while the stdlib migrates away from
metadata-shaped user APIs. They are intentionally not root `std` prelude
aliases; ordinary code should reach for `Region` and `Allocator` first.

The module is deliberately low-level. Raw allocations return pointers; they do
not initialize values, run destructors, or make a buffer safe by themselves.
Use higher-level handles such as `std::boxed::Box[T]`, `std::string::String`,
or `std::vec::Vec[T]` when those shapes match the job.

## API

```ari
zone::create(capacity: i64) -> own Zone
zone::default_capacity() -> i64
zone::alloc(ref mut Zone, bytes: i64, align: i64) -> ptr u8
zone::capacity(ref mut Zone) -> i64
zone::used(ref mut Zone) -> i64
zone::remaining(ref mut Zone) -> i64
zone::can_alloc(ref mut Zone, bytes: i64) -> bool
zone::can_alloc_array<T>(ref mut Zone, count: i64) -> bool
zone::alloc<T>(ref mut Zone) -> ptr T
zone::alloc_array<T>(ref mut Zone, count: i64) -> ptr T
zone::new<T>(ref mut Zone, value: T) -> ptr T
zone::promote<T>(ref mut target, source: ptr T) -> ptr T
zone::allocation_zone(data: ptr u8) -> ptr c_void
zone::metadata(data: ptr u8) -> ZoneMetadata
zone::from_zone(ref mut Zone) -> ZoneMetadata
zone::of<T: ZoneBacked>(ref value) -> ZoneMetadata
value.zone() -> ZoneMetadata
zone::reset(ref mut Zone) -> void
zone::destroy(zone: own Zone) -> void
zone { statements... }
zone(capacity) { statements... }

ZoneMetadata
metadata.as_ptr() -> ptr c_void
metadata.as_zone_ptr() -> ptr Zone
metadata.alloc(bytes: i64, align: i64) -> ptr u8
metadata.alloc_array<T>(count: i64) -> ptr T
metadata.capacity() -> i64
metadata.used() -> i64
metadata.remaining() -> i64
metadata.can_alloc(bytes: i64) -> bool
metadata.can_alloc_array<T>(count: i64) -> bool
metadata.equals(ref other) -> bool
ZoneBacked

create(capacity)
default_capacity()
alloc(ref mut zone, bytes, align)
capacity(ref mut zone)
used(ref mut zone)
remaining(ref mut zone)
can_alloc(ref mut zone, bytes)
can_alloc_array<T>(ref mut zone, count)
alloc<T>(ref mut zone)
alloc_array<T>(ref mut zone, count)
new<T>(ref mut zone, value)
promote<T>(ref mut target, source)
reset(ref mut zone)
destroy(zone)
```

## Lexical Allocation Blocks

Use `region { ... }` when a command, parser, formatter, or small hosted-program
operation needs temporary owned library values and all of those values should
die together at the end of the block. `zone { ... }` remains accepted as the
compatibility spelling for older low-level examples.

```ari
fn main() -> i64 {
  region {
    let name = std::string::from("hello");
    let line = format!("package {}", name);
    io::println(line)?;
  }

  return 0;
}
```

The block is compiler syntax. It lowers to a hidden
`std::region::Region` for `region { ... }`, pushes that owner as the current
allocation source, checks the body, and inserts `std::region::destroy` cleanup
on every exit. The compatibility form `zone { ... }` lowers to the older
hidden `Zone` temporary. `region::default_capacity()` and
`zone::default_capacity()` are both 4096 bytes today. A larger scratch region
can be requested with `region(capacity) { ... }`:

```ari
region(65536) {
  let source = fs::read_to_string("Ari.toml")?;
  let rendered = format!("manifest bytes = {}", source.as_slice().len);
}
```

Capacity is a runtime limit for the temporary arena. If `region { ... }` is too
small for the values created inside it, the hosted runtime writes a diagnostic
to stderr and exits. The diagnostic suggests `region(capacity) { ... }`,
`zone(capacity) { ... }`, or a larger explicit allocation source. Invalid
capacities, invalid handles, and malformed raw allocation requests also get
named runtime diagnostics instead of a silent exit.

Inside a current allocation block, calls may omit exactly one `ref mut Zone`
parameter. The checker inserts the current source for ordinary functions,
generic functions, ordinary methods, associated functions, trait-qualified
methods, dyn trait-object methods, and callable values when the arity
otherwise matches. This lets the natural, short spelling call the same stdlib
implementation:

```ari
region {
  let text = string::from("abc");                  // string::from(ref mut zone, ...)
  let args = env::args();                          // env::args(ref mut zone)
  let file = fs::read_to_string("notes.txt")?;     // fs::read_to_string(ref mut zone, ...)
  let rendered = std::fmt::Display::format_in(42); // Display::format_in(42, ref mut zone)
  let line = format!("{} {}", text, file.as_slice().len); // format_in!(ref mut zone, ...)
  let spare = zone::remaining();                          // remaining(ref mut zone)
}
```

Explicit `ref mut zone` still wins when data should live in another region:

```ari
var outer = region::create(4096);
region {
  let persistent = string::from(ref mut outer, "keep this");
}
region::destroy(outer);
```

Nested current allocation blocks shadow outer ones. Values allocated in an inner
block cannot escape to outer bindings or returns unless they are copied or
promoted into an outer explicit zone.

`alloc(ref mut zone, bytes, align)` is the raw byte allocator. It returns a
`ptr u8` with the requested alignment.

`capacity(ref mut zone)` returns the logical byte capacity originally chosen
for the zone. `used(ref mut zone)` returns the logical payload bytes allocated
so far. `remaining(ref mut zone)` returns `capacity - used`. These helpers are
for planning and diagnostics; they do not include runtime header padding or
alignment slack, and they do not make allocation recoverable.

`can_alloc(ref mut zone, bytes)` returns whether `bytes` additional logical
payload bytes fit in the zone right now. `can_alloc_array<T>(ref mut zone,
count)` does the same check for `count` values of `T` and avoids multiplication
overflow by comparing against the remaining capacity first. Both helpers return
`false` for negative inputs. A zero array count returns `true`, matching
`alloc_array<T>`, while raw `alloc(ref mut zone, 0, align)` is still an invalid
raw allocation request. These are preflight helpers for choosing capacities and
reporting nicer errors; another allocation can still consume the zone before a
later raw allocation if code interleaves work.

`alloc<T>(ref mut zone)` allocates uninitialized storage for one `T`.
`alloc_array<T>(ref mut zone, count)` allocates uninitialized storage for
`count` consecutive `T` values. A count of `0` returns a null pointer, and a
negative count traps through `std::assert`.

`new<T>(ref mut zone, value)` allocates storage for one `T`, writes `value`
into it, and returns the typed pointer.

`promote<T>(ref mut target, source)` copies the value at `source` into the
target zone. Use it when a temporary or shorter-lived zone value must move
into a longer-lived zone.

`allocation_zone(data)` is the raw allocation-header primitive. It reads the
header immediately before a non-null zone allocation and returns the opaque raw
zone handle. `metadata(data)` wraps that handle in `ZoneMetadata`.
`from_zone(ref mut zone)` creates the same typed metadata from an existing zone
capability without requiring a prior payload allocation. New public library
code should normally use `std::allocator::from_region`,
`std::allocator::from_data`, or `std::allocator::of` instead.

`ZoneBacked` is the high-level wrapper for library handles that own
zone-backed storage. `zone::of(ref value)` and `value.zone()` expose
`ZoneMetadata` for supported handles such as `Box[T]`, `String`, `Vec[T]`, and
the zone-backed `std::collections` handles, including map update-entry handles.
`metadata.as_ptr()` exposes the raw opaque handle, `metadata.as_zone_ptr()`
gives the same address typed as `ptr Zone`, and `metadata.equals(ref other)`
checks handle identity.
`metadata.alloc(bytes, align)` and `metadata.alloc_array<T>(count)` allocate
through that recovered runtime zone handle; these methods are kept for
compatibility and low-level tests. Normal heap-handle growth should use an
`Allocator` recovered by `std::allocator::of(ref handle)` instead of exposing
metadata in the surrounding API.
`metadata.capacity()`, `metadata.used()`, and `metadata.remaining()` read the
same logical counters through the recovered handle, which is useful when a
zone-backed stdlib container wants to report or plan follow-up allocation
without storing an extra zone field. Like other zone-backed handles,
`ZoneMetadata` is invalid after the source zone is reset or destroyed.
`metadata.can_alloc(bytes)` and `metadata.can_alloc_array<T>(count)` provide
the same preflight checks through the recovered handle. They remain useful for
runtime-oriented compatibility code, while public collection and formatter
helpers should prefer the `Allocator` vocabulary.

Raw header recovery requires an actual backing allocation. Heap-backed stdlib
handles therefore create a small backing allocation even when their logical
capacity is zero if later growth needs to recover the zone. Raw zero-count
allocation helpers may still return null, so use `from_zone(ref mut zone)` or a
constructor that establishes backing storage when code needs a zone before the
first logical element.

`reset(ref mut zone)` invalidates allocations from the zone while keeping the
zone object alive. `destroy(zone)` consumes the owning zone handle and releases
the backing storage.

## Example

```ari
fn main() -> i64 {
  var zone = zone::create(128);
  let values = zone::alloc_array<i64>(ref mut zone, 3);

  ptr_store(values, 10);
  ptr_store(ptr_add(values, 1), 20);
  ptr_store(ptr_add(values, 2), 30);

  let total =
    ptr_load(values) +
    ptr_load(ptr_add(values, 1)) +
    ptr_load(ptr_add(values, 2));

  zone::destroy(zone);
  return total;
}
```

## Safety Rules

Zone-backed pointers, boxes, strings, vectors, and derived slices keep
provenance in the checker when Ari can track the source zone. Using them after
`reset` or `destroy` is rejected.

Lexical allocation blocks are still explicit allocation. The `Region` or `Zone`
owner is just hidden because its lifetime is the block itself. If a function
has two zone parameters, or if the missing argument is not uniquely
identifiable as `ref mut Zone`, the checker requires the explicit call.

If code outside a current allocation block omits only the zone argument, Ari reports
that the call needs an allocation zone and suggests `region { ... }`,
`zone { ... }`, or an explicit `ref mut Zone`. That diagnostic is intentionally
different from a plain wrong-argument-count error because allocation lifetime
is the important choice.

The current allocation source is intentionally not exposed as a general
`zone::current()` or `region::current()` library function. Library APIs still
declare `ref mut Zone`; the compiler only fills that parameter at call sites
where the lifetime is lexical and obvious.
That keeps temporary allocation easy to write without letting library code
silently depend on a global allocator.

Raw memory is not automatically initialized. After `alloc<T>` or
`alloc_array<T>`, write each slot before reading it. If the element type owns
resources, prefer a higher-level handle or an API that clearly owns drop
behavior.

## Implementation Spec

Current-zone blocks are intentionally lexical and conservative:

1. `region { body }` and `zone { body }` parse as statements. Both words are
   contextual, so existing values named `region` or `zone` continue to work
   outside this exact statement shape.
2. The checker creates a hidden `own Region` for `region { ... }` or a hidden
   `own Zone` for compatibility `zone { ... }`, with the default capacity or
   the checked `i64` expression from the capacity form.
3. That hidden local is pushed onto the current-zone stack while `body` is
   checked. Nested blocks shadow outer current sources.
4. A call inside the body may omit exactly one argument when the omitted
   parameter is uniquely typed as `ref mut Zone`. Ordinary functions, generic
   functions, ordinary methods, associated functions, trait-qualified methods,
   dyn trait-object methods, and callable values use this rule.
5. `format!` is the compiler-owned formatting shortcut for the current source.
   Outside a current allocation block it remains a diagnostic and callers should use
   `format_in!(ref mut zone, ...)`.
6. The hidden owner is destroyed on fallthrough and when `return`, `break`,
   `continue`, or labeled exits leave the block. `region { ... }` emits
   `region::destroy`; compatibility `zone { ... }` emits `zone::destroy`.
7. Existing zone-provenance checks still decide whether pointers and
   zone-backed handles may escape. Current-zone syntax does not make a global
   heap and does not extend object lifetimes.

## Tests

- `tests/cases/standard-library/ok/zone/std-zone-alloc-array.ari` checks
  `std::zone::alloc_array`, the root `alloc_array` alias, null return for
  zero count, pointer loads/stores, LLVM symbol emission, and runtime result.
- `tests/cases/standard-library/ok/zone/std-zone-introspection.ari` checks
  `zone::capacity`, `zone::used`, `zone::remaining`, the matching
  `ZoneMetadata` methods, reset behavior, and LLVM symbol emission.
- `tests/cases/standard-library/ok/zone/std-region-zone-bridge.ari` checks
  that `ref mut Region` can call legacy `ref mut Zone` functions and methods
  without manually spelling `std::region::as_zone`.
- `tests/cases/standard-library/ok/zone/std-zone-backed.ari` checks
  `ZoneMetadata`, `ZoneBacked`, `zone::metadata(data)`, `zone::from_zone`,
  `metadata.alloc_array<T>`, `metadata.as_ptr()`,
  `metadata.as_zone_ptr()`, `zone::of(ref value)`, `value.zone()`, and raw
  allocation-header agreement for box, string, vector, set, map, map entry,
  sequence, linked list, heap, and priority queue handles.
- Existing zone, vector, string, and boxed tests cover reset/destroy
  invalidation and zone-backed handle provenance.
- `tests/cases/memory/ok/zone-current-block.ari` checks current-zone block
  syntax, default and explicit capacities, omitted `ref mut Zone` arguments for
  `std::string`, `std::fmt`, and `std::fs`, and current-zone `format!`.
- `tests/cases/memory/ok/region-current-block.ari` checks the preferred
  `region { ... }` spelling, Region-to-Zone current-source bridging,
  `format!`, stdlib String/Vec/Box construction, and inserted region cleanup.
- `tests/cases/memory/errors/region-current-block-escape.ari` checks that
  values allocated in a hidden region cannot escape through returns.
- `tests/cases/memory/ok/zone-current-stdlib.ari` checks omitted current-zone
  arguments across common stdlib APIs, including String/Vec/Box copy and
  growth methods, path joining, encoding decode, runtime `fmt::format`, and
  `fmt::concat2`.
- `tests/cases/memory/ok/zone-current-trait-calls.ari` checks omitted
  current-zone arguments through trait-qualified static dispatch and
  object-safe dyn dispatch.
- `tests/cases/memory/ok/zone-current-callable.ari` checks omitted
  current-zone arguments through a function pointer value.
- `tests/cases/memory/ok/zone-current-capacity.ari` checks explicit
  current-zone capacity for bulk scratch allocation and the LLVM runtime
  labels used by zone capacity diagnostics.
- `tests/cases/memory/errors/zone-current-missing-function.ari` and
  `tests/cases/memory/errors/zone-current-missing-method.ari`, plus the
  callable, trait-qualified, and trait-object variants, check targeted
  diagnostics for calls that need a current or explicit zone.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused zone allocation coverage.

## Future Work

- migrate remaining examples from compatibility `zone { ... }` to preferred
  `region { ... }` where they are not specifically testing low-level zones
- possible resource construction grammar for future named regions; today
  `region { ... }` keeps allocation lexical without reserving another global
  allocator concept
- capacity planning helpers for common parser/formatter workloads, so callers
  can choose `region(capacity) { ... }` before entering the block
- allocator traits after trait object and generic impl conventions settle
- richer diagnostics for raw allocation of ownership-heavy values
