# std::error

`std::error` is the shared vocabulary for recoverable failures. Ari does not
use exceptions for ordinary control flow: absence uses `Option[T]`, failure
with information uses `Result[T, E]`, and programmer mistakes use `assert` or
`panic`.

## Public API

```ari
enum Kind {
  NotFound,
  PermissionDenied,
  AlreadyExists,
  InvalidInput,
  InvalidData,
  Interrupted,
  WouldBlock,
  TimedOut,
  BrokenPipe,
  UnexpectedEof,
  Unsupported,
  OutOfMemory,
  ResourceBusy,
  InProgress,
  NotDirectory,
  IsDirectory,
  DirectoryNotEmpty,
  StorageFull,
  CrossDevice,
  Other,
  ConnectionRefused,
}

struct Error
type ErrorKind = Kind

new(kind: Kind) -> Error
with_code(kind: Kind, code: i64) -> Error
try_with_code(kind: Kind, code: i64) -> Option[Error]
from_errno(code: i64) -> Error
from_os_code(code: i64) -> Error
try_from_errno(code: i64) -> Option[Error]
try_from_os_code(code: i64) -> Option[Error]
from_raw(raw: i64) -> Error
try_from_raw(raw: i64) -> Option[Error]
map_raw[T](value: Result[T, i64]) -> Result[T, Error]
map_errno[T](value: Result[T, i64]) -> Result[T, Error]
map_os_code[T](value: Result[T, i64]) -> Result[T, Error]
to_raw[T](value: Result[T, Error]) -> Result[T, i64]

kind(ref Error) -> Kind
code(ref Error) -> i64
raw(ref Error) -> i64
is_kind(ref Error, kind: Kind) -> bool
is_not_found(ref Error) -> bool
is_interrupted(ref Error) -> bool
is_connection_refused(ref Error) -> bool
is_retryable(ref Error) -> bool
name(kind: Kind) -> string
message(ref Error) -> string
```

`std` re-exports `Error` and `ErrorKind`:

```ari
let reason: Error = error::from_errno(2);
let kind: ErrorKind = reason.kind();
```

`error::ErrorKind` is also an alias of `error::Kind`, so module-local code can
use either name without changing the payload shape.

`Error` currently stores a compact one-word representation. That is deliberate:
it lets filesystem and other OS-facing helpers return direct
`Result[T, Error]` values while still allowing raw compatibility boundaries.
Use `map_raw` when adapting an older `Result[T, i64]` helper into the
preferred `Result[T, Error]` shape. Use `to_raw` only when a runtime,
FFI, or compatibility test still needs the compact integer bridge.
Use `map_errno` or `map_os_code` only when the `i64` payload is
the platform error code itself rather than a packed Ari `Error.raw()` value.
Use strict constructors such as `with_code`, `from_errno`, and `from_raw`
when values are trusted or produced by Ari itself. Use `try_with_code`,
`try_from_errno`, and `try_from_raw` at FFI, OS, serialized-data, and test
fixture boundaries where invalid negative or out-of-range codes should become
`None`.

```ari
fn open_like() -> Result[i64, Error] {
  let reason = error::from_errno(2);
  return Err<i64, Error>(reason);
}

match open_like() {
  Ok(fd) => { /* use fd */ }
  Err(reason) => {
    if reason.is_not_found() {
      /* recover */
    }
  }
}
```

Bridge a legacy raw result:

```ari
fn old_open_like() -> Result[i64, i64] {
  let reason = error::from_errno(2);
  return Err<i64, i64>(reason.raw());
}

let result: Result[i64, Error] = error::map_raw<i64>(old_open_like());
```

Fallible error reconstruction:

```ari
match error::try_from_errno(code_from_c) {
  std::Some(reason) => {
    if reason.is_retryable() {
      /* try again */
    }
  }
  std::None => {
    /* invalid error code from the boundary */
  }
}
```

## Error Kinds

`Kind` groups platform-specific failures into stable categories. `code()`
preserves the platform code when one exists. `from_errno` maps common
POSIX/Linux `errno` values such as `ENOENT`, `EACCES`, `EEXIST`, `EINVAL`,
`EINTR`, `EAGAIN`, `ETIMEDOUT`, `EPIPE`, `ENOMEM`, `EBUSY`, `ENOTDIR`,
`EISDIR`, `ENOTEMPTY`, `ENOSPC`, `EXDEV`, `ECONNREFUSED`, `ENOSYS`,
`ENOTSUP`, and `EAFNOSUPPORT`.

Use `is_retryable` when an operation may be worth trying again. It currently
returns true for `Interrupted`, `WouldBlock`, `TimedOut`, and `InProgress`.

`name(kind)` and `message(ref error)` return stable lowercase messages for
failure reports. They are intentionally plain borrowed `string` values today;
localized text, structured error fields, and richer formatting policy are
future work.
`Error` and `Kind` implement the source `Display` and `Debug` traits. Display
uses the stable lowercase message (`"not found"`). Debug uses stable
diagnostic wrappers such as `Error(not found, code=2)` and
`ErrorKind(not found)`, which keeps logs readable without exposing raw packing.

Domain modules re-export the shared error vocabulary so signatures can stay
local without creating incompatible error types:

```ari
fs::Error
fs::ErrorKind
io::Error
io::ErrorKind
net::Error
net::ErrorKind
os::Error
os::ErrorKind
process::Error
process::ErrorKind
```

Those aliases all refer to `std::error::Error` and `std::error::Kind`.
Prefer them in public module signatures when it improves readability, but keep
the payload semantically shared so callers can move errors across modules.

## Error Handling Policy

- Use `Option[T]` when absence is expected and has no extra information.
- Use `Result[T, E]` when callers must inspect or propagate failure.
- Use `std::error::Error` for OS/runtime/library errors.
- Use module aliases such as `fs::Error` or `net::Error` when the module path
  makes a signature clearer; do not define incompatible per-module error
  payloads unless the error needs additional structured fields.
- Keep `raw()` and `Result[T, i64]` forms at runtime, FFI, and compatibility
  boundaries only.
- Use `try_*` constructors at untrusted boundaries instead of asserting on
  malformed codes.
- Use `assert`, `panic`, `todo`, and `unreachable` only for programmer errors
  or intentionally aborting paths.
- Avoid bool-only APIs for operations where callers need to distinguish
  retryable, missing, permission, invalid input, or unsupported failures.

## Current Limits

- `std::fs`, `std::io`, `std::net`, `std::os`, and `std::process` now expose
  shared `Error`/`ErrorKind` aliases and first direct `Result[T, Error]`
  helper surfaces, while compatibility bool/Option/raw helpers remain at older
  boundaries. Remaining low-level runtime hooks should be migrated in small
  tested slices.
- `from_errno` is a POSIX/Linux-oriented seed. Windows `GetLastError` mapping
  should live behind target-aware runtime wrappers.
- There are no owned dynamic error messages, structured path/operation fields,
  stack traces, source locations, panic payloads, typed exceptions, or generic
  formatter integration in this module yet.

## Tests

Focused positive coverage:

```text
tests/cases/standard-library/ok/error/std-error-basic.ari
tests/cases/standard-library/ok/error/std-error-integration.ari
tests/cases/standard-library/ok/error/std-error-validation.ari
```

The test covers `Kind`, compact `Error` values, `from_errno`, root aliases,
predicate helpers, message names, method wrappers, direct `Result[T, Error]`,
and raw-result conversion helpers. `std-error-validation.ari` covers
`try_with_code`, `try_from_raw`, and `try_from_errno` invalid-input handling.
`std-error-integration.ari` covers module aliases, errno-result conversion,
conversion traits, and Display/Debug formatting.
