# std::env

`std::env` is the user-facing home for process environment helpers. The first
implemented slice exposes process arguments with natural names and
`Option`-returning accessors. It is built on top of `std::context`, which owns
the low-level runtime hooks initialized by the generated host entry wrapper.

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

## Current Limits

- Environment variables are not exposed yet. Planned names should prefer
  natural APIs such as `env::get`, `env::has`, `env::set`, and `env::remove`
  once runtime string and OS wrapper policy is stable.
- Current working directory, executable path normalization, and child-process
  mutation are future OS-facing slices. Current process id/exit helpers live in
  `std::process`.
- Argument strings are borrowed from the host runtime context. Copy into a
  zone-backed `std::string::String` when owned text is needed.

## Tests

- `tests/cases/standard-library/ok/std-env-args.ari` checks the source wrappers,
  `Option` result shape, LLVM symbols, and executable behavior.
- `tests/cases/standard-library/ok/std-context-args.ari` remains the lower-level
  context hook and root alias coverage.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused runtime/source coverage.
