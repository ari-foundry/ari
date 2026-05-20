# std::parse

`std::parse` contains whole-input parsers for byte-oriented text. It builds on
`std::ascii` trimming and digit helpers, but gives application code natural
names for common value parsing: `integer`, `boolean`, and `float`.

Use this module when the entire `Slice[u8]` should be one value. Use
`std::ascii::parse_decimal_prefix` or `parse_hex_prefix` when a parser needs
to consume only a leading digit run and leave the rest of the bytes alone.

## API

```ari
parse::integer(bytes) -> Option[i64]
parse::is_integer(bytes) -> bool
parse::integer_or(bytes, fallback) -> i64
parse::integer_radix(bytes, radix) -> Option[i64]
parse::is_integer_radix(bytes, radix) -> bool
parse::integer_radix_or(bytes, radix, fallback) -> i64
parse::hex_integer(bytes) -> Option[i64]
parse::is_hex_integer(bytes) -> bool
parse::hex_integer_or(bytes, fallback) -> i64
parse::binary_integer(bytes) -> Option[i64]
parse::is_binary_integer(bytes) -> bool
parse::binary_integer_or(bytes, fallback) -> i64
parse::octal_integer(bytes) -> Option[i64]
parse::is_octal_integer(bytes) -> bool
parse::octal_integer_or(bytes, fallback) -> i64
parse::boolean(bytes) -> Option[bool]
parse::is_boolean(bytes) -> bool
parse::boolean_or(bytes, fallback) -> bool
parse::is_float(bytes) -> bool
parse::float_or(bytes, fallback) -> f64
parse::float(bytes) -> f64
```

`integer` trims ASCII whitespace, accepts an optional `+` or `-`, then requires
at least one decimal digit and no trailing garbage. It returns `None<i64>()`
for empty, sign-only, non-decimal input, or values outside the `i64` range.
`is_integer` is the validation-only form, and `integer_or` returns the parsed
value or the caller's fallback.

`integer_radix` parses the same signed whole-input shape in bases `2` through
`36`, using ASCII `0-9`, `A-Z`, and `a-z` digit spellings. It trims ASCII
whitespace and rejects invalid bases, sign-only values, invalid digits, and
trailing garbage with `None<i64>()`. Overflow is checked in the target radix:
`9223372036854775807` and `-9223372036854775808` are accepted in base 10, while
the next value on either side is rejected. It intentionally does not recognize
prefixes such as `0x` or `0b`; callers can strip those policies before calling
the parser. `hex_integer` and `binary_integer` are readable wrappers for bases
`16` and `2`, while `octal_integer` covers base `8` for permission bits and
other byte-oriented formats. All three wrappers have matching `is_*` and
`*_or` forms.

`boolean` trims ASCII whitespace and accepts only lowercase `true` or `false`.
It intentionally does not accept titlecase, uppercase, `1`, `0`, `yes`, or
`no`; those policies should be explicit at the call site. `is_boolean` checks
the same shape without exposing the parsed value, and `boolean_or` returns a
fallback when the input is not one of the accepted boolean spellings.

`is_float` validates the accepted decimal float shape without allocating:

```text
[digits][.digits][e[+|-]digits]
-[digits][.digits][e[+|-]digits]
digits[.digits][e[+|-]digits]
```

At least one digit is required before or after the decimal point. Exponents
accept `e` or `E`. Hex floats, `NaN`, `inf`, locale decimal separators, and
underscores are future policy work.

`float_or` returns the parsed `f64` or the caller's fallback for invalid input.
`float` is the asserting form: it panics if `is_float(bytes)` is false, then
returns the parsed `f64`.

## Why `float` Does Not Return `Option[f64]` Yet

The natural future API is `parse::float(bytes) -> Option[f64]` or
`parse::float(bytes) -> Result[f64, ParseError]`. Today's compiler cannot lower
`f64` enum payloads yet, so `Option[f64]` is not a safe public API. The current
split keeps user code readable now:

- use `is_float` when validation matters
- use `float_or` when invalid input should choose a fallback
- use `float` when invalid input is a programmer error

When enum payload lowering supports floats, this page should be updated before
changing the public API.

## Example

```ari
fn read_count(bytes: Slice[u8]) -> i64 {
  return parse::integer_or(bytes, 0);
}

fn read_hex_color(bytes: Slice[u8]) -> i64 {
  return parse::hex_integer_or(bytes, 0);
}

fn read_mode(bytes: Slice[u8]) -> i64 {
  return parse::octal_integer_or(bytes, 420); // 0644
}

fn read_flag(bytes: Slice[u8]) -> bool {
  return parse::boolean_or(bytes, false);
}

fn read_ratio(bytes: Slice[u8]) -> f64 {
  return parse::float_or(bytes, 1.0f64);
}
```

## Tests

```text
tests/cases/standard-library/ok/parse/std-parse-basic.ari
```

The focused test covers ASCII-trimmed signed integer parsing, radix wrappers
for binary, octal, and hexadecimal input, boolean parsing, float validation,
float conversion, and invalid whole-input cases. It is wired into
`make check-prelude` with LLVM symbol checks for the public helpers.

## Future Work

- `ParseError` once richer error values are worth the API weight
- `Option[f64]` or `Result[f64, E]` after float enum payloads are supported
