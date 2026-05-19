# std::string

`std::string` contains Ari's current owned byte-string handle. It exists so
programs can copy borrowed `string` literals, formatted output, or byte slices
into an explicit allocation `Zone` without introducing a hidden global heap.

Today's `String` is intentionally a byte string. It is useful for compiler
tests, CLI-style output, simple parser buffers, and ASCII-oriented text work.
It now has UTF-8 validation and scalar helpers, but it is not a Unicode
normalization, grapheme, or locale-aware text abstraction yet.

## When To Use It

Use `std::string::String` when bytes must outlive a borrowed literal or input
buffer and you can name the `Zone` that owns the storage. Use `Slice[u8]` when
you only need a borrowed view. Use `std::ascii` or the `String` ASCII helpers
for byte classification, ASCII-only comparison/search, trimming, and integer
parsing.

Avoid using `String` as a general text policy. UTF-8 helpers operate on Unicode
scalar values and byte offsets. Unicode normalization, grapheme iteration,
encoding conversion, and locale-sensitive case conversion are future library
work.

## Constructors And Copies

Constructors allocate in an explicit zone:

```ari
std::string::new(ref mut zone, capacity)
std::string::from_string(ref mut zone, "text")
std::string::from_slice_in(ref mut zone, bytes)
std::string::join_in(ref mut zone, parts, separator)
```

`new` creates an empty buffer with fixed starting capacity.
`from_string` copies a lowercase `string` literal/runtime value.
`from_slice_in` copies a borrowed `Slice[u8]`.
`join_in` joins a `Slice[Slice[u8]]` with a byte separator and returns an owned
`String` in the provided zone.

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
text.as_slice()
text.as_ptr()
```

`as_slice` returns a borrowed `Slice[u8]` over the current bytes. `as_ptr`
returns the backing byte pointer and preserves zone provenance in the checker.

## Mutation And Growth

Fixed-capacity mutation:

```ari
text.push(byte)
text.pop()
text.try_pop()
text.insert(index, byte)
text.clear()
text.truncate(length)
```

`try_pop` returns `None<u8>()` when the string is empty. `pop` asserts in that
case and is best when an empty string means a programmer error.

Growth-capable mutation uses the owning zone. The explicit forms are the
source-level contract:

```ari
text.push_in(ref mut zone, byte)
text.insert_in(ref mut zone, index, byte)
text.reserve(ref mut zone, capacity)
text.reserve_extra(ref mut zone, additional)
text.extend_from_slice_in(ref mut zone, bytes)
text.resize_in(ref mut zone, length, byte)
```

For tracked local `String` handles, Ari can infer the same source zone for the
common non-`_in` convenience calls documented in the language guide. The
explicit `_in` forms are still the clearest shape for library code and tests.

Appending formatted primitive values also grows through the explicit zone:

```ari
text.append_string_in(ref mut zone, "text")
text.append_i64_in(ref mut zone, value)
text.append_u64_in(ref mut zone, value)
text.append_bool_in(ref mut zone, value)
text.append_f32_in(ref mut zone, value, precision)
text.append_f64_in(ref mut zone, value, precision)
text.push_codepoint_in(ref mut zone, scalar)
```

These helpers are the current source-side building blocks used by owned
formatting paths. `push_codepoint_in` validates `scalar` with
`std::encoding::utf8_encoded_len` and appends its UTF-8 bytes, panicking for an
invalid Unicode scalar value.

## Search And Comparison

Search helpers operate on bytes:

```ari
text.index_of(byte)
text.contains(byte)
text.count(byte)
text.find(bytes)
text.contains_slice(bytes)
```

`index_of`, `contains`, and `count` operate on one byte. `find` searches for a
borrowed byte slice and returns the first byte offset or `-1`; an empty search
slice matches at `0`. `contains_slice` is the boolean wrapper.

Slice comparison and view helpers operate on borrowed `Slice[u8]` values:

```ari
text.slice(start, end)
text.split_at(index)
text.chunks(size)
text.windows(size)
text.split(delimiter)
text.starts_with(bytes)
text.ends_with(bytes)
text.equals(bytes)
```

`slice` and `split_at` return borrowed byte views. `chunks`, `windows`, and
delimiter `split` are lazy iterators over borrowed byte views and do not
allocate. They compare exact byte values and do not perform case folding or
decoding.

## ASCII Helpers

`String` exposes convenience methods for the `std::ascii` slice helpers:

```ari
text.equals_ignore_case(bytes)
text.starts_with_ignore_case(bytes)
text.ends_with_ignore_case(bytes)
text.index_of_ignore_case(bytes)
text.contains_ignore_case(bytes)
text.trim_start()
text.trim_start_to(ref mut zone)
text.trim_end()
text.trim_end_to(ref mut zone)
text.trim()
text.trim_to(ref mut zone)
text.parse_decimal()
text.parse_decimal_prefix()
text.parse_hex()
text.parse_hex_prefix()
```

The case-insensitive helpers fold only ASCII letters and then reuse the
`std::ascii` comparison/search policy. `index_of_ignore_case` returns the first
matching byte offset or `-1`; an empty search slice matches at `0`.

The trim methods return borrowed `Slice[u8]` views into the same storage; they
do not allocate or copy. The `*_to` forms return owned `String` copies in a
target zone, so the copied handle remains usable after the source zone is reset
or destroyed. The whole-string parse methods require the entire string to be a
valid decimal or hexadecimal ASCII integer and return `Option[i64]`. Empty
input, whitespace, or invalid bytes return `None<i64>()`. The prefix parsers
return `Option[std::ascii::ParsedInt]` with the parsed `value` and consumed
byte `len`, stopping before the first invalid byte. To accept surrounding ASCII
whitespace, trim first and parse the returned slice with `std::ascii`:

```ari
let view = text.trim();
let value = std::ascii::parse_decimal(view).unwrap_or(0);
```

Overflow behavior is not promised yet.

## UTF-8 Helpers

`String` remains byte-oriented, but it can validate and inspect UTF-8 through
`std::encoding`:

```ari
text.is_utf8()
text.codepoint_count()
text.codepoint_at(byte_index)
text.push_codepoint_in(ref mut zone, scalar)
```

`is_utf8` is the boolean validation form. `codepoint_count` returns
`Option[i64]`, with `None<i64>()` for invalid UTF-8. `codepoint_at` decodes one
Unicode scalar at a byte offset and returns `Option[std::encoding::Utf8Char]`.
It returns `None` for continuation-byte offsets, invalid sequences, and
out-of-range indexes. The returned `Utf8Char` has `scalar()`, `len()`, and
`next_index(byte_index)` accessors. These helpers count Unicode scalar values,
not grapheme clusters.

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
tests/cases/standard-library/ok/string/std-string-search.ari
tests/cases/standard-library/ok/string/std-string-split-join.ari
tests/cases/standard-library/ok/string/std-string-prefix-suffix.ari
tests/cases/standard-library/ok/string/std-string-equals.ari
tests/cases/standard-library/ok/string/std-string-ascii-helpers.ari
tests/cases/standard-library/ok/string/std-string-ascii-case-helpers.ari
tests/cases/standard-library/ok/string/std-string-prefix-parsers.ari
tests/cases/standard-library/ok/string/std-string-trim-copy.ari
tests/cases/standard-library/ok/string/std-string-grow.ari
tests/cases/standard-library/ok/string/std-string-append.ari
tests/cases/standard-library/ok/string/std-string-from-slice-in.ari
tests/cases/standard-library/ok/string/std-string-unicode-helpers.ari
```

Focused diagnostics include:

```text
tests/cases/standard-library/errors/string/std-string-trim-to-after-target-reset.ari
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
