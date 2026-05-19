# std::fs

`std::fs` is Ari's portable filesystem module. It exists so ordinary programs
can work with files without reaching directly for raw syscalls or C ABI
declarations. The first slices are deliberately small: check whether a path
exists, open a byte stream for reading, writing, or appending, read and write
bytes, close the handle, and remove a file.

The public names stay natural because the module path already says the domain:
use `open_read`, `open_write`, `open_append`, `read_byte`, `write_byte`,
`write_bytes`, `close`, `exists`, and `remove`, not type-suffixed names.

## API

```ari
fs::exists(path)
fs::remove(path)
fs::open_read(path)
fs::open_write(path)
fs::open_append(path)
fs::try_open_read(path)
fs::try_open_write(path)
fs::try_open_append(path)
fs::close(file)
fs::read_byte(file)
fs::write_byte(file, value)
fs::write_bytes(file, values)

File::invalid()
file.is_open()
file.close()
file.read_byte()
file.write_byte(value)
file.write_bytes(values)
```

`File` is a small value handle around the runtime file descriptor. Failed open
operations return an invalid handle where `file.is_open()` is false. Prefer
`try_open_read(path)`, `try_open_write(path)`, and `try_open_append(path)` for
normal control flow; they return `Option[File]` and avoid making callers
remember the invalid-handle sentinel.

`open_read(path)` opens an existing file for byte reads. `open_write(path)`
creates or truncates a file for byte writes using the current Linux/glibc LLVM
runtime path. `open_append(path)` creates the file if needed and writes new
bytes at the end without truncating existing contents. All three raw functions
return a `File`; check `is_open()` before using the raw form.

`read_byte(file)` returns the next byte as `i64` or `-1` at EOF or on failure.
`write_byte(file, value)` returns whether exactly one byte was written.
`write_bytes(file, values)` writes each byte in a `Slice[u8]` until one write
fails and returns the number of bytes written before that failure.

`close(file)` returns whether the host accepted the close request. Closing an
invalid handle returns `false`. The current first slice does not mutate the
`File` value after closing, so do not reuse a copied handle after `close`.

`exists(path)` checks whether the path exists. `remove(path)` removes a file
path and returns whether the host accepted the request.

## Examples

Write and read a small file:

```ari
fn main() -> i64 {
  let path = "build/prelude/example-fs.tmp";
  let writer = fs::try_open_write(path).unwrap_or(fs::File::invalid());
  if !writer.is_open() {
    return 1;
  }

  var data = [65 as u8, 66 as u8, 67 as u8];
  if writer.write_bytes(data.as_slice()) != 3 {
    writer.close();
    return 2;
  }
  writer.close();

  let reader = fs::try_open_read(path).unwrap_or(fs::File::invalid());
  if !reader.is_open() {
    return 3;
  }
  let first = reader.read_byte();
  reader.close();
  fs::remove(path);
  return first - 65;
}
```

Handle absence without a sentinel:

```ari
let maybe_file = fs::try_open_read("missing-file.txt");
if maybe_file.is_none() {
  return 0;
}
```

Append to an existing file:

```ari
let appender = fs::try_open_append(path).unwrap_or(fs::File::invalid());
if appender.is_open() {
  appender.write_byte(10 as u8);
  appender.close();
}
```

## Current Limits

- This slice is byte-oriented. There is no owned path type, directory
  iteration, metadata, permissions API, canonicalization, or text encoding
  policy yet.
- Runtime hooks currently target the Linux/glibc LLVM path through `access`,
  `unlink`, `open`, `read`, `write`, and `close`.
- `File` is not a tracked `own` resource yet. The caller must close each
  successful handle exactly once by convention. Future ownership work should
  make OS resources harder to copy and accidentally double-close.
- `open_write` creates or truncates a file. `open_append` creates or appends.
  Read-write modes are still a future API slice.
- Error details are not surfaced yet. Current APIs expose boolean success or a
  `-1` read sentinel. A future `std::os` or `std::io` error value can carry
  platform error codes.

## Tests

```text
tests/cases/standard-library/ok/fs/std-fs-basic.ari
tests/cases/standard-library/ok/fs/std-fs-append.ari
```

`make check-prelude` emits LLVM for the runtime hooks, checks the C syscall
declarations, runs the executable, and cleans up the temporary file under
`build/prelude/`. `std-fs-basic.ari` covers existence, create/truncate writes,
reads, byte-slice writes, close, removal, and `Option[File]` read/write opens.
`std-fs-append.ari` covers append-mode open, preservation of existing bytes,
and failed append opens through `Option[File]`.

## Next Work

- Add read-write open mode only after its name and failure policy are
  documented.
- Add file metadata and directory iteration as separate tested slices.
- Add path helpers after Ari has a clearer owned string/path story.
- Promote `File` toward an owned resource handle when the compiler can express
  OS-resource ownership and drop policy cleanly.
