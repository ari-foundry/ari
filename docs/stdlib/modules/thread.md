# std::thread

`std::thread` is Ari's hosted thread module. It can start a plain function
pointer on the current LLVM/Linux runtime path, join the returned handle, yield
or sleep the current OS thread, read Ari's runtime thread id, query hosted
parallelism, use a small `Builder` value for thread options, and keep
per-thread values in an explicit `ThreadLocal[T]` handle.

The API is deliberately explicit. Thread entries are still `fn() -> i64`, not
capturing closures, because cross-thread ownership transfer needs send/share
rules before Ari can make that surface safe. Shared-state primitives live in
`std::sync`.

## API

```ari
thread::spawn(entry: fn() -> i64) -> Thread
thread::join(thread: Thread) -> i64
thread::is_finished(thread: Thread) -> bool
thread::yield_now() -> void
thread::sleep(duration: std::time::Duration) -> void
thread::id() -> i64
thread::is_main() -> bool
thread::available_parallelism() -> i64
thread::is_join_error(status: i64) -> bool
thread::builder() -> Builder
thread::spawn_configured(entry: fn() -> i64, name: string, stack_size: i64) -> Thread
thread::thread_local<T>(ref mut Zone) -> ThreadLocal[T]
thread::thread_local_with_capacity<T>(ref mut Zone, capacity: i64) -> ThreadLocal[T]

Thread::spawn(entry: fn() -> i64) -> Thread
Thread::invalid() -> Thread
thread.id() -> i64
thread.is_valid() -> bool
thread.is_finished() -> bool
thread.join() -> i64

Builder::new() -> Builder
builder.name(value: string) -> Builder
builder.stack_size(bytes: i64) -> Builder
builder.configured_name() -> string
builder.configured_stack_size() -> i64
builder.spawn(entry: fn() -> i64) -> Thread

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

`spawn(entry)` starts a new OS thread on the hosted runtime path. The spawned
thread installs a positive Ari runtime id before it enters source code. The main
thread id is `0`. `thread::id()` reads the current id through
`std::context::thread_id()`, and `thread::is_main()` is the readable predicate.

`Thread` is a visible value handle with the native thread handle and the Ari
runtime id. Join a native handle at most once. `join(thread)` waits for the
entry result and returns `-1` on join failure or for an invalid handle; use
`is_join_error(status)` for that sentinel. A successful worker may also return
`-1`, so typed thread results remain future work.

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

`yield_now()` asks the host scheduler to let another runnable thread make
progress. It is only a hint and does not create a synchronization boundary.
`sleep(duration)` delegates to `std::time::sleep`. `available_parallelism()`
returns the number of online processors reported by the host and clamps
failures to `1`.

`ThreadLocal[T]` is the first user-facing thread-local storage shape. It is an
explicit zone-backed handle rather than a global `thread_local` declaration.
The handle owns a small table of `(thread id, Option[T])` slots protected by a
source `Mutex`. `get`, `get_mut`, `take`, and `remove` operate on the current
thread id. `thread::thread_local<T>(zone)` uses the default capacity; use
`thread_local_with_capacity` or `ThreadLocal::with_capacity` when a shared
handle must serve a known number of threads. `get_or_init` returns
`OptionRef[T]` today because Ari's borrow-return rules do not yet let this
helper expose a direct `ref T` from an indexed heap slot.

## Example

```ari
fn worker() -> i64 {
  if thread::is_main() {
    return -1;
  }
  thread::yield_now();
  thread::sleep(time::Duration::zero());
  return thread::id();
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

  if !handle.is_valid() {
    return 1;
  }
  if handle.is_finished() {
    // Advisory only. Always join to collect the result.
  }

  let child_id = handle.id();
  let result = handle.join();
  if thread::is_join_error(result) {
    return 2;
  }
  if result != child_id {
    return 3;
  }

  return 0;
}
```

## Current Limits

- Thread entry is a plain `fn() -> i64`, not a closure with captured state.
- `Thread` is a value handle. Do not join the same copied native handle more
  than once.
- Join failure is represented by `-1` until Ari has a richer thread result or
  status type.
- `is_finished` is advisory and backend-specific. It should be used for status
  checks, not resource reclamation.
- `Builder` options are implemented on the LLVM/Linux pthread backend. Other
  backends need equivalent thread-attribute hooks before they can promise the
  same behavior.
- `ThreadLocal[T]` is explicit handle storage. Ari still needs declaration
  sugar for `thread_local` statics, destructor policy for compiler-owned TLS,
  and captured thread entries before cross-thread shared handles become
  ergonomic.
- Cross-platform thread policy still needs non-pthread implementations.

## Tests

- `tests/cases/standard-library/ok/thread/std-thread-basic.ari` checks
  main-thread identity, function-pointer spawn, child thread ids, join result
  propagation, invalid-handle sentinels, method wrappers, root `Thread`, and
  scheduler yield lowering.
- `tests/cases/standard-library/ok/thread/std-thread-runtime-helpers.ari`
  checks `available_parallelism`, `thread::sleep`, child thread use of both
  helpers, and runtime hooks they lower through.
- `tests/cases/standard-library/ok/thread/std-thread-builder.ari` checks the
  `Builder` method surface, configured pthread spawn hook, stack-size lowering,
  thread-name hook, and the advisory `is_finished` hook.
- `tests/cases/standard-library/ok/thread/std-thread-local.ari` checks
  explicit `ThreadLocal[T]` construction, current-thread get/set/mutable
  access, lazy initialization, removal, root alias coverage, and zone-backed
  provenance.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused source/runtime coverage.
