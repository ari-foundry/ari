# std::process

`std::process` is the user-facing module for current-process operations and
small child-process control. The implemented surface is still deliberately
thin: read process identity values, terminate explicitly with `exit` or
`abort`, use natural status helper names in source Ari, and on the current
Linux/LLVM path fork and wait for a child process.

## Current API

```ari
process::id()
process::uid()
process::gid()
process::exit(code)
process::abort()
process::success()
process::failure()
process::is_success(code)
process::is_failure(code)
process::is_root()
process::fork_result()
process::fork()
process::wait_result(pid)
process::wait(pid)
process::is_child(pid)
process::is_parent(pid)
process::is_fork_error(pid)
process::is_wait_error(status)
```

`id()` returns the host process id as `i64`. `uid()` and `gid()` return the
current user id and group id as non-negative `i64` values. On the current
Linux/LLVM path these lower through runtime hooks backed by `getpid`, `getuid`,
and `getgid`.

`exit(code)` terminates the current process with the low bits of `code` as the
host exit status. It does not return.

`abort()` terminates the current process through the host abort path. It is for
immediate abnormal termination and should not be used for ordinary error
reporting.

`success()` returns `0`, `failure()` returns `1`, and the predicate helpers
make code that checks status values read clearly without hard-coding every
comparison at the call site.

`is_root()` returns whether `uid()` is `0`. It is a convenience predicate, not a
permission guarantee; filesystem and process permissions can still change after
the check.

`fork_result()` creates a child process on the current POSIX runtime path and
returns `Result[i64, Error]`: `Ok(0)` in the child, `Ok(child_pid)` in the
parent, or `Err(std::c::error())` when the host `fork` fails. `fork()` keeps the
older raw host convention: `0` in the child, a positive child pid in the
parent, or a negative value on failure. Use `is_child`, `is_parent`, and
`is_fork_error` instead of spelling those comparisons at every call site.

`wait_result(pid)` waits for a child process and returns `Ok(exit_status)` when
the child exited normally. Host `waitpid` failures become `Err(std::c::error())`.
If the child is observed in a non-normal state, the current source wrapper
returns `Err(Error(Other))` until richer process status values exist.
`wait(pid)` is the older raw compatibility helper; it returns the child's normal
exit status or `-1`. Use `is_wait_error(status)` to make that sentinel explicit.

## Example

```ari
fn main() -> i64 {
  let pid = process::id();
  if pid <= 0 {
    return process::failure();
  }
  if process::is_root() {
    return process::success();
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

Abnormal termination:

```ari
fn stop_now() -> void {
  process::abort();
}
```

Fork and wait:

```ari
fn main() -> i64 {
  match process::fork_result() {
    Err(_) => {
      return process::failure();
    }
    Ok(child) => {
      if process::is_child(child) {
        process::exit(37);
        return 1;
      }

      match process::wait_result(child) {
        Ok(status) => { return status; }
        Err(_) => { return process::failure(); }
      }
    }
  }
}
```

## Current Limits

- `uid`, `gid`, `fork`, and `wait` are POSIX-flavored hosted runtime hooks.
  Portable `spawn`, `exec`, `kill`, rich status values, and process handles are
  future work.
- Exit runs through the host process immediately. Do not expect Ari destructors
  or zone cleanup to run after `process::exit`.
- Abort also terminates immediately through the host runtime and should be
  treated as noreturn.
- `wait(pid)` currently decodes only normal child exit statuses and returns
  `-1` for wait failures, signaled children, and other non-normal states.
  `wait_result(pid)` preserves `Error` payloads for host `waitpid` failures,
  but still reports non-normal child states as `Error(Other)` until the module
  has a richer `ExitStatus`/signal model.
- The API is intentionally not a raw syscall grab bag. Keep future process
  work behind `std::process` or a future `std::os` submodule in small safe
  slices with clear ownership and platform policy.

## Tests

Focused positive coverage:

```text
tests/cases/standard-library/ok/process/std-process-basic.ari
tests/cases/standard-library/ok/process/std-process-identity.ari
tests/cases/standard-library/ok/process/std-process-exit.ari
tests/cases/standard-library/ok/process/std-process-abort.ari
tests/cases/standard-library/ok/process/std-process-fork-wait.ari
tests/cases/standard-library/ok/process/std-process-result.ari
```

`make check-prelude` emits LLVM, checks the runtime hook symbols, and executes
the programs. The abort fixture compiles and runs only a non-aborting path while
checking that the abort hook lowers to the host `abort` declaration. Public
declarations are tracked in `tests/std_api_manifest.txt` and checked by
`make check-std-api`.
