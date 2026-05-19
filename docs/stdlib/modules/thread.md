# std::thread

`std::thread` is the first Ari thread module. It starts deliberately small:
spawn a plain function pointer, join the returned handle, ask for the current
Ari runtime thread id, sleep/yield the current OS thread, and ask the runtime
how much parallelism is available.

The API is thin because thread ownership needs a strong foundation. Capturing
closures, shared ownership, locks, and typed result handles should grow in
later `std::sync` and richer `std::thread` slices instead of being hidden
behind a too-broad first wrapper. The first atomic primitive now lives in
`std::sync`, but thread entry capture and shared-state policy are still future
work.

## API

```ari
thread::spawn(entry: fn() -> i64) -> Thread
thread::join(thread: Thread) -> i64
thread::yield_now() -> void
thread::sleep(duration: std::time::Duration) -> void
thread::id() -> i64
thread::is_main() -> bool
thread::available_parallelism() -> i64
thread::is_join_error(status: i64) -> bool

Thread::spawn(entry: fn() -> i64) -> Thread
Thread::invalid() -> Thread
thread.id() -> i64
thread.is_valid() -> bool
thread.join() -> i64
```

`spawn(entry)` starts a new OS thread on the current LLVM/Linux runtime path.
The entry function must have the shape `fn() -> i64`. This is intentional:
captured values and ownership transfer need explicit send/share rules before
the standard library promises closure-like thread entry.

`Thread` is a visible value handle with two pieces of runtime state: the native
thread handle and the Ari runtime id assigned to the spawned thread. The main
thread id is `0`; spawned Ari threads receive positive ids before they enter
source code. `thread::id()` reads the current runtime id through
`std::context::thread_id()`, and `thread::is_main()` is the readable predicate
for the main-thread check.

`join(thread)` waits for a thread and returns the `i64` value returned by the
entry function. It returns `-1` on join failure or for an invalid handle; use
`is_join_error(status)` for that sentinel. A successful thread may also return
`-1`, so a richer result type is future work.

`yield_now()` asks the host scheduler to let another runnable thread make
progress. It is only a hint; it does not create a synchronization boundary.
`sleep(duration)` delegates to `std::time::sleep` so users can write
thread-oriented code without reaching into the time module at every call site.
`available_parallelism()` returns the number of online processors reported by
the hosted runtime and clamps host failures to `1`.

Ari already uses thread-local runtime storage internally for the current
thread id. General user-facing `ThreadLocal[T]` storage is not exposed yet
because it needs generic static storage, destructor, and ownership-transfer
policy.

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
  let handle = thread::spawn(worker);
  if !handle.is_valid() {
    return 1;
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
- The first handle is a value shape. Do not join the same copied native handle
  more than once.
- Join failure is represented by `-1` until Ari has a richer thread result or
  status type.
- There is no captured shared-state API yet. `std::sync::AtomicI64`,
  primitive `Mutex`, and `Once` exist, but value-protecting locks, `Shared`,
  channels, and send/share rules remain future `std::sync` work.
- There is no user-facing thread-local storage API yet. The runtime TLS slot
  currently stores only Ari's own thread id.
- Stack size is not configurable yet. A future `Builder` or
  `spawn_with_options` API should own custom stack size, names, and platform
  error details together.
- The current backend implementation uses pthreads on the LLVM/Linux path.
  Cross-platform thread policy remains roadmap work.

## Tests

- `tests/cases/standard-library/ok/thread/std-thread-basic.ari` checks
  main-thread identity, function-pointer spawn, child thread ids, join result
  propagation, invalid-handle sentinels, method wrappers, root `Thread`, and
  scheduler yield lowering.
- `tests/cases/standard-library/ok/thread/std-thread-runtime-helpers.ari`
  checks `available_parallelism`, `thread::sleep`, child thread use of both
  helpers, and the runtime hooks they lower through.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused source/runtime coverage.
