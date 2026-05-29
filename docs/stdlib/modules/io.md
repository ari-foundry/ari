# std::io

`std::io` is the byte-oriented process IO module. It keeps the raw runtime
hooks visible, then layers a small Ari-source interface over them so code can
talk about readers, writers, exact reads, line reads, whole-stream reads,
whole-stream string reads, stream copies, whole-slice writes, text output,
buffered wrappers, and
seekable streams with natural names.

Use `std::input` for ordinary stdin line/byte helpers, `std::fs` for files, and
`std::io` when you want the lower-level IO contracts directly.

Fallible helpers use natural names and return `Result[..., Error]`:
`read`, `read_exact`, `read_line_from`, `read_to_string`, `copy`, `write`,
`write_all`, and `flush`. `Reader` byte hooks can also be inspected through the
`ReadByte` status enum when code needs to distinguish byte, EOF, and
adapter-detected failure. `Writer` exposes natural `write(values)` and
`write_all(values)` trait methods, so generic writer-bound code can use method
syntax instead of dropping back to module helpers. Compatibility helpers that
discard error details use explicit suffixes such as `_unchecked`, `_optional`,
or `_or`. `Reader::read_byte` and `Writer::write_byte` still use the older
scalar/bool contracts because they mirror the current runtime hooks;
higher-level helpers are the default recoverable API.

Migration note: older snapshots exposed these recoverable helpers as
`read_exact`, `copy`, `write_all`, and `flush`,
while the natural names returned `bool`. Update fallible call sites to the
natural names and update bool-only call sites to the `_unchecked` names.

## Current Scope

Implemented now:

- stdout/stderr text helpers: `print`, `println`, `eprint`, `eprintln`, plus
  compatibility aliases `print_text`, `println_text`, `eprint_text`, and
  `eprintln_text`; these write plain text and return `Result`
- raw stdout/scalar hooks: `write_i64`, `write_u64`, `write_bool`,
  `write_byte`, `write_bytes`, `newline`
- raw stdin/line hooks: `read_byte`, `read_line`, `read_line_owned`
- source traits: `Reader`, `Writer`, `Seek`; `ReadByte` describes one byte,
  EOF, or an adapter-detected read error; `Writer` includes natural
  `write`, `write_all`, and `flush` Result methods plus the low-level
  `write_byte` hook
- source handles: `Stdin`, `Stdout`, `Stderr`, `Pipe`, `PipeReader`,
  `PipeWriter`, `Cursor`, `BufReader`, `BufWriter`
- filesystem adapters: `std::fs::File` implements `Reader`, `Writer`, and
  `Seek`
- source constructors and adapters: `stdin`, `stdout`, `stderr`, `pipe`,
  `pipe_optional`, `cursor`, `buf_reader`, `buf_reader_in`, `buf_writer`,
  `buf_writer_in`, Region variants `buf_reader_with_region` and
  `buf_writer_with_region`, `BufReader::new`, `BufReader::with_capacity`,
  `BufReader::with_capacity_with_region`, `BufWriter::new`,
  `BufWriter::with_capacity`, and `BufWriter::with_capacity_with_region`
- direct error helpers: `read_one`, `read`, `read_exact`, `read_line_from`,
  `read_to_string`, `copy`, `write`, `write_all`, `flush`, plus Region
  variants for owned reads
- collection helper: `read_all` and `read_all_with_region`
- compatibility helpers: `read_exact_unchecked`, `read_to_string_unchecked`,
  `try_copy`, `copy_unchecked`, `write_all_unchecked`, `flush_unchecked`

## API

```ari
io::Error
io::ErrorKind

pub trait Reader {
  fn read_byte(self: ref mut Self) -> i64;
}

pub trait Writer {
  fn write_byte(self: ref mut Self, value: u8) -> bool;
  fn write(self: ref mut Self, values: Slice[u8]) -> Result[i64, Error];
  fn write_all(self: ref mut Self, values: Slice[u8]) -> Result[(), Error];
  fn flush(self: ref mut Self) -> Result[(), Error];
}

pub trait Seek {
  fn position(self: ref Self) -> i64;
  fn seek(self: ref mut Self, position: i64) -> bool;
}

io::BufReader[R]
io::BufWriter[W]
io::ReadByte
ReadByte::is_byte() -> bool
ReadByte::is_eof() -> bool
ReadByte::is_error() -> bool
ReadByte::byte() -> Option[u8]
ReadByte::error() -> Option[Error]
io::Stdin
io::Stdout
io::Stderr
io::Pipe
io::PipeReader
io::PipeWriter
io::stdin() -> io::Stdin
io::stdout() -> io::Stdout
io::stderr() -> io::Stderr
io::pipe() -> Result[io::Pipe, Error]
io::pipe_optional() -> Option[io::Pipe]
pipe.read_end()
pipe.write_end()
pipe.take_reader()
pipe.take_writer()
pipe.close() -> Result[(), Error]
pipe.close_bool() -> bool
pipe_reader.as_fd()
pipe_reader.is_open()
pipe_reader.close() -> Result[(), Error]
pipe_reader.close_bool() -> bool
pipe_writer.as_fd()
pipe_writer.is_open()
pipe_writer.close_on_exec() -> Result[bool, Error]
pipe_writer.close_on_exec_optional() -> Option[bool]
pipe_writer.set_close_on_exec(enabled) -> Result[(), Error]
pipe_writer.set_close_on_exec_bool(enabled) -> bool
pipe_writer.close() -> Result[(), Error]
pipe_writer.close_bool() -> bool
io::cursor(values: Slice[u8]) -> io::Cursor
io::buf_reader[R: Reader](inner: R, buffer: Slice[u8]) -> io::BufReader[R]
io::buf_reader_in[R: Reader](zone: ref mut Zone, inner: R, capacity: i64) -> Result[io::BufReader[R], Error]
io::buf_reader_with_region[R: Reader](region: ref mut Region, inner: R, capacity: i64) -> Result[io::BufReader[R], Error]
io::buf_writer[W: Writer](inner: W, buffer: Slice[u8]) -> io::BufWriter[W]
io::buf_writer_in[W: Writer](zone: ref mut Zone, inner: W, capacity: i64) -> Result[io::BufWriter[W], Error]
io::buf_writer_with_region[W: Writer](region: ref mut Region, inner: W, capacity: i64) -> Result[io::BufWriter[W], Error]
io::BufReader::new<R>(inner: R, buffer: Slice[u8]) -> io::BufReader[R]
io::BufReader::with_capacity<R>(zone: ref mut Zone, inner: R, capacity: i64) -> Result[io::BufReader[R], Error]
io::BufReader::with_capacity_with_region<R>(region: ref mut Region, inner: R, capacity: i64) -> Result[io::BufReader[R], Error]
reader.read_one() -> ReadByte
reader.read(output) -> Result[i64, Error]
reader.read_line(zone) -> Result[String, Error]
reader.read_line_with_region(region) -> Result[String, Error]
reader.read_to_string(zone) -> Result[String, Error]
reader.read_to_string_with_region(region) -> Result[String, Error]
io::BufWriter::new<W>(inner: W, buffer: Slice[u8]) -> io::BufWriter[W]
io::BufWriter::with_capacity<W>(zone: ref mut Zone, inner: W, capacity: i64) -> Result[io::BufWriter[W], Error]
io::BufWriter::with_capacity_with_region<W>(region: ref mut Region, inner: W, capacity: i64) -> Result[io::BufWriter[W], Error]
writer.write(values) -> Result[i64, Error]
writer.write_all(values) -> Result[(), Error]

io::read_exact[R: Reader](reader: ref mut R, output: ptr u8, len: i64) -> Result[(), Error]
io::read_one[R: Reader](reader: ref mut R) -> ReadByte
io::read[R: Reader](reader: ref mut R, output: Slice[u8]) -> Result[i64, Error]
io::read_exact_unchecked[R: Reader](reader: ref mut R, output: ptr u8, len: i64) -> bool
io::read_all[R: Reader](zone: ref mut Zone, reader: ref mut R) -> std::vec::Vec[u8]
io::read_all_with_region[R: Reader](region: ref mut Region, reader: ref mut R) -> std::vec::Vec[u8]
io::read_line_from[R: Reader](zone: ref mut Zone, reader: ref mut R) -> Result[String, Error]
io::read_line_from_with_region[R: Reader](region: ref mut Region, reader: ref mut R) -> Result[String, Error]
io::read_to_string[R: Reader](zone: ref mut Zone, reader: ref mut R) -> Result[String, Error]
io::read_to_string_with_region[R: Reader](region: ref mut Region, reader: ref mut R) -> Result[String, Error]
io::read_to_string_unchecked[R: Reader](zone: ref mut Zone, reader: ref mut R) -> std::string::String
io::read_to_string_unchecked_with_region[R: Reader](region: ref mut Region, reader: ref mut R) -> std::string::String
io::copy[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W) -> Result[i64, Error]
io::try_copy[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W) -> Option[i64]
io::copy_unchecked[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W) -> bool
io::write[W: Writer](writer: ref mut W, values: Slice[u8]) -> Result[i64, Error]
io::write_all[W: Writer](writer: ref mut W, values: Slice[u8]) -> Result[(), Error]
io::write_all_unchecked[W: Writer](writer: ref mut W, values: Slice[u8]) -> bool
io::flush[W: Writer](writer: ref mut W) -> Result[(), Error]
io::flush_unchecked[W: Writer](writer: ref mut W) -> bool
io::print(text: Slice[u8]) -> Result[(), Error]
io::print_text(text: Slice[u8]) -> Result[(), Error]
io::print_string(text: ref String) -> Result[(), Error]
io::println(text: Slice[u8]) -> Result[(), Error]
io::println_text(text: Slice[u8]) -> Result[(), Error]
io::println_string(text: ref String) -> Result[(), Error]
io::eprint(text: Slice[u8]) -> Result[(), Error]
io::eprint_text(text: Slice[u8]) -> Result[(), Error]
io::eprint_string(text: ref String) -> Result[(), Error]
io::eprintln(text: Slice[u8]) -> Result[(), Error]
io::eprintln_text(text: Slice[u8]) -> Result[(), Error]
io::eprintln_string(text: ref String) -> Result[(), Error]

io::write_i64(value: i64) -> i64
io::write_u64(value: u64) -> i64
io::write_bool(value: bool) -> i64
io::write_byte(value: u8) -> i64
io::write_bytes(values: Slice[u8]) -> i64
io::newline() -> i64
io::read_byte() -> i64
io::read_line(ref mut Zone) -> std::string::String
io::read_line_with_region(ref mut Region) -> std::string::String
io::read_line_text() -> String
io::read_line_owned(ref mut Zone) -> std::string::String

write_i64(value)
write_u64(value)
write_bool(value)
write_byte(value)
write_bytes(values)
newline()
read_byte()
read_line(ref mut zone)
read_line_text()
read_line_owned(ref mut zone)
```

`Reader.read_byte` returns the next byte as `i64`, `-1` at EOF, or a value
below `-1` when an adapter detects a real read failure through today's scalar
hook. OS-backed file, pipe, TCP, and Unix-stream hooks split `read(2) == 0`
from `read(2) < 0`, so EOF and host read failures no longer share the same
sentinel in those adapters. The scalar hook remains the compatibility layer.
`read_one(ref mut reader)` wraps one low-level byte read in `ReadByte`:
`ReadByteValue(byte)` for a byte, `ReadByteEof` for ordinary end-of-stream,
and `ReadByteError(error)` when an adapter reports a real failure. Generic
readers backed only by `Reader::read_byte` map `-1` to EOF and values below
`-1` to the current adapter error category, `Error(BrokenPipe)`. `PipeReader`,
`File`, TCP streams, and Unix streams return that error for closed or invalid
handles, and Result-returning module helpers can map host errno where they own
the platform error channel.
Use `is_byte()`, `is_eof()`, `is_error()`, `byte()`, and `error()` for compact
branching.
`read(ref mut reader, output)` fills up to `output.len` bytes and returns
`Ok(count)`. EOF before any bytes is `Ok(0)`, EOF after a prefix is
`Ok(prefix_len)`, and adapter-detected errors return `Err(error)` when no byte
has been produced yet. If a prefix has already been copied, `read` returns the
prefix count and lets the next call observe the failure, matching ordinary
partial-read behavior. `read_exact` is stricter: it returns `Ok(())` only when
every requested byte was copied into `output`, `Err(Error(UnexpectedEof))` when
EOF arrives early, and the adapter error when the low-level reader reports a
real failure. Negative lengths return `Err(Error(InvalidInput))`.
`read_exact_unchecked` is the bool compatibility wrapper over that result
shape.
`read_all(ref mut zone, ref mut reader)` repeatedly reads until EOF and returns
a zone-backed `Vec[u8]`. New user-facing code should prefer
`read_all_with_region(ref mut region, ref mut reader)` or
`region.io_read_all(ref mut reader)`, which allocate the same bytes in a
public `Region` lifetime while keeping the old zone entry point as a
compatibility shim.
`read_line_from(ref mut zone, ref mut reader)` reads through the first newline
or EOF and returns a zone-backed `String`. EOF after some bytes is success;
EOF before any bytes returns an empty `String`; adapter-detected read failures
drop the partial line and return `Err(error)`. Reader methods named
`read_line(zone)` are available on `Stdin`, `Cursor`, `BufReader`, and
`PipeReader`; `read_line_with_region(region)` is the Region form.
`read_to_string(ref mut zone, ref mut reader)` follows the same EOF rule but
collects the whole remaining stream into an owned `std::string::String` inside
`Result`. Adapter-detected read failures drop the partial string and return
`Err(error)`. Ari strings are byte-backed, so this helper does not validate
UTF-8; call `text.try_utf8()` when a caller needs a validated UTF-8 view.
Prefer `read_to_string_with_region(ref mut region, ref mut reader)` or
`region.io_read_to_string(ref mut reader)` when the caller owns a public
allocation lifetime.
`read_to_string_unchecked` is the compatibility helper for older call sites
that intentionally discard the `Result` wrapper.
`copy(ref mut reader, ref mut writer)` streams from any `Reader` into any
`Writer`, flushes the writer after EOF, and returns `Ok(byte_count)` when all
reads, writes, and the final flush succeed. A failed read returns the reader
adapter error without flushing, a failed byte write returns
`Err(Error(BrokenPipe))`, and a failed final flush returns the writer's flush
error.
`try_copy` is the `Option[i64]` compatibility wrapper, and `copy_unchecked` is
the bool wrapper for call sites that only care whether the whole stream moved.

`Writer.write_byte` returns whether the byte was accepted and remains the
minimal hook implementors must define from their raw backend. `Writer.write`
returns the accepted byte count or `Err(Error(BrokenPipe))` on the first failed
byte write. `Writer.write_all` returns `Ok(())` after all bytes are accepted
and the same `BrokenPipe` error on failure. The module-level `io::write` and
`io::write_all` helpers delegate through the same contract, so
`writer.write(bytes)`, `writer.write_all(bytes)`, `io::write(ref mut writer,
bytes)`, and `io::write_all(ref mut writer, bytes)` agree. `Stdout`, `Stderr`,
`PipeWriter`, `BufWriter`, `std::fs::File`, `TcpStream`, and `UnixStream`
implement these trait methods. `write_all_unchecked` is the bool compatibility
wrapper. `Writer.flush()` returns `Result[(), Error]`; `io::flush` delegates to
that natural method, and `flush_unchecked` keeps the bool compatibility shape.
The current `Stdout.flush()` and `Stderr.flush()` are no-op successes because
the existing process stream hooks write immediately; real flush hooks belong
with future OS handles.

Custom `Writer` implementations should keep `write_byte` as the smallest raw
operation and usually implement `write`/`write_all` by delegating to
`io::write<YourWriter>(self, values)` and `io::write_all<YourWriter>(self,
values)`. Implementors with an efficient host bulk-write path, such as network
streams, can call that host path directly while preserving the same `Result`
semantics.

`print`, `println`, `eprint`, and `eprintln` are the Result-returning
plain-text helpers for hosted CLI messages. They accept borrowed bytes, so
string literals work directly, for example `io::print("arish> ")`. They do not
interpret formatting placeholders; build text first with `std::fmt` or string
helpers when a message needs interpolation. Use `print_string`,
`println_string`, `eprint_string`, and `eprintln_string` for owned `String`
handles. `print_text`, `println_text`, `eprint_text`, and `eprintln_text`
remain compatibility aliases for older borrowed-byte call sites. Root
`print`/`println` and `std::print`/`std::println` are still compiler-lowered
formatting calls, but the `io::` names belong to this source module.

`Cursor` reads from a borrowed `Slice[u8]`. It implements `Reader` and `Seek`,
so it is useful for tests, parsers, and examples that should not depend on
host stdin.

`io::pipe()` returns `Result[io::Pipe, Error]`; the error is the hosted OS
pipe creation error mapped through `std::error`. Use `io::pipe_optional()` only
when an application deliberately wants to discard the failure reason. A
successful pipe owns the raw descriptor pair through `std::os::Pipe`, then
`take_reader()` and `take_writer()` split it into a `PipeReader` and
`PipeWriter`.
`PipeReader` implements `Reader`, `PipeWriter` implements `Writer`, and both
ends expose `as_fd()`, `is_open()`, and explicit Result-returning `close()`
helpers. The `close_bool()` methods are compatibility wrappers for call sites
that intentionally discard close errors. `PipeWriter::close_on_exec()` and
`set_close_on_exec(enabled)` delegate to the owned descriptor flag and are used
by process setup-error pipes so successful `exec` closes the reporting channel
automatically. The `_optional` and `_bool` forms are compatibility wrappers for
call sites that intentionally discard the error detail. A pipe writer flush
succeeds while its descriptor is open because writes go directly to the
descriptor; use `BufWriter[PipeWriter]` for caller-managed buffering.

`BufReader[R]` and `BufWriter[W]` wrap any `Reader` or `Writer` with a byte
buffer. `buf_reader(inner, buffer)`, `buf_writer(inner, buffer)`, and the
`new` associated constructors borrow a caller-provided `Slice[u8]`; that slice
must stay alive while the wrapper is used. `buf_reader_in(zone, inner,
capacity)`, `buf_writer_in(zone, inner, capacity)`, and the `with_capacity`
associated constructors allocate the byte buffer in the supplied zone and
return `Err(InvalidInput)` when `capacity <= 0`. For public code, prefer
`buf_reader_with_region`, `buf_writer_with_region`, or the
`with_capacity_with_region` associated constructors; those buffers live until
the chosen region is reset or destroyed. `BufWriter` flushes when the buffer is
full, when `flush()` is called, or as a best-effort cleanup when the writer is
dropped while bytes remain buffered. Drop cleanup discards the `Result` because
destructors cannot report recoverable errors, so code that must observe write
failures should still call `flush()` explicitly before dropping the writer.

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
  if io::read_exact<io::BufReader[io::Cursor]>(ref mut reader, output_raw, 2).is_err() {
    return 1;
  }

  var out = io::stdout();
  var zone = zone::create(256);
  var writer = io::BufWriter::with_capacity<io::Stdout>(ref mut zone, out, 2).unwrap();
  io::write_all<io::BufWriter[io::Stdout]>(ref mut writer, output.as_slice()).unwrap();
  io::flush<io::BufWriter[io::Stdout]>(ref mut writer).unwrap();
  let buffered = reader.buffered_len();
  drop writer;
  zone::destroy(zone);
  return buffered;
}
```

```ari
fn main() -> i64 {
  var input = [65u8, 66u8];
  var cursor = io::cursor(input.as_slice());
  var output = [0u8, 0u8];
  match io::read_exact<io::Cursor>(ref mut cursor, output.as_slice().as_ptr(), 2) {
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
  var region = region::create(128);
  var input = [65u8, 66u8, 67u8];
  var cursor = io::cursor(input.as_slice());
  let bytes = io::read_all_with_region<io::Cursor>(ref mut region, ref mut cursor);
  let count = bytes.len();
  region::destroy(region);
  return count;
}
```

```ari
fn main() -> i64 {
  var region = region::create(128);
  var input = io::cursor("owned text");
  let text = region.io_read_to_string<io::Cursor>(ref mut input).unwrap();
  let count = text.len();
  region::destroy(region);
  return count;
}
```

```ari
fn main() -> i64 {
  io::println("Created package hello").unwrap();
  io::eprintln("error: Ari.toml not found").unwrap();
  return 0;
}
```

```ari
fn main() -> i64 {
  var zone = zone::create(128);
  var pipe = io::pipe().unwrap();
  var reader = pipe.take_reader();
  var writer = pipe.take_writer();
  var input = io::cursor("copy me");
  if io::copy<io::Cursor, io::PipeWriter>(ref mut input, ref mut writer).is_err() {
    return 1;
  }
  writer.close().unwrap();

  var output = io::read_all<io::PipeReader>(ref mut zone, ref mut reader);
  let count = output.len();
  reader.close().unwrap();
  zone::destroy(zone);
  return count;
}
```

```ari
fn main() -> i64 {
  var pipe = io::pipe().unwrap();
  var reader = pipe.take_reader();
  var writer = pipe.take_writer();
  io::write_all<io::PipeWriter>(ref mut writer, "ok").unwrap();
  writer.close().unwrap();

  var bytes = [0u8, 0u8];
  if io::read_exact<io::PipeReader>(ref mut reader, bytes.as_slice().as_ptr(), 2).is_err() {
    return 1;
  }
  reader.close().unwrap();
  return 0;
}
```

For formatted output, prefer `print`, `println`, `eprintln`, `print!`,
`println!`, and `eprintln!`.
`write_bool` and formatted bool placeholders write lowercase `true` or
`false`.
For raw byte output to stdout, create `var out = io::stdout()` and call
`io::write_all(ref mut out, bytes)`, or use the older `io::write_bytes(bytes)`
helper. For raw byte output to stderr, create `var err = io::stderr()` and use
the same `io::write_all`/`io::flush` helpers.
For formatted values, prefer `fmt::write_value(ref mut writer, ref mut zone,
value)` so the value's `Display` impl carries the type instead of spelling a
type suffix in the IO API. It returns `Result[(), Error]` from the underlying
writer; use `fmt::write_value_bool` only when discarding the write failure
reason is intentional. For direct stdout formatting without a `Writer` handle,
prefer `fmt::print_value(ref mut zone, value)` or
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
  contents, `read_line_from`, reader method forms, the unchecked
  compatibility helper, EOF after collection, and generated generic helper
  symbols.
- `tests/cases/standard-library/ok/io/std-io-natural-api.ari` checks
  natural `Writer::write`/`Writer::write_all` trait methods for stdout and
  stderr plus the Result-returning plain-text output helpers.
- `tests/cases/standard-library/ok/io/std-io-copy.ari` checks `try_copy` and
  `copy` over generic `Reader`/`Writer` values, copied byte counts, final
  flush, writer failure behavior, and generated generic helper symbols.
- `tests/cases/standard-library/ok/io/std-io-result.ari` checks
  `read_exact`, `write_all`, `flush`, and `copy` direct `Error` payloads plus
  the explicit `_unchecked` compatibility wrappers and Region-backed whole-read
  helpers.
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
  writer, best-effort drop-time flush, generated helper symbols, and stdout
  output.
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
