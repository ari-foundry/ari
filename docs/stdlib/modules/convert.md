# std::convert

`std::convert` is Ari's home for explicit value conversion vocabulary. The
module gives names to conversion traits and provides source helpers that make
generic conversion call sites read consistently.

The names intentionally avoid type suffixes. The source and destination types
belong in generic arguments and trait impls, not in public function names.

## When To Use It

Use `std::convert` when code should say "convert this value through a trait"
instead of spelling a direct cast or constructor at every call site.

Use `as` for primitive casts where Ari already has a clear built-in rule. Use
ordinary constructors when a conversion needs extra policy or allocation. Use
`convert::from` and `convert::into` when conversion cannot fail. Use
`convert::try_from` and `convert::try_into` when invalid input is ordinary and
the caller should receive `Option`.

## API

Trait surface:

```ari
convert::From[T]
convert::Into[T]
convert::TryFrom[T]
convert::TryInto[T]
```

Required trait methods:

```ari
fn From[T]::from(value: T) -> Self
fn Into[T]::into(self) -> T
fn TryFrom[T]::try_from(value: T) -> Option[Self]
fn TryInto[T]::try_into(self) -> Option[T]
```

Callable helpers:

```ari
convert::identity<T>(value)
convert::from<T, U>(value)
convert::into<T, U>(value)
convert::try_from<T, U>(value)
convert::try_into<T, U>(value)
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

`try_from<T, U>` calls `convert::TryFrom[T]` for destination type `U` and
returns `Option[U]`:

```ari
let maybe_value = convert::try_from<i64, CheckedByte>(input)
```

`try_into<T, U>` calls `convert::TryInto[T]` on source type `U` and returns
`Option[T]`:

```ari
let maybe_value = convert::try_into<CheckedByte, i64>(input)
```

The type argument order follows the trait relationship:

- `from<T, U>` means "build `U` from `T`"
- `into<T, U>` means "turn `U` into `T`"
- `try_from<T, U>` means "try to build `U` from `T`"
- `try_into<T, U>` means "try to turn `U` into `T`"

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

struct Percent {
  value: i64,
}

impl convert::TryFrom[i64] for Percent {
  fn try_from(value: i64) -> Option[Percent] {
    if value < 0 {
      return None<Percent>();
    }
    if value > 100 {
      return None<Percent>();
    }
    return Some<Percent>(Percent { value: value });
  }
}

impl convert::TryInto[Percent] for i64 {
  fn try_into(self) -> Option[Percent] {
    return convert::try_from<i64, Percent>(self);
  }
}

fn main() -> i64 {
  let a = convert::identity<i64>(7);
  let b = convert::from<i32, i64>(11 as i32);
  let c = convert::into<i64, i32>(13 as i32);
  let d = convert::try_from<i64, Percent>(99).unwrap().value;
  let e = convert::try_into<Percent, i64>(101).unwrap_or(Percent { value: 5 }).value;
  return a + b + c + d + e;
}
```

## Result Policy

Fallible conversion returns `Option` in this module. Use `Some(value)` when
the conversion is valid and `None<T>()` when the source value cannot be
represented. This keeps generic conversion lightweight for validation-style
cases such as checked numeric ranges, path/text boundary checks, and enum/raw
value reconstruction.

Use a domain-specific function returning `Result[T, E]` when callers need a
reason, position, errno, or other structured error. `std::convert` should stay
about the conversion relationship between two types, not about a particular
error taxonomy.

## Prelude Traits

The root prelude also exposes `From`, `Into`, `TryFrom`, and `TryInto` as
trait names with the same method shapes. The helpers on this page use the
`std::convert` trait paths, so implement `convert::From`, `convert::Into`,
`convert::TryFrom`, or `convert::TryInto` when calling the `std::convert`
helpers.

## Tests

The focused positive behavior tests are:

```text
tests/cases/standard-library/ok/convert/std-convert-value-helpers.ari
tests/cases/standard-library/ok/convert/std-convert-try-helpers.ari
```

`make check-prelude` emits LLVM for those files, checks representative public
helper symbols, builds executables, and verifies the exit codes. The public API
is tracked in `tests/std_api_manifest.txt` and checked by `make check-std-api`.

## Future Work

Potential next slices:

- unify root prelude and `std::convert` trait documentation once trait
  re-export policy is mature
- add conversion impl examples for library types such as slices, strings, and
  source vectors when ownership and allocation behavior is explicit
- add richer conversion families once type inference, associated output
  patterns, and ownership diagnostics can keep call sites short without hiding
  allocation or validation policy
