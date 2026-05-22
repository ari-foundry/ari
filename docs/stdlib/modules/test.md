# std::test

`std::test` is the small source helper module for executable unit-style tests.
It does not replace the repository test harness yet. Instead, it gives library
and application authors a plain Ari `Report` value that counts checks and turns
the final result into a process status code.

Use it when a test should keep running after one failed check, or when a test
needs a short-lived allocation zone.

## Current APIs

```ari
test::Report

test::report()
test::scratch(capacity)
test::check(ref mut report, condition)
test::equal<T>(ref mut report, left, right)
test::not_equal<T>(ref mut report, left, right)
test::passed(ref report)
test::failed(ref report)
test::ok(ref report)
test::finish(ref report)
test::require(ref report)

report.check(condition)
report.equal<T>(left, right)
report.not_equal<T>(left, right)
report.passed()
report.failed()
report.ok()
report.finish()
report.require()
```

`report()` creates a zeroed `Report`. `check` records one pass or one failure
and returns the condition value. `equal` and `not_equal` are generic wrappers
over `==` and `!=`, so the API stays natural instead of spelling type suffixes
such as `equal_i64`. The type belongs in the values or in an explicit generic
call when inference needs help.

`finish()` returns `0` when no failures were recorded and `1` otherwise.
`require()` calls `std::panic()` if the report contains any failure. Use
`require` near the end of a test when a failed report should abort the current
executable path.

`scratch(capacity)` creates an owned `Zone` for tests. It is intentionally just
a named helper over `zone::create(capacity)`, so tests still destroy the zone
explicitly:

```ari
fn main() -> i64 {
  var report = test::report();
  var zone = test::scratch(128);

  var text = std::string::new(ref mut zone, 8);
  text.push_in(ref mut zone, 'A');
  report.equal(text.len(), 1);
  report.equal(text.first(), 'A');

  let status = report.finish();
  zone::destroy(zone);
  return status;
}
```

## Existing Prelude Diagnostics

The root prelude already provides immediate assertion and panic helpers:

```ari
assert(condition)
debug_assert(condition)
assert_equal(left, right)
assert_not_equal(left, right)
assert_eq_i64(left, right)
assert_ne_i64(left, right)
assert_eq_bool(left, right)
assert_ne_bool(left, right)
panic()
todo()
unreachable()
```

Use `assert_equal` and `assert_not_equal` for normal source code so the
compared type stays in the values instead of the function name. The
type-specific `assert_eq_i64` and `assert_eq_bool` helpers remain as builtin
compatibility hooks and macro targets.
Use these when a single failed check should stop the program immediately.
Use `std::test::Report` when a test should aggregate several checks and return
one final status.

Current `todo()` and `unreachable()` lower to the same panic hook as
`panic()`. Ari does not yet attach source location, stack trace, or backtrace
metadata to those failures.

## Test Runner Integration

Repository tests are still Makefile-driven fixtures under
`tests/cases/standard-library/`. The current pattern is:

1. Write a small executable test.
2. Use `test::report()` for aggregated checks.
3. Return either `report.finish()` or a known score checked by the Makefile.
4. Add LLVM symbol checks when a public helper or generic specialization should
   be visible.

The first coverage file is
`tests/cases/standard-library/ok/test/std-test-report.ari`.

Future test-runner work should add a first-class `ari test` or `@test` path
that discovers test functions, runs each test independently, reports names and
locations, and integrates with the existing repository fixtures.

## Debugging And Logging Roadmap

Current debug output uses the existing IO, formatting, and logging surface:

- `print`, `println`, `eprintln`, `print!`, `println!`, and `eprintln!` for
  simple text and formatted values.
- `std::io::stderr()` with `std::io::write_all` for explicit error output.
- `std::log` for level-prefixed `stderr` diagnostic lines.
- `std::panic()`, `todo()`, and `unreachable()` for stop-the-program
  diagnostics.

`std::log` deliberately stays small today: it provides levels, byte-slice
messages, string messages, and convenience functions. Rich log records should
wait until source location and owned formatting policy are clear enough that
records can include stable file, line, function, and optional target metadata.

Backtrace, stack trace, panic payloads, source location values, benchmark
helpers, and fuzzing hooks are still roadmap items. They need runtime support
for call-frame metadata, stable panic reporting, and test runner ownership
before they become public API guarantees.

## Tests

- `tests/cases/standard-library/ok/test/std-test-report.ari`: `Report`
  construction, aggregated `check`, generic `equal`/`not_equal`, method
  wrappers, scratch zone creation, and explicit finish status.
