# Compiler Readiness Inventory

This page tracks whether the current C++ hosted Ari compiler is mature enough
to support compiler-shaped Ari programs and the separate Ari-written compiler
source root.

It is not the Ari-written compiler source roadmap. Do not use this inventory to
put Ari-written compiler implementation under `src/`, add `bootstrap/`,
`stage1/`, or `compiler/src/`, design a package manager, or track
stdlib/library work. The Ari-written compiler source lives in `compiler/`, with
planning in [Ari-Written Compiler](../notes/ari-written-compiler.md). The
question here is whether today's hosted compiler has reliable source identity,
diagnostics, modules, frontend behavior, compiler-shaped data, traits/generics,
and artifacts.

## How To Read This Page

Readiness is an engineering judgment, not a count of tests. A gate improves
only when the underlying compiler behavior is implemented or fixed. Tests and
goldens matter because they lock behavior after the implementation is real; they
do not make an unsupported feature mature by themselves.

The old executable readiness/model fixture bucket was removed. Compiler-shaped
Ari source pressure now belongs under `tests/cases/bootstrap-readiness/` or the
owning feature bucket. `tests/cases/compiler-development/` is artifact-only.

## Start Gates

Use this priority order when deciding the next small compiler task.

| Gate | What It Means | Current Proof | What Still Moves It |
| --- | --- | --- | --- |
| Source identity / source-map / span | Every source-backed diagnostic and artifact can name a stable source id, byte span, line/column, snippet, and file identity across single-file and imported-file paths. | `make check-source-map-unit`, source-map artifacts, imported diagnostic artifacts. | Fix any remaining source-less compiler path that should have a `Span`; add tests only for uncovered edge cases after behavior is confirmed. |
| Diagnostics | Errors are source-aware, stable, useful, and classified by compiler layer instead of falling through vague text. | Diagnostic catalog CLI, diagnostic artifacts, parser/module/sema/ownership/backend expected failures. | Retire fallback diagnostics for common user errors; add labels, notes, and help where they reduce guesswork. |
| Module/project flow | Multi-file Ari projects load deterministically, preserve imported-file source identity, and reject cycles, visibility mistakes, duplicate modules, and stale caches cleanly. | `make check-modules`, module graph artifacts, package fixtures. | Fix resolver/metadata/cache bugs before adding more project fixtures. |
| Frontend reliability | Lexer and parser stay stable for invalid characters, EOF edges, malformed declarations/expressions, recovery, syntax dumps, and span propagation. | Lexer/parser diagnostic artifacts and syntax artifacts. | Improve recovery or span generation when malformed input breaks later layers or produces misleading diagnostics. |
| Compiler-shaped data models | Ari can express small AST/HIR/token/source/diagnostic-shaped values with ordinary structs, enums, results, options, and ownership rules. | `make check-bootstrap-readiness`, focused struct/match/generic/ownership tests. | Fix real typecheck/codegen/ownership gaps; do not add source fixtures just to raise a score. |
| Trait/generic readiness | Generic aggregates, generic functions, trait bounds, method resolution, monomorphization, and generic ownership work for compiler-like data. | `make check-generics`, `make check-traits`, relevant diagnostic artifacts. | Fix semantic, diagnostic, or codegen bugs in the general generic/trait path, not by special-casing `Vec`, `Option`, `Result`, or map types. |
| Artifact comparison | Each compiler stage can emit normalized, deterministic text that makes regressions reviewable before executable behavior. | `make check-compiler-artifacts`, `--list-artifacts`, `--explain-artifact`. | Add or fix emitters when a stage is nondeterministic, missing, or too raw to compare. |
| Large multi-file Ari project readiness | Larger Ari programs can be split across normal files and checked by the hosted compiler and existing Make/test runner. | Module package fixtures and focused module checks. | Fix project/module/compiler behavior; an Ari-written build tool is not part of this gate. |
| Tool build flow | Later ecosystem work for an Ari package/build tool. | Existing Make runner only. | Track separately after the compiler is mature enough; do not subtract heavily from start readiness today. |

## Current Proof Locations

| Area | Primary Locations |
| --- | --- |
| Source maps and spans | `src/source_map.hpp`, `src/source_map.cpp`, `tests/source_map_unit.cpp`, `tests/cases/compiler-development/artifact/ok/source-map-*` |
| Diagnostics | `src/common.hpp`, `src/common.cpp`, `src/diagnostic_dump.cpp`, `tests/cases/compiler-development/artifact/errors/`, `tests/check_compiler_diagnostic_cli.py` |
| Modules and packages | `src/module_loader.*`, `src/sema.cpp`, `tests/cases/modules/`, `tests/packages/` |
| Frontend | `src/lexer.cpp`, `src/parser.cpp`, lexer/parser diagnostic artifacts, syntax artifacts |
| Bootstrap readiness pressure | `tests/cases/bootstrap-readiness/` |
| Generics and traits | `tests/cases/generics/`, `tests/cases/traits/`, related diagnostic artifacts |
| Artifact comparison | `src/compiler_summary_dump.cpp`, `src/driver.cpp`, `tests/cases/compiler-development/artifact/` |

## What Not To Add

- Do not recreate deleted source-model fixture buckets.
- Do not recreate deleted compiler-development source-error buckets.
- Do not start writing the Ari compiler in Ari inside `src/` or this roadmap;
  use `compiler/`.
- Do not add bootstrap/stage1 directories or `compiler/src/`.
- Do not count stdlib/library work toward this inventory.
- Do not count an Ari-written package/build tool as a start gate.
- Do not claim readiness improved when only tests were added for an
  unsupported feature.

## Small Work Loop

For each gate, use this loop:

1. Find the implementation path and the existing focused tests.
2. Classify the gap as implementation missing, implementation buggy, or
   verification missing.
3. Fix the hosted compiler when behavior is missing or wrong.
4. Add one focused fixture or golden only when behavior exists and needs to be
   locked.
5. Run the narrow target for that gate.
6. Update the relevant docs so the next contributor can find the same path.

## Focused Checks

| Change | First Check |
| --- | --- |
| SourceMap or source-location behavior | `make check-source-map-unit` |
| Diagnostic rendering, catalog, or diagnostic artifacts | `make check-compiler-artifacts` and `python3 tests/check_compiler_diagnostic_cli.py` when CLI/catalog behavior changes |
| Module loader, resolver, metadata, or cache behavior | `make check-modules` |
| Compiler artifact emitters or golden text | `make check-compiler-artifacts` |
| Bootstrap-readiness source fixtures | `make check-bootstrap-readiness` |
| Compiler docs index, roadmap, and readiness inventory | `make check-compiler-docs` |
| Generics | `make check-generics` |
| Traits | `make check-traits` |
| Ownership | `make check-ownership` |

Full `make check` is useful before large handoffs, but normal development
should start with the smallest target that can fail near the changed layer.
