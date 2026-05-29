# std::encoding

`std::encoding` contains byte validation and byte-to-text codec helpers. It is
separate from `std::string` because Ari strings are currently byte strings, and
separate from `std::hash` because hex/base64 are encodings, not hashing.

Use it when code needs to validate ASCII, UTF-8, or UTF-16 input, or when it
needs portable hex/base64 text for byte buffers.

Fallible UTF-8 validation and UTF-8 scalar/string conversion use the natural
API names and return `Result[..., Utf8Error]` so callers can report the exact
failing byte and category. Fallible UTF-16 counts and hex/base64 codecs use
`Result[..., std::error::Error]` with `InvalidData` for malformed input.
Hex/base64 callers that need a precise failure location can query the matching
`*_error` helper before or after choosing the shared `Result` decoder. The
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
Utf8Error::name() -> String
Utf8Error::name_with_region(ref mut Region) -> String
Utf8Error::message() -> String
Utf8Error::message_with_region(ref mut Region) -> String
encoding::utf8_error(bytes) -> Option[Utf8Error]
encoding::validate_utf8(bytes) -> Result[(), Utf8Error]
encoding::validate_utf8_optional(bytes) -> Option[Utf8Error]
encoding::decode_utf8(ref mut zone, bytes) -> Result[String, Utf8Error]
encoding::decode_utf8_in(ref mut zone, bytes) -> Result[String, Utf8Error]
encoding::decode_utf8_with_region(ref mut Region, bytes) -> Result[String, Utf8Error]
encoding::decode_utf8_optional_in(ref mut zone, bytes) -> Option[String]
encoding::decode_utf8_optional_with_region(ref mut Region, bytes) -> Option[String]
encoding::decode_utf8_unchecked_in(ref mut zone, bytes) -> String
encoding::decode_utf8_unchecked_with_region(ref mut Region, bytes) -> String
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
`name()` returns a stable short diagnostic label such as `overlong encoding`,
and `message()` returns a longer human-readable explanation suitable for CLI
errors and logs.
`validate_utf8` returns `Result[(), Utf8Error]`, keeping those details in the
recoverable path instead of collapsing them into a boolean.
`decode_utf8(ref mut zone, bytes)` first validates the whole byte slice, then
copies the bytes into a zone-backed `String`. `decode_utf8_in` is the
compatibility spelling for the same operation. New public code should prefer
`decode_utf8_with_region(ref mut region, bytes)` or
`region.decode_utf8(bytes)`, which copy into a named `Region` lifetime.
`decode_utf8_optional_in` and `decode_utf8_optional_with_region` return
`None` for invalid input, and the `_unchecked` forms panic for invalid input.

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
encoding::encode_utf8_with_region(ref mut Region, scalar) -> Result[String, Utf8Error]
encoding::encode_utf8_optional_in(ref mut zone, scalar) -> Option[String]
encoding::encode_utf8_optional_with_region(ref mut Region, scalar) -> Option[String]
encoding::try_encode_utf8_in(ref mut zone, scalar) -> Option[String]
encoding::try_encode_utf8_with_region(ref mut Region, scalar) -> Option[String]
encoding::encode_utf8_unchecked_in(ref mut zone, scalar) -> String
encoding::encode_utf8_unchecked_with_region(ref mut Region, scalar) -> String
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
invalid scalars. Use the `*_with_region` forms when the encoded scalar should
live in a public `Region`.

Hex helpers:

```ari
encoding::CodecErrorKind
encoding::CodecError
CodecError::name() -> String
CodecError::name_with_region(ref mut Region) -> String
CodecError::message() -> String
CodecError::message_with_region(ref mut Region) -> String
encoding::hex_encoded_len(bytes) -> i64
encoding::encode_hex_in(ref mut zone, bytes) -> String
encoding::encode_hex_with_region(ref mut Region, bytes) -> String
encoding::hex_error(bytes) -> Option[CodecError]
encoding::hex_decoded_len(bytes) -> Result[i64, std::error::Error]
encoding::hex_decoded_len_optional(bytes) -> Option[i64]
encoding::can_decode_hex(bytes) -> bool
encoding::decode_hex(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_hex_in(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_hex_with_region(ref mut Region, bytes) -> Result[String, std::error::Error]
encoding::decode_hex_optional_in(ref mut zone, bytes) -> Option[String]
encoding::decode_hex_optional_with_region(ref mut Region, bytes) -> Option[String]
encoding::try_decode_hex_in(ref mut zone, bytes) -> Option[String]
encoding::try_decode_hex_with_region(ref mut Region, bytes) -> Option[String]
encoding::decode_hex_unchecked_in(ref mut zone, bytes) -> String
encoding::decode_hex_unchecked_with_region(ref mut Region, bytes) -> String
```

Hex encoding uses lowercase `a..f`. Decoding accepts the same ASCII hex digits
accepted by `std::ascii::hex_value`, so uppercase input is valid.
`hex_error` returns `None` for valid input or `Some(CodecError)` for malformed
input. `CodecError` records the byte index, byte value, and a
`CodecErrorKind`: `InvalidLength`, `InvalidByte`, or `InvalidPadding`. Use
`index()`, `byte()`, and `kind()` for diagnostics, or the predicate helpers
`is_invalid_length()`, `is_invalid_byte()`, and `is_invalid_padding()` for
branching. `name()` returns a stable short label such as `invalid byte`, and
`message()` returns a longer human-readable explanation. Hex currently reports
odd-length input as `InvalidLength` at the last byte.
`hex_decoded_len`, `decode_hex`, and `decode_hex_in` return
`Error(InvalidData)` for invalid length or non-hex bytes. `decode_hex_in` is
the compatibility spelling for the explicit-zone decoder.
`hex_decoded_len_optional`, `decode_hex_optional_in`, and the older
`try_decode_hex_in` compatibility alias return `None` for invalid input.
`decode_hex_unchecked_in` is the asserting compatibility form and panics for
invalid input. Use `encode_hex_with_region`, `decode_hex_with_region`, or
`region.encode_hex` / `region.decode_hex` when codec output should belong to a
public region.

Base64 helpers:

```ari
encoding::base64_encoded_len(bytes) -> i64
encoding::encode_base64_in(ref mut zone, bytes) -> String
encoding::encode_base64_with_region(ref mut Region, bytes) -> String
encoding::base64_error(bytes) -> Option[CodecError]
encoding::base64_decoded_len(bytes) -> Result[i64, std::error::Error]
encoding::base64_decoded_len_optional(bytes) -> Option[i64]
encoding::can_decode_base64(bytes) -> bool
encoding::decode_base64(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_base64_in(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_base64_with_region(ref mut Region, bytes) -> Result[String, std::error::Error]
encoding::decode_base64_optional_in(ref mut zone, bytes) -> Option[String]
encoding::decode_base64_optional_with_region(ref mut Region, bytes) -> Option[String]
encoding::try_decode_base64_in(ref mut zone, bytes) -> Option[String]
encoding::try_decode_base64_with_region(ref mut Region, bytes) -> Option[String]
encoding::decode_base64_unchecked_in(ref mut zone, bytes) -> String
encoding::decode_base64_unchecked_with_region(ref mut Region, bytes) -> String
encoding::base64_mime_encoded_len(bytes) -> i64
encoding::encode_base64_mime_in(ref mut zone, bytes) -> String
encoding::encode_base64_mime_with_region(ref mut Region, bytes) -> String
encoding::base64_mime_error(bytes) -> Option[CodecError]
encoding::base64_mime_decoded_len(bytes) -> Result[i64, std::error::Error]
encoding::base64_mime_decoded_len_optional(bytes) -> Option[i64]
encoding::can_decode_base64_mime(bytes) -> bool
encoding::decode_base64_mime(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_base64_mime_in(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_base64_mime_with_region(ref mut Region, bytes) -> Result[String, std::error::Error]
encoding::decode_base64_mime_optional_in(ref mut zone, bytes) -> Option[String]
encoding::decode_base64_mime_optional_with_region(ref mut Region, bytes) -> Option[String]
encoding::try_decode_base64_mime_in(ref mut zone, bytes) -> Option[String]
encoding::try_decode_base64_mime_with_region(ref mut Region, bytes) -> Option[String]
encoding::decode_base64_mime_unchecked_in(ref mut zone, bytes) -> String
encoding::decode_base64_mime_unchecked_with_region(ref mut Region, bytes) -> String
encoding::base64_url_encoded_len(bytes) -> i64
encoding::base64_url_unpadded_encoded_len(bytes) -> i64
encoding::encode_base64_url_in(ref mut zone, bytes) -> String
encoding::encode_base64_url_with_region(ref mut Region, bytes) -> String
encoding::encode_base64_url_unpadded_in(ref mut zone, bytes) -> String
encoding::encode_base64_url_unpadded_with_region(ref mut Region, bytes) -> String
encoding::base64_url_error(bytes) -> Option[CodecError]
encoding::base64_url_decoded_len(bytes) -> Result[i64, std::error::Error]
encoding::base64_url_decoded_len_optional(bytes) -> Option[i64]
encoding::can_decode_base64_url(bytes) -> bool
encoding::decode_base64_url(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_base64_url_in(ref mut zone, bytes) -> Result[String, std::error::Error]
encoding::decode_base64_url_with_region(ref mut Region, bytes) -> Result[String, std::error::Error]
encoding::decode_base64_url_optional_in(ref mut zone, bytes) -> Option[String]
encoding::decode_base64_url_optional_with_region(ref mut Region, bytes) -> Option[String]
encoding::try_decode_base64_url_in(ref mut zone, bytes) -> Option[String]
encoding::try_decode_base64_url_with_region(ref mut Region, bytes) -> Option[String]
encoding::decode_base64_url_unchecked_in(ref mut zone, bytes) -> String
encoding::decode_base64_url_unchecked_with_region(ref mut Region, bytes) -> String
```

Base64 uses the standard `A-Z`, `a-z`, `0-9`, `+`, `/`, and `=` alphabet. The
decoder requires length to be a multiple of four and padding to appear only at
the end. MIME base64 uses the standard alphabet, wraps encoded output with CRLF
after each 76 encoded characters, and decodes by ignoring ASCII space, tab, CR,
and LF before applying the standard base64 validation rules. URL-safe base64
uses the same bit layout with `-` and `_` in place of `+` and `/`.
`encode_base64_url_in` emits padded output, while
`encode_base64_url_unpadded_in` omits trailing `=` for token and URL contexts.
`decode_base64_url` accepts both padded and unpadded URL-safe input, but it
rejects the standard `+` and `/` alphabet so callers do not accidentally mix
protocol policies.

`base64_error`, `base64_mime_error`, and `base64_url_error` return
`None<CodecError>()` for valid input and `Some(CodecError)` for invalid input.
`InvalidLength` covers lengths that cannot encode complete base64 groups,
including padded URL-safe input whose length is not a multiple of four and
unpadded URL-safe input with a one-byte remainder. `InvalidByte` reports bytes
outside the active alphabet. `InvalidPadding` reports `=` before the suffix
padding area, too much MIME padding, or non-padding data after padding has
started. MIME diagnostics report indexes in the original byte slice, including
ignored whitespace positions in the scan.

`base64_decoded_len`, `decode_base64`, `decode_base64_in`,
`base64_mime_decoded_len`, `decode_base64_mime`, `decode_base64_mime_in`,
`base64_url_decoded_len`, `decode_base64_url`, and `decode_base64_url_in`
return `Error(InvalidData)` for invalid length, alphabet, or padding
placement. The `_in` forms are compatibility spellings for explicit-zone
decoders. The `_optional` helpers and older `try_decode_*_in` compatibility
aliases discard that category into `None`.

All base64 encoders and decoders that allocate also have `*_with_region`
forms. Prefer those, or the matching `Region` methods, when protocol text or
decoded bytes should share the caller's public allocation lifetime. The
`*_in(ref mut Zone, ...)` forms remain source-compatible shims for older code.

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

Use the codec diagnostic helpers when user-facing tools need a precise message:

```ari
match encoding::base64_url_error(input) {
  std::Some(reason) => {
    if reason.is_invalid_byte() {
      // report reason.index(), reason.byte(), reason.name(ref mut zone), and reason.message(ref mut zone)
    }
  }
  std::None => {
    // input is decodable by decode_base64_url
  }
}
```

`Utf8Error` and `CodecError` expose `name(ref mut zone)` and
`message(ref mut zone)` for owned diagnostic strings. `name_with_region` and
`message_with_region` are the user-facing allocation forms. Their `_text`
variants return raw borrowed labels for compatibility and allocation-free
checks.

## Example

```ari
fn copy_hex(arena: ref mut region::Region, bytes: Slice[u8]) -> String {
  return encoding::encode_hex_with_region(arena, bytes);
}

fn decode_known_base64(arena: ref mut region::Region, text: Slice[u8]) -> String {
  return arena.decode_base64(text).unwrap();
}

fn read_utf8(arena: ref mut region::Region, bytes: Slice[u8]) -> Result[String, encoding::Utf8Error] {
  return arena.decode_utf8(bytes);
}
```

## Tests

```text
tests/cases/standard-library/ok/encoding/std-encoding-text.ari
tests/cases/standard-library/ok/encoding/std-encoding-utf8-codepoints.ari
tests/cases/standard-library/ok/encoding/std-encoding-codec.ari
```

`std-encoding-text.ari` covers ASCII, UTF-8, UTF-8 diagnostic details, and
UTF-8 decoding, UTF-16 validation/counting, the `Result` validation/count
helpers, and Region-backed UTF-8 allocation helpers.
`std-encoding-utf8-codepoints.ari` covers scalar validation, UTF-8 lead-byte
width, byte-offset decoding, next-index helpers, natural Result scalar
encoding with detailed `Utf8Error`, optional compatibility scalar encoding,
and unchecked scalar encoding.
`std-encoding-codec.ari` covers hex/base64 length helpers, structured
`CodecError` diagnostics, standard, MIME, and URL-safe base64 encoding, MIME
folding and decoding, padded and unpadded URL-safe base64 decoding, natural
Result decoding, optional compatibility decoding, unchecked decoding, Region
codec allocation helpers, and invalid input guards. These tests are wired into
`make check-prelude` with LLVM symbol checks.

## Future Work

- Unicode normalization, grapheme clusters, and transcoding only after Ari has
  a deliberate text policy beyond byte strings
- optional compression helpers in a separate module once byte-buffer ownership
  and error handling are stronger
