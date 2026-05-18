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
value.is_some_and(op)
value.is_none_or(op)
value.unwrap_or(fallback)
value.unwrap_or_else(op)
value.unwrap()
value.expect("message")
value.map<U>(op)
value.and_then<U>(op)
value.flatten()
value.transpose()
value.or(fallback)
value.or_else(op)
value.xor(other)
value.ok_or<E>(error)
value.ok_or_else<E>(op)
```

`unwrap` and `expect` panic on `None`. Prefer `unwrap_or`,
`unwrap_or_else`, `match`, or `?` in normal control flow.

`is_some_and` and `is_none_or` consume the `Option[T]` and pass the payload to
`op` only for `Some`. Use borrowed `is_some` or `is_none` when you only need
to inspect the case without consuming a local value.

`ok_or` and `ok_or_else` convert `Option[T]` into `Result[T, E]`. The lazy
form calls its function only for `None`.

`flatten` is available on nested `Option[Option[T]]` values. It keeps the
inner `Some(T)`, turns `Some(None<T>())` into `None<T>()`, and also keeps an
outer `None<Option[T]>()` as `None<T>()`.

`transpose` is available on `Option[Result[T, E]]` values. It turns
`Some(Ok(T))` into `Ok(Some(T))`, `Some(Err(E))` into `Err(E)`, and
`None<Result[T, E]>()` into `Ok(None<T>())`.

## Result API

```ari
value.is_ok()
value.is_err()
value.is_ok_and(op)
value.is_err_and(op)
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
value.or<F>(fallback)
value.or_else<F>(op)
```

`ok` keeps the success payload as `Option[T]`. `err` keeps the error payload as
`Option[E]`. `unwrap_or_else` receives the error and returns a fallback success
value. `or` uses an already-built fallback `Result[T, F]` when the receiver is
`Err`; `or_else` builds that fallback lazily from the original error.

`is_ok_and` consumes the `Result[T, E]` and predicates the success payload.
`is_err_and` consumes it and predicates the error payload.

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
tests/cases/standard-library/ok/prelude-option-result-predicates.ari
tests/cases/standard-library/ok/prelude-option-flatten.ari
tests/cases/standard-library/ok/prelude-option-transpose.ari
tests/cases/standard-library/ok/prelude-option-result-combinators.ari
tests/cases/standard-library/ok/prelude-option-result-conversions.ari
tests/cases/standard-library/ok/prelude-option-result-unwrap.ari
```

`make check-prelude` compiles the tests to LLVM, checks representative method
symbols, links executable cases, and verifies results.

The public API is tracked in `tests/std_api_manifest.txt` and checked by
`make check-std-api`.
