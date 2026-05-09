# Operators

## Arithmetic

```ari
a + b
a - b
a * b
a / b
a % b
```

Modulo requires integer operands.

Integer literals can carry an exact-width suffix:

```ari
let byte = 255u8
let delta = -12i16
let wide = 10i64
let mask = 0xffu8
let mode = 0o755
let bits = 0b1010
```

Supported integer suffixes are `i8`, `i16`, `i32`, `i64`, `u8`, `u16`,
`u32`, and `u64`. Suffixed literals are range-checked immediately. See
[Literals](literals.md) for base prefixes and string escape forms.

Float literals can carry a precision suffix:

```ari
let small = 1.0f32
let normal = 1.0e+1f64
```

`f32`, `f64`, and `f128` lower on the LLVM host backend as `float`, `double`,
and `fp128`. The freestanding backend can materialize local `f32`/`f64` literal
values, move them through raw `ptr f32`/`ptr f64` loads and stores, and lower
`f32`/`f64` arithmetic, ordered comparisons, width casts, and integer/float
casts. It still rejects calls and `f128`.

## Bit Operations

```ari
a & b
a | b
a ^ b
a << bits
a >> bits
```

Bit operations require integer operands. Shift results keep the left operand's
type.

## Assignment Shorthands

Mutable local bindings support arithmetic and bitwise compound assignment:

```ari
var total = 1
total += 2
total *= 3
total <<= 1
total ^= 4
```

Supported forms are `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`,
and `>>=`. They are checked like `name = name <op> value`, so the target must
be mutable and the computed value must be assignable back to the target type.

## Comparisons

```ari
a == b
a != b
a < b
a <= b
a > b
a >= b
```

Comparisons return `bool`.

Enum values in the lowered tagged-union subset support `==` and `!=`.

## Boolean Logic

```ari
ready && !failed
cached || compute()
```

`&&` and `||` short-circuit:

- the right side of `&&` runs only when the left side is true
- the right side of `||` runs only when the left side is false

Operands must be `bool`.

## Try Propagation

The postfix `?` operator works on non-generic, two-case enum values whose cases
look like `Maybe` or `Result`:

```ari
enum MaybeI32 {
  None,
  Some(i32),
}

fn bump(value: MaybeI32) -> MaybeI32 {
  let inner = value?
  return Some(inner + 1i32)
}
```

`Some`, `Ok`, and `Success` are treated as success cases and must carry one
payload. `None`, `Err`, `Error`, and `Failure` are treated as residual cases.
When the value is successful, `?` evaluates to the success payload. Otherwise
it immediately returns a residual value from the current function.

The current function can return the same enum type as the expression being
unwrapped, or another non-generic Maybe/Result-style enum with a compatible
residual case. This lets `ResultI32` convert into another result enum with a
different success payload as long as both residual cases carry the same payload
type, and lets no-payload `None`/`Failure` residuals convert across compatible
Maybe-style enums. Generic prelude `Option[T]`, its `Maybe[T]` alias, and
`Result[T, E]` are source `std` enums using `None`/`Some(T)` and
`Err(E)`/`Ok(T)` and can use `?` on the LLVM backend path. Aggregate-layout
enums such as `MaybeI64` can use `?` when the current function returns that
same enum type; cross-enum residual conversion for aggregate enum layouts is
still planned.

## Range Operators

Exclusive range syntax (`start..end`) and inclusive range syntax
(`start..=end`) produce prelude range values. They are supported directly in
`for` loops and inside `Slice[T]` indexing expressions such as
`view[start..end]`.

## Null Coalescing

The `??` operator works on the same non-generic, two-case enum shape as postfix
`?`, but it does not return early:

```ari
let value = maybe_value() ?? 0i32
```

If the left side is `Some`, `Ok`, or `Success`, the expression evaluates to the
success payload. If it is `None`, `Err`, `Error`, or `Failure`, Ari evaluates
the right side and uses that fallback value. The fallback must have the same
type as the success payload.

Generic prelude `Option[T]`, its `Maybe[T]` alias, and `Result[T, E]` values
use the same rule on the LLVM backend path.

Ari's current operator-sugar set is intentionally closed around compound
assignment, range syntax, postfix `?`, and `??`. Additional null/result
operators need a separate design pass rather than being reserved implicitly.

## Bitwise Not

Bitwise-not (`~`) is supported for integer values:

```ari
let inverted = ~0u8
```

## Precedence

The parser follows a conventional expression precedence order:

1. calls, paths, grouping
2. postfix `?`
3. unary operators such as `!` and unary `-`
4. casts with `as`
5. multiplication, division, modulo
6. addition and subtraction
7. shifts
8. comparisons
9. equality
10. bit `&`, `^`, `|`
11. logical `&&`
12. logical `||`
13. null coalescing `??`

Use parentheses when the grouping matters to readers.
