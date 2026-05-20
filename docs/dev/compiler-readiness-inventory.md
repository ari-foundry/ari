# Compiler Readiness Inventory

This page is the inventory for Ari compiler development. It answers one
question: what has to become true before starting an Ari-written compiler track
is a good engineering decision?

This is not a bootstrap implementation plan. The immediate work is still normal
compiler development: make the hosted compiler reliable, make the language
pleasant for large programs, and make compiler behavior easy to test in small
pieces.

In short: normal compiler development comes first.

## Current Readiness

Ari is about **38-42% ready** to begin a serious compiler-in-Ari track. That
leaves about **58-62% remaining** before self-hosting work is likely to be
productive.

The number is conservative because Ari already compiles useful programs, but a
compiler is a large program with hard requirements: stable diagnostics,
multi-file project flow, data-heavy generic models, deterministic artifacts,
and tests that fail near the layer that regressed.

## Already Strong

| Area | Current Strength | Why It Matters |
| --- | --- | --- |
| Hosted LLVM backend | Ari emits LLVM IR, objects, executables, and shared libraries through the LLVM path. | Compiler work can be validated as real artifacts instead of toy interpreter behavior. |
| Core executable language | Functions, locals, control flow, integers, bools, enums, structs, modules, FFI, and formatting are available. | Compiler-shaped fixtures can be written as ordinary Ari programs. |
| Generic calls and ADTs | Generic functions, generic structs, generic enums, `Option[T]`, and `Result[T, E]` exist. | Source models, tokens, AST nodes, and expected failures can use natural types. |
| Ownership checks | Move, borrow, drop, and explicit-zone checks catch many unsafe flows. | Large compiler graphs can be kept explicit instead of hiding allocation in globals. |
| Focused test culture | Many feature folders already separate `ok` and `errors` tests. | New compiler behavior can be guarded with one small fixture at a time. |

## Blocking Gaps

| Gap | Needed State | First Work |
| --- | --- | --- |
| Source identity | Stable `SourceId`, byte spans, line/column lookup, and snippets for every diagnostic. | Add compiler/tooling source-map fixtures and golden source rendering checks. |
| Diagnostics | Stable codes, labels, notes, and normalized golden output. | Move errors toward data-first diagnostics before polishing renderer text. |
| File-backed projects | Predictable module roots, `.ari`/`.arih` policy, metadata, cache invalidation, and Makefile flows. | Harden module search and add stale/private/missing file diagnostics. |
| Generic aggregate scale | Nested structs, enums, vectors, maps, sets, and `Result` payloads need fewer edge cases. | Keep adding compiler-shaped model fixtures under `tests/cases/compiler-development/ok/model/`. |
| Trait selection | `Eq`, `Ord`, `Hash`, `Debug`, formatting, `Drop`, and iterator dispatch need stable ambiguity errors. | Add positive dispatch tests and negative ambiguous-impl tests. |
| Pass artifacts | Token, syntax, HIR, typed IR, LLVM, object, executable, and shared outputs need a comparison order. | Add normalized text dumps before broad executable checks. |
| Build ergonomics | Large Ari tools need boring Make targets before a package manager exists. | Keep `make check-compiler-development` small and add one target per compiler slice. |

## Development Backlog

Use this order for general compiler development:

1. Frontend contracts: lexer/parser span accuracy, literal behavior, recovery,
   and malformed syntax diagnostics.
2. Source model: `SourceFile`, `SourceId`, `Span`, byte offsets, line/column
   conversion, and snippet extraction as compiler/tooling concepts.
3. Diagnostic model: diagnostic codes, severity, primary/secondary labels,
   notes, stable sorting, and path normalization.
4. Module projects: file-backed modules, project roots, header/source
   separation, module metadata, and module caches.
5. Type and trait maturity: generic aggregate monomorphization, associated
   types, trait selection, and clear ambiguity diagnostics.
6. Ownership scale: arena/zone patterns for compiler graphs, borrowed views,
   scratch reset rules, and predictable drops.
7. IR contract: lower resolved facts into typed IR so LLVM codegen is mostly a
   mechanical emitter.
8. Artifact testing: normalize and compare token, diagnostic, syntax, HIR,
   typed IR, LLVM, object symbol, and executable outputs.

## Start Gate

Do not start a compiler-in-Ari tree just because a small lexer can be written.
Start it when these gates are green enough that the work will improve Ari
instead of creating a private dialect:

| Gate | Green Signal |
| --- | --- |
| Frontend reliability | New syntax has ok tests, error tests, docs, and span-aware diagnostics. |
| Compiler data models | Compiler-shaped structs/enums/generic containers compile without awkward casts or hidden runtime hooks. |
| Diagnostic stability | Expected failures can be reviewed as normalized text artifacts. |
| Module project flow | A multi-directory Ari tool builds from Make with explicit search paths and no hidden stage flags. |
| Artifact comparison | The project can compare earlier artifacts before relying on linked executable behavior. |
| Developer loop | A contributor can find the right source file and run one focused check from docs alone. |

## Natural Syntax Pressure

When compiler code looks ugly, prefer improving the general language surface:

- use `type` aliases for domain terms such as `SourceId`, `ByteOffset`,
  `SymbolId`, and `TypeId`
- use `char` literals like `'0'` instead of integer byte constants
- use `Result[T, E]` for expected compiler failures
- use tuple returns for always-present product values such as
  `(value, overflowed)`
- use `Option[T]` for absence instead of sentinel values when the type can be
  expressed cleanly
- use named formatting captures for stable artifact text
- keep compiler/tooling diagnostics outside runtime `std`

Do not add bootstrap-only keywords, hidden global allocation, or backend hooks
that ordinary Ari programs cannot use.

## Test Inventory

Current readiness tests:

- `tests/cases/bootstrap-readiness/ok/model/model-token-span.ari`: token/span
  model seed fixture.
- `tests/cases/bootstrap-readiness/ok/source/source-line-column.ari`: source
  lookup seed fixture.
- `tests/cases/bootstrap-readiness/ok/errors/errors-result-flow.ari`:
  `Result[T, E]` expected-failure fixture.
- `tests/cases/bootstrap-readiness/ok/formatting/formatting-artifact-line.ari`:
  stable artifact text seed fixture.
- `tests/cases/compiler-development/ok/model/compiler-pass-worklist.ari`:
  normal compiler pass/worklist data model.
- `tests/cases/compiler-development/ok/model/compiler-diagnostic-workflow.ari`:
  normal diagnostic data flow as Ari values.

The first command to run after changing this area is:

```text
make check-compiler-development
```

Run `make check-compiler-dev-docs` when changing the roadmap, maturity gates,
or this inventory.
