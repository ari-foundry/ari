# std::bits

`std::bits` contains source-only helpers for bit-mask code, rotations,
power-of-two rounding, low-bit masks, alignment, and zero/one-run bit scans.
The current slices use `u64` signatures because Ari does not yet have the
numeric trait vocabulary needed to express one generic integer API.

The names are intentionally natural. When generic numeric traits are ready,
the library should grow these names instead of adding public type suffixes.

## When To Use It

Use `std::bits` for flag masks, capability bits, low-level layout code, and
manual alignment or bit inspection where the value is already an unsigned
64-bit integer.

Use `std::math` for ordinary arithmetic helpers. Use `std::mem::align_of<T>()`
when you need the compiler-known alignment of an Ari type.

## API

Mask predicates:

```ari
bits::is_set(value, mask)
bits::any_set(value, mask)
```

`is_set` returns true when every bit in `mask` is present in `value`.
`any_set` returns true when at least one bit overlaps.

Mask transforms:

```ari
bits::set(value, mask)
bits::clear(value, mask)
bits::toggle(value, mask)
bits::rotate_left(value, count)
bits::rotate_right(value, count)
```

`rotate_left` and `rotate_right` assert that `count` is non-negative and rotate
modulo 64, so a count of `64` returns the original value and `65` behaves like
`1`.

Power-of-two helpers:

```ari
bits::is_power_of_two(value)
bits::bit_width(value)
bits::floor_power_of_two(value)
bits::ceil_power_of_two(value)
bits::low_mask(width)
bits::align_down(value, alignment)
bits::align_up(value, alignment)
```

`bit_width` returns the number of bits needed to represent `value`, with `0`
returning `0`. `floor_power_of_two` returns the largest power of two not above
`value`, with `0` returning `0`. `ceil_power_of_two` returns the smallest power
of two not below `value`, with `0` and `1` returning `1`; it asserts when the
next power would overflow `u64`. `low_mask(width)` returns the lowest `width`
bits set and asserts that `width` is between `0` and `64`.

`align_down` and `align_up` assert that `alignment` is a non-zero power of two.
They do not define overflow behavior for `value + alignment - 1`; a future
numeric-policy slice should add `u64` checked/wrapping alignment variants after
the first `i64` checked/wrapping/saturating helpers in `std::math`.

Bit scan and byte-order helpers:

```ari
bits::count_ones(value)
bits::population_count(value)
bits::count_zeros(value)
bits::byte_swap(value)
bits::leading_zeros(value)
bits::trailing_zeros(value)
bits::leading_ones(value)
bits::trailing_ones(value)
```

`count_ones` and `count_zeros` count set and unset bits across the whole
64-bit value. `population_count` is the same operation with the more explicit
hardware/algorithm name, so code can use whichever spelling reads best.
`byte_swap` reverses the eight bytes in a `u64` and is useful for endian-aware
codecs or binary file formats. `leading_zeros` and `trailing_zeros` return
`64` for `0u64`. `leading_ones` and `trailing_ones` count contiguous one bits
from the high or low end; they return `0` for `0u64` and `64` for `~0u64`.
The current implementation is a straightforward Ari source loop; future
intrinsic-backed lowering should preserve the same edge-case behavior.

## Example

```ari
const READ: u64 = 0b0001u64;
const WRITE: u64 = 0b0010u64;

fn enable_write(flags: u64) -> u64 {
  if bits::is_set(flags, READ) {
    return bits::set(flags, WRITE);
  }
  return flags;
}
```

## Tests

The focused positive tests are:

```text
tests/cases/standard-library/ok/bits/std-bits-mask-helpers.ari
tests/cases/standard-library/ok/bits/std-bits-rotate-helpers.ari
tests/cases/standard-library/ok/bits/std-bits-scan-helpers.ari
tests/cases/standard-library/ok/bits/std-bits-one-run-helpers.ari
tests/cases/standard-library/ok/bits/std-bits-width-helpers.ari
tests/cases/standard-library/ok/bits/std-bits-byte-population.ari
```

`make check-prelude` compiles them to LLVM, checks representative public
symbols, links them through the LLVM backend, and verifies executable results.

The public API is tracked in `tests/std_api_manifest.txt` and checked by
`make check-std-api`.

## Future Work

Potential next slices:

- checked/wrapping alignment variants after unsigned overflow policy is
  documented
- generic integer traits so `u8`, `u16`, `u32`, `u64`, and signed variants can
  share the same public names
- optional intrinsic-backed implementations for the existing bit scan,
  population count, and byte-swap helpers after the source contract and generic
  integer policy are stable
