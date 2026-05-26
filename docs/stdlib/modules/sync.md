# std::sync

`std::sync` is Ari's explicit thread-coordination module. It now covers the
small production vocabulary that other standard-library modules need first:
concrete atomics, primitive locks, condition variables, one-time initialization,
a reusable barrier, and a first MPSC channel shape.

The current implementations are intentionally teachable building blocks.
`AtomicI64` lowers to LLVM atomics on the hosted backend, including explicit
memory-order variants; wider wrappers compose over it. `Mutex`, `RwLock`,
`Condvar`, `Barrier`, `Once`, `OnceLock`, and channels spin/yield instead of
using a futex-backed blocking runtime yet. That keeps the API useful while
preserving space for later value-protecting locks, poisoning, send/share, and
wait/wake policy.

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
sync::is_compare_exchange_order(success, failure) -> bool
```

Default atomic methods are sequentially consistent. Explicit-order methods
lower to matching LLVM atomic orderings on the hosted backend:

- `Relaxed` lowers to LLVM `monotonic`
- `Acquire` lowers to LLVM `acquire`
- `Release` lowers to LLVM `release`
- `AcqRel` lowers to LLVM `acq_rel`
- `SeqCst` lowers to LLVM `seq_cst`

Invalid ordering combinations assert before reaching the runtime hook. Loads
accept `Relaxed`, `Acquire`, and `SeqCst`; stores accept `Relaxed`, `Release`,
and `SeqCst`; read-modify-write operations accept every current variant.
Compare-exchange validates both success and failure orderings: failure may be
`Relaxed`, may be `Acquire` only when the success ordering observes reads, and
may be `SeqCst` only when success is also `SeqCst`.

Invalid memory-order arguments are programmer errors, not recoverable runtime
failures. The ordered atomic methods assert/panic for invalid combinations
instead of returning `Result`, so callers should validate dynamic ordering
values with `is_load_order`, `is_store_order`, or
`is_compare_exchange_order` before calling them.

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
atomic.compare_exchange(expected, replacement) -> Result[old, current]
atomic.compare_exchange_bool(expected, replacement) -> bool

i64_atomic.fetch_add(amount)
i64_atomic.fetch_add_order(amount, ordering)
i64_atomic.swap_order(replacement, ordering)
i64_atomic.compare_exchange_order(expected, replacement, success, failure) -> Result[old, current]
i64_atomic.compare_exchange_order_bool(expected, replacement, success, failure) -> bool
usize_atomic.fetch_add(amount)
```

`AtomicI64` is the runtime-backed primitive. `AtomicBool`, `AtomicUsize`, and
`AtomicPtr[T]` are source wrappers that preserve the natural API names while
reusing the i64 primitive. `AtomicPtr[T]` stores the raw pointer value as a
machine-sized unsigned integer; normal pointer loads, stores, casts, and payload
access still use the returned user pointer directly.

The natural `compare_exchange` methods now return `Result`: `Ok(old)` means
the expected value was observed and replaced, and `Err(current)` reports the
current value observed after a failed exchange. `_bool` compatibility methods
keep the older success/failure-only shape for low-level code. `AtomicPtr[T]`
returns the old/current raw pointer values as `u64` in the Result payload; cast
back to `ptr T` only at the boundary where a raw pointer is actually needed.

The root prelude re-exports `AtomicI64`, `AtomicBool`, `AtomicUsize`, and
`AtomicPtr`.

## Mutex API

```ari
Mutex::new() -> Mutex

sync::try_lock(ref mut mutex) -> bool
sync::lock(ref mut mutex) -> void
sync::unlock(ref mut mutex) -> void
sync::is_locked(ref mutex) -> bool

mutex.try_lock() -> Option[MutexGuard]
mutex.lock() -> MutexGuard
mutex.try_lock_bool() -> bool
mutex.lock_raw() -> void
mutex.unlock()
mutex.is_locked()

guard.unlock()
guard.is_active()
```

`Mutex` is a primitive explicit lock. The natural method API now returns a
guard: `try_lock` performs one compare-exchange and returns `Some(MutexGuard)`
on success, while `lock` spins and calls `thread::yield_now()` until it can
return an active guard. `MutexGuard::unlock` releases the lock once and then
marks the guard inactive; calling it again is harmless. Explicit `drop guard`
also releases an active guard.

The top-level `sync::try_lock`, `sync::lock`, and method compatibility helpers
`try_lock_bool` and `lock_raw` expose the older manual bool/void behavior for
low-level code such as `Condvar`. They require a matching explicit `unlock`.
`is_locked` is a diagnostic predicate, not a proof that a later operation will
observe the same state.

This is still not `Mutex[T]`: the guard owns the unlock operation, not a
protected payload borrow. Current Ari code should use `guard.unlock()` or
explicit `drop guard`; automatic scope/early-return RAII unlock is not a
language guarantee yet.

Ari's current `Mutex` is not poisoned. If code panics or otherwise fails while
holding a lock, later lock acquisition does not report that event. Consistency
of shared state after failure is the caller's responsibility. A future
`PoisonMutex`/`PoisonRwLock` can add opt-in poisoning without changing this
default no-poison policy.

## RwLock API

```ari
RwLock::new() -> RwLock

lock.try_read() -> Option[RwLockReadGuard]
lock.read() -> RwLockReadGuard
lock.try_write() -> Option[RwLockWriteGuard]
lock.write() -> RwLockWriteGuard

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

read_guard.unlock()
read_guard.is_active()
write_guard.unlock()
write_guard.is_active()
```

`RwLock` allows multiple readers or one writer. Readers increment the state
while no writer is present; writers use `-1` as the exclusive state. The
natural method API returns guards: `read` and `write` spin/yield until they can
return active guards, and `try_read`/`try_write` return `None` when acquisition
would block. `RwLockReadGuard::unlock` and `RwLockWriteGuard::unlock` are
idempotent, and explicit `drop guard` releases an active guard.

The `*_lock` and `*_unlock` functions and methods remain manual compatibility
APIs for low-level code. Like `Mutex`, this is still a primitive lock without
protected payload borrows, poisoning policy, fairness policy, or futex-backed
parking. Use explicit `drop guard` or `guard.unlock()` today; automatic
scope/early-return guard cleanup remains language/runtime work.

`RwLock` follows the same no-poison policy as `Mutex`: unlock after a panic or
explicit failure does not mark the lock poisoned, and later readers/writers do
not receive a poison error.

## Once And OnceLock

```ari
Once::new() -> Once
sync::call_once(ref mut once, action: fn() -> void) -> bool
sync::is_completed(ref once) -> bool
once.call_once(action) -> bool
once.is_completed() -> bool

OnceLock::new<T>() -> OnceLock[T]
once_lock.set(value) -> Result[(), T]
once_lock.set_bool(value) -> bool
once_lock.get() -> option::OptionRef[T]
once_lock.get_mut() -> option::OptionMut[T]
once_lock.get_or_init(initializer: fn() -> T) -> ref T
once_lock.get_or_try_init(initializer: fn() -> Result[T, Error]) -> Result[(), Error]
once_lock.take() -> Option[T]
once_lock.is_initialized() -> bool
once_lock.is_empty() -> bool
```

`Once` runs a plain `fn() -> void` at most once. It returns `true` for the caller
that ran the action and `false` for later callers. Concurrent callers spin/yield
while the action is in progress.

`OnceLock[T]` is the sync-facing one-time value slot. `set` wins only when the
slot is empty and returns the rejected value on failure. `set_bool` preserves
the older compatibility shape and drops the rejected value. `get` and `get_mut`
expose borrowed option views so callers can inspect initialized values without
copying. `get_or_init` initializes lazily and returns a shared reference.
`get_or_try_init` lets the initializer fail; on `Err`, the slot is reset to
empty and callers can try again. Because current Ari `Result` payloads cannot
ergonomically carry `ref T`, `get_or_try_init` returns `Result[(), Error]`;
call `get()` after `Ok(())` to borrow the initialized value. `take` resets an
initialized slot and returns its value.

## Condvar

```ari
Condvar::new() -> Condvar
condvar.notify_one() -> void
condvar.notify_all() -> void
condvar.generation() -> i64
condvar.wait(ref mut mutex) -> void
condvar.wait_timeout(ref mut mutex, duration) -> Result[WaitTimeoutResult, Error]
condvar.wait_while(ref mut mutex, condition: fn() -> bool) -> void
wait_result.timed_out() -> bool
```

The current `Condvar` is generation-based and source-level. `notify_one` and
`notify_all` both advance a notification generation. `wait` records the current
generation, unlocks the mutex, yields until the generation changes, then locks
the mutex again. `wait_timeout` follows the same spin/yield policy but returns
`Ok(WaitTimeoutResult)` with `timed_out() == true` when the monotonic deadline
expires before another generation is observed. `wait_while` repeats while the
predicate returns `true`.

This API gives code the usual condition-variable shape now. It does not yet
promise OS parking, fairness, spurious wake policy, or a guard-bearing
`Mutex[T]` integration. `notify_one` is currently not selective; like
`notify_all`, it only advances the shared generation. Treat the current
primitive as a hosted runtime spin/yield condition helper, not as a pthread
condition variable or futex-backed sleeping wait.

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

sender.send(value) -> Result[(), SendError[T]]
sender.try_send(value) -> Result[(), TrySendError[T]]
sender.send_bool(value) -> bool
sender.clone() -> Sender[T]
sender.close() -> void
sender.is_closed() -> bool

receiver.try_recv() -> Result[T, TryRecvError]
receiver.try_recv_optional() -> Option[T]
receiver.recv() -> Result[T, RecvError]
receiver.recv_optional() -> Option[T]
receiver.close() -> void
receiver.is_closed() -> bool
receiver.is_empty() -> bool
```

The first channel is a bounded single-slot MPSC shape. `channel` and
`mpsc_channel` are aliases for that capacity-1 channel; there is no unbounded
channel constructor yet. `try_send` returns `TrySendFull(value)` when the slot
already holds a value and `TrySendClosed(value)` when the channel is closed.
`send` yields while the slot is full and returns `SendClosed(value)` if the
channel closes before the value is accepted. `try_recv` returns
`TryRecvEmpty` for an open empty slot and `TryRecvClosed` for a closed empty
channel; `recv` yields until a value arrives or the channel closes. The
`_optional` and `_bool` helpers intentionally discard error detail for
compatibility. The channel state is allocated in the caller's `Zone`; `Sender`,
`Receiver`, and `Channel` carry only the shared state pointer, not a redundant
zone handle.

`sender.clone()` creates another sender handle to the same single-slot channel
state. Cloned senders do not allocate and they do not carry separate ownership
counts yet; closing any sender closes the shared channel for all sender and
receiver handles.

Future channel work should add explicit `bounded_channel(capacity)`,
unbounded-channel policy if desired, sender-counted close semantics, timeout
receives, blocking wake integration, richer close semantics, and send/share
trait checks.

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
  once.set(make_value()).unwrap();
  let current = once.get();
  if current.unwrap() != 77 {
    return 2;
  }

  var mutex = Mutex::new();
  var guard = mutex.lock();
  if !mutex.is_locked() {
    return 3;
  }
  guard.unlock();

  var rwlock = RwLock::new();
  var read_guard = rwlock.read();
  if rwlock.reader_count() != 1 {
    return 4;
  }
  drop read_guard;

  var zone = zone::create(4096);
  let channel = sync::channel<i64>(ref mut zone);
  var tx = channel.sender();
  var tx2 = tx.clone();
  var rx = channel.receiver();
  if tx2.send(10).is_err() {
    return 5;
  }
  if rx.recv().unwrap_or(0) != 10 {
    return 6;
  }
  zone::destroy(zone);
  return 0;
}
```

## Current Limits

- `AtomicBool`, `AtomicUsize`, and `AtomicPtr[T]` are wrappers over
  `AtomicI64`, not target-specialized native atomic widths yet.
- `Mutex`, `RwLock`, `Condvar`, `Barrier`, and channels spin/yield; they do not
  park on futexes or condition-variable OS primitives.
- `Mutex` and `RwLock` do not poison after panic/failure. Shared-state
  consistency is caller-owned unless a future poison-aware type is introduced.
- `MutexGuard`, `RwLockReadGuard`, and `RwLockWriteGuard` own unlock
  operations, but there are no value-protecting `Mutex[T]` or `RwLock[T]`
  payload guards yet.
- Guard drops release active locks when callers use explicit `drop guard`;
  automatic RAII cleanup at scope exit or early return is not promised yet.
- There is no `LazyLock[T]`, semaphore, channel timeout receive,
  sender-counted close policy, or configurable channel capacity yet. Explicit
  `ThreadLocal[T]` handles live in `std::thread`; compiler-level
  `thread_local` declarations remain future work.
- Send/share trait checking is still roadmap work, so cross-thread value
  transfer APIs remain conservative.

## Synchronization Roadmap

| Feature | Status |
| --- | --- |
| Atomic types | Current `AtomicI64`, `AtomicBool`, `AtomicUsize`, and `AtomicPtr[T]`; future target-native width policy and more integer widths. |
| Memory ordering | Current explicit `Ordering` vocabulary with LLVM hosted lowering for load/store/RMW/compare-exchange; future examples and non-LLVM backend policy. |
| Mutex/RwLock | Current primitive no-poison lock state with explicit unlock guards; future value-protecting payload guards, optional poison-aware lock types, fairness, and futex-backed parking. |
| Condvar | Current generation-based source API with spin/yield timeout waits; future blocking wait/wake and spurious wake documentation. |
| Once/OnceLock | Current source one-time execution, value slot, value-preserving `set`, and fallible initializer status; future ref-in-Result return ergonomics, panic policy, and optional `LazyLock`. |
| Barrier | Current source reusable barrier; future parking implementation. |
| MPSC channel | Current single-slot MPSC shape with Result errors, unsent-value return, and clonable sender handles; future configurable bounded queues, sender-counted close semantics, timeout receives, blocking wake, and richer close semantics. |
| Thread local | Current explicit `std::thread::ThreadLocal[T]` handles; future compiler-level static TLS declarations and destructor policy. |

## Tests

- `tests/cases/standard-library/ok/sync/std-sync-atomic-i64.ari` checks
  module functions, method wrappers, root `AtomicI64`, compare-exchange success
  and failure paths, and LLVM atomic lowering.
- `tests/cases/standard-library/ok/sync/std-sync-mutex-once.ari` checks source
  `Mutex` lock/unlock helpers, guard acquisition, explicit guard unlock/drop,
  source `Once` transitions, method wrappers, root aliases, and
  compare-exchange lowering through the implementation.
- `tests/cases/standard-library/ok/sync/std-sync-rwlock.ari` checks source
  `RwLock` read/write state transitions, read/write guard acquisition,
  explicit guard unlock/drop, method wrappers, root alias, reader counts, and
  compare-exchange/fetch-add lowering.
- `tests/cases/standard-library/ok/sync/std-sync-concurrency-api.ari` checks
  explicit order validation, order-specific LLVM lowering, `AtomicBool`,
  `AtomicUsize`, `AtomicPtr[T]`, `OnceLock`, `Condvar`, `Barrier`, and the
  single-slot channel API.

For small sync edits, run `make check-std-api`, then compile the relevant test
with `--emit-llvm` and run the resulting executable. Use the broader prelude
target when the change touches shared `std` loading, root aliases, or runtime
hooks.
