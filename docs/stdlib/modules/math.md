# std::math

`std::math` contains small source-only arithmetic helpers. The current APIs use
`i64` signatures because Ari does not yet have the numeric trait vocabulary
needed for one generic implementation across every integer type.

The public names intentionally avoid type suffixes. Call `math::gcd(left,
right)` instead of `gcd_i64` so existing code can keep the same spelling when
the helpers later grow into generic numeric APIs.

## API

Basic signed integer helpers:

```ari
math::abs(value)
math::sign(value)
math::is_positive(value)
math::is_negative(value)
math::is_zero(value)
math::is_even(value)
math::is_odd(value)
```

`abs` returns the non-negative magnitude of the input. `sign` returns `-1`,
`0`, or `1`. `is_positive`, `is_negative`, and `is_zero` are small predicate
forms for the same sign policy. The parity helpers use integer remainder and
work for negative values too. `abs` is the simple arithmetic spelling; use
`checked_abs` or `saturating_abs` below when the minimum `i64` value is a
possible input.

Overflow-policy helpers:

```ari
math::checked_add(left, right)
math::checked_sub(left, right)
math::checked_neg(value)
math::checked_abs(value)
math::wrapping_add(left, right)
math::overflowing_add(left, right)
math::saturating_add(left, right)
math::saturating_sub(left, right)
math::saturating_neg(value)
math::saturating_abs(value)
```

The `checked_*` helpers return `std::Option[i64]`: `Some(value)` when the
operation is representable, and `None<i64>()` when it would overflow or
underflow. The `saturating_*` helpers clamp to the nearest `i64` bound instead:
positive overflow becomes `9223372036854775807`, and negative overflow becomes
`-9223372036854775808`.

`wrapping_add` returns the two's-complement wrapped result. `overflowing_add`
returns an `Overflowing` value with `value()` and `overflowed()` accessors, so a
caller can keep the wrapped result and still branch on the overflow flag:

```ari
let sum = math::overflowing_add(left, right);
if sum.overflowed() {
  return sum.value();
}
```

This gives Ari source code documented spellings for integer edge cases before
the compiler grows dedicated overflow intrinsics for every integer width.

Power and divisor helpers:

```ari
math::pow(base, exponent)
math::div_floor(numerator, denominator)
math::div_ceil(numerator, denominator)
math::mod_floor(numerator, denominator)
math::gcd(left, right)
math::lcm(left, right)
```

`pow` multiplies in a source loop and asserts that `exponent >= 0`.
`div_floor` rounds signed division toward negative infinity, `div_ceil` rounds
toward positive infinity, and `mod_floor` returns the paired floor remainder:

```text
numerator == div_floor(numerator, denominator) * denominator + mod_floor(numerator, denominator)
```

The division helpers assert that `denominator != 0`. `gcd` normalizes negative
inputs through `abs` and returns the greatest common divisor. `lcm` also
normalizes negative inputs and returns `0` when either input is `0`.

## Limits

The checked, wrapping, overflowing, and saturating helpers define their `i64`
overflow behavior. Other helpers still use ordinary `i64` arithmetic
internally, so keep their inputs in a range where intermediate values are
meaningful for your program. Future slices should add checked multiplication
and then widen these natural names through numeric traits once the compiler has
a stronger intrinsic story.

Use `std::bits` for bit masks, rotations, power-of-two rounding, low-bit masks,
alignment helpers, and bit scans. Use plain operators for ordinary arithmetic
when no helper communicates intent better.

## Example

```ari
fn tile_score(width: i64, height: i64) -> i64 {
  let step = math::div_ceil(width, 8);
  let common = math::gcd(step, height);
  let repeat = math::lcm(step, height);
  if math::is_positive(width) && math::is_even(repeat) {
    return common + repeat;
  }
  return common;
}
```

## Tests

The focused behavior test is:

```text
tests/cases/standard-library/ok/math/std-math-integer-helpers.ari
tests/cases/standard-library/ok/math/std-math-division-rounding.ari
tests/cases/standard-library/ok/math/std-math-checked-saturating.ari
tests/cases/standard-library/ok/math/std-math-wrapping-overflowing.ari
```

`make check-prelude` emits LLVM for those files, checks the public helper
symbols, builds executables, and verifies the exit codes.
