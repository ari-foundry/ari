# std::os

`std::os` is the boundary for operating-system concepts that are too sharp for
portable modules to expose as loose integers. The current slice introduces a
small descriptor model with two shapes:

- `Fd`: a non-owning descriptor identity view.
- `OwnedFd`: a descriptor owner that has the responsibility to close once.

This module is intentionally not a raw syscall collection. Close and duplicate
are the first owner operations because they define descriptor lifetime.
Close-on-exec, nonblocking mode, `fcntl`, `poll`, Linux `epoll`, signals, and
memory mapping are added in small layers after descriptor error policy is
stable.

## API

```ari
os::fd(raw)
os::invalid()
os::stdin()
os::stdout()
os::stderr()

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
owned.close()

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
  let file = std::fs::open("build/output.tmp", "w");
  var owned = std::os::OwnedFd::from_raw(file.descriptor().raw());
  var copy = owned.try_clone().unwrap();
  copy.close();
  if owned.close() {
    return 0;
  }
  return 1;
}
```

## Tests

Focused tests:

```text
tests/cases/standard-library/ok/os/std-os-fd.ari
tests/cases/standard-library/ok/os/std-os-owned-fd.ari
tests/cases/standard-library/ok/os/std-os-owned-fd-duplicate.ari
```

`make check-std-api` tracks the public declarations. `make check-prelude`
compiles and runs the focused descriptor fixtures.

## Future Work

- Add fallible descriptor operations returning `std::error::Error` once
  `Result[T, Error]` is directly representable.
- Add close-on-exec and nonblocking setters before exposing readiness APIs.
- Add duplication flags such as close-on-exec-on-dup when the API shape is
  stable.
- Add `poll` as the first portable readiness primitive.
- Add Linux-only `epoll`, `eventfd`, `timerfd`, `signalfd`, `pidfd`, and
  `memfd` under target-guarded APIs after owned descriptors are stable.
- Add signal and memory-mapping APIs only after lifetime and safety rules are
  documented.
