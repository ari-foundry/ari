# std::source

`std::source` contains the small value types used to describe where something
came from in source code. It also provides a borrowed `SourceFile` view that can
convert byte offsets into one-based line/column locations, a cached `LineMap`
for compiler-style code that performs many lookups in the same file, and a
small `SourceMap` registry that assigns file ids and keeps cached line maps for
multiple borrowed files. It is a bootstrap-facing module: lexer, parser,
diagnostic, and test tools can share the same coordinate vocabulary without
inventing ad hoc tuples everywhere.

The module is source-only and does not read files. Plain coordinate helpers are
allocation-free. `line_map(ref mut zone, file)` explicitly allocates its cached
line-start table in the caller's `Zone`. `source_map(ref mut zone, capacity)`
allocates explicit storage for a bounded number of borrowed `SourceFile` values
and cached `LineMap` values.

## Coordinate Policy

- `FileId` is an opaque non-negative file number chosen by a source map.
- `Span` is a half-open byte range: `start <= offset < end`.
- `Span::touches(offset)` is inclusive at the end for cursor and caret logic.
- `LineCol` and `Location` use one-based human coordinates.
- `SourceFile` borrows a `Slice[u8]`; it does not own or copy source text.
- `LineMap` borrows the same `SourceFile` text and carries a cached newline
  table allocated in an explicit zone.
- `SourceMap` is an explicit-zone registry for borrowed `SourceFile` values.
  It assigns monotonically increasing `FileId` values starting at zero and
  stores one cached `LineMap` per registered file.
- `line_count(text)` returns at least `1`, so an empty file still has line 1.
- `line_start`, `line_end`, and `line_span` take one-based line numbers.
- `locate(file, text, offset)` accepts `offset == text.len()` for EOF
  diagnostics.
- `LineMap::locate(offset)` accepts the same EOF offset and uses a binary
  search over cached line starts instead of rescanning from byte zero.
- Byte offsets are not Unicode scalar indexes. Decode UTF-8 through
  `std::encoding` when a tool needs character-level behavior.

## API

```ari
source::FileId
source::Span
source::LineCol
source::Location
source::SourceFile
source::LineMap
source::SourceMap

source::file_id(value)
source::root_file()
source::file(id, text)
source::span(file, start, end)
source::empty_span(file, offset)
source::line_col(line, column)
source::location(file, line, column)
source::full_span(file, text)
source::line_count(text)
source::line_start(text, line)
source::line_end(text, line)
source::line_span(file, text, line)
source::locate(file, text, offset)
source::line_map(ref mut zone, file)
source::source_map(ref mut zone, capacity)
source::source_count(ref map)
source::source_capacity(ref map)
source::source_add(ref mut map, ref mut zone, text)
source::source_file(ref map, id)
source::source_lines(ref map, id)
source::source_locate(ref map, span)
source::len(ref span)
source::is_empty(ref span)
source::contains(ref span, offset)
source::touches(ref span, offset)
source::same_file(ref left, ref right)
source::before(ref left, ref right)
source::merge(ref left, ref right)
```

The same operations are available as methods where that reads better:

```ari
let file = source::file_id(2);
let name = source::span(file, 10, 14);
let keyword = source::span(file, 0, 3);
let whole = name.merge(ref keyword);

if whole.contains(12) {
  log::debug("span hit");
}

let input = source::file(file, "alpha\nbeta\n");
let beta = input.line_span(2);
let place = input.locate(beta.start());

if place.line() == 2 && place.column() == 1 {
  log::debug("second line starts here");
}
```

Build a `LineMap` when a tool will ask many location questions for the same
file:

```ari
var scratch = zone::create(4096);
let input = source::file(source::file_id(7), "let x = 1;\nreturn x;\n");
let map = input.line_map(ref mut scratch);
let token = map.line_span(2);
let where = map.locate(token.start());

if where.line() == 2 {
  log::debug("token starts on line two");
}
```

Use the direct `SourceFile` methods for one-off checks and `LineMap` for lexer,
parser, formatter, and diagnostic loops.

Use `SourceMap` when a tool needs a small multi-file registry but is not ready
for owned source text yet:

```ari
var scratch = zone::create(8192);
var sources = source::source_map(ref mut scratch, 16);

let main_id = sources.add(ref mut scratch, "fn main() -> i64 {\n  return 0;\n}\n");
let lib_id = sources.add(ref mut scratch, "pub fn answer() -> i64 {\n  return 42;\n}\n");

let main_file = sources.file(main_id);
let lib_lines = sources.lines(lib_id);
let where = sources.locate(source::span(lib_id, lib_lines.line_start(2), lib_lines.line_end(2)));

if main_file.line_count() == 4 && where.line() == 2 {
  log::debug("registered two borrowed source files");
}
```

`SourceMap::add(ref mut zone, text)` stores the borrowed text view and builds
the cached `LineMap` immediately. `SourceMap::file(id)` returns the borrowed
file view, `SourceMap::lines(id)` returns the cached line map, and
`SourceMap::locate(span)` uses the cached map for the span's file id. The free
functions with `source_` prefixes exist for code that prefers namespace-style
calls; the method API is the natural form for most library code.

## Invariants

`file_id(value)` asserts that `value >= 0`.

`span(file, start, end)` asserts that both offsets are non-negative and that
`end >= start`.

`line_col(line, column)` and `location(file, line, column)` assert that line
and column are both at least `1`.

`line_start(text, line)`, `line_end(text, line)`, and
`line_span(file, text, line)` assert that `line >= 1` and that the requested
line exists in `text`.

`locate(file, text, offset)` asserts that `0 <= offset <= text.len()`.

`merge(ref left, ref right)` asserts that both spans point at the same file.

`line_map(ref mut zone, file)` stores one entry per source line. Its line
methods assert that the requested line exists, and `LineMap::locate(offset)`
asserts that `0 <= offset <= file.len()`.

`source_map(ref mut zone, capacity)` asserts that `capacity >= 0`.
`SourceMap::add` asserts that the registry still has capacity. `SourceMap`
lookup methods assert that the file id exists in that registry. The stored
`SourceFile` values still borrow caller-owned text; the map only owns the
arrays used to index those borrowed views and cached line starts.

## Why This Exists

Compiler code needs many source coordinates:

- a token spans the bytes that produced it
- a parser error has a primary span
- a diagnostic label points at one span
- a note may point at a second span
- golden tests need stable byte and line/column output

Using named value types keeps those APIs readable. A tuple like `(file, start,
end)` is compact, but it does not carry invariants or methods. `Span` does.

## Current Limits

`std::source` does not yet include an owned source map that stores file names or
source text. `SourceFile`, `LineMap`, and `SourceMap` are borrowed views over
caller-owned text. A future source-map owner should own file names/text, keep
one cached `LineMap` per file, and feed a diagnostic builder.

`Location` is just a file/line/column value. Use `SourceFile::locate(offset)`
or `source::locate(file, text, offset)` when converting a byte offset to a
human coordinate.

## Tests

- `tests/cases/standard-library/ok/source/std-source-location.ari` checks
  constructors, invariants reachable through asserting constructors, scalar
  accessors, half-open containment, inclusive boundary checks, same-file
  merging, ordering, and method wrappers.
- `tests/cases/standard-library/ok/source/std-source-text.ari` checks borrowed
  source text, line counts, line starts/ends, line spans, EOF locations, and
  method wrappers.
- `tests/cases/standard-library/ok/source/std-source-line-map.ari` checks
  explicit-zone cached line maps, binary-search location lookup, line spans,
  empty-file behavior, and the `SourceFile::line_map` method wrapper.
- `tests/cases/standard-library/ok/source/std-source-map.ari` checks
  multi-file `SourceMap` registration, cached line-map lookup, free-function
  helpers, and natural method wrappers.
- `tests/cases/standard-library/errors/source/std-source-map-after-reset.ari`
  checks that a `SourceMap` is rejected after its backing zone is reset.
- `make check-source` compiles the focused fixtures, inspects the generated
  source helper symbols, and runs the executables.
- `make check-std-api` tracks every public declaration in
  `tests/std_api_manifest.txt`.
