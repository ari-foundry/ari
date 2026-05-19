# std::encoding

`std::encoding` contains byte validation and byte-to-text codec helpers. It is
separate from `std::string` because Ari strings are currently byte strings, and
separate from `std::hash` because hex/base64 are encodings, not hashing.

Use it when code needs to validate ASCII, UTF-8, or UTF-16 input, or when it
needs portable hex/base64 text for byte buffers.

## API

Validation helpers:

```ari
encoding::is_ascii(bytes) -> bool
encoding::utf8_count(bytes) -> Option[i64]
encoding::is_utf8(bytes) -> bool
encoding::utf16_count(words) -> Option[i64]
encoding::is_utf16(words) -> bool
```

`is_ascii` checks that every byte is `0..127`. `utf8_count` validates UTF-8
and returns the number of Unicode scalar values represented by the byte slice.
It rejects overlong encodings, surrogate code points, invalid continuation
bytes, and values above `U+10FFFF`. `is_utf8` is the boolean form.

`utf16_count` validates `Slice[u16]` input and counts code points, treating a
valid surrogate pair as one code point. It rejects lone high surrogates, lone
low surrogates, and broken pairs. `is_utf16` is the boolean form.

Hex helpers:

```ari
encoding::hex_encoded_len(bytes) -> i64
encoding::encode_hex_in(ref mut zone, bytes) -> String
encoding::hex_decoded_len(bytes) -> Option[i64]
encoding::can_decode_hex(bytes) -> bool
encoding::decode_hex_in(ref mut zone, bytes) -> String
```

Hex encoding uses lowercase `a..f`. Decoding accepts the same ASCII hex
digits accepted by `std::ascii::hex_value`, so uppercase input is valid.
`decode_hex_in` panics for invalid input; call `can_decode_hex` or
`hex_decoded_len` first when invalid input is ordinary.

Base64 helpers:

```ari
encoding::base64_encoded_len(bytes) -> i64
encoding::encode_base64_in(ref mut zone, bytes) -> String
encoding::base64_decoded_len(bytes) -> Option[i64]
encoding::can_decode_base64(bytes) -> bool
encoding::decode_base64_in(ref mut zone, bytes) -> String
```

Base64 uses the standard `A-Z`, `a-z`, `0-9`, `+`, `/`, and `=` alphabet. The
decoder requires length to be a multiple of four and padding to appear only at
the end. Line-wrapped MIME base64 and URL-safe base64 are future variants, not
accepted by this first slice.

## Why Decoders Do Not Return `Option[String]` Yet

The natural future API is `decode_hex_in(...) -> Option[String]` or
`Result[String, DecodeError]`. Today's checker rejects zone-backed strings
escaping through enum payloads because the payload would carry a zone pointer
through `Option`. The current API therefore separates validation from decoding:

```ari
if encoding::can_decode_hex(input) {
  let text = encoding::decode_hex_in(ref mut zone, input);
}
```

When zone-backed enum payloads are supported, the public API can grow a
fallible decoding wrapper without changing the lower-level validating helpers.

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
tests/cases/standard-library/ok/encoding/std-encoding-codec.ari
```

`std-encoding-text.ari` covers ASCII, UTF-8, and UTF-16 validation/counting.
`std-encoding-codec.ari` covers hex/base64 length helpers, encoding, decoding,
and invalid input guards. Both are wired into `make check-prelude` with LLVM
symbol checks.

## Future Work

- URL-safe base64 and optional line-wrapped MIME base64
- richer decode errors after `Result[String, E]` can carry zone-backed values
- text normalization and transcoding only after Ari has a deliberate Unicode
  string policy
- optional compression helpers in a separate module once byte-buffer ownership
  and error handling are stronger
