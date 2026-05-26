# std::sync

`std::sync` is Ari's explicit thread-coordination module. It now covers the
small production vocabulary that other standard-library modules need first:
concrete atomics, primitive locks, value-protecting locks, condition variables,
one-time initialization, a reusable barrier, and a first MPSC channel shape.

The current implementations are intentionally teachable building blocks.
`AtomicI64` lowers to LLVM atomics on the hosted backend, including explicit
memory-order variants; wider wrappers compose over it. `Mutex[T]`,
`RwLock[T]`, `RawMutex`, `RawRwLock`, `Condvar`, `Barrier`, `Once`, `OnceLock`,
and channels spin/yield instead of using a futex-backed blocking runtime yet.
That keeps the API useful while preserving space for later poisoning,
send/share, fairness, and wait/wake policy.

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
Mutex::new<T>(value) -> Mutex[T]
mutex.try_lock() -> Option[MutexGuard[T]]
mutex.lock() -> MutexGuard[T]
mutex.get_mut() -> ref mut T
mutex.into_inner() -> T
mutex.is_locked()

guard.value() -> T
guard.value_ref() -> ref T
guard.value_mut() -> ref mut T
guard.set(value) -> void
guard.replace(value) -> T
guard.unlock()
guard.is_active()

RawMutex::new() -> RawMutex

sync::try_lock(ref mut raw_mutex) -> bool
sync::lock(ref mut raw_mutex) -> void
sync::unlock(ref mut raw_mutex) -> void
sync::is_locked(ref raw_mutex) -> bool

raw_mutex.try_lock() -> Option[RawMutexGuard]
raw_mutex.lock() -> RawMutexGuard
raw_mutex.try_lock_bool() -> bool
raw_mutex.lock_raw() -> void
raw_mutex.unlock()
raw_mutex.is_locked()

raw_guard.unlock()
raw_guard.is_active()
```

`Mutex[T]` is the value-protecting form for ordinary shared state. It owns a
small source lock plus the payload, so `lock` returns `MutexGuard[T]` and the
guard controls both unlock and payload access. `try_lock` performs one
compare-exchange and returns `Some(MutexGuard[T])` on success; `lock` spins and
calls `thread::yield_now()` until it can return an active guard. Use
`value_ref` for shared borrowing, `value_mut` when a short mutable borrow fits
the current borrow checker, and `set`/`replace` when you want mutation without
keeping a returned reference alive. `get_mut` is for exclusive access to the
whole `Mutex[T]`, before other code has an active guard; `into_inner` consumes
the lock and returns the payload.

`MutexGuard[T]::unlock` releases the lock once and then marks the guard
inactive; calling it again is harmless. Explicit `drop guard` also releases an
active guard. Automatic scope/early-return RAII unlock is not a language
guarantee yet, so current Ari code should still use `guard.unlock()` or
explicit `drop guard` at clear ownership boundaries.

`RawMutex` is the manual primitive for low-level code such as `Condvar` and
compatibility tests. Its top-level helpers `sync::try_lock`, `sync::lock`,
`sync::unlock`, and method helpers `try_lock_bool` and `lock_raw` expose the
older bool/void lock shape and require a matching explicit `unlock`.
`RawMutex::is_locked` is a diagnostic predicate, not a proof that a later
operation will observe the same state.

Ari's current `Mutex[T]` and `RawMutex` are not poisoned. If code panics or
otherwise fails while holding a lock, later lock acquisition does not report
that event. Consistency of shared state after failure is the caller's
responsibility. A future `PoisonMutex`/`PoisonRwLock` can add opt-in poisoning
without changing this default no-poison policy.

```ari
var counter = std::sync::Mutex::new<i64>(0);
{
  var guard = counter.lock();
  guard.set(guard.value() + 1);
  drop guard;
}
assert(counter.into_inner() == 1);
```

## RwLock API

```ari
RwLock::new<T>(value) -> RwLock[T]

lock.try_read() -> Option[RwLockReadGuard[T]]
lock.read() -> RwLockReadGuard[T]
lock.try_write() -> Option[RwLockWriteGuard[T]]
lock.write() -> RwLockWriteGuard[T]
lock.get_mut() -> ref mut T
lock.into_inner() -> T
lock.reader_count() -> i64
lock.is_read_locked() -> bool
lock.is_write_locked() -> bool
lock.is_locked() -> bool

read_guard.value() -> T
read_guard.value_ref() -> ref T
read_guard.unlock()
read_guard.is_active()

write_guard.value() -> T
write_guard.value_ref() -> ref T
write_guard.value_mut() -> ref mut T
write_guard.set(value) -> void
write_guard.replace(value) -> T
write_guard.unlock()
write_guard.is_active()

RawRwLock::new() -> RawRwLock

sync::try_read_lock(ref mut lock) -> bool
sync::read_lock(ref mut lock) -> void
sync::read_unlock(ref mut lock) -> void
sync::try_write_lock(ref mut lock) -> bool
sync::write_lock(ref mut lock) -> void
sync::write_unlock(ref mut lock) -> void
sync::reader_count(ref lock) -> i64
sync::is_read_locked(ref lock) -> bool
sync::is_write_locked(ref lock) -> bool

raw_lock.try_read_lock()
raw_lock.read_lock()
raw_lock.read_unlock()
raw_lock.try_write_lock()
raw_lock.write_lock()
raw_lock.write_unlock()
raw_lock.reader_count()
raw_lock.is_read_locked()
raw_lock.is_write_locked()
raw_lock.is_locked()

raw_read_guard.unlock()
raw_read_guard.is_active()
raw_write_guard.unlock()
raw_write_guard.is_active()
```

`RwLock[T]` allows multiple readers or one writer around an owned payload.
Readers increment the raw state while no writer is present; writers use `-1` as
the exclusive state. The natural method API returns value guards: `read` and
`write` spin/yield until they can return active guards, and `try_read`/
`try_write` return `None` when acquisition would block. Read guards expose
`value` and `value_ref`, while write guards also expose `value_mut`, `set`, and
`replace`. `RwLockReadGuard[T]::unlock` and
`RwLockWriteGuard[T]::unlock` are idempotent, and explicit `drop guard`
releases an active guard.

`RawRwLock` is the manual reader-writer primitive. The `*_lock` and
`*_unlock` functions and methods remain compatibility APIs for low-level code.
Like the value lock, it is no-poison and spin/yield based; it does not add
fairness policy or futex-backed parking.

`RwLock[T]` and `RawRwLock` follow the same no-poison policy as `Mutex[T]`:
unlock after a panic or explicit failure does not mark the lock poisoned, and
later readers/writers do not receive a poison error.

```ari
var cached = std::sync::RwLock::new<i64>(40);
{
  var writer = cached.write();
  writer.replace(41);
  drop writer;
}
{
  var reader = cached.read();
  assert(reader.value() == 41);
  drop reader;
}
```

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
condvar.wait(ref mut raw_mutex) -> void
condvar.wait_timeout(ref mut raw_mutex, duration) -> Result[WaitTimeoutResult, Error]
condvar.wait_while(ref mut raw_mutex, condition: fn() -> bool) -> void
wait.timed_out() -> bool
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
`RawMutex` integration. `notify_one` is currently not selective; like
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
receiver.recv_timeout(duration) -> Result[T, RecvTimeoutError]
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
channel; `recv` yields until a value arrives or the channel closes.
`recv_timeout(duration)` checks for an already available value first, then
yields until a value arrives, the channel closes, or a monotonic
`std::time::Deadline` expires. It returns `RecvTimedOut` for an open empty
channel that reaches the deadline and `RecvTimeoutClosed` when the channel is
closed before a value is received. The `_optional` and `_bool` helpers
intentionally discard error detail for compatibility. The channel state is
allocated in the caller's `Zone`; `Sender`, `Receiver`, and `Channel` carry
only the shared state pointer, not a redundant zone handle.

`sender.clone()` creates another sender handle to the same single-slot channel
state. Cloned senders do not allocate and they do not carry separate ownership
counts yet; closing any sender closes the shared channel for all sender and
receiver handles.

Future channel work should add explicit `bounded_channel(capacity)`,
unbounded-channel policy if desired, sender-counted close semantics, blocking
wake integration, richer close semantics, and send/share trait checks.

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

  var mutex = Mutex::new<i64>(0);
  var guard = mutex.lock();
  if !mutex.is_locked() {
    return 3;
  }
  guard.unlock();

  var rwlock = RwLock::new<i64>(0);
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
- `Mutex[T]`, `RwLock[T]`, `RawMutex`, `RawRwLock`, `Condvar`, `Barrier`, and
  channels spin/yield; they do not park on futexes or condition-variable OS
  primitives.
- `Mutex[T]`, `RwLock[T]`, `RawMutex`, and `RawRwLock` do not poison after
  panic/failure. Shared-state
  consistency is caller-owned unless a future poison-aware type is introduced.
- Guard drops release active locks when callers use explicit `drop guard`;
  automatic RAII cleanup at scope exit or early return is not promised yet.
- There is no `LazyLock[T]`, semaphore, sender-counted close policy, or
  configurable channel capacity yet. Explicit `ThreadLocal[T]` handles live in
  `std::thread`; compiler-level
  `thread_local` declarations remain future work.
- Send/share trait checking is still roadmap work, so cross-thread value
  transfer APIs remain conservative.

## Synchronization Roadmap

| Feature | Status |
| --- | --- |
| Atomic types | Current `AtomicI64`, `AtomicBool`, `AtomicUsize`, and `AtomicPtr[T]`; future target-native width policy and more integer widths. |
| Memory ordering | Current explicit `Ordering` vocabulary with LLVM hosted lowering for load/store/RMW/compare-exchange; future examples and non-LLVM backend policy. |
| Mutex/RwLock | Current value-protecting no-poison `Mutex[T]`/`RwLock[T]` guards plus manual `RawMutex`/`RawRwLock` primitives for low-level code; future optional poison-aware lock types, fairness, and futex-backed parking. |
| Condvar | Current generation-based source API with spin/yield timeout waits; future blocking wait/wake and spurious wake documentation. |
| Once/OnceLock | Current source one-time execution, value slot, value-preserving `set`, and fallible initializer status; future ref-in-Result return ergonomics, panic policy, and optional `LazyLock`. |
| Barrier | Current source reusable barrier; future parking implementation. |
| MPSC channel | Current single-slot MPSC shape with Result errors, timeout receives, unsent-value return, and clonable sender handles; future configurable bounded queues, sender-counted close semantics, blocking wake, and richer close semantics. |
| Thread local | Current explicit `std::thread::ThreadLocal[T]` handles; future compiler-level static TLS declarations and destructor policy. |

## Tests

- `tests/cases/standard-library/ok/sync/std-sync-atomic-i64.ari` checks
  module functions, method wrappers, root `AtomicI64`, compare-exchange success
  and failure paths, and LLVM atomic lowering.
- `tests/cases/standard-library/ok/sync/std-sync-mutex-once.ari` checks source
  `RawMutex` lock/unlock helpers, guard acquisition, explicit guard unlock/drop,
  source `Once` transitions, method wrappers, root aliases, and
  compare-exchange lowering through the implementation.
- `tests/cases/standard-library/ok/sync/std-sync-rwlock.ari` checks source
  `RawRwLock` read/write state transitions, read/write guard acquisition,
  explicit guard unlock/drop, method wrappers, root alias, reader counts, and
  compare-exchange/fetch-add lowering.
- `tests/cases/standard-library/ok/sync/std-sync-value-locks.ari` checks
  `Mutex[T]` and `RwLock[T]` construction, guard payload access,
  try-lock failure while a guard is active, explicit unlock idempotence, and
  explicit drop release.
- `tests/cases/standard-library/ok/sync/std-sync-concurrency-api.ari` checks
  explicit order validation, order-specific LLVM lowering, `AtomicBool`,
  `AtomicUsize`, `AtomicPtr[T]`, `OnceLock`, `Condvar`, `Barrier`, and the
  single-slot channel API.

For small sync edits, run `make check-std-api`, then compile the relevant test
with `--emit-llvm` and run the resulting executable. Use the broader prelude
target when the change touches shared `std` loading, root aliases, or runtime
hooks.
