# Compiler Development Roadmap

This page is the day-to-day roadmap for developing the Ari compiler.

It is not a plan to start bootstrapping today. Bootstrapping is only one later
readiness signal. The immediate job is to make the current compiler a reliable,
pleasant, general-purpose language implementation: predictable frontend,
maintainable semantic analysis, stable IR, testable LLVM output, and clear
diagnostics for ordinary Ari users.

Read this page before the bootstrap-specific pages:

- [Architecture](architecture.md) explains the current C++ compiler shape.
- [Compiler Pipeline](compiler-pipeline.md) explains the source-to-LLVM path.
- [Compiler Contributor Guide](compiler-contributor-guide.md) is the practical
  edit map and small-test loop for day-to-day compiler changes.
- [Compiler Readiness Inventory](compiler-readiness-inventory.md) lists the
  current strengths, blockers, backlog, and start gate.
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
- [Compiler Artifact Testing](compiler-artifact-testing.md) defines how token,
  diagnostic, syntax, HIR, typed IR, LLVM, object, and executable artifacts are
  compared.
- [Production Compiler Design](production-compiler-design.md) explains the
  language contract a future Ari-written compiler would also rely on.
- [Bootstrap Readiness](bootstrap-readiness.md) tracks how close Ari is to
  beginning compiler-in-Ari work, but it does not replace this roadmap.

## Current Position

Ari is a hosted C++17 compiler prototype with an LLVM backend. It can already
compile a meaningful systems-language subset, link normal Linux/glibc
executables, build shared libraries, emit LLVM IR and objects, and run a broad
test suite.

The active work is compiler development, not bootstrapping. Use
[Compiler Maturity Gates](compiler-maturity-gates.md) to judge whether a change
improves the normal compiler and language surface enough to move the later
bootstrap start gate.

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

- Improve Ari as a general language, not as a private bootstrap dialect.
- Keep stage0 changes in the current compiler normal and reviewable.
- Prefer public language features over hidden compiler-only shortcuts.
- Keep unsupported features rejected with clear diagnostics.
- Make each compiler feature observable through a focused test.
- Keep source-level resolution in sema; codegen should consume lowered IR facts.
- Keep allocation explicit and capability-oriented.
- Treat bootstrapping percentage as a health metric, not as the current task.

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

- Keep `--emit-tokens`, `--emit-syntax`, `--emit-diagnostics`, and
  `--emit-typed-ir` as the first detailed artifact producers.
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

### Phase 7: Bootstrap Start Readiness

This is not where development starts. It is the point where Ari is mature
enough to begin an Ari-written compiler component without creating a private
language fork.

Start that track only when:

- frontend diagnostics are stable enough for golden tests
- module/project flow is predictable
- compiler-shaped generic data models compile cleanly
- source spans and diagnostic renderers exist outside runtime `std`
- artifact comparison is defined for tokens, syntax, HIR, IR, and LLVM text

## Test Layout Policy

Compiler tests should stay grouped by what they prove:

```text
tests/cases/lexer/
tests/cases/parser/
tests/cases/modules/
tests/cases/generics/
tests/cases/traits/
tests/cases/variables/
tests/cases/control-flow/
tests/cases/ffi/
tests/cases/bootstrap-readiness/
tests/errors/
```

Use `ok/` for programs that should compile and usually run. Use `errors/` for
programs that should fail with stable diagnostics. Use descriptive filenames:

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
make check-compiler-dev-docs
make check-bootstrap-readiness
```

Run `make check` before handing off larger compiler code changes. Sanitizer
checks are valuable for parser, sema, ownership, and codegen changes, but they
are intentionally not required by this document for every small slice.

## What Not To Do

- Do not implement bootstrapping as the current task.
- Do not add bootstrap-only syntax, stdlib APIs, or runtime hooks.
- Do not move source-map and diagnostic-builder APIs into runtime `std`.
- Do not make codegen re-resolve names that sema already resolved.
- Do not hide large compiler data behind a magical global heap.
- Do not make one giant self-host test the first signal of success.

## Bootstrap Readiness Estimate

As of this roadmap, Ari is roughly **38-42% ready** to begin a serious
compiler-in-Ari track, leaving about **58-62% remaining** before that attempt
is likely to be productive.

That number is intentionally conservative. The next useful work is ordinary
compiler development: better diagnostics, cleaner sema boundaries, mature
generic aggregates and traits, reliable modules, and focused artifact tests.
