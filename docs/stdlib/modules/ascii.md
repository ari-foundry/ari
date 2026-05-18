# std::ascii

`std::ascii` contains source-only helpers for code that treats text as ASCII
bytes. It exists because today's `std::string::String` is a byte string, not a
full Unicode text abstraction. Keeping these helpers in a separate module makes
that policy visible at every call site.

## When To Use It

Use `std::ascii` when you have a `u8` or `Slice[u8]` from a `String`, raw byte
input, or a byte-oriented parser and you need simple ASCII classification,
case conversion, trimming, or small integer parsing.

Do not use it as a Unicode or locale-aware text API. Those policies are future
library work and should not be hidden behind ASCII helper names.

## API

Classification helpers return `bool`:

```ari
ascii::is_digit(byte)
ascii::is_lower(byte)
ascii::is_upper(byte)
ascii::is_alpha(byte)
ascii::is_alphanumeric(byte)
ascii::is_whitespace(byte)
ascii::is_hex_digit(byte)
```

Case helpers return the converted byte when the input is an ASCII letter and
return the original byte otherwise:

```ari
ascii::to_lower(byte)
ascii::to_upper(byte)
```

Digit helpers return `Option[i64]`:

```ari
ascii::digit_value(byte)
ascii::hex_value(byte)
```

`digit_value` accepts `0` through `9`. `hex_value` accepts `0` through `9`,
`a` through `f`, and `A` through `F`.

Slice helpers operate on borrowed `Slice[u8]` values:

```ari
ascii::skip_whitespace(bytes)
ascii::trim_start(bytes)
ascii::trim_end(bytes)
ascii::trim(bytes)
ascii::parse_decimal(bytes)
ascii::parse_hex(bytes)
```

`skip_whitespace` returns the first non-whitespace byte index or `bytes.len`
when the slice is all whitespace. The trim helpers return borrowed sub-slices;
they do not allocate or copy. `parse_decimal` and `parse_hex` parse the entire
slice and return `None<i64>()` for empty input or invalid bytes. Overflow
behavior is not promised yet.

## Example

```ari
fn score(byte: u8) -> i64 {
  if ascii::is_digit(byte) {
    return ascii::digit_value(byte).unwrap_or(0);
  }
  if ascii::is_alpha(byte) {
    return ascii::to_lower(byte) as i64;
  }
  return 0;
}

fn parse_score(bytes: Slice[u8]) -> i64 {
  let trimmed = ascii::trim(bytes);
  return ascii::parse_decimal(trimmed).unwrap_or(0);
}
```

## Tests

The focused positive test is:

```text
tests/cases/standard-library/ok/std-ascii-byte-helpers.ari
tests/cases/standard-library/ok/std-ascii-slice-helpers.ari
```

`make check-prelude` compiles them to LLVM, checks representative public
symbols, links them through the LLVM backend, and verifies executable results.

The public API is tracked in `tests/std_api_manifest.txt` and checked by
`make check-std-api`.

## Future Work

Potential next slices:

- prefix parsers that return both the parsed value and consumed byte count
- signed decimal parsing after the numeric overflow policy is documented
- a separate text/Unicode module after Ari has a deliberate text policy
