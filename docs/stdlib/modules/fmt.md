# std::fmt

`std::fmt` defines the formatting trait names that Ari code can depend on and
the first source formatting helpers for explicit-zone strings and
`std::io::Writer` values. Compiler-assisted formatting macros still exist for
the familiar `print!`, `println!`, and `format_in!` path.
The root prelude names `Display` and `Debug` are public re-exports of
`std::fmt::Display` and `std::fmt::Debug`, so `impl Display for T` and
`impl fmt::Display for T` describe the same trait.

## Public API

```ari
trait Debug

trait Display {
  fn format_in(self: ref Self, zone: ref mut Zone) -> std::string::String;
}

struct FormatSpec

decimal() -> FormatSpec
hex() -> FormatSpec
binary() -> FormatSpec
octal() -> FormatSpec

with_width(spec: FormatSpec, width: i64) -> FormatSpec
with_precision(spec: FormatSpec, precision: i64) -> FormatSpec
left(spec: FormatSpec) -> FormatSpec
right(spec: FormatSpec) -> FormatSpec
center(spec: FormatSpec) -> FormatSpec
uppercase(spec: FormatSpec) -> FormatSpec
alternate(spec: FormatSpec) -> FormatSpec

unsigned_in(zone: ref mut Zone, value: u64, spec: FormatSpec) -> String
integer_in(zone: ref mut Zone, value: i64) -> String
boolean_in(zone: ref mut Zone, value: bool) -> String
float_in(zone: ref mut Zone, value: f64, precision: i64) -> String
text_in(zone: ref mut Zone, value: string) -> String
debug_text_in(zone: ref mut Zone, value: string) -> String

write_unsigned[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: u64, spec: FormatSpec) -> bool
write_integer[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: i64) -> bool
write_boolean[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: bool) -> bool
write_text[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: string) -> bool
write_value[W: io::Writer, T: Display](writer: ref mut W, zone: ref mut Zone, value: T) -> bool
```

`Display::format_in` writes an owned byte string into an explicit target zone.
That keeps allocation visible and matches Ari's current standard-library rule:
owned strings never appear from a hidden global heap.
The standard library implements `Display` for `i64`, `u64`, `bool`, `f32`,
`f64`, lowercase `string`, and `std::string::String`, so
`String.append_value(value)` and custom formatting code can use those common
values without adding type-suffixed append helpers. Float `Display` uses six
fractional digits, matching the default compiler-assisted `{}` float surface.

Treat `FormatSpec` as a value built by helper functions. Start with a base such
as `fmt::hex()` or `fmt::binary()`, then chain natural modifiers:

```ari
var zone = zone::create(128);
let spec = fmt::alternate(fmt::uppercase(fmt::with_width(fmt::hex(), 6)));
let text = fmt::unsigned_in(ref mut zone, 255u64, spec); // "  0XFF"
zone::destroy(zone);
```

Integer precision means "minimum digit count" for `unsigned_in`, so
`fmt::with_precision(fmt::octal(), 3)` renders `7u64` as `007`. Width and
alignment apply after prefix and precision padding. `alternate` adds `0b`,
`0o`, `0x`, or `0X` for binary, octal, and hexadecimal specs.

Use the `write_*` helpers when the destination is a buffer, file-like handle,
or other type implementing `std::io::Writer`. The helper still takes a zone
because Ari keeps temporary formatted text allocation explicit:

```ari
var stdout = io::stdout();
let ok = fmt::write_unsigned<io::Stdout>(
  ref mut stdout,
  ref mut zone,
  31u64,
  fmt::uppercase(fmt::hex()),
);
```

`debug_text_in` is a small seed for debug-style output; it quotes a literal
`string`. The broader `Debug` trait is still a trait marker until custom debug
formatter dispatch is designed.
Use `float_in(ref mut zone, value, precision)` when source code wants an
explicit float precision without going through a format string.

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

The source helpers complement the macros:

- Use `format_in!` for mixed literal templates and type-safe argument counting.
- Use `Display::format_in` for standard display values and user-defined values
  that participate in `{}`.
- Use `write_value` when a `std::io::Writer` should receive any `Display`
  value without choosing a type-suffixed writer helper.
- Use `float_in` when code wants to name float precision directly.
- Use `FormatSpec` plus `unsigned_in` or `write_unsigned` when code needs
  explicit binary, octal, hexadecimal, width, precision, or alignment control
  without adding more compiler lowering.

## Current Limits

- `Debug` is still a trait surface. Full debug formatting, derived debug, and
  custom debug formatter dispatch are future work.
- `Display` is intentionally small first: signed/unsigned 64-bit integers,
  floats, bools, literal `string`, owned `String`, and user-defined impls.
  `char`/`u8` needs a distinct text-vs-number policy before getting a default
  impl.
- `unsigned_in` handles base-specific formatting for `u64`. Negative signed
  integer base formatting should wait for the generic integer policy rather
  than adding type-suffixed one-off helpers.
- Float width/alignment still lives in the compiler-assisted macro path; source
  `float_in` covers explicit precision only.
- Prefer natural formatting names. Type appears in the value signature and
  generic bounds, not as a suffix, unless the compiler/runtime primitive truly
  requires a distinct symbol.
- Custom formatter objects, allocator-returning `format!` without an explicit
  zone, and direct writer streaming without a temporary string remain roadmap
  work.

## Tests

Representative coverage lives in:

```text
tests/cases/standard-library/ok/format/std-fmt-format-spec.ari
tests/cases/standard-library/ok/format/format-print.ari
tests/cases/standard-library/ok/format/format-print-u64.ari
tests/cases/standard-library/ok/prelude/prelude-format-in.ari
tests/cases/standard-library/errors/format/prelude-macro-format-no-default-zone.ari
tests/cases/standard-library/errors/format/prelude-format-in-no-display.ari
```
