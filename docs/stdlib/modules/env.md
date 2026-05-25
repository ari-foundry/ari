# std::env

`std::env` is the user-facing home for process environment helpers. It exposes
process arguments with natural `Result`-returning accessors, a small
environment-variable surface, and the basic process-local path state that most
CLIs need: current directory and executable path. Argument helpers are built on
top of `std::context`, which owns the low-level runtime hooks initialized by
the generated host entry wrapper.

Environment variable helpers use natural fallible names: `get`, `get_os`,
`set`, and `remove` return `Result[..., env::Error]`. Compact compatibility
helpers that discard error detail are named explicitly: `try_get`,
`try_get_os`, `get_or_default`, `get_os_or_default`, `set_unchecked`, and
`remove_unchecked`.

Argument and process-path helpers follow the default stdlib error model:
natural fallible names return `Result`, while `_optional`, `_or_default`,
`_raw`, or `_unchecked` names opt into information-discarding or boundary
behavior. Transitional `*_result` aliases remain for older code that already
used the explicit suffix while the natural names migrated.

Use `std::env` from application code when you want friendly arguments,
environment variables, or current process path state. Use `std::context` when
you specifically need the startup snapshot captured by the runtime context.

## API

```ari
env::Error
env::ErrorKind
env::arg_count() -> i64
env::arg(index: i64) -> Result[string, env::Error]
env::arg_optional(index: i64) -> Option[string]
env::arg_unchecked(index: i64) -> string
env::has_arg(index: i64) -> bool
env::try_arg(index: i64) -> Option[string]
env::arg_result(index: i64) -> Result[string, env::Error]
env::arg_os(index: i64) -> Result[std::string::OsStr, env::Error]
env::arg_os_optional(index: i64) -> Option[std::string::OsStr]
env::arg_os_unchecked(index: i64) -> std::string::OsStr
env::try_arg_os(index: i64) -> Option[std::string::OsStr]
env::arg_os_result(index: i64) -> Result[std::string::OsStr, env::Error]
env::program_name() -> Result[string, env::Error]
env::program_name_optional() -> Option[string]
env::program_name_os() -> Result[std::string::OsStr, env::Error]
env::program_name_os_optional() -> Option[std::string::OsStr]
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
env::current_dir() -> Result[string, env::Error]
env::current_dir_or_default() -> string
env::current_dir_optional() -> Option[string]
env::try_current_dir() -> Option[string]
env::current_dir_raw() -> string
env::current_dir_os() -> Result[std::string::OsStr, env::Error]
env::current_dir_os_or_default() -> std::string::OsStr
env::current_dir_os_optional() -> Option[std::string::OsStr]
env::try_current_dir_os() -> Option[std::string::OsStr]
env::current_dir_path() -> Result[std::path::PathBytes, env::Error]
env::current_dir_path_or_default() -> std::path::PathBytes
env::current_dir_path_optional() -> Option[std::path::PathBytes]
env::try_current_dir_path() -> Option[std::path::PathBytes]
env::set_current_dir(path: string) -> Result[(), env::Error]
env::set_current_dir_raw(path: string) -> bool
env::set_current_dir_unchecked(path: string) -> bool
env::executable_path() -> Result[string, env::Error]
env::executable_path_or_default() -> string
env::executable_path_optional() -> Option[string]
env::try_executable_path() -> Option[string]
env::executable_path_raw() -> string
env::executable_path_os() -> Result[std::string::OsStr, env::Error]
env::executable_path_os_or_default() -> std::string::OsStr
env::executable_path_os_optional() -> Option[std::string::OsStr]
env::try_executable_path_os() -> Option[std::string::OsStr]
```

`arg_count()` returns the number of host arguments.

`has_arg(index)` returns `true` only when `0 <= index < arg_count()`.
Negative indexes are always false.

`arg(index)` returns `Ok(argument)` for an available argument and
`Err(Error(NotFound))` for an out-of-range index. `arg_optional(index)` and the
older `try_arg(index)` keep only the optional success payload.
`arg_unchecked(index)` exposes the raw runtime-context string hook and keeps
the older empty-string behavior for out-of-range indexes.

`arg_os(index)` applies the same `Result` policy to an `std::string::OsStr`
view. Use it when the argument should stay OS-boundary bytes until the caller
explicitly validates it as UTF-8. `arg_os_optional(index)` and
`try_arg_os(index)` discard the reason, while `arg_os_unchecked(index)` uses
the raw context hook.

`program_name()` is `arg(0)`, so it returns the host-provided executable name
when one exists and `NotFound` when the host context has no argument 0.
`program_name_optional()` is the explicit optional compatibility form.
`program_name_os()` and `program_name_os_optional()` mirror that policy for
`OsStr`. The `program_name_result()` and `program_name_os_result()` aliases are
kept for source compatibility.

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

`current_dir()` returns the process current working directory as
`Result[string, Error]`. The success string is borrowed from a runtime buffer.
`current_dir_or_default()` is the compatibility helper for older code that
wants an empty string when the host cannot provide one, and
`current_dir_optional()` / `try_current_dir()` keep only the optional success
payload. `current_dir_raw()` exposes the runtime hook directly and should be
used only at boundary or compatibility sites.

`current_dir_os()` and `current_dir_path()` use the same `Result` policy for
`std::string::OsStr` and `std::path::PathBytes` views. `PathBytes` is the
preferred form for lexical path operations such as `is_absolute`, `components`,
`file_name`, or `normalize_in`. `_or_default` and `_optional` variants discard
the `Error` detail explicitly; the `try_*` names are compatibility aliases for
the optional forms.

`set_current_dir(path)` changes the current process working directory and
returns `Ok(())` when the host accepts the request. This is process-local
state: later relative paths in this process will observe the change, and child
processes spawned later should inherit it. `set_current_dir_unchecked(path)` and
`set_current_dir_raw(path)` keep the older boolean compatibility shape.

`std::context::cwd()` is different: it is the working-directory snapshot taken
before source `main` runs, so it stays stable even if `set_current_dir(path)`
later succeeds.

`executable_path()` returns the host path to the running executable as
`Result[string, Error]` when the platform can provide it. On the current Linux
backend this uses `/proc/self/exe`. If the path cannot be read or does not fit
the runtime buffer, it returns `Err(Error(Other))`. `executable_path_or_default`
keeps the older empty-string behavior, and `executable_path_optional()` /
`try_executable_path()` keep only the optional success payload.

`executable_path_os()` exposes the executable path as `Result[OsStr, Error]`.
Convert the `Ok` value with `std::path::from_os` when the next operation is path
manipulation. `executable_path_os_or_default()` and
`executable_path_os_optional()` are the explicit information-discarding forms.

## Example

```ari
fn main() -> i64 {
  match env::program_name() {
    Ok(name) => println("program={}", name),
    Err(_) => {}
  }

  match env::arg(1) {
    Ok(value) => println("first arg={}", value),
    Err(reason) => {
      if reason.is_not_found() {
        println("no first argument");
      }
    }
  }

  let raw_first = env::arg_os_optional(1);
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
  match env::current_dir() {
    Ok(cwd) => {
      println("cwd={}", cwd);

      if env::set_current_dir(cwd).is_ok() {
        println("cwd unchanged");
      }
    }
    Err(_) => println("cwd unavailable"),
  }

  match env::current_dir_path() {
    Ok(path) => {
      if path.is_absolute() {
        println("cwd is absolute");
      }
    }
    Err(_) => {}
  }

  match env::executable_path() {
    Ok(exe) => println("exe={}", exe),
    Err(_) => println("exe unavailable"),
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
