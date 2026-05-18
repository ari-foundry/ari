# std::input

`std::input` is the friendly stdin-facing layer. It sits beside lower-level
`std::io` hooks and gives programs names that read like input operations:
`input::line()` for borrowed line input, `input::owned_line(...)` for an owned
copy, and `input::try_read_byte()` when EOF should be handled with `Option`.

The module is intentionally narrow. It does not manage files, terminals,
environment variables, or process handles. `std::env` owns process argument
helpers today, and future OS-facing modules such as `std::fs` and
`std::process` should own their resource handles explicitly.

## API

```ari
input::read_byte() -> i64
input::try_read_byte() -> Option[u8]
input::line() -> string
input::owned_line(ref mut Zone) -> std::string::String

read_byte() -> i64
read_line() -> string
read_line_owned(ref mut Zone) -> std::string::String
input() -> string
input_owned(ref mut Zone) -> std::string::String
```

`input::read_byte()` is the raw byte hook. It returns the next byte as an
`i64`, or `-1` at end of input.

`input::try_read_byte()` is the source helper to prefer in ordinary Ari code.
It returns `Some(byte)` for a byte and `None` at EOF.

`input::line()` returns a borrowed lowercase `string` backed by an internal
runtime line buffer. A later line read can overwrite that buffer.

`input::owned_line(ref mut zone)` copies the line into a zone-backed
`std::string::String`, so the text can survive later input reads as long as the
zone remains valid.

## Example

```ari
fn main() -> i64 {
  let byte = input::try_read_byte();
  if byte.is_none() {
    return 0;
  }

  return byte.unwrap_or(0 as u8) as i64;
}
```

## Relationship To std::io

`std::io` keeps the lower-level process IO hooks together:

```ari
io::read_byte()
io::read_line()
io::read_line_owned(ref mut zone)
io::write_i64(value)
io::write_u64(value)
io::write_bool(value)
io::write_byte(value)
io::write_bytes(values)
io::newline()
```

Use `std::input` when the code is about reading stdin as user input. Use
`std::io` when testing or documenting the raw runtime hooks directly.

## Tests

- `tests/cases/standard-library/ok/prelude-input.ari` checks raw byte input
  names and runtime hook lowering.
- `tests/cases/standard-library/ok/std-input-byte-option.ari` checks
  `input::try_read_byte()` and EOF conversion to `Option[u8]`.
- `tests/cases/standard-library/ok/prelude-read-line.ari` checks borrowed line
  input aliases.
- `tests/cases/standard-library/ok/prelude-read-line-owned.ari` checks owned
  line input with explicit zones.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused stdin/runtime coverage.
