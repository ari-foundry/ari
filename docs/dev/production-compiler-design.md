# Production Compiler Design

This page defines the language and compiler design Ari needs before a serious
compiler-in-Ari bootstrap track should start.

It is deliberately not a bootstrap-only checklist. The goal is for an
Ari-written compiler to be an ordinary production Ari program: multi-file,
well-tested, explicit about allocation, pleasant to read, and built from public
language features that other Ari programs can use too.

Read this page with [Bootstrap Readiness](bootstrap-readiness.md) and
[Self-Host Roadmap](self-host-roadmap.md):

- this page defines the production language and compiler contract
- Bootstrap Readiness defines the start gate
- Self-Host Roadmap defines the stage0, stage1, and stage2 path

## Current Bootstrap Readiness

Current practical estimate:

- **35-40% ready** to begin full compiler bootstrapping
- **60-65% remaining** before a self-host attempt is likely to be productive

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
