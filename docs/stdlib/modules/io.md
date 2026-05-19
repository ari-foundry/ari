# std::io

`std::io` is the low-level process IO module. It exposes the current runtime
hooks for stdout, stdin byte reads, and host line input. Higher-level names,
such as `std::input`, build on the same hooks when a friendlier API shape is
useful.

The module stays small on purpose. It is not a file API, terminal API, async
IO API, or process API. Those belong to future modules after C FFI and resource
ownership conventions are more complete.

## API

```ari
io::write_i64(value: i64) -> i64
io::write_u64(value: u64) -> i64
io::write_bool(value: bool) -> i64
io::write_byte(value: u8) -> i64
io::write_bytes(values: Slice[u8]) -> i64
io::newline() -> i64
io::read_byte() -> i64
io::read_line() -> string
io::read_line_owned(ref mut Zone) -> std::string::String

write_i64(value)
write_u64(value)
write_bool(value)
write_byte(value)
write_bytes(values)
newline()
read_byte()
read_line()
read_line_owned(ref mut zone)
```

`write_byte` writes one byte through the backend hook. `write_bytes` is an Ari
source helper that writes each byte in a `Slice[u8]` and returns the number of
bytes attempted.

`read_byte` returns the next byte as `i64`, or `-1` at EOF. Prefer
`input::try_read_byte()` in ordinary stdin code when EOF should be represented
as `Option[u8]`.

`read_line` returns a borrowed lowercase `string` backed by the runtime line
buffer. `read_line_owned(ref mut zone)` copies into a zone-backed
`std::string::String`.

## Example

```ari
fn main() -> i64 {
  var bytes = [65 as u8, 66 as u8, 10 as u8];
  let written = io::write_bytes(bytes.as_slice());
  return written;
}
```

## Relationship To Formatting

Use `print`, `println`, `print!`, and `println!` for formatted user-facing
output. Use `std::io` when you need explicit low-level hook behavior, raw byte
output, or tests that should make the backend IO calls visible in LLVM IR.

## Tests

- `tests/cases/standard-library/ok/prelude/prelude-io.ari` checks scalar IO names and
  root aliases.
- `tests/cases/standard-library/ok/io/std-io-byte-slice.ari` checks
  `io::write_bytes`, `std::io::write_bytes`, root `write_bytes`, stdout, and
  the returned byte count.
- `tests/cases/standard-library/ok/input/prelude-input.ari` checks byte input hook
  lowering.
- `tests/cases/standard-library/ok/input/prelude-read-line.ari` and
  `tests/cases/standard-library/ok/input/prelude-read-line-owned.ari` cover borrowed
  and owned line input.

Run `make check-std-api` after public API edits and `make check-prelude` for
the focused process IO coverage.
