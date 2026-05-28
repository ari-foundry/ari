# std::zone

`std::zone` is Ari's explicit allocation capability module. It exists so code
can allocate memory without hiding ownership behind a process-wide heap.
Programs create a `Zone`, pass it as `ref mut Zone` to allocation APIs, and
release it with `zone::destroy(zone)` when all derived pointers and handles are
done.

The module is deliberately low-level. Raw allocations return pointers; they do
not initialize values, run destructors, or make a buffer safe by themselves.
Use higher-level handles such as `std::boxed::Box[T]`, `std::string::String`,
or `std::vec::Vec[T]` when those shapes match the job.

## API

```ari
zone::create(capacity: i64) -> own Zone
zone::default_capacity() -> i64
zone::alloc(ref mut Zone, bytes: i64, align: i64) -> ptr u8
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
metadata.equals(ref other) -> bool
ZoneBacked

create(capacity)
default_capacity()
alloc(ref mut zone, bytes, align)
alloc<T>(ref mut zone)
alloc_array<T>(ref mut zone, count)
new<T>(ref mut zone, value)
promote<T>(ref mut target, source)
reset(ref mut zone)
destroy(zone)
```

## Current-Zone Blocks

Use `zone { ... }` when a command, parser, formatter, or small hosted-program
operation needs temporary owned library values and all of those values should
die together at the end of the block.

```ari
fn main() -> i64 {
  zone {
    let name = std::string::from("hello");
    let line = format!("package {}", name);
    io::println(line)?;
  }

  return 0;
}
```

The block is compiler syntax. It lowers to a hidden
`zone::temp(zone::default_capacity())` local, pushes that local as the current
allocation zone, checks the body, and inserts the same cleanup that an
explicit temporary zone binding receives. `zone::default_capacity()` is 4096
bytes today. A larger scratch region can be requested with
`zone(capacity) { ... }`:

```ari
zone(65536) {
  let source = fs::read_to_string("Ari.toml")?;
  let rendered = format!("manifest bytes = {}", source.as_slice().len);
}
```

Capacity is a runtime limit for the temporary arena. If `zone { ... }` is too
small for the values created inside it, the hosted runtime writes a diagnostic
to stderr and exits. The diagnostic suggests `zone(capacity) { ... }` or a
larger explicit zone. Invalid capacities, invalid zone handles, and malformed
raw allocation requests also get named runtime diagnostics instead of a silent
exit.

Inside a current-zone block, calls may omit exactly one `ref mut Zone`
parameter. The checker inserts the current zone for ordinary functions,
generic functions, ordinary methods, associated functions, trait-qualified
methods, dyn trait-object methods, and callable values when the arity
otherwise matches. This lets the natural, short spelling call the same stdlib
implementation:

```ari
zone {
  let text = string::from("abc");                  // string::from(ref mut zone, ...)
  let args = env::args();                          // env::args(ref mut zone)
  let file = fs::read_to_string("notes.txt")?;     // fs::read_to_string(ref mut zone, ...)
  let rendered = std::fmt::Display::format_in(42); // Display::format_in(42, ref mut zone)
  let line = format!("{} {}", text, file.as_slice().len); // format_in!(ref mut zone, ...)
}
```

Explicit `ref mut zone` still wins when data should live in another region:

```ari
var outer = zone::create(4096);
zone {
  let persistent = string::from(ref mut outer, "keep this");
}
zone::destroy(outer);
```

Nested current-zone blocks shadow outer ones. Values allocated in an inner
block cannot escape to outer bindings or returns unless they are copied or
promoted into an outer explicit zone.

`alloc(ref mut zone, bytes, align)` is the raw byte allocator. It returns a
`ptr u8` with the requested alignment.

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
zone handle. Prefer `metadata(data)`, which wraps that handle in
`ZoneMetadata`. `from_zone(ref mut zone)` creates the same typed metadata from
an existing zone capability without requiring a prior payload allocation.

`ZoneBacked` is the high-level wrapper for library handles that own
zone-backed storage. `zone::of(ref value)` and `value.zone()` expose
`ZoneMetadata` for supported handles such as `Box[T]`, `String`, `Vec[T]`, and
the zone-backed `std::collections` handles, including map update-entry handles.
`metadata.as_ptr()` exposes the raw opaque handle, `metadata.as_zone_ptr()`
gives the same address typed as `ptr Zone`, and `metadata.equals(ref other)`
checks handle identity.
`metadata.alloc(bytes, align)` and `metadata.alloc_array<T>(count)` allocate
through that recovered runtime zone handle; this is the preferred internal
building block for heap handles that need to grow without carrying an explicit
`ref mut Zone` argument.

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

Current-zone blocks are still explicit allocation. The zone is just hidden
because its lifetime is the block itself. If a function has two zone
parameters, or if the missing argument is not uniquely identifiable as
`ref mut Zone`, the checker requires the explicit call.

If code outside a current-zone block omits only the zone argument, Ari reports
that the call needs an allocation zone and suggests either `zone { ... }` or an
explicit `ref mut Zone`. That diagnostic is intentionally different from a
plain wrong-argument-count error because allocation lifetime is the important
choice.

The current zone is intentionally not exposed as a general `zone::current()`
library function. Library APIs still declare `ref mut Zone`; the compiler only
fills that parameter at call sites where the lifetime is lexical and obvious.
That keeps temporary allocation easy to write without letting library code
silently depend on a global allocator.

Raw memory is not automatically initialized. After `alloc<T>` or
`alloc_array<T>`, write each slot before reading it. If the element type owns
resources, prefer a higher-level handle or an API that clearly owns drop
behavior.

## Implementation Spec

Current-zone blocks are intentionally lexical and conservative:

1. `zone { body }` parses as a statement. `zone` is contextual, so existing
   values named `zone` continue to work outside this exact statement shape.
2. The checker creates a hidden `own Zone` local with the default capacity,
   or with the checked `i64` expression from `zone(capacity)`.
3. That hidden local is pushed onto the current-zone stack while `body` is
   checked. Nested blocks shadow outer current zones.
4. A call inside the body may omit exactly one argument when the omitted
   parameter is uniquely typed as `ref mut Zone`. Ordinary functions, generic
   functions, ordinary methods, associated functions, trait-qualified methods,
   dyn trait-object methods, and callable values use this rule.
5. `format!` is the compiler-owned formatting shortcut for the current zone.
   Outside a current-zone block it remains a diagnostic and callers should use
   `format_in!(ref mut zone, ...)`.
6. The hidden zone receives the same cleanup treatment as `zone::temp`: it is
   destroyed on fallthrough and when `return`, `break`, `continue`, or labeled
   exits leave the block.
7. Existing zone-provenance checks still decide whether pointers and
   zone-backed handles may escape. Current-zone syntax does not make a global
   heap and does not extend object lifetimes.

## Tests

- `tests/cases/standard-library/ok/zone/std-zone-alloc-array.ari` checks
  `std::zone::alloc_array`, the root `alloc_array` alias, null return for
  zero count, pointer loads/stores, LLVM symbol emission, and runtime result.
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

- scoped allocation helpers that can express lifetime boundaries in source
- possible `new zone { ... }` spelling if Ari later gets a general resource
  construction grammar; today `zone { ... }` keeps `zone` contextual and
  avoids reserving another keyword
- capacity planning helpers for common parser/formatter workloads, so callers
  can choose `zone(capacity) { ... }` before entering the block
- allocator traits after trait object and generic impl conventions settle
- richer diagnostics for raw allocation of ownership-heavy values
