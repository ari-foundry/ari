# Compiler Development Roadmap

This page is the day-to-day roadmap for developing the Ari compiler. The job is
to make the current compiler a reliable, pleasant, general-purpose language
implementation: predictable frontend, maintainable semantic analysis, stable
IR, testable LLVM output, clear diagnostics, and ordinary Ari programs that are
comfortable to write.

Read this page for normal compiler work:

- [Compiler Development Dashboard](compiler-development-dashboard.md) gives the
  one-page status, next actions, and small checks.
- [Architecture](architecture.md) explains the current C++ compiler shape.
- [Compiler Pipeline](compiler-pipeline.md) explains the source-to-LLVM path.
- [Compiler Contributor Guide](compiler-contributor-guide.md) is the practical
  edit map and small-test loop for day-to-day compiler changes.
- [Compiler Concepts Glossary](compiler-concepts-glossary.md) explains the
  layer terms, artifact vocabulary, and review language used by the roadmap.
- [Compiler Source Identity](compiler-source-identity.md) defines source files,
  ids, byte spans, line/column lookup, and source-map artifact policy.
- [Compiler Module Project Authoring](compiler-module-project-authoring.md)
  defines how file modules, roots, search paths, metadata, caches, and module
  graph artifacts should be changed.
- [Compiler Artifact Authoring](compiler-artifact-authoring.md) defines how to
  design deterministic artifact producers, goldens, normalization, and review
  rules.
- [Compiler Diagnostic Authoring](compiler-diagnostic-authoring.md) explains
  how to design diagnostic codes, messages, labels, notes, and golden tests.
- [Compiler Test Authoring](compiler-test-authoring.md) explains how to choose
  fixture buckets, file names, focused checks, and artifact update rules.
- [Compiler Implementation Playbook](compiler-implementation-playbook.md)
  turns roadmap items into small implementation tickets with first files,
  artifacts, focused checks, and review criteria.
- [Compiler Next Slices](compiler-next-slices.md) names the near-term tickets
  to pick from while developing the hosted compiler.
- [Compiler Change Checklist](compiler-change-checklist.md) is the handoff
  checklist for docs, tests, diagnostics, sema, IR, and non-goals.
- [Compiler Readiness Inventory](compiler-readiness-inventory.md) lists the
  current strengths, blockers, backlog, and development gates.
- [Compiler Pass Contracts](compiler-pass-contracts.md) defines the pass
  input/output boundaries and review rules that keep compiler code maintainable.
- [Feature Test Matrix](test-matrix.md) tracks feature coverage.
- [Compiler Maturity Gates](compiler-maturity-gates.md) tracks the general
  compiler-development gates that remain before an Ari compiler tree is
  productive.
- [Compiler Project Model](compiler-project-model.md) defines the file-backed
  module, project-root, metadata, cache, and Makefile flow for large Ari tools.
- [Compiler Source And Diagnostics](compiler-source-diagnostics.md) defines
  the source-map and diagnostic tooling layer that should stay outside runtime
  `std`.
- [Compiler Artifact Testing](compiler-artifact-testing.md) defines how
  capability inventory, token, diagnostic, syntax, HIR, typed IR, LLVM,
  object, and executable artifacts are compared.
- [Production Compiler Design](production-compiler-design.md) is a long-term
  appendix; keep day-to-day work anchored in the compiler surfaces above.

## Current Position

Ari is a hosted C++17 compiler prototype with an LLVM backend. It can already
compile a meaningful systems-language subset, link normal Linux/glibc
executables, build shared libraries, emit LLVM IR and objects, and run a broad
test suite.

Use [Compiler Maturity Gates](compiler-maturity-gates.md) to judge whether a
change improves the normal compiler and language surface.

The compiler is still not production-grade. The main gaps are not ambition;
they are compiler engineering scale:

- frontend code is large and needs clearer boundaries
- diagnostics need stable codes, spans, and golden-output discipline
- file-backed modules and module-cache behavior need hardening
- generic aggregate and trait behavior need fewer special cases
- IR lowering should carry more resolved metadata instead of re-resolving names
- backend output needs more normalized artifact tests
- focused test targets need to stay grouped by compiler feature and failure mode

## Development Principles

- Improve Ari as a general language and compiler, not as a private tool dialect.
- Keep hosted-compiler changes normal and reviewable.
- Prefer public language features over hidden compiler-only shortcuts.
- Keep unsupported features rejected with clear diagnostics.
- Make each compiler feature observable through a focused test.
- Keep source-level resolution in sema; codegen should consume lowered IR facts.
- Keep allocation explicit and capability-oriented.
- Keep long-term side goals as context, not as the design driver.

## Immediate Compiler Work Queue

Use this queue when the request is "make Ari a real compiler". Each slice is
useful for ordinary Ari users and compiler contributors on its own.

| Slice | Build | Focused Check | Done When |
| --- | --- | --- | --- |
| Source identity | Stable source ownership, source ids, byte spans, line/column lookup, and snippet extraction as compiler/tooling data. | `make check-compiler-development` and source-map artifact checks. | Lexer/parser diagnostics can name files and spans without runtime `std` carrying compiler-only APIs. |
| Diagnostic data | Diagnostic codes, severity, labels, notes, normalized paths, and golden rendering. | `make check-compiler-artifacts` for `--emit-diagnostics`. | Expected compiler failures are reviewed as deterministic text artifacts. |
| Project flow | File-backed modules, package roots, `.ari`/`.arih` policy, module metadata, and cache validation. | Module ok/error fixtures plus `--emit-module-graph`. | A multi-directory Ari tool builds from Make without hidden stage flags. |
| Compiler data models | Structs, enum payloads, type aliases, nested generics, tuple returns, and `Result[T, E]` in compiler-shaped code. | `tests/cases/compiler-development/ok/model/compiler-stage-gates.ari`. | Tokens, syntax nodes, pass states, and diagnostics can be expressed naturally in Ari. |
| Pass artifacts | Token, syntax, HIR, typed IR, LLVM text, object symbols, and executable behavior in comparison order. | `make check-compiler-artifacts`. | Regressions fail near the compiler layer that changed. |
| Backend contract | LLVM/object/shared output, ABI facts, runtime hooks, and symbol mangling stay deterministic. | Focused `--emit-llvm`, object, and shared-library checks. | Codegen consumes resolved IR facts instead of re-resolving source-level names. |

When a slice exposes ugly Ari code, fix the public language/compiler surface:
type aliases for intent, `char` literals for character data, tuple returns for
always-present products, `Option`/`Result` for absence and failure, and named
formatting captures for artifact text. Do not solve those problems with private
compiler-only syntax or hidden runtime hooks.

## Concrete Implementation Backlog

Use this as the near-term implementation queue. Each item is a normal compiler
feature and should land with a focused fixture before it becomes part of any
future Ari-written compiler plan.

| Priority | Work Item | First Implementation Shape | Small Check |
| --- | --- | --- | --- |
| P0 | Source identity hardening | Keep source ids, filenames, byte offsets, newline tables, and snippets as deterministic compiler artifacts. | `make check-compiler-artifacts` plus one `--emit-source-map` golden. |
| P0 | Diagnostic code/data model | Classify lexer, parser, module, type, ownership, IR, and backend failures with stable codes before polishing prose. | `--emit-diagnostics` golden under `tests/cases/compiler-development/artifact/errors/`. |
| P0 | Test classification | Keep compiler fixtures grouped by `model`, `artifact`, `ok`, and `errors`, and name each file by the behavior it protects. | `make check-language-docs` and `make check-compiler-development`. |
| P0 | Compiler health scorecard | Keep the maturity estimate tied to weighted compiler-development gates instead of a vague long-term milestone. | `tests/cases/compiler-development/ok/model/compiler-readiness-scorecard.ari`. |
| P0 | Development dashboard | Keep current status, next actions, small checks, and non-goals visible from one page. | `tests/cases/compiler-development/ok/model/compiler-development-dashboard.ari`. |
| P0 | Concepts glossary | Keep compiler layer terms, artifact vocabulary, and review language understandable to first-time compiler contributors. | `tests/cases/compiler-development/ok/model/compiler-concepts-glossary.ari`. |
| P0 | Compiler layer map | Keep source-file ownership, first artifacts, docs, and focused checks discoverable for each compiler layer. | `tests/cases/compiler-development/ok/model/compiler-layer-map.ari`. |
| P0 | Compiler triage guide | Route symptoms, bug reports, and artifact diffs to the earliest owning layer and smallest useful check. | `tests/cases/compiler-development/ok/model/compiler-triage-guide.ari`. |
| P0 | Source identity authoring | Keep source files, source ids, byte spans, line/column lookup, and source-map artifacts deterministic. | `tests/cases/compiler-development/ok/model/compiler-source-identity.ari`. |
| P0 | Module project authoring | Keep file modules, roots, search paths, metadata, caches, and module graph artifacts reviewable. | `tests/cases/compiler-development/ok/model/compiler-module-project-authoring.ari`. |
| P0 | Artifact authoring | Keep stage artifacts deterministic, normalized, small, and ordered before executable checks. | `tests/cases/compiler-development/ok/model/compiler-artifact-authoring.ari`. |
| P0 | Diagnostic authoring | Keep error codes, messages, labels, notes, and golden expected-failure tests stable and user-oriented. | `tests/cases/compiler-development/ok/model/compiler-diagnostic-authoring.ari`. |
| P0 | Test authoring policy | Keep fixture buckets, file names, expected results, and artifact update rules reviewable. | `tests/cases/compiler-development/ok/model/compiler-test-authoring.ari`. |
| P1 | Parser and declaration artifacts | Expand syntax and declaration-index dumps around attributes, patterns, generics, modules, and malformed input. | One syntax/declaration golden per changed surface. |
| P1 | Module project ergonomics | Harden package roots, `-I`, `.ari`/`.arih`, visibility, metadata, and cache invalidation diagnostics. | `make check-modules` or a single module fixture while iterating. |
| P1 | Generic aggregate stress | Add compiler-shaped nested structs/enums/vectors/maps/results without special lowering escapes. | `tests/cases/compiler-development/ok/model/` fixture plus LLVM smoke. |
| P2 | Trait and formatting selection | Make `Eq`, `Ord`, `Hash`, `Debug`, formatting, and `Drop` dispatch predictable in generic data-heavy code. | `make check-traits` and one compiler-model fixture. |
| P2 | Ownership fact visibility | Add a small artifact for owner states, borrow sources, and inserted drops before broadening ownership behavior. | Future `--emit-ownership-facts` golden or equivalent typed-IR section. |
| P2 | HIR sketch | Define the minimal lowered node vocabulary and artifact format before implementing a large HIR pass. | HIR text golden before backend checks. |

Keep each implementation slice small enough that a contributor can answer three
questions from the diff alone: which compiler layer changed, which artifact or
fixture proves it, and which public Ari language rule became clearer.

## Compiler Areas

| Area | Goal | Current Direction |
| --- | --- | --- |
| Lexer | Tokenize source with exact spans, escapes, comments, and stable invalid-token diagnostics. | Keep lexer rules boring and documented; add negative tests for every new literal or delimiter rule. |
| Parser | Build AST without guessing later semantic meaning. | Keep grammar errors precise, recover where useful, and preserve enough syntax for diagnostics and meta features. |
| AST | Represent parsed syntax without backend concepts leaking in. | Keep AST nodes simple, source-oriented, and easy to dump for tests. |
| Sema | Resolve names, modules, types, traits, ownership, and lowering metadata. | Continue decomposing large logic into local helpers and shared layout/name services. |
| IR | Hold typed, resolved facts for backend codegen. | Add metadata in IR when codegen would otherwise have to rediscover source-level decisions. |
| LLVM backend | Emit deterministic LLVM IR, object files, executables, and shared libraries. | Keep all executable/object emission on the LLVM path; add artifact checks for ABI and symbol behavior. |
| Runtime hooks | Own the narrow hosted runtime boundary. | Keep `extern "ari"` hooks validated by compiler metadata and documented in runtime/backend docs. |
| Diagnostics | Explain errors in terms users wrote, not internal lowering details. | Add stable codes, source spans, and focused golden checks before growing complex recovery. |
| Tests | Prove each feature from positive, negative, and backend angles. | Keep targets small: one compiler feature, one expected behavior, one artifact family where possible. |

## Roadmap

### Phase 1: Frontend Reliability

- Tighten lexer and parser diagnostics for literals, modules, attributes,
  patterns, generics, and meta syntax.
- Keep `--emit-source-map` deterministic so byte offsets, line lookup, newline
  policy, and source snippets are checked before lexer/parser behavior changes.
- Keep `--emit-declaration-index` deterministic so declaration names,
  signatures, visibility, and source locations are checked before sema behavior
  changes.
- Add negative tests for malformed but likely user-written code.
- Keep grammar docs and quick reference aligned with parser behavior.

Exit criteria:

- A user can understand most frontend errors without reading compiler code.
- Each new syntax feature has at least one ok test and one error test.

### Phase 2: Sema Boundaries

- Continue decomposing `src/sema.cpp` around names, modules, types, traits,
  ownership, patterns, and IR lowering.
- Move repeated layout and enum-payload decisions into shared helpers.
- Make diagnostics carry enough context to name the source construct that failed.

Exit criteria:

- A feature change usually touches one sema sub-area and one shared helper, not
  unrelated lowering code.

### Phase 3: Generic Aggregate And Trait Maturity

- Harden generic structs, tuple structs, enums, type aliases, and nested
  containers.
- Keep trait selection deterministic for `Drop`, `Eq`, `Ord`, `Hash`, `Debug`,
  and formatting.
- Add compiler-shaped fixtures that use nested `Vec`, maps, `Result`, and enum
  payloads without awkward encodings.

Exit criteria:

- Compiler-like data models can be written in normal Ari and compiled through
  LLVM without one-off compiler escape hatches.

### Phase 4: Module And Project Ergonomics

- Stabilize file-backed modules, package roots, module search paths, and module
  cache invalidation.
- Keep `--emit-module-graph` deterministic so project layout, imports, and
  item surfaces can be reviewed before sema or backend output changes.
- Improve diagnostics for missing files, private items, duplicate modules, and
  stale cache summaries.
- Document the expected layout for large Ari projects before a package manager
  exists.

Exit criteria:

- A multi-directory Ari tool can be built from Make without relying on hidden
  stage-specific flags.

### Phase 5: Diagnostic Infrastructure

- Expand the current `--emit-diagnostics` code-family bridge into explicit
  diagnostic codes at lexer, parser, module, type, ownership, IR, and backend
  throw sites.
- Attach source spans and source snippets to frontend and sema errors.
- Add golden tests for single-label, multi-label, note, and recovery messages.

Exit criteria:

- Expected compiler failures can be reviewed as stable text artifacts.

### Phase 6: IR And Backend Artifacts

- Keep `--emit-capability-inventory`, `--emit-source-map`, `--emit-tokens`,
  `--emit-syntax`, `--emit-diagnostics`, `--emit-declaration-index`, and
  `--emit-typed-ir` as the first detailed artifact producers.
- Keep `ari --list-capabilities` and `ari --explain-capability <name>` aligned
  with the same capability table so contributors can find the owner and first
  focused check without generating a file artifact.
- Use `--emit-pass-summary` for quick stage-boundary counts while extending the
  same golden comparison pattern to module graphs, structured diagnostics, HIR,
  and richer typed IR.
- Emit resolved IR facts so LLVM codegen stays mechanical.
- Add normalized LLVM IR checks for ABI, visibility, runtime hooks, and symbols.
- Keep object/shared-library tests focused on actual exported or relocated
  behavior.

Exit criteria:

- Backend regressions show up in small IR/object tests before broad executable
  failures.

### Phase 7: Production Compiler Hardening

After the compiler has stable frontend, sema, module, artifact, and backend
contracts, harden the ordinary production path:

- make error recovery useful without hiding invalid programs
- keep module-cache and project-root behavior fail-closed
- normalize LLVM/object/shared-library artifact checks
- make CI targets fast enough that contributors can run focused checks locally
- document every user-visible compiler flag in language and developer docs

## Test Layout Policy

Compiler tests should stay grouped by what they prove:

```text
tests/cases/modules/
tests/cases/generics/
tests/cases/traits/
tests/cases/variables/
tests/cases/control-flow/
tests/cases/ffi/
tests/cases/compiler-development/ok/model/
tests/cases/compiler-development/artifact/ok/
tests/cases/compiler-development/artifact/errors/
```

Use `ok/` for programs that should compile and usually run. Use `errors/` for
programs that should fail with stable diagnostics. Use `artifact/ok` and
`artifact/errors` for committed text outputs and mismatch reports. Use
descriptive filenames:

- `char-literal-escapes.ari`
- `module-private-import.ari`
- `generic-enum-payload.ari`
- `trait-associated-type-ambiguity.ari`
- `llvm-aggregate-return.ari`

Small focused checks are preferred while developing:

```text
build/ari path/to/test.ari --check
build/ari path/to/test.ari --emit-llvm build/focused/name.ll
build/ari path/to/test.ari -o build/focused/name.elf
build/ari --explain-capability trait-resolution
make check-compiler-dev-docs
make check-compiler-artifacts
```

Run `make check` before handing off larger compiler code changes. Sanitizer
checks are valuable for parser, sema, ownership, and codegen changes, but they
are intentionally not required by this document for every small slice.

## Non-Goals

- Do not add private compiler-only syntax, stdlib APIs, or runtime hooks.
- Do not move source-map and diagnostic-builder APIs into runtime `std`.
- Do not make codegen re-resolve names that sema already resolved.
- Do not hide large compiler data behind a magical global heap.
- Do not make one giant executable test the first signal of success.

## Compiler Development Health

Treat Ari as roughly **45-46% through the current compiler-development
maturity work**. That number is intentionally conservative and should move only
when normal compiler surfaces improve: better diagnostics, cleaner sema
boundaries, mature generic aggregates and traits, reliable modules, and focused
artifact tests.
