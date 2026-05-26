# std::os

`std::os` is the boundary for operating-system concepts that are too sharp for
portable modules to expose as loose integers. The current slice introduces a
small descriptor model with two shapes:

- `Fd`: a non-owning descriptor identity view.
- `OwnedFd`: a descriptor owner that has the responsibility to close once.
- `Pipe`: a pair of owned read/write descriptors created by `pipe()`.

This module is intentionally not a raw syscall collection. Close, duplicate,
pipe creation, close-on-exec, and nonblocking mode are the first owner
operations because they define descriptor lifetime, inheritance, data-flow
edges, and blocking behavior. Broad `fcntl`, `poll`, Linux `epoll`, signals,
and memory mapping are added in small layers after descriptor error policy is
stable.

## API

```ari
os::Error
os::ErrorKind
os::fd(raw)
os::invalid()
os::stdin()
os::stdout()
os::stderr()
os::pipe()
os::pipe_optional()

fd.raw()
fd.is_valid()
fd.is_invalid()
fd.is_stdin()
fd.is_stdout()
fd.is_stderr()
fd.is_standard()
fd.equals(other)

OwnedFd::from_raw(raw)
OwnedFd::invalid()
owned.raw()
owned.as_fd()
owned.is_open()
owned.is_closed()
owned.take()
owned.try_clone()
owned.clone()
owned.close_on_exec()
owned.close_on_exec_optional()
owned.set_close_on_exec(enabled)
owned.set_close_on_exec_bool(enabled)
owned.is_nonblocking()
owned.is_nonblocking_optional()
owned.set_nonblocking(enabled)
owned.set_nonblocking_bool(enabled)
owned.close()
owned.close_bool()

pipe.read_end()
pipe.write_end()
pipe.take_read_end()
pipe.take_write_end()
pipe.close_read_end()
pipe.close_read_end_bool()
pipe.close_write_end()
pipe.close_write_end_bool()
pipe.close()
pipe.close_bool()

file.descriptor()
```

`Fd` is a borrowed view, not an owner. Dropping an `Fd` never closes the
underlying descriptor. Use it when an API needs to inspect or pass descriptor
identity; keep lifetime and closing policy with the module that actually owns
the resource.

`invalid()` returns descriptor `-1`. `stdin()`, `stdout()`, and `stderr()`
return descriptors `0`, `1`, and `2`. `is_standard()` is true for those three
values only.

`std::fs::File.descriptor()` returns an `Fd` view over the current file handle.
It does not transfer ownership and it does not keep the file open if the
original `File` is later closed. This shape keeps `std::fs` readable today and
leaves room for a future `OwnedFd`/borrowed-descriptor split.

`OwnedFd::from_raw(raw)` creates an owner around an existing descriptor. Use it
only when the caller is taking responsibility for exactly one close. `as_fd()`
borrows the descriptor as `Fd`; `take()` disarms the owner and returns the
borrowed view without closing. `close()` disarms the owner before invoking the
runtime close hook, so calling `close()` twice on the same `OwnedFd` will not
close a reused descriptor accidentally.

`try_clone()` duplicates the underlying descriptor and returns a new `OwnedFd`
when the host accepts the operation. It returns `None` for closed descriptors
or OS-level duplicate failures. The original and cloned owners must be closed
independently.
`clone()` is the error-preserving form: closed descriptors return
`Error(InvalidInput)` and host duplicate failures return `Error(Other)`.

`close_on_exec()` reads whether the descriptor is marked close-on-exec and
returns `Result[bool, Error]`. `set_close_on_exec(enabled)` updates that flag
and returns `Result[(), Error]`. Both methods live on `OwnedFd` first so flag
mutation stays near the code that owns the descriptor. Closed handles use
`Error(InvalidInput)` and host flag failures use `Error(Other)`.
`close_on_exec_optional()` and `set_close_on_exec_bool(enabled)` are
compatibility helpers for callers that intentionally discard the reason.

`is_nonblocking()` reads whether the descriptor has nonblocking mode enabled
and returns `Result[bool, Error]`. `set_nonblocking(enabled)` toggles that flag
and returns `Result[(), Error]`. The API intentionally avoids exposing raw flag
integers; future socket and readiness wrappers can build on this
owned-descriptor policy. `is_nonblocking_optional()` and
`set_nonblocking_bool(enabled)` are the compatibility forms.

`close()` disarms an open `OwnedFd` before calling the runtime close hook and
returns `Error(InvalidInput)` if the owner was already closed. `close_bool()`
follows the same lifecycle rule while discarding the reason. After any
`close()` call on an open owner, the handle is invalidated even if the host
reports a close failure, so the same `OwnedFd` cannot accidentally close a
reused descriptor later.

`pipe()` returns `Result[Pipe, Error]`; `pipe_optional()` returns
`Option[Pipe]`. A successful `Pipe` owns both ends of the host pipe:
`read_end()` and `write_end()` borrow descriptor views, `take_*` methods move
one owned end out and leave that side invalid inside the pair, and the
`close_*`/`close()` methods explicitly release the remaining owned ends.
`Pipe::close()` is idempotent for a pair whose ends have already been
taken or closed: it returns `Ok(())` when no owned end remains open. Closing a
specific already-closed side through `close_read_end()` or
`close_write_end()` returns `Error(InvalidInput)`. This is the OS-level
ownership layer; use `std::io::pipe()` when the same pipe should be split into
`Reader`/`Writer` adapters.

## Example

```ari
fn is_output(value: std::os::Fd) -> bool {
  return value.is_stdout() || value.is_stderr();
}

fn main() -> i64 {
  let out = std::os::stdout();
  if is_output(out) {
    return out.raw();
  }
  return 1;
}
```

```ari
fn main() -> i64 {
  let file = std::fs::open("build/output.tmp", "w").unwrap();
  var owned = std::os::OwnedFd::from_raw(file.descriptor().raw());
  owned.set_close_on_exec(true).unwrap();
  owned.set_nonblocking(true).unwrap();
  var copy = owned.try_clone().unwrap();
  copy.close().unwrap();
  if owned.close().is_ok() {
    return 0;
  }
  return 1;
}
```

```ari
fn main() -> i64 {
  var pipe = std::os::pipe().unwrap();
  if !pipe.write_end().is_valid() {
    return 1;
  }
  var reader = pipe.take_read_end();
  var writer = pipe.take_write_end();
  reader.close().unwrap();
  writer.close().unwrap();
  return 0;
}
```

## Tests

Focused tests:

```text
tests/cases/standard-library/ok/os/std-os-fd.ari
tests/cases/standard-library/ok/os/std-os-owned-fd.ari
tests/cases/standard-library/ok/os/std-os-owned-fd-duplicate.ari
tests/cases/standard-library/ok/os/std-os-owned-fd-flags.ari
tests/cases/standard-library/ok/os/std-os-owned-fd-nonblocking.ari
tests/cases/standard-library/ok/os/std-os-pipe.ari
```

`std-os-owned-fd.ari` covers closed-handle `Result` errors, close invalidation,
and duplicate/flag query failure behavior. `std-os-pipe.ari` covers
`pipe`, `Pipe::close`, and flag result helpers on live pipe
ends. `make check-std-api` tracks the public declarations. `make
check-prelude` compiles and runs the focused descriptor fixtures.

## Future Work

- Add duplication flags such as close-on-exec-on-dup when the API shape is
  stable.
- Add richer descriptor flag tests on sockets and terminals.
- Add `poll` as the first portable readiness primitive.
- Add Linux-only `epoll`, `eventfd`, `timerfd`, `signalfd`, `pidfd`, and
  `memfd` under target-guarded APIs after owned descriptors are stable.
- Add signal and memory-mapping APIs only after lifetime and safety rules are
  documented.
