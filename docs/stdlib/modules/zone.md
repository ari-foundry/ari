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
zone::reset(ref mut Zone) -> void
zone::destroy(zone: own Zone) -> void

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
- Existing zone, vector, string, and boxed tests cover reset/destroy
  invalidation and zone-backed handle provenance.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused zone allocation coverage.

## Future Work

- scoped allocation helpers that can express lifetime boundaries in source
- allocator traits after trait object and generic impl conventions settle
- richer diagnostics for raw allocation of ownership-heavy values
