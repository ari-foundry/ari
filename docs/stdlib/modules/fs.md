# std::fs

`std::fs` is Ari's portable filesystem module. It exists so ordinary programs
can work with files without reaching directly for raw syscalls or C ABI
declarations. The first slices are deliberately small: check whether a path
exists, query access-style read/write/execute permissions, open a byte stream
with a compact mode string, read and write bytes, write or append a byte slice
in one call, create/truncate/copy small files, read a whole file into Ari's
byte-oriented `String` with either compatibility or `Option`-returning absence
behavior, rename paths, create hard or symbolic links, read symbolic-link
targets, query no-follow symbolic-link metadata, ensure a single regular file exists without truncating an existing file, create or remove one empty
directory, ensure a single directory exists without treating an existing
directory as failure, recursively create missing directory parents, read
directory entry names through an explicit `Dir` handle, collect `DirEntry`
values with names, joined paths, lazy metadata helpers, and `Error`-returning
directory query forms, query basic file metadata with `Option` or
`Result[..., Error]`, ask direct path-kind predicates such as `is_file`
and `is_dir`, read and change POSIX permission bits, resolve an existing path
to an absolute canonical path with `Option` or `Result` failure details, pass `File` handles to generic `std::io::Reader`
and `std::io::Writer` helpers, inspect and move a file cursor through
`std::io::Seek`, close the handle, and remove a file.

The public names stay natural because the module path already says the domain:
use `open(path, mode)`, `open_optional(path, mode)`, `try_open(path, mode)`,
`create`, `create_optional`, `try_create`,
`can_read`, `can_write`, `can_execute`, `permissions`, `read_byte`,
`try_read_byte`, `write_byte`, `write_bytes`, `read`, `read_result`,
`try_read`, `read_to_string_result`, `write`,
`write_result`, `append`, `append_result`, `try_write`, `try_append`,
`truncate`, `copy`, `copy_result`, `try_copy`, `metadata`,
`metadata_result`, `try_metadata`, `symlink_metadata`,
`symlink_metadata_result`, `try_symlink_metadata`,
`file_type_result`, `try_file_type`, `is_file`, `is_dir`, `is_symlink`,
`is_other`, `mode`, `mode_result`, `try_mode`, `set_mode`, `set_permissions`,
`canonicalize`, `canonicalize_result`, `try_canonicalize`, `rename`, `hard_link`,
`symbolic_link`, `read_link`, `read_link_result`, `try_read_link`, `ensure_file`, `create_dir`, `ensure_dir`, `remove_dir`, `remove_dir_all`,
`create_dir_all`, `ensure_dir_all`, `open_dir_result`, `try_open_dir`, `read_dir`,
`read_dir_result`, `try_read_dir`, `read_dir_entries`,
`read_dir_entries_result`, `try_read_dir_entries`,
`read_dir_next`, `position`, `seek`, `close_dir`, `close`, `exists`, and
`remove`, not type-suffixed names. For the handle APIs touched by lifecycle
cleanup, natural names return `Result`; `_optional` and `try_*` helpers keep
absence-only compatibility, `_unchecked` helpers keep the old boolean,
sentinel, or invalid-handle behavior, and `_raw` helpers keep compact integer
errors.

## API

```ari
fs::Error
fs::ErrorKind
fs::exists(path)
fs::can_read(path)
fs::can_write(path)
fs::can_execute(path)
fs::permissions(path)
fs::metadata(path)
fs::metadata_raw_result(path)
fs::metadata_result(path)
fs::try_metadata(path)
fs::symlink_metadata(path)
fs::symlink_metadata_raw_result(path)
fs::symlink_metadata_result(path)
fs::try_symlink_metadata(path)
fs::file_type_raw_result(path)
fs::file_type_result(path)
fs::try_file_type(path)
fs::is_file(path)
fs::is_dir(path)
fs::is_symlink(path)
fs::is_other(path)
fs::mode(path)
fs::mode_raw_result(path)
fs::mode_result(path)
fs::try_mode(path)
fs::set_mode(path, mode)
fs::set_permissions(path, permissions)
fs::canonicalize(ref mut zone, path)
fs::canonicalize_result(ref mut zone, path)
fs::try_canonicalize(ref mut zone, path)
fs::remove(path)
fs::rename(source, target)
fs::hard_link(existing, link_path)
fs::symbolic_link(target, link_path)
fs::read_link(ref mut zone, path)
fs::read_link_result(ref mut zone, path)
fs::try_read_link(ref mut zone, path)
fs::ensure_file(path)
fs::create_dir(path)
fs::create_dir_raw_result(path)
fs::create_dir_result(path)
fs::ensure_dir(path)
fs::create_dir_all(path)
fs::ensure_dir_all(path)
fs::remove_dir(path)
fs::remove_dir_raw_result(path)
fs::remove_dir_result(path)
fs::remove_dir_all(path)
fs::open_dir(path)
fs::open_dir_raw_result(path)
fs::open_dir_result(path)
fs::try_open_dir(path)
fs::read_dir(ref mut zone, path)
fs::read_dir_result(ref mut zone, path)
fs::try_read_dir(ref mut zone, path)
fs::read_dir_entries(ref mut zone, path)
fs::read_dir_entries_result(ref mut zone, path)
fs::try_read_dir_entries(ref mut zone, path)
fs::read_dir_next(ref mut zone, dir)
fs::close_dir(dir)
fs::close_dir_raw(dir)
fs::close_dir_unchecked(dir)
fs::open(path, mode)
fs::open_optional(path, mode)
fs::open_unchecked(path, mode)
fs::create(path)
fs::create_optional(path)
fs::create_unchecked(path)
fs::remove_raw_result(path)
fs::remove_result(path)
fs::rename_raw_result(source, target)
fs::rename_result(source, target)
fs::open_read(path)
fs::open_write(path)
fs::open_append(path)
fs::open_read_optional(path)
fs::open_write_optional(path)
fs::open_append_optional(path)
fs::open_read_unchecked(path)
fs::open_write_unchecked(path)
fs::open_append_unchecked(path)
fs::try_open(path, mode)
fs::try_create(path)
fs::try_open_read(path)
fs::try_open_write(path)
fs::try_open_append(path)
fs::close(file)
fs::close_raw(file)
fs::close_unchecked(file)
fs::read_byte(file)
fs::try_read_byte(file)
fs::write_byte(file, value)
fs::write_byte_raw(file, value)
fs::write_byte_unchecked(file, value)
fs::write_bytes(file, values)
fs::write_bytes_raw(file, values)
fs::write_bytes_unchecked(file, values)
fs::position(file)
fs::position_raw(file)
fs::position_or(file, fallback)
fs::seek(file, position)
fs::seek_raw(file, position)
fs::seek_unchecked(file, position)
fs::read(ref mut zone, path)
fs::read_result(ref mut zone, path)
fs::try_read(ref mut zone, path)
fs::write(path, values)
fs::write_raw_result(path, values)
fs::write_result(path, values)
fs::try_write(path, values)
fs::append(path, values)
fs::append_raw_result(path, values)
fs::append_result(path, values)
fs::try_append(path, values)
fs::truncate(path)
fs::copy(source, target)
fs::copy_raw_result(source, target)
fs::copy_result(source, target)
fs::try_copy(source, target)
fs::read_to_string(ref mut zone, path)
fs::read_to_string_result(ref mut zone, path)
fs::try_read_to_string(ref mut zone, path)

fs::open_raw_result(path, mode)
fs::open_result(path, mode)
fs::create_raw_result(path)
fs::create_result(path)
fs::open_options()
OpenOptions::new()
options.read(enabled)
options.write(enabled)
options.append(enabled)
options.truncate(enabled)
options.create(enabled)
options.create_new(enabled)
options.open(path)
options.open_optional(path)
options.open_unchecked(path)
options.open_raw_result(path)
options.open_result(path)
options.try_open(path)

File::invalid()
file.descriptor()
file.is_open()
file.close()
file.close_unchecked()
file.read_byte()
file.try_read_byte()
file.write_byte(value)
file.write_byte_unchecked(value)
file.write_bytes(values)
file.write_bytes_unchecked(values)
file.position()
file.position_or(fallback)
file.seek(position)
file.seek_unchecked(position)

impl std::io::Reader for File
impl std::io::Writer for File
impl std::io::Seek for File

Dir::invalid()
dir.is_open()
dir.next(ref mut zone)
dir.close()
dir.close_unchecked()

entry.name()
entry.path()
entry.name_equals(value)
entry.path_equals(value)
entry.try_metadata()
entry.metadata()
entry.try_symlink_metadata()
entry.symlink_metadata()
entry.try_file_type()
entry.is_file()
entry.is_dir()
entry.is_symlink()
entry.is_other()

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
metadata.accessed()
metadata.modified()
metadata.changed()
metadata.is_file()
metadata.is_dir()
metadata.is_symlink()
metadata.is_other()
```

`File` is a small value handle around the runtime file descriptor. Failed open
operations return an invalid handle where `file.is_open()` is false only
through `_unchecked` compatibility helpers. Prefer `open(path, mode)` for
normal control flow; it returns `Result[File, Error]` and preserves the reason.
Use `open_optional(path, mode)` or `try_open(path, mode)` only when
presence/absence is enough. Use
`file.descriptor()` when code needs a non-owning `std::os::Fd` view over the
handle. That view does not close or extend the lifetime of the file. If code
intentionally takes over close responsibility from a file descriptor, wrap the
raw value with `std::os::OwnedFd::from_raw(file.descriptor().raw())` and do not
also close the original `File` value.

`File` implements `std::io::Reader`, `std::io::Writer`, and `std::io::Seek`,
so file handles can flow through generic IO helpers without a file-specific
suffix:

```ari
var zone = zone::create(512);
var input = fs::try_open("input.txt", "r").unwrap_or(fs::File::invalid());
let text = io::read_to_string<std::fs::File>(ref mut zone, ref mut input);
input.close().unwrap();

var source = fs::try_open("input.txt", "r").unwrap_or(fs::File::invalid());
var target = fs::try_open("output.txt", "w").unwrap_or(fs::File::invalid());
io::copy<std::fs::File, std::fs::File>(ref mut source, ref mut target).unwrap();
source.close().unwrap();
target.close().unwrap();
zone::destroy(zone);
```

The `Writer` `flush` method is currently a direct-descriptor success check:
file writes are not buffered by `File` itself. Use future or explicit buffered
wrappers when the library grows owned buffering policy.

`position(file)` returns `Result[i64, Error]` with the current byte offset from
the host descriptor. `seek(file, position)` moves to an absolute byte offset
from the start of the file and returns `Result[(), Error]`. Invalid handles and
negative seek offsets return `Error(InvalidInput)`, while host seek/tell
failures return errno-derived `Error` values. The `position_or` and
`seek_unchecked` compatibility helpers discard those errors for older sentinel
and boolean call sites. The `std::io::Seek` trait methods currently retain the
older sentinel shape, which keeps generic `S: io::Seek` code readable while the
trait error model is migrated:

```ari
fn rewind[S: io::Seek](stream: ref mut S) -> bool {
  if stream.position() < 0 {
    return false;
  }
  return stream.seek(0);
}
```

The `position_raw(file)` and `seek_raw(file, position)` variants keep the same
policy with raw integer errors for compatibility adapters.

Append-mode handles still follow host append semantics for writes: seeking can
change the read cursor and reported position, but writes on an append descriptor
may still land at the file end.

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

`open(path, mode)` returns `Result[File, Error]`. Invalid or unsupported mode
strings return `InvalidInput`; missing paths and host failures preserve their
`std::error::Error` category. `open_optional(path, mode)` and
`try_open(path, mode)` turn failures into `None`, while `open_unchecked`
returns the old invalid-handle sentinel directly. The Result shape lets
callers branch on
`reason.kind()`, `reason.code()`, or predicates such as `reason.is_not_found()`.
`open_raw_result(path, mode)` remains available for low-level compatibility
tests and FFI-style bridges that still need `Result[File, i64]`.

Use `OpenOptions` when the call site needs named policy instead of a mode
string:

```ari
let options = fs::OpenOptions::new()
  .read(true)
  .write(true)
  .create(true);

match options.open("cache.bin") {
  std::Ok(file) => {
    file.close();
  }
  std::Err(reason) => {
    return reason.code();
  }
}
```

`OpenOptions::new()` starts with every flag disabled. The value-style builder
methods return a new options value, so chained calls stay immutable and easy to
review. `read(true)` enables reads, `write(true)` enables ordinary writes,
`append(true)` makes writes land at the file end, `truncate(true)` truncates an
existing file when it is opened, `create(true)` creates a missing file, and
`create_new(true)` creates exclusively and fails when the path already exists.
`truncate`, `create`, and `create_new` require `write(true)` or `append(true)`;
`append(true).truncate(true)` is rejected because Ari keeps that ambiguous
combination out of the portable surface. `options.open(path)` returns
`Result[File, Error]`, `options.open_optional(path)` and
`options.try_open(path)` turn failures into `None`, and
`options.open_unchecked(path)` returns the old invalid-handle sentinel.
`options.open_result(path)` remains a compatibility alias for callers already
using the older Result-suffixed name.
`options.open_raw_result(path)` is the compatibility form with a raw integer
error payload.

`create(path)` is the natural Result-returning spelling for `open(path, "w")`:
it creates a file if needed and truncates existing contents.
`create_optional(path)` and `try_create(path)` wrap that same operation in
`Option[File]`; `create_unchecked(path)` returns the old invalid-handle
sentinel; `create_result(path)` is a compatibility alias; and
`create_raw_result(path)` keeps the old raw payload shape.

`ensure_file(path)` is the idempotent regular-file helper. It returns `true`
when `path` is already a regular file, creates an empty file when the path is
missing, and returns `false` when another path kind already exists or when a
parent directory is missing. Unlike `create(path)`, it does not truncate an
existing file, so it is the safer setup primitive for tests, caches, and tools
that merely need a file to exist.

`open_read`, `open_write`, and `open_append` are Result-returning convenience
wrappers over `open(path, "r")`, `open(path, "w")`, and `open(path, "a")`.
The matching `_optional`/`try_open_*` helpers discard the reason, while the
matching `_unchecked` helpers preserve the old invalid-handle convention. New
docs and tests should usually prefer `open`, `OpenOptions`, or `create` when
creating a file.

`read_byte(file)` returns the next byte as `i64` or `-1` at EOF or on failure.
Use `try_read_byte(file)` or `file.try_read_byte()` when EOF is ordinary
control flow; they return `Option[u8]` and hide the sentinel from call sites.
`write_byte(file, value)` returns `Result[(), Error]`, and
`write_bytes(file, values)` writes a whole `Slice[u8]` and returns
`Result[i64, Error]` with the byte count. Closed or invalid handles return
`Error(InvalidInput)`, host write failures return the current errno-derived
`Error`, and successful whole-slice writes return `Ok(byte_count)`.
`write_byte_unchecked` and `write_bytes_unchecked` keep the older boolean and
partial-count shapes; `write_byte_raw` and `write_bytes_raw` keep raw integer
errors for compatibility adapters.

`write_result(path, values)` opens `path` with `"w"`, writes the whole
`Slice[u8]`, closes the handle, and returns `Ok(byte_count)` when the
complete write and close succeeded. `append_result(path, values)` does the
same with `"a"` mode. Failed opens, short writes, or failed closes return
`Err(Error)`.
`try_write(path, values)` and `try_append(path, values)` are byte-counting
`Option` wrappers over the same operation. Use `write_raw_result` and
`append_raw_result` only when a compatibility caller needs `Result[i64, i64]`.

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
`copy_result(source, target)` is the same operation but keeps open/write/close
failures as `Err(Error)`. `copy_raw_result(source, target)` keeps the raw
integer bridge for compatibility tests and low-level adapters.

`copy(source, target)` is the compatibility boolean wrapper over `try_copy`.

`close(file)` and `file.close()` return `Result[(), Error]`. Closing an invalid
handle returns `Error(InvalidInput)`; host close failures return errno-derived
`Error` values. Use `close_unchecked(file)` or `file.close_unchecked()` only
when the older boolean compatibility shape is desired. The current first slice
does not mutate or disarm copied `File` values after closing, so close a file
handle once and do not reuse any copied handle after one copy has been closed.

`exists(path)` checks whether the path exists. `remove(path)` removes a file
path and returns whether the host accepted the request. `remove_result(path)`
is the error-preserving form; it returns `Result[(), Error]`.
`remove_raw_result(path)` keeps the old `Result[(), i64]` bridge.

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
programs that treat a missing path as a programmer error. This helper follows
symbolic links, matching the common "tell me about the thing this path names"
policy.
`metadata_result(path)` is the production-friendly form. It returns
`Result[Metadata, Error]`, preserving errno-derived kinds such as `NotFound`,
`PermissionDenied`, or `NotDirectory`. `metadata_raw_result(path)` keeps the
same operation with a compact raw `i64` payload for compatibility-only
bridges.

`try_symlink_metadata(path)` returns `Option[Metadata]` using a no-follow path
lookup. For ordinary files and directories it looks like `try_metadata`; for a
symbolic link it reports the link object itself, so `file_type()` is
`Symlink` and `len()` is the stored target byte length on the current
Linux/glibc backend. `symlink_metadata(path)` is the asserting wrapper. Use
this helper before rewriting or removing links when the distinction between
the link and its target matters. The embedded `Permissions` value still uses
the same access-style `permissions(path)` snapshot as ordinary metadata;
portable symlink permission-bit policy is intentionally not promised yet.
`symlink_metadata_result(path)` and `symlink_metadata_raw_result(path)` are the
direct `Error` and raw compatibility variants of the same no-follow metadata
lookup.

`try_file_type(path)` is the lightweight `Option[FileKind]` helper for code
that only needs the path kind without building the full metadata/permission
snapshot. `is_file(path)`, `is_dir(path)`,
`is_symlink(path)`, and `is_other(path)` are direct path predicates. `is_file`,
`is_dir`, `is_other`, and `try_file_type` follow symbolic links through the
ordinary metadata policy; `is_symlink` uses no-follow metadata so it can detect
the link object itself. Missing or unstatable paths return `false`. Prefer
these predicates at call sites that only branch on the kind, and keep
`metadata(path)` or `symlink_metadata(path)` for code that also needs size or
permissions.
`file_type_result(path)` returns `Result[FileKind, Error]` for callers that
need to distinguish a missing path from a permission or path-shape failure.
`file_type_raw_result(path)` is the raw compatibility variant.

`Metadata::len()` returns the byte length reported by the host. Directories and
special files can have host-specific sizes; only regular-file sizes should be
used as portable byte counts. `metadata.file_type()` returns a `FileKind` enum
with `Regular`, `Directory`, `Symlink`, and `Other` variants. The convenience
methods `metadata.is_file()`, `metadata.is_dir()`, `metadata.is_symlink()`, and
`metadata.is_other()` cover common branches when you already have metadata.
The current Linux/glibc runtime uses `stat` for target-following metadata and
`lstat` for no-follow symlink metadata. `metadata.accessed()` and
`metadata.modified()` expose access and modification times as
`std::time::SystemTime`. `metadata.changed()` exposes the POSIX status-change
timestamp (`ctime`), not a portable creation time. A separate creation/birth-time
API should wait until the platform policy is explicit.

`try_mode(path)` returns the current POSIX permission bits as
`Option[i64]`. The value is already masked to the low `0777` permission bits,
so code can compare it directly with familiar octal-style values written as
decimal literals today: `420` for `0644`, `292` for `0444`, and `511` for
`0777`. `mode(path)` is the asserting convenience wrapper. `set_mode(path,
mode)` uses the host `chmod` path and rejects values outside `0..511`.
`set_permissions(path, permissions)` is the structured version for code that
already has a `Permissions` value.
`mode_result(path)` returns `Result[i64, Error]` and should be used when a
missing path or permission failure is normal input. `mode_raw_result(path)` is
kept for compatibility adapters.

`try_canonicalize(ref mut zone, path)` resolves an existing path through the
host filesystem and returns an owned absolute `String` in the caller-provided
zone. Missing paths, permission failures, and paths that cannot be resolved
return `None`. `canonicalize(ref mut zone, path)` is the asserting convenience
wrapper for code that treats an unresolvable path as a programmer error. The
current Linux/glibc runtime uses `realpath`, so it follows symbolic links and
requires the path to exist.
`canonicalize_result(ref mut zone, path)` returns `Result[String, Error]` so
tools can report the specific filesystem failure instead of collapsing it to
`None`.

`rename(source, target)` asks the host to move or rename one path to another.
On the current Linux/glibc runtime path this follows host `rename` behavior,
including replacing some existing targets when the OS allows it. Portable
overwrite policy is still a future documentation point, so tests use a missing
target path. `rename_result(source, target)` keeps the same operation but
returns `Result[(), Error]` so callers can distinguish failures such as
`NotFound`. `rename_raw_result(source, target)` is the compatibility raw
payload form.

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

`try_read_link(ref mut zone, path)` reads the target bytes stored in a symbolic
link and copies them into a `String` owned by the caller's zone. Missing paths,
regular files, unreadable links, and targets that do not fit the current
runtime buffer return `None`. `read_link(ref mut zone, path)` is the asserting
wrapper. This is intentionally different from `canonicalize`: `read_link`
returns the link text as stored, while `canonicalize` resolves an existing path
to an absolute host path.
`read_link_result(ref mut zone, path)` preserves the same stored-link-target
behavior while returning `Err(Error)` for missing paths, non-links, or host
read failures.

`ensure_file(path)` is the file counterpart to `ensure_dir`: it creates one
empty file only when the path is missing and otherwise succeeds only for an
existing regular file. It returns `false` for directories, symlinks resolved to
non-regular targets, other path kinds, or missing parents. It is deliberately
not recursive and deliberately not an `OpenOptions` replacement; use
`open(path, mode)` or `create(path)` when the caller needs a handle and failure
reason.

`create_dir(path)` creates one directory with the current default permission
mode used by Ari's runtime shim. It does not create parent directories.
`create_dir_result(path)` returns `Result[(), Error]` for failed
single-directory creation. `remove_dir_result(path)` does the same for
`remove_dir(path)`. The `*_raw_result` variants keep raw integer payloads for
compatibility.
`ensure_dir(path)` is the idempotent single-directory helper: it returns `true`
when `path` is already a directory, creates it when it is missing, and returns
`false` when another kind of path already exists or a parent directory is
missing. It is deliberately not recursive; use it when a test or tool needs one
known output directory without treating reruns as errors.
`create_dir_all(path)` creates every missing parent directory needed for
`path`, treats existing directories as success, and returns `false` when the
path is empty, an existing non-directory blocks the final path, or an
intermediate component cannot be created or searched. `ensure_dir_all(path)` is
the natural idempotent alias for callers that read the operation as setup
rather than creation. Both helpers use the runtime's current default directory
mode for newly created components, with the host process umask still applying.
`remove_dir(path)` removes one empty directory. `remove_dir_all(path)` removes
a directory tree recursively, including regular files and symlinks inside the
tree. It rejects missing paths and non-directory roots, and it treats symlinks
as link entries to unlink rather than directories to follow.

`open_dir_result(path)` opens one directory and returns `Result[Dir, Error]`;
`open_dir_raw_result(path)` keeps the raw compatibility payload.
`try_open_dir(path)` opens one directory and returns `Option[Dir]`.
`dir.next(ref mut zone)` returns the next entry name as `Option[String]`,
skipping the host `"."` and `".."` entries. `dir.close()` closes the directory
handle and preserves invalid-handle and host close failures as `Result`. Use
`dir.close_unchecked()` only for the older boolean compatibility shape. `Dir`
follows the same value-handle rule as `File`: close it once and do not reuse
copied handles after close.
`try_read_dir(ref mut zone, path)` is the convenient one-shot helper:
it opens the directory, collects names into `std::vec::Vec[String]`, closes the
handle, and returns `None` when the directory cannot be opened or closed.
`read_dir_result(ref mut zone, path)` keeps open/close failures as
`Result[Vec[String], Error]`. `read_dir(ref mut zone, path)` is the asserting
wrapper for code that treats a failed directory read as a programmer error. Use
`try_read_dir_entries(ref mut zone, path)` or
`read_dir_entries_result(ref mut zone, path)` or
`read_dir_entries(ref mut zone, path)` when the call site needs the entry name,
the joined child path, and lazy metadata. `DirEntry::name()` and
`DirEntry::path()` return borrowed `String` references; the `*_equals` helpers
keep common tests short. `entry.try_metadata()`, `entry.metadata()`,
`entry.try_file_type()`, `entry.is_file()`, `entry.is_dir()`, and
`entry.is_other()` follow symbolic links, matching the path-level
`fs::metadata` policy. `entry.try_symlink_metadata()`,
`entry.symlink_metadata()`, and `entry.is_symlink()` query the entry path with
the no-follow `lstat` policy. The low-level `open_dir`, `read_dir_next`, and
`close_dir` names exist for direct runtime-hook coverage, but ordinary code
should prefer `try_read_dir`/`try_read_dir_entries` for collection-style reads
or `try_open_dir` plus the `Dir` methods for manual streaming.

## Feature Status

| Need | Status |
| --- | --- |
| open | Current: `open(path, mode)` returns `Error`, `open_optional(path, mode)`/`try_open(path, mode)` discard reasons, `open_unchecked(path, mode)` preserves the invalid-handle compatibility shape, `open_result(path, mode)` remains a compatibility alias, raw compatibility `open_raw_result`, Result/optional/unchecked convenience wrappers for read/write/append modes, and `OpenOptions` for named read/write/append/truncate/create/create-new policy plus `OpenOptions::open`/`open_optional`/`try_open`/`open_unchecked`/`open_result`/`open_raw_result`. |
| create | Current: `create(path)` returns `Error`, `create_optional(path)`/`try_create(path)` discard reasons, `create_unchecked(path)` preserves the invalid-handle compatibility shape, `create_result(path)` remains a compatibility alias, and `ensure_file(path)` provides non-truncating idempotent file setup. |
| read | Current: byte `read_byte`/`try_read_byte`, whole-file `read`/`read_to_string`, direct `Error` helpers `read_result`/`read_to_string_result`, and fallible `try_read`/`try_read_to_string`. Splitting byte-read EOF from byte-read errors remains roadmap. |
| write | Current: byte `write_byte`, `write_bytes`, `write_byte_unchecked`, `write_bytes_unchecked`, whole-file `write`, byte-counting `try_write`, `Error`-returning `write_result`, and raw compatibility `write_raw_result`/`write_byte_raw`/`write_bytes_raw`. |
| append | Current: `"a"`/`"a+"` modes, whole-file `append`, byte-counting `try_append`, `Error`-returning `append_result`, and raw compatibility `append_raw_result`. |
| truncate | Current: `truncate(path)` and `"w"`/`"w+"` modes. |
| metadata | Current: `try_metadata(path)`/`metadata(path)`, `metadata_result(path)` with `Error`, and raw compatibility `metadata_raw_result(path)` over the Linux/glibc `stat` runtime path; `try_symlink_metadata(path)`/`symlink_metadata(path)`, `symlink_metadata_result(path)` with `Error`, raw compatibility `symlink_metadata_raw_result(path)`, and `is_symlink(path)` over the Linux/glibc `lstat` runtime path; plus `try_file_type(path)`, `file_type_result(path)`, `is_file(path)`, `is_dir(path)`, `is_other(path)`, `Metadata`, `FileKind`, and `Metadata` access/modification/status-change timestamps; creation/birth time is platform-policy roadmap work. |
| permissions | Current: access-style `can_read`, `can_write`, `can_execute`, `permissions`, stat-backed `try_mode`/`mode`, `mode_result` with `Error`, raw compatibility `mode_raw_result`, and chmod-backed `set_mode`/`set_permissions`; richer ACL/owner/group policy is roadmap. |
| rename | Current: `rename(source, target)`, `rename_result(source, target)` with `Error`, and raw compatibility `rename_raw_result`; portable overwrite policy is roadmap. |
| remove | Current: file removal with `remove(path)`/`remove_result(path)` plus raw compatibility `remove_raw_result`, empty directory removal with `remove_dir(path)`/`remove_dir_result(path)` plus raw compatibility `remove_dir_raw_result`, and recursive tree removal with `remove_dir_all(path)` using no-follow symlink policy for entries. |
| copy | Current: source streaming `copy(source, target)`, byte-counting `try_copy(source, target)`, `Error`-returning `copy_result(source, target)`, and raw compatibility `copy_raw_result(source, target)` for byte files. |
| hard link | Current: `hard_link(existing, link_path)` runtime hook. |
| symbolic link | Current: `symbolic_link(target, link_path)`, `try_read_link(ref mut zone, path)`, `read_link_result(ref mut zone, path)` with `Error`, and asserting `read_link(ref mut zone, path)` on the Linux/glibc path; Windows split is roadmap. |
| canonicalize | Current: `try_canonicalize(ref mut zone, path)`, `canonicalize_result(ref mut zone, path)` with `Error`, and asserting `canonicalize(ref mut zone, path)` over the Linux/glibc `realpath` runtime path. |
| file handle lifecycle | Current: explicit Result-returning `close`, `position`, and `seek` on value `File` handles, `_unchecked` compatibility helpers, and `_raw` integer-error helpers. Close once; copied handles are not disarmed. |
| read directory | Current: `try_read_dir(ref mut zone, path)`, `read_dir_result(ref mut zone, path)` with `Error`, `read_dir(ref mut zone, path)`, `try_read_dir_entries(ref mut zone, path)`, `read_dir_entries_result(ref mut zone, path)` with `Error`, `read_dir_entries(ref mut zone, path)`, `open_dir_result(path)` with `Error`, raw compatibility `open_dir_raw_result(path)`, `try_open_dir(path)`, `Dir`, `DirEntry`, `dir.next(ref mut zone)`, `dir.close()`/`dir.close_unchecked()`, borrowed entry name/path methods, and lazy `DirEntry` metadata/file-kind predicates; richer per-entry errors are roadmap. |
| create directory | Current: single-directory `create_dir(path)`, `Error`-returning `create_dir_result(path)`, raw compatibility `create_dir_raw_result(path)`, and idempotent `ensure_dir(path)`, plus recursive `create_dir_all(path)` and `ensure_dir_all(path)` for missing parent directories. |
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

var zone = zone::create(4096);
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

Create nested output directories:

```ari
let nested = "build/prelude/example-fs-dir.tmp/cache/shards";
if fs::create_dir_all(nested) {
  fs::write("build/prelude/example-fs-dir.tmp/cache/shards/data.txt", "ok");
}
```

Remove a directory tree:

```ari
let tree = "build/prelude/example-fs-dir.tmp";
if fs::remove_dir_all(tree) {
  assert(!fs::exists(tree));
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

Read entries with joined paths:

```ari
var zone = zone::create(512);
let entries = fs::read_dir_entries(ref mut zone, "build/prelude");
var index = 0;
while index < entries.len() {
  let entry = entries.get(index);
  if entry.name_equals("example-fs.tmp") {
    let path = entry.path();
    if path.equals_text("build/prelude/example-fs.tmp") {
      index = entries.len();
    }
  }
  index = index + 1;
}
zone::destroy(zone);
```

Read entry metadata without rebuilding each path:

```ari
var zone = zone::create(512);
let entries = fs::read_dir_entries(ref mut zone, "build/prelude");
var index = 0;
while index < entries.len() {
  let entry = entries.get(index);
  if entry.is_file() {
    let info = entry.metadata();
    if info.len() > 0 {
      index = entries.len();
    }
  }
  index = index + 1;
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

- This slice is byte-oriented. There is no owned path type, file locking API,
  temporary-file API, or text encoding policy yet.
  Directory reads expose names, joined child paths, and lazy per-entry metadata
  methods, but not per-entry error values. Basic `stat`/`lstat` metadata,
  permission-bit lookup and mutation, and existing-path canonicalization exist,
  including access, modification, and POSIX status-change timestamps. Richer
  owner/group/ACL policy, portable creation/birth time, richer link metadata,
  and platform-specific symlink policy are still future work.
- Runtime hooks currently target the Linux/glibc LLVM path through `access`,
  `stat`, `lstat`, `chmod`, `realpath`, `readlink`, `unlink`, `rename`, `link`,
  `symlink`, `mkdir`, `rmdir`, `opendir`, `readdir`, `closedir`, `open`,
  `read`, `write`, `lseek`, and `close`; `DirEntry` metadata methods use
  stack-local byte-path adapters over those same stat/access hooks, and the
  one-shot `read_dir` helpers are source Ari over the directory hooks.
- `File` and `Dir` are not tracked `own` resources yet. The caller must close
  each successful handle exactly once by convention. Future ownership work
  should make OS resources harder to copy and accidentally double-close.
- The mode-string surface is intentionally small. Use `"rw"` or `"r+"` for
  existing read/write files, `"w+"` for create/truncate read/write, and `"a+"`
  for read/append. More detailed flags belong in a future options API.
- Prefer the `*_result` helpers when the caller needs a reason. Compatibility
  bool, `Option`, empty-string, and `-1` sentinel helpers remain for compact
  code and older tests, but new filesystem APIs should use `std::error::Error`
  instead of adding more sentinel conventions.

## Tests

```text
tests/cases/standard-library/ok/fs/std-fs-basic.ari
tests/cases/standard-library/ok/fs/std-fs-try-byte.ari
tests/cases/standard-library/ok/fs/std-fs-append.ari
tests/cases/standard-library/ok/fs/std-fs-open-modes.ari
tests/cases/standard-library/ok/fs/std-fs-open-options.ari
tests/cases/standard-library/ok/fs/std-fs-open-result.ari
tests/cases/standard-library/ok/fs/std-fs-read-write.ari
tests/cases/standard-library/ok/fs/std-fs-read-result.ari
tests/cases/standard-library/ok/fs/std-fs-byte-result.ari
tests/cases/standard-library/ok/fs/std-fs-try-read.ari
tests/cases/standard-library/ok/fs/std-fs-create-truncate-copy.ari
tests/cases/standard-library/ok/fs/std-fs-io-traits.ari
tests/cases/standard-library/ok/fs/std-fs-seek.ari
tests/cases/standard-library/ok/fs/std-fs-rename-dir.ari
tests/cases/standard-library/ok/fs/std-fs-mutation-result.ari
tests/cases/standard-library/ok/fs/std-fs-ensure-dir.ari
tests/cases/standard-library/ok/fs/std-fs-ensure-file.ari
tests/cases/standard-library/ok/fs/std-fs-create-dir-all.ari
tests/cases/standard-library/ok/fs/std-fs-remove-dir-all.ari
tests/cases/standard-library/ok/fs/std-fs-read-dir.ari
tests/cases/standard-library/ok/fs/std-fs-dir-entry-metadata.ari
tests/cases/standard-library/ok/fs/std-fs-links.ari
tests/cases/standard-library/ok/fs/std-fs-read-link.ari
tests/cases/standard-library/ok/fs/std-fs-symlink-metadata.ari
tests/cases/standard-library/ok/fs/std-fs-permissions.ari
tests/cases/standard-library/ok/fs/std-fs-mode.ari
tests/cases/standard-library/ok/fs/std-fs-metadata.ari
tests/cases/standard-library/ok/fs/std-fs-metadata-times.ari
tests/cases/standard-library/ok/fs/std-fs-canonicalize.ari
```

`make check-prelude` emits LLVM for the runtime hooks, checks the C syscall
declarations, runs the executable, and cleans up the temporary file under
`build/prelude/`. `std-fs-basic.ari` covers existence, create/truncate writes,
reads, byte-slice writes, close, removal, and `Option[File]` read/write opens.
`std-fs-try-byte.ari` covers `Option[u8]` byte reads for module and `File`
method forms, EOF, invalid handles, and character-literal byte writes.
`std-fs-append.ari` covers append-mode open, preservation of existing bytes,
and failed append opens through `Option[File]`. `std-fs-open-modes.ari` covers
the mode-string contract, including `"rw"`, `"r+"`, `"w+"`, `"a+"`, empty modes,
and invalid mode strings. `std-fs-open-options.ari` covers the `OpenOptions`
value builder, exclusive creation, non-truncating read/write opens,
append-with-read behavior, and invalid option combinations.
`std-fs-open-result.ari` covers natural `open`, `create`, `OpenOptions::open`,
their `_optional` compatibility helpers, existing `*_result` aliases,
`open_raw_result`, `create_raw_result`, and `OpenOptions::open_raw_result` for
invalid input, missing files, exclusive-create failures, and successful
handles. `std-fs-read-write.ari` covers source whole-file
write/append compatibility wrappers, `try_write`/`try_append` byte counts,
read-to-byte-string, missing-file empty reads, and truncating rewrite behavior.
`std-fs-read-result.ari` covers direct `Error` whole-file read helpers,
successful byte-string reads, missing-file `NotFound`, and compatibility
`try_read` absence.
`std-fs-query-result.ari` covers direct `Error` metadata, no-follow metadata,
path-kind, mode, canonicalization, symbolic-link target, directory-open, and
directory-list helpers.
`std-fs-byte-result.ari` covers `write_result`, `append_result`, and
`copy_result` direct Error payloads plus raw compatibility helpers.
`std-fs-try-read.ari` covers `Option[String]` whole-file
reads where missing files become `None` and empty files stay `Some(empty)`.
`std-fs-create-truncate-copy.ari` covers source `create`, `try_create`,
`read`, `truncate`, missing-source copy failure, whole-file copy behavior, and
`try_copy` byte counts.
`std-fs-io-traits.ari` covers `File` as a generic `std::io::Reader` and
`std::io::Writer`, including `read_to_string`, EOF through `read_exact`,
file-to-file `try_copy`, whole-slice `write_all`, direct-descriptor `flush`,
and invalid-handle trait behavior.
`std-fs-seek.ari` covers runtime-backed file cursor `position`/`seek`, trait
method syntax, direct module hooks, negative seek rejection, and generic
`std::io::Seek` dispatch.
`std-fs-rename-dir.ari` covers runtime-backed `rename`,
`create_dir`, and `remove_dir` behavior. `std-fs-mutation-result.ari` covers
`remove_result`, `rename_result`, `create_dir_result`, and
`remove_dir_result` success/Error cases plus raw compatibility variants.
`std-fs-ensure-dir.ari` covers
idempotent single-directory creation, file-path rejection, missing-parent
failure, and cleanup. `std-fs-create-dir-all.ari` covers recursive parent
creation, existing-directory idempotence, `ensure_dir_all`, file-path
rejection, blocked child creation, nested writes, and cleanup.
`std-fs-remove-dir-all.ari` covers recursive tree removal, non-directory root
rejection, empty-directory removal, missing-path failure, internal file and
symlink unlinking, and no-follow directory symlink behavior.
`std-fs-ensure-file.ari` covers idempotent single-file
creation, existing-file preservation, directory-path rejection, missing-parent
failure, and cleanup. `std-fs-read-dir.ari` covers
runtime-backed `Dir` open/next/close behavior, one-shot
`try_read_dir`/`read_dir` name-list helpers,
`try_read_dir_entries`/`read_dir_entries` entry-list helpers, joined paths,
missing-directory failure, dot entry skipping, invalid-handle `None`, and
cleanup. `std-fs-dir-entry-metadata.ari` covers `DirEntry` target-following
metadata, no-follow symlink metadata, file-kind predicates, byte length,
permission snapshots, and stored zone path reuse. `std-fs-links.ari` covers
runtime-backed `hard_link` and `symbolic_link` behavior plus read-through
checks. `std-fs-read-link.ari` covers runtime-backed symbolic-link target
reads, `Option[String]` failure behavior for regular and missing paths, and
the asserting `read_link` helper. `std-fs-symlink-metadata.ari` covers
no-follow `lstat` metadata, `metadata` following a symbolic link to its target,
direct `is_symlink` behavior, and missing-path `None`. `std-fs-permissions.ari` covers access-style readable/writable/
executable checks, the `Permissions` wrapper methods, and missing-path
all-false behavior. `std-fs-mode.ari` covers stat-backed mode lookup,
chmod-backed mutation calls, `Permissions` constructors, mode conversion,
invalid mode rejection, missing-path failure, and cleanup restoration without
depending on exact chmod effects from the host filesystem.
`std-fs-metadata.ari` covers target-following `Option[Metadata]`,
`Option[FileKind]`, direct path-kind predicates, regular-file byte length,
`FileKind`, metadata methods, directory predicates, and missing-path `None`.
`std-fs-metadata-times.ari` covers `Metadata::accessed`,
`Metadata::modified`, and `Metadata::changed` for target-following metadata,
no-follow symlink metadata, and `DirEntry` lazy metadata over stored paths.
`std-fs-canonicalize.ari` covers `Option[String]` path resolution,
absolute canonical paths, filename preservation, and missing-path `None`.

## Next Work

- Add richer permission metadata for owner/group/ACL policy.
- Add explicit overwrite/platform policy for `rename`.
- Grow canonicalization toward owned path values and platform-specific policy.
- Expand link support with clearer platform-specific symlink policy.
- Add portable metadata creation/birth time only after the platform policy is
  explicit.
- Expand directory reads with richer per-entry error reporting and owned
  OS-resource iterator handles.
- Grow `std::path` from lexical helpers into owned path values after Ari has a
  clearer owned string/path story.
- Add secure temporary files and optional file locking after the resource and
  platform policy is documented.
- Migrate remaining bool/Option-only filesystem helpers to richer
  `Result[..., Error]` variants where callers need diagnostics.
- Promote `File` toward an owned resource handle when the compiler can express
  OS-resource ownership and drop policy cleanly.
