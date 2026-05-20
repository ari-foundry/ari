# std::source

`std::source` contains the small value types used to describe where something
came from in source code. It is a bootstrap-facing module: lexer, parser,
diagnostic, and test tools can share the same coordinate vocabulary without
inventing ad hoc tuples everywhere.

The module is source-only. It does not read files and does not allocate.

## Coordinate Policy

- `FileId` is an opaque non-negative file number chosen by a source map.
- `Span` is a half-open byte range: `start <= offset < end`.
- `Span::touches(offset)` is inclusive at the end for cursor and caret logic.
- `LineCol` and `Location` use one-based human coordinates.
- Byte offsets are not Unicode scalar indexes. Decode UTF-8 through
  `std::encoding` when a tool needs character-level behavior.

## API

```ari
source::FileId
source::Span
source::LineCol
source::Location

source::file_id(value)
source::root_file()
source::span(file, start, end)
source::empty_span(file, offset)
source::line_col(line, column)
source::location(file, line, column)
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
```

## Invariants

`file_id(value)` asserts that `value >= 0`.

`span(file, start, end)` asserts that both offsets are non-negative and that
`end >= start`.

`line_col(line, column)` and `location(file, line, column)` assert that line
and column are both at least `1`.

`merge(ref left, ref right)` asserts that both spans point at the same file.

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

`std::source` does not yet include a source map that stores file names, text, or
line-start tables. The next bootstrap slice should build that on top of these
value types, then use it from a diagnostic builder.

`Location` is just a file/line/column value. It is not automatically derived
from `Span`; that conversion belongs in the future source-map layer because it
needs access to file text.

## Tests

- `tests/cases/standard-library/ok/source/std-source-location.ari` checks
  constructors, invariants reachable through asserting constructors, scalar
  accessors, half-open containment, inclusive boundary checks, same-file
  merging, ordering, and method wrappers.
- `make check-source` compiles the focused fixture, inspects the generated
  source helper symbols, and runs the executable.
- `make check-std-api` tracks every public declaration in
  `tests/std_api_manifest.txt`.
