# std::thread

`std::thread` is Ari's hosted thread module. It can start a plain function
pointer on the current LLVM/Linux runtime path, join or detach the returned
owning handle, yield or sleep the current OS thread, read Ari's runtime thread
id, query hosted parallelism, use a small `Builder` value for thread options,
and keep per-thread values in an explicit `ThreadLocal[T]` handle.

The API is deliberately explicit. Thread entries are still `fn() -> i64`, not
capturing closures, because cross-thread ownership transfer needs send/share
rules before Ari can make that surface safe. Shared-state primitives live in
`std::sync`.

## API

```ari
thread::Error
thread::ErrorKind
thread::ThreadId
thread::JoinHandle
thread::JoinError
thread::spawn(entry: fn() -> i64) -> Result[JoinHandle, thread::Error]
thread::spawn_unchecked(entry: fn() -> i64) -> Thread
thread::join(ref mut JoinHandle) -> Result[i64, JoinError]
thread::join_thread(thread: Thread) -> Result[i64, JoinError]
thread::join_compat(thread: Thread) -> Result[i64, thread::Error]
thread::join_unchecked(thread: Thread) -> i64
thread::is_finished(thread: Thread) -> bool
thread::yield_now() -> void
thread::sleep(duration: std::time::Duration) -> void
thread::id() -> ThreadId
thread::id_raw() -> i64
thread::current() -> Thread
thread::is_main() -> bool
thread::available_parallelism() -> Result[u64, thread::Error]
thread::available_parallelism_or(default: u64) -> u64
thread::available_parallelism_raw() -> i64
thread::is_join_error(status: i64) -> bool
thread::builder() -> Builder
thread::spawn_configured(entry: fn() -> i64, name: string, stack_size: i64) -> Result[JoinHandle, thread::Error]
thread::spawn_configured_unchecked(entry: fn() -> i64, name: string, stack_size: i64) -> Thread
thread::thread_local<T>(ref mut Zone) -> ThreadLocal[T]
thread::thread_local_with_capacity<T>(ref mut Zone, capacity: i64) -> ThreadLocal[T]

ThreadId::current() -> ThreadId
ThreadId::from_raw(raw: i64) -> ThreadId
thread_id.as_i64() -> i64
thread_id.is_main() -> bool
thread_id.is_valid() -> bool
thread_id.equals(other: ThreadId) -> bool

Thread::spawn(entry: fn() -> i64) -> Result[JoinHandle, thread::Error]
Thread::spawn_unchecked(entry: fn() -> i64) -> Thread
Thread::current() -> Thread
Thread::invalid() -> Thread
thread.id() -> ThreadId
thread.id_raw() -> i64
thread.is_valid() -> bool
thread.is_joinable() -> bool
thread.is_finished() -> bool
thread.join() -> Result[i64, JoinError]
thread.join_unchecked() -> i64

JoinHandle::invalid() -> JoinHandle
JoinHandle::from_thread(thread: Thread) -> JoinHandle
handle.thread() -> Thread
handle.thread_id() -> ThreadId
handle.id() -> ThreadId
handle.is_valid() -> bool
handle.is_finished() -> bool
handle.detach() -> Result[(), JoinError]
handle.join() -> Result[i64, JoinError]

Builder::new() -> Builder
builder.name(value: string) -> Builder
builder.stack_size(bytes: i64) -> Builder
builder.configured_name() -> string
builder.configured_stack_size() -> i64
builder.spawn(entry: fn() -> i64) -> Result[JoinHandle, thread::Error]
builder.spawn_unchecked(entry: fn() -> i64) -> Thread

ThreadLocal::new<T>(ref mut Zone) -> ThreadLocal[T]
ThreadLocal::with_capacity<T>(ref mut Zone, capacity: i64) -> ThreadLocal[T]
thread_local.capacity() -> i64
thread_local.is_initialized() -> bool
thread_local.set(value: T) -> Option[T]
thread_local.get() -> OptionRef[T]
thread_local.get_mut() -> OptionMut[T]
thread_local.get_or_init(op: fn() -> T) -> OptionRef[T]
thread_local.take() -> Option[T]
thread_local.remove() -> Option[T]
```

`spawn(entry)` starts a new OS thread on the hosted runtime path and returns a
`JoinHandle`. The spawned thread installs a positive Ari runtime id before it
enters source code. The main thread id is `0`. `thread::id()` returns a
`ThreadId`; use `as_i64()` or the raw compatibility `thread::id_raw()` when an
integer is needed. `thread::current()` returns non-owning information for the
current thread and is not joinable.

`JoinHandle` owns the right to join or detach the native thread. Join a handle
at most once. `join(ref mut handle)` and `handle.join()` are the natural
recoverable forms; they return `JoinError::InvalidHandle`,
`JoinError::AlreadyJoined`, `JoinError::Detached`, or
`JoinError::JoinFailed` for lifecycle and host join failures. `detach()` marks
the handle detached and consumes the join right. Dropping a live `JoinHandle`
does not currently detach automatically, so hosted programs should explicitly
join or detach handles they create.

`Thread` is the raw visible thread information value that carries the native
handle and Ari runtime id. It remains for compatibility and inspection.
`Thread::current()` is valid but not joinable. `join_thread(thread)` provides a
one-shot compatibility bridge from raw `Thread` to the new `JoinError` model;
`join_compat(thread)` maps invalid handles to `Error(InvalidInput)` and
negative join status to `Error(Other)`. `join_unchecked(thread)` and
`thread.join_unchecked()` keep the raw sentinel form and return the raw `i64`
status.

`is_finished(thread)` is an advisory completion predicate. On the current
pthread-backed LLVM host it treats invalid handles as finished and asks
`pthread_kill(handle, 0)` whether the native handle still exists. It does not
consume the handle and it is not a replacement for `join`.

`Builder` records a requested name and stack size in a plain value.
`Builder::spawn` delegates to `thread::spawn_configured`, which applies the
requested stack size through pthread attributes on the current LLVM/Linux host
backend when `stack_size > 0`. Non-empty names are passed to
`pthread_setname_np` after successful creation as a best-effort host hint; Linux
thread names are short and rejected names do not make spawn fail. Plain
`thread::spawn` remains the zero-option helper.
`spawn`, `spawn_configured`, `Thread::spawn`, and `Builder::spawn` turn invalid
returned handles into `Error(Other)`. `spawn_unchecked`,
`spawn_configured_unchecked`, `Thread::spawn_unchecked`, and
`Builder::spawn_unchecked` are compatibility forms that return invalid handles
instead of `Error`.

`yield_now()` asks the host scheduler to let another runnable thread make
progress. It is only a hint and does not create a synchronization boundary.
`sleep(duration)` delegates to `std::time::sleep`. `available_parallelism()`
is the natural Result-returning hosted processor count helper. Use
`available_parallelism_or(default)` when a fallback is preferred, or
`available_parallelism_raw()` only at compatibility/runtime boundaries.

`ThreadLocal[T]` is the first user-facing thread-local storage shape. It is an
explicit zone-backed handle rather than a global `thread_local` declaration.
The handle owns a small table of `(thread id, Option[T])` slots protected by a
source `Mutex`. `get`, `get_mut`, `take`, and `remove` operate on the current
thread id. `thread::thread_local<T>(zone)` uses the default capacity; use
`thread_local_with_capacity` or `ThreadLocal::with_capacity` when a shared
handle must serve a known number of threads. Capacity exhaustion is a
programmer error and asserts. The backing zone must outlive the handle.
`get_or_init` returns `OptionRef[T]` today because Ari's borrow-return rules do
not yet let this helper expose a direct `ref T` from an indexed heap slot.

## Example

```ari
fn worker() -> i64 {
  if thread::is_main() {
    return -1;
  }
  thread::yield_now();
  thread::sleep(time::Duration::zero());
  return thread::id().as_i64();
}

fn make_local_value() -> i64 {
  return 99;
}

fn main() -> i64 {
  var zone = zone::create(4096);
  var local = thread::thread_local_with_capacity<i64>(ref mut zone, 2);
  assert(local.set(10).is_none());
  let local_value = local.get();
  assert(local_value.unwrap() == 10);
  let initialized = local.get_or_init(make_local_value);
  assert(initialized.unwrap() == 10);
  drop local;
  zone::destroy(zone);

  let handle = thread::builder()
    .name("worker")
    .stack_size(0)
    .spawn(worker);

  if handle.is_err() {
    return 1;
  }
  var joined = handle.unwrap();
  let child_id = joined.id();
  let result = joined.join();
  if result.is_err() {
    return 2;
  }
  if result.unwrap() != child_id.as_i64() {
    return 3;
  }

  return 0;
}
```

## Current Limits

- Thread entry is a plain `fn() -> i64`, not a closure with captured state.
- `JoinHandle` owns a join right, but Ari values can still be copied in places
  that expose the raw `Thread`. Do not join the same native handle through a
  copied raw handle.
- Join failure is represented by `-1` in the compatibility API. Natural
  `JoinHandle` APIs report lifecycle problems through `JoinError` until Ari
  has a richer generic thread result or status type.
- `is_finished` is advisory and backend-specific. It should be used for status
  checks, not resource reclamation.
- `Builder` options are implemented on the LLVM/Linux pthread backend. Other
  backends need equivalent thread-attribute hooks before they can promise the
  same behavior.
- `ThreadLocal[T]` is explicit handle storage. Ari still needs declaration
  sugar for `thread_local` statics, destructor policy for compiler-owned TLS,
  and captured thread entries before cross-thread shared handles become
  ergonomic.
- `ThreadLocal[T]` is zone-backed and fixed-capacity. Capacity overflow is a
  programmer error in this stdlib slice, not a recoverable runtime error.
- Cross-platform thread policy still needs non-pthread implementations.

## Tests

- `tests/cases/standard-library/ok/thread/std-thread-basic.ari` checks
  main-thread identity, function-pointer spawn, child thread ids, join result
  propagation, invalid-handle sentinels, Result-returning spawn/join helpers,
  method wrappers, root `Thread`, and scheduler yield lowering.
- `tests/cases/standard-library/ok/thread/std-thread-runtime-helpers.ari`
  checks `available_parallelism`, `thread::sleep`, child thread use of both
  helpers, and runtime hooks they lower through.
- `tests/cases/standard-library/ok/thread/std-thread-builder.ari` checks the
  `Builder` method surface, configured pthread spawn hook, stack-size lowering,
  thread-name hook, Result-returning builder spawn, unchecked builder spawn
  compatibility, and the advisory `is_finished` hook.
- `tests/cases/standard-library/ok/thread/std-thread-local.ari` checks
  explicit `ThreadLocal[T]` construction, current-thread get/set/mutable
  access, lazy initialization, removal, root alias coverage, and zone-backed
  provenance.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused source/runtime coverage.
