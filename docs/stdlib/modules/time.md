# std::time

`std::time` provides the first small, runtime-backed time surface for Ari. It
keeps raw OS time reads behind `extern "ari"` hooks, then exposes source value
wrappers so ordinary code can talk about durations, monotonic instants, and
wall-clock time without passing naked integer timestamps everywhere.

## Current API

```ari
struct Duration
struct Instant
struct SystemTime
struct Deadline

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
time::deadline_at(instant)
time::timeout_after(duration)
time::timeout(duration)

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
instant.saturating_duration_since(earlier)
instant.try_duration_since(earlier)
instant.elapsed()
instant.add(duration)

SystemTime::now()
system_time.as_unix_nanos()
system_time.duration_since_unix_epoch()

Deadline::at(instant)
Deadline::after(duration)
deadline.instant()
deadline.has_expired()
deadline.remaining()
deadline.sleep()
```

`Duration` values are non-negative. The constructor helpers assert if given a
negative value. `as_micros`, `as_millis`, and `as_seconds` truncate toward
zero, which keeps them useful for status checks and coarse timing.

`Instant` is monotonic and should be used for measuring elapsed time. Use
`now()` or `Instant::now()` to read it. `duration_since` asserts when the
earlier instant is actually later; `try_duration_since` returns
`Option[Duration]` for that branch. `saturating_duration_since` returns zero
for backwards comparisons. `add(duration)` creates a future monotonic instant
for deadline calculations.

`SystemTime` reads wall-clock Unix time in nanoseconds. Use it for timestamps,
not elapsed-time measurement. Wall-clock time can move if the host clock is
changed.

`Deadline` is the current timeout value. `timeout(duration)` and
`timeout_after(duration)` create a deadline relative to `Instant::now()`;
`deadline_at(instant)` creates one from an absolute monotonic instant.
`has_expired()` checks whether `now()` reached the deadline, `remaining()`
returns a saturating duration, and `sleep()` sleeps only for the remaining
duration. Use `Deadline` in APIs that need timeout policy without yet owning
async cancellation or OS descriptor integration.

`sleep(duration)` sleeps the current thread for the duration. `std::thread`
also re-exports this behavior as `thread::sleep(duration)` for thread-oriented
code. The first runtime hook is intentionally thin and best-effort; it does not
expose interruption or remaining-time details yet.

Calendar conversion and timezone handling are deliberately not in the current
source API. Future calendar helpers should start with UTC-only conversion from
`SystemTime` to a plain date/time value. Full timezone databases usually belong
outside a tiny systems-language standard library, or behind a separate,
platform-backed package with explicit data-version policy.

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

Timeout/deadline check:

```ari
fn main() -> i64 {
  let deadline = time::timeout(time::milliseconds(10));

  if deadline.has_expired() {
    return process::failure();
  }

  let remaining = deadline.remaining();
  if remaining.as_nanos() >= 0 {
    return process::success();
  }
  return process::failure();
}
```

## Current Limits

- The runtime hook currently targets the Linux/glibc LLVM path through
  `clock_gettime` and `nanosleep`.
- `sleep` does not report interruption or partial sleep.
- `Deadline` is monotonic and process-local. It does not cancel work by
  itself; IO/process/thread APIs must opt into accepting it.
- Overflow checks for very large duration arithmetic are not designed yet.
- No calendar, timezone, formatting, timerfd, or async integration exists yet.
  Calendar conversion is roadmap work; timezone databases are intentionally
  outside the first standard library slice.

## Tests

Focused positive coverage:

```text
tests/cases/standard-library/ok/time/std-time-basic.ari
tests/cases/standard-library/ok/time/std-time-timeout.ari
```

`make check-prelude` emits LLVM for the runtime hooks, checks the
`clock_gettime`/`nanosleep` declarations, and runs the executable with a
deterministic exit-code score. It also checks timeout/deadline helper symbols.
Public declarations are tracked in `tests/std_api_manifest.txt` and checked by
`make check-std-api`.
