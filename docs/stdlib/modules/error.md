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
}

struct Error

new(kind: Kind) -> Error
with_code(kind: Kind, code: i64) -> Error
from_errno(code: i64) -> Error
from_raw(raw: i64) -> Error

kind(ref Error) -> Kind
code(ref Error) -> i64
raw(ref Error) -> i64
is_kind(ref Error, kind: Kind) -> bool
is_not_found(ref Error) -> bool
is_interrupted(ref Error) -> bool
is_retryable(ref Error) -> bool
name(kind: Kind) -> string
message(ref Error) -> string
```

`std` re-exports `Error` and `ErrorKind`:

```ari
let reason: Error = error::from_errno(2);
let kind: ErrorKind = reason.kind();
```

`Error` currently stores a compact one-word representation. That is deliberate:
today's generic enum payload storage can carry scalar success values and scalar
error values together, so `raw()` and `from_raw()` are the bridge for
`Result[T, i64]` until `Result[T, Error]` can carry mixed aggregate payloads
directly.

```ari
fn open_like() -> Result[i64, i64] {
  let reason = error::from_errno(2);
  return Err<i64, i64>(reason.raw());
}

match open_like() {
  Ok(fd) => { /* use fd */ }
  Err(raw) => {
    let reason = error::from_raw(raw);
    if reason.is_not_found() {
      /* recover */
    }
  }
}
```

## Error Kinds

`Kind` groups platform-specific failures into stable categories. `code()`
preserves the platform code when one exists. `from_errno` maps common
POSIX/Linux `errno` values such as `ENOENT`, `EACCES`, `EEXIST`, `EINVAL`,
`EINTR`, `EAGAIN`, `ETIMEDOUT`, `EPIPE`, `ENOMEM`, `EBUSY`, `ENOTDIR`,
`EISDIR`, `ENOTEMPTY`, `ENOSPC`, and `EXDEV`.

Use `is_retryable` when an operation may be worth trying again. It currently
returns true for `Interrupted`, `WouldBlock`, `TimedOut`, and `InProgress`.

`name(kind)` and `message(ref error)` return stable lowercase messages for
failure reports. They are intentionally plain borrowed `string` values today;
localized text, structured error fields, and formatter integration are future
work.

## Error Handling Policy

- Use `Option[T]` when absence is expected and has no extra information.
- Use `Result[T, E]` when callers must inspect or propagate failure.
- Use `std::error::Error` or its `raw()` form for OS/runtime/library errors.
- Use `assert`, `panic`, `todo`, and `unreachable` only for programmer errors
  or intentionally aborting paths.
- Avoid bool-only APIs for operations where callers need to distinguish
  retryable, missing, permission, invalid input, or unsupported failures.

## Current Limits

- `Result[T, Error]` is roadmap work for mixed payload storage. Use
  `Result[T, i64]` with `error.raw()` and `error::from_raw(raw)` for now when
  `T` is scalar.
- `from_errno` is a POSIX/Linux-oriented seed. Windows `GetLastError` mapping
  should live behind target-aware runtime wrappers.
- There is no stack trace, source location, panic payload, or typed exception
  mechanism in this module.
- Existing `std::fs`, `std::io`, `std::process`, and `std::net` APIs still use
  bools, `Option`, or sentinel values in places; new richer APIs should return
  error values instead of extending those sentinel conventions.

## Tests

Focused positive coverage:

```text
tests/cases/standard-library/ok/error/std-error-basic.ari
```

The test covers `Kind`, compact `Error` values, `from_errno`, root aliases,
predicate helpers, message names, method wrappers, and the current
`Result[T, i64]` raw-error bridge.
