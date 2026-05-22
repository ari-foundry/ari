# std::thread

`std::thread` is Ari's hosted thread module. It can start a plain function
pointer on the current LLVM/Linux runtime path, join the returned handle, yield
or sleep the current OS thread, read Ari's runtime thread id, query hosted
parallelism, and use a small `Builder` value for future thread options.

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

`Builder` records a requested name and stack size in a plain value. The current
`Builder::spawn` delegates to `thread::spawn`; runtime enforcement of names and
custom stack sizes is reserved for the platform-specific thread-attribute
slice. Keeping the builder shape now lets user code and docs settle on natural
method names without pretending that every backend option is already wired.

`yield_now()` asks the host scheduler to let another runnable thread make
progress. It is only a hint and does not create a synchronization boundary.
`sleep(duration)` delegates to `std::time::sleep`. `available_parallelism()`
returns the number of online processors reported by the host and clamps
failures to `1`.

Ari already uses thread-local runtime storage internally for the current thread
id. General user-facing `ThreadLocal[T]` or `thread_local` storage is not
exposed yet because it needs generic static storage, destructor policy, and
ownership-transfer rules.

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

fn main() -> i64 {
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
- `Builder` records name and stack-size options, but the current runtime does
  not apply those options to `pthread_create` yet.
- User-facing thread-local storage remains roadmap work.
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
  `Builder` method surface and the advisory `is_finished` hook.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused source/runtime coverage.
