# Ari Memory Model

Ari's memory model is explicit, capability-oriented, and intentionally small.
The goal is to make ownership and allocation visible in source code without
adding a hidden process-wide heap or pretending raw memory is automatically
safe.

## Core Shape

There are four layers:

| Layer | Public Role | Current Implementation |
| --- | --- | --- |
| Ownership and borrowing | compile-time movement, `drop`, `ref`, `ref mut`, and raw pointer checks | semantic checker state over locals, fields, and borrows |
| `std::region::Region` | user-facing bounded allocation lifetime | owned wrapper around a low-level `Zone` |
| `std::allocator::Allocator` | growth capability for containers and helper APIs | compact handle recovered from a region or region-backed storage |
| `std::zone::Zone` | compatibility and runtime implementation detail | builtin runtime allocation area with metadata hooks |

Ordinary code should start with `Region`. Library internals should accept
`Allocator` when they only need to allocate more bytes in the same lifetime.
`Zone` remains available for low-level tests, older APIs, and compiler/runtime
implementation work.

## Region

A `Region` owns a bounded allocation area:

```ari
var region = region::create(4096);
let value = region.new<i64>(42);
let bytes = region.alloc_array<u8>(128);
let text = region.string("hello");
var values = region.vec<i64>(4);
values.push(10);
region.reset();
region::destroy(region);
```

`reset` invalidates every pointer and handle allocated from the region but
keeps the region itself alive. `destroy` consumes the region owner and releases
the backing storage. Passing a region by `ref mut Region` means the callee may
allocate from that lifetime but does not own the lifetime.

`region::as_zone(ref mut Region) -> ref mut Zone` exists only as a compatibility
escape for APIs that have not moved to `Region` yet. New APIs should not expose
`Zone` unless they are explicitly low-level.

## Allocator

An `Allocator` is not a lifetime owner. It is a capability to allocate more
storage from a known region-backed allocation family:

```ari
var region = region::create(4096);
let allocator = region.allocator();
let scratch = allocator.alloc_array<u8>(64);
region::destroy(region);
```

Containers such as `String`, `Vec[T]`, and maps can recover an allocator from
their existing backing storage. This keeps growth paths from storing redundant
zone or region fields in every view and iterator.

The checker now treats `Region` as an allocation source, not merely as an
ordinary struct that happens to contain a `Zone`. A `String` or `Vec[T]`
created through `region.string(...)` or `region.vec<T>(...)` carries the region
source for implicit growth calls, and using that handle after
`region.reset()` or `region::destroy(region)` is rejected.

## Library Rules

User-facing allocation APIs should follow these rules:

- constructors that choose a lifetime take `ref mut Region`
- helpers that only need growth take `Allocator`
- convenience constructors may live on `Region` when they are the clearest
  user-facing path to a standard handle
- `_in` means "allocate into this explicit region"
- `_to` means "copy into this destination"
- `Zone`, `ZoneMetadata`, and allocation-header recovery stay in low-level
  modules and compatibility bridges
- no stdlib API should invent a hidden global heap to avoid passing a
  capability

The current stdlib still has older `ref mut Zone` entry points. Those are
compatibility APIs and should migrate a module at a time, with focused tests
for each public surface change.

## Compiler Rules

The compiler tracks:

- moved, dropped, and live owners
- owner-bearing aggregate fields
- immutable and mutable borrows
- use-after-reset and use-after-destroy for many region-backed values
- method calls on owned struct receivers when the method borrows `self`
- `Region` as a first-class allocation source for zone-backed handles
- `Allocator` parameters as allocation-capability sources inside helper APIs

The model is intentionally diagnostic, not a full formal proof of raw memory
safety. Raw pointers, casts, manual allocation, and FFI remain explicit escape
hatches.

## Rejected Shapes

Ari does not use a magical current heap for ordinary allocation. It makes code
shorter in tiny examples but hides lifetime choice in large programs.

Ari also avoids making `ZoneMetadata` the normal user API. Metadata is useful
for allocator recovery, but making users pass metadata handles teaches the
runtime implementation rather than the memory model.

## Future Direction

The best-looking future surface is a short region scope form that still creates
an explicit owner:

```ari
region scratch {
  let text = string::copy("hello");
}
```

That syntax should lower to a named `Region`, make it available as the default
destination only inside the block, and destroy it at block exit. It should be a
lexical compile-time convenience, not a process-global allocator.

Open work:

- migrate remaining stdlib APIs from `Zone` parameters to `Region`,
  `Allocator`, or `Region` convenience methods
- improve ownership diagnostics for owner-bearing structs beyond the Region
  wrapper case
- replace compatibility examples that still spell `zone::create` when a
  `Region` form now exists
- decide whether short region scopes use a keyword, a standard macro-like
  form, or ordinary library sugar after block-lifetime lowering is stronger
- keep `Zone` documented as low-level compatibility until old APIs are gone
