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

The preferred short-lived spelling is a lexical region block:

```ari
region {
  let text = string::from("hello");
  let line = format!("message: {}", text);
}
```

The block still creates an owned allocation source. It is just hidden because
the lifetime is exactly the block body. `zone { ... }` remains accepted as
compatibility syntax for older examples and tests.

## Region

A `Region` owns a bounded allocation area:

```ari
var region = region::create(4096);
let value = region.new<i64>(42);
let bytes = region.alloc_array<u8>(128);
let text = region.string("hello");
let text_copy = region.string_copy(ref text);
var values = region.vec<i64>(4);
values.push(10);
let values_copy = region.vec_copy<i64>(ref values);
region.reset();
region::destroy(region);
```

`reset` invalidates every pointer and handle allocated from the region but
keeps the region itself alive. `destroy` consumes the region owner and releases
the backing storage. Passing a region by `ref mut Region` means the callee may
allocate from that lifetime but does not own the lifetime.

`region::as_zone(ref mut Region) -> ref mut Zone` exists as the concrete
compatibility operation for APIs that have not moved to `Region` yet. Source
code normally does not need to spell it: when a call expects exactly
`ref mut Zone`, passing `ref mut region` lowers through `region::as_zone`
automatically. This bridge still requires an explicit mutable region borrow,
so it does not create a hidden heap or ambient allocation capability.

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
  user-facing path to a standard handle, including copy constructors that move
  data into an explicitly chosen region and facade methods for common CLI
  handles such as environment strings, process commands, captured process
  output, and temporary paths
- module-level `*_with_region` functions are the migration spelling for public
  APIs whose older names still take `ref mut Zone`
- final-form `_in` means "allocate into this explicit region"; during the
  migration, old `_in(ref mut Zone, ...)` APIs remain compatibility shims
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
- `ref mut Region` arguments as compatibility input for old `ref mut Zone`
  parameters by lowering through `region::as_zone`

The model is intentionally diagnostic, not a full formal proof of raw memory
safety. Raw pointers, casts, manual allocation, and FFI remain explicit escape
hatches.

## Rejected Shapes

Ari does not use a magical current heap for ordinary allocation. It makes code
shorter in tiny examples but hides lifetime choice in large programs.

Ari also avoids making `ZoneMetadata` the normal user API. Metadata is useful
for allocator recovery, but making users pass metadata handles teaches the
runtime implementation rather than the memory model.

## Lexical Region Blocks

`region { ... }` and `region(capacity) { ... }` are statement forms for local
scratch allocation:

```ari
region(8192) {
  let path = path::join("target", "ari")?;
  let message = format!("built {}", path);
}
```

The checker lowers the block to a hidden `own std::region::Region`, makes that
region the current allocation source while the body is checked, and inserts
`std::region::destroy` when control leaves the block. Calls may omit exactly
one `ref mut Zone` parameter inside the block because the hidden region is
bridged through `std::region::as_zone`. This is the same compatibility bridge
used when source code passes `ref mut region` to a legacy zone-taking API.

Values allocated in the hidden region cannot escape through returns, outer
bindings, struct literals, or other lifetime-extending paths. If the result
must outlive the block, allocate it in a caller-owned `Region` and pass that
region explicitly.

## Future Direction

Open work:

- migrate remaining stdlib APIs from `Zone` parameters to `Region`,
  `Allocator`, or `Region` convenience methods
- keep the Region-to-Zone argument bridge narrow: only explicit
  `ref mut Region` may satisfy a legacy `ref mut Zone` parameter
- improve ownership diagnostics for owner-bearing structs beyond the Region
  wrapper case
- replace compatibility examples that still spell `zone::create` when a
  `Region` form now exists
- keep `Zone` documented as low-level compatibility until old APIs are gone
