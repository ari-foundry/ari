# std::env

`std::env` is the user-facing home for process environment helpers. It exposes
process arguments with natural names and `Option`-returning accessors, a small
environment-variable surface, and the basic process-local path state that most
CLIs need: current directory and executable path. Argument helpers are built on
top of `std::context`, which owns the low-level runtime hooks initialized by
the generated host entry wrapper.

Environment variable helpers use natural fallible names: `get`, `get_os`,
`set`, and `remove` return `Result[..., env::Error]`. Compact compatibility
helpers that discard error detail are named explicitly: `try_get`,
`try_get_os`, `get_or_default`, `get_os_or_default`, `set_unchecked`, and
`remove_unchecked`.

Argument and process-path helpers still include transitional `*_result` names
because their natural names are tied to long-standing runtime hooks. The final
direction remains the same across the module: natural fallible names return
`Result`, while `_optional`, `_or`, `_or_default`, `_raw`, or `_unchecked`
names opt into information-discarding or boundary behavior.

Use `std::env` from application code when you want friendly arguments,
environment variables, or current process path state. Use `std::context` when
you specifically need the startup snapshot captured by the runtime context.

## API

```ari
env::Error
env::ErrorKind
env::arg_count() -> i64
env::arg(index: i64) -> string
env::has_arg(index: i64) -> bool
env::try_arg(index: i64) -> Option[string]
env::arg_result(index: i64) -> Result[string, env::Error]
env::arg_os(index: i64) -> std::string::OsStr
env::try_arg_os(index: i64) -> Option[std::string::OsStr]
env::arg_os_result(index: i64) -> Result[std::string::OsStr, env::Error]
env::program_name() -> Option[string]
env::program_name_os() -> Option[std::string::OsStr]
env::program_name_result() -> Result[string, env::Error]
env::program_name_os_result() -> Result[std::string::OsStr, env::Error]
env::get(name: string) -> Result[string, env::Error]
env::get_os(name: string) -> Result[std::string::OsStr, env::Error]
env::get_or_default(name: string) -> string
env::get_os_or_default(name: string) -> std::string::OsStr
env::has(name: string) -> bool
env::try_get(name: string) -> Option[string]
env::try_get_os(name: string) -> Option[std::string::OsStr]
env::set(name: string, value: string) -> Result[(), env::Error]
env::set_unchecked(name: string, value: string) -> bool
env::remove(name: string) -> Result[(), env::Error]
env::remove_unchecked(name: string) -> bool
env::current_dir() -> string
env::try_current_dir() -> Option[string]
env::current_dir_result() -> Result[string, env::Error]
env::current_dir_os() -> std::string::OsStr
env::try_current_dir_os() -> Option[std::string::OsStr]
env::current_dir_os_result() -> Result[std::string::OsStr, env::Error]
env::current_dir_path() -> std::path::PathBytes
env::try_current_dir_path() -> Option[std::path::PathBytes]
env::current_dir_path_result() -> Result[std::path::PathBytes, env::Error]
env::set_current_dir(path: string) -> bool
env::set_current_dir_result(path: string) -> Result[(), env::Error]
env::executable_path() -> string
env::try_executable_path() -> Option[string]
env::executable_path_result() -> Result[string, env::Error]
env::executable_path_os() -> std::string::OsStr
env::try_executable_path_os() -> Option[std::string::OsStr]
env::executable_path_os_result() -> Result[std::string::OsStr, env::Error]
```

`arg_count()` returns the number of host arguments.

`has_arg(index)` returns `true` only when `0 <= index < arg_count()`.
Negative indexes are always false.

`try_arg(index)` is the preferred normal-control-flow accessor. It returns
`Some(argument)` for an available argument and `None<string>()` when the index
is missing.

`arg_result(index)` returns `Ok(argument)` for an available argument and
`Err(Error(NotFound))` for an out-of-range index. `arg_os_result(index)` uses
the same policy for OS-string bytes.

`arg_os(index)` and `try_arg_os(index)` expose the same argument as an
`std::string::OsStr`. Use them when the argument should stay OS-boundary bytes
until the caller explicitly validates it as UTF-8.

`program_name()` is `try_arg(0)`, so it returns the host-provided executable
name when one exists.

`program_name_os()` is the OS-string form of `program_name()`.
`program_name_result()` and `program_name_os_result()` are the corresponding
`Result` helpers and return `NotFound` if the host context has no argument 0.

`arg(index)` returns the raw lowercase Ari `string` from the runtime context.
It currently returns an empty string for out-of-range indexes, so prefer
`try_arg` or `has_arg` unless absence is impossible in the surrounding code.

`has(name)` returns whether an environment variable is visible to the current
process. `get(name)` returns `Ok(value)` when the variable exists and
`Err(Error(NotFound))` when it is absent. Missing environment variables are
ordinary absence, not a runtime failure, but the `Result` shape lets APIs
compose with other fallible hosted helpers without erasing the reason.
`try_get(name)` keeps only the success payload as `Option[string]`, and
`get_or_default(name)` is the compatibility helper for older code that wants
an empty string when the variable is missing.

`get_os(name)` applies the same `Result` policy to an `std::string::OsStr`
view. `try_get_os(name)` keeps only the optional success value, and
`get_os_or_default(name)` is the compatibility helper. On the current POSIX
target, OS-string environment values preserve the raw bytes from the runtime C
string and let callers choose `try_utf8()` or a byte-level policy at the
boundary.

`set(name, value)` overwrites the current process environment variable and
returns `Ok(())` when the host accepts the mutation. `remove(name)` unsets the
variable and returns `Ok(())` on success. These mutations affect the current
process and children spawned later by this process; they do not edit a user's
shell profile or global system environment. `set_unchecked` and
`remove_unchecked` keep the older boolean compatibility shape. Today failed
host mutations become `Error(Other)` because the runtime bool hook does not
yet expose a platform errno payload.

`current_dir()` returns the process current working directory as a borrowed
lowercase Ari `string`. It returns an empty string if the host cannot provide
one. `try_current_dir()` is the preferred accessor when failure is an ordinary
branch.
`current_dir_result()` returns `Err(Error(Other))` for that same runtime
failure sentinel.

`current_dir_os()` and `try_current_dir_os()` expose the current directory as
`std::string::OsStr`. `current_dir_path()` and `try_current_dir_path()` expose
the same borrowed bytes as `std::path::PathBytes`, which is the preferred form
for lexical path operations such as `is_absolute`, `components`, `file_name`,
or `normalize_in`.
`current_dir_os_result()` and `current_dir_path_result()` keep the same borrowed
view policy while preserving the failure branch as `Result`.

`set_current_dir(path)` changes the current process working directory and
returns whether the host accepted the request. This is process-local state:
later relative paths in this process will observe the change, and child
processes spawned later should inherit it.
`set_current_dir_result(path)` returns `Ok(())` on success and `Error(Other)`
when the current runtime rejects the change.

`std::context::cwd()` is different: it is the working-directory snapshot taken
before source `main` runs, so it stays stable even if `set_current_dir(path)`
later succeeds.

`executable_path()` returns the host path to the running executable when the
platform can provide it. On the current Linux backend this uses
`/proc/self/exe`. It returns an empty string if the path cannot be read or does
not fit the runtime buffer. `try_executable_path()` wraps that policy in
`Option[string]`.
`executable_path_result()` returns `Err(Error(Other))` for the same failure
sentinel.

`executable_path_os()` and `try_executable_path_os()` expose the executable
path as `std::string::OsStr`. Convert that value with `std::path::from_os`
when the next operation is path manipulation.
`executable_path_os_result()` is the `Result` form for code that wants to keep
path lookup failures explicit.

## Example

```ari
fn main() -> i64 {
  let name = env::program_name();

  if name.is_some() {
    println("program={}", name.unwrap());
  }

  let first = env::try_arg(1);
  if first.is_some() {
    println("first arg={}", first.unwrap());
  }

  match env::arg_result(1) {
    Ok(value) => println("first arg={}", value),
    Err(reason) => {
      if reason.is_not_found() {
        println("no first argument");
      }
    }
  }

  let raw_first = env::try_arg_os(1);
  if raw_first.is_some() {
    let first_os = raw_first.unwrap();
    let maybe_text = first_os.try_utf8();
    if maybe_text.is_some() {
      println("first arg was valid UTF-8");
    }
  }

  return 0;
}
```

Environment variables:

```ari
fn main() -> i64 {
  if env::set("ARI_MODE", "dev").is_ok() {
    match env::get("ARI_MODE") {
      Ok(value) => println("mode={}", value),
      Err(_) => {}
    }
  }

  env::remove("ARI_MODE").unwrap();
  return 0;
}
```

Current directory and executable path:

```ari
fn main() -> i64 {
  let cwd = env::try_current_dir();
  if cwd.is_some() {
    println("cwd={}", cwd.unwrap());
  }

  let cwd_path = env::try_current_dir_path();
  if cwd_path.is_some() {
    let path = cwd_path.unwrap();
    if path.is_absolute() {
      println("cwd is absolute");
    }
  }

  if env::set_current_dir(env::current_dir()) {
    println("cwd unchanged");
  }

  match env::current_dir_result() {
    Ok(path) => println("cwd={}", path),
    Err(_) => println("cwd unavailable"),
  }

  let exe = env::try_executable_path();
  if exe.is_some() {
    println("exe={}", exe.unwrap());
  }

  return 0;
}
```

## Current Limits

- Path strings are borrowed from fixed runtime buffers. Copy into a
  zone-backed `std::string::String` before storing them beyond the immediate
  use site or before calling another path helper that may reuse the same
  buffer.
- The `*_os` and `*_path` helpers are borrowed views over those same runtime
  buffers. They clarify API intent but do not extend the lifetime of the
  underlying bytes.
- Executable path lookup is currently Linux-specific and reads
  `/proc/self/exe`. Cross-platform normalization belongs behind this module
  later.
- Child-process mutation is a future OS-facing slice. Current process
  identity and termination helpers live in `std::process`.
- Argument strings are borrowed from the host runtime context. Copy into a
  zone-backed `std::string::String` when owned text is needed.
- Environment variable values are also borrowed host strings. Copy into a
  zone-backed `std::string::String` before storing them beyond the immediate
  use site.
- The current backend implements these helpers through the host C environment
  functions. Platform-specific environment behavior stays behind this module.

## Tests

- `tests/cases/standard-library/ok/env/std-env-args.ari` checks the source wrappers,
  `Option` and `Result` result shapes, LLVM symbols, and executable behavior.
- `tests/cases/standard-library/ok/env/std-env-vars.ari` checks current-process
  environment `get`/`get_or_default`/`has`/`try_get`/`get_os`/`set`/`remove`
  plus `_unchecked` compatibility behavior and runtime hook lowering.
- `tests/cases/standard-library/ok/env/std-env-paths.ari` checks current
  directory, `set_current_dir`, executable-path lookup, `Option` and `Result`
  wrappers, LLVM symbols, and executable behavior.
- `tests/cases/standard-library/ok/env/std-env-os-path-views.ari` checks
  `OsStr` argument/environment/path views and `PathBytes` current-directory
  views.
- `tests/cases/standard-library/ok/context/std-context-args.ari` remains the lower-level
  context hook and root alias coverage.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused runtime/source coverage.
