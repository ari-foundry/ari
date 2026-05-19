# std::io

`std::io` is the byte-oriented process IO module. It keeps the raw runtime
hooks visible, then layers a small Ari-source interface over them so code can
talk about readers, writers, exact reads, whole-slice writes, and seekable
in-memory cursors with natural names.

Use `std::input` for ordinary stdin line/byte helpers, `std::fs` for files, and
`std::io` when you want the lower-level IO contracts directly.

## Current Scope

Implemented now:

- raw stdout/scalar hooks: `write_i64`, `write_u64`, `write_bool`,
  `write_byte`, `write_bytes`, `newline`
- raw stdin/line hooks: `read_byte`, `read_line`, `read_line_owned`
- source traits: `Reader`, `Writer`, `Seek`
- source handles: `Stdin`, `Stdout`, `Cursor`
- source constructors and adapters: `stdin`, `stdout`, `cursor`
- source helpers: `read_exact`, `write_all`, `flush`

Roadmap, not implemented yet:

- `Stderr` and `stderr()`: needs a runtime stderr write hook or an owned OS
  handle model.
- `pipe()`: needs explicit read/write handle ownership, close behavior, and
  platform split.
- `BufReader[R]` and `BufWriter[W]`: need zone-backed buffers, generic wrapper
  layout policy, and flush/drop rules.
- `File` as `Reader`/`Writer`/`Seek`: should land with the owned file-resource
  policy so copied handles cannot accidentally double-close.

## API

```ari
pub trait Reader {
  fn read_byte(self: ref mut Self) -> i64;
}

pub trait Writer {
  fn write_byte(self: ref mut Self, value: u8) -> bool;
  fn flush(self: ref mut Self) -> bool;
}

pub trait Seek {
  fn position(self: ref Self) -> i64;
  fn seek(self: ref mut Self, position: i64) -> bool;
}

io::stdin() -> io::Stdin
io::stdout() -> io::Stdout
io::cursor(values: Slice[u8]) -> io::Cursor

io::read_exact[R: Reader](reader: ref mut R, output: ptr u8, len: i64) -> bool
io::write_all[W: Writer](writer: ref mut W, values: Slice[u8]) -> bool
io::flush[W: Writer](writer: ref mut W) -> bool

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

`Reader.read_byte` returns the next byte as `i64` or `-1` at EOF. That sentinel
matches the existing runtime hook and keeps `read_exact` simple: it returns
`true` only when all requested bytes were copied into `output`.

`Writer.write_byte` returns whether the byte was accepted. `write_all` returns
`false` on the first failed byte write. `flush` delegates to the writer. The
current `Stdout.flush()` is a no-op success because the existing stdout hook is
immediate; a real flush hook belongs with future OS handles.

`Cursor` reads from a borrowed `Slice[u8]`. It implements `Reader` and `Seek`,
so it is useful for tests, parsers, and examples that should not depend on
host stdin.

## Examples

```ari
fn main() -> i64 {
  var input = [65u8, 66u8, 67u8];
  var cursor = io::cursor(input.as_slice());

  var output = [0u8, 0u8];
  let output_view = output.as_slice();
  let output_raw = output_view.as_ptr();
  if !io::read_exact<io::Cursor>(ref mut cursor, output_raw, 2) {
    return 1;
  }

  var out = io::stdout();
  io::write_all<io::Stdout>(ref mut out, output.as_slice());
  io::flush<io::Stdout>(ref mut out);
  return cursor.position();
}
```

For formatted output, prefer `print`, `println`, `print!`, and `println!`.
For raw byte output to stdout, create `var out = io::stdout()` and call
`io::write_all(ref mut out, bytes)`, or use the older `io::write_bytes(bytes)`
helper.

## Design Notes

The trait names intentionally stay short and conventional: `Reader`, `Writer`,
`Seek`, `read_exact`, `write_all`, and `flush`. Avoid type-suffixed helper
names when trait bounds already carry the type information.

The module still separates process IO from filesystem IO. `std::fs::File` will
implement these traits only after the file handle ownership policy is strong
enough to explain close, copy, drop, and seek behavior consistently.

## Tests

- `tests/cases/standard-library/ok/io/std-io-byte-slice.ari` checks
  `io::write_bytes`, `std::io::write_bytes`, root `write_bytes`, stdout, and
  the returned byte count.
- `tests/cases/standard-library/ok/io/std-io-traits-cursor.ari` checks
  `Reader`, `Writer`, `Seek`, `Cursor`, `stdin`, `stdout`, `read_exact`,
  `write_all`, `flush`, generated helper symbols, and stdout output.
- `tests/cases/standard-library/ok/input/prelude-input.ari` checks byte input
  hook lowering.
- `tests/cases/standard-library/ok/input/prelude-read-line.ari` and
  `tests/cases/standard-library/ok/input/prelude-read-line-owned.ari` cover
  borrowed and owned line input.

Run `make check-std-api` after public API edits. For this module, a focused
manual check is:

```sh
build/ari tests/cases/standard-library/ok/io/std-io-traits-cursor.ari -o build/std-io-traits-cursor.elf
build/std-io-traits-cursor.elf
```
