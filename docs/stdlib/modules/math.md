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
math::is_even(value)
math::is_odd(value)
```

`abs` returns the non-negative magnitude of the input. `sign` returns `-1`,
`0`, or `1`. The parity helpers use integer remainder and work for negative
values too.

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

These helpers do not define overflow behavior yet. Keep inputs in a range where
the intermediate `i64` arithmetic is meaningful for your program.

Use `std::bits` for bit masks, rotations, power-of-two rounding, low-bit masks,
alignment helpers, and bit scans. Use plain operators for ordinary arithmetic
when no helper communicates intent better.

## Example

```ari
fn tile_score(width: i64, height: i64) -> i64 {
  let step = math::div_ceil(width, 8);
  let common = math::gcd(step, height);
  let repeat = math::lcm(step, height);
  if math::is_even(repeat) {
    return common + repeat;
  }
  return common;
}
```

## Tests

The focused behavior test is:

```text
tests/cases/standard-library/ok/std-math-integer-helpers.ari
tests/cases/standard-library/ok/std-math-division-rounding.ari
```

`make check-prelude` emits LLVM for those files, checks the public helper
symbols, builds executables, and verifies the exit codes.
