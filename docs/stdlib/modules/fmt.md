# std::fmt

`std::fmt` defines the formatting trait names that Ari code can depend on and
the first source formatting helpers for explicit-zone strings,
`std::io::Writer` values, and direct stdout output. Compiler-assisted
formatting macros still exist for the familiar `print!`, `println!`,
`eprintln!`, and `format_in!` path.
The root prelude names `Display` and `Debug` are public re-exports of
`std::fmt::Display` and `std::fmt::Debug`, so `impl Display for T` and
`impl fmt::Display for T` describe the same trait.

## Public API

```ari
trait Debug {
  fn debug_in(self: ref Self, zone: ref mut Zone) -> std::string::String;
}

trait Display {
  fn format_in(self: ref Self, zone: ref mut Zone) -> std::string::String;
}

struct FormatSpec

decimal() -> FormatSpec
hex() -> FormatSpec
binary() -> FormatSpec
octal() -> FormatSpec

with_width(spec: FormatSpec, width: i64) -> FormatSpec
try_with_width(spec: FormatSpec, width: i64) -> Option[FormatSpec]
with_precision(spec: FormatSpec, precision: i64) -> FormatSpec
try_with_precision(spec: FormatSpec, precision: i64) -> Option[FormatSpec]
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
char_in(zone: ref mut Zone, value: char) -> String
debug_text_in(zone: ref mut Zone, value: string) -> String
debug_char_in(zone: ref mut Zone, value: char) -> String
format_value[T: Display](zone: ref mut Zone, value: T) -> String
debug_value[T: Debug](zone: ref mut Zone, value: T) -> String
format[T: Display](zone: ref mut Zone, template: string, value: T) -> Result[String, Error]
format2[A: Display, B: Display](zone: ref mut Zone, template: string, first: A, second: B) -> Result[String, Error]
format3[A: Display, B: Display, C: Display](zone: ref mut Zone, template: string, first: A, second: B, third: C) -> Result[String, Error]
format4[A: Display, B: Display, C: Display, D: Display](zone: ref mut Zone, template: string, first: A, second: B, third: C, fourth: D) -> Result[String, Error]
concat2[A: Display, B: Display](zone: ref mut Zone, first: A, second: B) -> String
concat3[A: Display, B: Display, C: Display](zone: ref mut Zone, first: A, second: B, third: C) -> String
concat4[A: Display, B: Display, C: Display, D: Display](zone: ref mut Zone, first: A, second: B, third: C, fourth: D) -> String
write_concat2[W: io::Writer, A: Display, B: Display](writer: ref mut W, zone: ref mut Zone, first: A, second: B) -> Result[(), Error]
write_concat2_bool[W: io::Writer, A: Display, B: Display](writer: ref mut W, zone: ref mut Zone, first: A, second: B) -> bool
write_concat3[W: io::Writer, A: Display, B: Display, C: Display](writer: ref mut W, zone: ref mut Zone, first: A, second: B, third: C) -> Result[(), Error]
write_concat3_bool[W: io::Writer, A: Display, B: Display, C: Display](writer: ref mut W, zone: ref mut Zone, first: A, second: B, third: C) -> bool
write_concat4[W: io::Writer, A: Display, B: Display, C: Display, D: Display](writer: ref mut W, zone: ref mut Zone, first: A, second: B, third: C, fourth: D) -> Result[(), Error]
write_concat4_bool[W: io::Writer, A: Display, B: Display, C: Display, D: Display](writer: ref mut W, zone: ref mut Zone, first: A, second: B, third: C, fourth: D) -> bool
write_format[W: io::Writer, T: Display](writer: ref mut W, zone: ref mut Zone, template: string, value: T) -> Result[(), Error]
write_format2[W: io::Writer, A: Display, B: Display](writer: ref mut W, zone: ref mut Zone, template: string, first: A, second: B) -> Result[(), Error]
write_format3[W: io::Writer, A: Display, B: Display, C: Display](writer: ref mut W, zone: ref mut Zone, template: string, first: A, second: B, third: C) -> Result[(), Error]
write_format4[W: io::Writer, A: Display, B: Display, C: Display, D: Display](writer: ref mut W, zone: ref mut Zone, template: string, first: A, second: B, third: C, fourth: D) -> Result[(), Error]
write_format_stream[W: io::Writer, T: Display](writer: ref mut W, zone: ref mut Zone, template: string, value: T) -> Result[(), Error]
write_format_stream2[W: io::Writer, A: Display, B: Display](writer: ref mut W, zone: ref mut Zone, template: string, first: A, second: B) -> Result[(), Error]
write_format_stream3[W: io::Writer, A: Display, B: Display, C: Display](writer: ref mut W, zone: ref mut Zone, template: string, first: A, second: B, third: C) -> Result[(), Error]
write_format_stream4[W: io::Writer, A: Display, B: Display, C: Display, D: Display](writer: ref mut W, zone: ref mut Zone, template: string, first: A, second: B, third: C, fourth: D) -> Result[(), Error]

write_unsigned[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: u64, spec: FormatSpec) -> Result[(), Error]
write_unsigned_bool[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: u64, spec: FormatSpec) -> bool
write_integer[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: i64) -> Result[(), Error]
write_integer_bool[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: i64) -> bool
write_boolean[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: bool) -> Result[(), Error]
write_boolean_bool[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: bool) -> bool
write_text[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: string) -> Result[(), Error]
write_text_bool[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: string) -> bool
write_unsigned_stream[W: io::Writer](writer: ref mut W, value: u64, spec: FormatSpec) -> Result[(), Error]
write_integer_stream[W: io::Writer](writer: ref mut W, value: i64) -> Result[(), Error]
write_boolean_stream[W: io::Writer](writer: ref mut W, value: bool) -> Result[(), Error]
write_text_stream[W: io::Writer](writer: ref mut W, value: string) -> Result[(), Error]
write_char_stream[W: io::Writer](writer: ref mut W, value: char) -> Result[(), Error]
write_value[W: io::Writer, T: Display](writer: ref mut W, zone: ref mut Zone, value: T) -> Result[(), Error]
write_value_bool[W: io::Writer, T: Display](writer: ref mut W, zone: ref mut Zone, value: T) -> bool
write_debug[W: io::Writer, T: Debug](writer: ref mut W, zone: ref mut Zone, value: T) -> Result[(), Error]
write_debug_bool[W: io::Writer, T: Debug](writer: ref mut W, zone: ref mut Zone, value: T) -> bool
write_line_text[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: string) -> Result[(), Error]
write_line_text_bool[W: io::Writer](writer: ref mut W, zone: ref mut Zone, value: string) -> bool
write_line_value[W: io::Writer, T: Display](writer: ref mut W, zone: ref mut Zone, value: T) -> Result[(), Error]
write_line_value_bool[W: io::Writer, T: Display](writer: ref mut W, zone: ref mut Zone, value: T) -> bool
write_line_debug[W: io::Writer, T: Debug](writer: ref mut W, zone: ref mut Zone, value: T) -> Result[(), Error]
write_line_debug_bool[W: io::Writer, T: Debug](writer: ref mut W, zone: ref mut Zone, value: T) -> bool

print_value[T: Display](zone: ref mut Zone, value: T) -> i64
println_value[T: Display](zone: ref mut Zone, value: T) -> i64
print_debug[T: Debug](zone: ref mut Zone, value: T) -> i64
println_debug[T: Debug](zone: ref mut Zone, value: T) -> i64
```

`Display::format_in` writes an owned byte string into an explicit target zone.
That keeps allocation visible and matches Ari's current standard-library rule:
owned strings never appear from a hidden global heap.
The standard library implements `Display` for `char`, `i64`, `u64`, `bool`,
`f32`, `f64`, lowercase `string`, and `std::string::String`, so
`String.append_value(value)` and custom formatting code can use those common
values without adding type-suffixed append helpers. Float `Display` uses six
fractional digits, matching the default compiler-assisted `{}` float surface.
`char` displays as the single byte character, matching `print` and
`format_in!` byte-character formatting.
`Debug::debug_in` follows the same explicit-zone allocation rule, but describes
diagnostic output instead of ordinary display text. The standard library
implements `Debug` for the same initial scalar/text set. Literal `string` and
owned `String` debug output are quoted, and `char` debug output uses
single-quoted byte-character syntax such as `'A'` or `'\n'`.

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
`with_width` and `with_precision` are strict builders: they assert that the
chosen number is non-negative. Use `try_with_width` and `try_with_precision`
when the number came from configuration, command-line parsing, or another
runtime input path:

```ari
match fmt::try_with_width(fmt::decimal(), parsed_width) {
  std::Some(spec) => {
    let text = fmt::unsigned_in(ref mut zone, value, spec);
  }
  std::None => {
    // Reject or report the invalid width.
  }
}
```

Use the `write_*` helpers when the destination is a buffer, file-like handle,
or other type implementing `std::io::Writer`. The helpers return
`Result[(), Error]`, preserving the writer failure from `io::write_all`; use the
matching `_bool` wrappers only for compatibility call sites that intentionally
discard the reason. Each helper still takes a zone because Ari keeps temporary
formatted text allocation explicit. The `write_line_*` helpers are the same
operations with one trailing newline written through the same writer; they
return the first failure from either the formatted value or the newline write:

```ari
var stdout = io::stdout();
fmt::write_unsigned<io::Stdout>(
  ref mut stdout,
  ref mut zone,
  31u64,
  fmt::uppercase(fmt::hex()),
).unwrap();

fmt::write_line_value<io::Stdout, string>(
  ref mut stdout,
  ref mut zone,
  "Created package hello",
).unwrap();
```

Use the `*_stream` scalar helpers when the caller already knows the value kind
and wants to write directly to a `std::io::Writer` without first building an
owned `String`. `write_integer_stream` handles the full signed `i64` range by
streaming magnitude digits left-to-right, and `write_unsigned_stream` supports
the same base, width, precision, alignment, uppercase, and alternate-prefix
`FormatSpec` policy as `unsigned_in`:

```ari
var stdout = io::stdout();
fmt::write_integer_stream<io::Stdout>(ref mut stdout, -42).unwrap();
fmt::write_text_stream<io::Stdout>(ref mut stdout, " files").unwrap();
```

Use `print_value` and `println_value` for direct stdout output when a macro is
not the right fit and the value already implements `Display`:

```ari
var zone = zone::create(64);
fmt::print_value(ref mut zone, "score=");
fmt::println_value(ref mut zone, 42);
zone::destroy(zone);
```

Use `debug_value`, `write_debug`, `print_debug`, and `println_debug` when a
type's `Debug` impl should drive the output:

```ari
var zone = zone::create(64);
let quoted = fmt::debug_value(ref mut zone, "ari"); // "\"ari\""
fmt::println_debug(ref mut zone, quoted);
zone::destroy(zone);
```

`debug_text_in` remains a convenience for quoting a literal `string` directly.
Use `float_in(ref mut zone, value, precision)` when source code wants an
explicit float precision without going through a format string.

Use `format_value` when a caller wants the ordinary `Display` string for one
value without naming the trait method at the call site. Use `concat2`,
`concat3`, and `concat4` for short hosted-program messages assembled from
`Display` values without invoking a compiler format macro:

```ari
var zone = zone::create(128);
let name = std::string::from(ref mut zone, "hello");
let line = fmt::concat2(ref mut zone, "Compiling ", name);
let output = fmt::concat3(ref mut zone, "target/debug/", 7, ".ari");
let command = fmt::concat4(ref mut zone, "arix ", "build ", "--jobs=", 4);
zone::destroy(zone);
```

These helpers still allocate through the explicit zone and use `Display`, so
they follow the same text policy as `format_in!`, `print_value`, and
`String.append_value`.

Use `format`, `format2`, `format3`, and `format4` when the template is a
runtime string and the caller still wants checked placeholder counting. The
template language is intentionally small and byte-oriented:

- `{}` consumes the next `Display` value.
- `{{` writes a literal `{`.
- `}}` writes a literal `}`.
- unmatched braces, a placeholder count mismatch, or any placeholder other than
  `{}` returns `Err(std::error::InvalidInput)`.

The functions allocate the resulting `String` in the supplied zone and return
`Result[String, Error]` because template mistakes are recoverable user-input or
configuration errors:

```ari
var zone = zone::create(128);
match fmt::format2(ref mut zone, "{}={}", "name", 7) {
  std::Ok(text) => {
    // text == "name=7"
  }
  std::Err(err) => {
    // Invalid template or wrong number of placeholders.
  }
}
zone::destroy(zone);
```

Use `write_concat2`, `write_concat3`, and `write_concat4` when the destination
is already an `io::Writer` and the caller wants to stream a short status
message without constructing one combined `String` first:

```ari
var stdout = io::stdout();
fmt::write_concat3<io::Stdout, string, String, string>(
  ref mut stdout,
  ref mut zone,
  "Compiling ",
  name,
  "\n",
).unwrap();
```

These helpers write each `Display` value in order and return the first writer
error. The `_bool` variants are compatibility wrappers for call sites that
intentionally discard the failure reason.
Use `write_format`, `write_format2`, `write_format3`, and `write_format4` for
the same runtime template rules when the final destination is an `io::Writer`. These helpers
parse the template and write literal bytes and placeholder values directly to
the writer instead of building one combined output `String` first. Each
placeholder still formats through `Display::format_in`, so per-value temporary
allocation remains explicit through the supplied zone. Template mistakes return
`Err(Error(InvalidInput))`; writer failures return the writer error, with the
current byte-oriented fallback using `BrokenPipe`. Because these are streaming
operations, bytes written before a later template or writer error are not rolled
back:

```ari
var stdout = io::stdout();
fmt::write_format2<io::Stdout, string, i64>(
  ref mut stdout,
  ref mut zone,
  "Compiling {} ({})",
  "hello",
  1,
).unwrap();
```

`write_format_stream`, `write_format_stream2`, `write_format_stream3`, and
`write_format_stream4` are spelling-explicit aliases for the writer-backed
runtime-template path. They distinguish writer streaming from owned-string
`format*` helpers while the compiler does not yet support a generic
writer-facing `Display` trait for per-value zero-temporary formatting.

## Formatting Macros

The executable formatting surface today is still macro-based:

```ari
print!("value={}", value)
println!("value={}", value)
eprintln!("value={}", value)
println!("value={value}")
println!("point={point.x} pair={pair.0}")
println!("debug={:?}", value)
println!("debug={value:?}")
format_in!(ref mut zone, "value={}", value)
format_in!(ref mut zone, "value={value}")
format_in!(ref mut zone, "debug={:?}", value)
```

Format strings must be literals. The macros currently support strings,
integers, bools, `f32`, `f64`, and values accepted through the compiler's
current `Display` path for `{}`. Use `{name}`, `{name.field}`, or `{name.0}`
to capture a local binding, named field, or tuple field without passing it
again as a separate argument; `{name:?}` and `{name:.N}` are the named forms of
`{:?}` and `{:.N}`. Named captures still start from a local binding, so use
ordinary `{}` arguments for module paths, indexing, method calls, and computed
expressions. `eprintln!` follows the same placeholder rules as `println!` but
writes to stderr. `{:?}` is the debug placeholder: `format_in!` dispatches it
through `Debug::debug_in`, while direct stdout/stderr `print!`, `println!`,
and `eprintln!` support it for built-in printable values and lowercase
`string` without requiring a temporary zone at the call site. For custom debug
output to stdout, use `fmt::print_debug` or `fmt::println_debug` with an
explicit zone.
`format!` without an explicit zone is intentionally not executable yet; use
`format_in!` so the allocation zone is clear.

The source helpers complement the macros:

- Use `format_in!` for mixed literal templates and type-safe argument counting.
- Use `format_value` for one `Display` value when a named source function is
  clearer than calling `value.format_in(zone)` directly.
- Use `format`, `format2`, `format3`, and `format4` for runtime templates with
  one through four `{}` placeholders and recoverable invalid-template errors.
- Use `concat2`, `concat3`, and `concat4` for small CLI/status strings such as
  `"Compiling " + name` while Ari does not have general string interpolation.
- Use `write_concat2`, `write_concat3`, and `write_concat4` for the same short
  CLI/status message shape when the destination is already an `io::Writer`.
- Use `write_format`, `write_format2`, `write_format3`, and `write_format4`
  when the writer path needs the runtime-template rules and must preserve
  writer/template failures.
- Use `write_integer_stream`, `write_unsigned_stream`,
  `write_boolean_stream`, `write_text_stream`, and `write_char_stream` when a
  known scalar/text value should go directly to an `io::Writer`.
- Use `Display::format_in` for standard display values and user-defined values
  that participate in `{}`.
- Use `Debug::debug_in`, `{:?}`, and `debug_value` for diagnostic output that
  should be quoted or otherwise distinguished from user-facing display text.
- Use `String.append_debug(value)` when building an owned byte string in a
  tracked local zone and the value already implements `Debug`.
- Use `write_value` when a `std::io::Writer` should receive any `Display`
  value without choosing a type-suffixed writer helper and write failures should
  stay recoverable.
- Use `write_debug` when a `std::io::Writer` should receive any `Debug` value.
- Use `write_line_text`, `write_line_value`, and `write_line_debug` when a
  writer should receive exactly one formatted line and failures must remain
  recoverable.
- Use `print_value` and `println_value` when stdout should receive any
  `Display` value without choosing a type-suffixed IO hook.
- Use `print_debug` and `println_debug` for direct stdout debug output.
- Use `float_in` when code wants to name float precision directly.
- Use `FormatSpec` plus `unsigned_in` or `write_unsigned` when code needs
  explicit binary, octal, hexadecimal, width, precision, or alignment control
  without adding more compiler lowering.
- Use `try_with_width` and `try_with_precision` before formatting values from
  untrusted input; use strict `with_*` builders once the value is already
  validated.

## Current Limits

- `Debug` is real source dispatch now, including `{:?}` for `format_in!`.
  Custom formatter objects are still future work.
- `Display` is intentionally small first: byte characters, signed/unsigned
  64-bit integers, floats, bools, literal `string`, owned `String`, and
  user-defined impls. Because `char` is currently a `u8` alias, byte-oriented
  `u8` values also follow the byte-character display policy in text-shaped
  formatting paths.
- `unsigned_in` handles base-specific formatting for `u64`. Negative signed
  integer base formatting should wait for the generic integer policy rather
  than adding type-suffixed one-off helpers.
- Float width/alignment still lives in the compiler-assisted macro path; source
  `float_in` covers explicit precision only.
- Prefer natural formatting names. Type appears in the value signature and
  generic bounds, not as a suffix, unless the compiler/runtime primitive truly
  requires a distinct symbol.
- Custom formatter objects, allocator-returning `format!` without an explicit
  zone, true variadic/default-zone formatting, and generic per-value streaming
  display dispatch beyond the current fixed-arity one-through-four writer
  helpers remain roadmap work.

## Tests

Representative coverage lives in:

```text
tests/cases/standard-library/ok/format/std-fmt-format-spec.ari
tests/cases/standard-library/ok/format/std-fmt-concat-format-value.ari
tests/cases/standard-library/ok/format/std-fmt-format-validation.ari
tests/cases/standard-library/ok/format/std-fmt-debug-values.ari
tests/cases/standard-library/ok/format/std-fmt-char-values.ari
tests/cases/standard-library/ok/format/std-fmt-print-value.ari
tests/cases/standard-library/ok/format/format-eprintln.ari
tests/cases/standard-library/ok/format/format-print.ari
tests/cases/standard-library/ok/format/format-print-u64.ari
tests/cases/standard-library/ok/prelude/prelude-format-in.ari
tests/cases/standard-library/ok/prelude/prelude-format-in-debug.ari
tests/cases/standard-library/errors/format/prelude-macro-format-no-default-zone.ari
tests/cases/standard-library/errors/format/prelude-format-in-no-display.ari
tests/cases/standard-library/errors/format/prelude-format-in-no-debug.ari
```
