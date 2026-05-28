# std::string

`std::string` contains Ari's current owned byte-string handle. It exists so
programs can copy borrowed `string` literals, formatted output, or byte slices
into an explicit allocation `Zone` without introducing a hidden global heap.

Today's `String` is intentionally a byte string. It is useful for compiler
tests, CLI-style output, simple parser buffers, and ASCII-oriented text work.
It now has UTF-8 validation and scalar helpers, but it is not a Unicode
normalization, grapheme, or locale-aware text abstraction yet.

For public stdlib APIs, `String` is the normal owned text shape. Lowercase
`string` is reserved for borrowed literals, static compiler/runtime strings,
C/OS boundaries, and compatibility helpers that say so explicitly, such as
`_text`, `_raw`, or `_unchecked`. Because Ari does not have a hidden global
heap, helpers that return owned text take `ref mut Zone` and copy into that
zone.

## When To Use It

Use `std::string::String` when bytes must outlive a borrowed literal or input
buffer and you can name the `Zone` that owns the storage. The same `String`
handle is also the standard appendable buffer for diagnostics, command output,
and small manifest/parser strings. Use `Slice[u8]` when you only need a
borrowed view. Use `std::ascii` or the `String` ASCII helpers for byte
classification, ASCII-only comparison/search, trimming, and integer parsing.

For parser-style code that works mostly with borrowed bytes, use the module
view helpers. They accept `Slice[u8]`, so string literals can be passed
directly and owned `String` values can pass `text.as_slice()`:

```ari
std::string::lines(bytes)
std::string::trim(bytes)
std::string::split_once(bytes, "=")
std::string::starts_with(bytes, "[")
std::string::ends_with(bytes, "]")
std::string::strip_prefix(bytes, "\"")
std::string::strip_suffix(bytes, "\"")
```

Use the typed view helpers when the byte source has a more specific meaning:

```ari
std::string::utf8(bytes)
std::string::utf8_string(ref mut zone, bytes)
std::string::utf8_string_optional(ref mut zone, bytes)
std::string::utf8_string_unchecked(ref mut zone, bytes)
std::string::codepoints(bytes)
std::string::os_str(bytes)
std::string::os_string(ref mut zone, bytes)
std::string::os_string_from_text(ref mut zone, "literal")
std::string::c_str("literal")
std::string::c_len("literal")
std::string::c_bytes("literal")
std::string::bytes("literal")
```

`utf8(bytes)` validates a borrowed `Slice[u8]` and returns
`Option[std::string::Utf8]`. `codepoints(bytes)` performs the same validation
and returns `Option[std::string::Codepoints]`, a lazy scalar iterator over the
borrowed bytes. `utf8_string(ref mut zone, bytes)` is the owned counterpart:
it validates the bytes, copies them into the target zone, and returns
`Result[Utf8String, std::encoding::Utf8Error]`. Use
`std::encoding::validate_utf8` when invalid input needs a detailed error
without allocating. `os_str(bytes)` keeps operating-system bytes distinct from
normal text; the current POSIX slice stores raw bytes and may not be valid
UTF-8. `os_string(ref mut zone, bytes)` is the owned raw OS-byte counterpart;
`os_string_from_text` copies an Ari `string` literal or value into that shape.
`c_str(text)` is a convenience wrapper for
`std::c::from_string(text)` and returns the shared `std::c::CStr` type, while
`c_len` and `c_bytes` expose bytes before the trailing NUL. `bytes(text)` is
the named helper for code that wants to make the boundary explicit. In normal
calls and local initializers, a string literal can also flow directly into
borrowed or local byte-storage expectations:

```ari
let bytes: Slice[u8] = "true";
var buffer: Vec[u8] = "ari";
let fixed: [u8, 3] = "lib";
ascii::parse_decimal("123");
```

String literals also act as borrowed `Slice[u8]` receivers for read-only slice
helpers. This keeps everyday byte checks close to other modern standard
libraries without forcing `std::string::bytes(...)` noise:

```ari
"hello".len()
"hello".starts_with("he")
"hello".find("ll")
"hello".slice(1, 4).equals("ell")
```

The view length stops before the first embedded NUL byte, matching the existing
literal-to-`Slice[u8]` boundary. Use an owned `String`, `Vec[u8]`, or explicit
byte storage when embedded NUL bytes are data rather than a C-string boundary.

Avoid using `String` as a general text policy. UTF-8 helpers operate on Unicode
scalar values and byte offsets. Unicode normalization, grapheme iteration,
encoding conversion, and locale-sensitive case conversion are future library
work.

## Constructors And Copies

Constructors allocate in an explicit zone:

```ari
std::string::new(ref mut zone, capacity)
std::string::empty(ref mut zone)
std::string::from(ref mut zone, "text")
std::string::from_string(ref mut zone, "text")
std::string::copy(ref mut zone, bytes)
std::string::from_slice_in(ref mut zone, bytes)
std::string::join_in(ref mut zone, parts, separator)
std::string::replace(ref mut zone, bytes, needle, replacement)
```

`new` creates an empty buffer with fixed starting capacity.
`empty` is the zero-capacity spelling for a string you plan to grow.
`from` is the natural constructor for Ari `string` values and forwards to
`from_string`. `copy` is the natural constructor for borrowed byte slices and
forwards to `from_slice_in`. The older names stay documented because they make
the backing source explicit in low-level library code.
`join_in` joins a `Slice[Slice[u8]]` with a byte separator and returns an owned
`String` in the provided zone.
`replace` copies `bytes` into the provided zone while replacing non-overlapping
matches of `needle` with `replacement`. An empty `needle` is treated as a no-op
copy so byte-oriented parser code does not accidentally allocate unbounded
separator output.

Copies also require a target zone:

```ari
std::string::copy_to(ref text, ref mut zone)
text.copy_to(ref mut zone)
text.trim_start_to(ref mut zone)
text.trim_end_to(ref mut zone)
text.trim_to(ref mut zone)
```

The returned handle is tied to the target zone. Ari's checker rejects use of a
tracked string, its raw pointer, or derived views after that zone is reset or
destroyed.

The `trim_*_to` and `trim_to` methods first make the same ASCII-trimmed view as
`trim_start`, `trim_end`, or `trim`, then copy that view into the target zone.
Use them when the trimmed bytes must outlive the source zone.

## Metadata, Bytes, And Views

Basic metadata and checked byte access:

```ari
text.len()
text.capacity()
text.is_empty()
text.first()
text.try_first()
text.last()
text.try_last()
text.get(index)
text.try_get(index)
text.set(index, byte)
text.replace(index, byte)
```

`first`, `last`, `get`, `set`, and `replace` assert their bounds at runtime.
Use `try_first`, `try_last`, and `try_get` when missing bytes are normal input;
they return `Option[u8]`. `replace` returns the previous byte.

Borrowed views and raw pointers:

```ari
text.bytes()
text.as_slice()
text.as_ptr()
```

`bytes` is the natural read-only borrowed view. `as_slice` is the generic
sequence spelling for the same `Slice[u8]` view. `as_ptr` returns the backing
byte pointer and preserves zone provenance in the checker.

## Mutation And Growth

Fixed-capacity mutation:

```ari
text.set(index, char)
text.replace(index, char)
text.push(char)
text.pop()
text.try_pop()
text.remove(index)
text.try_remove(index)
text.insert(index, char)
text.clear()
text.truncate(length)
text.retain(keep)
```

`try_pop` returns `None<u8>()` when the string is empty. `try_remove` returns
`None<u8>()` when the byte index is absent. `pop` and `remove` assert in those
cases and are best when absence means a programmer error. `retain` keeps bytes
accepted by `keep: fn(ref u8) -> bool` and preserves their order. These are
byte operations; UTF-8-aware deletion or filtering should be built on explicit
scalar boundary checks.

Growth-capable mutation uses the owning zone. The explicit forms are the
source-level contract:

```ari
text.append(ref mut zone, "text")
text.append_byte(ref mut zone, char)
text.append_bytes(ref mut zone, bytes)
text.push_str(ref mut zone, bytes)
text.push_in(ref mut zone, char)
text.insert_in(ref mut zone, index, char)
text.reserve(ref mut zone, capacity)
text.reserve_extra(ref mut zone, additional)
text.extend_from_slice_in(ref mut zone, bytes)
text.resize_in(ref mut zone, length, char)
```

Use `append` for Ari `string` values, `append_byte` for one ASCII byte
character such as `'!'`, and `append_bytes` or `push_str` for a borrowed
`Slice[u8]`. `push_str` is the CLI/parser-friendly growth spelling:

```ari
var message = std::string::empty(ref mut zone);
message.push_str(ref mut zone, "Created package ");
message.push_str(ref mut zone, name);
```

`char` is a public alias for `u8`, so binary byte buffers can still call these
APIs, but text-like call sites should prefer single-quoted character literals.
The `_in` forms remain the lower level names used by older tests and
compiler-assisted formatting.

For tracked local `String` handles, Ari can infer the same source zone for the
common non-`_in` convenience calls documented in the language guide. The
explicit `_in` forms are still the clearest shape for library code and tests.

Appending formatted primitive values also grows through the explicit zone:

```ari
text.append_string_in(ref mut zone, "text")
text.append_i64_in(ref mut zone, value)
text.append_u64_in(ref mut zone, value)
text.append_bool_in(ref mut zone, value)
text.append_value_in(ref mut zone, display_value)
text.append_debug_in(ref mut zone, debug_value)
text.append_f32_in(ref mut zone, value, precision)
text.append_f64_in(ref mut zone, value, precision)
text.push_codepoint_in(ref mut zone, scalar)
```

These helpers are the current source-side building blocks used by owned
formatting paths. `append_value_in[T: std::fmt::Display]` calls
`value.format_in(ref mut zone)` and appends the rendered bytes, so standard
display values and user-defined types can participate without adding names such
as `append_point_in`. `append_debug_in[T: std::fmt::Debug]` mirrors that shape
for diagnostic text and calls `value.debug_in(ref mut zone)`.
Tracked local strings can call the natural convenience form:

```ari
text.append_value(point)
text.append_debug(point)
```

The compiler lowers that call to the explicit same-zone form, just like
`append_i64(value)` and `resize(length, char)`. `push_codepoint_in` validates
`scalar` with `std::encoding::utf8_encoded_len` and appends its UTF-8 bytes,
panicking for an invalid Unicode scalar value.

## Search And Comparison

Module view helpers and `String` methods operate on bytes:

```ari
std::string::lines(bytes)
std::string::trim_start(bytes)
std::string::trim_end(bytes)
std::string::trim(bytes)
std::string::split(bytes, delimiter)
std::string::split_once(bytes, delimiter)
std::string::find(bytes, needle)
std::string::contains(bytes, needle)
std::string::starts_with(bytes, prefix)
std::string::ends_with(bytes, suffix)
std::string::strip_prefix(bytes, prefix)
std::string::strip_suffix(bytes, suffix)
std::string::substring(bytes, start, end)
```

`lines` is a borrowed `'\n'` split iterator. It leaves any preceding `'\r'` in
the line; call `trim` on each line for CRLF-tolerant ASCII manifest parsing.
`split_once` returns `Option[std::string::SplitOnce]`; call `.left()` and
`.right()` for the borrowed views around the first delimiter occurrence. Empty
delimiter matches at byte offset `0`. `strip_prefix` and `strip_suffix` return
`Option[Slice[u8]]`, preserving the reason-less absence shape used by borrowed
view helpers.

Owned `String` search helpers use the same byte policy:

```ari
text.index_of(byte)
text.contains(byte)
text.count(byte)
text.find(bytes)
text.find_text("text")
text.contains_slice(bytes)
text.contains_text("text")
```

`index_of`, `contains`, and `count` operate on one byte. `find` searches for a
borrowed byte slice and returns the first byte offset or `-1`; an empty search
slice matches at `0`. `contains_slice` is the boolean wrapper. The `_text`
forms accept Ari `string` values directly, so callers do not have to spell
`std::string::bytes("literal")` at every search site.

Slice comparison and view helpers operate on borrowed `Slice[u8]` values:

```ari
text.slice(start, end)
text.split_at(index)
text.chunks(size)
text.windows(size)
text.split(delimiter)
text.starts_with(bytes)
text.starts_with_text("text")
text.ends_with(bytes)
text.ends_with_text("text")
text.equals(bytes)
text.equals_text("text")
text.eq(other_owned_string)
text == other_owned_string
```

`slice` and `split_at` return borrowed byte views. `chunks`, `windows`, and
delimiter `split` are lazy iterators over borrowed byte views and do not
allocate. They compare exact byte values and do not perform case folding or
decoding. The `_text` variants are exact byte comparisons against Ari `string`
values without the trailing NUL. `eq` and the `==` / `!=` operators compare
owned `String` values by byte contents, which makes `String` usable as a
`HashMap` key alongside `std::hash::Hash[String]`.

## ASCII Helpers

`String` exposes convenience methods for the `std::ascii` slice helpers:

```ari
text.equals_ignore_case(bytes)
text.equals_text_ignore_case("text")
text.starts_with_ignore_case(bytes)
text.starts_with_text_ignore_case("text")
text.ends_with_ignore_case(bytes)
text.ends_with_text_ignore_case("text")
text.index_of_ignore_case(bytes)
text.index_of_text_ignore_case("text")
text.contains_ignore_case(bytes)
text.contains_text_ignore_case("text")
text.trim_start()
text.trim_start_to(ref mut zone)
text.trimmed_start(ref mut zone)
text.trim_end()
text.trim_end_to(ref mut zone)
text.trimmed_end(ref mut zone)
text.trim()
text.trim_to(ref mut zone)
text.trimmed(ref mut zone)
text.parse_decimal()
text.parse_decimal_prefix()
text.parse_signed_decimal()
text.parse_signed_decimal_prefix()
text.parse_hex()
text.parse_hex_prefix()
```

The case-insensitive helpers fold only ASCII letters and then reuse the
`std::ascii` comparison/search policy. `index_of_ignore_case` returns the first
matching byte offset or `-1`; an empty search slice matches at `0`.

The trim methods return borrowed `Slice[u8]` views into the same storage; they
do not allocate or copy. The `*_to` forms return owned `String` copies in a
target zone, so the copied handle remains usable after the source zone is reset
or destroyed. `trimmed_start`, `trimmed_end`, and `trimmed` are the friendlier
owned-copy aliases for those `*_to` methods. The whole-string parse methods
require the entire string to be a valid decimal or hexadecimal ASCII integer
and return `Option[i64]`. Empty input, whitespace, or invalid bytes return
`None<i64>()`. `parse_signed_decimal` accepts one optional leading `+` or `-`
and still rejects empty input, bare signs, whitespace, and trailing bytes. The
prefix parsers return `Option[std::ascii::ParsedInt]` with the parsed `value`
and consumed byte `len`, stopping before the first invalid byte.
`parse_signed_decimal_prefix` counts an accepted sign in `len` and requires at
least one digit after that sign. To accept surrounding ASCII whitespace, trim
first and parse the returned slice with `std::ascii`:

```ari
let view = text.trim();
let value = std::ascii::parse_decimal(view).unwrap_or(0);
```

Small manifest-style parsing can stay allocation-free until it needs to keep a
value:

```ari
var lines = std::string::lines(file_text.as_slice());
let line = std::string::trim(lines.next().unwrap());
if std::string::starts_with(line, "#") {
  return 0;
}
let pair = std::string::split_once(line, "=").unwrap();
let key = std::string::trim(pair.left());
let raw_value = std::string::trim(pair.right());
let unquoted = std::string::strip_suffix(
  std::string::strip_prefix(raw_value, "\"").unwrap(),
  "\""
).unwrap();
```

Overflow behavior is not promised yet.

## UTF-8 Helpers

`String` remains byte-oriented, but it can validate and inspect UTF-8 through
`std::encoding`:

```ari
text.is_utf8()
text.try_utf8()
text.codepoint_count()
text.codepoint_at(byte_index)
text.codepoint_next_index(byte_index)
text.codepoints()
text.push_codepoint_in(ref mut zone, scalar)
```

`is_utf8` is the boolean validation form. `try_utf8` returns
`Option[std::string::Utf8]` so validated text can be passed around without
rechecking at each call site. `codepoint_count` returns
`Option[i64]`, with `None<i64>()` for invalid UTF-8. `codepoint_at` decodes one
Unicode scalar at a byte offset and returns `Option[std::encoding::Utf8Char]`.
It returns `None` for continuation-byte offsets, invalid sequences, and
out-of-range indexes. The returned `Utf8Char` has `scalar()`, `len()`, and
`next_index(byte_index)` accessors. `codepoint_next_index` returns the byte
offset after the scalar at `byte_index`, or `None<i64>()` for the same invalid
inputs as `codepoint_at`, so callers can iterate by byte offset without
decoding the same scalar twice. `text.codepoints()` validates the whole byte
string and returns `Option[std::string::Codepoints]`; the iterator yields
`std::encoding::Utf8Char` values in byte order and returns `None` when the input
is invalid or after the last scalar. These helpers count Unicode scalar values,
not grapheme clusters.

Validated borrowed UTF-8 bytes use `Utf8`:

```ari
let view = std::string::utf8(bytes).unwrap();
let literal_view: std::string::Utf8 = "\xC3\xA9";
view.as_slice()
view.len()
view.is_empty()
view.codepoint_count()
view.codepoint_at(byte_index)
view.next_index(byte_index)
view.codepoints()
```

`Utf8` records the API intent that the bytes are text. Construct the view with
`std::string::utf8(bytes)` when invalid UTF-8 is normal input; do not mutate
the underlying bytes while keeping a validated view. A string literal can also
coerce directly to `Utf8` when that type is expected; the compiler validates
the literal bytes and rejects invalid UTF-8. For the UTF-8-specific scalar
helpers, the literal can be the receiver directly:

```ari
"\xC3\xA9".codepoint_count()
"\xC3\xA9".codepoint_at(0)
```

`Utf8::codepoints()` does not revalidate because constructing `Utf8` already
proved the borrowed bytes valid. The returned `Codepoints` handle is a normal
`std::Iterator[std::encoding::Utf8Char]`, so it can be passed to iterator
consumers or advanced directly:

```ari
var iter = text.try_utf8().unwrap().codepoints();
while true {
  match iter.next() {
    std::Some(ch) => {
      // ch.scalar(), ch.len()
    }
    std::None => {
      break;
    }
  }
}
```

Use `Utf8String` when validated text needs to own its storage:

```ari
let owned = std::string::utf8_string(ref mut zone, bytes).unwrap();
owned.as_slice()
owned.as_string()
owned.as_utf8()
owned.len()
owned.is_empty()
owned.codepoint_count()
owned.codepoint_at(byte_index)
owned.next_index(byte_index)
owned.codepoints()
owned.to_string(ref mut zone)
```

`Utf8String` is a distinct owned wrapper around a zone-backed `String`.
Construction validates the whole byte slice before copying. `utf8_string` keeps
the detailed `Utf8Error`; `utf8_string_optional` collapses invalid UTF-8 to
`None`; `utf8_string_unchecked` is the asserting trusted-input form. The type
does not expose mutable access to the underlying bytes because that would break
the validation invariant.

## OS Strings And C ABI Views

`OsStr` makes OS-boundary data explicit. C ABI text uses the single
`std::c::CStr` borrowed view:

```ari
let os = std::string::os_str(bytes);
let owned_os = std::string::os_string(ref mut zone, bytes);
let owned_text_os = std::string::os_string_from_text(ref mut zone, "literal");
let literal_os: std::string::OsStr = "literal";
os.as_slice()
os.len()
os.is_empty()
os.is_utf8()
os.try_utf8()
owned_os.as_slice()
owned_os.as_string()
owned_os.as_os_str()
owned_os.len()
owned_os.is_empty()
owned_os.is_utf8()
owned_os.try_utf8()
owned_os.try_utf8_string(ref mut zone)
owned_os.to_string(ref mut zone)

let c = std::string::c_str("literal");
let literal_c: std::c::CStr = "literal";
c.as_ptr()
c.as_slice()
c.len()
c.is_empty()
```

`OsStr` and `OsString` are not text by default. Convert with `try_utf8` only
after deciding the OS bytes should be interpreted as UTF-8; use
`try_utf8_string` when that validated text also needs owned storage. Use
`std::path::from_os(os)` when the same bytes should be interpreted as path
bytes. `OsString` is currently a POSIX byte buffer over `String`; richer
platform-specific OS-string storage remains roadmap work.
`std::c::CStr.as_slice()` and `std::string::c_bytes(text)` exclude the trailing
NUL because Ari byte-slice helpers operate on logical content bytes. Keep owned
C-shaped storage in `std::c::CString`; it lives with `std::c::CStr`, POSIX
`errno`, and dynamic loader handles in the dedicated C ABI module.
String literals can use the OS-boundary validation helpers directly as
receivers too: `"name".is_utf8()` and `"name".try_utf8()`.

## Example

```ari
fn main() -> i64 {
  var zone = zone::create(128);
  var text = std::string::from_string(ref mut zone, " 123 ");

  let trimmed = text.trim();
  let value = std::ascii::parse_decimal(trimmed).unwrap_or(0);

  text.clear();
  text.append_string_in(ref mut zone, "score=");
  text.append_i64_in(ref mut zone, value);

  // For user-defined Display/Debug types, prefer text.append_value(value) or
  // text.append_debug(value) at call sites where the String receiver came from
  // a tracked local zone.

  let answer = text.len() + value;
  zone::destroy(zone);
  return answer;
}
```

## Tests

Focused positive tests include:

```text
tests/cases/standard-library/ok/string/std-string-handle.ari
tests/cases/standard-library/ok/string/std-string-first-last.ari
tests/cases/standard-library/ok/string/std-string-try-byte-access.ari
tests/cases/standard-library/ok/string/std-string-byte-remove.ari
tests/cases/standard-library/ok/string/std-string-byte-retain.ari
tests/cases/standard-library/ok/string/std-string-search.ari
tests/cases/standard-library/ok/string/std-string-split-join.ari
tests/cases/standard-library/ok/string/std-string-natural-api.ari
tests/cases/standard-library/ok/string/std-string-module-views.ari
tests/cases/standard-library/ok/string/std-string-prefix-suffix.ari
tests/cases/standard-library/ok/string/std-string-equals.ari
tests/cases/standard-library/ok/string/std-string-ascii-helpers.ari
tests/cases/standard-library/ok/string/std-string-ascii-case-helpers.ari
tests/cases/standard-library/ok/string/std-string-prefix-parsers.ari
tests/cases/standard-library/ok/string/std-string-signed-parsers.ari
tests/cases/standard-library/ok/string/std-string-trim-copy.ari
tests/cases/standard-library/ok/string/std-string-grow.ari
tests/cases/standard-library/ok/string/std-string-append.ari
tests/cases/standard-library/ok/string/std-string-append-debug.ari
tests/cases/standard-library/ok/string/std-string-from-slice-in.ari
tests/cases/standard-library/ok/string/std-string-byte-literals.ari
tests/cases/standard-library/ok/string/std-string-unicode-helpers.ari
tests/cases/standard-library/ok/string/std-string-text-kinds.ari
```

Focused diagnostics include:

```text
tests/cases/standard-library/errors/string/std-string-trim-to-after-target-reset.ari
tests/cases/standard-library/errors/string/std-string-append-debug-different-zone.ari
```

`make check-prelude` compiles these to LLVM, checks representative symbols and
runtime hooks, and runs executable checks where behavior is observable. Public
methods are tracked in `tests/std_api_manifest.txt` and checked by
`make check-std-api`.

## Future Work

Potential next slices:

- signed and overflow-checked parsers after numeric policy is documented
- a deliberate text/Unicode module for normalization, grapheme clusters, and
  transcoding beyond the UTF-8 scalar helpers
- broader formatter integration as `Display` and `Debug` dispatch mature
