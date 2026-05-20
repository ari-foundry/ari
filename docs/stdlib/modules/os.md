# std::os

`std::os` is the boundary for operating-system concepts that are too sharp for
portable modules to expose as loose integers. The current slice introduces a
small non-owning file-descriptor view. It is useful for code that needs to
identify standard descriptors or carry a descriptor value through an API
without making that value look like an ordinary counter.

This module is intentionally not a raw syscall collection. Closing, duplicating,
setting close-on-exec, nonblocking mode, `fcntl`, `poll`, Linux `epoll`,
signals, and memory mapping all need owned handle and error policies before
they become stable public APIs.

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

## Tests

Focused tests:

```text
tests/cases/standard-library/ok/os/std-os-fd.ari
```

`make check-std-api` tracks the public declarations. `make check-prelude`
compiles and runs the focused descriptor-view fixture.

## Future Work

- Add an owned descriptor type that closes exactly once.
- Add fallible descriptor operations returning `std::error::Error` once
  `Result[T, Error]` is directly representable.
- Add close-on-exec and nonblocking setters before exposing readiness APIs.
- Add `poll` as the first portable readiness primitive.
- Add Linux-only `epoll`, `eventfd`, `timerfd`, `signalfd`, `pidfd`, and
  `memfd` under target-guarded APIs after owned descriptors are stable.
- Add signal and memory-mapping APIs only after lifetime and safety rules are
  documented.
