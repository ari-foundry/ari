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
struct UtcDateTime

time::monotonic_nanos()
time::unix_nanos()
time::sleep_nanos(nanos)

time::nanoseconds(value)
time::try_nanoseconds(value)
time::microseconds(value)
time::try_microseconds(value)
time::milliseconds(value)
time::try_milliseconds(value)
time::seconds(value)
time::try_seconds(value)
time::now()
time::system_now()
time::system_from_unix(seconds, nanosecond)
time::try_system_from_unix(seconds, nanosecond)
time::is_leap_year(year)
time::days_in_month(year, month)
time::try_days_in_month(year, month)
time::utc_from_unix(seconds, nanosecond)
time::try_utc_from_unix(seconds, nanosecond)
time::elapsed(start)
time::sleep(duration)
time::deadline_at(instant)
time::timeout_after(duration)
time::timeout(duration)

Duration::zero()
Duration::try_nanoseconds(value)
Duration::try_microseconds(value)
Duration::try_milliseconds(value)
Duration::try_seconds(value)
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
SystemTime::from_unix(seconds, nanosecond)
SystemTime::try_from_unix(seconds, nanosecond)
system_time.as_unix_nanos()
system_time.as_unix_seconds()
system_time.subsec_nanos()
system_time.duration_since_unix_epoch()
system_time.to_utc()

Deadline::at(instant)
Deadline::after(duration)
deadline.instant()
deadline.has_expired()
deadline.remaining()
deadline.sleep()

UtcDateTime::from_unix(seconds, nanosecond)
UtcDateTime::try_from_unix(seconds, nanosecond)
utc.year()
utc.month()
utc.day()
utc.hour()
utc.minute()
utc.second()
utc.nanosecond()
utc.is_leap_year()
```

`Duration` values are non-negative. The direct constructor helpers
`nanoseconds`, `microseconds`, `milliseconds`, and `seconds` assert if given a
negative value, which is appropriate for constants and programmer-owned values.
Use the `try_*` forms for user input, config files, protocol fields, or any
branch where invalid input should become `None` instead of aborting the
program. The associated `Duration::try_*` forms are aliases for the same
policy. `as_micros`, `as_millis`, and `as_seconds` truncate toward zero, which
keeps them useful for status checks and coarse timing.

`Instant` is monotonic and should be used for measuring elapsed time. Use
`now()` or `Instant::now()` to read it. `duration_since` asserts when the
earlier instant is actually later; `try_duration_since` returns
`Option[Duration]` for that branch. `saturating_duration_since` returns zero
for backwards comparisons. `add(duration)` creates a future monotonic instant
for deadline calculations.

`SystemTime` reads wall-clock Unix time in nanoseconds. Use it for timestamps,
not elapsed-time measurement. Wall-clock time can move if the host clock is
changed. `system_from_unix(seconds, nanosecond)` and
`SystemTime::from_unix(seconds, nanosecond)` build deterministic wall-clock
values from non-negative Unix timestamps for tests, parsers, and protocol
code. Use `try_system_from_unix` or `SystemTime::try_from_unix` when seconds
or nanoseconds came from user input and invalid ranges should become `None`.

`UtcDateTime` is a plain UTC calendar value using the proleptic Gregorian
calendar and Unix epoch rules. `utc_from_unix(seconds, nanosecond)` converts a
non-negative Unix timestamp into year/month/day/hour/minute/second/nanosecond
fields. `UtcDateTime::from_unix` is the associated constructor, and
`try_utc_from_unix`/`UtcDateTime::try_from_unix` return `Option[UtcDateTime]`
for invalid negative seconds or out-of-range subsecond nanoseconds.
`SystemTime::to_utc()` is the method form for already-validated system times.
`is_leap_year(year)` and `days_in_month(year, month)` expose the same calendar
policy for validation and tests. `days_in_month` is strict and asserts for
invalid month numbers; `try_days_in_month` returns `None` for values outside
1 through 12.

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

Timezone handling is deliberately not in the current source API. UTC calendar
conversion is deterministic and portable; local time, daylight-saving rules,
and named timezone databases usually belong outside a tiny systems-language
standard library, or behind a separate platform-backed package with explicit
data-version policy.

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

Fallible duration construction:

```ari
fn main() -> i64 {
  match time::try_seconds(30) {
    std::Some(timeout) => {
      let deadline = time::timeout(timeout);
      if !deadline.has_expired() {
        return process::success();
      }
    }
    std::None => {}
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

UTC calendar conversion:

```ari
fn main() -> i64 {
  let stamp = time::SystemTime::from_unix(1704067200, 0);
  let utc = stamp.to_utc();

  if utc.year() == 2024 && utc.month() == 1 && utc.day() == 1 {
    return 0;
  }
  return 1;
}
```

Fallible month validation:

```ari
fn main() -> i64 {
  match time::try_days_in_month(2024, 2) {
    std::Some(days) => {
      if days == 29 {
        return process::success();
      }
    }
    std::None => {}
  }
  return process::failure();
}
```

Fallible Unix timestamp conversion:

```ari
fn main() -> i64 {
  match time::SystemTime::try_from_unix(1704067200, 0) {
    std::Some(stamp) => {
      let utc = stamp.to_utc();
      if utc.year() == 2024 {
        return process::success();
      }
    }
    std::None => {}
  }
  return process::failure();
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
- `UtcDateTime` only supports non-negative Unix timestamps for now. Dates
  before 1970 and leap seconds are future policy work.
- The `try_*` duration and Unix timestamp constructors reject negative inputs
  today. Duration scaling and Unix nanosecond assembly still need overflow
  policy for very large values.
- No timezone, formatting, timerfd, or async integration exists yet. Timezone
  databases are intentionally outside the first standard library slice.

## Tests

Focused positive coverage:

```text
tests/cases/standard-library/ok/time/std-time-basic.ari
tests/cases/standard-library/ok/time/std-time-try-duration.ari
tests/cases/standard-library/ok/time/std-time-try-unix.ari
tests/cases/standard-library/ok/time/std-time-calendar-validation.ari
tests/cases/standard-library/ok/time/std-time-timeout.ari
tests/cases/standard-library/ok/time/std-time-utc-calendar.ari
```

`make check-prelude` emits LLVM for the runtime hooks, checks the
`clock_gettime`/`nanosleep` declarations, and runs the executable with a
deterministic exit-code score. It also checks timeout/deadline and UTC
calendar helper symbols. `std-time-try-duration.ari` covers fallible
constructor behavior for both module functions and `Duration::try_*`
associated helpers. `std-time-try-unix.ari` covers fallible Unix timestamp
construction for `SystemTime` and `UtcDateTime`.
`std-time-calendar-validation.ari` covers strict and fallible month-length
validation. Public declarations are tracked in `tests/std_api_manifest.txt`
and checked by `make check-std-api`.
