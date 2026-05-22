# Compiler Source Identity

This page defines how Ari compiler work models source files, source ids, byte
spans, line/column lookup, snippets, and source-map artifacts. It is ordinary
hosted-compiler development.

Source identity is the base layer for diagnostics, module graphs, declaration
indexes, syntax dumps, typed IR traces, and future compiler tooling. If source
identity is unstable, every later artifact becomes harder to trust.

## Goals

Source identity should make these facts deterministic:

- which files were loaded
- which generated, built-in, or unknown-origin sources exist
- which `SourceId` belongs to each loaded source
- which byte range a token, AST node, diagnostic label, or declaration covers
- how byte offsets map to one-based line and column values for users
- how source paths appear in golden artifacts

This layer should not know type checking, ownership, trait selection, LLVM
details, or private stage flags.

## Current Implementation

The hosted C++ compiler keeps the first source identity bridge in
`src/common.hpp` and `src/common.cpp`:

- `SourceId` is a small wrapper around an integer value.
- `SourceFile` stores `id`, `kind`, canonical `path`, diagnostic
  `display_name`, owned source `text`, `line_starts`, and `eof_offset`.
- `Span` stores `source_id`, zero-based byte `start`, and zero-based byte
  `end`. `SourceSpan` is a compatibility alias for the same C++ type.
- `SourceLocation` stores the canonical `Span` plus one-based `line` and
  `column`, legacy `byte_start` and `byte_end` cache fields, a
  `has_byte_range` flag, and the display `source_name`.
- `CompileError(SourceLocation, message)` is the preferred diagnostic path.
  The old string-based `CompileError(where(loc) + ": ...")` bridge remains so
  older throw sites can preserve structured locations while they migrate.
- `register_source_file(path, display_name, text, kind)` registers or updates
  a source with separate identity and diagnostic names.
- `register_source_text(name, text, kind)` is the compatibility wrapper for
  file-backed sources where path and display are the same. An empty name is
  treated as an in-memory source with `Unknown` kind unless the caller passes a
  more specific non-file kind.
- `register_in_memory_source(display_name, text, kind)` creates a deterministic
  `<memory:N>` identity for test, REPL, generated, or virtual source text.

Registration currently happens at the source ingestion edge:

- `lex_source(source, input_path)` registers CLI input for token, syntax,
  diagnostic, pass-summary, and normal compile flows.
- `parse_file_in_module(path, ...)` registers root and file-backed module
  sources before parsing.
- Cached module AST summaries are not materialized from summary-only data while
  summaries do not preserve source spans. The loader falls back to cached source
  text, lexes it, and keeps normal source identity.

## SourceFile Ownership

One `SourceFile` owns all stable facts needed to locate source bytes:

- `path`: the canonical identity key used for registration and de-duplication.
- `display_name`: the text shown in diagnostics and committed artifacts.
- `text`: the complete source text bytes.
- `line_starts`: zero-based byte offsets for each line start, including the EOF
  line start when the source ends in a newline.
- `eof_offset`: the byte offset of the end of the file, equal to `text.size()`.

File-backed sources usually pass the same normalized project path for `path`
and `display_name`. Tools may pass a stable canonical path plus a shorter
display name when diagnostics should hide host-specific absolute paths.
In-memory sources must use `register_in_memory_source` unless they already have
a stable caller-owned path-like identity.

## SourceMap API

`SourceMap` is the owner that turns source bytes into stable lookup data. If a
tool has a `SourceMap`, it can answer which file owns any byte span and which
one-based line and column that span starts at.

Current C++ API shape:

```cpp
SourceMap sources;
SourceId main = sources.add_file("src/main.ari", main_text);
const SourceFile* file = sources.get(main);
Span name = sources.span(main, 3, 7);
LineColumn point = sources.location(main, 3);
SourceSnippet rendered = sources.snippet(name);
```

Rules:

- `add_file(path, text)` registers file-backed text with `path` as both the
  canonical identity and the diagnostic display name.
- `add_source(path, display_name, text, kind)` is for callers that need
  canonical and display names to differ.
- `add_in_memory(display_name, text, kind)` creates deterministic in-memory
  identities such as `<memory:0>`.
- `get(SourceId)` returns the stable `SourceFile` for source-level lookup.
- `span(source_id, start, end)` creates a half-open byte span tied to that
  source id. The `SourceMap` constructor clamps to the file EOF and normalizes
  reversed input into an empty insertion span, so callers do not accidentally
  manufacture out-of-range spans.
- `valid_span(span)` verifies that a span's source id is registered, that
  `start <= end`, and that `end` is within the owning source.
- `location(source_id, byte_offset)` uses the file's line table to return a
  one-based `LineColumn`.
- `snippet(span)` returns source name, line text, and caret marker data without
  requiring a diagnostic renderer.
- The hosted compiler's legacy free functions call `default_source_map()` so
  existing lexer/parser/sema code keeps using the same source model.

Tokens store `Span` directly. AST and IR nodes currently expose `SourceLocation`
for compatibility, and that location carries the same canonical `Span` in
`loc.span`; line and column are derived display data. Diagnostic labels store
`Span`, not a bare byte range. Each label can carry a different `SourceId`, so
one diagnostic can point at the use site in one file and a declaration or import
site in another file.

## Source Kinds

Ari should distinguish source origin without changing span math:

| Kind | Meaning | Example |
| --- | --- | --- |
| File source, `File` | Bytes loaded from a real `.ari` or `.arih` path. | Entry files and file-backed modules. |
| Generated source, `Generated` | Compiler-created source-like text with stable ownership. | Future test runners or macro expansion artifacts. |
| Builtin source, `Builtin` | Compiler-known source that did not come from a project file. | Future built-in prelude snippets or synthesized declarations. |
| Virtual source, `Unknown` | Registered text whose origin is not classified yet. | Transitional tool buffers or migration cases. |

Every kind still receives a `SourceId`. Generated, built-in, and tool-provided
sources should be visible in artifacts instead of borrowing another file's
identity.

## Invalid SourceId Policy

`SourceId{-1}` is the only invalid/null source id. It is for empty
`SourceLocation` values, internal errors that are not tied to user source, and
transitional code paths while a real span is being threaded through the
compiler.

User-facing spans must not rely on invalid ids:

- A token span must have a valid `SourceId`.
- A parser or sema diagnostic should use a token or AST location that already
  has a valid `SourceId`.
- A byte range without a valid source id is not enough to render a diagnostic
  snippet.
- `--emit-diagnostics` only emits source fields when it can resolve a valid
  source id. Otherwise it falls back to a message-only diagnostic instead of
  emitting a misleading span.
- `source_id=invalid` is reserved for debugging internal source-map state and
  should not appear in committed diagnostic or token goldens.

## Stable SourceId Rules

`SourceId` stability is scoped to one compiler invocation:

- assign ids in deterministic load order
- keep the entry file first
- record file-backed child modules when they are resolved
- do not recycle ids during one invocation
- store source kind, canonical path, and display name beside the id
- normalize repository-local paths in golden text
- keep absolute host paths out of committed artifacts when possible

Do not promise that a `SourceId` is stable across separate builds. Golden files
should compare rendered source paths and stage facts, not raw pointer identity
or allocation order.

The current numeric artifact format is fixed as `source_id=N`, where `N` is the
decimal `SourceId.value` assigned during that invocation. The entry source is
registered first in normal CLI flows, so single-file token and diagnostic
goldens use `source_id=0`; file-backed module source maps show the root as
`source_id=0` and each loaded child source with the next deterministic id.

## Span Rules

Use byte spans internally:

```ari
type BytePos = i64;

struct Span {
  source: SourceId,
  start: BytePos,
  end: BytePos,
}
```

Rules:

- `Span.start` is inclusive
- `Span.end` is exclusive
- `start <= end`
- empty spans are allowed for insertion points, EOF diagnostics, and parser
  "expected here" errors
- spans must stay within the source byte length
- spans are byte ranges, not Unicode scalar or display-width ranges
- line/column lookup is derived from byte offsets and source text

Use byte spans in lexer, parser, sema, and artifacts. Add Unicode display-width
later inside rendering, not inside core span math.

In C++ code, `Span` is the canonical range. `SourceLocation` is the diagnostic
view that keeps `loc.span` together with cached line/column and display-name
data. Set `has_byte_range=true` only when `loc.span`, `source_id`,
`byte_start`, and `byte_end` describe the same range. Prefer
`set_location_span(loc, span)` over writing those fields by hand.

Current helper API:

- `source_span(id, start, end)` constructs a half-open span and normalizes
  `end < start` to an empty span at `start`.
- `span_from_location(loc)` extracts the canonical span from a diagnostic
  location.
- `set_location_span(loc, span)` keeps `loc.span`, `source_id`, `byte_start`,
  `byte_end`, and `has_byte_range` synchronized.
- `span_has_valid_order(span)` checks `start <= end`.
- `span_is_empty(span)` reports insertion spans.
- `span_length(span)` returns `end - start` for valid ordered spans.
- `span_contains(span, byte_offset)` follows half-open range rules. Empty spans
  contain no bytes.
- `span_contains(outer, inner)` accepts empty inner spans when the insertion
  point lies inside the outer range.
- `span_intersects(lhs, rhs)` is false for empty spans and spans from different
  sources.
- `merge_spans(lhs, rhs)` returns the smallest span covering both ranges when
  they belong to the same source. Merging different sources returns an invalid
  span, so multi-file diagnostics must keep separate labels instead.

Generated and built-in spans follow the same rules as file spans: register the
source text with `Generated` or `Builtin` kind, keep its `SourceId`, and point
spans into that registered text. Do not borrow a user file's source id for
compiler-created nodes. Use `SourceId{-1}` only when there is truly no
user-facing span to render.

## Line And Column Lookup

Rendered diagnostics should use one-based line and column values. Internal
offsets remain zero-based.

Lookup policy:

- line starts at `1`
- column starts at `1`
- newline bytes advance the line and reset the column
- line/column lookup should use `SourceFile.line_starts`, not a fresh scan of
  source text at each diagnostic render
- CRLF should be normalized or explicitly represented in source-map artifacts
- EOF spans should render at the final valid insertion point
- invalid spans should fail before rendering or fall back to a message-only
  diagnostic

The source-map artifact should make this policy reviewable with tiny examples,
including empty files, trailing newlines, multi-line spans, and file-backed
module sources.

## Artifact Policy

Use deterministic artifacts before relying on executable behavior:

| Artifact | Proves |
| --- | --- |
| `--emit-source-map` | Loaded sources, source ids, source kind, canonical path, display name, EOF offset, line table size, byte lengths, and line starts. |
| `--emit-tokens` | Token spans, source ids, and source ownership after lexing. |
| `--emit-syntax` | AST spans and source-shaped parser output. |
| `--emit-declaration-index` | Declaration spans, names, visibility, and module ownership. |
| `--emit-module-graph` | File-backed module sources and import edges. |
| `--emit-diagnostics` | Diagnostic source ids, labels, line/column lookup, snippets, and normalized paths. |

Put source identity goldens under
`tests/cases/compiler-development/artifact/ok/` or
`tests/cases/compiler-development/artifact/errors/` depending on whether the
artifact represents a valid source map or an expected diagnostic.

## Implementation Slices

Use these slices for normal compiler work:

1. Source table: one owner for loaded source bytes, source kind, canonical path,
   display name, line starts, EOF offset, and stable ids.
2. Span validation: reject invalid source ids, negative byte positions,
   reversed ranges, and out-of-bounds ranges.
3. Line table: derive line starts from bytes and expose byte-column lookup.
4. Snippet extraction: return source lines and caret ranges without formatting
   the whole diagnostic.
5. Artifact normalization: keep paths, generated ids, and line endings stable.
6. Multi-file coverage: include root plus file-backed module sources.
7. Diagnostic integration: labels should carry spans instead of only strings.

Each slice should land with one focused fixture or golden artifact.

## Developer Workflow

When adding or changing a diagnostic:

1. Prefer carrying an existing token, AST node, or IR location into the error.
2. Throw `CompileError(loc, message)` when the code already has a
   `SourceLocation`.
3. Use `where(loc)` only for older helper APIs that still take text; it now
   preserves `source_name`, line/column, and byte range for the transitional
   parser in `CompileError`.
4. If the error is about the whole input rather than a token, use an EOF
   location from `source_end_location(input)`.
5. Add or update the smallest artifact that proves the source identity behavior:
   `--emit-tokens`, `--emit-source-map`, or `--emit-diagnostics`.

When adding a new source origin:

1. Register file-backed source text once at the ingestion edge with
   `register_source_file` when canonical path and display name differ, or
   `register_source_text` when they are the same.
2. Choose `File`, `Generated`, `Builtin`, or `Unknown` explicitly.
3. Store and pass the returned `SourceId`; do not infer identity from a path
   string later if the id is already available.
4. Keep artifact output as `source_id=N`, plus a separate display path or source
   name field.
5. Use `register_in_memory_source` for test buffers, REPL snippets, generated
   source, or virtual sources that do not have a filesystem path.

## Review Checklist

Before handing off source identity work, answer:

- Which source kind does the change affect?
- Does the entry file still receive a deterministic first id?
- Are generated, built-in, or unknown-origin sources represented explicitly?
- Are spans byte-based, inclusive/exclusive, and bounds-checked?
- Does line/column rendering stay one-based for users?
- Is the capability inventory, source-map, token, syntax, declaration, module,
  or diagnostic artifact the closest proof?
- Did diagnostics improve without moving compiler-only APIs into runtime `std`?
- Does every committed diagnostic/token/source-map golden avoid
  `source_id=invalid`?

Strong source identity moves Ari toward a real compiler by making every later
stage reviewable. It should help all Ari tools.
