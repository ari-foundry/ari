# std::process

`std::process` is the user-facing module for current-process operations and
child-process control. The implemented surface reads process identity values,
terminates explicitly with `exit` or `abort`, uses natural status helper names
in source Ari, and on the current Linux/LLVM path can fork/wait directly or use
the first `Command` builder for `spawn`, `status`, `output_in`, `exec`, `kill`,
environment setup, working-directory setup, typed `ExitStatus` inspection, and
small stdout/stderr capture.

## Current API

```ari
process::Error
process::ErrorKind
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
process::wait_status_result(pid)
process::wait_result(pid)
process::wait(pid)
process::is_child(pid)
process::is_parent(pid)
process::is_fork_error(pid)
process::is_wait_error(status)
process::arg(value)
process::env_var(name, value)
process::command(program)
process::command_with_args(program, args)
process::spawn(command)
process::status(command)
process::exit_status(command)
process::output_in(command, zone)
process::exec(command)
process::kill(pid, signal)
process::terminate(pid)

process::Command::new(program)
process::Command::with_args(program, args)
Command::args(args)
Command::env(env_values)
Command::current_dir(path)
Command::spawn()
Command::status()
Command::exit_status()
Command::output_in(zone)
Command::exec()

ExitStatus::raw()
ExitStatus::exited()
ExitStatus::signaled()
ExitStatus::code()
ExitStatus::code_or(fallback)
ExitStatus::signal()
ExitStatus::signal_or(fallback)
ExitStatus::is_success()
ExitStatus::is_failure()

Output::status()
Output::exit_status()
Output::is_success()
Output::stdout()
Output::stderr()

Child::pid()
Child::wait()
Child::wait_status()
Child::kill(signal)
Child::terminate()
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

`wait_status_result(pid)` waits for a child process and returns
`Result[ExitStatus, Error]`. Host `waitpid` failures become
`Err(std::c::error())`; normal exit and signal termination are represented in
the returned `ExitStatus`. Use `ExitStatus::code()` for an optional normal exit
code and `ExitStatus::signal()` for an optional terminating signal number.

`wait_result(pid)` keeps the older convenient `Result[i64, Error]` shape: it
returns `Ok(exit_code)` only for a normal child exit and reports non-normal child
states as `Error(Other)`. `wait(pid)` is the raw compatibility helper; it returns
the child's normal exit status or `-1`. Use `is_wait_error(status)` to make that
sentinel explicit.

`Command` is the first higher-level process builder. It stores the program,
argument slice, child environment assignments, and child working directory.
`status()` spawns the command, waits for it, and returns `Result[i64, Error]`
for compatibility with earlier tests. New code should prefer `exit_status()`
when signal termination matters. `spawn()` returns a `Child` handle with `pid`,
`wait`, `wait_status`, `kill`, and `terminate` methods. `exec()` applies the
setup and replaces the current process with the program; if `execvp` returns,
Ari reports the host error.

The module-level wrappers `process::spawn(ref command)`,
`process::status(ref command)`, `process::exit_status(ref command)`,
`process::output_in(ref command, ref mut zone)`, and
`process::exec(ref command)` call the matching `Command` methods. They exist so
code can use either fluent builder style or the direct `process::spawn(cmd)`
shape common in other standard libraries without losing `Error` detail.

`output_in(zone)` spawns the command with stdout and stderr redirected to
temporary pipes, waits for the child, reads both streams into `Vec[u8]` values
allocated in `zone`, and returns `Result[Output, Error]`. `Output::exit_status()`
returns the typed child status, `Output::status()` returns the normal exit code
or `-1` for compatibility, `Output::is_success()` checks the typed status, and
`Output::stdout()` / `Output::stderr()` expose borrowed `Slice[u8]` views over
the captured bytes. The `_in` suffix is intentional: captured output owns
buffers, so the caller chooses the allocation zone.

Arguments use `process::arg("...")` rather than raw `string` slices so the
builder can keep an executable-friendly C argv representation. Environment
entries use `process::env_var(name, value)`. The ergonomic rule is: call sites
still look like command construction, while the stdlib owns the pointer-level
ABI shape.

`kill(pid, signal)` sends a POSIX signal and returns `Result[(), Error]`.
`terminate(pid)` sends signal `15` (`SIGTERM`) as the conventional graceful
termination request.

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

Command status:

```ari
fn main() -> i64 {
  var args = [process::arg("-c"), process::arg("exit 17")];
  var cmd = process::Command::with_args("sh", args.as_slice());

  match cmd.status() {
    Ok(status) => { return status; }
    Err(_) => { return process::failure(); }
  }
}
```

Command setup and child handle:

```ari
fn main() -> i64 {
  var args = [process::arg("-c"), process::arg("test \"$ARI_MODE\" = test")];
  var env = [process::env_var("ARI_MODE", "test")];
  var cmd = process::command_with_args("sh", args.as_slice());
  cmd.env(env.as_slice());
  cmd.current_dir("build/prelude");

  match cmd.spawn() {
    Err(_) => { return process::failure(); }
    Ok(child) => {
      if child.pid() <= 0 {
        return process::failure();
      }
      return child.wait().unwrap();
    }
  }
}
```

Command output capture:

```ari
fn main() -> i64 {
  var zone = zone::temp(512);
  var args = [process::arg("-c"), process::arg("printf 'ok'")];
  var cmd = process::command_with_args("sh", args.as_slice());

  match cmd.output_in(ref mut zone) {
    Err(_) => { return process::failure(); }
    Ok(value) => {
      var output = value;
      if output.status() != process::success() {
        return process::failure();
      }
      return output.stdout().len;
    }
  }
}
```

Typed exit status:

```ari
fn main() -> i64 {
  var args = [process::arg("-c"), process::arg("kill -TERM $$")];
  var cmd = process::command_with_args("sh", args.as_slice());

  match cmd.exit_status() {
    Err(_) => { return process::failure(); }
    Ok(status) => {
      if status.signaled() && status.signal_or(0) == 15 {
        return process::success();
      }
      return process::failure();
    }
  }
}
```

## Current Limits

- `uid`, `gid`, `fork`, `wait`, `Command`, and `kill` are POSIX-flavored hosted
  runtime hooks today. The API shape is intended to stay portable, but Windows
  mapping still needs a separate implementation.
- `Command` currently supports `spawn`, `status`, `exit_status`, `output_in`,
  `exec`, arguments, environment assignments, working-directory setup, captured
  stdout and stderr, typed status inspection, and `Child` wait/kill helpers.
  `output_in` is intended for small captured outputs today: large concurrent
  stdout/stderr streams need future readiness/nonblocking draining to avoid
  pipe-buffer backpressure. Explicit stdin redirection, inherited/cleared
  environment policies, and portable platform-specific status detail are future
  work.
- Exit runs through the host process immediately. Do not expect Ari destructors
  or zone cleanup to run after `process::exit`.
- Abort also terminates immediately through the host runtime and should be
  treated as noreturn.
- `wait(pid)` currently decodes only normal child exit statuses and returns
  `-1` for wait failures, signaled children, and other non-normal states.
  Prefer `wait_status_result(pid)` or `Child::wait_status()` when signal
  termination matters.
- `exec()` never returns on success. Use it only when replacing the current
  process is the desired behavior.
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
tests/cases/standard-library/ok/process/std-process-exit-status.ari
tests/cases/standard-library/ok/process/std-process-command.ari
tests/cases/standard-library/ok/process/std-process-output.ari
```

`make check-prelude` emits LLVM, checks the runtime hook symbols, and executes
the programs. The abort fixture compiles and runs only a non-aborting path while
checking that the abort hook lowers to the host `abort` declaration. The command
fixture covers argument passing, environment setup, working-directory setup,
method and module-level `status`/`spawn`, module-level `exit_status`,
`Child::wait`, and non-destructive `kill(0)`. The typed-status fixture covers
`ExitStatus`, `Command::exit_status`, `Child::wait_status`, normal exit codes,
and signal termination. The output fixture covers method and module-level small
stdout/stderr capture, exit status accessors, and missing command status `127`.
Public declarations are tracked in
`tests/std_api_manifest.txt` and checked by `make check-std-api`.
