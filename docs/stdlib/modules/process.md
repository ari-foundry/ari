# std::process

`std::process` is the user-facing module for current-process operations and
small child-process control. The implemented surface is still deliberately
thin: read the current process id, exit with an explicit status code, use
natural status helper names in source Ari, and on the current Linux/LLVM path
fork and wait for a child process.

## Current API

```ari
process::id()
process::exit(code)
process::success()
process::failure()
process::is_success(code)
process::is_failure(code)
process::fork()
process::wait(pid)
process::is_child(pid)
process::is_parent(pid)
process::is_fork_error(pid)
process::is_wait_error(status)
```

`id()` returns the host process id as `i64`. On the current Linux/LLVM path it
lowers through a runtime hook backed by `getpid`.

`exit(code)` terminates the current process with the low bits of `code` as the
host exit status. It does not return.

`success()` returns `0`, `failure()` returns `1`, and the predicate helpers
make code that checks status values read clearly without hard-coding every
comparison at the call site.

`fork()` creates a child process on the current POSIX runtime path. The return
value follows the host convention: `0` in the child, a positive child pid in the
parent, or a negative value on failure. Use `is_child`, `is_parent`, and
`is_fork_error` instead of spelling those comparisons at every call site.

`wait(pid)` waits for a child process and returns the child's normal exit
status as `i64`. It returns `-1` when `waitpid` fails or when the child did not
exit normally. Use `is_wait_error(status)` to make that sentinel explicit.

## Example

```ari
fn main() -> i64 {
  let pid = process::id();
  if pid <= 0 {
    return process::failure();
  }

  return process::success();
}
```

Explicit termination:

```ari
fn main() -> i64 {
  process::exit(42);
  return 0;
}
```

Fork and wait:

```ari
fn main() -> i64 {
  let child = process::fork();
  if process::is_fork_error(child) {
    return process::failure();
  }

  if process::is_child(child) {
    process::exit(37);
    return 1;
  }

  let status = process::wait(child);
  if process::is_wait_error(status) {
    return process::failure();
  }
  return status;
}
```

## Current Limits

- `fork` and `wait` are the first POSIX child-process slice. Portable
  `spawn`, rich status values, and process handles are future work.
- Exit runs through the host process immediately. Do not expect Ari destructors
  or zone cleanup to run after `process::exit`.
- `wait(pid)` currently decodes only normal child exit statuses and returns
  `-1` for wait failures, signaled children, and other non-normal states.
- The API is intentionally not a raw syscall grab bag. Keep future process
  work behind `std::process` or a future `std::os` submodule in small safe
  slices with clear ownership and platform policy.

## Tests

Focused positive coverage:

```text
tests/cases/standard-library/ok/process/std-process-basic.ari
tests/cases/standard-library/ok/process/std-process-exit.ari
tests/cases/standard-library/ok/process/std-process-fork-wait.ari
```

`make check-prelude` emits LLVM, checks the runtime hook symbols, and executes
the programs. Public declarations are tracked in `tests/std_api_manifest.txt`
and checked by `make check-std-api`.
