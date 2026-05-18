# std::context

`std::context` exposes the process argument slice that the Ari host entry
wrapper receives from the operating system. It is intentionally small: the
module answers "how many arguments did the program start with?" and "is this
argument index available?" without promising environment, process, or file
system APIs yet.

The primitive values come from runtime hooks initialized by `@ari_entry`.
Small policy helpers, such as `has_arg`, live in Ari source so their behavior
is readable and testable like ordinary library code.

## API

```ari
context::argc() -> i64
context::arg(index: i64) -> string
context::has_arg(index: i64) -> bool

arg_count() -> i64
arg(index: i64) -> string
has_arg(index: i64) -> bool
```

`arg_count()` is the root alias for `context::argc()`. `arg(index)` and
`has_arg(index)` are root aliases for the matching `std::context` functions.

`has_arg(index)` returns `true` only when `0 <= index < context::argc()`.
Negative indexes are always false. Use it before `arg(index)` when absence is
an ordinary branch in your program.

`arg(index)` returns a lowercase Ari `string`, which is currently a borrowed
pointer-shaped value. If `index` is out of range, it returns an empty string.
Index `0` is the host-provided `argv[0]` value.

## Example

```ari
fn main() -> i64 {
  println("argc={}", arg_count());

  if has_arg(1) {
    println("first user arg={}", arg(1));
  }

  return 0;
}
```

## Current Limits

- There is no `std::env`, `std::process`, or filesystem module yet.
- Argument strings are borrowed from the runtime context. Copy into a
  zone-backed `std::string::String` later when longer-lived owned text is
  needed.
- Shared-library context behavior is still tracked separately in the compiler
  test matrix.

## Tests

- `tests/cases/standard-library/ok/context-argc.ari` checks direct argc access.
- `tests/cases/standard-library/ok/context-arg.ari` checks direct argument hook
  lowering.
- `tests/cases/standard-library/ok/prelude-input.ari` keeps context access
  covered beside prelude input hooks.
- `tests/cases/standard-library/ok/std-context-args.ari` checks `has_arg`,
  root aliases, LLVM hook visibility, and executable behavior.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused runtime/source coverage.
