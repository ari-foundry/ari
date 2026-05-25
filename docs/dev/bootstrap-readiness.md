# Bootstrap Readiness

This page answers one practical question: when can Ari start a real
compiler-in-Ari bootstrap track?

It complements [Production Compiler Design](production-compiler-design.md),
[Compiler Maturity Gates](compiler-maturity-gates.md),
[Compiler Pass Contracts](compiler-pass-contracts.md),
[Compiler Project Model](compiler-project-model.md),
[Compiler Source And Diagnostics](compiler-source-diagnostics.md),
[Compiler Artifact Testing](compiler-artifact-testing.md),
[Compiler Bootstrap Fixture Plan](bootstrap-fixture-plan.md), and
[Self-Host Roadmap](self-host-roadmap.md). The production design page defines
the ordinary language/compiler contract Ari needs; the maturity gates define
the active compiler-development checklist; the pass contracts define the
lexer/parser/resolver/sema/IR/backend boundaries; the project model defines the
file-backed module, package-root, metadata, cache, and Makefile shape; the
source/diagnostic page defines the tooling layer that blocks good lexer and
parser work; the artifact testing page defines comparison order and
normalization policy; the fixture plan defines the pre-bootstrap test groups;
the self-host roadmap explains the long path from stage0 to stage2; this page
is the entry gate for starting that work without turning the current C++
compiler into a second rewrite project.

For active compiler work, use
[Compiler Development Roadmap](compiler-development-roadmap.md) and
[Compiler Readiness Inventory](compiler-readiness-inventory.md). This page is a
readiness gauge, not a request to implement bootstrapping now.

## Current Estimate

The active start-readiness estimate is now gate-based and lives in
[Compiler Readiness Inventory](compiler-readiness-inventory.md). Do not use the
old single-number self-host estimate or a deleted readiness fixture as the current
source of truth.

This gate is about the hosted compiler core only: source identity, diagnostics,
module/project flow, frontend reliability, compiler-shaped data models,
trait/generic readiness, artifact comparison, and large multi-file Ari project
behavior. It deliberately excludes stdlib/library maturity, an Ari-written
package/build tool, and any actual Ari compiler rewrite.

The remaining work should be treated as normal production compiler work, not as
bootstrap-only exceptions. Source-coordinate, source-map, and diagnostic
builder APIs belong in compiler/tooling layers, not in runtime `std`.
A complete self-hosting compiler should wait until the start gate below is
green.

## Bootstrap Start Gate

Start the first real Ari compiler-in-Ari track only when these are all true:

| Gate | Required State | Why It Matters |
| --- | --- | --- |
| Source identity | The C++ hosted compiler has stable `SourceId`, byte spans, line/column lookup, snippets, EOF behavior, CRLF behavior, imported-file ids, and duplicate-source handling. | Lexer/parser diagnostics need exact source spans. |
| Error reporting | The hosted compiler emits stable codes, labels, notes, help, source ids, and normalized golden diagnostics for common user errors. | Golden tests need comparable errors before the parser grows. |
| File modules | The hosted compiler loads file-backed modules in a predictable project shape and reports missing, private, cyclic, duplicate, stale-cache, and imported-file errors cleanly. | A compiler cannot stay a single file for long. |
| Frontend reliability | Lexer and parser reject malformed input with stable recovery, spans, and syntax/diagnostic artifacts. | Future compiler work depends on trustworthy frontend failures. |
| Compiler data | Ari can express small compiler-shaped source, token, diagnostic, symbol, and result values using normal structs, enums, generics, traits, and ownership rules. | AST/HIR and name resolution need predictable data models. |
| Traits and generics | Generic aggregates, generic functions, trait selection, method resolution, and monomorphization work for compiler-shaped data without name-specific shortcuts. | Compiler data naturally uses nested generic containers and trait bounds. |
| Artifact comparison | Token, source-map, syntax, diagnostic, module graph, declaration, typed IR, pass summary, LLVM fragment, symbol, and runtime-output artifacts are deterministic enough for focused review. | Each compiler layer must be reviewable in isolation. |
| No private shortcuts | Any C++ change is a normal language/compiler fix, not private syntax or a hidden helper for a future Ari compiler. | Keeps the future rewrite honest and maintainable. |

The recommended first green target is not "self compile". It is still a
hosted-compiler proof:

```text
the C++ hosted compiler emits stable source, token, syntax, diagnostic,
module, typed-IR, and backend artifacts for focused fixtures
```

## Needed Language Work

These are the highest-value language and compiler features before broad
bootstrapping:

1. File-backed module ergonomics: stable search paths, package roots, module
   metadata, and clear errors for missing or private items.
2. Generic aggregate maturity: fewer special cases for nested generic structs,
   enums, vectors, maps, and Result-like payloads.
3. Trait ergonomics: predictable static dispatch for formatting, hashing,
   equality, ordering, and collection defaults.
4. Source maps: build filename/text ownership, byte spans, line/column lookup,
   and a persistent source-map owner in a compiler/tooling package instead of
   runtime `std`.
5. Error values: grow the current compact `Error` values into `Result[T, E]`
   workflows that avoid panic in expected failure paths, and keep compiler
   error-report values in compiler/tooling packages rather than runtime `std`.
6. More natural text APIs: keep reducing awkward casts and helper suffixes in
   code that manipulates source bytes, chars, and Unicode boundaries.
7. Better build surfaces: the existing Make/test runner is enough for this
   readiness gate; an Ari-written build/package tool is a later ecosystem task.
8. Production compiler contract: keep the language changes public and useful to
   ordinary Ari projects, as described in
   [Production Compiler Design](production-compiler-design.md).

## Needed Standard Library Work

Standard library maturity is intentionally out of scope for the current
compiler-writing readiness score. Keep stdlib/library work in the library
roadmap and test targets. This page may mention text, collections, or zones as
future compiler needs, but do not count library progress as a start-gate
increase.

For this gate, prefer hosted compiler fixes and focused compiler artifacts over
new library APIs. Avoid threads, sockets, dynamic loading, package-manager work,
freestanding startup, async IO, arbitrary plugin loading, and optimizer work.

## Proposed Bootstrap Project Shape

Do not create this tree during readiness work. Keep the shape here only as a
future reference for the day the lexer milestone starts:

```text
bootstrap/
  README.md
  Makefile
  stage1/
    src/
      main.ari
      source/
      lex/
      report/
      syntax/
      parse/
      hir/
      resolve/
      types/
      lower/
      emit/
    tests/
      fixtures/
      lex/
      parse/
      report/
      hir/
      golden/
```

The first future `Makefile` should only call `build/ari`, run a focused tool,
and compare output. A package manager can replace this later; readiness work
should not build one now.

## Roadmap To Start

### Phase A: Bootstrap Docs And Gates

- Keep this readiness document updated.
- Keep [Self-Host Roadmap](self-host-roadmap.md) aligned with this start gate.
- Add focused checks that fail when required sections disappear.

Exit criteria:

- New contributors can tell the difference between "can write Ari tools now"
  and "can self-host now".

### Phase B: Source And Error Reporting Foundations

- Build owned source-map storage in a compiler/tooling package with byte spans,
  line/column lookup, and filename/text ownership.
- Build compiler error reporting as a separate builder and stable renderer
  instead of adding bootstrap-only APIs to `std`.
- Add golden tests for line/column rendering, multiple notes, and labels.

Exit criteria:

- An Ari tool can report a lexical error with a stable span and message.

### Phase C: Lexer Pilot

- Implement a lexer as a standalone Ari tool.
- Cover identifiers, keywords, numbers, char/string literals, comments,
  operators, punctuation, whitespace, and invalid input recovery.
- Compare token dumps and diagnostic dumps.

Exit criteria:

- The lexer can tokenize selected current `lib/std/*.arih` files without
  crashing, even if later parser features are incomplete.

### Phase D: Parser Pilot

- Parse declarations, functions, modules, statements, expressions, patterns,
  and types into a syntax tree.
- Emit a stable syntax dump.
- Keep recovery behavior deterministic enough for tests.

Exit criteria:

- The parser can parse a focused standard-library subset and reject malformed
  fixture files with stable diagnostics.

### Phase E: HIR And Name Resolution

- Lower syntax to HIR.
- Resolve module paths, imports, visibility, and symbols.
- Add symbol table and interned-name helpers.

Exit criteria:

- Stage1 can build a module graph for its lexer/parser source and selected
  standard-library files.

### Phase F: Type And Ownership Subset

- Type-check the subset used by stage1.
- Add ownership checks for zones, strings, vectors, maps, diagnostics, and AST
  nodes.

Exit criteria:

- Stage1 can type-check its own lexer/parser/HIR modules.

### Phase G: Backend Artifact

- Emit normalized HIR/IR text first.
- Add LLVM text only after frontend comparisons are stable.

Exit criteria:

- Stage0-built stage1 and later stage1-built stage2 can compare artifacts.

## Test Plan

Bootstrap tests should be grouped by artifact, not by compiler phase name
alone:

| Folder | Test Type |
| --- | --- |
| `bootstrap/stage1/tests/fixtures/` | Shared `.ari` inputs used by multiple stage tools. |
| `bootstrap/stage1/tests/lex/ok/` | Valid source files and expected token dumps. |
| `bootstrap/stage1/tests/lex/errors/` | Invalid source files and expected lexical diagnostics. |
| `bootstrap/stage1/tests/parse/ok/` | Valid source files and expected syntax dumps. |
| `bootstrap/stage1/tests/parse/errors/` | Invalid source files and expected parser diagnostics. |
| `bootstrap/stage1/tests/report/` | Source-map and error rendering fixtures. |
| `bootstrap/stage1/tests/golden/` | Checked text outputs committed for review. |

Each test should say what it covers in the filename:

- `lex-string-escapes.ari`
- `lex-nested-comments.ari`
- `parse-function-decls.ari`
- `parse-match-patterns.ari`
- `report-multiline-span.ari`

Do not begin with a single giant "compile Ari" test. The first tests should
compile and run in seconds.

## Stage Comparison Policy

Before real self-hosting, define this comparison order:

1. Compare token dumps.
2. Compare syntax dumps.
3. Compare HIR dumps.
4. Compare typed IR dumps.
5. Compare LLVM text after normalizing symbol ordering, file paths, and
   temporary names.
6. Compare executable behavior only after the frontend artifacts match.

This keeps regressions understandable. If stage1 and stage2 differ, the failing
artifact tells which compiler layer changed.

## What Not To Do Yet

- Do not start by porting `src/sema.cpp`.
- Do not make stage0-only syntax for stage1.
- Do not depend on threads, sockets, dynamic loading, or freestanding startup.
- Do not hide allocation behind an implicit global heap.
- Do not begin with optimization.
- Do not treat "can compile a lexer" as "self-hosted".

The next useful work is boring on purpose: make the source, span, diagnostic,
and lexer layers so clear that every later compiler phase can stand on them.
