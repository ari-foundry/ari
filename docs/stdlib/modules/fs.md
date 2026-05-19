# std::fs

`std::fs` is Ari's portable filesystem module. It exists so ordinary programs
can work with files without reaching directly for raw syscalls or C ABI
declarations. The first slices are deliberately small: check whether a path
exists, open a byte stream with a compact mode string, read and write bytes,
close the handle, and remove a file.

The public names stay natural because the module path already says the domain:
use `open(path, mode)`, `try_open(path, mode)`, `read_byte`, `write_byte`,
`write_bytes`, `close`, `exists`, and `remove`, not type-suffixed names.

## API

```ari
fs::exists(path)
fs::remove(path)
fs::open(path, mode)
fs::open_read(path)
fs::open_write(path)
fs::open_append(path)
fs::try_open(path, mode)
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
`try_open(path, mode)` for normal control flow; it returns `Option[File]` and
avoids making callers remember the invalid-handle sentinel.

`open(path, mode)` takes a small mode string. Ari follows the familiar C/Python
shape for common modes while also accepting the direct `"rw"` spelling:

| Mode | Meaning |
| --- | --- |
| `"r"` | Open an existing file for reading. |
| `"w"` | Create or truncate a file for writing. |
| `"a"` | Create a file if needed and append writes at the end. |
| `"rw"` | Open an existing file for reading and writing. |
| `"r+"` | Alias for `"rw"` for C/Python familiarity. |
| `"w+"` | Create or truncate a file for reading and writing. |
| `"a+"` | Create if needed, read, and append writes at the end. |

Invalid or unsupported mode strings return an invalid `File`; `try_open`
turns that into `None`. Rust's standard library usually exposes richer
`OpenOptions` builder values instead of strings. Ari can add that later when
the language has stronger resource ownership, but mode strings are the right
small first surface because they keep call sites short and familiar.

`open_read`, `open_write`, `open_append`, and their `try_open_*` partners are
compatibility wrappers over `open(path, "r")`, `open(path, "w")`, and
`open(path, "a")`. New docs and tests should prefer the mode-string form.

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
  let writer = fs::try_open(path, "w").unwrap_or(fs::File::invalid());
  if !writer.is_open() {
    return 1;
  }

  var data = [65 as u8, 66 as u8, 67 as u8];
  if writer.write_bytes(data.as_slice()) != 3 {
    writer.close();
    return 2;
  }
  writer.close();

  let reader = fs::try_open(path, "r").unwrap_or(fs::File::invalid());
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
let maybe_file = fs::try_open("missing-file.txt", "r");
if maybe_file.is_none() {
  return 0;
}
```

Append to an existing file:

```ari
let appender = fs::try_open(path, "a").unwrap_or(fs::File::invalid());
if appender.is_open() {
  appender.write_byte(10 as u8);
  appender.close();
}
```

Open an existing file for both reading and writing:

```ari
let file = fs::try_open(path, "rw").unwrap_or(fs::File::invalid());
if file.is_open() {
  let first = file.read_byte();
  file.write_byte((first + 1) as u8);
  file.close();
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
- The mode-string surface is intentionally small. Use `"rw"` or `"r+"` for
  existing read/write files, `"w+"` for create/truncate read/write, and `"a+"`
  for read/append. More detailed flags belong in a future options API.
- Error details are not surfaced yet. Current APIs expose boolean success or a
  `-1` read sentinel. A future `std::os` or `std::io` error value can carry
  platform error codes.

## Tests

```text
tests/cases/standard-library/ok/fs/std-fs-basic.ari
tests/cases/standard-library/ok/fs/std-fs-append.ari
tests/cases/standard-library/ok/fs/std-fs-open-modes.ari
```

`make check-prelude` emits LLVM for the runtime hooks, checks the C syscall
declarations, runs the executable, and cleans up the temporary file under
`build/prelude/`. `std-fs-basic.ari` covers existence, create/truncate writes,
reads, byte-slice writes, close, removal, and `Option[File]` read/write opens.
`std-fs-append.ari` covers append-mode open, preservation of existing bytes,
and failed append opens through `Option[File]`. `std-fs-open-modes.ari` covers
the mode-string contract, including `"rw"`, `"r+"`, `"w+"`, `"a+"`, empty modes,
and invalid mode strings.

## Next Work

- Add file metadata and directory iteration as separate tested slices.
- Add path helpers after Ari has a clearer owned string/path story.
- Add an `OpenOptions`-style builder after Ari has a clearer owned
  OS-resource story and enough named flags to justify it.
- Promote `File` toward an owned resource handle when the compiler can express
  OS-resource ownership and drop policy cleanly.
