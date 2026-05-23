# Compiler Source And Diagnostics

This page defines the compiler/tooling source-map and diagnostic layer Ari uses
for precise errors and stable artifacts. The current C++ compiler has the
production-ready baseline for the supported surface: loaded sources receive
`SourceId`s, tokens and AST locations retain byte spans, common lexer, parser,
module, semantic, trait, and ownership errors render with source-backed labels,
and golden diagnostic artifacts record deterministic source data.

Do not put these APIs into runtime `std`. Runtime `std` should keep broadly
useful facilities such as strings, formatting, paths, files, logging, tests,
and errors. Source maps, labels, fix-its, and golden compiler reports belong in
a compiler/tooling package.

Read this page with:

- [Compiler Maturity Gates](compiler-maturity-gates.md)
- [Compiler Development Roadmap](compiler-development-roadmap.md)
- [Compiler Source Identity](compiler-source-identity.md)
- [Compiler Diagnostic Authoring](compiler-diagnostic-authoring.md)
- [Production Compiler Design](production-compiler-design.md)

## Goals

The source/diagnostic layer makes compiler errors precise, stable, and pleasant
to maintain. Its renderer owns stable golden rendering for compiler tests.

The supported C++ layer provides:

- stable source identity through `SourceId`
- owned source files through `SourceFile`
- byte-based spans through `Span`
- line and column lookup from byte offsets
- source snippets for rendered diagnostics
- diagnostic codes and severity
- primary and secondary labels
- notes and help text
- deterministic rendering for golden tests

Future Ari tooling packages may add fix-it edits, color, richer multi-primary
diagnostics, and LSP-specific projections. Those are extensions to this source
contract, not replacements for it.

It should not provide:

- runtime panic policy
- general application logging
- stack traces or backtraces
- private compiler-only shortcuts
- hidden global source storage
- implicit allocation from a magical heap

## Package Shape

Recommended future package layout:

```text
compiler/
  source/
    id.ari          SourceId and source collection handles
    file.ari        SourceFile, filename, source text ownership
    span.ari        Span, BytePos, line/column helpers
    map.ari         SourceMap owner and lookup APIs
  report/
    severity.ari    Error, Warning, Note, Help
    code.ari        diagnostic code wrappers
    label.ari       primary/secondary labels
    diagnostic.ari  Diagnostic value and builders
    render.ari      stable text rendering
    fixit.ari       replacement/insert/delete suggestions
  artifact/
    normalize.ari   path and symbol normalization for golden text
    compare.ari     expected/actual comparison helpers
```

The first implementation can live in a smaller folder. The important boundary
is that `compiler/source` owns source coordinates and `compiler/report` owns
diagnostic values and rendering.

## Core Types

The exact Ari syntax can evolve, but the concepts should stay stable.

```ari
type BytePos = i64;

struct SourceId {
  index: i64,
}

struct Span {
  source: SourceId,
  start: BytePos,
  end: BytePos,
}

struct LineColumn {
  line: i64,
  column: i64,
}

struct SourceFile {
  id: SourceId,
  path: std::string::OsStr,
  display_name: std::string::String,
  text: std::string::String,
  line_starts: std::vec::Vec[BytePos],
  eof_offset: BytePos,
}

struct SourceMap {
  // add_file(path, text) -> SourceId
  // get(id) -> SourceFile
  // span(id, start, end) -> Span
  // valid_span(span) -> bool
  // location(id, byte_offset) -> LineColumn
  // byte_offset(id, line, column) -> Option[BytePos]
  // snippet(span) -> SourceSnippet
}

struct SourceSnippetLine {
  line_number: i64,
  byte_start: BytePos,
  text: std::string::String,
  marker_start: BytePos,
  marker_len: BytePos,
  has_marker: bool,
  truncated_start: bool,
  truncated_end: bool,
}

struct SourceSnippet {
  span: Span,
  start: LineColumn,
  source_name: std::string::String,
  lines: std::vec::Vec[SourceSnippetLine],
  context_lines: i64,
  truncated: bool,
  valid: bool,
}
```

Policy:

- `Span.start` is inclusive.
- `Span.end` is exclusive.
- spans are byte ranges, not Unicode scalar ranges
- empty spans are valid insertion points and EOF locations
- line and column lookup is derived from source text
- diagnostic line and column values are one-based byte columns
- internal source coordinates are zero-based byte offsets
- `\r\n` is one line break; snippets hide the `\r` byte and both newline bytes
  map to the previous line's final visible column
- UTF-8 does not change core column math yet: multi-byte scalars count by bytes,
  not display cells or UTF-16 code units
- `SourceFile` owns the line start table and EOF offset so diagnostics do not
  rescan source text for every lookup
- file-backed sources may use different canonical `path` and diagnostic
  `display_name` values
- columns should start as byte columns; Unicode display-width policy can come
  later
- `SourceId` is stable within one compiler invocation, not across builds
- generated sources should still receive a `SourceId`
- invalid/null source is represented by a sentinel id and must not be used for
  a user-facing diagnostic span
- `SourceMap` is the only component that should need source text to answer
  line/column or snippet questions
- `SourceSnippet` is source-map output. It owns rendered line slices and marker
  byte ranges, while the diagnostic renderer owns gutter text, severity text,
  notes, and color policy.
- `SourceSnippet.valid=false` is the missing-source fallback. Renderers should
  omit the source excerpt instead of inventing a misleading file or byte range.
- C++ tokens store `Span` directly; AST/IR nodes use `SourceLocation`, whose
  `loc.span` is the same canonical `Span` plus cached rendering coordinates
- use `merge_spans`, `span_contains`, `span_intersects`, and
  `SourceMap::valid_span` instead of open-coded byte arithmetic

This byte-first policy keeps lexer and parser code simple. Unicode-aware
display width can be added to the renderer without changing the compiler's
internal span math.

## Diagnostic Values

Diagnostics should be data first and rendering second. In practice, this means
diagnostics should be data first at every compiler layer, with rendering kept at
the edge.

```ari
enum Severity {
  Error,
  Warning,
  Note,
  Help,
}

struct DiagnosticCode {
  family: std::string::String,
  number: i64,
}

struct Label {
  span: Span,
  message: std::string::String,
  primary: bool,
}

struct Diagnostic {
  severity: Severity,
  code: std::Option[DiagnosticCode],
  message: std::string::String,
  labels: std::vec::Vec[Label],
  notes: std::vec::Vec[std::string::String],
}
```

Policy:

- parser and sema should create diagnostics, not print them directly
- diagnostics may contain multiple labels
- each label's `Span` carries its own `SourceId`, so multi-file diagnostics are
  represented without borrowing the primary label's file identity
- every common user-facing error must retain source identity through `SourceId`
  or a registered source path
- artifact rows for located diagnostics include severity, diagnostic code,
  family, message, source id, source path, byte start/end, line, column,
  end-line, end-column, label role, notes/help, and snippets when source text is
  available
- line and column values in user-facing output are one-based byte columns; byte
  spans remain zero-based and half-open
- labels, notes, and help entries keep deterministic insertion order
- stage0 diagnostics should preserve the original lexer/parser `SourceLocation`
  through module loading and sema. `CompileError(SourceLocation, message)` is
  the current bridge; `CompileError(where(loc) + ": ...")` is only for
  transitional helpers that still accept text.
- one diagnostic should have at most one primary label unless a future design
  explicitly supports multiple primaries
- notes are ordered and deterministic
- diagnostic codes should be stable once published
- rendering should be tested as text output, not by manually inspecting stderr

## Rendering Policy

Rendered diagnostics should be deterministic so tests can compare golden files.

The first renderer should print:

```text
error[E0001]: expected expression
  --> path/to/file.ari:4:12
   |
 4 |   let x =
   |           ^ expected expression here
   |
   = note: Ari requires a value after `=`
```

Initial rules:

- normalize repository-local paths in golden tests
- use one-based line and column numbers for users
- keep internal byte offsets zero-based
- request source excerpts through `SourceMap::snippet` instead of slicing
  source text inside diagnostic code
- keep zero context lines for compact default diagnostics; request explicit
  context only when a renderer or artifact needs surrounding lines
- render every marked line in a multi-line span with its own caret range
- render empty spans and EOF spans with one caret at the insertion point
- preserve tabs in marker padding so caret alignment follows the line text
- respect snippet truncation flags and keep `...` in golden output for long
  lines
- omit snippets gracefully when `SourceSnippet.valid=false`
- sort diagnostics by first primary span, then insertion order
- keep note order exactly as emitted
- avoid terminal color in golden output
- write color support later as a renderer option

## Error Code Policy

Diagnostic codes should help users search and help maintainers classify
regressions.

Recommended families:

```text
L0001 lexer
P0001 parser
M0001 modules
T0001 types and traits
O0001 ownership and borrowing
I0001 IR lowering
B0001 backend and artifact emission
```

Rules:

- do not reuse a code for a different rule
- do not require every early prototype diagnostic to have a code
- add codes first for stable, user-facing errors
- document code families before adding many individual codes

Current transitional bridge:

- `--emit-diagnostics` still catches string-based `CompileError` values.
- `classify_diagnostic_code` maps common messages into the first stable
  families: `L0001`, `P0001`, `M0001`, `T0001`, `O0001`, `I0001`, and `B0001`.
  Lexer literal/comment/escape failures stay in `L0001`, parser grammar
  failures stay in `P0001`, and the covered user-facing semantic patterns
  include unknown names, duplicate declarations, wrong argument counts,
  assignment errors, return errors, type mismatches, private visibility access,
  trait failures, generic inference and explicit type-argument failures,
  aggregate literal/field failures, enum constructor/payload failures, match
  pattern validation failures, non-exhaustive enum matches, and ownership
  failures. The checked diagnostic artifact set should not contain
  `ari/compiler` fallback rows for common source-level errors.
- `diagnostic_code_family` renders the owning layer name, such as
  `family=parser`, next to the stable code in diagnostic artifacts.
- Diagnostic artifacts render `Source`, `Label`, `Snippet`, `Note`, and `Help`
  rows instead of requiring tools to parse prose. Located rows carry
  `source_id=`, `source=`, `line=`, `column=`, `end_line=`, `end_column=`,
  `byte_start=`, and `byte_end=` fields.
- Message-only notes and help render as `location=none`; located notes and help
  render as `location=source` plus the same span fields as labels.
- Diagnostic artifacts suppress structured source fields when an error has no
  valid source id, so a byte range without source ownership does not flow into
  golden diagnostics.
- `--emit-diagnostic-catalog` renders the current code, family, owner, and
  fallback table from compiler code.
- `--list-diagnostics` renders the same catalog without requiring a source
  input file.
- `--explain-diagnostic P0001` renders one code's family, source owner, first
  check, and artifact route for triage.
- Unknown messages keep the fallback `ari/compiler` code so tools remain
  compatible for source-less driver/backend/internal failures while individual
  diagnostics move to explicit codes.
- This bridge is for artifact stability and compatibility. New diagnostics
  should prefer data-first construction at lexer, parser, resolver, sema,
  ownership, IR, or backend throw sites.

## Current Integration Audit

The current compiler paths are classified as follows.

| Path | Status | Contract |
| --- | --- | --- |
| `SourceId`, `SourceFile`, `Span`, `SourceMap`, `SourceLocation` | already SourceId/Span-backed | `src/common.*` owns source registration, line tables, EOF offsets, byte-span helpers, line/column lookup, CRLF handling, UTF-8 byte-column policy, snippet extraction, and missing-source fallback. |
| Lexer diagnostics | already SourceId/Span-backed | Tokens and lexer errors carry spans. `diagnostic-unexpected-character.diagnostic` and `diagnostic-lexer-unterminated-string.diagnostic` verify code, source id/path, byte span, line/column, label, and snippet. |
| Parser diagnostics | already SourceId/Span-backed | Expected/unexpected token failures use token spans. `diagnostic-parser-expected.diagnostic` and `diagnostic-parser-top-level.diagnostic` are the representative goldens. |
| Module/import/visibility diagnostics | already SourceId/Span-backed for common paths | Missing modules, ambiguous module candidates, cyclic imports, and private access preserve the source of the import or qualified access. `diagnostic-missing-module.diagnostic`, `diagnostic-ambiguous-module.diagnostic`, `diagnostic-cyclic-module.diagnostic`, and `diagnostic-private-access.diagnostic` verify the artifact rows. |
| Semantic/type/name diagnostics | already SourceId/Span-backed for common paths | Unknown names, duplicate declarations, type mismatch, wrong arity, wrong argument type, invalid return, invalid assignment, non-struct field access, unknown struct fields, match arm type mismatches, and non-exhaustive enum matches are covered by focused diagnostic artifacts, including `diagnostic-struct-field-access-non-struct.diagnostic`, `diagnostic-struct-field-unknown.diagnostic`, `diagnostic-match-expression-type-mismatch.diagnostic`, and `diagnostic-match-nonexhaustive.diagnostic`. |
| Trait diagnostics | already SourceId/Span-backed for the minimum subset | Unknown trait, missing impl, ambiguous method, duplicate/wrong impl checks flow through semantic locations and golden diagnostics. |
| Ownership/borrow/drop diagnostics | already SourceId/Span-backed for supported ownership checks | Borrow conflict, use-after-move, live-owner return/break/continue exits, moving borrowed owners, invalid enum payload moves, compact enum payload ref-pattern rejection, ownership- or borrow-carrying aggregate enum payload rejection, partial moves, and unsupported dynamic or temporary owner-element moves point at the expression, binding, payload pattern, type payload, or control-flow keyword span; `diagnostic-borrow-conflict.diagnostic`, `diagnostic-use-after-move.diagnostic`, `diagnostic-return-live-owner.diagnostic`, `diagnostic-loop-break-live-owner.diagnostic`, `diagnostic-loop-continue-live-owner.diagnostic`, `diagnostic-move-borrowed-owner.diagnostic`, `diagnostic-enum-payload-invalid-move.diagnostic`, `diagnostic-compact-enum-payload-ref.diagnostic`, `diagnostic-ownership-aggregate-enum-payload.diagnostic`, `diagnostic-borrow-aggregate-enum-payload.diagnostic`, `diagnostic-ownership-partial-move.diagnostic`, `diagnostic-ownership-vector-dynamic-move.diagnostic`, and `diagnostic-ownership-temporary-element-move.diagnostic` are the golden fixtures. |
| `CompileError(SourceLocation, message)` | structured bridge | Keeps `SourceId`, byte range, cached line/column, primary label, and snippet. This is the preferred C++ bridge while the compiler still throws `CompileError`. |
| `CompileError(where(loc) + text)` | SourceLocation-backed transitional path | Older helpers that accept strings still parse structured `:bytes=start..end` prefixes back into source-aware labels. Keep this path only while migrating call sites. |
| String-only diagnostics | justified fallback | CLI misuse, missing input files, artifact option conflicts, backend/toolchain failures, and internal generated errors may be source-less because no user source span exists. They use `ari/compiler`, `B0001`, or another layer code as appropriate. Common source-level lexer, parser, module, type, trait, generic, aggregate, match, and ownership errors must not use the `ari/compiler` fallback. |
| Deferred features | intentionally unsupported | Fix-its, terminal color, LSP display-width columns, multi-primary diagnostics, and a fully data-first non-exception pipeline remain future tooling work. |

## Ownership And Allocation

Source text and diagnostic data should use explicit ownership.

Recommended policy:

- long-lived source text lives in a compiler-owned arena or source map owner
- spans store `SourceId` and byte offsets, not borrowed string slices
- source owners store canonical path, display name, line start offsets, and EOF
  offset together with the text
- diagnostics that point at source must carry the same `SourceId` as the token,
  AST node, or generated source they describe
- generated and built-in diagnostics should point into registered
  `Generated`/`Builtin` sources; they should not reuse an unrelated file span
- diagnostics own their messages or allocate them in a report arena
- temporary render buffers can use a scratch zone
- golden comparisons should not depend on pointer identity or allocation order

Avoid a global source map. Passing the source map owner explicitly makes tests
and future parallel tools easier to reason about.

## Implementation Slices

Land this layer in small slices:

| Slice | Deliverable | Focused Tests |
| --- | --- | --- |
| SourceId | Stable source ids and file registration. | `source-id-stability`, duplicate path handling. |
| SourceFile | Canonical path, display name, owned text, line table, EOF offset, and in-memory source registration. | `tests/source_map_unit.cpp`, `source-map-file-module.map`. |
| SourceMap | `add_file`, `get`, `span`, `location`, and `snippet` APIs over one source owner. | `make check-source-map-unit`, multi-file SourceMap smoke tests. |
| Span | Byte range construction, validation, merge, contains, and intersects helpers. | empty span, single-byte span, end-before-start normalization/rejection, source mismatch merge. |
| Line lookup | Byte offset to line/column mapping and one-based line/column back to byte offset. | empty file, one-line, multi-line, EOF, CRLF, UTF-8 byte-column policy in `tests/source_map_unit.cpp`. |
| Source map artifact | Deterministic source ids, kind, canonical/display paths, EOF offsets, line tables, byte, line, and snippet text. | `source-map-file-module.map`, CRLF policy. |
| Snippets | Extract source lines, context lines, underline ranges, and truncation flags. | single-line, multi-line, empty span, EOF span, tab policy, long line, missing source. |
| Diagnostic values | Severity, code, label, note data structures. | label ordering, note ordering, optional code. |
| Renderer | Stable plain-text diagnostic rendering. | single label, multi-label, notes, path normalization. |
| Golden runner | Compare expected and actual text outputs. | mismatch report, update policy documentation. |

Keep each slice independently testable. Do not wait for a full lexer to test
line lookup or label rendering.

## Test Layout

Keep source/diagnostic fixtures under compiler-development artifact or feature
folders:

```text
tests/cases/compiler-development/artifact/ok/
tests/cases/compiler-development/artifact/errors/
tests/cases/<feature>/errors/
```

Test names should describe behavior:

- `source-id-stability.ari`
- `span-byte-range.ari`
- `line-column-lookup.ari`
- `report-single-label.ari`
- `report-multiline-span.ari`
- `report-note-order.ari`
- `report-path-normalization.ari`

## Integration With The Current Compiler

The current C++ compiler treats source-aware diagnostics as the normal path for
source-level failures:

- use `--emit-source-map` to review source files, byte offsets, line lengths,
  newline kind, and snippets before lexer or parser output is involved
- keep frontend diagnostics specific and anchored to the token, AST node,
  declaration, import, or expression that caused the failure
- avoid backend diagnostics for source-level mistakes
- move repeated diagnostic spelling into helpers when the same rule appears in
  several semantic paths
- keep source spans available through parser, sema, trait, ownership, and IR
  data instead of re-resolving source names later
- add or refresh a golden diagnostic artifact when a user-facing diagnostic is
  introduced or its source span changes
- document any remaining source-less diagnostic as driver, backend, generated,
  internal, or deferred

This means source identity is part of the compiler contract today. The future
tooling package shape can make diagnostics nicer to author, but it should not
weaken the current `SourceId`/`Span` artifact guarantees.

## Readiness Impact

SourceMap and diagnostic integration is production-ready for the current
compiler surface. The protected contract covers:

- SourceId assignment, lookup, replacement, generated sources, and invalid
  source fallback
- byte spans, EOF spans, empty files, multi-line files, CRLF files, UTF-8 byte
  columns, context snippets, long-line snippet truncation, and snippet
  extraction
- structured `CompileError(SourceLocation, message)` diagnostics and the
  transitional `where(loc)` string bridge preserve `SourceId`, byte spans,
  labels, and snippets
- lexer, parser, module, semantic, trait, and ownership representative
  diagnostics
- deterministic diagnostic artifacts with source rows, labels, byte ranges,
  line/column data, snippets, notes, and help rows where available, including
  a repeated lexer diagnostic artifact comparison for deterministic rendering

Remaining work is intentionally scoped to future polish rather than correctness:
individual rule-specific code expansion beyond the first stable families,
fix-its, color, LSP column projections, richer multi-label authoring helpers,
and continued migration away from transitional string constructors.
