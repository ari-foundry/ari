# std::fs

`std::fs` is Ari's portable filesystem module. It exists so ordinary programs
can work with files without reaching directly for raw syscalls or C ABI
declarations. The first slices are deliberately small: check whether a path
exists, query access-style read/write/execute permissions, open a byte stream
with a compact mode string, read and write bytes, write or append a byte slice
in one call, create/truncate/copy small files, read a whole file into Ari's
byte-oriented `String` with natural `Result` APIs and explicit compatibility
forms, rename paths, create hard or symbolic links, read symbolic-link
targets, query no-follow symbolic-link metadata, ensure a single regular file exists without truncating an existing file, create or remove one empty
directory, ensure a single directory exists without treating an existing
directory as failure, recursively create missing directory parents, read
directory entry names through an explicit `Dir` handle, collect `DirEntry`
values with names, joined paths, lazy metadata helpers, and natural
`Error`-returning directory query forms, query basic file metadata with
`Option` or `Result[..., Error]`, ask direct path-kind predicates such as `is_file`
and `is_dir`, read and change POSIX permission bits, resolve an existing path
to an absolute canonical path with `Option` or `Result` failure details, create
hosted temporary files and directories, pass `File` handles to generic `std::io::Reader`
and `std::io::Writer` helpers, inspect and move a file cursor through
`std::io::Seek`, close the handle, and remove a file.

The default spelling rule is Result-first: recoverable filesystem operations
use natural names such as `open`, `create`, `read`, `read_to_string`,
`read_bytes`, `write`, `write_string`, `append`, `copy`, `metadata`,
`symlink_metadata`, `file_type`, `mode`, `canonicalize`, `read_link`,
`read_dir`, `read_dir_entries`, `read_dir_names`, `create_dir`,
`create_dir_all`, `remove_file`, `remove_dir`, `remove_dir_all`, `rename`,
`hard_link`, `symbolic_link`, `temp_file`, `temp_dir`, `position`, `seek`,
`close`, `lock_shared`, `lock_exclusive`, `unlock`, and `close_dir`, and they
return `Result[..., fs::Error]` unless the operation is only a predicate such
as `exists`, `can_read`, or `is_file`.

Suffixes are reserved for non-default behavior. `_optional` and older `try_*`
helpers discard error details and return `Option`; `_or_default` helpers keep
the old empty fallback; `_unchecked` helpers keep the old assert or
invalid-handle behavior; `_bool` helpers keep old success-flag behavior; and
`_raw` helpers expose compact host integer errors. Ordinary
old result-suffixed migration aliases are retired from `std::fs`; new and existing code
should use the natural Result-returning names unless it deliberately needs one
of the explicit compatibility suffixes.

Allocation-returning helpers take `ref mut Zone` because Ari has no hidden
global heap. Whole-file reads, directory collection, link reads, and
canonicalization copy bytes into the caller's zone. The current path model is
POSIX-style byte paths: `std::path::PathBytes` is a borrowed byte view and
`std::path::PathBuf` is an owned `std::string::String` alias. In this module
`canonicalize(ref mut zone, path)` therefore returns an owned byte string that
can be used as a `PathBuf`/path byte source.

Common natural API shapes:

| Operation | Preferred API | Compatibility APIs |
| --- | --- | --- |
| Read text/bytes | `read(ref mut zone, path) -> Result[String, Error]`, `read_to_string(ref mut zone, path) -> Result[String, Error]`, `read_bytes(ref mut zone, path) -> Result[Vec[u8], Error]` | `read_optional`, `try_read`, `read_or_default`, `read_unchecked` |
| Write text/bytes | `write(path, values) -> Result[i64, Error]`, `write_string(path, text) -> Result[(), Error]`, `append(path, values) -> Result[i64, Error]` | `write_bool`, `try_write`, `write_raw`, `append_bool`, `try_append`, `append_raw` |
| Directories | `create_dir(path) -> Result[(), Error]`, `create_dir_all(path) -> Result[(), Error]`, `remove_dir(path) -> Result[(), Error]`, `remove_dir_all(path) -> Result[(), Error]` | `*_bool`, `*_unchecked`, `_optional`/`try_*`, `_raw` |
| Files and moves | `remove_file(path) -> Result[(), Error]`, `rename(source, target) -> Result[(), Error]`, `copy(source, target) -> Result[i64, Error]` | `remove_bool`, `rename_bool`, `copy_bool`, `try_copy`, `_raw` |
| Paths and directories | `canonicalize(ref mut zone, path) -> Result[PathBuf, Error]`, `read_dir(ref mut zone, path) -> Result[Vec[DirEntry], Error]` | `_optional`, `try_*`, `_unchecked`, `_raw` where raw host errno is useful |
| Temporary paths | `temp_file(ref mut zone) -> Result[TempFile, Error]`, `temp_file_in(ref mut zone, prefix) -> Result[TempFile, Error]`, `temp_dir(ref mut zone) -> Result[TempDir, Error]`, `temp_dir_in(ref mut zone, prefix) -> Result[TempDir, Error]` | Use `TempFile::close`, `TempFile::remove`, `TempFile::close_and_remove`, and `TempDir::remove` for cleanup. |
| Advisory locks | `lock_shared(file) -> Result[(), Error]`, `lock_exclusive(file) -> Result[(), Error]`, `try_lock_shared(file) -> Result[bool, Error]`, `try_lock_exclusive(file) -> Result[bool, Error]`, `unlock(file) -> Result[(), Error]` | `*_raw` variants expose packed integer errors. |

## API

```ari
fs::Error
fs::ErrorKind
fs::TempFile
fs::TempDir
fs::exists(path)
fs::can_read(path)
fs::can_write(path)
fs::can_execute(path)
fs::permissions(path)
fs::temp_file(ref mut zone)
fs::temp_file_in(ref mut zone, prefix)
fs::temp_dir(ref mut zone)
fs::temp_dir_in(ref mut zone, prefix)
fs::metadata(path)
fs::metadata_optional(path)
fs::metadata_unchecked(path)
fs::metadata_raw(path)
fs::try_metadata(path)
fs::symlink_metadata(path)
fs::symlink_metadata_optional(path)
fs::symlink_metadata_unchecked(path)
fs::symlink_metadata_raw(path)
fs::try_symlink_metadata(path)
fs::file_type(path)
fs::file_type_optional(path)
fs::file_type_raw(path)
fs::file_type_unchecked(path)
fs::try_file_type(path)
fs::is_file(path)
fs::is_dir(path)
fs::is_symlink(path)
fs::is_other(path)
fs::mode(path)
fs::mode_optional(path)
fs::mode_raw(path)
fs::mode_unchecked(path)
fs::try_mode(path)
fs::set_mode(path, mode)
fs::set_permissions(path, permissions)
fs::canonicalize(ref mut zone, path)
fs::canonicalize_optional(ref mut zone, path)
fs::canonicalize_unchecked(ref mut zone, path)
fs::try_canonicalize(ref mut zone, path)
fs::remove(path)
fs::remove_file(path)
fs::remove_bool(path)
fs::remove_file_bool(path)
fs::remove_unchecked(path)
fs::rename(source, target)
fs::rename_bool(source, target)
fs::rename_unchecked(source, target)
fs::hard_link(existing, link_path)
fs::hard_link_bool(existing, link_path)
fs::hard_link_raw(existing, link_path)
fs::hard_link_unchecked(existing, link_path)
fs::symbolic_link(target, link_path)
fs::symbolic_link_bool(target, link_path)
fs::symbolic_link_raw(target, link_path)
fs::symbolic_link_unchecked(target, link_path)
fs::read_link(ref mut zone, path)
fs::read_link_optional(ref mut zone, path)
fs::read_link_unchecked(ref mut zone, path)
fs::try_read_link(ref mut zone, path)
fs::ensure_file(path)
fs::create_dir(path)
fs::create_dir_bool(path)
fs::create_dir_raw(path)
fs::create_dir_unchecked(path)
fs::ensure_dir(path)
fs::create_dir_all(path)
fs::create_dir_all_bool(path)
fs::create_dir_all_raw(path)
fs::create_dir_all_unchecked(path)
fs::ensure_dir_all(path)
fs::remove_dir(path)
fs::remove_dir_bool(path)
fs::remove_dir_raw(path)
fs::remove_dir_unchecked(path)
fs::remove_dir_all(path)
fs::remove_dir_all_bool(path)
fs::open_dir(path)
fs::open_dir_raw(path)
fs::open_dir_unchecked(path)
fs::try_open_dir(path)
fs::read_dir(ref mut zone, path)
fs::read_dir_optional(ref mut zone, path)
fs::read_dir_unchecked(ref mut zone, path)
fs::try_read_dir(ref mut zone, path)
fs::read_dir_names(ref mut zone, path)
fs::read_dir_names_optional(ref mut zone, path)
fs::read_dir_names_unchecked(ref mut zone, path)
fs::try_read_dir_names(ref mut zone, path)
fs::read_dir_entries(ref mut zone, path)
fs::read_dir_entries_optional(ref mut zone, path)
fs::read_dir_entries_unchecked(ref mut zone, path)
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
fs::remove_raw(path)
fs::rename_raw(source, target)
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
fs::lock_shared(file)
fs::lock_shared_raw(file)
fs::try_lock_shared(file)
fs::lock_exclusive(file)
fs::lock_exclusive_raw(file)
fs::try_lock_exclusive(file)
fs::unlock(file)
fs::unlock_raw(file)
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
fs::read_bytes(ref mut zone, path)
fs::read_optional(ref mut zone, path)
fs::read_or_default(ref mut zone, path)
fs::read(ref mut zone, path)
fs::read_unchecked(ref mut zone, path)
fs::try_read(ref mut zone, path)
fs::write(path, values)
fs::write_bool(path, values)
fs::write_raw(path, values)
fs::write(path, values)
fs::write_string(path, text)
fs::try_write(path, values)
fs::append(path, values)
fs::append_bool(path, values)
fs::append_raw(path, values)
fs::append(path, values)
fs::try_append(path, values)
fs::truncate(path)
fs::copy(source, target)
fs::copy_bool(source, target)
fs::copy_raw(source, target)
fs::copy(source, target)
fs::try_copy(source, target)
fs::read_to_string(ref mut zone, path)
fs::read_to_string_optional(ref mut zone, path)
fs::read_to_string_or_default(ref mut zone, path)
fs::read_to_string(ref mut zone, path)
fs::read_to_string_unchecked(ref mut zone, path)
fs::try_read_to_string(ref mut zone, path)

fs::open_raw(path, mode)
fs::open(path, mode)
fs::create_raw(path)
fs::create(path)
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
options.open_raw(path)
options.open(path)
options.try_open(path)

File::invalid()
file.descriptor()
file.is_open()
file.close()
file.close_unchecked()
file.lock_shared()
file.try_lock_shared()
file.lock_exclusive()
file.try_lock_exclusive()
file.unlock()
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
entry.metadata()
entry.metadata_optional()
entry.metadata_unchecked()
entry.try_metadata()
entry.symlink_metadata()
entry.symlink_metadata_optional()
entry.symlink_metadata_unchecked()
entry.try_symlink_metadata()
entry.file_type()
entry.file_type_optional()
entry.file_type_unchecked()
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
var input = fs::open("input.txt", "r").unwrap();
let text = io::read_to_string<std::fs::File>(ref mut zone, ref mut input).unwrap();
input.close().unwrap();

var source = fs::open("input.txt", "r").unwrap();
var target = fs::open("output.txt", "w").unwrap();
io::copy<std::fs::File, std::fs::File>(ref mut source, ref mut target).unwrap();
source.close().unwrap();
target.close().unwrap();
zone::destroy(zone);
```

The `Writer` `flush` method returns `Ok(())` while the descriptor is open and
`Err(InvalidInput)` after close: file writes are not buffered by `File` itself.
Use explicit buffered wrappers when code needs caller-managed buffering.

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
`open_raw(path, mode)` remains available for low-level compatibility
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
`options.open_raw(path)` is the compatibility form with a raw integer
error payload.

`create(path)` is the natural Result-returning spelling for `open(path, "w")`:
it creates a file if needed and truncates existing contents.
`create_optional(path)` and `try_create(path)` wrap that same operation in
`Option[File]`; `create_unchecked(path)` returns the old invalid-handle
sentinel; and `create_raw(path)` keeps the old raw payload shape.

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

`write(path, values)` opens `path` with `"w"`, writes the whole `Slice[u8]`,
closes the handle, and returns `Ok(byte_count)` when the complete write and
close succeeded. `write_string(path, text)` is the text-shaped convenience
wrapper for writing Ari's byte-oriented `String`; it returns `Result[(),
Error]` after discarding the byte count. `append(path, values)` does the same
with `"a"` mode. Failed opens, short writes, or failed closes return
`Err(Error)`.
`try_write(path, values)` and `try_append(path, values)` are byte-counting
`Option` wrappers over the same operation. Use `write_raw` and
`append_raw` only when a compatibility caller needs `Result[i64, i64]`.

`write_bool(path, values)` and `append_bool(path, values)` are compatibility
boolean wrappers over `try_write` and `try_append`.

`try_read_to_string(ref mut zone, path)` opens `path` with `"r"`, reads bytes
until the current `read_byte` EOF/failure sentinel, closes the handle, and
returns `Some[String]`. Missing or unopenable files return `None`, so this is
the preferred whole-file helper when absence must be distinguished from an empty
file. The current Ari `String` is byte-oriented; this helper does not validate
UTF-8 or any other text encoding.

`try_read(ref mut zone, path)` is the short natural alias for
`try_read_to_string(ref mut zone, path)`.

`read_to_string(ref mut zone, path)` is the natural fallible whole-file read:
it returns `Ok(String)` with the file bytes on success and `Err(Error)` for
open or close failures. `read_to_string_optional(ref mut zone, path)` keeps only the
success payload as `Option[String]`, while `read_to_string_or_default(ref mut
zone, path)` preserves the old empty-string fallback behavior.
`read_to_string_unchecked(ref mut zone, path)` asserts on failure.

`read(ref mut zone, path)` is the short natural alias for
`read_to_string(ref mut zone, path)`. It is still byte-oriented and returns the
same `Result[String, Error]` shape. `read_optional`, `read_or_default`, and
`read_unchecked` mirror the corresponding `read_to_string_*` compatibility
helpers. Whole-file byte-vector reads can use
`read_bytes(ref mut zone, path)`, which copies the byte-oriented
`String` into a `Vec[u8]`.

`truncate(path)` creates the file if needed, truncates it to empty, closes the
handle, and returns whether the operation and close succeeded.

`try_copy(source, target)` opens `source` for reading, opens `target` with
truncating `"w"` semantics, streams bytes from one handle to the other, closes
both handles, and returns `Some(byte_count)` when the copy and both closes
succeeded. Missing sources, failed target opens, failed byte writes, and failed
closes return `None`. Detailed read error reporting remains future `std::io` or
`std::os` work because `read_byte` still uses one EOF/failure sentinel.
`copy(source, target)` is the natural Result-returning operation and keeps
open/write/close failures as `Err(Error)` while returning the byte count on
success. `copy_raw(source, target)` keeps the raw integer bridge for
compatibility tests and low-level adapters. `copy_bool(source, target)` is the
compatibility boolean wrapper over `try_copy`.

`close(file)` and `file.close()` return `Result[(), Error]`. Closing an invalid
handle returns `Error(InvalidInput)`; host close failures return errno-derived
`Error` values. Use `close_unchecked(file)` or `file.close_unchecked()` only
when the older boolean compatibility shape is desired. The current first slice
does not mutate or disarm copied `File` values after closing, so close a file
handle once and do not reuse any copied handle after one copy has been closed.

`exists(path)` checks whether the path exists. `remove(path)` removes a file
path and returns `Result[(), Error]`; `remove_file(path)` is the explicit file
spelling for the same operation. `remove_bool(path)`/`remove_file_bool(path)` keep the old boolean
compatibility shape, `remove_unchecked(path)` is the raw runtime hook, and
`remove_raw(path)` keeps the old `Result[(), i64]` bridge.

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

`metadata(path)` returns `Result[Metadata, Error]`, preserving errno-derived
kinds such as `NotFound`, `PermissionDenied`, or `NotDirectory`. Successful
calls snapshot the byte length, `FileKind`, and the same access-style
`Permissions` used by `permissions(path)`. This helper follows symbolic links,
matching the common "tell me about the thing this path names" policy.
`metadata_optional(path)` and the older `try_metadata(path)` discard the
failure reason and return `Option[Metadata]`. `metadata_unchecked(path)` is the
compatibility wrapper for programs that treat a missing path as a programmer
error. `metadata_raw(path)` keeps the same operation with a compact raw
`i64` payload for compatibility-only bridges.

`symlink_metadata(path)` returns `Result[Metadata, Error]` using a no-follow
path lookup. For ordinary files and directories it looks like `metadata`; for
a symbolic link it reports the link object itself, so `file_type()` is
`Symlink` and `len()` is the stored target byte length on the current
Linux/glibc backend. Use this helper before rewriting or removing links when
the distinction between the link and its target matters. The embedded
`Permissions` value still uses the same access-style `permissions(path)`
snapshot as ordinary metadata; portable symlink permission-bit policy is
intentionally not promised yet. `symlink_metadata_optional(path)` and the older
`try_symlink_metadata(path)` discard the reason. `symlink_metadata_unchecked`
asserts on failure, and `symlink_metadata_raw` is the raw compatibility
variant.

`file_type(path)` returns `Result[FileKind, Error]` for callers that need to
distinguish a missing path from a permission or path-shape failure without
building the full metadata/permission snapshot. `file_type_optional(path)` and
the older `try_file_type(path)` are the lightweight `Option[FileKind]`
helpers for code that only needs the path kind. `file_type_unchecked(path)`
asserts on failure. `is_file(path)`, `is_dir(path)`,
`is_symlink(path)`, and `is_other(path)` are direct path predicates. `is_file`,
`is_dir`, `is_other`, and `file_type` follow symbolic links through the
ordinary metadata policy; `is_symlink` uses no-follow metadata so it can detect
the link object itself. Missing or unstatable paths return `false`. Prefer
these predicates at call sites that only branch on the kind, and keep
`metadata(path)` or `symlink_metadata(path)` for code that also needs size or
permissions. `file_type_raw(path)` is the raw compatibility variant.

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

`mode(path)` returns the current POSIX permission bits as
`Result[i64, Error]`. The successful value is already masked to the low `0777`
permission bits, so code can compare it directly with familiar octal-style
values written as decimal literals today: `420` for `0644`, `292` for `0444`,
and `511` for `0777`. `mode_optional(path)` and the older `try_mode(path)`
discard the failure reason. `mode_unchecked(path)` asserts on failure.
`set_mode(path, mode)` uses the host `chmod` path and rejects values outside
`0..511`. `set_permissions(path, permissions)` is the structured version for
code that already has a `Permissions` value. `mode_raw(path)` is kept
for compatibility adapters.

`canonicalize(ref mut zone, path)` resolves an existing path through the host
filesystem and returns an owned absolute `String` in the caller-provided zone
as `Result[String, Error]`. The current Linux/glibc runtime uses `realpath`, so
it follows symbolic links and requires the path to exist.
`canonicalize_optional(ref mut zone, path)` and the older
`try_canonicalize(ref mut zone, path)` collapse missing paths, permission
failures, and unresolved paths to `None`. `canonicalize_unchecked(ref mut zone,
path)` asserts on failure.

`rename(source, target)` asks the host to move or rename one path to another
and returns `Result[(), Error]` so callers can distinguish failures such as
`NotFound`. On the current Linux/glibc runtime path this follows host
`rename` behavior, including replacing some existing targets when the OS
allows it. Portable overwrite policy is still a future documentation point, so
tests use a missing target path. `rename_bool(source, target)` keeps the old boolean
compatibility shape, `rename_unchecked(source, target)` is the direct runtime
hook, and `rename_raw(source, target)` is the compatibility raw payload
form.

`hard_link(existing, link_path)` creates a new hard link to an existing file
and returns `Result[(), Error]`. The destination path must not already exist.
`hard_link_bool(existing, link_path)` keeps the old boolean compatibility
shape, `hard_link_unchecked(existing, link_path)` is the direct runtime hook,
and `hard_link_raw(existing, link_path)` keeps raw integer errors. Cross
filesystem limitations and platform differences follow the OS.

`symbolic_link(target, link_path)` creates a symbolic link at `link_path` that
points at `target` and returns `Result[(), Error]`. The destination path must
not already exist. `symbolic_link_bool(target, link_path)` keeps the old
boolean compatibility shape, `symbolic_link_unchecked(target, link_path)` is
the direct runtime hook, and `symbolic_link_raw(target, link_path)`
keeps raw integer errors. On the current Linux/glibc runtime path this is a
direct `symlink` wrapper. Relative targets are resolved by the host relative to
the directory containing `link_path`, not the caller's current directory.
Windows-specific file-vs-directory symlink behavior remains future platform
work.

`read_link(ref mut zone, path)` reads the target bytes stored in a symbolic
link and copies them into a `String` owned by the caller's zone as
`Result[String, Error]`. This is intentionally different from `canonicalize`:
`read_link` returns the link text as stored, while `canonicalize` resolves an
existing path to an absolute host path. Missing paths, regular files,
unreadable links, and targets that do not fit the current runtime buffer return
`Err(Error)`. `read_link_optional(ref mut zone, path)` and
`try_read_link(ref mut zone, path)` discard the reason; `read_link_unchecked`
asserts on failure.

`ensure_file(path)` is the file counterpart to `ensure_dir`: it creates one
empty file only when the path is missing and otherwise succeeds only for an
existing regular file. It returns `false` for directories, symlinks resolved to
non-regular targets, other path kinds, or missing parents. It is deliberately
not recursive and deliberately not an `OpenOptions` replacement; use
`open(path, mode)` or `create(path)` when the caller needs a handle and failure
reason.

`create_dir(path)` creates one directory with the current default permission
mode used by Ari's runtime shim and returns `Result[(), Error]`. It does not
create parent directories. `create_dir_bool(path)` keeps the old boolean compatibility shape, and
`create_dir_unchecked(path)` is the direct runtime hook.
`remove_dir(path)` removes one empty directory and returns `Result[(), Error]`;
`remove_dir_bool(path)` keeps the old boolean compatibility shape, and
`remove_dir_unchecked(path)` is the
direct runtime hook. The `*_raw` variants keep raw integer payloads for
compatibility.
`ensure_dir(path)` is the idempotent single-directory helper: it returns `true`
when `path` is already a directory, creates it when it is missing, and returns
`false` when another kind of path already exists or a parent directory is
missing. It is deliberately not recursive; use it when a test or tool needs one
known output directory without treating reruns as errors.
`create_dir_all(path)` creates every missing parent directory needed for
`path`, treats existing directories as success, and returns `Result[(),
Error]`. It fails when the path is empty, an existing non-directory blocks the
final path, or an intermediate component cannot be created or searched.
`create_dir_all_bool(path)` keeps the old boolean compatibility shape,
`create_dir_all_unchecked(path)` is the direct runtime hook, and
`create_dir_all_raw(path)` keeps raw integer errors. `ensure_dir_all`
returns a boolean idempotent setup shape for callers that read the operation as
setup rather than creation. Both helpers use the runtime's current default
directory mode for newly created components, with the host process umask still
applying. `remove_dir_all(path)` removes a directory tree recursively,
including regular files and symlinks inside the tree, and returns `Result[(),
Error]`. It rejects missing paths and non-directory roots, and it treats
symlinks as link entries to unlink rather than directories to follow.
`remove_dir_all_bool(path)` keeps the old boolean compatibility shape.

`open_dir(path)` opens one directory and returns `Result[Dir, Error]`;
`open_dir_raw(path)` keeps the raw compatibility payload.
`try_open_dir(path)` opens one directory and returns `Option[Dir]`.
`dir.next(ref mut zone)` returns the next entry name as `Option[String]`,
skipping the host `"."` and `".."` entries. `dir.close()` closes the directory
handle and preserves invalid-handle and host close failures as `Result`. Use
`dir.close_unchecked()` only for the older boolean compatibility shape. `Dir`
follows the same value-handle rule as `File`: close it once and do not reuse
copied handles after close.
`read_dir(ref mut zone, path)` is the convenient one-shot helper: it opens the
directory, collects `DirEntry` values, closes the handle, and returns
`Result[Vec[DirEntry], Error]`. `read_dir_optional(ref mut zone, path)` discards the reason,
and `read_dir_unchecked(ref mut zone, path)` asserts on failure. Use
`read_dir_names(ref mut zone, path)` when the call site only needs the old
`Vec[String]` name list. `read_dir_names_optional`,
`try_read_dir_names(ref mut zone, path)`, `try_read_dir(ref mut zone, path)`,
and `read_dir_names_unchecked` provide compatibility shapes for that name-list
operation. `read_dir_entries(ref mut zone, path)`
remains the explicit entry-list spelling and follows the same `Result` policy.
`read_dir_entries_optional`, `try_read_dir_entries`,
and `read_dir_entries_unchecked` provide the
matching compatibility shapes. `DirEntry::name()` and
`DirEntry::path()` return borrowed `String` references; the `*_equals` helpers
keep common tests short. `entry.metadata()`, `entry.file_type()`,
`entry.is_file()`, `entry.is_dir()`, and `entry.is_other()` follow symbolic
links, matching the path-level `fs::metadata` policy. Use
`entry.metadata_optional()`/`entry.try_metadata()` or
`entry.file_type_optional()`/`entry.try_file_type()` when absence-only
branching is enough, and `entry.metadata_unchecked()` or
  `entry.file_type_unchecked()` only for compatibility code that should panic on
  failure. `entry.symlink_metadata()` and `entry.is_symlink()` query the entry
  path with the no-follow `lstat` policy, with matching `_optional`, `try_*`, and
  `_unchecked` compatibility methods. The low-level `open_dir`, `read_dir_next`,
  and `close_dir` names exist for direct runtime-hook coverage, but ordinary code
  should prefer `read_dir`/`read_dir_entries` for Result-returning collection
  reads or `open_dir` plus the `Dir` methods for manual streaming.

## Feature Status

| Need | Status |
| --- | --- |
| open | Current: `open(path, mode)` returns `Error`, `open_optional(path, mode)`/`try_open(path, mode)` discard reasons, `open_unchecked(path, mode)` preserves the invalid-handle compatibility shape, `open(path, mode)` remains a compatibility alias, raw compatibility `open_raw`, Result/optional/unchecked convenience wrappers for read/write/append modes, and `OpenOptions` for named read/write/append/truncate/create/create-new policy plus `OpenOptions::open`/`open_optional`/`try_open`/`open_unchecked`/`open`/`open_raw`. |
| create | Current: `create(path)` returns `Error`, `create_optional(path)`/`try_create(path)` discard reasons, `create_unchecked(path)` preserves the invalid-handle compatibility shape, `create(path)` remains a compatibility alias, and `ensure_file(path)` provides non-truncating idempotent file setup. |
| read | Current: byte `read_byte`/`try_read_byte`, Result-first whole-file `read`/`read_to_string`, byte-vector `read_bytes`, `_optional`/`try_*` absence-only helpers, `_or_default` empty-string compatibility, and `_unchecked` asserting compatibility. Splitting byte-read EOF from byte-read errors remains roadmap. |
| write | Current: byte `write_byte`, `write_bytes`, `write_byte_unchecked`, `write_bytes_unchecked`, Result-first whole-file `write`, string convenience `write_string`, `write_bool` boolean compatibility, byte-counting `try_write`, and raw compatibility `write_raw`/`write_byte_raw`/`write_bytes_raw`. |
| append | Current: `"a"`/`"a+"` modes, Result-first whole-file `append`, `append_bool` boolean compatibility, byte-counting `try_append`, and raw compatibility `append_raw`. |
| truncate | Current: `truncate(path)` and `"w"`/`"w+"` modes. |
| metadata | Current: Result-first `metadata(path)`, `symlink_metadata(path)`, and `file_type(path)` with `Error`; `_optional`/`try_*` compatibility helpers that discard reasons; `_unchecked` compatibility helpers that assert; raw compatibility `metadata_raw(path)`, `symlink_metadata_raw(path)`, and `file_type_raw(path)` over Linux/glibc `stat`/`lstat`; direct predicates `is_file(path)`, `is_dir(path)`, `is_symlink(path)`, `is_other(path)`, `Metadata`, `FileKind`, and `Metadata` access/modification/status-change timestamps; creation/birth time is platform-policy roadmap work. |
| permissions | Current: access-style `can_read`, `can_write`, `can_execute`, `permissions`, Result-first stat-backed `mode(path)` with `Error`, `mode_optional(path)`/`try_mode(path)`, `mode_unchecked(path)`, raw compatibility `mode_raw`, and chmod-backed `set_mode`/`set_permissions`; richer ACL/owner/group policy is roadmap. |
| rename | Current: Result-first `rename(source, target)`, `rename_bool(source, target)` boolean compatibility, `rename_unchecked(source, target)` runtime hook, and raw compatibility `rename_raw`; portable overwrite policy is roadmap. |
| remove | Current: Result-first file removal with `remove(path)`/`remove_file(path)`, `remove_bool(path)`/`remove_file_bool(path)` boolean compatibility, raw compatibility `remove_raw`, Result-first empty directory removal with `remove_dir(path)` plus bool/raw compatibility, and Result-first recursive tree removal with `remove_dir_all(path)` using no-follow symlink policy for entries plus bool compatibility. |
| copy | Current: Result-first source streaming `copy(source, target)`, byte-counting `try_copy(source, target)`, `copy_bool(source, target)` boolean compatibility, and raw compatibility `copy_raw(source, target)` for byte files. |
| hard link | Current: Result-first `hard_link(existing, link_path)`, `hard_link_bool` boolean compatibility, `hard_link_unchecked` runtime hook, and `hard_link_raw` raw compatibility. |
| symbolic link | Current: Result-first `symbolic_link(target, link_path)`, `symbolic_link_bool` boolean compatibility, `symbolic_link_unchecked` runtime hook, `symbolic_link_raw` raw compatibility, Result-first `read_link(ref mut zone, path)`, `read_link_optional(ref mut zone, path)`/`try_read_link(ref mut zone, path)`, and `read_link_unchecked(ref mut zone, path)` on the Linux/glibc path; Windows split is roadmap. |
| canonicalize | Current: Result-first `canonicalize(ref mut zone, path)`, `canonicalize_optional(ref mut zone, path)`/`try_canonicalize(ref mut zone, path)`, and `canonicalize_unchecked(ref mut zone, path)` over the Linux/glibc `realpath` runtime path. |
| file handle lifecycle | Current: explicit Result-returning `close`, `position`, and `seek` on value `File` handles, `_unchecked` compatibility helpers, and `_raw` integer-error helpers. Close once; copied handles are not disarmed. |
| read directory | Current: Result-first entry-list `read_dir(ref mut zone, path)` and `read_dir_entries(ref mut zone, path)`, name-list `read_dir_names(ref mut zone, path)` with `try_read_dir` compatibility, `_optional`/`try_*` absence-only helpers, `_unchecked` asserting compatibility, `open_dir(path)` with `Error`, raw compatibility `open_dir_raw(path)`, `try_open_dir(path)`, `open_dir_unchecked(path)`, `Dir`, `DirEntry`, `dir.next(ref mut zone)`, `dir.close()`/`dir.close_unchecked()`, borrowed entry name/path methods, and lazy `DirEntry` metadata/file-kind predicates; richer per-entry errors are roadmap. |
| create directory | Current: Result-first single-directory `create_dir(path)`, `create_dir_bool(path)` boolean compatibility, raw compatibility `create_dir_raw(path)`, idempotent `ensure_dir(path)`, Result-first recursive `create_dir_all(path)`, `create_dir_all_bool(path)` boolean compatibility, raw compatibility `create_dir_all_raw(path)`, and `ensure_dir_all(path)` for missing parent directories. |
| temporary files | Current: Result-first `temp_file(ref mut zone)` / `temp_file_in(ref mut zone, prefix)` and `temp_dir(ref mut zone)` / `temp_dir_in(ref mut zone, prefix)` wrappers over the hosted POSIX `mkstemp`/`mkdtemp` implementation. `TempFile` owns the descriptor and path buffer, `TempDir` owns the path buffer, and callers should explicitly close/remove the resource when they want recoverable cleanup errors. |
| path manipulation | Current: source lexical helpers in `std::path`, borrowed `PathBytes`/`Path` views, and owned POSIX `PathBuf` as a `std::string::String` alias; platform-specific owned paths remain roadmap. |
| file locking | Current: hosted POSIX advisory locks through `flock(2)` with shared, exclusive, nonblocking try, and unlock helpers on `File`; Windows and mandatory-locking policy remain roadmap. |

Temporary files and directories are exposed from `std::fs` because ordinary
filesystem code should not need to reach through `std::process` for scratch
paths. They reuse the same hosted POSIX implementation and handle types as
`std::process`: `TempFile` owns an open file descriptor and the allocated path
buffer, while `TempDir` owns the allocated path buffer. `TempFile::close()`,
`TempFile::remove()`, `TempFile::close_and_remove()`, and `TempDir::remove()`
return `Result[(), Error]`; call them explicitly when cleanup failures matter.
Drop cleanup is best-effort and cannot report errors.

File locks are advisory hosted locks. `lock_shared(file)` and
`lock_exclusive(file)` block according to the host `flock(2)` behavior until
the lock can be acquired or an OS error is reported. `try_lock_shared(file)` and
`try_lock_exclusive(file)` use the nonblocking host flag and return
`Ok(false)` when the lock would block; invalid handles and other OS failures
return `Err(Error)`. `unlock(file)` releases the lock associated with the file
descriptor. Closing the file also releases host locks, but call `unlock` when
the release point needs to be visible in source.

## Examples

Write and read a small file:

```ari
fn main() -> i64 {
  let path = "build/prelude/example-fs.tmp";
  let writer = fs::open(path, "w").unwrap();

  var data = ['A', 'B', 'C'];
  if writer.write_bytes(data.as_slice()).is_err() {
    if writer.close().is_err() {
      return 2;
    }
    return 2;
  }
  writer.close().unwrap();

  let reader = fs::open(path, "r").unwrap();
  let first = reader.read_byte();
  reader.close().unwrap();
  fs::remove(path).unwrap();
  return first - 65;
}
```

Handle absence without losing the reason:

```ari
let file = fs::open("missing-file.txt", "r");
if file.is_err() {
  return 0;
}
```

Append to an existing file:

```ari
let appender = fs::open(path, "a").unwrap();
appender.write_byte('\n').unwrap();
appender.close().unwrap();
```

Use the whole-file byte helpers for small files:

```ari
var data = ['A', 'B'];
fs::write(path, data.as_slice()).unwrap();

var tail = ['C'];
fs::append(path, tail.as_slice()).unwrap();

var zone = zone::create(4096);
let text = fs::read(ref mut zone, path).unwrap();
let first = text.get(0);
zone::destroy(zone);
```

Create, copy, and truncate small files:

```ari
let file = fs::create(path).unwrap();
file.close().unwrap();

let backup_path = "build/prelude/example-fs.bak";
fs::copy(path, backup_path).unwrap();
fs::truncate(path);
```

Rename a file and create a single empty directory:

```ari
let moved = "build/prelude/example-fs-moved.tmp";
if fs::rename(path, moved).is_ok() {
  fs::remove(moved).unwrap();
}

let dir = "build/prelude/example-fs-dir.tmp";
if fs::create_dir(dir).is_ok() {
  fs::remove_dir(dir).unwrap();
}
```

Create nested output directories:

```ari
let nested = "build/prelude/example-fs-dir.tmp/cache/shards";
if fs::create_dir_all(nested).is_ok() {
  fs::write_string("build/prelude/example-fs-dir.tmp/cache/shards/data.txt", "ok").unwrap();
}
```

Remove a directory tree:

```ari
let tree = "build/prelude/example-fs-dir.tmp";
if fs::remove_dir_all(tree).is_ok() {
  assert(!fs::exists(tree));
}
```

Read directory entries:

```ari
var zone = zone::create(512);
let entries = fs::read_dir(ref mut zone, "build/prelude").unwrap();

var index = 0;
while index < entries.len() {
  let entry = entries.get(index);
  if entry.name_equals("example-fs.tmp") {
    index = entries.len();
  }
  index = index + 1;
}
zone::destroy(zone);
```

Read only names when entries are not needed:

```ari
var zone = zone::create(512);
let names = fs::read_dir_names(ref mut zone, "build/prelude").unwrap();
if names.len() > 0 {
  let first = names.get(0);
  if first.is_empty() {
    panic();
  }
}
zone::destroy(zone);
```

Manually stream a directory when you do not want to collect every name:

```ari
var zone = zone::create(512);
match fs::open_dir("build/prelude") {
  std::Ok(dir) => {
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
    dir.close().unwrap();
  }
  std::Err(_) => {
    panic();
  }
}
zone::destroy(zone);
```

Read entries with joined paths:

```ari
var zone = zone::create(512);
let entries = fs::read_dir_entries(ref mut zone, "build/prelude").unwrap();
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
let entries = fs::read_dir_entries(ref mut zone, "build/prelude").unwrap();
var index = 0;
while index < entries.len() {
  let entry = entries.get(index);
  if entry.is_file() {
    match entry.metadata() {
      std::Ok(info) => {
        if info.len() > 0 {
          index = entries.len();
        }
      }
      std::Err(_) => {}
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
if fs::hard_link(path, hard).is_ok() {
  fs::remove(hard).unwrap();
}
if fs::symbolic_link(symbolic_target, symbolic).is_ok() {
  fs::remove(symbolic).unwrap();
}
```

Check current-process access permissions:

```ari
let access = fs::permissions(path);
if access.can_read() && access.can_write() {
  match fs::open(path, "rw") {
    std::Ok(file) => {
      file.close().unwrap();
    }
    std::Err(_) => {
      return 1;
    }
  }
}
```

Change POSIX permission bits:

```ari
let read_only = fs::Permissions::read_only();
if fs::set_permissions(path, read_only) {
  match fs::mode(path) {
    std::Ok(bits) => {
      if bits == 292 {
        return 0;
      }
    }
    std::Err(_) => {}
  }
}
```

Open an existing file for both reading and writing:

```ari
match fs::open(path, "rw") {
  std::Ok(file) => {
    let first = file.read_byte();
    file.write_byte((first + 1) as u8).unwrap();
    file.close().unwrap();
  }
  std::Err(_) => {
    return 1;
  }
}
```

## Current Limits

- This slice is byte-oriented and keeps text encoding explicit.
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
- Prefer natural Result-returning names when the caller needs a reason.
  Ordinary old result-suffixed migration aliases have been removed from `std::fs`.
  Compatibility bool, `Option`, empty-string, unchecked, raw, and `-1`
  sentinel helpers remain for compact code and older tests, but new filesystem
  APIs should use `std::error::Error` instead of adding more sentinel
  conventions.

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
their `_optional` compatibility helpers,
`open_raw`, `create_raw`, and `OpenOptions::open_raw` for
invalid input, missing files, exclusive-create failures, and successful
handles. `std-fs-read-write.ari` covers natural Result-returning whole-file
write/append, `try_write`/`try_append` byte counts, read-to-byte-string,
explicit missing-file default reads, and truncating rewrite behavior.
`std-fs-read-result.ari` covers natural `Error` whole-file read helpers,
successful byte-string reads, missing-file `NotFound`, and compatibility
`try_read`/`read_optional` absence plus unchecked read aliases.
`std-fs-query-result.ari` covers direct `Error` metadata, no-follow metadata,
path-kind, mode, canonicalization, symbolic-link target, directory-open, and
directory-list helpers.
`std-fs-byte-result.ari` covers natural `write`/`append`/`copy` direct Error
payloads plus raw compatibility helpers.
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
`remove`, `rename`, `create_dir`, and
`remove_dir` success/Error cases plus raw compatibility variants.
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
the Result-returning `read_link` helper. `std-fs-symlink-metadata.ari` covers
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
- Grow temporary-file policy beyond the current hosted POSIX wrappers only
  after platform cleanup and directory-selection rules are documented.
- Grow advisory locking beyond the current POSIX `flock(2)` helpers only after
  Windows and mandatory-locking policy are documented.
- Migrate remaining bool/Option-only filesystem helpers to richer
  `Result[..., Error]` variants where callers need diagnostics.
- Promote `File` toward an owned resource handle when the compiler can express
  OS-resource ownership and drop policy cleanly.
