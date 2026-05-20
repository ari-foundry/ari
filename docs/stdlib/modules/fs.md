# std::fs

`std::fs` is Ari's portable filesystem module. It exists so ordinary programs
can work with files without reaching directly for raw syscalls or C ABI
declarations. The first slices are deliberately small: check whether a path
exists, query access-style read/write/execute permissions, open a byte stream
with a compact mode string, read and write bytes, write or append a byte slice
in one call, create/truncate/copy small files, read a whole file into Ari's
byte-oriented `String` with either compatibility or `Option`-returning absence
behavior, rename paths, create hard or symbolic links, create or remove one
empty directory, read directory entry names through an explicit `Dir` handle,
query basic file metadata, read and change POSIX permission bits, resolve an
existing path to an absolute canonical path, close the handle, and remove a
file.

The public names stay natural because the module path already says the domain:
use `open(path, mode)`, `try_open(path, mode)`, `create`, `try_create`,
`can_read`, `can_write`, `can_execute`, `permissions`, `read_byte`,
`write_byte`, `write_bytes`, `read`, `try_read`, `write`, `append`,
`try_write`, `try_append`, `truncate`, `copy`, `try_copy`, `metadata`,
`try_metadata`, `mode`, `try_mode`, `set_mode`, `set_permissions`,
`canonicalize`, `try_canonicalize`, `rename`, `hard_link`,
`symbolic_link`, `create_dir`, `remove_dir`, `try_open_dir`, `read_dir`,
`try_read_dir`, `read_dir_next`, `close_dir`, `close`, `exists`, and
`remove`, not type-suffixed names.

## API

```ari
fs::exists(path)
fs::can_read(path)
fs::can_write(path)
fs::can_execute(path)
fs::permissions(path)
fs::metadata(path)
fs::try_metadata(path)
fs::mode(path)
fs::try_mode(path)
fs::set_mode(path, mode)
fs::set_permissions(path, permissions)
fs::canonicalize(ref mut zone, path)
fs::try_canonicalize(ref mut zone, path)
fs::remove(path)
fs::rename(source, target)
fs::hard_link(existing, link_path)
fs::symbolic_link(target, link_path)
fs::create_dir(path)
fs::remove_dir(path)
fs::open_dir(path)
fs::try_open_dir(path)
fs::read_dir(ref mut zone, path)
fs::try_read_dir(ref mut zone, path)
fs::read_dir_next(ref mut zone, dir)
fs::close_dir(dir)
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
fs::try_read(ref mut zone, path)
fs::write(path, values)
fs::try_write(path, values)
fs::append(path, values)
fs::try_append(path, values)
fs::truncate(path)
fs::copy(source, target)
fs::try_copy(source, target)
fs::read_to_string(ref mut zone, path)
fs::try_read_to_string(ref mut zone, path)

File::invalid()
file.is_open()
file.close()
file.read_byte()
file.write_byte(value)
file.write_bytes(values)

Dir::invalid()
dir.is_open()
dir.next(ref mut zone)
dir.close()

Permissions::none()
Permissions::read_only()
Permissions::all()
permissions.can_read()
permissions.can_write()
permissions.can_execute()
permissions.any()
permissions.to_mode()

metadata.len()
metadata.file_type()
metadata.permissions()
metadata.is_file()
metadata.is_dir()
metadata.is_symlink()
metadata.is_other()
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

`try_write(path, values)` opens `path` with `"w"`, writes the whole
`Slice[u8]`, closes the handle, and returns `Some(byte_count)` when the
complete write and close succeeded. `try_append(path, values)` does the same
with `"a"` mode. Failed opens, short writes, or failed closes return `None`.

`write(path, values)` and `append(path, values)` are compatibility boolean
wrappers over `try_write` and `try_append`.

`try_read_to_string(ref mut zone, path)` opens `path` with `"r"`, reads bytes
until the current `read_byte` EOF/failure sentinel, closes the handle, and
returns `Some[String]`. Missing or unopenable files return `None`, so this is
the preferred whole-file helper when absence must be distinguished from an empty
file. The current Ari `String` is byte-oriented; this helper does not validate
UTF-8 or any other text encoding.

`try_read(ref mut zone, path)` is the short natural alias for
`try_read_to_string(ref mut zone, path)`.

`read_to_string(ref mut zone, path)` keeps the original compatibility behavior:
it returns the file bytes on success and an empty `String` when the file cannot
be opened. Prefer `try_read_to_string` for new code that handles ordinary
filesystem failure.

`read(ref mut zone, path)` is the short natural alias for
`read_to_string(ref mut zone, path)`. It is still byte-oriented and returns the
same `std::string::String` handle.

`truncate(path)` creates the file if needed, truncates it to empty, closes the
handle, and returns whether the operation and close succeeded.

`try_copy(source, target)` opens `source` for reading, opens `target` with
truncating `"w"` semantics, streams bytes from one handle to the other, closes
both handles, and returns `Some(byte_count)` when the copy and both closes
succeeded. Missing sources, failed target opens, failed byte writes, and failed
closes return `None`. Detailed read error reporting remains future `std::io` or
`std::os` work because `read_byte` still uses one EOF/failure sentinel.

`copy(source, target)` is the compatibility boolean wrapper over `try_copy`.

`close(file)` returns whether the host accepted the close request. Closing an
invalid handle returns `false`. The current first slice does not mutate the
`File` value after closing, so do not reuse a copied handle after `close`.

`exists(path)` checks whether the path exists. `remove(path)` removes a file
path and returns whether the host accepted the request.

`can_read(path)`, `can_write(path)`, and `can_execute(path)` ask the host
whether the current process can read, write, or execute/search the path. These
are access checks, not security guarantees: the filesystem can change between
the check and the later operation. Use them for friendly preflight behavior and
diagnostics while still handling later open/read/write failures.

`permissions(path)` groups the same three checks into a small `Permissions`
value. `permissions.can_read()`, `permissions.can_write()`, and
`permissions.can_execute()` expose the stored booleans, and `permissions.any()`
is useful for treating a missing or inaccessible path as no visible access.
`Permissions::none()` creates that all-false value directly.
`Permissions::read_only()` and `Permissions::all()` are convenience
constructors for common chmod-style updates, and `permissions.to_mode()`
converts the three booleans into POSIX permission bits. The conversion applies
the same read/write/execute choice to user, group, and other bits, so
read-only becomes `0444` and all-permissions becomes `0777`.

`try_metadata(path)` returns `Option[Metadata]`. Missing paths and failed host
metadata lookups return `None`; successful calls snapshot the byte length,
`FileKind`, and the same access-style `Permissions` used by
`permissions(path)`. `metadata(path)` is the asserting convenience wrapper for
programs that treat a missing path as a programmer error.

`Metadata::len()` returns the byte length reported by the host. Directories and
special files can have host-specific sizes; only regular-file sizes should be
used as portable byte counts. `metadata.file_type()` returns a `FileKind` enum
with `Regular`, `Directory`, `Symlink`, and `Other` variants. The convenience
predicates `is_file`, `is_dir`, `is_symlink`, and `is_other` cover common
branches. The current Linux/glibc runtime uses `stat`, so `metadata` follows
symbolic links; a separate no-follow `symlink_metadata` helper is future work.

`try_mode(path)` returns the current POSIX permission bits as
`Option[i64]`. The value is already masked to the low `0777` permission bits,
so code can compare it directly with familiar octal-style values written as
decimal literals today: `420` for `0644`, `292` for `0444`, and `511` for
`0777`. `mode(path)` is the asserting convenience wrapper. `set_mode(path,
mode)` uses the host `chmod` path and rejects values outside `0..511`.
`set_permissions(path, permissions)` is the structured version for code that
already has a `Permissions` value.

`try_canonicalize(ref mut zone, path)` resolves an existing path through the
host filesystem and returns an owned absolute `String` in the caller-provided
zone. Missing paths, permission failures, and paths that cannot be resolved
return `None`. `canonicalize(ref mut zone, path)` is the asserting convenience
wrapper for code that treats an unresolvable path as a programmer error. The
current Linux/glibc runtime uses `realpath`, so it follows symbolic links and
requires the path to exist.

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
`remove_dir(path)` removes one empty directory.

`try_open_dir(path)` opens one directory and returns `Option[Dir]`.
`dir.next(ref mut zone)` returns the next entry name as `Option[String]`,
skipping the host `"."` and `".."` entries. `dir.close()` closes the directory
handle. `try_read_dir(ref mut zone, path)` is the convenient one-shot helper:
it opens the directory, collects names into `std::vec::Vec[String]`, closes the
handle, and returns `None` when the directory cannot be opened or closed.
`read_dir(ref mut zone, path)` is the asserting wrapper for code that treats a
failed directory read as a programmer error. The low-level `open_dir`,
`read_dir_next`, and `close_dir` names exist for direct runtime-hook coverage,
but ordinary code should prefer `try_read_dir` for collection-style reads or
`try_open_dir` plus the `Dir` methods for manual streaming. Current directory
reads return names only; richer `DirEntry` metadata, recursive helpers, and
owned resource tracking are future slices.

## Feature Status

| Need | Status |
| --- | --- |
| open | Current: `open(path, mode)`, `try_open(path, mode)`, and wrappers. |
| create | Current: `create(path)` and `try_create(path)` over `"w"` mode. |
| read | Current: byte `read_byte`, whole-file `read`/`read_to_string`, and fallible `try_read`/`try_read_to_string`. |
| write | Current: byte `write_byte`, `write_bytes`, whole-file `write`, and byte-counting `try_write`. |
| append | Current: `"a"`/`"a+"` modes, whole-file `append`, and byte-counting `try_append`. |
| truncate | Current: `truncate(path)` and `"w"`/`"w+"` modes. |
| metadata | Current: `try_metadata(path)`/`metadata(path)`, `Metadata`, and `FileKind` over the Linux/glibc `stat` runtime path; no-follow symlink metadata and richer timestamps are roadmap. |
| permissions | Current: access-style `can_read`, `can_write`, `can_execute`, `permissions`, stat-backed `try_mode`/`mode`, and chmod-backed `set_mode`/`set_permissions`; richer ACL/owner/group policy is roadmap. |
| rename | Current: `rename(source, target)` hook; portable overwrite policy is roadmap. |
| remove | Current: file removal with `remove(path)` and empty directory removal with `remove_dir(path)`. |
| copy | Current: source streaming `copy(source, target)` and byte-counting `try_copy(source, target)` for byte files. |
| hard link | Current: `hard_link(existing, link_path)` runtime hook. |
| symbolic link | Current: `symbolic_link(target, link_path)` runtime hook on the Linux/glibc path; Windows split is roadmap. |
| canonicalize | Current: `try_canonicalize(ref mut zone, path)` and asserting `canonicalize(ref mut zone, path)` over the Linux/glibc `realpath` runtime path. |
| read directory | Current: `try_read_dir(ref mut zone, path)`, `read_dir(ref mut zone, path)`, `try_open_dir(path)`, `Dir`, `dir.next(ref mut zone)`, and `dir.close()` for entry names; richer `DirEntry` metadata and owned OS-resource policy are roadmap. |
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

  var data = ['A', 'B', 'C'];
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
  appender.write_byte('\n');
  appender.close();
}
```

Use the whole-file byte helpers for small files:

```ari
var data = ['A', 'B'];
fs::write(path, data.as_slice());

var tail = ['C'];
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

Read the names in one directory:

```ari
var zone = zone::create(512);
let names = fs::try_read_dir(ref mut zone, "build/prelude").unwrap_or(
  std::vec::new<std::string::String>(ref mut zone, 0)
);

var index = 0;
while index < names.len() {
  let name = names.get(index);
  if name.equals_text("example-fs.tmp") {
    index = names.len();
  }
  index = index + 1;
}
zone::destroy(zone);
```

Manually stream a directory when you do not want to collect every name:

```ari
var zone = zone::create(512);
let dir = fs::try_open_dir("build/prelude").unwrap_or(fs::Dir::invalid());
if dir.is_open() {
  var reading = true;
  while reading {
    match dir.next(ref mut zone) {
      std::Some(name) => {
        if name.equals_text("example-fs.tmp") {
          reading = false;
        }
      }
      std::None => {
        reading = false;
      }
    }
  }
  dir.close();
}
zone::destroy(zone);
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

Check current-process access permissions:

```ari
let access = fs::permissions(path);
if access.can_read() && access.can_write() {
  let file = fs::try_open(path, "rw").unwrap_or(fs::File::invalid());
  if file.is_open() {
    file.close();
  }
}
```

Change POSIX permission bits:

```ari
let read_only = fs::Permissions::read_only();
if fs::set_permissions(path, read_only) {
  let bits = fs::mode(path);
  if bits == 292 {
    return 0;
  }
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

- This slice is byte-oriented. There is no owned path type, recursive directory
  API, file locking API, temporary-file API, or text encoding policy yet.
  Directory reads currently return entry names only and do not expose metadata
  or per-entry errors. Basic `stat` metadata, permission-bit lookup and
  mutation, and existing-path canonicalization exist, but richer timestamps,
  owner/group/ACL policy, no-follow symlink metadata, link metadata, and
  platform-specific symlink policy are still future work.
- Runtime hooks currently target the Linux/glibc LLVM path through `access`,
  `stat`, `chmod`, `realpath`, `unlink`, `rename`, `link`, `symlink`,
  `mkdir`, `rmdir`, `opendir`, `readdir`, `closedir`, `open`, `read`,
  `write`, and `close`; the one-shot `read_dir` helpers are source Ari over
  those hooks.
- `File` and `Dir` are not tracked `own` resources yet. The caller must close
  each successful handle exactly once by convention. Future ownership work
  should make OS resources harder to copy and accidentally double-close.
- The mode-string surface is intentionally small. Use `"rw"` or `"r+"` for
  existing read/write files, `"w+"` for create/truncate read/write, and `"a+"`
  for read/append. More detailed flags belong in a future options API.
- Error details are not surfaced from `std::fs` yet. Current APIs expose
  boolean success, `Option` for fallible whole-file reads and opens, an
  empty-string read fallback for compatibility, or a `-1` read sentinel. New
  richer APIs should use `std::error::Error` instead of growing more sentinel
  conventions.

## Tests

```text
tests/cases/standard-library/ok/fs/std-fs-basic.ari
tests/cases/standard-library/ok/fs/std-fs-append.ari
tests/cases/standard-library/ok/fs/std-fs-open-modes.ari
tests/cases/standard-library/ok/fs/std-fs-read-write.ari
tests/cases/standard-library/ok/fs/std-fs-try-read.ari
tests/cases/standard-library/ok/fs/std-fs-create-truncate-copy.ari
tests/cases/standard-library/ok/fs/std-fs-rename-dir.ari
tests/cases/standard-library/ok/fs/std-fs-read-dir.ari
tests/cases/standard-library/ok/fs/std-fs-links.ari
tests/cases/standard-library/ok/fs/std-fs-permissions.ari
tests/cases/standard-library/ok/fs/std-fs-mode.ari
tests/cases/standard-library/ok/fs/std-fs-metadata.ari
tests/cases/standard-library/ok/fs/std-fs-canonicalize.ari
```

`make check-prelude` emits LLVM for the runtime hooks, checks the C syscall
declarations, runs the executable, and cleans up the temporary file under
`build/prelude/`. `std-fs-basic.ari` covers existence, create/truncate writes,
reads, byte-slice writes, close, removal, and `Option[File]` read/write opens.
`std-fs-append.ari` covers append-mode open, preservation of existing bytes,
and failed append opens through `Option[File]`. `std-fs-open-modes.ari` covers
the mode-string contract, including `"rw"`, `"r+"`, `"w+"`, `"a+"`, empty modes,
and invalid mode strings. `std-fs-read-write.ari` covers source whole-file
write/append compatibility wrappers, `try_write`/`try_append` byte counts,
read-to-byte-string, missing-file empty reads, and truncating rewrite behavior.
`std-fs-try-read.ari` covers `Option[String]` whole-file
reads where missing files become `None` and empty files stay `Some(empty)`.
`std-fs-create-truncate-copy.ari` covers source `create`, `try_create`,
`read`, `truncate`, missing-source copy failure, whole-file copy behavior, and
`try_copy` byte counts.
`std-fs-rename-dir.ari` covers runtime-backed `rename`,
`create_dir`, and `remove_dir` behavior. `std-fs-read-dir.ari` covers
runtime-backed `Dir` open/next/close behavior, one-shot
`try_read_dir`/`read_dir` name-list helpers, missing-directory failure, dot
entry skipping, invalid-handle `None`, and cleanup. `std-fs-links.ari` covers
runtime-backed `hard_link` and `symbolic_link` behavior plus read-through
checks. `std-fs-permissions.ari` covers access-style readable/writable/
executable checks, the `Permissions` wrapper methods, and missing-path
all-false behavior. `std-fs-mode.ari` covers stat-backed mode lookup,
chmod-backed mutation calls, `Permissions` constructors, mode conversion,
invalid mode rejection, missing-path failure, and cleanup restoration without
depending on exact chmod effects from the host filesystem.
`std-fs-metadata.ari` covers `Option[Metadata]`,
regular-file byte length, `FileKind`, directory predicates, and missing-path
`None`. `std-fs-canonicalize.ari` covers `Option[String]` path resolution,
absolute canonical paths, filename preservation, and missing-path `None`.

## Next Work

- Add richer permission metadata for owner/group/ACL policy.
- Add explicit overwrite/platform policy for `rename`.
- Grow canonicalization toward owned path values and platform-specific policy.
- Expand link support with metadata/readlink helpers and clearer
  platform-specific symlink policy.
- Expand metadata with modified/accessed/created timestamps and a no-follow
  `symlink_metadata` helper.
- Expand directory reads with `DirEntry` metadata, richer error reporting, and
  owned OS-resource iterator handles.
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
