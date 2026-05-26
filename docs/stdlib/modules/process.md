# std::process

`std::process` is the user-facing module for current-process operations and
child-process control. The implemented surface reads process identity values,
terminates explicitly with `exit` or `abort`, uses natural status helper names
in source Ari, and on the current Linux/LLVM path can fork/wait directly or use
the `Command` builder for `arg`, `args`, `env`, `env_values`, `env_var`,
`arg_bytes`, `env_bytes`, `current_dir`, `current_dir_path`,
by-value `with_arg`/`with_env`/`with_clear_env`/`with_inherit_env`/
`with_current_dir` chaining, explicit child environment inheritance or clearing
policy, explicit child stdin redirection from a file or `/dev/null` at execution
time, `spawn`,
`status`, `status_code`, `output`, `output_in`, `exec`, typed `ExitStatus` and
`ExitCode` inspection, typed signal helpers, small stdout/stderr capture,
child-stream endpoint aliases, current path wrappers, and temp file/temp dir
creation.

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
process::exit_code(value)
process::success_code()
process::failure_code()
process::is_success(code)
process::is_failure(code)
process::is_root()
process::fork() -> Result[i64, process::Error]
process::fork_raw() -> i64
process::wait_status(pid) -> Result[process::ExitStatus, process::Error]
process::wait(pid) -> Result[i64, process::Error]
process::wait_raw(pid) -> i64
process::is_child(pid)
process::is_parent(pid)
process::is_fork_error(pid)
process::is_wait_error(status)
process::signal(value)
process::sig_check()
process::sighup()
process::sigint()
process::sigquit()
process::sigkill()
process::sigterm()
process::arg(value)
process::arg_bytes(zone, value) -> Result[process::Arg, process::Error]
process::env_var(name, value)
process::env_var_bytes(zone, name, value) -> Result[process::EnvVar, process::Error]
process::command(program)
process::command_with_args(program, args)
process::spawn(command)
process::status(command) -> Result[process::ExitStatus, process::Error]
process::status_code(command) -> Result[i64, process::Error]
process::status_with_stdin(command, values) -> Result[process::ExitStatus, process::Error]
process::status_with_stdin_string(command, text) -> Result[process::ExitStatus, process::Error]
process::exit_status(command)
process::output_in(command, zone)
process::output(command, zone)
process::exec(command)
process::kill(pid, signal)
process::kill_signal(pid, signal)
process::terminate(pid)
process::current_dir() -> Result[string, process::Error]
process::current_dir_or_default()
process::current_dir_optional()
process::try_current_dir()
process::executable_path() -> Result[string, process::Error]
process::executable_path_or_default()
process::executable_path_optional()
process::try_executable_path()
process::temp_file(zone)
process::temp_file_in(zone, prefix)
process::temp_dir(zone)
process::temp_dir_in(zone, prefix)

process::ChildStdin
process::ChildStdout
process::ChildStderr

process::Command::new(program)
process::Command::with_args(program, args)
Command::arg(zone, value)
Command::arg_bytes(zone, value) -> Result[(), process::Error]
Command::arg_value(zone, value)
Command::args(args)
Command::clear_env()
Command::env(zone, name, value)
Command::env_bytes(zone, name, value) -> Result[(), process::Error]
Command::env_values(env_values)
Command::env_var(zone, name, value)
Command::env_value(zone, value)
Command::inherit_env()
Command::current_dir(path)
Command::current_dir_bytes(zone, path) -> Result[(), process::Error]
Command::current_dir_path(zone, path) -> Result[(), process::Error]
Command::with_arg(zone, value)
Command::with_clear_env()
Command::with_env(zone, name, value)
Command::with_inherit_env()
Command::with_current_dir(path)
Command::spawn()
Command::spawn_with_stdin_file(path) -> Result[process::Child, process::Error]
Command::spawn_with_stdin_null() -> Result[process::Child, process::Error]
Command::status() -> Result[process::ExitStatus, process::Error]
Command::status_with_stdin(values) -> Result[process::ExitStatus, process::Error]
Command::status_with_stdin_string(text) -> Result[process::ExitStatus, process::Error]
Command::status_with_stdin_file(path) -> Result[process::ExitStatus, process::Error]
Command::status_with_stdin_file_bytes(zone, path) -> Result[process::ExitStatus, process::Error]
Command::status_with_stdin_file_path(zone, path) -> Result[process::ExitStatus, process::Error]
Command::status_with_stdin_null() -> Result[process::ExitStatus, process::Error]
Command::status_code() -> Result[i64, process::Error]
Command::exit_status()
Command::output_in(zone)
Command::output(zone)
Command::exec()

ExitCode::raw()
ExitCode::code()
ExitCode::is_success()
ExitCode::is_failure()
ExitCode::exit()

ExitStatus::raw()
ExitStatus::exited()
ExitStatus::signaled()
ExitStatus::code()
ExitStatus::code_or(fallback)
ExitStatus::exit_code()
ExitStatus::signal()
ExitStatus::signal_or(fallback)
ExitStatus::is_success()
ExitStatus::success()
ExitStatus::is_failure()

Signal::raw()
Signal::is_check()

Output::status()
Output::exit_status()
Output::is_success()
Output::stdout()
Output::stdout_string(zone)
Output::stderr()
Output::stderr_string(zone)

TempFile::path()
TempFile::as_c_str()
TempFile::as_fd()
TempFile::is_open()
TempFile::close() -> Result[(), process::Error]
TempFile::close_bool() -> bool
TempFile::remove()
TempFile::close_and_remove()

TempDir::path()
TempDir::as_c_str()
TempDir::remove()

Child::pid()
Child::wait()
Child::wait_status()
Child::kill(signal)
Child::signal(signal)
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
comparison at the call site. `ExitCode` is the typed form for code values that
will be returned or passed to `exit`: use `success_code`, `failure_code`, or
`exit_code(value)` and then inspect `is_success`, `is_failure`, or `raw`.

`is_root()` returns whether `uid()` is `0`. It is a convenience predicate, not a
permission guarantee; filesystem and process permissions can still change after
the check.

`fork()` creates a child process on the current POSIX runtime path and returns
`Result[i64, Error]`: `Ok(0)` in the child, `Ok(child_pid)` in the parent, or
`Err(std::c::error())` when the host `fork` fails. `fork_raw()` keeps the older
raw host convention: `0` in the child, a positive child pid in the parent, or a
negative value on failure. Use `is_child`, `is_parent`, and `is_fork_error`
instead of spelling those comparisons at every call site.

`wait_status(pid)` waits for a child process and returns
`Result[ExitStatus, Error]`. Host `waitpid` failures become
`Err(std::c::error())`; normal exit and signal termination are represented in
the returned `ExitStatus`. Use `ExitStatus::code()` for an optional normal exit
code and `ExitStatus::signal()` for an optional terminating signal number.

`wait(pid)` returns `Ok(exit_code)` only for a normal child exit and reports
non-normal child states as `Error(Other)`. `wait_raw(pid)` is the raw
compatibility helper; it returns the child's normal exit status or `-1`. Use
`is_wait_error(status)` to make that sentinel explicit.

`Command` is the higher-level process builder. It stores the program, argument
slice, child environment assignments, and child working directory. `args`
replaces the current argv slice, and `env_values` replaces the child
environment assignment slice. `arg(zone, value)`, `env(zone, name, value)`, and
`env_var(zone, name, value)` append one item by allocating a new backing slice
in the caller-provided zone; this keeps the builder dynamic without storing a
redundant allocator handle in `Command`. The zone must outlive the call to
`spawn`, `status`, `status_code`, `exit_status`, `output`, or `exec`.

There are two ergonomic construction styles:

```ari
var cmd = process::Command::new("sh");
cmd.arg(ref mut zone, "-c");
cmd.arg(ref mut zone, "exit 0");
cmd.env(ref mut zone, "ARI_MODE", "test");

var chained = process::Command::new("sh")
  .with_arg(ref mut zone, "-c")
  .with_arg(ref mut zone, "exit 0")
  .with_env(ref mut zone, "ARI_MODE", "test");
```

The mutating `arg`/`env` methods return `void` because they take both
`ref mut Command` and `ref mut Zone`; Ari's borrow-return tracking cannot
currently return `ref mut Command` from a function with two mutable-borrow
parameters. The `with_*` methods take the command by value, append using the
explicit zone, and return the updated command, so they support arix-style
builder chains without hiding allocation.

Use `process::arg("...")` and `process::env_var("NAME", "value")` for string
literals and other borrowed C-string-shaped values. Use `arg_bytes`,
`env_var_bytes`, `Command::arg_bytes`, `Command::env_bytes`,
`Command::current_dir_bytes`, or `Command::current_dir_path` when the value
comes from an owned `String`, `PathBuf`, `Slice[u8]`, or `PathBytes`. These
byte-boundary helpers allocate NUL-terminated storage in the caller's zone,
return `Result`, and reject interior NUL bytes as `Error(InvalidInput)` through
the shared C-string boundary helpers.

`status()` spawns the command, waits for it, and returns
`Result[ExitStatus, Error]` so natural fallible process APIs preserve signal
termination detail. `status_code()` is the explicit compatibility helper for
callers that only want a normal exit code; it reports signal termination as
`Error(Other)`. `exit_status()` is kept as a readable alias for code written
before `status()` became typed. `spawn()` returns a `Child` handle with `pid`,
`wait`, `wait_status`, raw `kill`, typed `signal`, and `terminate` methods.
`exec()` applies the setup and replaces the current process with the program;
if `execvp` returns, Ari reports the host error.

`spawn`, `status`, `status_code`, `exit_status`, and `output` inherit the
parent process stdin/stdout/stderr by default unless a helper explicitly says it
captures output. Child environment policy is inherited by default:
`env`/`env_values` add or overwrite selected variables in the inherited
environment. Call `clear_env()` before `env(...)`, or use
`with_clear_env().with_env(...)`, when a tool should start from an empty
environment and then opt selected variables back in. Call `inherit_env()` or
`with_inherit_env()` to return to the default inherited-environment policy
without losing already configured environment assignments. On the current
Linux/POSIX runtime path, clearing uses `clearenv(3)` in the child setup just
before `execvp`; assignment uses `setenv(3)` after any clear step, so explicit
`env(...)` values remain visible to the child. `current_dir` is applied in the
child just before `exec`. For `exec()` in the current process, setup failures
return `Error`. For fork-based `spawn`/`status`/`output`, setup or `execvp`
failure happens in the child and currently appears to the parent as exit status
`127`; richer parent-visible setup errors require a future error-reporting pipe
in the process runtime.

`status_with_stdin(values)` spawns the child with a pipe connected to stdin,
writes the provided bytes from the parent, closes the write end, and then waits
for typed `ExitStatus`. `status_with_stdin_string(text)` is the string-literal
convenience form. These helpers are for bounded CLI-style input such as tests,
manifest snippets, or generated config text. If the parent cannot write the
complete byte slice, the helper waits for the child to avoid leaving a zombie
and returns the writer `Error`.

`status_with_stdin_file(path)` and `spawn_with_stdin_file(path)` redirect the
child's standard input from `path` during the child setup step. They take the
stdin source at execution time instead of storing it inside `Command`, which
keeps the command builder's allocation and lifetime rules simple. Use
`status_with_stdin_file_bytes(zone, bytes)` or
`status_with_stdin_file_path(zone, path)` when the path comes from a zone-backed
`String`, `PathBuf`, `Slice[u8]`, or `PathBytes`; those helpers reject interior
NUL bytes with `Error(InvalidInput)`. `status_with_stdin_null()` and
`spawn_with_stdin_null()` are the explicit no-input policy and redirect stdin
from `/dev/null`. If `current_dir` is also configured, the command applies the
working directory before opening the stdin path, so relative stdin paths are
resolved in the child working directory. Open or `dup2` failures happen in the
forked child today and surface as exit status `127`; parent-visible setup
errors require a future error-reporting pipe in the process runtime.

The module-level wrappers `process::spawn(ref command)`,
`process::status(ref command)`, `process::status_code(ref command)`,
`process::status_with_stdin(ref command, bytes)`,
`process::status_with_stdin_string(ref command, text)`,
`process::exit_status(ref command)`, `process::output_in(ref command, ref mut
zone)`, `process::output(ref command, ref mut zone)`, and
`process::exec(ref command)` call the matching `Command` methods. They exist so
code can use either builder method style or the direct `process::spawn(cmd)`
shape common in other standard libraries without losing `Error` detail.

`output_in(zone)` spawns the command with stdout and stderr redirected to
temporary pipes, waits for the child, reads both streams into `Vec[u8]` values
allocated in `zone`, and returns `Result[Output, Error]`. `Output::exit_status()`
returns the typed child status, `Output::status()` returns the normal exit code
or `-1` for compatibility, `Output::is_success()` checks the typed status, and
`Output::stdout()` / `Output::stderr()` expose borrowed `Slice[u8]` views over
the captured bytes. `stdout_string(zone)` and `stderr_string(zone)` validate the
captured bytes as UTF-8, copy them into a zone-owned `String`, and return
`Error(InvalidData)` for non-UTF-8 output. The `_in` suffix is intentional:
captured output owns buffers, so the caller chooses the allocation zone.

Arguments use `process::arg("...")` rather than raw `string` slices so the
builder can keep an executable-friendly C argv representation. Environment
entries use `process::env_var(name, value)`. The ergonomic rule is: call sites
still look like command construction, while the stdlib owns the pointer-level
ABI shape. When values come from `env::args`, path joins, format output, or
other owned byte strings, pass `value.as_slice()` to the `_bytes` helpers and
handle the returned `Result`.

`kill(pid, signal)` sends a POSIX signal and returns `Result[(), Error]`.
`kill_signal(pid, signal)` is the typed wrapper. `sig_check()` is signal `0`
for existence/permission checks, and `sighup`, `sigint`, `sigquit`, `sigkill`,
and `sigterm` expose the common POSIX signal values. `terminate(pid)` sends
`SIGTERM` as the conventional graceful termination request.

`ChildStdin`, `ChildStdout`, and `ChildStderr` are aliases for the current pipe
endpoint types used by process IO. Full streaming `spawn` redirection is still
future work, but these names let API users and future stdlib code share the
right handle vocabulary now.

`current_dir` and `executable_path` delegate to `std::env` and return
`Result[string, Error]` so process-oriented code can stay inside
`std::process` without losing failure detail. The `_optional` wrappers keep only
the success payload, `try_current_dir` and `try_executable_path` are
compatibility aliases for those optional forms, and `_or_default` wrappers keep
the older empty-string fallback behavior.

`temp_file(zone)` and `temp_dir(zone)` create unique paths under `/tmp` on the
current hosted backend. The `_in` variants accept a path prefix. Temp paths are
owned by the returned handle; `TempFile` owns the file descriptor until
`close`, `close_and_remove`, or drop, while path removal remains explicit.
`TempFile::close()` preserves close failures as `Result[(), Error]`;
`close_bool()` is the compatibility helper for callers that intentionally only
need a success flag.

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
  match process::fork() {
    Err(_) => {
      return process::failure();
    }
    Ok(child) => {
      if process::is_child(child) {
        process::exit(37);
        return 1;
      }

      match process::wait(child) {
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
  var zone = zone::temp(128);
  var cmd = process::Command::new("sh")
    .with_arg(ref mut zone, "-c")
    .with_arg(ref mut zone, "exit 17");

  match cmd.status() {
    Ok(status) => { return status.code_or(process::failure()); }
    Err(_) => { return process::failure(); }
  }
}
```

arix-style compiler invocation with owned output text:

```ari
fn compile_main(
  zone: ref mut Zone,
  compiler: string,
  output: std::string::String
) -> std::Result[(), std::error::Error] {
  var cmd = process::Command::new(compiler)
    .with_arg(zone, "src/main.ari")
    .with_arg(zone, "-o");

  cmd.arg_bytes(zone, output.as_slice())?;

  let status = cmd.status()?;
  if !status.success() {
    return std::Err<(), std::error::Error>(std::error::new(std::error::Other));
  }
  return std::Ok<(), std::error::Error>(());
}
```

Command stdin redirection:

```ari
fn run_with_inline_manifest(zone: ref mut Zone) -> Result[process::ExitStatus, error::Error] {
  var cmd = process::Command::new("sh")
    .with_arg(zone, "-c")
    .with_arg(zone, "read name; test \"$name\" = package");

  return cmd.status_with_stdin_string("package\n");
}

fn run_with_manifest_input(zone: ref mut Zone) -> Result[process::ExitStatus, error::Error] {
  var cmd = process::Command::new("sh")
    .with_arg(zone, "-c")
    .with_arg(zone, "read name; test \"$name\" = package")
    .with_current_dir("build/prelude");

  return cmd.status_with_stdin_file("Ari.toml");
}
```

Typed signals and temp paths:

```ari
fn main() -> i64 {
  var zone = zone::temp(256);
  if process::kill_signal(process::id(), process::sig_check()).is_err() {
    return process::failure();
  }

  match process::temp_file(ref mut zone) {
    Err(_) => { return process::failure(); }
    Ok(value) => {
      var file = value;
      if file.path().len == 0 {
        return process::failure();
      }
      return file.close_and_remove().is_ok() as i64;
    }
  }
}
```

Command setup and child handle:

```ari
fn main() -> i64 {
  var args = [process::arg("-c"), process::arg("test \"$ARI_MODE\" = test")];
  var env = [process::env_var("ARI_MODE", "test")];
  var cmd = process::command_with_args("sh", args.as_slice());
  cmd.env_values(env.as_slice());
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
      match output.stdout_string(ref mut zone) {
        Err(_) => { return process::failure(); }
        Ok(text) => { return text.len(); }
      }
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
- `Command` currently supports `arg`, `args`, `arg_bytes`, `env`, `env_var`,
  `env_bytes`, `clear_env`, `inherit_env`, by-value
  `with_arg`/`with_env`/`with_clear_env`/`with_inherit_env`/
  `with_current_dir`, explicit file or `/dev/null` stdin redirection at
  `spawn`/`status` time, bounded pipe-backed stdin for `status`,
  `spawn`, `status`, `exit_status`, `output`, `output_in`, `exec`, inherited
  stdio by default, inherited or cleared environment plus selected assignments,
  working-directory setup, captured stdout and stderr, typed status inspection,
  and `Child` wait/kill/signal helpers.
  `output_in` is intended for small captured outputs today: large concurrent
  stdout/stderr streams need future readiness/nonblocking draining to avoid
  pipe-buffer backpressure. Interactive streaming stdin, parent-visible setup
  errors, and portable platform-specific status detail are future work.
- Exit runs through the host process immediately. Do not expect Ari destructors
  or zone cleanup to run after `process::exit`.
- Abort also terminates immediately through the host runtime and should be
  treated as noreturn.
- `wait_raw(pid)` currently decodes only normal child exit statuses and returns
  `-1` for wait failures, signaled children, and other non-normal states.
  Prefer `wait_status(pid)` or `Child::wait_status()` when signal termination
  matters.
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
tests/cases/standard-library/ok/process/std-process-high-level.ari
tests/cases/standard-library/ok/process/std-process-stdin.ari
```

`make check-prelude` emits LLVM, checks the runtime hook symbols, and executes
the programs. The abort fixture compiles and runs only a non-aborting path while
checking that the abort hook lowers to the host `abort` declaration. The command
fixture covers argument passing, environment setup, working-directory setup,
method and module-level `status`/`spawn`, module-level `exit_status`,
`Child::wait`, non-destructive `kill(0)`, and the env-backed Result path
wrappers. The typed-status fixture covers
`ExitStatus`, `Command::exit_status`, `Child::wait_status`, normal exit codes,
and signal termination. The output fixture covers method and module-level small
stdout/stderr capture, exit status accessors, and missing command status `127`.
The high-level fixture covers zone-backed `Command::arg`/`env_var`, typed
by-value `with_arg`/`with_env`/`with_current_dir` chains, byte-boundary
`arg_bytes`/`env_bytes`/`current_dir_path` helpers, NUL rejection,
`ExitCode`, typed `Signal`, child stream endpoint aliases, current path
wrappers, and temp file/temp dir constructors.
The stdin fixture covers pipe-backed byte and string stdin, file-backed stdin
redirection, `/dev/null`, owned-byte path validation, `PathBytes` input, and
current-directory-relative stdin paths.
Public declarations are tracked in
`tests/std_api_manifest.txt` and checked by `make check-std-api`.
