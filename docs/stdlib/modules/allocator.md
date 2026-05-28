# std::allocator

`std::allocator` is the public allocation-capability layer. It exists because
most code does not want to think in terms of allocation headers or
`ZoneMetadata`; it wants a value that can allocate, report capacity, and be
passed to helper code.

Ari still uses zones as the first concrete allocator implementation. The
preferred public model is now `Region` plus `Allocator`: a `Region` owns a
bounded lifetime, and an `Allocator` is the smaller capability view over that
storage. Prefer `std::allocator::Allocator` in new library APIs that need to
grow existing region-backed handles, and keep direct `Region` parameters for
constructors, explicit regions, and lifecycle control. `Zone` and
`ZoneMetadata` are still present for compatibility and compiler/runtime
implementation work, but they are not the ordinary user-facing model.

## API

```ari
Allocator

allocator::from_region(ref mut Region) -> Allocator
allocator::from_zone(ref mut Zone) -> Allocator
allocator::from_data(data: ptr u8) -> Allocator
allocator::from_zone_metadata(metadata: zone::ZoneMetadata) -> Allocator
allocator::of<T: zone::ZoneBacked>(ref value) -> Allocator
allocator::of_mut<T: zone::ZoneBacked>(ref mut value) -> Allocator

allocator::alloc(ref Allocator, bytes: i64, align: i64) -> ptr u8
allocator::alloc_array<T>(ref Allocator, count: i64) -> ptr T
allocator::capacity(ref Allocator) -> i64
allocator::used(ref Allocator) -> i64
allocator::remaining(ref Allocator) -> i64
allocator::can_alloc(ref Allocator, bytes: i64) -> bool
allocator::can_alloc_array<T>(ref Allocator, count: i64) -> bool

allocator.metadata() -> zone::ZoneMetadata
allocator.as_ptr() -> ptr c_void
allocator.alloc(bytes: i64, align: i64) -> ptr u8
allocator.alloc_array<T>(count: i64) -> ptr T
allocator.capacity() -> i64
allocator.used() -> i64
allocator.remaining() -> i64
allocator.can_alloc(bytes: i64) -> bool
allocator.can_alloc_array<T>(count: i64) -> bool
allocator.equals(ref other) -> bool
```

## Choosing A Shape

Use `Region` when the operation creates or owns an allocation lifetime:

```ari
var arena = region::create(4096);
let text = string::from(ref mut arena, "hello");
region::destroy(arena);
```

Use `Allocator` when a handle already has storage and later operations need to
grow from the same allocation capability:

```ari
var values = std::vec::new<i64>(ref mut arena, 4);
let allocator = std::allocator::of(ref values);
let scratch = allocator.alloc_array<i64>(8);
```

Use `allocator::from_region(ref mut region)` when an API wants the smaller
allocation capability without accepting lifecycle operations such as
`reset`/`destroy`:

```ari
fn reserve_scratch(allocator: std::allocator::Allocator) -> ptr i64 {
  return allocator.alloc_array<i64>(16);
}

var region = region::create(4096);
let scratch = reserve_scratch(std::allocator::from_region(ref mut region));
```

`allocator::of(ref value)` is the user-facing spelling for "recover the
allocation capability from this handle." It currently delegates to the
zone-backed implementation, but callers should not depend on allocation-header
layout or the `ZoneMetadata` name.

Use `allocator::of_mut(ref mut value)` inside mutating growth helpers that
already have a mutable handle receiver. It returns the same capability as
`of`, while keeping source code from spelling `value.zone()` or
`from_zone_metadata(...)` directly.

## Compatibility

`from_zone_metadata` and `allocator.metadata()` are migration bridges. They
allow existing compiler/runtime-oriented code to move from `ZoneMetadata` to
`Allocator` without a large rewrite. New public code should usually use
`from_region`, `from_data`, or `of`. `from_zone` is kept for older explicit
zone APIs and simply captures the same region-backed capability.

The current implementation is still region-backed and bounded. `can_alloc` and
`can_alloc_array` are preflight checks over logical payload counters; they do
not reserve memory.

## Future Direction

The long-term model should be:

- `Region` owns bulk lifetime.
- `Allocator` is the capability used by containers, strings, formatters, and
  builders that need to allocate.
- `Zone` and raw allocation-header recovery APIs become compatibility or
  internal implementation details.
- Future allocator implementations can add debug, page, fixed-buffer, or
  OS-backed policies without changing ordinary collection APIs.
