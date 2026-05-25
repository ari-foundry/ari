# std::encoding

`std::encoding` contains byte validation and byte-to-text codec helpers. It is
separate from `std::string` because Ari strings are currently byte strings, and
separate from `std::hash` because hex/base64 are encodings, not hashing.

Use it when code needs to validate ASCII, UTF-8, or UTF-16 input, or when it
needs portable hex/base64 text for byte buffers.

Fallible UTF-8 validation and UTF-8 scalar/string conversion use the natural
API names and return `Result[..., Utf8Error]` so callers can report the exact
failing byte and category. Fallible UTF-16 counts and hex/base64 codecs use
`Result[..., std::error::Error]` with `InvalidData` for malformed input. The
`_optional` helpers keep the compact compatibility surface when callers only
need presence or absence, `try_*` names are compatibility aliases for those
optional helpers, `_in` names are compatibility aliases for explicit-zone
allocation APIs, and `_unchecked` helpers keep the old asserting behavior for
trusted input.

## API

Validation helpers:

```ari
encoding::is_ascii(bytes) -> bool
encoding::is_unicode_scalar(value) -> bool
encoding::Utf8ErrorKind
encoding::Utf8Error
encoding::utf8_error(bytes) -> Option[Utf8Error]
encoding::validate_utf8(bytes) -> Result[(), Utf8Error]
encoding::validate_utf8_optional(bytes) -> Option[Utf8Error]
encoding::decode_utf8(ref mut zone, bytes) -> Result[String, Utf8Error]
encoding::decode_utf8_in(ref mut zone, bytes) -> Result[String, Utf8Error]
encoding::decode_utf8_optional_in(ref mut zone, bytes) -> Option[String]
encoding::decode_utf8_unchecked_in(ref mut zone, bytes) -> String
encoding::utf8_count(bytes) -> Result[i64, std::error::Error]
encoding::utf8_count_optional(bytes) -> Option[i64]
encoding::is_utf8(bytes) -> bool
encoding::utf16_count(words) -> Result[i64, std::error::Error]
encoding::utf16_count_optional(words) -> Option[i64]
encoding::is_utf16(words) -> bool
```

`is_ascii` checks that every byte is `0..127`. `utf8_count` validates UTF-8,
returns `Result[i64, Error]`, and counts the number of Unicode scalar values
represented by the byte slice. It rejects overlong encodings, surrogate code
points, invalid continuation bytes, and values above `U+10FFFF` with
`Error(InvalidData)`. `utf8_count_optional` and `is_utf8` are the
information-discarding compatibility forms.
`utf8_error` and `validate_utf8_optional` return `None` for valid UTF-8 and
`Some(Utf8Error)` for invalid input. `Utf8Error` records the failing byte
index, the byte that best identifies the failure, and a `Utf8ErrorKind`:
`InvalidLead`, `UnexpectedEnd`, `InvalidContinuation`, `OverlongEncoding`,
`SurrogateCodePoint`, or `OutOfRangeCodePoint`. Use `index()`, `byte()`, and
`kind()` when reporting diagnostics, or the predicate helpers
`is_invalid_lead()`, `is_unexpected_end()`, `is_invalid_continuation()`,
`is_overlong()`, `is_surrogate()`, and `is_out_of_range()` when branching.
`validate_utf8` returns `Result[(), Utf8Error]`, keeping those details in the
recoverable path instead of collapsing them into a boolean.
`decode_utf8(ref mut zone, bytes)` first validates the whole byte slice, then
copies the bytes into a zone-backed `String`. `decode_utf8_in` is the
compatibility spelling for the same operation. `decode_utf8_optional_in`
returns `None` for invalid input, and `decode_utf8_unchecked_in` panics for
invalid input.

`utf16_count` validates `Slice[u16]` input and counts code points through
`Result[i64, Error]`, treating a valid surrogate pair as one code point. It
rejects lone high surrogates, lone low surrogates, and broken pairs with
`Error(InvalidData)`. `utf16_count_optional` and `is_utf16` are the compact
compatibility forms.

UTF-8 scalar helpers:

```ari
encoding::Utf8Char
encoding::utf8_width(first_byte) -> Option[i64]
encoding::utf8_encoded_len(scalar) -> Option[i64]
encoding::utf8_at(bytes, byte_index) -> Option[Utf8Char]
encoding::utf8_next_index(bytes, byte_index) -> Option[i64]
encoding::encode_utf8(ref mut zone, scalar) -> Result[String, Utf8Error]
encoding::encode_utf8_in(ref mut zone, scalar) -> Result[String, Utf8Error]
encoding::encode_utf8_optional_in(ref mut zone, scalar) -> Option[String]
encoding::try_encode_utf8_in(ref mut zone, scalar) -> Option[String]
encoding::encode_utf8_unchecked_in(ref mut zone, scalar) -> String
```

`Utf8Char` stores one decoded Unicode scalar value and the number of bytes
consumed. Use `scalar()`, `len()`, and `next_index(byte_index)` to inspect it.
`utf8_width` classifies a UTF-8 lead byte only; it does not validate following
continuation bytes. `utf8_at` validates and decodes at a byte offset, returning
`None<Utf8Char>()` for out-of-range indexes, continuation-byte offsets,
overlong encodings, surrogate scalar values, truncated sequences, or values
above `U+10FFFF`. `encode_utf8` returns `Err(Utf8Error)` for invalid scalars:
surrogate values report `SurrogateCodePoint`, and values above `U+10FFFF`
report `OutOfRangeCodePoint`. `encode_utf8_in` is the compatibility spelling
for the same operation. `encode_utf8_optional_in` and its
`try_encode_utf8_in` compatibility alias return `None` for invalid scalars.
`encode_utf8_unchecked_in` is the asserting compatibility form and panics for
invalid scalars.

Hex helpers:

```ari
encoding::hex_encoded_len(bytes) -> i64
encoding::encode_hex_in(ref mut zone, bytes) -> String
encoding::hex_decoded_len(bytes) -> Result[i64, std::error::Error]
encoding::hex_decoded_len_optional(bytes) -> Option[i64]
encoding::can_decode_hex(bytes) -> bool
encoding::decode_hex(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_hex_in(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_hex_optional_in(ref mut zone, bytes) -> Option[String]
encoding::try_decode_hex_in(ref mut zone, bytes) -> Option[String]
encoding::decode_hex_unchecked_in(ref mut zone, bytes) -> String
```

Hex encoding uses lowercase `a..f`. Decoding accepts the same ASCII hex digits
accepted by `std::ascii::hex_value`, so uppercase input is valid.
`hex_decoded_len`, `decode_hex`, and `decode_hex_in` return
`Error(InvalidData)` for invalid length or non-hex bytes. `decode_hex_in` is
the compatibility spelling for the explicit-zone decoder.
`hex_decoded_len_optional`, `decode_hex_optional_in`, and the older
`try_decode_hex_in` compatibility alias return `None` for invalid input.
`decode_hex_unchecked_in` is the asserting compatibility form and panics for
invalid input.

Base64 helpers:

```ari
encoding::base64_encoded_len(bytes) -> i64
encoding::encode_base64_in(ref mut zone, bytes) -> String
encoding::base64_decoded_len(bytes) -> Result[i64, std::error::Error]
encoding::base64_decoded_len_optional(bytes) -> Option[i64]
encoding::can_decode_base64(bytes) -> bool
encoding::decode_base64(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_base64_in(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_base64_optional_in(ref mut zone, bytes) -> Option[String]
encoding::try_decode_base64_in(ref mut zone, bytes) -> Option[String]
encoding::decode_base64_unchecked_in(ref mut zone, bytes) -> String
```

Base64 uses the standard `A-Z`, `a-z`, `0-9`, `+`, `/`, and `=` alphabet. The
decoder requires length to be a multiple of four and padding to appear only at
the end. Line-wrapped MIME base64 and URL-safe base64 are future variants, not
accepted by this first slice. `base64_decoded_len`, `decode_base64`, and
`decode_base64_in` return `Error(InvalidData)` for invalid length, alphabet, or
padding placement. `decode_base64_in` is the compatibility spelling for the
explicit-zone decoder. The `_optional` helpers and older
`try_decode_base64_in` compatibility alias discard that category into `None`.

## Fallible Decoding

Use the natural decoder names when the caller should keep a shared
`std::error::Error` failure shape:

```ari
match encoding::decode_hex(ref mut zone, input) {
  std::Ok(text) => {
    // use text
  }
  std::Err(reason) => {
    assert(reason.is_kind(error::InvalidData));
  }
}
```

Use `_optional_in` when only success or absence matters:

```ari
match encoding::decode_hex_optional_in(ref mut zone, input) {
  std::Some(text) => {
    // use text
  }
  std::None => {
    // reject input
  }
}
```

The lower-level `*_decoded_len`, `*_decoded_len_optional`, and `can_decode_*`
helpers remain useful when a caller wants to validate before reserving storage
or report a simpler boolean failure.

## Example

```ari
fn copy_hex(zone: ref mut Zone, bytes: Slice[u8]) -> String {
  return encoding::encode_hex_in(zone, bytes);
}

fn decode_known_base64(zone: ref mut Zone, text: Slice[u8]) -> String {
  return encoding::decode_base64(zone, text).unwrap();
}

fn read_utf8(zone: ref mut Zone, bytes: Slice[u8]) -> Result[String, encoding::Utf8Error] {
  return encoding::decode_utf8(zone, bytes);
}
```

## Tests

```text
tests/cases/standard-library/ok/encoding/std-encoding-text.ari
tests/cases/standard-library/ok/encoding/std-encoding-utf8-codepoints.ari
tests/cases/standard-library/ok/encoding/std-encoding-codec.ari
```

`std-encoding-text.ari` covers ASCII, UTF-8, UTF-8 diagnostic details, and
UTF-8 decoding, and UTF-16 validation/counting, including the `Result`
validation/count helpers.
`std-encoding-utf8-codepoints.ari` covers scalar validation, UTF-8 lead-byte
width, byte-offset decoding, next-index helpers, natural Result scalar
encoding with detailed `Utf8Error`, optional compatibility scalar encoding,
and unchecked scalar encoding.
`std-encoding-codec.ari` covers hex/base64 length helpers, encoding, decoding,
natural Result decoding, optional compatibility decoding, unchecked decoding,
and invalid input guards. These tests are wired into `make check-prelude` with
LLVM symbol checks.

## Future Work

- URL-safe base64 and optional line-wrapped MIME base64
- richer structured decode errors beyond the current shared `InvalidData`
  result category
- Unicode normalization, grapheme clusters, and transcoding only after Ari has
  a deliberate text policy beyond byte strings
- optional compression helpers in a separate module once byte-buffer ownership
  and error handling are stronger
