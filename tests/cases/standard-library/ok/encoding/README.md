# Standard Library Encoding Tests

These cases cover source-only byte encoding and validation helpers in
`std::encoding`.

Current files:

- `std-encoding-text.ari`: ASCII, UTF-8, and UTF-16 validation/count helpers.
- `std-encoding-utf8-codepoints.ari`: UTF-8 width, scalar decode, next-index,
  scalar validation, and scalar encoding helpers.
- `std-encoding-codec.ari`: hex and base64 length, encode, decode, and invalid
  input behavior.
