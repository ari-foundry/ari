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

ZoneMetadata
metadata.as_ptr() -> ptr c_void
metadata.as_zone_ptr() -> ptr Zone
metadata.alloc(bytes: i64, align: i64) -> ptr u8
metadata.alloc_array<T>(count: i64) -> ptr T
metadata.equals(ref other) -> bool
ZoneBacked

create(capacity)
alloc(ref mut zone, bytes, align)
alloc<T>(ref mut zone)
alloc_array<T>(ref mut zone, count)
new<T>(ref mut zone, value)
promote<T>(ref mut target, source)
reset(ref mut zone)
destroy(zone)
```

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
the zone-backed `std::collections` handles. `metadata.as_ptr()` exposes the
raw opaque handle, `metadata.as_zone_ptr()` gives the same address typed as
`ptr Zone`, and `metadata.equals(ref other)` checks handle identity.
`metadata.alloc(bytes, align)` and `metadata.alloc_array<T>(count)` allocate
through that recovered runtime zone handle; this is the preferred internal
building block for heap handles that need to grow without carrying an explicit
`ref mut Zone` argument.

Raw header recovery requires an actual backing allocation. Empty or
zero-capacity handles may have no data pointer header to read, so use
`from_zone(ref mut zone)` or a handle API that caches `ZoneMetadata` when code
needs a zone before the first allocation.

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

Raw memory is not automatically initialized. After `alloc<T>` or
`alloc_array<T>`, write each slot before reading it. If the element type owns
resources, prefer a higher-level handle or an API that clearly owns drop
behavior.

## Tests

- `tests/cases/standard-library/ok/zone/std-zone-alloc-array.ari` checks
  `std::zone::alloc_array`, the root `alloc_array` alias, null return for
  zero count, pointer loads/stores, LLVM symbol emission, and runtime result.
- `tests/cases/standard-library/ok/zone/std-zone-backed.ari` checks
  `ZoneMetadata`, `ZoneBacked`, `zone::metadata(data)`, `zone::from_zone`,
  `metadata.alloc_array<T>`, `metadata.as_ptr()`,
  `metadata.as_zone_ptr()`, `zone::of(ref value)`, `value.zone()`, and raw
  allocation-header agreement for box, string, vector, set, map, sequence,
  linked list, heap, and priority queue handles.
- Existing zone, vector, string, and boxed tests cover reset/destroy
  invalidation and zone-backed handle provenance.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused zone allocation coverage.

## Future Work

- scoped allocation helpers that can express lifetime boundaries in source
- allocator traits after trait object and generic impl conventions settle
- richer diagnostics for raw allocation of ownership-heavy values
