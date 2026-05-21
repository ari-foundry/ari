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
try_with_code(kind: Kind, code: i64) -> Option[Error]
from_errno(code: i64) -> Error
try_from_errno(code: i64) -> Option[Error]
from_raw(raw: i64) -> Error
try_from_raw(raw: i64) -> Option[Error]
from_raw_result[T](value: Result[T, i64]) -> Result[T, Error]
to_raw_result[T](value: Result[T, Error]) -> Result[T, i64]

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
it lets filesystem and other OS-facing helpers return direct
`Result[T, Error]` values while still allowing raw compatibility boundaries.
Use `from_raw_result` when adapting an older `Result[T, i64]` helper into the
preferred `Result[T, Error]` shape. Use `to_raw_result` only when a runtime,
FFI, or compatibility test still needs the compact integer bridge.
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

let result: Result[i64, Error] = error::from_raw_result<i64>(old_open_like());
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
- Use `std::error::Error` for OS/runtime/library errors.
- Keep `raw()` and `Result[T, i64]` forms at runtime, FFI, and compatibility
  boundaries only.
- Use `try_*` constructors at untrusted boundaries instead of asserting on
  malformed codes.
- Use `assert`, `panic`, `todo`, and `unreachable` only for programmer errors
  or intentionally aborting paths.
- Avoid bool-only APIs for operations where callers need to distinguish
  retryable, missing, permission, invalid input, or unsupported failures.

## Current Limits

- `std::fs`, `std::io`, `std::net`, and `std::process` now expose first direct
  `Result[T, Error]` helper surfaces, while compatibility bool/Option/raw
  helpers remain at older boundaries. Remaining low-level runtime hooks should
  be migrated in small tested slices.
- `from_errno` is a POSIX/Linux-oriented seed. Windows `GetLastError` mapping
  should live behind target-aware runtime wrappers.
- There are no owned dynamic error messages, structured path/operation fields,
  stack traces, source locations, panic payloads, typed exceptions, or generic
  formatter integration in this module yet.

## Tests

Focused positive coverage:

```text
tests/cases/standard-library/ok/error/std-error-basic.ari
tests/cases/standard-library/ok/error/std-error-validation.ari
```

The test covers `Kind`, compact `Error` values, `from_errno`, root aliases,
predicate helpers, message names, method wrappers, direct `Result[T, Error]`,
and raw-result conversion helpers. `std-error-validation.ari` covers
`try_with_code`, `try_from_raw`, and `try_from_errno` invalid-input handling.
