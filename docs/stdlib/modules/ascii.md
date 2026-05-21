# std::ascii

`std::ascii` contains source-only helpers for code that treats text as ASCII
bytes. It exists because today's `std::string::String` is a byte string, not a
full Unicode text abstraction. Keeping these helpers in a separate module makes
that policy visible at every call site.

## When To Use It

Use `std::ascii` when you have a `u8`, a string literal, or `Slice[u8]` from a
`String`, raw byte input, or a byte-oriented parser and you need simple ASCII
classification, case conversion, comparison, substring search, trimming, or
small integer parsing.

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
ascii::is_blank(byte)
ascii::is_whitespace(byte)
ascii::is_control(byte)
ascii::is_printable(byte)
ascii::is_graphic(byte)
ascii::is_punctuation(byte)
ascii::is_hex_digit(byte)
```

`is_blank` accepts `' '` and `'\t'`. `is_whitespace` accepts `' '`, `'\t'`,
`'\n'`, and `'\r'`. `is_control` accepts `'\x00'..'\x1f'` and `'\x7f'`.
`is_printable` accepts `' '..'~'`, including space. `is_graphic` accepts
`'!'..'~'`, excluding space. `is_punctuation` accepts printable graphic bytes
that are not ASCII letters or digits. Prefer byte character literals like
`'0'`, `'A'`, and `'\n'` over decimal byte casts at call sites.

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

`digit_value` accepts `'0'` through `'9'`. `hex_value` accepts `'0'` through
`'9'`, `'a'` through `'f'`, and `'A'` through `'F'`.

Prefix parsers return a small named result type:

```ari
ascii::ParsedInt
```

Slice helpers operate on borrowed `Slice[u8]` values. A string literal can be
passed directly anywhere a `Slice[u8]` is expected; Ari lowers it to a borrowed
view over the literal bytes without the trailing NUL. The same literal can
initialize local `Vec[u8]` and `[u8, N]` storage when owned local bytes are more
convenient.

```ari
ascii::equals_ignore_case(left, right)
ascii::starts_with_ignore_case(bytes, prefix)
ascii::ends_with_ignore_case(bytes, suffix)
ascii::index_of_ignore_case(bytes, needle)
ascii::contains_ignore_case(bytes, needle)
ascii::skip_whitespace(bytes)
ascii::trim_start(bytes)
ascii::trim_end(bytes)
ascii::trim(bytes)
ascii::parse_decimal(bytes)
ascii::parse_decimal_prefix(bytes)
ascii::parse_signed_decimal(bytes)
ascii::parse_signed_decimal_prefix(bytes)
ascii::parse_hex(bytes)
ascii::parse_hex_prefix(bytes)
```

For example:

```ari
ascii::parse_decimal("123")
ascii::starts_with_ignore_case("AriLang", "ari")
```

The `*_ignore_case` helpers compare only ASCII letter case. Bytes outside
`A..Z` and `a..z` compare by exact byte value after the same `to_lower`
conversion used by the scalar helpers. Empty prefixes, suffixes, and search
needles match. `index_of_ignore_case` returns the first matching byte offset,
or `-1` when the needle is not found; `contains_ignore_case` is the boolean
form of that search.

`skip_whitespace` returns the first non-whitespace byte index or `bytes.len`
when the slice is all whitespace. The trim helpers return borrowed sub-slices;
they do not allocate or copy. `parse_decimal`, `parse_signed_decimal`, and
`parse_hex` parse the entire slice and return `None<i64>()` for empty input or
invalid bytes. `parse_signed_decimal` accepts one optional leading `+` or `-`
and then requires at least one digit. These helpers do not trim whitespace;
call `ascii::trim` first when that policy is wanted. Decimal, signed decimal,
and hexadecimal parsers return `None` instead of wrapping when the digit run
exceeds the `i64` range. Signed decimal parsing accepts `-9223372036854775808`
as the minimum `i64` value.

`ParsedInt` is the result shape for prefix parsers:

```ari
let parsed = ascii::parse_decimal_prefix(bytes).unwrap();
let value = parsed.value;
let consumed = parsed.len;
```

`parse_decimal_prefix`, `parse_signed_decimal_prefix`, and `parse_hex_prefix`
parse the leading digit run and stop before the first invalid byte. They return
`None<ParsedInt>()` when the first byte is missing or invalid. The signed
prefix parser accepts one leading `+` or `-`, requires a digit after it, and
counts the sign in `ParsedInt.len`. They do not trim whitespace, recognize
`0x`, or return partial values after overflow. A digit run that exceeds `i64`
returns `None<ParsedInt>()` even when later bytes would otherwise stop the
prefix parser.

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

fn has_ari_prefix(bytes: Slice[u8]) -> bool {
  var prefix: Vec[u8] = ['A', 'R', 'I'];
  return ascii::starts_with_ignore_case(bytes, prefix.as_slice());
}

fn find_lib(bytes: Slice[u8]) -> i64 {
  var needle: Vec[u8] = ['l', 'i', 'b'];
  return ascii::index_of_ignore_case(bytes, needle.as_slice());
}

fn read_prefix(bytes: Slice[u8]) -> i64 {
  match ascii::parse_decimal_prefix(bytes) {
    std::Some(parsed) => parsed.value + parsed.len,
    std::None => 0,
  }
}
```

## Tests

The focused positive tests are:

```text
tests/cases/standard-library/ok/ascii/std-ascii-byte-helpers.ari
tests/cases/standard-library/ok/ascii/std-ascii-class-helpers.ari
tests/cases/standard-library/ok/ascii/std-ascii-slice-helpers.ari
tests/cases/standard-library/ok/ascii/std-ascii-prefix-parsers.ari
tests/cases/standard-library/ok/ascii/std-ascii-signed-parsers.ari
tests/cases/standard-library/ok/ascii/std-ascii-overflow-parsers.ari
tests/cases/standard-library/ok/ascii/std-ascii-case-compare.ari
tests/cases/standard-library/ok/ascii/std-ascii-case-search.ari
```

`make check-prelude` compiles them to LLVM, checks representative public
symbols, links them through the LLVM backend, and verifies executable results.

The public API is tracked in `tests/std_api_manifest.txt` and checked by
`make check-std-api`.

## Future Work

Potential next slices:

- byte-window helpers after collection substring policy grows beyond
  first-match search
- a separate text/Unicode module after Ari has a deliberate text policy
