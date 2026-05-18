# std::option And std::result

`Option[T]` and `Result[T, E]` are defined at the `std` root because they are
part of everyday Ari control flow. Their method implementations live in
`std::option` and `std::result` so the helpers remain ordinary Ari source.

## When To Use Them

Use `Option[T]` for a value that may be absent. Use `Result[T, E]` when failure
has information the caller should inspect or propagate.

Use `assert` or `panic` only for programmer errors. Ordinary missing values and
recoverable failures should flow through `Option` or `Result`.

## Option API

```ari
value.is_some()
value.is_none()
value.unwrap_or(fallback)
value.unwrap_or_else(op)
value.unwrap()
value.expect("message")
value.map<U>(op)
value.and_then<U>(op)
value.or(fallback)
value.or_else(op)
value.xor(other)
value.ok_or<E>(error)
value.ok_or_else<E>(op)
```

`unwrap` and `expect` panic on `None`. Prefer `unwrap_or`,
`unwrap_or_else`, `match`, or `?` in normal control flow.

`ok_or` and `ok_or_else` convert `Option[T]` into `Result[T, E]`. The lazy
form calls its function only for `None`.

## Result API

```ari
value.is_ok()
value.is_err()
value.unwrap_or(fallback)
value.unwrap_or_else(op)
value.unwrap()
value.expect("message")
value.unwrap_err()
value.expect_err("message")
value.ok()
value.err()
value.map<U>(op)
value.map_err<F>(op)
value.and_then<U>(op)
value.or_else<F>(op)
```

`ok` keeps the success payload as `Option[T]`. `err` keeps the error payload as
`Option[E]`. `unwrap_or_else` receives the error and returns a fallback success
value.

## Example

```ari
fn recover(code: i64) -> i64 {
  return code + 1;
}

fn parse_digit(byte: u8) -> Result[i64, i64] {
  return std::ascii::digit_value(byte).ok_or<i64>(100);
}

fn main() -> i64 {
  let value = parse_digit(55 as u8).unwrap_or_else(recover);
  return value;
}
```

## Tests

Focused positive tests:

```text
tests/cases/standard-library/ok/prelude-option-result-methods.ari
tests/cases/standard-library/ok/prelude-option-result-combinators.ari
tests/cases/standard-library/ok/prelude-option-result-conversions.ari
tests/cases/standard-library/ok/prelude-option-result-unwrap.ari
```

`make check-prelude` compiles the tests to LLVM, checks representative method
symbols, links executable cases, and verifies results.

The public API is tracked in `tests/std_api_manifest.txt` and checked by
`make check-std-api`.
