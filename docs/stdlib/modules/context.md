# std::context

`std::context` exposes the low-level runtime context that the Ari host entry
wrapper receives or creates before source `main` runs. It is intentionally
small: the module answers "what did the program start with?", "where did it
start?", and "which Ari runtime thread is running this code?" without owning the full process,
environment, filesystem, or thread-management surface.

The primitive values come from runtime hooks initialized by `@ari_entry`.
Small policy helpers, such as `has_arg` and `is_main_thread`, live in Ari
source so their behavior is readable and testable like ordinary library code.

## API

```ari
context::argc() -> i64
context::arg(ref mut Zone, index: i64) -> std::string::String
context::arg_text(index: i64) -> String
context::try_arg(ref mut Zone, index: i64) -> Option[std::string::String]
context::try_arg_text(index: i64) -> Option[String]
context::thread_id() -> i64
context::cwd(ref mut Zone) -> std::string::String
context::cwd_text() -> String
context::executable_path(ref mut Zone) -> std::string::String
context::executable_path_text() -> String
context::has_args() -> bool
context::has_arg(index: i64) -> bool
context::user_arg_count() -> i64
context::has_user_args() -> bool
context::is_main_thread() -> bool
context::has_cwd() -> bool
context::try_cwd(ref mut Zone) -> Option[std::string::String]
context::try_cwd_text() -> Option[String]
context::cwd_os() -> std::string::OsStr
context::try_cwd_os() -> Option[std::string::OsStr]
context::cwd_path() -> std::path::PathBytes
context::has_executable_path() -> bool
context::try_executable_path(ref mut Zone) -> Option[std::string::String]
context::try_executable_path_text() -> Option[String]
context::executable_path_os() -> std::string::OsStr
context::try_executable_path_os() -> Option[std::string::OsStr]

arg_count() -> i64
arg(ref mut Zone, index: i64) -> Result[std::string::String, std::error::Error]
arg_text(index: i64) -> String
has_arg(index: i64) -> bool
```

`arg_count()` is the root alias for `context::argc()`. Root `arg(ref mut zone,
index)` is the user-facing `std::env::arg` alias and returns
`Result[String, Error]`. `arg_text(index)` and `context::*_text` functions are
borrowed runtime-boundary compatibility views.

`has_args()` is true when the host provided at least one argument slot.
On ordinary executable starts that means `argv[0]` exists, but embedders and
future shared-library hosts should still be allowed to install an empty context.

`has_arg(index)` returns `true` only when `0 <= index < context::argc()`.
Negative indexes are always false. Use it before `context::arg_text(index)`
when absence is an ordinary branch in low-level context code.

`user_arg_count()` counts arguments after `argv[0]`. It returns zero for an
empty context and for a program that was started without user arguments.
`has_user_args()` is the boolean form of that policy.

`context::arg(ref mut zone, index)` copies the startup argument into a
zone-backed `String`. If `index` is out of range, it copies the raw hook's empty
string. Prefer `std::env::arg(ref mut zone, index)` when absence should be an
error. `context::try_arg(ref mut zone, index)` returns `None` for missing
arguments, and `arg_text(index)` exposes the borrowed hook directly.

`thread_id()` returns the Ari runtime thread id. The main thread is `0`.
`std::thread` installs nonzero ids for spawned Ari threads before they enter
source code, so `is_main_thread()` is the low-level predicate for the current
runtime context.

`cwd(ref mut zone)` is the current working directory captured by `@ari_entry`
before source `main` runs, copied into an owned `String`. It is a startup
snapshot: later `std::env::set_current_dir` calls do not rewrite it.
`has_cwd()` and `try_cwd(ref mut zone)` are the safe branches for hosts that
could not provide a working directory. `cwd_text()` is the borrowed raw hook,
`cwd_os()` keeps the same bytes in an OS-string view, and `cwd_path()` exposes
them to lexical path helpers.

`executable_path(ref mut zone)` is the startup executable path snapshot when
the host can provide one. On Linux hosted builds it is read from
`/proc/self/exe`. `has_executable_path()`, `try_executable_path(ref mut zone)`,
and the OS-string wrappers mirror the `cwd` helpers.

Application code should usually prefer the user-facing `std::env` wrappers for
arguments and current process state. `std::context` stays useful for runtime,
tests, embedders, and other standard-library modules that need exact startup
context behavior.

## Example

```ari
fn main() -> i64 {
  var zone = zone::create(1024);
  println("argc={}", arg_count());

  if has_arg(1) {
    match arg(ref mut zone, 1) {
      Ok(value) => {
        println("first user arg={}", value);
      }
      Err(_) => {}
    }
  }

  if context::is_main_thread() {
    println("running on the main Ari thread");
  }

  println("started in {}", context::cwd(ref mut zone));

  zone::destroy(zone);
  return 0;
}
```

## Current Limits

- `std::env` provides the user-facing argument helpers.
- `*_text` helpers are borrowed from the runtime context and should stay at
  runtime/module boundaries. Natural names copy into zone-backed
  `std::string::String`.
- `std::thread` owns thread creation and joining. `std::context` intentionally
  stays limited to reading the current runtime id.
- Shared-library context behavior is still tracked separately in the compiler
  test matrix.

## Tests

- `tests/cases/standard-library/ok/context/context-argc.ari` checks direct argc access.
- `tests/cases/standard-library/ok/context/context-arg.ari` checks direct argument hook
  lowering.
- `tests/cases/standard-library/ok/input/prelude-input.ari` keeps context access
  covered beside prelude input hooks.
- `tests/cases/standard-library/ok/context/std-context-args.ari` checks `has_arg`,
  argument-count helpers, startup cwd/executable path snapshots,
  main-thread helpers, root aliases, LLVM hook visibility, and executable
  behavior.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused runtime/source coverage.
