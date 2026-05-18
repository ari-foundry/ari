# std::bits

`std::bits` contains source-only helpers for bit-mask code, power-of-two
alignment, and bit scans. The current slices use `u64` signatures because Ari
does not yet have the numeric trait vocabulary needed to express one generic
integer API.

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
```

Power-of-two helpers:

```ari
bits::is_power_of_two(value)
bits::align_down(value, alignment)
bits::align_up(value, alignment)
```

`align_down` and `align_up` assert that `alignment` is a non-zero power of two.
They do not define overflow behavior for `value + alignment - 1`; checked and
wrapping variants belong in a later numeric policy slice.

Bit scan helpers:

```ari
bits::count_ones(value)
bits::count_zeros(value)
bits::leading_zeros(value)
bits::trailing_zeros(value)
```

`count_ones` and `count_zeros` count set and unset bits across the whole
64-bit value. `leading_zeros` and `trailing_zeros` return `64` for `0u64`.
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
tests/cases/standard-library/ok/std-bits-mask-helpers.ari
tests/cases/standard-library/ok/std-bits-scan-helpers.ari
```

`make check-prelude` compiles them to LLVM, checks representative public
symbols, links them through the LLVM backend, and verifies executable results.

The public API is tracked in `tests/std_api_manifest.txt` and checked by
`make check-std-api`.

## Future Work

Potential next slices:

- checked, wrapping, and saturating arithmetic helpers after overflow policy is
  documented
- generic integer traits so `u8`, `u16`, `u32`, `u64`, and signed variants can
  share the same public names
- optional intrinsic-backed implementations for the existing bit scan helpers
  after the source contract and generic integer policy are stable
