# std::fmt

`std::fmt` defines the formatting trait names that Ari code can depend on
while formatting macro lowering remains compiler-assisted.

## Public API

```ari
trait Debug

trait Display {
  fn format_in(self: ref Self, zone: ref mut Zone) -> std::string::String;
}
```

`Display::format_in` writes an owned byte string into an explicit target zone.
That keeps allocation visible and matches Ari's current standard-library rule:
owned strings never appear from a hidden global heap.

## Formatting Macros

The executable formatting surface today is still macro-based:

```ari
print!("value={}", value)
println!("value={}", value)
format_in!(ref mut zone, "value={}", value)
```

Format strings must be literals. The macros currently support strings,
integers, bools, `f32`, `f64`, and values accepted through the compiler's
current `Display` path. `format!` without an explicit zone is intentionally
not executable yet; use `format_in!` so the allocation zone is clear.

## Current Limits

- `Debug` is a trait surface only; derived or blanket debug formatting is
  future work.
- More formatting behavior should move into source Ari once trait dispatch and
  string allocation rules are strong enough.
- Prefer natural formatting names. Do not add type-suffixed formatting
  helpers unless the compiler/runtime primitive truly requires a distinct
  symbol.

## Tests

Representative coverage lives in:

```text
tests/cases/standard-library/ok/format/format-print.ari
tests/cases/standard-library/ok/format/format-print-u64.ari
tests/cases/standard-library/ok/prelude/prelude-format-in.ari
tests/cases/standard-library/errors/format/prelude-macro-format-no-default-zone.ari
tests/cases/standard-library/errors/format/prelude-format-in-no-display.ari
```
