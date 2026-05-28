# std::log

`std::log` is Ari's first source-only logging helper module. It writes
level-prefixed diagnostic lines to `stderr` through `std::io::Stderr`, so
small tools and tests can report progress without inventing one-off debug
printing functions.

The module is intentionally narrow. It does not own global filters, structured
records, stack traces, or backtraces yet.

## Current APIs

```ari
log::Level
log::Trace
log::Debug
log::Info
log::Warn
log::Error

log::rank(level)
log::name(ref mut zone, level) -> String
log::name_text(level) -> string
log::enabled(level, minimum)
log::write(level, bytes)
log::message(level, text)
log::trace(text)
log::debug(text)
log::info(text)
log::warn(text)
log::error(text)
```

`rank(level)` gives the severity order. `name(ref mut zone, level)` copies the
lowercase label into the caller's zone as a `String`; `name_text(level)` is the
raw borrowed compatibility label used by the logging writer itself.

| Level | Rank |
| --- | --- |
| `Trace` | `0` |
| `Debug` | `1` |
| `Info` | `2` |
| `Warn` | `3` |
| `Error` | `4` |

`enabled(level, minimum)` is true when `level` is at least as severe as
`minimum`. There is no global logging threshold today; callers keep filtering
explicit:

```ari
if log::enabled(log::Debug, minimum) {
  log::debug("loaded config");
}
```

`write(level, bytes)` writes a borrowed `Slice[u8]` as one line. `message`
writes a null-terminated Ari `string` as one line. The convenience functions
`trace`, `debug`, `info`, `warn`, and `error` call `message` with the matching
level.

Every line currently uses this shape:

```text
[level] message
```

For example:

```ari
log::info("ready");
log::warn("using fallback");
```

emits:

```text
[info] ready
[warn] using fallback
```

## Relationship To IO And Test Support

Use `std::io` when you need custom writers, buffering, exact byte reads, or
stdout/stderr routing. Use `std::log` when a diagnostic line with a severity
label is enough.

Use `std::test::Report` to count pass/fail checks. Use `std::log` inside tests
when a human-readable progress or failure breadcrumb should go to `stderr`.

## Roadmap

Future logging and diagnostics work should grow in this order:

1. Optional log records with target/module fields and thread ids.
2. Panic/assert messages that can reuse the same formatting path.
3. Stack trace and backtrace support after runtime frame metadata and
   symbolization policy are documented.
4. Test-runner integration so `@test` or `ari test` can capture logs per test.

Source-code rendering belongs in a compiler/tooling package rather than this
runtime logging helper.

## Tests

- `tests/cases/standard-library/ok/log/std-log-basic.ari`: level ordering,
  threshold checks, byte-slice logging, string logging, convenience functions,
  and stderr output format.
