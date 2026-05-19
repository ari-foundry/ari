# std::time

`std::time` provides the first small, runtime-backed time surface for Ari. It
keeps raw OS time reads behind `extern "ari"` hooks, then exposes source value
wrappers so ordinary code can talk about durations, monotonic instants, and
wall-clock time without passing naked integer timestamps everywhere.

## Current API

```ari
time::monotonic_nanos()
time::unix_nanos()
time::sleep_nanos(nanos)

time::nanoseconds(value)
time::microseconds(value)
time::milliseconds(value)
time::seconds(value)
time::now()
time::system_now()
time::elapsed(start)
time::sleep(duration)

Duration::zero()
duration.as_nanos()
duration.as_micros()
duration.as_millis()
duration.as_seconds()
duration.is_zero()
duration.add(other)
duration.saturating_sub(other)

Instant::now()
instant.as_nanos()
instant.duration_since(earlier)
instant.try_duration_since(earlier)
instant.elapsed()

SystemTime::now()
system_time.as_unix_nanos()
system_time.duration_since_unix_epoch()
```

`Duration` values are non-negative. The constructor helpers assert if given a
negative value. `as_micros`, `as_millis`, and `as_seconds` truncate toward
zero, which keeps them useful for status checks and coarse timing.

`Instant` is monotonic and should be used for measuring elapsed time. Use
`now()` or `Instant::now()` to read it. `duration_since` asserts when the
earlier instant is actually later; `try_duration_since` returns
`Option[Duration]` for that branch.

`SystemTime` reads wall-clock Unix time in nanoseconds. Use it for timestamps,
not elapsed-time measurement. Wall-clock time can move if the host clock is
changed.

`sleep(duration)` sleeps the current thread for the duration. The first runtime
hook is intentionally thin and best-effort; it does not expose interruption or
remaining-time details yet.

## Example

```ari
fn main() -> i64 {
  let start = time::now();
  time::sleep(time::milliseconds(0));
  let took = start.elapsed();

  if took.as_nanos() >= 0 {
    return process::success();
  }
  return process::failure();
}
```

Wall-clock timestamp:

```ari
fn main() -> i64 {
  let stamp = time::system_now();
  if stamp.as_unix_nanos() > 0 {
    return 0;
  }
  return 1;
}
```

## Current Limits

- The runtime hook currently targets the Linux/glibc LLVM path through
  `clock_gettime` and `nanosleep`.
- `sleep` does not report interruption or partial sleep.
- Overflow checks for very large duration arithmetic are not designed yet.
- No calendar, timezone, formatting, timer, or async integration exists yet.

## Tests

Focused positive coverage:

```text
tests/cases/standard-library/ok/time/std-time-basic.ari
```

`make check-prelude` emits LLVM for the runtime hooks, checks the
`clock_gettime`/`nanosleep` declarations, and runs the executable with a
deterministic exit-code score. Public declarations are tracked in
`tests/std_api_manifest.txt` and checked by `make check-std-api`.
