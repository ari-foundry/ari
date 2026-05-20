# Production Compiler Design

This page defines the language and compiler design Ari needs before a serious
compiler-in-Ari bootstrap track should start.

It is deliberately not a bootstrap-only checklist. The goal is for an
Ari-written compiler to be an ordinary production Ari program: multi-file,
well-tested, explicit about allocation, pleasant to read, and built from public
language features that other Ari programs can use too.

For current work, start with
[Compiler Development Roadmap](compiler-development-roadmap.md). Ari is
developing the compiler now; bootstrapping is a later readiness milestone, not
the implementation task for this page.

[Compiler Maturity Gates](compiler-maturity-gates.md) is the concise checklist
for deciding whether ordinary compiler work is moving Ari toward that later
start gate.
[Compiler Source And Diagnostics](compiler-source-diagnostics.md) expands the
hardest current blocker: source ownership, spans, labels, reports, and golden
rendering outside runtime `std`.
[Compiler Artifact Testing](compiler-artifact-testing.md) defines the stage
output comparison policy so regressions fail near the compiler layer that
changed.

Read this page with [Compiler Bootstrap Fixture Plan](bootstrap-fixture-plan.md),
[Bootstrap Readiness](bootstrap-readiness.md), and
[Self-Host Roadmap](self-host-roadmap.md):

- this page defines the production language and compiler contract
- Compiler Bootstrap Fixture Plan defines pre-bootstrap fixture groups
- Bootstrap Readiness defines the start gate
- Self-Host Roadmap defines the stage0, stage1, and stage2 path

## Current Bootstrap Readiness

Current practical estimate:

- **38-42% ready** to begin full compiler bootstrapping
- **58-62% remaining** before a self-host attempt is likely to be productive

This is not a judgment on Ari's ambition. Ari already has a large hosted
systems-language surface: LLVM output, modules, structs, enums, traits,
generics, zones, ownership checks, C FFI, formatting, file IO, collections, and
tests.

The gap is mostly production compiler scale:

- file-backed project layout that feels boring and predictable
- source ownership, spans, and line/column lookup outside runtime `std`
- structured compiler diagnostics with stable golden output
- generic aggregate and trait behavior that stays comfortable in large code
- enough module metadata and build/test flow to support many compiler files
- stage artifact comparison so regressions are explained by the failing layer

Small Ari tools can be written now. A full compiler bootstrap should wait until
the start bar below is green.

## Readiness Scorecard

The 38-42% number comes from Ari having a real hosted systems-language
substrate, not from having an Ari compiler implementation already underway.
The missing percent is mostly about scale, project shape, and compiler-quality
tooling.

| Area | Current State | Bootstrap Impact |
| --- | --- | --- |
| Hosted executable pipeline | Usable. The current compiler emits LLVM IR, links through an LLVM driver, and runs Linux/glibc executables. | Good enough for stage0-built tools. |
| Core language model | Usable but still maturing. Functions, control flow, structs, enums, traits, generics, ownership, zones, C FFI, formatting, and modules exist. | Enough for small tools; large compiler code will stress generic aggregates and trait dispatch. |
| File-backed projects | Partial. File-backed modules and module cache work exist, but large project ergonomics and diagnostics need hardening. | Start blocker for a multi-directory compiler project. |
| Text and bytes | Partial. `char`, byte strings, string slices, ASCII, UTF-8 validation, formatting, and file reads exist. | Good for a lexer pilot; source-map ownership and diagnostic snippets are still missing. |
| Compiler diagnostics | Missing as a reusable Ari layer. Runtime `std` has logging, panic, and formatting, but not compiler source maps, labels, reports, or golden renderers. | Hard blocker before parser-scale work. |
| Data structures | Partial. Vectors, slices, maps, sets, trees, heaps, and iterators exist, but nested generic aggregate behavior needs more stress coverage. | Hard blocker before AST/HIR/symbol tables are comfortable. |
| Error flow | Partial. `Option`, `Result`, and compact errors exist, but large expected-failure workflows still need ergonomic pressure testing. | Soft blocker for lexer; hard blocker for parser/sema. |
| Build/test flow | Partial. `make` and focused compiler/std checks exist. Bootstrap-specific fixtures and stage comparison do not exist yet. | Hard blocker before claiming self-host progress. |
| Stage comparison | Not started. Token, syntax, HIR, typed IR, LLVM text, and executable comparison policy is documented but not implemented. | Hard blocker before stage1/stage2 comparison. |

The practical interpretation:

- Ari can start experimental compiler components now.
- Ari should not start a full self-hosting tree until source maps,
  diagnostics, multi-file project flow, and stage artifact comparison have
  their first focused tests.

## Design Goal

The Ari compiler written in Ari should look like a normal Ari project, not like
a secret dialect of Ari.

Good production Ari compiler code should be able to:

- model tokens, syntax nodes, HIR, symbols, and IR using public structs, enums,
  tuples, fixed arrays, vectors, maps, sets, and type aliases
- use `char` for ASCII scalar intent and `u8` for raw bytes without repeated
  casts in common text code
- use named formatting captures such as `println!("{name}")` where the compiler
  can resolve the local name
- propagate expected failures with `Result[T, E]` and small error structs
  instead of panicking for ordinary lexer, parser, IO, or type errors
- allocate long-lived compiler graphs from explicit zones or arenas
- keep compiler diagnostics in a compiler/tooling package instead of growing
  runtime `std` with bootstrap-only source-location APIs
- compile and test each compiler layer independently before any self-compile
  attempt

If a feature is only useful because stage1 is awkward to write, it is probably
the wrong feature. If the feature also makes ordinary Ari projects clearer,
safer, or more maintainable, it belongs on the normal language/compiler
roadmap.

## Bootstrap Start Bar

Create a real `bootstrap/` tree only when all of these are true:

| Area | Required State |
| --- | --- |
| File modules | File-backed modules, package roots, module paths, private/public visibility, and module-cache metadata have stable diagnostics. |
| Data modeling | Structs, tuple structs, enums, tuples, fixed arrays, type aliases, and generic aggregate monomorphization are reliable enough for AST and HIR graphs. |
| Traits and generics | Trait dispatch, bounds, associated items, formatting, hashing, equality, ordering, and collection defaults behave predictably in generic compiler code. |
| Text handling | UTF-8 source text, ASCII classifiers, `char`, `u8`, slices, strings, split/search/join, and parse helpers are natural enough for lexer/parser code. |
| Source maps | A compiler/tooling package owns source files, `SourceId`, byte spans, line/column lookup, and source snippets. |
| Diagnostics | A compiler/tooling package can build labels, notes, reports, and stable rendered diagnostics for golden tests. |
| Error values | `Result[T, E]` workflows are ergonomic enough that stage1 does not use `panic` for expected compiler errors. |
| Allocation | Explicit zone or arena policy is documented for source files, tokens, syntax trees, HIR, interned strings, symbols, and temporary test data. |
| Build/test | A focused Makefile can build one Ari tool, run fixtures, compare golden output, and keep test folders grouped by feature and expected outcome. |
| Stage comparison | Token, syntax, HIR, typed IR, LLVM text, and executable behavior have an ordered comparison policy. |

The first green target should be a lexer tool, not self-compilation:

```text
stage0 builds an Ari lexer tool
the lexer reads fixture .ari files
the lexer emits stable token and diagnostic output
tests compare that output to golden files
```

## Production Language Contract

These are the compiler-facing language features that need production-quality
behavior before the Ari compiler itself can move comfortably into Ari.

| Contract | Production Requirement | Why It Matters |
| --- | --- | --- |
| File-backed modules | Stable module roots, import paths, visibility, metadata, and cache invalidation. | The compiler cannot remain one source file, and stale module state must be debuggable. |
| Type aliases | Aliases such as `char`, target-sized aliases, and domain names remain clear in diagnostics while lowering to concrete layouts. | Compiler code needs readable domain types without losing layout control. |
| Aggregates | Structs, tuple structs, tuples, fixed arrays, and enum payloads lower consistently through sema, IR, ABI checks, and LLVM. | Tokens, AST nodes, HIR nodes, diagnostics, and symbols are aggregate-heavy. |
| Generic aggregates | Nested generic structs, enums, maps, vectors, and result payloads monomorphize without one-off special cases. | A compiler naturally builds generic containers of compiler data. |
| Traits | Static dispatch, coherence, associated types, `Drop`, `Hash`, `Eq`, `Ord`, `Debug`, and formatting traits stay predictable. | Compiler data needs maps, sets, sorting, formatting, and cleanup hooks. |
| Pattern matching | Enum and aggregate patterns remain reliable in statements and expression-valued control flow. | Parsers and semantic passes read better when cases are explicit. |
| Text literals | String and char literals keep source intent visible: `"` for UTF-8 text, `'x'` for `char`, raw bytes only when requested. | Lexer code should not be full of numeric byte constants like `48 as u8`. |
| Formatting | Type-safe formatting, named captures, debug formatting, numeric bases, width, precision, and buffer-backed formatting have stable lowering. | Diagnostics and golden dumps depend on deterministic text output. |
| Errors | `Option`, `Result`, small error structs, and conversion helpers are comfortable in ordinary control flow. | Compiler failures are normal data, not exceptional crashes. |
| Memory | Explicit zones, typed allocation, ownership, drops, and borrow diagnostics scale to long-lived compiler graphs. | AST/HIR ownership needs to be obvious to readers and maintainers. |
| FFI and backend | C ABI, LLVM IR emission, object output, target ABI facts, and runtime hooks remain documented and testable. | Stage1 should reuse the same public backend contract as other Ari tools. |

## General Language Design Work

This work should improve Ari as a general language, not just make the future
stage1 compiler easier to write.

Use this rule for every proposed compiler change:

```text
If ordinary Ari programs benefit from the feature, it belongs on the normal
language/compiler roadmap.

If only the bootstrap compiler benefits, redesign the stage1 code or keep the
need in the compiler/tooling package instead of changing the language.
```

The near-term language design priorities are:

1. Natural type intent: keep `type` aliases such as `char` visible enough in
   diagnostics, avoid forcing numeric casts for character code, and keep layout
   lowering explicit.
2. Natural call sites: prefer names such as `open(path, "rw")`,
   `is_file(path)`, `try_get(index)`, and `format_in!(...)` over type-suffixed
   helper names when generics, module paths, or expected types carry the
   meaning.
3. File-backed modules: make project roots, imports, visibility, cache
   invalidation, and missing/private/ambiguous module diagnostics predictable.
4. Generic aggregate scale: nested `Vec`, maps, `Result`, AST node enums, and
   symbol-table values should monomorphize without special C++-side escape
   hatches.
5. Trait coherence: `Hash`, `Eq`, `Ord`, `Debug`, `Display`, and `Drop`
   should have deterministic selection rules and diagnostics before stage1
   depends on them heavily.
6. Error ergonomics: expected failure paths should read as ordinary data flow
   with `Option`/`Result`, not as panic-heavy control flow.
7. Compiler tooling as Ari packages: source spans, source maps, diagnostics,
   labels, fix-its, and golden renderers belong in Ari tooling modules, not in
   runtime `std` and not in hidden stage0 builtins.

## Compiler Tooling Layer

Source-coordinate and diagnostic APIs should not be added to production `std`
just because the compiler needs them. They belong in a compiler/tooling layer
that can also be reused by lint, LSP, formatter, and future package tools.

Recommended modules:

```text
compiler/
  source/       SourceId, SourceFile, SourceMap, byte spans, line lookup
  report/       Diagnostic, Label, Note, FixIt, rendered output
  arena/        AST/HIR arena policy and scratch allocation helpers
  intern/       interned names and stable symbol ids
  lex/          token kinds, lexer state, token dumps
  syntax/       syntax nodes and syntax tree dumps
  parse/        parser state and recovery
  hir/          lowered compiler-owned IR
  resolve/      module graph, imports, visibility, symbols
  types/        type facts, substitutions, trait obligations
  emit/         normalized IR and backend artifact text
```

This layer may have source spans, file maps, compiler report builders, and
golden renderers. Runtime `std` should keep only broadly useful facilities:
formatting, logging, panic, file IO, paths, collections, time, process, C FFI,
and tests.

## Implementation Roadmap

### Phase 0: Contract Documentation

Status: active.

- Keep this page, Bootstrap Readiness, and Self-Host Roadmap linked.
- Keep one focused documentation check for the start bar and estimates.
- Keep the roadmap explicit about production language work versus bootstrap
  shortcuts.

Exit criteria:

- A new contributor can explain why Ari is not self-hosting yet and which
  public language/compiler features block it.

### Phase 0.5: Readiness Fixtures

- Add small compiler-facing fixtures that compile with today's Ari and stress
  the public language surface a future stage1 compiler would use.
- Keep them outside a real `bootstrap/` tree until the lexer pilot starts.
- Group fixtures by language pressure point: modules, generic aggregates,
  traits, formatting, source-text bytes, zones, and error values.

Exit criteria:

- The project has focused tests that reveal whether Ari can express
  compiler-shaped data before any Ari-written compiler implementation exists.

### Phase 1: Project And Module Ergonomics

- Harden file-backed modules, package roots, search paths, and module-cache
  summaries.
- Make module diagnostics useful for missing files, private items, duplicate
  modules, stale cache data, and ambiguous imports.
- Document the project layout a large Ari tool should use before `bootstrap/`
  exists.

Exit criteria:

- A multi-file Ari tool can be built from a Makefile without relying on
  one-off stage0 flags.

### Phase 2: Compiler-Scale Data Modeling

- Finish generic aggregate/type monomorphization for nested compiler data.
- Keep type aliases visible enough in diagnostics to make domain code readable.
- Stress structs, enums, tuples, `Vec`, maps, sets, and `Result` payloads in
  compiler-sized fixtures.

Exit criteria:

- Token, syntax, diagnostic, and symbol-table data structures can be expressed
  directly in Ari without awkward encoding tricks.

### Phase 3: Trait And Formatting Maturity

- Stabilize trait-driven `Hash`, `Eq`, `Ord`, `Debug`, and formatting paths.
- Keep named formatting captures and buffer-backed formatting deterministic.
- Define the exact comparison and hashing policy expected by maps and sets.

Exit criteria:

- Golden dumps and diagnostic renderers can be written without type-specific
  formatting suffixes or hand-rolled map/set helpers.

### Phase 4: Tooling Source And Diagnostics

- Build `SourceMap`, byte spans, line/column lookup, source snippets, labels,
  notes, and stable diagnostic rendering in a compiler/tooling package.
- Keep these APIs out of runtime `std`.
- Add golden tests for single-line spans, multi-line spans, notes, recovery,
  and deterministic ordering.

Exit criteria:

- An Ari tool can report a lexer error with filename, line, column, label, and
  stable rendered text.

### Phase 5: Lexer And Parser Pilot

- Implement a standalone Ari lexer.
- Add token dumps and lexical diagnostic golden tests.
- Implement a parser that emits stable syntax dumps before HIR exists.
- Test recovery on malformed fixtures.

Exit criteria:

- Stage0 can compile Ari lexer/parser tools, and the tools can process focused
  language fixtures with stable outputs.

### Phase 6: HIR, Resolve, Types, Ownership

- Lower syntax to HIR.
- Resolve module paths, imports, visibility, and symbols.
- Type-check the subset used by the stage1 compiler itself.
- Add ownership and zone checks for compiler-owned graphs.

Exit criteria:

- Stage1 can type-check its lexer/parser/HIR modules and selected fixtures.

### Phase 7: Backend Artifact And Stage Comparison

- Emit normalized HIR or IR text first.
- Add LLVM text only after frontend artifacts are stable.
- Define artifact normalization before comparing stage1 and stage2.

Exit criteria:

- Stage0-built stage1 and stage1-built stage2 can compare token, syntax, HIR,
  typed IR, and later LLVM text outputs.

## First Implementation Slices

These are the next small, reviewable compiler-design slices. Each should land
with docs and focused tests before a broad bootstrap tree appears.

| Slice | Deliverable | Focused Tests |
| --- | --- | --- |
| Source identity model | Ari tooling structs for `SourceId`, byte `Span`, and owned `SourceFile` handles. | `source-id-stability`, `span-byte-range`, `line-column-lookup`. |
| Diagnostic renderer | Ari tooling values for diagnostic severity, labels, notes, and deterministic text output. | `report-single-label`, `report-multiline-span`, `report-note-order`. |
| File module hardening | Better package-root and module-cache diagnostics in the current compiler. | `module-missing-file`, `module-private-item`, `module-cache-stale`. |
| Generic aggregate stress | Compiler-shaped structs/enums using nested vectors, maps, and results. | `generic-ast-node`, `generic-symbol-map`, `result-error-payload`. |
| Trait selection stress | Deterministic dispatch for formatting, equality, ordering, hashing, and drop hooks. | `trait-debug-node`, `trait-hash-key`, `trait-ord-symbol`. |
| Stage artifact runner | A tiny runner that compares text artifacts with normalized paths and symbol names. | `compare-token-dump`, `compare-syntax-dump`, `compare-ir-normalized`. |

Do not implement all of these inside `std`; source and diagnostic work is not in runtime `std`.
The source and diagnostic slices are compiler/tooling packages. The module,
aggregate, trait, and artifact slices are compiler/language/test work.

## Test Strategy

Bootstrap tests should stay small and grouped by what they prove.

Recommended future layout:

```text
bootstrap/
  Makefile
  stage1/
    src/
    tests/
      fixtures/
      source/ok/
      source/errors/
      lex/ok/
      lex/errors/
      parse/ok/
      parse/errors/
      report/ok/
      report/errors/
      hir/ok/
      hir/errors/
      golden/
```

Test naming should describe the feature:

- `lex-char-literals.ari`
- `lex-string-escapes.ari`
- `parse-function-decls.ari`
- `parse-match-patterns.ari`
- `report-multiline-span.ari`
- `resolve-private-import.ari`
- `type-generic-result-payload.ari`

The first test target should not run the whole suite. Keep focused targets:

```text
make check-bootstrap-docs
make -C bootstrap check-lex
make -C bootstrap check-parse
make -C bootstrap check-report
```

Only add broad stage comparison after the small targets are fast and stable.

## What To Avoid

- Do not add bootstrap-only syntax.
- Do not add source-location or diagnostic-builder APIs to runtime `std`.
- Do not start by porting `src/sema.cpp`.
- Do not hide allocation behind a magical global heap.
- Do not require threads, sockets, dynamic loading, or freestanding startup for
  the first hosted compiler path.
- Do not treat "the lexer compiles" as "Ari is self-hosted".
- Do not build a special stage1 shortcut that ordinary Ari programs cannot use.

The healthiest path is slower but cleaner: make Ari a good general-purpose
systems language first, then let the compiler become one more large Ari
program.
