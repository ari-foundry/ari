# std::input

`std::input` is the friendly stdin-facing layer. It sits beside lower-level
`std::io` hooks and gives programs names that read like input operations:
`input::line(ref mut zone)` for owned line input, `input::line_text()` for the
borrowed runtime hook, and `input::try_read_byte()` when EOF should be handled
with `Option`.

The module is intentionally narrow. It does not manage files, terminals,
environment variables, or process handles. `std::env` owns process argument
helpers today, and future OS-facing modules such as `std::fs` and
`std::process` should own their resource handles explicitly.

## API

```ari
input::read_byte() -> i64
input::try_read_byte() -> Option[u8]
input::line(ref mut Zone) -> std::string::String
input::line_text() -> string
input::owned_line(ref mut Zone) -> std::string::String

read_byte() -> i64
read_line(ref mut Zone) -> std::string::String
read_line_text() -> string
read_line_owned(ref mut Zone) -> std::string::String
input(ref mut Zone) -> std::string::String
input_text() -> string
input_owned(ref mut Zone) -> std::string::String
```

`input::read_byte()` is the raw byte hook. It returns the next byte as an
`i64`, or `-1` at end of input.

`input::try_read_byte()` is the source helper to prefer in ordinary Ari code.
It returns `Some(byte)` for a byte and `None` at EOF.

`input::line(ref mut zone)` copies the line into a zone-backed
`std::string::String`, so the text survives later input reads as long as the
zone remains valid. `input(ref mut zone)` is the root alias.

`input::line_text()` and `read_line_text()` return a borrowed lowercase
`string` backed by an internal runtime line buffer. A later line read can
overwrite that buffer. Keep those names for raw/runtime-boundary code.
`input::owned_line(ref mut zone)` and `input_owned(ref mut zone)` remain
compatibility aliases for the owned behavior.

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
io::read_line(ref mut zone)
io::read_line_text()
io::read_line_owned(ref mut zone)
io::write_i64(value)
io::write_u64(value)
io::write_bool(value)
io::write_byte(value)
io::write_bytes(values)
io::stdin()
io::stdout()
io::cursor(values)
io::buf_reader(inner, buffer)
io::buf_writer(inner, buffer)
io::read_exact[R: Reader](reader: ref mut R, output, len)
io::write_all[W: Writer](writer: ref mut W, values)
io::flush[W: Writer](writer: ref mut W)
io::newline()
```

Use `std::input` when the code is about reading stdin as user input. Use
`std::io` when testing or documenting the raw runtime hooks directly, or when
generic byte code should accept a `Reader`, `Writer`, or `Seek` implementation.

## Tests

- `tests/cases/standard-library/ok/input/prelude-input.ari` checks raw byte input
  names and runtime hook lowering.
- `tests/cases/standard-library/ok/input/std-input-byte-option.ari` checks
  `input::try_read_byte()` and EOF conversion to `Option[u8]`.
- `tests/cases/standard-library/ok/input/prelude-read-line.ari` checks owned line
  input aliases and the raw borrowed hook.
- `tests/cases/standard-library/ok/input/prelude-read-line-owned.ari` checks owned
  line input with explicit zones.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused stdin/runtime coverage.
