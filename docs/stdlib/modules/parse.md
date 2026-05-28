# std::parse

`std::parse` contains whole-input parsers for byte-oriented text. It builds on
`std::ascii` trimming and digit helpers, but gives application code natural
names for common value parsing: `integer`, `boolean`, and `float`.

Use this module when the entire `Slice[u8]` should be one value. Use
`std::ascii::parse_decimal_prefix` or `parse_hex_prefix` when a parser needs
to consume only a leading digit run and leave the rest of the bytes alone.
For call sites that should read like typed parsing, use the `Parse` trait entry
points: `parse::parse[T](bytes)`, `parse::parse_or[T](bytes, fallback)`, and
`parse::is_parse[T](bytes)`.

The integer, boolean, and float families use the natural fallible names for
`Result[..., std::error::Error]` APIs. The `_optional` helpers keep the compact
compatibility surface for validation-style call sites that do not need an error
reason, and `_or` helpers keep the explicit fallback pattern. Invalid radices
return `InvalidInput`; syntactically invalid, empty, or out-of-range values
return `InvalidData`.

For callers that need precise diagnostics without changing the default
`Result[T, std::error::Error]` surface, the integer and float families expose
`*_error` helpers. They return `Option[ParseError]`: `None` means the same
input would parse successfully, and `Some(error)` carries a `ParseErrorKind`
plus the byte offset in the ASCII-trimmed input where the diagnostic was
detected.

Like other public text APIs, `ParseError.name(ref mut zone)` and
`ParseError.message(ref mut zone)` return owned `String` values. Use
`name_text()` and `message_text()` only when raw borrowed labels are the desired
compatibility form.

## API

```ari
trait Parse
parse::ParseError
parse::ParseErrorKind
parse_error.kind() -> ParseErrorKind
parse_error.offset() -> i64
parse_error.name(ref mut Zone) -> String
parse_error.message(ref mut Zone) -> String
parse_error.name_text() -> string
parse_error.message_text() -> string
parse_error.is_empty_input() -> bool
parse_error.is_expected_digit() -> bool
parse_error.is_invalid_radix() -> bool
parse_error.is_invalid_digit() -> bool
parse_error.is_invalid_sign() -> bool
parse_error.is_invalid_separator() -> bool
parse_error.is_overflow() -> bool
parse_error.is_underflow() -> bool
parse::parse[T: Parse](bytes) -> T
parse::parse_or[T: Parse](bytes, fallback) -> T
parse::is_parse[T: Parse](bytes) -> bool
parse::integer(bytes) -> Result[i64, std::error::Error]
parse::integer_error(bytes) -> Option[ParseError]
parse::integer_optional(bytes) -> Option[i64]
parse::is_integer(bytes) -> bool
parse::integer_or(bytes, fallback) -> i64
parse::integer_with_underscores(bytes) -> Result[i64, std::error::Error]
parse::integer_with_underscores_error(bytes) -> Option[ParseError]
parse::integer_with_underscores_optional(bytes) -> Option[i64]
parse::is_integer_with_underscores(bytes) -> bool
parse::integer_with_underscores_or(bytes, fallback) -> i64
parse::integer_radix(bytes, radix) -> Result[i64, std::error::Error]
parse::integer_radix_error(bytes, radix) -> Option[ParseError]
parse::integer_radix_optional(bytes, radix) -> Option[i64]
parse::is_integer_radix(bytes, radix) -> bool
parse::integer_radix_or(bytes, radix, fallback) -> i64
parse::integer_radix_with_underscores(bytes, radix) -> Result[i64, std::error::Error]
parse::integer_radix_with_underscores_error(bytes, radix) -> Option[ParseError]
parse::integer_radix_with_underscores_optional(bytes, radix) -> Option[i64]
parse::is_integer_radix_with_underscores(bytes, radix) -> bool
parse::integer_radix_with_underscores_or(bytes, radix, fallback) -> i64
parse::unsigned(bytes) -> Result[u64, std::error::Error]
parse::unsigned_error(bytes) -> Option[ParseError]
parse::unsigned_optional(bytes) -> Option[u64]
parse::is_unsigned(bytes) -> bool
parse::unsigned_or(bytes, fallback) -> u64
parse::unsigned_with_underscores(bytes) -> Result[u64, std::error::Error]
parse::unsigned_with_underscores_error(bytes) -> Option[ParseError]
parse::unsigned_with_underscores_optional(bytes) -> Option[u64]
parse::is_unsigned_with_underscores(bytes) -> bool
parse::unsigned_with_underscores_or(bytes, fallback) -> u64
parse::unsigned_radix(bytes, radix) -> Result[u64, std::error::Error]
parse::unsigned_radix_error(bytes, radix) -> Option[ParseError]
parse::unsigned_radix_optional(bytes, radix) -> Option[u64]
parse::is_unsigned_radix(bytes, radix) -> bool
parse::unsigned_radix_or(bytes, radix, fallback) -> u64
parse::unsigned_radix_with_underscores(bytes, radix) -> Result[u64, std::error::Error]
parse::unsigned_radix_with_underscores_error(bytes, radix) -> Option[ParseError]
parse::unsigned_radix_with_underscores_optional(bytes, radix) -> Option[u64]
parse::is_unsigned_radix_with_underscores(bytes, radix) -> bool
parse::unsigned_radix_with_underscores_or(bytes, radix, fallback) -> u64
parse::hex_integer(bytes) -> Result[i64, std::error::Error]
parse::hex_integer_optional(bytes) -> Option[i64]
parse::is_hex_integer(bytes) -> bool
parse::hex_integer_or(bytes, fallback) -> i64
parse::hex_integer_with_underscores(bytes) -> Result[i64, std::error::Error]
parse::hex_integer_with_underscores_optional(bytes) -> Option[i64]
parse::is_hex_integer_with_underscores(bytes) -> bool
parse::hex_integer_with_underscores_or(bytes, fallback) -> i64
parse::binary_integer(bytes) -> Result[i64, std::error::Error]
parse::binary_integer_optional(bytes) -> Option[i64]
parse::is_binary_integer(bytes) -> bool
parse::binary_integer_or(bytes, fallback) -> i64
parse::binary_integer_with_underscores(bytes) -> Result[i64, std::error::Error]
parse::binary_integer_with_underscores_optional(bytes) -> Option[i64]
parse::is_binary_integer_with_underscores(bytes) -> bool
parse::binary_integer_with_underscores_or(bytes, fallback) -> i64
parse::octal_integer(bytes) -> Result[i64, std::error::Error]
parse::octal_integer_optional(bytes) -> Option[i64]
parse::is_octal_integer(bytes) -> bool
parse::octal_integer_or(bytes, fallback) -> i64
parse::octal_integer_with_underscores(bytes) -> Result[i64, std::error::Error]
parse::octal_integer_with_underscores_optional(bytes) -> Option[i64]
parse::is_octal_integer_with_underscores(bytes) -> bool
parse::octal_integer_with_underscores_or(bytes, fallback) -> i64
parse::boolean(bytes) -> Result[bool, std::error::Error]
parse::boolean_optional(bytes) -> Option[bool]
parse::is_boolean(bytes) -> bool
parse::boolean_or(bytes, fallback) -> bool
parse::is_float(bytes) -> bool
parse::float(bytes) -> Result[f64, std::error::Error]
parse::float_error(bytes) -> Option[ParseError]
parse::float_optional(bytes) -> Option[f64]
parse::float_or(bytes, fallback) -> f64
parse::is_float_with_underscores(bytes) -> bool
parse::float_with_underscores(bytes) -> Result[f64, std::error::Error]
parse::float_with_underscores_error(bytes) -> Option[ParseError]
parse::float_with_underscores_optional(bytes) -> Option[f64]
parse::float_with_underscores_or(bytes, fallback) -> f64
parse::float_unchecked(bytes) -> f64
```

`integer` trims ASCII whitespace, accepts an optional `+` or `-`, then requires
at least one decimal digit and no trailing garbage. It returns
`Result[i64, Error]`, with `InvalidData` for empty input, invalid digits, or
overflow. `integer_optional` is the compatibility form that returns
`None<i64>()` for those failures, `is_integer` is the validation-only form, and
`integer_or` returns the parsed value or the caller's fallback.

`integer_radix` parses the same signed whole-input shape in bases `2` through
`36`, using ASCII `0-9`, `A-Z`, and `a-z` digit spellings. It trims ASCII
whitespace and returns `InvalidInput` for invalid bases and `InvalidData` for
sign-only values, invalid digits, trailing garbage, or overflow. Overflow is
checked in the target radix: `9223372036854775807` and
`-9223372036854775808` are accepted in base 10, while the next value on either
side is rejected. It intentionally does not recognize prefixes such as `0x` or
`0b`; callers can strip those policies before calling the parser.
`integer_radix_optional` discards those error details. `hex_integer`,
`binary_integer`, and `octal_integer` are readable wrappers for bases `16`,
`2`, and `8`. All three wrappers have matching `_optional`, `is_*`, and
`*_or` forms.

`integer_error` and `integer_radix_error` are diagnostic helpers for callers
that need to explain a parse failure. They do not replace the natural
Result-returning parser; instead, they return `None` for valid input and
`Some(ParseError)` for invalid input. `ParseError::kind()` returns one of:
`EmptyInput`, `ExpectedDigit`, `InvalidRadix`, `InvalidDigit`,
`InvalidSign`, `InvalidSeparator`, `Overflow`, or `Underflow`.
`ParseError::offset()` is the byte offset in the trimmed input where that
condition was detected. For example, `12x` reports `InvalidDigit` at offset
`2`, an invalid radix reports `InvalidRadix` at offset `0`, signed integer
overflow reports `Overflow` at the first digit that made the value exceed
`i64`, and extreme decimal float exponents report the exponent digit that moved
the value outside Ari's accepted finite `f64` range.
`ParseError::name()` returns a stable, short lowercase label such as
`"invalid digit"` or `"overflow"`, while `ParseError::message()` returns a
one-sentence explanation suitable for CLI diagnostics. The `is_*` predicates
mirror every `ParseErrorKind` so callers can branch without spelling a `match`
when they only care about one class of parse failure.

Default integer parsers are strict and reject underscores. Use the explicit
`*_with_underscores` family for human-edited configuration values such as
`1_000`, `ff_ff`, or `1010_0101`. Underscores are accepted only between two
valid digits in the chosen radix. Leading, trailing, repeated, sign-adjacent,
prefix-adjacent, and suffix-adjacent underscores are rejected:
`_1`, `1_`, `1__0`, `+_1`, and `1_e2` are invalid. The underscore-aware
integer helpers keep the same overflow checks and the same `InvalidInput` versus
`InvalidData` split as the strict radix helpers, and they have matching
`_optional`, `is_*`, and `*_or` forms.
`integer_with_underscores_error` and
`integer_radix_with_underscores_error` keep those same separator rules but
return `InvalidSeparator` for separator placement failures, which makes config
file diagnostics more precise than the broad `InvalidData` category.

`unsigned` and `unsigned_radix` mirror the signed integer parsers for `u64`.
They trim ASCII whitespace, accept an optional leading `+`, reject `-`, and
reject values larger than `18446744073709551615`. Use them for ids, bitmasks,
wire-format counters, and other fields where negative numbers are not valid.
They follow the same `Result` policy as the signed parsers, with `_optional`
forms for callers that only need presence or absence.
`unsigned_error`, `unsigned_radix_error`, and their underscore-aware variants
return the same `ParseError` shape; unsigned-only diagnostics additionally use
`InvalidSign` at offset `0` for a leading `-`.

`boolean` trims ASCII whitespace and accepts only lowercase `true` or `false`.
It intentionally does not accept titlecase, uppercase, `1`, `0`, `yes`, or
`no`; those policies should be explicit at the call site. `is_boolean` checks
the same shape without exposing the parsed value, and `boolean_or` returns a
fallback when the input is not one of the accepted boolean spellings.
`boolean` returns `Err(Error(InvalidData))` for every unsupported spelling;
`boolean_optional` preserves the older `Option[bool]` shape.

`is_float` validates the accepted decimal float shape without allocating:

```text
[digits][.digits][e[+|-]digits]
-[digits][.digits][e[+|-]digits]
digits[.digits][e[+|-]digits]
```

At least one digit is required before or after the decimal point. Exponents
accept `e` or `E`. Hex floats, `NaN`, `inf`, and locale decimal separators are
future policy work.

`float` returns `Result[f64, Error]` and reports `InvalidData` for empty input,
unsupported spelling, trailing garbage, overflow, or underflow outside Ari's
accepted finite nonzero `f64` range. `float_error` is the diagnostic helper for
the same strict spelling. It returns `EmptyInput` for empty input,
`ExpectedDigit` for sign-only input, a bare `.`, or a missing exponent digit,
`InvalidSeparator` for `_` in the default strict parser, `InvalidDigit` for
other trailing or unsupported bytes, `Overflow` for values whose decimal
spelling is above the maximum finite `f64`, and `Underflow` for nonzero decimal
spellings below the minimum subnormal boundary. Range diagnostics use the byte
offset of the exponent digit that crossed the broad exponent boundary when an
exponent is present; when the exponent is exactly at the edge, they report the
mantissa digit that crosses `1.7976931348623157e308` or the first significant
digit below the `5e-324` subnormal boundary. `float_optional` is the compact
compatibility form for validation-style callers, `float_or` returns the parsed
`f64` or the caller's fallback, and `float_unchecked` preserves the old
asserting behavior by panicking when `is_float(bytes)` is false.

Default float parsing also rejects underscores. Use
`float_with_underscores`, `float_with_underscores_optional`,
`is_float_with_underscores`, or `float_with_underscores_or` when a decimal
float may contain digit separators. The underscore rule is the same as for
integers: `_` must sit between two decimal digits within the integer, fraction,
or exponent digit run. Examples such as `1_000.5_0e1_2` and `.2_5` are valid;
`1_.0`, `1._0`, `1e_2`, and `1__0` are invalid.
`float_with_underscores_error` validates that same separator-aware policy and
returns `InvalidSeparator` at the separator that made the spelling invalid.
It uses the same `Overflow` and `Underflow` range diagnostics as strict
`float_error` after ignoring valid digit separators.

The `Parse` trait gives generic code one spelling for common built-in types:

```ari
let count = parse::parse[i64]("42");
let size = parse::parse[u64]("18446744073709551615");
let enabled = parse::parse[bool]("true");
let ratio = parse::parse[f64]("1.25e2");
let fallback = parse::parse_or[i64]("not an int", 0);
```

The trait-backed `parse` function is the asserting form for every implemented
type. Use `parse_or` or `is_parse` when invalid input is ordinary. `Parse` is
currently implemented for `i64`, `u64`, `bool`, and `f64`; user types can add
their own impls when they have a stable parsing policy.

## Example

```ari
fn read_count(bytes: Slice[u8]) -> i64 {
  return parse::integer_or(bytes, 0);
}

fn read_size(bytes: Slice[u8]) -> u64 {
  return parse::unsigned_or(bytes, 0u64);
}

fn read_hex_color(bytes: Slice[u8]) -> i64 {
  return parse::hex_integer_with_underscores_or(bytes, 0);
}

fn read_mode(bytes: Slice[u8]) -> i64 {
  return parse::octal_integer_or(bytes, 420); // 0644
}

fn read_flag(bytes: Slice[u8]) -> bool {
  return parse::boolean_or(bytes, false);
}

fn require_count(bytes: Slice[u8]) -> Result[i64, Error] {
  return parse::integer(bytes);
}

fn require_ratio(bytes: Slice[u8]) -> Result[f64, Error] {
  return parse::float_with_underscores(bytes);
}
```

## Tests

```text
tests/cases/standard-library/ok/parse/std-parse-basic.ari
```

The focused test covers ASCII-trimmed signed integer parsing, radix wrappers
for binary, octal, and hexadecimal input, unsigned integer parsing, boolean
parsing, natural `Result` error categories for invalid data and invalid radix
input, richer integer/unsigned/float `ParseError` diagnostics and offsets,
float overflow/underflow range diagnostics including finite/subnormal boundary
edges, `_optional` compatibility helpers,
underscore-aware integer/radix/float parsing, `Result` float parsing, float
validation, float conversion, trait-backed typed parsing, and invalid
whole-input cases. It is wired into `make check-prelude` with LLVM symbol
checks for the public helpers.

## Future Work

- a richer parse error taxonomy only if real callers need more categories than
  the current stable spelling, offset, overflow, and underflow diagnostics
