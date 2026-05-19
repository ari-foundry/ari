# std::fs

`std::fs` is Ari's portable filesystem module. It exists so ordinary programs
can work with files without reaching directly for raw syscalls or C ABI
declarations. The first slices are deliberately small: check whether a path
exists, open a byte stream with a compact mode string, read and write bytes,
write or append a byte slice in one call, create/truncate/copy small files,
read a whole file into Ari's byte-oriented `String`, rename paths, create hard
or symbolic links, create or remove one empty directory, close the handle, and
remove a file.

The public names stay natural because the module path already says the domain:
use `open(path, mode)`, `try_open(path, mode)`, `create`, `try_create`,
`read_byte`, `write_byte`, `write_bytes`, `read`, `write`, `append`,
`truncate`, `copy`, `rename`, `hard_link`, `symbolic_link`, `create_dir`,
`remove_dir`, `close`, `exists`, and `remove`, not type-suffixed names.

## API

```ari
fs::exists(path)
fs::remove(path)
fs::rename(source, target)
fs::hard_link(existing, link_path)
fs::symbolic_link(target, link_path)
fs::create_dir(path)
fs::remove_dir(path)
fs::open(path, mode)
fs::create(path)
fs::open_read(path)
fs::open_write(path)
fs::open_append(path)
fs::try_open(path, mode)
fs::try_create(path)
fs::try_open_read(path)
fs::try_open_write(path)
fs::try_open_append(path)
fs::close(file)
fs::read_byte(file)
fs::write_byte(file, value)
fs::write_bytes(file, values)
fs::read(ref mut zone, path)
fs::write(path, values)
fs::append(path, values)
fs::truncate(path)
fs::copy(source, target)
fs::read_to_string(ref mut zone, path)

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

`create(path)` is the natural spelling for `open(path, "w")`: it creates a
file if needed and truncates existing contents. `try_create(path)` wraps that
same operation in `Option[File]`.

`open_read`, `open_write`, `open_append`, and their `try_open_*` partners are
compatibility wrappers over `open(path, "r")`, `open(path, "w")`, and
`open(path, "a")`. New docs and tests should usually prefer the mode-string
form or `create`/`try_create` when creating a file.

`read_byte(file)` returns the next byte as `i64` or `-1` at EOF or on failure.
`write_byte(file, value)` returns whether exactly one byte was written.
`write_bytes(file, values)` writes each byte in a `Slice[u8]` until one write
fails and returns the number of bytes written before that failure.

`write(path, values)` opens `path` with `"w"`, writes the whole `Slice[u8]`,
closes the handle, and returns whether the complete write and close succeeded.
`append(path, values)` does the same with `"a"` mode. Both helpers are source
Ari over `try_open`, `File.write_bytes`, and `File.close`.

`read_to_string(ref mut zone, path)` opens `path` with `"r"`, reads bytes until
the current `read_byte` EOF/failure sentinel, closes the handle, and returns a
zone-backed `std::string::String`. The current Ari `String` is byte-oriented;
this helper does not validate UTF-8 or any other text encoding. A missing or
unopenable file returns an empty `String`, so use `fs::exists(path)` or
`fs::try_open(path, "r")` first when absence must be distinguished from an
empty file.

`read(ref mut zone, path)` is the short natural alias for
`read_to_string(ref mut zone, path)`. It is still byte-oriented and returns the
same `std::string::String` handle.

`truncate(path)` creates the file if needed, truncates it to empty, closes the
handle, and returns whether the operation and close succeeded.

`copy(source, target)` opens `source` for reading, opens `target` with
truncating `"w"` semantics, streams bytes from one handle to the other, closes
both handles, and returns whether the copy and both closes succeeded. It is
source Ari over the current byte APIs. Detailed read/write error reporting is
future `std::io` or `std::os` work because `read_byte` still uses one
EOF/failure sentinel.

`close(file)` returns whether the host accepted the close request. Closing an
invalid handle returns `false`. The current first slice does not mutate the
`File` value after closing, so do not reuse a copied handle after `close`.

`exists(path)` checks whether the path exists. `remove(path)` removes a file
path and returns whether the host accepted the request.

`rename(source, target)` asks the host to move or rename one path to another.
On the current Linux/glibc runtime path this follows host `rename` behavior,
including replacing some existing targets when the OS allows it. Portable
overwrite policy is still a future documentation point, so tests use a missing
target path.

`hard_link(existing, link_path)` creates a new hard link to an existing file
and returns whether the host accepted the request. The destination path must
not already exist. This is a thin runtime hook over the host filesystem; cross
filesystem limitations and platform differences follow the OS.

`symbolic_link(target, link_path)` creates a symbolic link at `link_path` that
points at `target`. The destination path must not already exist. On the current
Linux/glibc runtime path this is a direct `symlink` wrapper. Relative targets
are resolved by the host relative to the directory containing `link_path`, not
the caller's current directory. Windows-specific file-vs-directory symlink
behavior remains future platform work.

`create_dir(path)` creates one directory with the current default permission
mode used by Ari's runtime shim. It does not create parent directories.
`remove_dir(path)` removes one empty directory. Recursive directory creation,
recursive removal, and directory iteration are separate future slices.

## Feature Status

| Need | Status |
| --- | --- |
| open | Current: `open(path, mode)`, `try_open(path, mode)`, and wrappers. |
| create | Current: `create(path)` and `try_create(path)` over `"w"` mode. |
| read | Current: byte `read_byte`, whole-file `read`, and `read_to_string`. |
| write | Current: byte `write_byte`, `write_bytes`, and whole-file `write`. |
| append | Current: `"a"`/`"a+"` modes and whole-file `append`. |
| truncate | Current: `truncate(path)` and `"w"`/`"w+"` modes. |
| metadata | Roadmap: needs a portable `Metadata` value and runtime `stat`/platform wrappers. |
| permissions | Roadmap: should be a small permissions value built on metadata/chmod hooks. |
| rename | Current: `rename(source, target)` hook; portable overwrite policy is roadmap. |
| remove | Current: file removal with `remove(path)` and empty directory removal with `remove_dir(path)`. |
| copy | Current: source streaming `copy(source, target)` for byte files. |
| hard link | Current: `hard_link(existing, link_path)` runtime hook. |
| symbolic link | Current: `symbolic_link(target, link_path)` runtime hook on the Linux/glibc path; Windows split is roadmap. |
| canonicalize | Roadmap: runtime path resolution returning an owned Ari string/path. |
| read directory | Roadmap: `DirEntry`/iterator handle and OS-resource ownership policy. |
| create directory | Current: single-directory `create_dir(path)`; recursive creation is roadmap. |
| temporary files | Roadmap: secure temp file/dir constructors after owned handles and paths. |
| path manipulation | Current: source lexical helpers in `std::path`; owned `Path`/`PathBuf` and platform-specific paths are roadmap. |
| file locking | Optional roadmap: advisory locking after platform behavior is documented. |

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

Use the whole-file byte helpers for small files:

```ari
var data = [65 as u8, 66 as u8];
fs::write(path, data.as_slice());

var tail = [67 as u8];
fs::append(path, tail.as_slice());

var zone = zone::create(512);
let text = fs::read(ref mut zone, path);
let first = text.get(0);
zone::destroy(zone);
```

Create, copy, and truncate small files:

```ari
let file = fs::try_create(path).unwrap_or(fs::File::invalid());
if file.is_open() {
  file.close();
}

let backup_path = "build/prelude/example-fs.bak";
fs::copy(path, backup_path);
fs::truncate(path);
```

Rename a file and create a single empty directory:

```ari
let moved = "build/prelude/example-fs-moved.tmp";
if fs::rename(path, moved) {
  fs::remove(moved);
}

let dir = "build/prelude/example-fs-dir.tmp";
if fs::create_dir(dir) {
  fs::remove_dir(dir);
}
```

Create hard and symbolic links to a file:

```ari
let hard = "build/prelude/example-fs-hard.tmp";
let symbolic = "build/prelude/example-fs-symbolic.tmp";
let symbolic_target = "example-fs.tmp";
if fs::hard_link(path, hard) {
  fs::remove(hard);
}
if fs::symbolic_link(symbolic_target, symbolic) {
  fs::remove(symbolic);
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
  iteration, metadata, permissions API, canonicalization, recursive directory
  API, file locking API, temporary-file API, or text encoding policy yet. Link
  creation exists, but richer link metadata and platform-specific symlink
  policy are still future work.
- Runtime hooks currently target the Linux/glibc LLVM path through `access`,
  `unlink`, `rename`, `link`, `symlink`, `mkdir`, `rmdir`, `open`, `read`,
  `write`, and `close`.
- `File` is not a tracked `own` resource yet. The caller must close each
  successful handle exactly once by convention. Future ownership work should
  make OS resources harder to copy and accidentally double-close.
- The mode-string surface is intentionally small. Use `"rw"` or `"r+"` for
  existing read/write files, `"w+"` for create/truncate read/write, and `"a+"`
  for read/append. More detailed flags belong in a future options API.
- Error details are not surfaced yet. Current APIs expose boolean success, an
  empty-string read fallback, or a `-1` read sentinel. A future `std::os` or
  `std::io` error value can carry platform error codes.

## Tests

```text
tests/cases/standard-library/ok/fs/std-fs-basic.ari
tests/cases/standard-library/ok/fs/std-fs-append.ari
tests/cases/standard-library/ok/fs/std-fs-open-modes.ari
tests/cases/standard-library/ok/fs/std-fs-read-write.ari
tests/cases/standard-library/ok/fs/std-fs-create-truncate-copy.ari
tests/cases/standard-library/ok/fs/std-fs-rename-dir.ari
tests/cases/standard-library/ok/fs/std-fs-links.ari
```

`make check-prelude` emits LLVM for the runtime hooks, checks the C syscall
declarations, runs the executable, and cleans up the temporary file under
`build/prelude/`. `std-fs-basic.ari` covers existence, create/truncate writes,
reads, byte-slice writes, close, removal, and `Option[File]` read/write opens.
`std-fs-append.ari` covers append-mode open, preservation of existing bytes,
and failed append opens through `Option[File]`. `std-fs-open-modes.ari` covers
the mode-string contract, including `"rw"`, `"r+"`, `"w+"`, `"a+"`, empty modes,
and invalid mode strings. `std-fs-read-write.ari` covers source whole-file
write, append, read-to-byte-string, missing-file empty reads, and truncating
rewrite behavior. `std-fs-create-truncate-copy.ari` covers source `create`,
`try_create`, `read`, `truncate`, missing-source copy failure, and whole-file
copy behavior. `std-fs-rename-dir.ari` covers runtime-backed `rename`,
`create_dir`, and `remove_dir` behavior. `std-fs-links.ari` covers
runtime-backed `hard_link` and `symbolic_link` behavior plus read-through
checks.

## Next Work

- Add `metadata(path)`, `permissions(path)`, and permission mutation as a
  separate tested runtime slice.
- Add explicit overwrite/platform policy for `rename`.
- Add canonicalization as an OS-wrapper slice returning an owned Ari string or
  path value.
- Expand link support with metadata/readlink helpers and clearer
  platform-specific symlink policy.
- Add directory iteration after Ari can represent owned OS-resource iterator
  handles.
- Add recursive directory creation/removal after the single-directory hooks
  have stable policy.
- Grow `std::path` from lexical helpers into owned path values after Ari has a
  clearer owned string/path story.
- Add secure temporary files and optional file locking after the resource and
  platform policy is documented.
- Add an `OpenOptions`-style builder after Ari has a clearer owned
  OS-resource story and enough named flags to justify it.
- Promote `File` toward an owned resource handle when the compiler can express
  OS-resource ownership and drop policy cleanly.
