# Bootstrap Readiness

This page answers one practical question: when can Ari start a real
compiler-in-Ari bootstrap track?

It complements [Production Compiler Design](production-compiler-design.md),
[Compiler Maturity Gates](compiler-maturity-gates.md),
[Compiler Bootstrap Fixture Plan](bootstrap-fixture-plan.md), and
[Self-Host Roadmap](self-host-roadmap.md). The production design page defines
the ordinary language/compiler contract Ari needs; the maturity gates define
the active compiler-development checklist; the fixture plan defines the
pre-bootstrap test groups; the self-host roadmap explains the long path from
stage0 to stage2; this page is the entry gate for starting that work without
turning the current C++ compiler into a second rewrite project.

For active compiler work, use
[Compiler Development Roadmap](compiler-development-roadmap.md). This page is a
readiness gauge, not a request to implement bootstrapping now.

## Current Estimate

As of the current hosted compiler and standard library, Ari is roughly:

- **38-42% ready to start full compiler bootstrapping**
- **58-62% remaining before a self-host attempt is likely to be productive**

This estimate is about practical implementation readiness, not language
ambition. Ari already has many pieces needed by a compiler: modules, structs,
enums, traits, generics, zones, strings, vectors, maps/sets, formatting,
filesystem IO, process/environment helpers, logging/error helpers, and an
LLVM-backed executable pipeline. The remaining work should be treated as
general production language and compiler design, not as bootstrap-only
exceptions. Source-coordinate, source-map, and diagnostic builder APIs do not
belong in production `std`; they need to exist as a compiler/tooling-local
layer before the lexer/parser bootstrap track starts. The missing work is
mostly around scale, ergonomics, owned source text maps, diagnostic rendering,
stable compiler data structures, multi-file project flow, and comparison
tooling.

Small Ari-written compiler components can start now. A complete self-hosting
compiler should wait until the start gate below is green.

## Bootstrap Start Gate

Start the first real `bootstrap/` tree only when these are all true:

| Gate | Required State | Why It Matters |
| --- | --- | --- |
| Source text | Ari can read source files and validate UTF-8; a compiler/tooling-local source-map layer still needs byte spans and line/column lookup. | Lexer/parser diagnostics need exact source spans. |
| Error reporting | Runtime `std` only supplies logging, panic, formatting, errors, and test helpers; compiler tools still need their own source spans, labels, report builders, and stable golden renderers. | Golden tests need comparable errors before the parser grows. |
| Strings | `String`, string slices, ASCII, UTF-8, split/search/join, and C/OS/path string boundaries are documented and tested. | Compiler frontend code is mostly text handling. |
| Collections | `Vec`, `Slice`, maps, sets, iterators, and common algorithms are stable enough for syntax trees and symbol tables. | AST/HIR and name resolution need predictable containers. |
| File modules | Ari can load file-backed modules in a predictable project shape without special one-off flags. | A compiler cannot stay a single file for long. |
| Result/errors | `Result[T, E]` and small error structs are comfortable enough for parse and IO error propagation. | Stage1 should not use panic for ordinary compiler errors. |
| Zone policy | A documented arena policy exists for AST, HIR, interned strings, diagnostics, and per-test scratch data. | Self-host data lives longer than toy examples. |
| Test runner | Focused fixtures can run stage tools and compare stdout/stderr/golden files. | Each compiler layer must be reviewable in isolation. |
| Backend strategy | Stage1 has an initial artifact target: token dump, syntax dump, HIR dump, normalized IR, then LLVM text. | Stage1 should not wait for full executable output. |
| No stage0 hacks | Any stage0 C++ change is a normal language/std/runtime fix, not a private shortcut for stage1. | Keeps bootstrap honest and maintainable. |

The recommended first green target is not "self compile". It is:

```text
stage0 builds an Ari lexer tool
the lexer tool reads fixture .ari files
the lexer tool emits stable token and diagnostic output
tests compare that output to golden files
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
7. Better build surfaces: Makefile support is fine for now, but stage1 needs a
   repeatable project layout and per-component fixture targets.
8. Production compiler contract: keep the language changes public and useful to
   ordinary Ari projects, as described in
   [Production Compiler Design](production-compiler-design.md).

## Needed Standard Library Work

The stage1 compiler should start with a conservative hosted subset:

| Area | Minimum Surface |
| --- | --- |
| Text | `String`, `Slice[u8]`, `char`, ASCII helpers, UTF-8 validation/decode, split/search/join, trim, parse integer/bool/float. |
| Collections | `Vec`, `Slice`, `HashMap`, `HashSet`, `TreeMap`, `TreeSet`, iterators, sort, binary search, dedup, copy/fill, and stable comparison helpers. |
| IO/FS | `read`, `try_read`, `write`, `try_write`, `read_dir`, `read_dir_entries`, path join/normalize/canonicalize, current directory, env args. |
| Error reporting | formatting, debug formatting, log output, panic/unreachable messages, test report helpers, and compiler-tooling source spans/maps/report builders/renderers outside production `std`. |
| Memory | explicit `Zone`, temporary zones, copy-to-zone helpers, same-zone container growth, and reset/destroy invalidation checks. |
| Process | command-line args and exit codes; do not require spawn/fork for the first lexer/parser stage. |
| Platform | target facts, pointer sizes, errno policy, and hosted Linux/glibc assumptions documented for stage0. |

Avoid these until the single-threaded hosted compiler path works:

- threads and channels
- sockets
- dynamic loading
- kernel/freestanding APIs
- async IO
- arbitrary plugin loading
- optimizing backend work

## Proposed Bootstrap Project Shape

Create this tree only when the lexer milestone starts:

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

The first `Makefile` should only call `build/ari`, run a focused tool, and
compare output. A package manager can replace this later; bootstrapping should
not wait for one.

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
