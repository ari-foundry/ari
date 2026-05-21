# Compiler Source And Diagnostics

This page defines the compiler/tooling source-map and diagnostic layer Ari
needs for precise errors, stable artifacts, lint, LSP, formatter, and package
tools.

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

The source/diagnostic layer should make compiler errors precise, stable, and
pleasant to maintain. Its renderer owns stable golden rendering for compiler
tests.

It should provide:

- stable source identity through `SourceId`
- owned source files through `SourceFile`
- byte-based spans through `Span`
- line and column lookup from byte offsets
- source snippets for rendered diagnostics
- diagnostic codes and severity
- primary and secondary labels
- notes and help text
- optional fix-it edits
- deterministic rendering for golden tests

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
  text: std::string::String,
}
```

Policy:

- `Span.start` is inclusive.
- `Span.end` is exclusive.
- spans are byte ranges, not Unicode scalar ranges
- line and column lookup is derived from source text
- columns should start as byte columns; Unicode display-width policy can come
  later
- `SourceId` is stable within one compiler invocation, not across builds
- generated sources should still receive a `SourceId`

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
- `diagnostic_code_family` renders the owning layer name, such as
  `family=parser`, next to the stable code in diagnostic artifacts.
- Diagnostic artifacts render locations as `source=`, `line=`, and `column=`
  fields instead of requiring tools to parse prose.
- `--emit-diagnostic-catalog` renders the current code, family, owner, and
  fallback table from compiler code.
- `--list-diagnostics` renders the same catalog without requiring a source
  input file.
- `--explain-diagnostic P0001` renders one code's family, source owner, first
  check, and artifact route for triage.
- Unknown messages keep the fallback `ari/compiler` code so tools remain
  compatible while individual diagnostics move to explicit codes.
- This bridge is only for artifact stability. The long-term design is still
  data-first diagnostics created at lexer, parser, resolver, sema, ownership,
  IR, or backend throw sites.

## Ownership And Allocation

Source text and diagnostic data should use explicit ownership.

Recommended policy:

- long-lived source text lives in a compiler-owned arena or source map owner
- spans store `SourceId` and byte offsets, not borrowed string slices
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
| Span | Byte range construction and validation. | empty span, single-byte span, end-before-start rejection. |
| Line lookup | Byte offset to line/column mapping. | start, middle, newline, EOF, CRLF policy. |
| Source map artifact | Deterministic file, byte, line, and snippet text. | `source-map-file-module.map`, CRLF policy. |
| Snippets | Extract source line and underline span. | single-line, empty span, tab policy, EOF span. |
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

The current C++ compiler should improve in parallel:

- use `--emit-source-map` to review source files, byte offsets, line lengths,
  newline kind, and snippets before lexer or parser output is involved
- keep frontend diagnostics specific
- avoid backend diagnostics for source-level mistakes
- move repeated diagnostic spelling into helpers
- keep source spans available through parser and sema data
- add stable substrings or golden output when diagnostics become user-facing

This means the current compiler should produce better diagnostics while the
tooling package shape is designed.

## Readiness Impact

This layer is one of the largest remaining blockers. It affects:

- lexer diagnostics
- parser recovery
- module errors
- type and trait errors
- ownership errors
- artifact comparison
- LSP and lint integration

This layer becomes mature when source ids, spans, line lookup, labels, notes,
and plain-text golden rendering are all covered by focused artifacts.
