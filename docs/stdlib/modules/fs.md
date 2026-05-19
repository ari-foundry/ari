# std::fs

`std::fs` is Ari's portable filesystem module. It exists so ordinary programs
can work with files without reaching directly for raw syscalls or C ABI
declarations. The first slice is deliberately small: check whether a path
exists, open a byte stream for reading or writing, read and write bytes, close
the handle, and remove a file.

The public names stay natural because the module path already says the domain:
use `open_read`, `open_write`, `read_byte`, `write_byte`, `write_bytes`,
`close`, `exists`, and `remove`, not type-suffixed names.

## API

```ari
fs::exists(path)
fs::remove(path)
fs::open_read(path)
fs::open_write(path)
fs::try_open_read(path)
fs::try_open_write(path)
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
`try_open_read(path)` and `try_open_write(path)` for normal control flow; they
return `Option[File]` and avoid making callers remember the invalid-handle
sentinel.

`open_read(path)` opens an existing file for byte reads. `open_write(path)`
creates or truncates a file for byte writes using the current Linux/glibc LLVM
runtime path. Both functions return a `File`; check `is_open()` before using
the raw form.

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

## Current Limits

- This slice is byte-oriented. There is no owned path type, directory
  iteration, metadata, permissions API, canonicalization, or text encoding
  policy yet.
- Runtime hooks currently target the Linux/glibc LLVM path through `access`,
  `unlink`, `open`, `read`, `write`, and `close`.
- `File` is not a tracked `own` resource yet. The caller must close each
  successful handle exactly once by convention. Future ownership work should
  make OS resources harder to copy and accidentally double-close.
- `open_write` creates or truncates a file. Append and read-write modes are
  future API slices.
- Error details are not surfaced yet. Current APIs expose boolean success or a
  `-1` read sentinel. A future `std::os` or `std::io` error value can carry
  platform error codes.

## Tests

```text
tests/cases/standard-library/ok/fs/std-fs-basic.ari
```

`make check-prelude` emits LLVM for the runtime hooks, checks the C syscall
declarations, runs the executable, and cleans up the temporary file under
`build/prelude/`.

## Next Work

- Add append/read-write open modes only after their names and failure policy
  are documented.
- Add file metadata and directory iteration as separate tested slices.
- Add path helpers after Ari has a clearer owned string/path story.
- Promote `File` toward an owned resource handle when the compiler can express
  OS-resource ownership and drop policy cleanly.
