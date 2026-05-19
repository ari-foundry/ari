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
context::arg(index: i64) -> string
context::thread_id() -> i64
context::cwd() -> string
context::executable_path() -> string
context::has_args() -> bool
context::has_arg(index: i64) -> bool
context::user_arg_count() -> i64
context::has_user_args() -> bool
context::is_main_thread() -> bool
context::has_cwd() -> bool
context::try_cwd() -> Option[string]
context::cwd_os() -> std::string::OsStr
context::try_cwd_os() -> Option[std::string::OsStr]
context::cwd_path() -> std::path::PathBytes
context::has_executable_path() -> bool
context::try_executable_path() -> Option[string]
context::executable_path_os() -> std::string::OsStr
context::try_executable_path_os() -> Option[std::string::OsStr]

arg_count() -> i64
arg(index: i64) -> string
has_arg(index: i64) -> bool
```

`arg_count()` is the root alias for `context::argc()`. `arg(index)` and
`has_arg(index)` are root aliases for the matching `std::context` functions.

`has_args()` is true when the host provided at least one argument slot.
On ordinary executable starts that means `argv[0]` exists, but embedders and
future shared-library hosts should still be allowed to install an empty context.

`has_arg(index)` returns `true` only when `0 <= index < context::argc()`.
Negative indexes are always false. Use it before `arg(index)` when absence is
an ordinary branch in low-level context code.

`user_arg_count()` counts arguments after `argv[0]`. It returns zero for an
empty context and for a program that was started without user arguments.
`has_user_args()` is the boolean form of that policy.

`arg(index)` returns a lowercase Ari `string`, which is currently a borrowed
pointer-shaped value. If `index` is out of range, it returns an empty string.
Index `0` is the host-provided `argv[0]` value.

`thread_id()` returns the Ari runtime thread id. The main thread is `0`.
`std::thread` installs nonzero ids for spawned Ari threads before they enter
source code, so `is_main_thread()` is the low-level predicate for the current
runtime context.

`cwd()` is the current working directory captured by `@ari_entry` before source
`main` runs. It is a startup snapshot: later `std::env::set_current_dir` calls
do not rewrite it. `has_cwd()` and `try_cwd()` are the safe branches for hosts
that could not provide a working directory. `cwd_os()` keeps the same bytes in
an OS-string view, and `cwd_path()` exposes them to lexical path helpers.

`executable_path()` is the startup executable path snapshot when the host can
provide one. On Linux hosted builds it is read from `/proc/self/exe`.
`has_executable_path()`, `try_executable_path()`, and the OS-string wrappers
mirror the `cwd` helpers.

Application code should usually prefer the user-facing `std::env` wrappers for
arguments and current process state. `std::context` stays useful for runtime,
tests, embedders, and other standard-library modules that need exact startup
context behavior.

## Example

```ari
fn main() -> i64 {
  println("argc={}", arg_count());

  if has_arg(1) {
    println("first user arg={}", arg(1));
  }

  if context::is_main_thread() {
    println("running on the main Ari thread");
  }

  println("started in {}", context::cwd());

  return 0;
}
```

## Current Limits

- `std::env` provides the user-facing argument helpers.
- Argument strings, `cwd()`, and `executable_path()` are borrowed from the
  runtime context. Copy into a zone-backed `std::string::String` later when
  longer-lived owned text is needed.
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
