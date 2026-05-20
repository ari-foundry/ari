# std::io

`std::io` is the byte-oriented process IO module. It keeps the raw runtime
hooks visible, then layers a small Ari-source interface over them so code can
talk about readers, writers, exact reads, whole-slice writes, buffered wrappers,
and seekable in-memory cursors with natural names.

Use `std::input` for ordinary stdin line/byte helpers, `std::fs` for files, and
`std::io` when you want the lower-level IO contracts directly.

## Current Scope

Implemented now:

- raw stdout/scalar hooks: `write_i64`, `write_u64`, `write_bool`,
  `write_byte`, `write_bytes`, `newline`
- raw stdin/line hooks: `read_byte`, `read_line`, `read_line_owned`
- source traits: `Reader`, `Writer`, `Seek`
- source handles: `Stdin`, `Stdout`, `Stderr`, `Pipe`, `PipeReader`,
  `PipeWriter`, `Cursor`, `BufReader`, `BufWriter`
- source constructors and adapters: `stdin`, `stdout`, `stderr`, `pipe`,
  `cursor`, `buf_reader`, `buf_writer`, `BufReader::new`, `BufWriter::new`
- source helpers: `read_exact`, `write_all`, `flush`

Roadmap, not implemented yet:

- `File` as `Reader`/`Writer`/`Seek`: should land with the owned file-resource
  policy so copied handles cannot accidentally double-close.
- zone-owning buffered constructors and drop-time writer flush: need compiler
  support for new std types that own zone-backed raw buffers and a clear
  generic `Drop`/flush policy.

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

io::BufReader[R]
io::BufWriter[W]
io::Stdin
io::Stdout
io::Stderr
io::Pipe
io::PipeReader
io::PipeWriter
io::stdin() -> io::Stdin
io::stdout() -> io::Stdout
io::stderr() -> io::Stderr
io::pipe() -> Option[io::Pipe]
pipe.read_end()
pipe.write_end()
pipe.take_reader()
pipe.take_writer()
pipe.close()
pipe_reader.as_fd()
pipe_reader.is_open()
pipe_reader.close()
pipe_writer.as_fd()
pipe_writer.is_open()
pipe_writer.close()
io::cursor(values: Slice[u8]) -> io::Cursor
io::buf_reader[R: Reader](inner: R, buffer: Slice[u8]) -> io::BufReader[R]
io::buf_writer[W: Writer](inner: W, buffer: Slice[u8]) -> io::BufWriter[W]
io::BufReader::new<R>(inner: R, buffer: Slice[u8]) -> io::BufReader[R]
io::BufWriter::new<W>(inner: W, buffer: Slice[u8]) -> io::BufWriter[W]

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
current `Stdout.flush()` and `Stderr.flush()` are no-op successes because the
existing process stream hooks write immediately; real flush hooks belong with
future OS handles.

`Cursor` reads from a borrowed `Slice[u8]`. It implements `Reader` and `Seek`,
so it is useful for tests, parsers, and examples that should not depend on
host stdin.

`io::pipe()` returns `Option[io::Pipe]`. A successful pipe owns the raw
descriptor pair through `std::os::Pipe`, then `take_reader()` and
`take_writer()` split it into a `PipeReader` and `PipeWriter`.
`PipeReader` implements `Reader`, `PipeWriter` implements `Writer`, and both
ends expose `as_fd()`, `is_open()`, and explicit `close()` helpers. A pipe
writer flush succeeds while its descriptor is open because writes go directly
to the descriptor; use `BufWriter[PipeWriter]` for caller-managed buffering.

`BufReader[R]` and `BufWriter[W]` wrap any `Reader` or `Writer` with an
explicit caller-provided `Slice[u8]` buffer. This keeps allocation visible and
lets the wrappers be implemented in Ari source today. The buffer slice must
stay alive while the wrapper is used. `BufWriter` flushes only when the buffer
is full or when `flush()` is called; there is no implicit drop-time flush yet.

## Examples

```ari
fn main() -> i64 {
  var input = ['A', 'B', 'C'];
  var cursor = io::cursor(input.as_slice());
  var storage = [0u8, 0u8];
  var reader = io::buf_reader<io::Cursor>(cursor, storage.as_slice());

  var output = [0u8, 0u8];
  let output_view = output.as_slice();
  let output_raw = output_view.as_ptr();
  if !io::read_exact<io::BufReader[io::Cursor]>(ref mut reader, output_raw, 2) {
    return 1;
  }

  var out = io::stdout();
  var out_storage = [0u8, 0u8];
  var writer = io::BufWriter::new<io::Stdout>(out, out_storage.as_slice());
  io::write_all<io::BufWriter[io::Stdout]>(ref mut writer, output.as_slice());
  io::flush<io::BufWriter[io::Stdout]>(ref mut writer);
  return reader.buffered_len();
}
```

```ari
fn main() -> i64 {
  var pipe = io::pipe().unwrap();
  var reader = pipe.take_reader();
  var writer = pipe.take_writer();
  io::write_all<io::PipeWriter>(ref mut writer, "ok");
  writer.close();

  var bytes = [0u8, 0u8];
  if !io::read_exact<io::PipeReader>(ref mut reader, bytes.as_slice().as_ptr(), 2) {
    return 1;
  }
  reader.close();
  return 0;
}
```

For formatted output, prefer `print`, `println`, `print!`, and `println!`.
`write_bool` and formatted bool placeholders write lowercase `true` or
`false`.
For raw byte output to stdout, create `var out = io::stdout()` and call
`io::write_all(ref mut out, bytes)`, or use the older `io::write_bytes(bytes)`
helper. For raw byte output to stderr, create `var err = io::stderr()` and use
the same `io::write_all`/`io::flush` helpers.
For formatted values, prefer `fmt::write_value(ref mut writer, ref mut zone,
value)` so the value's `Display` impl carries the type instead of spelling a
type suffix in the IO API. For direct stdout formatting without a `Writer`
handle, prefer `fmt::print_value(ref mut zone, value)` or
`fmt::println_value(ref mut zone, value)`.

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
- `tests/cases/standard-library/ok/io/std-io-stderr.ari` checks `Stderr`,
  stderr routing, explicit flush success, generated helper symbols, and
  stdout/stderr stream separation.
- `tests/cases/standard-library/ok/io/std-io-pipe.ari` checks `Pipe`,
  `PipeReader`, `PipeWriter`, `std::os::Pipe` ownership splitting, trait-based
  whole-slice writes, exact reads, EOF after writer close, explicit closes,
  and runtime read/write hook lowering.
- `tests/cases/standard-library/ok/io/std-io-buffered.ari` checks
  `BufReader`, `BufWriter`, caller-provided buffers, associated constructors,
  exact reads through a buffered reader, whole-slice writes through a buffered
  writer, generated helper symbols, and stdout output.
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
