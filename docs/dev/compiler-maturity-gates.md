# Compiler Maturity Gates

This page tracks the work needed for Ari to feel like a practical compiler
project: a reliable frontend, clean semantic boundaries, stable diagnostics,
deterministic artifacts, and language features that are pleasant for any large
Ari program.

Use this page with:

- [Compiler Development Roadmap](compiler-development-roadmap.md) for the
  day-to-day development order.
- [Compiler Contributor Guide](compiler-contributor-guide.md) for the practical
  edit map, test categories, and small-check loop.
- [Compiler Readiness Inventory](compiler-readiness-inventory.md) for the
  current strengths, blocking gaps, backlog, and health scorecard.
- [Compiler Pass Contracts](compiler-pass-contracts.md) for lexer, parser,
  resolver, sema, IR, and backend boundary rules.
- [Compiler Source And Diagnostics](compiler-source-diagnostics.md) for the
  source-map and diagnostic tooling layer.
- [Compiler Project Model](compiler-project-model.md) for file-backed modules,
  package roots, module search paths, metadata, cache policy, and Make targets.
- [Compiler Artifact Testing](compiler-artifact-testing.md) for stage output,
  normalization, and golden comparison policy.
- [Production Compiler Design](production-compiler-design.md) for the long-term
  language contract appendix.
- [Feature Test Matrix](test-matrix.md) for existing test coverage.

## Current Estimate

Ari is currently about **45-46% through the current compiler-development
maturity work**.

This estimate is intentionally conservative. Ari already has enough language
surface to build small tools. The remaining work is about compiler scale:
multi-file project flow, stable diagnostics, source maps, generic aggregate
maturity, predictable trait selection, artifact comparison, and tests that fail
near the layer that regressed.

## How To Read The Gates

Each gate below is a normal compiler-development goal. A gate is not green just
because one toy program compiles; it is green when the feature is documented,
covered by positive and negative tests, and stable enough for several thousand
lines of Ari source to rely on it without awkward encodings.

The healthiest order is:

1. Make the current C++ compiler more reliable.
2. Add public Ari language features only when ordinary Ari programs benefit.
3. Add compiler/tooling packages for source maps, diagnostics, and artifact
   rendering.
4. Add broader tools only after the gates that feed them are green.

## Maturity Gates

| Gate | Required State | Test Shape | Status |
| --- | --- | --- | --- |
| Frontend grammar | Lexer and parser rules are documented, boring, and reject malformed syntax with useful spans. | `tests/cases/<feature>/ok` and `errors` plus parser-focused diagnostics. | Partial |
| Source identity | A compiler/tooling package can own files, `SourceId`, byte spans, line/column lookup, and snippets. | `source/ok/source-line-column`, span edge cases, multi-file source maps. | Seed fixture only |
| Diagnostics | Compiler errors have stable codes, labels, notes, and stable golden rendering outside runtime `std`. | Golden text tests for single-label, multi-label, note order, and recovery. | Seeded |
| Module projects | File-backed modules, roots, search paths, visibility, and module cache invalidation are predictable. | Missing/private/stale-cache diagnostics plus multi-file ok fixtures. | Partial |
| Generic data models | Nested structs, enums, aliases, tuples, vectors, maps, sets, and `Result` payloads monomorphize cleanly for local/codegen-supported shapes. | User-defined aggregate, stdlib stress, compiler-shaped token, AST, symbol-table, and error-model fixtures. | Good first pass |
| Trait selection | `Drop`, `Debug`, formatting, `Eq`, `Ord`, and `Hash` dispatch are deterministic and diagnosable. | Static dispatch tests, trait-object tests where supported, ambiguous impl errors. | Partial |
| Error flow | Expected compiler failures use `Result[T, E]` and small error structs instead of panic-heavy paths. | Result propagation fixtures and negative parser/lexer recovery tests. | Seed fixture only |
| Allocation model | Zones, scratch arenas, ownership, drops, and borrow diagnostics scale to compiler graphs. | Arena fixture tests, reset/destroy invalidation tests, owned graph cleanup tests. | Partial |
| IR contract | Sema lowers resolved facts into IR so backend codegen stays mechanical. | IR text checks for names, layouts, ABI, runtime hooks, and symbols. | Partial |
| Backend artifacts | LLVM IR, object, executable, and shared library output are deterministic enough to inspect. | Focused `--emit-llvm`, `--emit-obj`, symbol, relocation, and exit-code checks. | Good first pass |
| Tool build flow | A Makefile can build one Ari tool, run its fixtures, and compare outputs without hidden flags. | Focused tool targets that run in seconds and compare artifacts. | Planned |
| Stage comparison | Token, syntax, HIR, typed IR, LLVM text, and executable behavior have a comparison order. | Normalized text artifact checks, then executable checks only after earlier layers match. | Seeded |

## Implementation Order

### 1. Keep The Hosted Compiler Maintainable

Improve the current C++ compiler as a normal compiler:

- keep lexer/parser diagnostics precise
- keep sema responsible for source-level resolution
- keep IR typed and resolved
- keep LLVM codegen mechanical
- keep unsupported features rejected clearly

Done means a change can usually be reviewed in one compiler area and one
focused test target.

### 2. Harden Project And Module Flow

Large Ari programs need boring project behavior before an Ari compiler tree
exists.

Required work:

- package root selection
- file-backed modules and module path lookup
- private/public visibility diagnostics
- module cache summary validation
- stale cache and duplicate module errors

Focused tests should live under existing module/package test folders until a
real compiler-tool tree exists.

### 3. Make Compiler-Shaped Data Easy

Compiler code naturally uses recursive and nested data:

- token structs and enums
- syntax node enums
- HIR nodes
- symbol tables
- type substitutions
- diagnostic labels and notes
- vectors and maps of all of the above

The implementation target is not "special support for the compiler"; it is
generic aggregate and trait behavior that ordinary Ari applications can also
use.

### 4. Build Tooling APIs Outside Runtime Std

Source maps and diagnostic builders are compiler/tooling APIs, not production
runtime `std` APIs.

Recommended package shape:

```text
compiler/
  source/       SourceId, SourceFile, Span, line and column lookup
  report/       Diagnostic, Label, Note, FixIt, stable renderer
  arena/        long-lived compiler graph allocation policy
  intern/       interned names and stable ids
  artifact/     capability inventory, token, syntax, HIR, IR, LLVM text normalization
```

This package can later be reused by lint, LSP, formatter, and package tools.

### 5. Keep Tooling Incremental

The first useful compiler-adjacent tool should:

- read `.ari` files
- emit stable token dumps
- emit stable lexical diagnostics
- compare output with committed golden files
- run in seconds from a focused Make target

Only after that is stable should parser, HIR, type checking, and backend
artifacts depend on the same workflow.

## Test Classification

Compiler-development tests should say which layer they protect.

| Test Family | Purpose | Example Names |
| --- | --- | --- |
| Grammar ok | Prove accepted syntax stays accepted. | `parser-function-decls.ari`, `parser-match-patterns.ari` |
| Grammar errors | Prove malformed syntax fails clearly. | `parser-missing-brace.ari`, `lexer-invalid-escape.ari` |
| Sema ok | Prove names, types, ownership, traits, and modules resolve. | `generic-symbol-table.ari`, `trait-debug-node.ari` |
| Sema errors | Prove misuse fails before codegen. | `module-private-import.ari`, `trait-ambiguous-impl.ari` |
| IR artifacts | Prove resolved facts survive lowering. | `ir-aggregate-layout.ari`, `ir-runtime-hook.ari` |
| Backend artifacts | Prove LLVM/object/shared output shape. | `llvm-symbols.ari`, `object-relocations.ari` |
| Tooling fixtures | Prove future Ari tools can model compiler data. | `source-line-column.ari`, `errors-result-flow.ari` |
| Golden text | Prove rendered compiler outputs are stable. | `report-single-label.golden`, `token-dump-basic.golden` |

Small checks should stay narrow while developing:

```text
build/ari tests/cases/modules/ok/module-llvm.ari --check
build/ari tests/cases/generics/ok/generic-function.ari --emit-llvm build/focused/generic.ll
make check-language-docs
make check-compiler-dev-docs
make check-compiler-artifacts
```

Run broad `make check` only before handing off large compiler changes.
Sanitizer checks are useful for parser, sema, ownership, and codegen changes,
but they are intentionally separate from this focused docs-and-design gate.

## Natural Language Design Rules

When compiler work exposes awkward Ari code, fix the general language or
public APIs instead of adding private compiler-only tricks.

Good fixes:

- `type char = u8` stays readable in source and diagnostics.
- Character literals like `'0'` work where byte-like character intent is clear.
- Tuple returns are used for always-present paired values such as
  `(value, overflowed)`.
- `Option` and `Result` are used when a value may be absent or an operation may
  fail.
- Natural names such as `checked_pow`, `open(path, "rw")`, and
  `try_get(index)` are preferred over noisy type suffixes.
- Named formatting captures and buffer-backed formatting are used for stable
  artifacts.

Bad fixes:

- private compiler-only keywords
- hidden compiler-only allocation
- runtime `std` source-location APIs used only by the compiler
- codegen rediscovering names or types that sema already resolved
- one giant executable test as the first signal of progress

## Readiness Formula

Use this formula when updating the readiness estimate:

```text
readiness =
  language surface that can model compiler data
+ diagnostics and source-map maturity
+ project/module/build flow
+ artifact comparison quality
+ focused test coverage
- private shortcuts or untested special cases
```

The current estimate is about **45-46%** because Ari has a substantial hosted
systems-language base, but diagnostics, compiler-tooling packages, module
scale, and artifact comparison still need hardening.

## Non-Goals For Now

- Do not port `src/sema.cpp` first.
- Do not add private compiler-only stdlib APIs.
- Do not require threads, sockets, dynamic loading, kernel APIs, or
  freestanding startup for the first hosted compiler path.
- Do not treat one compiled tool as proof that the compiler is mature.
