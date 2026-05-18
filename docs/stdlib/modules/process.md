# std::process

`std::process` is the user-facing module for current-process operations and,
later, child-process handles. The first implemented slice is intentionally
small: read the current process id, exit with an explicit status code, and use
natural status helper names in source Ari.

## Current API

```ari
process::id()
process::exit(code)
process::success()
process::failure()
process::is_success(code)
process::is_failure(code)
```

`id()` returns the host process id as `i64`. On the current Linux/LLVM path it
lowers through a runtime hook backed by `getpid`.

`exit(code)` terminates the current process with the low bits of `code` as the
host exit status. It does not return.

`success()` returns `0`, `failure()` returns `1`, and the predicate helpers
make code that checks status values read clearly without hard-coding every
comparison at the call site.

## Example

```ari
fn main() -> i64 {
  let pid = process::id();
  if pid <= 0 {
    return process::failure();
  }

  return process::success();
}
```

Explicit termination:

```ari
fn main() -> i64 {
  process::exit(42);
  return 0;
}
```

## Current Limits

- `id` and `exit` are current-process helpers only.
- Child processes, `spawn`, `wait`, platform `fork`, current directory, and
  process handles are future work.
- Exit runs through the host process immediately. Do not expect Ari destructors
  or zone cleanup to run after `process::exit`.
- Platform-specific process APIs should stay behind `std::process` or a future
  `std::os` submodule instead of becoming a raw syscall grab bag.

## Tests

Focused positive coverage:

```text
tests/cases/standard-library/ok/std-process-basic.ari
tests/cases/standard-library/ok/std-process-exit.ari
```

`make check-prelude` emits LLVM, checks the runtime hook symbols, and executes
both programs. Public declarations are tracked in `tests/std_api_manifest.txt`
and checked by `make check-std-api`.
