# std::sync

`std::sync` is Ari's explicit thread-coordination module. It now covers the
small production vocabulary that other standard-library modules need first:
concrete atomics, primitive locks, condition variables, one-time initialization,
a reusable barrier, and a first MPSC channel shape.

The current implementations are intentionally teachable source-level building
blocks. `AtomicI64` lowers to LLVM atomics on the hosted backend; wider wrappers
compose over it. `Mutex`, `RwLock`, `Condvar`, `Barrier`, `Once`, `OnceLock`,
and channels spin/yield instead of using a futex-backed blocking runtime yet.
That keeps the API useful while preserving space for later guard, poisoning,
send/share, and wait/wake policy.

## Memory Ordering

```ari
enum sync::Ordering {
  Relaxed,
  Acquire,
  Release,
  AcqRel,
  SeqCst,
}

sync::seq_cst() -> sync::Ordering
sync::is_load_order(ordering) -> bool
sync::is_store_order(ordering) -> bool
sync::is_rmw_order(ordering) -> bool
```

All current atomic lowering is sequentially consistent. The explicit
`Ordering` enum exists so APIs can name their contract now, and so user code can
be written in the same shape that weaker backend lowering will use later.
Invalid ordering combinations assert:

- loads accept `Relaxed`, `Acquire`, and `SeqCst`
- stores accept `Relaxed`, `Release`, and `SeqCst`
- read-modify-write operations accept all current variants

## Atomic API

```ari
AtomicI64::new(value)
AtomicBool::new(value)
AtomicUsize::new(value)
AtomicPtr::new<T>(ptr)
AtomicPtr::null_ptr<T>()

atomic.load()
atomic.load_order(ordering)
atomic.store(replacement)
atomic.store_order(replacement, ordering)
atomic.swap(replacement)
atomic.compare_exchange(expected, replacement)

i64_atomic.fetch_add(amount)
i64_atomic.fetch_add_order(amount, ordering)
i64_atomic.swap_order(replacement, ordering)
i64_atomic.compare_exchange_order(expected, replacement, success, failure)
usize_atomic.fetch_add(amount)
```

`AtomicI64` is the runtime-backed primitive. `AtomicBool`, `AtomicUsize`, and
`AtomicPtr[T]` are source wrappers that preserve the natural API names while
reusing the i64 primitive. `AtomicPtr[T]` stores the raw pointer value as a
machine-sized unsigned integer; normal pointer loads, stores, casts, and payload
access still use the returned user pointer directly.

The root prelude re-exports `AtomicI64`, `AtomicBool`, `AtomicUsize`, and
`AtomicPtr`.

## Mutex API

```ari
Mutex::new() -> Mutex

sync::try_lock(ref mut mutex) -> bool
sync::lock(ref mut mutex) -> void
sync::unlock(ref mut mutex) -> void
sync::is_locked(ref mutex) -> bool

mutex.try_lock()
mutex.lock()
mutex.unlock()
mutex.is_locked()
```

`Mutex` is a primitive explicit lock. `try_lock` performs one compare-exchange,
`lock` spins and calls `thread::yield_now()` until it succeeds, and `unlock`
stores the unlocked state. `is_locked` is a diagnostic predicate, not a proof
that a later operation will observe the same state.

This is not yet `Mutex[T]` and does not produce a guard. Keep lock/unlock scopes
small and explicit until Ari has value-protecting locks and guard lifetimes.

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

lock.try_read_lock()
lock.read_lock()
lock.read_unlock()
lock.try_write_lock()
lock.write_lock()
lock.write_unlock()
lock.reader_count()
lock.is_read_locked()
lock.is_write_locked()
lock.is_locked()
```

`RwLock` allows multiple readers or one writer. Readers increment the state
while no writer is present; writers use `-1` as the exclusive state. Blocking
forms spin/yield. Like `Mutex`, this is still a primitive lock without protected
payloads, guards, poisoning policy, fairness policy, or futex-backed parking.

## Once And OnceLock

```ari
Once::new() -> Once
sync::call_once(ref mut once, action: fn() -> void) -> bool
sync::is_completed(ref once) -> bool
once.call_once(action) -> bool
once.is_completed() -> bool

OnceLock::new<T>() -> OnceLock[T]
once_lock.set(value) -> bool
once_lock.get() -> option::OptionRef[T]
once_lock.get_mut() -> option::OptionMut[T]
once_lock.get_or_init(initializer: fn() -> T) -> ref T
once_lock.take() -> Option[T]
once_lock.is_initialized() -> bool
once_lock.is_empty() -> bool
```

`Once` runs a plain `fn() -> void` at most once. It returns `true` for the caller
that ran the action and `false` for later callers. Concurrent callers spin/yield
while the action is in progress.

`OnceLock[T]` is the sync-facing one-time value slot. `set` wins only when the
slot is empty. `get` and `get_mut` expose borrowed option views so callers can
inspect initialized values without copying. `get_or_init` initializes lazily and
returns a shared reference. `take` resets an initialized slot and returns its
value.

## Condvar

```ari
Condvar::new() -> Condvar
condvar.notify_one() -> void
condvar.notify_all() -> void
condvar.generation() -> i64
condvar.wait(ref mut mutex) -> void
condvar.wait_while(ref mut mutex, condition: fn() -> bool) -> void
```

The current `Condvar` is generation-based and source-level. `notify_one` and
`notify_all` both advance a notification generation. `wait` records the current
generation, unlocks the mutex, yields until the generation changes, then locks
the mutex again. `wait_while` repeats while the predicate returns `true`.

This API gives code the usual condition-variable shape now. It does not yet
promise OS parking, fairness, spurious wake policy, timeout waits, or a
guard-bearing `Mutex[T]` integration.

## Barrier

```ari
Barrier::new(parties) -> Barrier
barrier.wait() -> bool
barrier.parties() -> i64
```

`Barrier` coordinates a fixed number of callers. `wait` returns `true` to the
caller that completes the generation and `false` to callers released by that
completion. The implementation is reusable across generations and spins/yields
while waiting.

## Channel And MPSC

```ari
sync::channel<T>(ref mut zone) -> Channel[T]
sync::mpsc_channel<T>(ref mut zone) -> Channel[T]

channel.sender() -> Sender[T]
channel.receiver() -> Receiver[T]
channel.split() -> Channel[T]

sender.send(value) -> bool
sender.close() -> void
sender.is_closed() -> bool

receiver.try_recv() -> Option[T]
receiver.recv() -> Option[T]
receiver.close() -> void
receiver.is_closed() -> bool
receiver.is_empty() -> bool
```

The first channel is a single-slot MPSC shape. `send` is nonblocking and returns
`false` when the slot is full or closed. `try_recv` returns immediately.
`recv` yields until a value arrives or the channel closes, then returns
`Some(value)` or `None`. The channel state is allocated in the caller's `Zone`;
`Sender`, `Receiver`, and `Channel` carry only the shared state pointer, not a
redundant zone handle.

Future channel work should add bounded capacity, blocking wake integration,
`send` returning the unsent value on failure, and send/share trait checks.

## Example

```ari
fn make_value() -> i64 {
  return 77;
}

fn main() -> i64 {
  var flag = AtomicBool::new(false);
  flag.store_order(true, sync::Release);
  if !flag.load_order(sync::Acquire) {
    return 1;
  }

  var once = OnceLock::new<i64>();
  once.set(make_value());
  let current = once.get();
  if current.unwrap() != 77 {
    return 2;
  }

  var zone = zone::create(4096);
  let channel = sync::channel<i64>(ref mut zone);
  var tx = channel.sender();
  var rx = channel.receiver();
  if !tx.send(10) {
    return 3;
  }
  if rx.recv().unwrap_or(0) != 10 {
    return 4;
  }
  zone::destroy(zone);
  return 0;
}
```

## Current Limits

- Atomic lowering is still effectively sequentially consistent even when APIs
  accept `Ordering` values.
- `AtomicBool`, `AtomicUsize`, and `AtomicPtr[T]` are wrappers over
  `AtomicI64`, not target-specialized native atomic widths yet.
- `Mutex`, `RwLock`, `Condvar`, `Barrier`, and channels spin/yield; they do not
  park on futexes or condition-variable OS primitives.
- There are no value-protecting `Mutex[T]` or `RwLock[T]` guard types yet.
- There is no `LazyLock[T]`, semaphore, or timeout wait yet. Explicit
  `ThreadLocal[T]` handles live in `std::thread`; compiler-level
  `thread_local` declarations remain future work.
- Send/share trait checking is still roadmap work, so cross-thread value
  transfer APIs remain conservative.

## Synchronization Roadmap

| Feature | Status |
| --- | --- |
| Atomic types | Current `AtomicI64`, `AtomicBool`, `AtomicUsize`, and `AtomicPtr[T]`; future target-native width policy and more integer widths. |
| Memory ordering | Current explicit `Ordering` vocabulary with SeqCst lowering; future relaxed/acquire/release backend mapping and examples. |
| Mutex/RwLock | Current primitive lock state; future value-protecting guards, poisoning/no-poisoning, fairness, and futex-backed parking. |
| Condvar | Current generation-based source API; future blocking wait/wake, spurious wake documentation, and timeout waits. |
| Once/OnceLock | Current source one-time execution and value slot; future panic/poison policy and optional `LazyLock`. |
| Barrier | Current source reusable barrier; future parking implementation. |
| MPSC channel | Current single-slot MPSC shape; future bounded queues, blocking wake, unsent-value return, and close semantics. |
| Thread local | Current explicit `std::thread::ThreadLocal[T]` handles; future compiler-level static TLS declarations and destructor policy. |

## Tests

- `tests/cases/standard-library/ok/sync/std-sync-atomic-i64.ari` checks
  module functions, method wrappers, root `AtomicI64`, compare-exchange success
  and failure paths, and LLVM atomic lowering.
- `tests/cases/standard-library/ok/sync/std-sync-mutex-once.ari` checks source
  `Mutex` lock/unlock helpers, source `Once` transitions, method wrappers, root
  aliases, and compare-exchange lowering through the implementation.
- `tests/cases/standard-library/ok/sync/std-sync-rwlock.ari` checks source
  `RwLock` read/write state transitions, method wrappers, root alias, reader
  counts, and compare-exchange/fetch-add lowering.
- `tests/cases/standard-library/ok/sync/std-sync-concurrency-api.ari` checks
  explicit order validation, `AtomicBool`, `AtomicUsize`, `AtomicPtr[T]`,
  `OnceLock`, `Condvar`, `Barrier`, and the single-slot channel API.

For small sync edits, run `make check-std-api`, then compile the relevant test
with `--emit-llvm` and run the resulting executable. Use the broader prelude
target when the change touches shared `std` loading, root aliases, or runtime
hooks.
