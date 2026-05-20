# Compiler Source Identity

This page defines how Ari compiler work should model source files, source ids,
byte spans, line/column lookup, and source-map artifacts. It is ordinary
hosted-compiler development, not bootstrap implementation.

Source identity is the base layer for diagnostics, module graphs, declaration
indexes, syntax dumps, typed IR traces, and future compiler tooling. If source
identity is unstable, every later artifact becomes harder to trust.

## Goals

Source identity should make these facts deterministic:

- which files were loaded
- which generated or virtual sources exist
- which `SourceId` belongs to each loaded source
- which byte range a token, AST node, diagnostic label, or declaration covers
- how byte offsets map to one-based line and column values for users
- how source paths appear in golden artifacts

This layer should not know type checking, ownership, trait selection, LLVM
details, or bootstrap-only stage flags.

## Source Kinds

Ari should distinguish source origin without changing span math:

| Kind | Meaning | Example |
| --- | --- | --- |
| File source | Bytes loaded from a real `.ari` or `.arih` path. | Entry files and file-backed modules. |
| Generated source | Compiler-created source-like text with stable ownership. | Future test runners or macro expansion artifacts. |
| Virtual source | Tool-provided text that may not have a filesystem path. | LSP buffers or in-memory tests. |

Every kind still receives a `SourceId`. Generated and virtual sources should be
visible in artifacts instead of borrowing another file's identity.

## Stable SourceId Rules

`SourceId` stability is scoped to one compiler invocation:

- assign ids in deterministic load order
- keep the entry file first
- record file-backed child modules when they are resolved
- do not recycle ids during one invocation
- store source kind and display path beside the id
- normalize repository-local paths in golden text
- keep absolute host paths out of committed artifacts when possible

Do not promise that a `SourceId` is stable across separate builds. Golden files
should compare rendered source paths and stage facts, not raw pointer identity
or allocation order.

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
- empty spans are allowed for insertion points
- spans must stay within the source byte length
- spans are byte ranges, not Unicode scalar or display-width ranges
- line/column lookup is derived from byte offsets and source text

Use byte spans in lexer, parser, sema, and artifacts. Add Unicode display-width
later inside rendering, not inside core span math.

## Line And Column Lookup

Rendered diagnostics should use one-based line and column values. Internal
offsets remain zero-based.

Lookup policy:

- line starts at `1`
- column starts at `1`
- newline bytes advance the line and reset the column
- CRLF should be normalized or explicitly represented in source-map artifacts
- EOF spans should render at the final valid insertion point
- invalid spans should fail before rendering

The source-map artifact should make this policy reviewable with tiny examples,
including empty files, trailing newlines, multi-line spans, and file-backed
module sources.

## Artifact Policy

Use deterministic artifacts before relying on executable behavior:

| Artifact | Proves |
| --- | --- |
| `--emit-source-map` | Loaded sources, byte lengths, line starts, and display paths. |
| `--emit-tokens` | Token spans and source ownership after lexing. |
| `--emit-syntax` | AST spans and source-shaped parser output. |
| `--emit-declaration-index` | Declaration spans, names, visibility, and module ownership. |
| `--emit-module-graph` | File-backed module sources and import edges. |
| `--emit-diagnostics` | Diagnostic labels, line/column lookup, and normalized paths. |

Put source identity goldens under
`tests/cases/compiler-development/artifact/ok/` or
`tests/cases/compiler-development/artifact/errors/` depending on whether the
artifact represents a valid source map or an expected diagnostic.

## Implementation Slices

Use these slices for normal compiler work:

1. Source table: one owner for loaded source bytes, source kind, display path,
   and stable ids.
2. Span validation: reject invalid source ids, negative byte positions,
   reversed ranges, and out-of-bounds ranges.
3. Line table: derive line starts from bytes and expose byte-column lookup.
4. Snippet extraction: return source lines and caret ranges without formatting
   the whole diagnostic.
5. Artifact normalization: keep paths, generated ids, and line endings stable.
6. Multi-file coverage: include root plus file-backed module sources.
7. Diagnostic integration: labels should carry spans instead of only strings.

Each slice should land with one focused fixture or golden artifact.

## Review Checklist

Before handing off source identity work, answer:

- Which source kind does the change affect?
- Does the entry file still receive a deterministic first id?
- Are generated or virtual sources represented explicitly?
- Are spans byte-based, inclusive/exclusive, and bounds-checked?
- Does line/column rendering stay one-based for users?
- Is the source-map, token, syntax, declaration, module, or diagnostic artifact
  the closest proof?
- Did diagnostics improve without moving compiler-only APIs into runtime `std`?

Strong source identity moves Ari toward a real compiler by making every later
stage reviewable. It should help all Ari tools, not just a future self-hosting
track.
