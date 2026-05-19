# std::env

`std::env` is the user-facing home for process environment helpers. It exposes
process arguments with natural names and `Option`-returning accessors, a small
environment-variable surface, and the basic process-local path state that most
CLIs need: current directory and executable path. Argument helpers are built on
top of `std::context`, which owns the low-level runtime hooks initialized by
the generated host entry wrapper.

Use `std::env` from application code when you want to talk about program
startup state. Use `std::context` only when you are testing or extending the
runtime context layer itself.

## API

```ari
env::arg_count() -> i64
env::arg(index: i64) -> string
env::has_arg(index: i64) -> bool
env::try_arg(index: i64) -> Option[string]
env::program_name() -> Option[string]
env::get(name: string) -> string
env::has(name: string) -> bool
env::try_get(name: string) -> Option[string]
env::set(name: string, value: string) -> bool
env::remove(name: string) -> bool
env::current_dir() -> string
env::try_current_dir() -> Option[string]
env::set_current_dir(path: string) -> bool
env::executable_path() -> string
env::try_executable_path() -> Option[string]
```

`arg_count()` returns the number of host arguments.

`has_arg(index)` returns `true` only when `0 <= index < arg_count()`.
Negative indexes are always false.

`try_arg(index)` is the preferred normal-control-flow accessor. It returns
`Some(argument)` for an available argument and `None<string>()` when the index
is missing.

`program_name()` is `try_arg(0)`, so it returns the host-provided executable
name when one exists.

`arg(index)` returns the raw lowercase Ari `string` from the runtime context.
It currently returns an empty string for out-of-range indexes, so prefer
`try_arg` or `has_arg` unless absence is impossible in the surrounding code.

`has(name)` returns whether an environment variable is visible to the current
process. `try_get(name)` is the preferred normal-control-flow accessor for
environment variables: it returns `Some(value)` when the variable exists and
`None<string>()` when it is missing.

`get(name)` returns a borrowed lowercase Ari `string` from the host
environment. Missing variables return an empty string, so use `try_get` or
`has` when absence matters.

`set(name, value)` overwrites the current process environment variable and
returns whether the host accepted the mutation. `remove(name)` unsets the
variable and returns whether the host accepted the request. These mutations
affect the current process and children spawned later by this process; they do
not edit a user's shell profile or global system environment.

`current_dir()` returns the process current working directory as a borrowed
lowercase Ari `string`. It returns an empty string if the host cannot provide
one. `try_current_dir()` is the preferred accessor when failure is an ordinary
branch.

`set_current_dir(path)` changes the current process working directory and
returns whether the host accepted the request. This is process-local state:
later relative paths in this process will observe the change, and child
processes spawned later should inherit it.

`executable_path()` returns the host path to the running executable when the
platform can provide it. On the current Linux backend this uses
`/proc/self/exe`. It returns an empty string if the path cannot be read or does
not fit the runtime buffer. `try_executable_path()` wraps that policy in
`Option[string]`.

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

  return 0;
}
```

Environment variables:

```ari
fn main() -> i64 {
  if env::set("ARI_MODE", "dev") {
    let value = env::try_get("ARI_MODE");
    if value.is_some() {
      println("mode={}", value.unwrap());
    }
  }

  env::remove("ARI_MODE");
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

  if env::set_current_dir(env::current_dir()) {
    println("cwd unchanged");
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
- Executable path lookup is currently Linux-specific and reads
  `/proc/self/exe`. Cross-platform normalization belongs behind this module
  later.
- Child-process mutation is a future OS-facing slice. Current process id/exit
  helpers live in `std::process`.
- Argument strings are borrowed from the host runtime context. Copy into a
  zone-backed `std::string::String` when owned text is needed.
- Environment variable values are also borrowed host strings. Copy into a
  zone-backed `std::string::String` before storing them beyond the immediate
  use site.
- The current backend implements these helpers through the host C environment
  functions. Platform-specific environment behavior stays behind this module.

## Tests

- `tests/cases/standard-library/ok/env/std-env-args.ari` checks the source wrappers,
  `Option` result shape, LLVM symbols, and executable behavior.
- `tests/cases/standard-library/ok/env/std-env-vars.ari` checks current-process
  environment `get`/`has`/`try_get`/`set`/`remove` behavior and runtime hook
  lowering.
- `tests/cases/standard-library/ok/env/std-env-paths.ari` checks current
  directory, `set_current_dir`, executable-path lookup, `Option` wrappers,
  LLVM symbols, and executable behavior.
- `tests/cases/standard-library/ok/context/std-context-args.ari` remains the lower-level
  context hook and root alias coverage.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused runtime/source coverage.
