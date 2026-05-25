# std::encoding

`std::encoding` contains byte validation and byte-to-text codec helpers. It is
separate from `std::string` because Ari strings are currently byte strings, and
separate from `std::hash` because hex/base64 are encodings, not hashing.

Use it when code needs to validate ASCII, UTF-8, or UTF-16 input, or when it
needs portable hex/base64 text for byte buffers.

Naming is in transition. The long-term stdlib direction is for natural
fallible names such as UTF-8 validation/counting and decoding helpers to return
`Result`, with `_optional`, `_or`, `_raw`, and `_unchecked` reserved for
information-discarding or boundary behavior. The current `*_result` codec and
validation names are transitional compatibility APIs while existing
Option/asserting names are migrated.

## API

Validation helpers:

```ari
encoding::is_ascii(bytes) -> bool
encoding::is_unicode_scalar(value) -> bool
encoding::Utf8ErrorKind
encoding::Utf8Error
encoding::utf8_error(bytes) -> Option[Utf8Error]
encoding::validate_utf8(bytes) -> Option[Utf8Error]
encoding::validate_utf8_result(bytes) -> Result[(), std::error::Error]
encoding::utf8_count(bytes) -> Option[i64]
encoding::utf8_count_result(bytes) -> Result[i64, std::error::Error]
encoding::is_utf8(bytes) -> bool
encoding::utf16_count(words) -> Option[i64]
encoding::utf16_count_result(words) -> Result[i64, std::error::Error]
encoding::is_utf16(words) -> bool
```

`is_ascii` checks that every byte is `0..127`. `utf8_count` validates UTF-8
and returns the number of Unicode scalar values represented by the byte slice.
It rejects overlong encodings, surrogate code points, invalid continuation
bytes, and values above `U+10FFFF`. `is_utf8` is the boolean form.
`utf8_error` and its alias `validate_utf8` return `None` for valid UTF-8 and
`Some(Utf8Error)` for invalid input. `Utf8Error` records the failing byte
index, the byte that best identifies the failure, and a `Utf8ErrorKind`:
`InvalidLead`, `UnexpectedEnd`, `InvalidContinuation`, `OverlongEncoding`,
`SurrogateCodePoint`, or `OutOfRangeCodePoint`. Use `index()`, `byte()`, and
`kind()` when reporting diagnostics, or the predicate helpers
`is_invalid_lead()`, `is_unexpected_end()`, `is_invalid_continuation()`,
`is_overlong()`, `is_surrogate()`, and `is_out_of_range()` when branching.
`validate_utf8_result` and `utf8_count_result` are the `Result` forms for code
that composes with other fallible APIs. They use `Error(InvalidData)` for
invalid UTF-8; call `utf8_error` when the diagnostic needs the exact byte and
failure category.

`utf16_count` validates `Slice[u16]` input and counts code points, treating a
valid surrogate pair as one code point. It rejects lone high surrogates, lone
low surrogates, and broken pairs. `is_utf16` is the boolean form.
`utf16_count_result` returns `Error(InvalidData)` for invalid UTF-16.

UTF-8 scalar helpers:

```ari
encoding::Utf8Char
encoding::utf8_width(first_byte) -> Option[i64]
encoding::utf8_encoded_len(scalar) -> Option[i64]
encoding::utf8_at(bytes, byte_index) -> Option[Utf8Char]
encoding::utf8_next_index(bytes, byte_index) -> Option[i64]
encoding::encode_utf8_in(ref mut zone, scalar) -> String
encoding::try_encode_utf8_in(ref mut zone, scalar) -> Option[String]
encoding::encode_utf8_result_in(ref mut zone, scalar) -> Result[String, std::error::Error]
```

`Utf8Char` stores one decoded Unicode scalar value and the number of bytes
consumed. Use `scalar()`, `len()`, and `next_index(byte_index)` to inspect it.
`utf8_width` classifies a UTF-8 lead byte only; it does not validate following
continuation bytes. `utf8_at` validates and decodes at a byte offset, returning
`None<Utf8Char>()` for out-of-range indexes, continuation-byte offsets,
overlong encodings, surrogate scalar values, truncated sequences, or values
above `U+10FFFF`. `try_encode_utf8_in` returns `None` for invalid scalars.
`encode_utf8_result_in` returns `Error(InvalidData)` for invalid scalars.
`encode_utf8_in` is the asserting form and panics for invalid scalars.

Hex helpers:

```ari
encoding::hex_encoded_len(bytes) -> i64
encoding::encode_hex_in(ref mut zone, bytes) -> String
encoding::hex_decoded_len(bytes) -> Option[i64]
encoding::hex_decoded_len_result(bytes) -> Result[i64, std::error::Error]
encoding::can_decode_hex(bytes) -> bool
encoding::decode_hex_in(ref mut zone, bytes) -> String
encoding::try_decode_hex_in(ref mut zone, bytes) -> Option[String]
encoding::decode_hex_result_in(ref mut zone, bytes) -> Result[String, std::error::Error]
```

Hex encoding uses lowercase `a..f`. Decoding accepts the same ASCII hex
digits accepted by `std::ascii::hex_value`, so uppercase input is valid.
`try_decode_hex_in` returns `None` for invalid input. `hex_decoded_len_result`
and `decode_hex_result_in` return `Error(InvalidData)` for invalid length or
non-hex bytes. `decode_hex_in` is the asserting form and panics for invalid
input.

Base64 helpers:

```ari
encoding::base64_encoded_len(bytes) -> i64
encoding::encode_base64_in(ref mut zone, bytes) -> String
encoding::base64_decoded_len(bytes) -> Option[i64]
encoding::base64_decoded_len_result(bytes) -> Result[i64, std::error::Error]
encoding::can_decode_base64(bytes) -> bool
encoding::decode_base64_in(ref mut zone, bytes) -> String
encoding::try_decode_base64_in(ref mut zone, bytes) -> Option[String]
encoding::decode_base64_result_in(ref mut zone, bytes) -> Result[String, std::error::Error]
```

Base64 uses the standard `A-Z`, `a-z`, `0-9`, `+`, `/`, and `=` alphabet. The
decoder requires length to be a multiple of four and padding to appear only at
the end. Line-wrapped MIME base64 and URL-safe base64 are future variants, not
accepted by this first slice. The `*_result` base64 helpers return
`Error(InvalidData)` for invalid length, alphabet, or padding placement.

## Fallible Decoding

Use the `try_decode_*_in` helpers for untrusted input:

```ari
match encoding::try_decode_hex_in(ref mut zone, input) {
  std::Some(text) => {
    // use text
  }
  std::None => {
    // reject input
  }
}
```

Use the `*_result_in` helpers when the caller should keep a shared
`std::error::Error` failure shape:

```ari
match encoding::decode_hex_result_in(ref mut zone, input) {
  std::Ok(text) => {
    // use text
  }
  std::Err(reason) => {
    assert(reason.is_kind(error::InvalidData));
  }
}
```

The lower-level `*_decoded_len` and `can_decode_*` helpers remain useful when a
caller wants to validate before reserving storage or report a simpler boolean
failure.

## Example

```ari
fn copy_hex(zone: ref mut Zone, bytes: Slice[u8]) -> String {
  return encoding::encode_hex_in(zone, bytes);
}

fn decode_known_base64(zone: ref mut Zone, text: Slice[u8]) -> String {
  assert(encoding::can_decode_base64(text));
  return encoding::decode_base64_in(zone, text);
}
```

## Tests

```text
tests/cases/standard-library/ok/encoding/std-encoding-text.ari
tests/cases/standard-library/ok/encoding/std-encoding-utf8-codepoints.ari
tests/cases/standard-library/ok/encoding/std-encoding-codec.ari
```

`std-encoding-text.ari` covers ASCII, UTF-8, UTF-8 diagnostic details, and
UTF-16 validation/counting, including the `Result` validation/count helpers.
`std-encoding-utf8-codepoints.ari` covers scalar validation, UTF-8 lead-byte
width, byte-offset decoding, next-index helpers, asserting scalar encoding,
fallible scalar encoding, and `Result` scalar encoding.
`std-encoding-codec.ari` covers hex/base64 length helpers, encoding, decoding,
fallible decoding, `Result` decoding, and invalid input guards. These tests
are wired into `make check-prelude` with LLVM symbol checks.

## Future Work

- URL-safe base64 and optional line-wrapped MIME base64
- richer structured decode errors beyond the current shared `InvalidData`
  result category
- Unicode normalization, grapheme clusters, and transcoding only after Ari has
  a deliberate text policy beyond byte strings
- optional compression helpers in a separate module once byte-buffer ownership
  and error handling are stronger
