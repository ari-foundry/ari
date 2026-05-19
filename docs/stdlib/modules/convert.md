# std::convert

`std::convert` is Ari's home for explicit value conversion vocabulary. It
starts small: the module gives names to conversion traits and provides source
helpers that make generic conversion call sites read consistently.

The names intentionally avoid type suffixes. The source and destination types
belong in generic arguments and trait impls, not in public function names.

## When To Use It

Use `std::convert` when code should say "convert this value through a trait"
instead of spelling a direct cast or constructor at every call site.

Use `as` for primitive casts where Ari already has a clear built-in rule. Use
ordinary constructors when a conversion needs extra policy, validation, or
allocation. Use `convert::from` and `convert::into` when the conversion is
part of a reusable generic API.

## API

Trait surface:

```ari
convert::From[T]
convert::Into[T]
convert::TryFrom[T]
convert::TryInto[T]
```

Current callable helpers:

```ari
convert::identity<T>(value)
convert::from<T, U>(value)
convert::into<T, U>(value)
```

`identity` returns the value unchanged. It is useful in generic examples,
higher-order calls, and places where a function value is needed but no
conversion is desired.

`from<T, U>` calls `convert::From[T]` for destination type `U`:

```ari
let value = convert::from<i32, i64>(small)
```

`into<T, U>` calls `convert::Into[T]` on source type `U`:

```ari
let value = convert::into<i64, i32>(small)
```

The type argument order follows the trait relationship:

- `from<T, U>` means "build `U` from `T`"
- `into<T, U>` means "turn `U` into `T`"

## Example

```ari
impl convert::From[i32] for i64 {
  fn from(value: i32) -> i64 {
    return value as i64;
  }
}

impl convert::Into[i64] for i32 {
  fn into(self) -> i64 {
    return self as i64;
  }
}

fn main() -> i64 {
  let a = convert::identity<i64>(7);
  let b = convert::from<i32, i64>(11 as i32);
  let c = convert::into<i64, i32>(13 as i32);
  return a + b + c;
}
```

## Limits

`TryFrom` and `TryInto` are currently trait names only. They do not have
required methods yet because Ari's fallible conversion result shape still
needs a deliberate policy.

The root prelude also exposes `From`, `Into`, `TryFrom`, and `TryInto` as
reserved trait names. The helpers on this page use the `std::convert` trait
paths, so implement `convert::From` or `convert::Into` when calling
`convert::from` or `convert::into`.

## Tests

The focused positive behavior test is:

```text
tests/cases/standard-library/ok/convert/std-convert-value-helpers.ari
```

`make check-prelude` emits LLVM for that file, checks representative public
helper symbols, builds an executable, and verifies the exit code. The public
API is tracked in `tests/std_api_manifest.txt` and checked by
`make check-std-api`.

## Future Work

Potential next slices:

- unify root prelude and `std::convert` trait documentation once trait
  re-export policy is mature
- add fallible conversion methods after the `Result` shape and error policy
  are documented
- add conversion impl examples for library types such as slices, strings, and
  source vectors when ownership and allocation behavior is explicit
