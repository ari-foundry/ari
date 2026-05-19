# std::sync

`std::sync` holds the small pieces that make concurrent code explicit. The
current slice has a concrete `AtomicI64`, plus source `Mutex` and `Once`
helpers built on that atomic primitive. It also includes a first explicit
`RwLock` primitive for reader/writer coordination. It gives Ari tested
synchronization vocabulary without pretending that captured thread entries,
shared ownership, guard lifetimes, condition variables, or channels are fully
designed yet.

All current operations use sequentially consistent ordering. Ari does not
expose memory-order parameters yet because the language still needs a clear
send/share story before weaker ordering choices are useful to most users.

## Atomic API

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

## Mutex API

```ari
Mutex::new() -> Mutex

sync::try_lock(ref mut mutex) -> bool
sync::lock(ref mut mutex) -> void
sync::unlock(ref mut mutex) -> void
sync::is_locked(ref mutex) -> bool

mutex.try_lock() -> bool
mutex.lock() -> void
mutex.unlock() -> void
mutex.is_locked() -> bool
```

`Mutex` is a small explicit lock primitive. `try_lock` attempts one
compare-exchange and returns immediately. `lock` spins and yields with
`thread::yield_now()` until the lock is acquired. `unlock` stores the unlocked
state. `is_locked` is a diagnostic predicate, not a synchronization proof.

This is not yet a generic `Mutex[T]` and does not create an RAII guard. Use
explicit `lock`/`unlock` in tightly scoped code and keep future value-protecting
locks on the roadmap until Ari has a send/share and guard-lifetime policy.

## RwLock API

```ari
RwLock::new() -> RwLock

sync::try_read_lock(ref mut lock) -> bool
sync::read_lock(ref mut lock) -> void
sync::read_unlock(ref mut lock) -> void
sync::try_write_lock(ref mut lock) -> bool
sync::write_lock(ref mut lock) -> void
sync::write_unlock(ref mut lock) -> void
sync::reader_count(ref lock) -> i64
sync::is_read_locked(ref lock) -> bool
sync::is_write_locked(ref lock) -> bool

lock.try_read_lock() -> bool
lock.read_lock() -> void
lock.read_unlock() -> void
lock.try_write_lock() -> bool
lock.write_lock() -> void
lock.write_unlock() -> void
lock.reader_count() -> i64
lock.is_read_locked() -> bool
lock.is_write_locked() -> bool
lock.is_locked() -> bool
```

`RwLock` allows multiple readers or one writer. `try_read_lock` increments the
reader count only when no writer is active. `try_write_lock` succeeds only when
there are no readers and no writer. The blocking forms spin and yield with
`thread::yield_now()`. `reader_count` and the predicate helpers are diagnostic
views, not synchronization proofs.

Like `Mutex`, this is not yet a generic `RwLock[T]` and does not create read or
write guards. Use explicit unlock calls in small scopes. Future guard-bearing
APIs need send/share, value storage, poisoning/no-poisoning, and wait/wake
policy first.

## Once API

```ari
Once::new() -> Once

sync::call_once(ref mut once, action: fn() -> void) -> bool
sync::is_completed(ref once) -> bool

once.call_once(action) -> bool
once.is_completed() -> bool
```

`call_once` runs `action` exactly once for a given `Once` value. It returns
`true` for the caller that ran the action and `false` for later callers.
Concurrent callers spin/yield while another caller is running the action.
If the action panics, the current executable exits through Ari's panic path; a
recoverable poisoning policy is intentionally not promised yet.

The root prelude re-exports `Mutex`, `RwLock`, and `Once` alongside
`AtomicI64`.

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

```ari
fn initialize() -> void {
  return;
}

fn main() -> i64 {
  var mutex = Mutex::new();
  mutex.lock();
  mutex.unlock();

  var readers = RwLock::new();
  readers.read_lock();
  readers.read_unlock();

  var once = Once::new();
  once.call_once(initialize);
  return 0;
}
```

## Current Limits

- Only `AtomicI64` exists in the atomic family. Generic atomics should wait
  for a stable numeric and memory-model design.
- All operations are sequentially consistent.
- `Mutex` and `RwLock` are primitive locks only. There is no `Mutex[T]`,
  `RwLock[T]`, guard type, or poisoning/no-poisoning policy yet.
- `Once` runs plain `fn() -> void` entries only. `OnceLock[T]` and `LazyLock`
  need value storage, initialization result policy, and shared access rules.
- There is no `Shared`, `Weak`, `Condvar`, `Barrier`, channel, or send/share
  trait policy yet.
- Current `std::thread` entries cannot capture references to an atomic value.
  This module is still useful for backend/runtime lowering and for future
  thread-sharing work, but it is not a full shared-state story by itself.

## Synchronization Roadmap

| Feature | Status |
| --- | --- |
| Atomic types | Current `AtomicI64`; future generic integer atomics and pointer atomics. |
| Memory ordering | Current operations are sequentially consistent; future `Ordering` parameters should land only with teachable memory-model docs and tests. |
| Mutex | Current source primitive `Mutex`; future value-protecting `Mutex[T]`, guard lifetime, poisoning/no-poisoning policy, and futex-backed runtime implementation. |
| RwLock | Current source primitive with explicit read/write lock and unlock helpers; future value-protecting `RwLock[T]`, read/write guards, fairness policy, and futex-backed implementation. |
| Condvar | Roadmap after `Mutex` guards and OS wait/wake runtime hooks exist. |
| Once | Current source `Once`; future panic/poison policy. |
| OnceLock | Roadmap after value storage and shared access policy are stable. |
| LazyLock | Roadmap after `OnceLock` and function/closure initialization policy. |
| Barrier | Roadmap after thread group coordination and wake primitives. |
| Semaphore | Optional roadmap after permit ownership and async/blocking policy. |
| MPSC channel | Roadmap after allocation ownership, send/share traits, and blocking wake policy. |
| Futex primitives | Linux internal implementation detail for future blocking locks, not a portable public API. |

## Tests

- `tests/cases/standard-library/ok/sync/std-sync-atomic-i64.ari` checks
  module functions, method wrappers, root `AtomicI64`, compare-exchange success
  and failure paths, and LLVM atomic lowering.
- `tests/cases/standard-library/ok/sync/std-sync-mutex-once.ari` checks
  source `Mutex` lock/unlock helpers, source `Once` state transitions, method
  wrappers, root aliases, and atomic compare-exchange lowering through the
  implementation.
- `tests/cases/standard-library/ok/sync/std-sync-rwlock.ari` checks source
  `RwLock` read/write state transitions, method wrappers, root alias, reader
  counts, and atomic compare-exchange/fetch-add lowering through the
  implementation.

For small sync edits, run `make check-std-api`, then compile this test with
`--emit-llvm` and run the resulting executable. Use the broader prelude target
when the change touches shared `std` loading or root aliases.
