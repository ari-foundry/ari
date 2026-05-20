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
math::min_value()
math::max_value()
math::abs(value)
math::sign(value)
math::is_positive(value)
math::is_negative(value)
math::is_zero(value)
math::is_even(value)
math::is_odd(value)
```

`min_value` and `max_value` return the current signed `i64` lower and upper
bounds. They keep user code from spelling long sentinel literals by hand while
the module is still `i64`-only. `abs` returns the non-negative magnitude of the
input. `sign` returns `-1`, `0`, or `1`. `is_positive`, `is_negative`, and
`is_zero` are small predicate forms for the same sign policy. The parity
helpers use integer remainder and work for negative values too. `abs` is the
simple arithmetic spelling; use `checked_abs` or `saturating_abs` below when
the minimum value is a possible input.

Overflow-policy helpers:

```ari
math::checked_add(left, right)
math::checked_sub(left, right)
math::checked_mul(left, right)
math::checked_div(left, right)
math::checked_rem(left, right)
math::checked_neg(value)
math::checked_abs(value)
math::wrapping_add(left, right)
math::wrapping_sub(left, right)
math::wrapping_mul(left, right)
math::overflowing_add(left, right)
math::overflowing_sub(left, right)
math::overflowing_mul(left, right)
math::saturating_add(left, right)
math::saturating_sub(left, right)
math::saturating_mul(left, right)
math::saturating_div(left, right)
math::saturating_neg(value)
math::saturating_abs(value)
```

The `checked_*` helpers return `std::Option[i64]`: `Some(value)` when the
operation is representable, and `None<i64>()` when it would overflow or
underflow. The `saturating_*` helpers clamp to the nearest `i64` bound instead:
positive overflow becomes `max_value()`, and negative overflow becomes
`min_value()`.

`checked_mul` and `saturating_mul` follow the same policy for multiplication,
using guarded division checks before multiplying so the successful branch does
not rely on signed overflow behavior. `checked_div` and `checked_rem` return
`None<i64>()` for division by zero and the unrepresentable
`min_value() / -1` edge. `saturating_div` asserts that the divisor is non-zero
and clamps only that signed-minimum overflow edge to `max_value()`.

`wrapping_add`, `wrapping_sub`, and `wrapping_mul` return the
two's-complement wrapped result. `wrapping_mul` routes through `u64` so the
source code says "modulo 2^64" directly instead of depending on signed
multiplication overflow. `overflowing_add`, `overflowing_sub`, and
`overflowing_mul` return `(value, overflowed)`, a tuple whose first slot is the
wrapped result and whose second slot is the overflow flag:

```ari
let (sum, overflowed) = math::overflowing_add(left, right);
if overflowed {
  return sum;
}
```

This deliberately uses a tuple because the operation always produces both
pieces. `checked_*` still uses `Option[i64]` because the value may be absent.
Together they give Ari source code documented spellings for integer edge cases
before the compiler grows dedicated overflow intrinsics for every integer width.

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

The division helpers assert that `denominator != 0` and that
`numerator / denominator` is representable. `gcd` normalizes negative inputs
through `abs` and returns the greatest common divisor. `lcm` also normalizes
negative inputs and returns `0` when either input is `0`.

## Limits

The checked, wrapping, overflowing, and saturating helpers define their `i64`
overflow behavior. Other helpers still use ordinary `i64` arithmetic
internally, so keep their inputs in a range where intermediate values are
meaningful for your program. The existing natural names should widen through
numeric traits once the compiler has a stronger intrinsic story for every
integer width.

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
