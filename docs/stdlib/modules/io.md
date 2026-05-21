# std::io

`std::io` is the byte-oriented process IO module. It keeps the raw runtime
hooks visible, then layers a small Ari-source interface over them so code can
talk about readers, writers, exact reads, whole-stream reads, whole-stream
string reads, stream copies, whole-slice writes, buffered wrappers, and
seekable streams with natural names.

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
- filesystem adapters: `std::fs::File` implements `Reader`, `Writer`, and
  `Seek`
- source constructors and adapters: `stdin`, `stdout`, `stderr`, `pipe`,
  `cursor`, `buf_reader`, `buf_writer`, `BufReader::new`, `BufWriter::new`
- direct error helpers: `read_exact_result`, `copy_result`,
  `write_all_result`, `flush_result`
- compatibility helpers: `read_exact`, `read_all`, `read_to_string`,
  `try_copy`, `copy`, `write_all`, `flush`

Roadmap, not implemented yet:

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

io::read_exact_result[R: Reader](reader: ref mut R, output: ptr u8, len: i64) -> Result[(), Error]
io::read_exact[R: Reader](reader: ref mut R, output: ptr u8, len: i64) -> bool
io::read_all[R: Reader](zone: ref mut Zone, reader: ref mut R) -> std::vec::Vec[u8]
io::read_to_string[R: Reader](zone: ref mut Zone, reader: ref mut R) -> std::string::String
io::copy_result[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W) -> Result[i64, Error]
io::try_copy[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W) -> Option[i64]
io::copy[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W) -> bool
io::write_all_result[W: Writer](writer: ref mut W, values: Slice[u8]) -> Result[(), Error]
io::write_all[W: Writer](writer: ref mut W, values: Slice[u8]) -> bool
io::flush_result[W: Writer](writer: ref mut W) -> Result[(), Error]
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
matches the existing runtime hook. Prefer `read_exact_result` when the caller
needs recoverable failure: it returns `Ok(())` only when every requested byte
was copied into `output`, and `Err(Error(UnexpectedEof))` when EOF arrives
early. Negative lengths return `Err(Error(InvalidInput))`. `read_exact` is the
bool compatibility wrapper over that result shape.
`read_all(ref mut zone, ref mut reader)` repeatedly reads until EOF and returns
a zone-backed `Vec[u8]`. Use it when the caller wants the whole remaining byte
stream and can make allocation explicit through the zone.
`read_to_string(ref mut zone, ref mut reader)` follows the same EOF rule but
collects directly into an owned `std::string::String`. Ari strings are
byte-backed, so this helper does not validate UTF-8; call `text.try_utf8()`
when a caller needs a validated UTF-8 view.
`copy_result(ref mut reader, ref mut writer)` streams from any `Reader` into
any `Writer`, flushes the writer after EOF, and returns `Ok(byte_count)` when
all writes and the final flush succeed. A failed byte write returns
`Err(Error(BrokenPipe))`; a failed final flush returns `Err(Error(Other))`.
`try_copy` is the `Option[i64]` compatibility wrapper, and `copy` is the bool
wrapper for call sites that only care whether the whole stream moved.

`Writer.write_byte` returns whether the byte was accepted.
`write_all_result` returns `Ok(())` after all bytes are accepted and
`Err(Error(BrokenPipe))` on the first failed byte write. `write_all` is its bool
compatibility wrapper. `flush_result` delegates to the writer and turns a
failed flush into `Err(Error(Other))`; `flush` is the bool wrapper. The current
`Stdout.flush()` and `Stderr.flush()` are no-op successes because the existing
process stream hooks write immediately; real flush hooks belong with future OS
handles.

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
  var input = [65u8, 66u8];
  var cursor = io::cursor(input.as_slice());
  var output = [0u8, 0u8];
  match io::read_exact_result<io::Cursor>(ref mut cursor, output.as_slice().as_ptr(), 2) {
    Ok(_) => { return 0; }
    Err(reason) => {
      if reason.is_kind(error::UnexpectedEof) {
        return 1;
      }
      return 2;
    }
  }
}
```

```ari
fn main() -> i64 {
  var zone = zone::create(128);
  var input = [65u8, 66u8, 67u8];
  var cursor = io::cursor(input.as_slice());
  let bytes = io::read_all<io::Cursor>(ref mut zone, ref mut cursor);
  let count = bytes.len();
  zone::destroy(zone);
  return count;
}
```

```ari
fn main() -> i64 {
  var zone = zone::create(128);
  var input = io::cursor("owned text");
  let text = io::read_to_string<io::Cursor>(ref mut zone, ref mut input);
  let count = text.len();
  zone::destroy(zone);
  return count;
}
```

```ari
fn main() -> i64 {
  var zone = zone::create(128);
  var pipe = io::pipe().unwrap();
  var reader = pipe.take_reader();
  var writer = pipe.take_writer();
  var input = io::cursor("copy me");
  if !io::copy<io::Cursor, io::PipeWriter>(ref mut input, ref mut writer) {
    return 1;
  }
  writer.close();

  var output = io::read_all<io::PipeReader>(ref mut zone, ref mut reader);
  let count = output.len();
  reader.close();
  zone::destroy(zone);
  return count;
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
`Seek`, `read_exact`, `read_all`, `read_to_string`, `try_copy`, `copy`,
`write_all`, and `flush`. Avoid type-suffixed helper names when trait bounds
already carry the type information.

The module keeps process IO and filesystem construction separate, but
`std::fs::File` now adapts to `Reader`, `Writer`, and `Seek`. Create the file
handle in `std::fs`, then pass it to generic helpers such as `read_to_string`,
`copy`, `try_copy`, `write_all`, `flush`, or your own `S: io::Seek` helpers.
Close policy still belongs to the file handle owner: `File` is not an owned
drop resource yet, and `flush` only checks that the direct descriptor remains
open because `File` itself does not buffer.

## Tests

- `tests/cases/standard-library/ok/io/std-io-byte-slice.ari` checks
  `io::write_bytes`, `std::io::write_bytes`, root `write_bytes`, stdout, and
  the returned byte count.
- `tests/cases/standard-library/ok/io/std-io-traits-cursor.ari` checks
  `Reader`, `Writer`, `Seek`, `Cursor`, `stdin`, `stdout`, `read_exact`,
  `write_all`, `flush`, generated helper symbols, and stdout output.
- `tests/cases/standard-library/ok/io/std-io-read-all.ari` checks
  `read_all` over `Cursor` and `BufReader[Cursor]`, returned byte order, EOF
  after collection, and generated generic helper symbols.
- `tests/cases/standard-library/ok/io/std-io-read-to-string.ari` checks
  `read_to_string` over `Cursor` and `BufReader[Cursor]`, owned byte-string
  contents, EOF after collection, and generated generic helper symbols.
- `tests/cases/standard-library/ok/io/std-io-copy.ari` checks `try_copy` and
  `copy` over generic `Reader`/`Writer` values, copied byte counts, final
  flush, writer failure behavior, and generated generic helper symbols.
- `tests/cases/standard-library/ok/io/std-io-result.ari` checks
  `read_exact_result`, `write_all_result`, `flush_result`, and `copy_result`
  direct `Error` payloads plus the compatibility wrappers they now delegate to.
- `tests/cases/standard-library/ok/fs/std-fs-io-traits.ari` checks the
  filesystem adapter side: `File` as `Reader`/`Writer`, generic
  `read_to_string`, `read_exact`, `try_copy`, `write_all`, `flush`, and
  invalid-handle behavior.
- `tests/cases/standard-library/ok/fs/std-fs-seek.ari` checks the filesystem
  seek adapter side: `File` as `Seek`, module `position`/`seek` hooks, method
  syntax, generic `S: io::Seek` dispatch, and invalid seek behavior.
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
