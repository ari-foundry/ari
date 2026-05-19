# std::sync

`std::sync` holds the small pieces that make concurrent code explicit. The
first slice is intentionally narrow: a concrete `AtomicI64` value with natural
operation names. It gives Ari a tested synchronization primitive without
pretending that captured thread entries, shared ownership, locks, or channels
are designed yet.

All current operations use sequentially consistent ordering. Ari does not
expose memory-order parameters yet because the language still needs a clear
send/share story before weaker ordering choices are useful to most users.

## API

```ari
AtomicI64::new(value: i64) -> AtomicI64

sync::load(value: ref AtomicI64) -> i64
sync::store(value: ref mut AtomicI64, replacement: i64) -> void
sync::swap(value: ref mut AtomicI64, replacement: i64) -> i64
sync::fetch_add(value: ref mut AtomicI64, amount: i64) -> i64
sync::compare_exchange(value: ref mut AtomicI64, expected: i64, replacement: i64) -> bool

atomic.load() -> i64
atomic.store(replacement) -> void
atomic.swap(replacement) -> i64
atomic.fetch_add(amount) -> i64
atomic.compare_exchange(expected, replacement) -> bool
```

`load` reads the current value. `store` replaces it without returning the old
value. `swap` replaces the value and returns the previous value.
`fetch_add` adds to the value and returns the previous value.
`compare_exchange` stores `replacement` only when the current value equals
`expected`; it returns whether the exchange happened.

The root prelude re-exports `AtomicI64`, so ordinary programs can write
`AtomicI64::new(0)` and keep the operation names on the value.

## Example

```ari
fn main() -> i64 {
  var counter = AtomicI64::new(0);

  let before = counter.fetch_add(1);
  if before != 0 {
    return 1;
  }

  if !counter.compare_exchange(1, 10) {
    return 2;
  }

  return counter.load();
}
```

## Current Limits

- Only `AtomicI64` exists. Generic atomics should wait for a stable numeric and
  memory-model design.
- All operations are sequentially consistent.
- There is no `Shared`, `Weak`, `Mutex`, channel, or send/share trait policy
  yet.
- Current `std::thread` entries cannot capture references to an atomic value.
  This module is still useful for backend/runtime lowering and for future
  thread-sharing work, but it is not a full shared-state story by itself.

## Tests

- `tests/cases/standard-library/ok/sync/std-sync-atomic-i64.ari` checks
  module functions, method wrappers, root `AtomicI64`, compare-exchange success
  and failure paths, and LLVM atomic lowering.

For small sync edits, run `make check-std-api`, then compile this test with
`--emit-llvm` and run the resulting executable. Use the broader prelude target
when the change touches shared `std` loading or root aliases.
