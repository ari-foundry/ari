# Ari Compiler Developer Overview

This directory documents the current C++ hosted Ari compiler. It is not a plan
to start the Ari-written compiler yet, and it is not a standard-library
readiness scorecard. Use it to understand where compiler behavior lives, how to
change it safely, and which narrow check proves the change.

## First-Hour Path

Read these in order when you are new to compiler work in this repository:

1. [Architecture](architecture.md): the major compiler pieces and why they
   exist.
2. [Compiler Pipeline](compiler-pipeline.md): the order from source text to
   diagnostics, typed IR, LLVM IR, object files, and executables.
3. [Compiler Layer Map](compiler-layer-map.md): where each feature belongs and
   which test bucket should cover it.
4. [Compiler Source Identity](compiler-source-identity.md): `SourceId`,
   `Span`, line/column lookup, snippets, and multi-file identity.
5. [Compiler Diagnostic Authoring](compiler-diagnostic-authoring.md): how to
   produce stable source-aware errors.
6. [Compiler Artifact Authoring](compiler-artifact-authoring.md): how to emit
   deterministic stage outputs.
7. [Compiler Test Authoring](compiler-test-authoring.md): where to add focused
   `ok`, `errors`, and artifact fixtures.
8. [Build And Test](build-test.md): the commands to run while iterating.
9. [Compiler Readiness Inventory](compiler-readiness-inventory.md): which
   compiler gates still need implementation work.
10. [Roadmap](roadmap.md): the active compiler work queue and non-goals.

## Where To Edit

| Task | Start Here | Usual Check |
| --- | --- | --- |
| Lexer token, token span, invalid character, string/comment edge | `src/lexer.cpp` | focused lexer/parser artifact or feature target |
| Parser rule, syntax node, recovery, parser diagnostic | `src/parser.cpp`, `src/ast.hpp` | focused parser/syntax artifact |
| Name lookup, modules, types, ownership, IR lowering | `src/sema.cpp` and helper semantics files | owning feature target |
| Typed IR shape | `src/ir.hpp`, `src/sema.cpp` | typed IR artifact or owning feature target |
| LLVM IR, object, shared library, runtime boundary | `src/llvm_codegen.cpp`, `src/driver.cpp` | backend/artifact target |
| Source ids, line/column, snippets | `src/source_map.hpp`, `src/source_map.cpp` | `make check-source-map-unit` |
| Diagnostic rendering or catalog output | `src/common.hpp`, `src/common.cpp`, `src/diagnostic_dump.cpp` | `python3 tests/check_compiler_diagnostic_cli.py` plus focused artifact |
| Compiler summaries, artifacts, pass listings | `src/compiler_summary_dump.cpp`, `src/driver.cpp` | `make check-compiler-artifacts` |

## What To Read By Goal

| Goal | Docs |
| --- | --- |
| Add or debug a source-aware error | [Compiler Diagnostic Authoring](compiler-diagnostic-authoring.md), [Compiler Source Identity](compiler-source-identity.md) |
| Add an artifact or normalize an existing one | [Compiler Artifact Authoring](compiler-artifact-authoring.md) |
| Add a module or package fixture | [Compiler Module Project Authoring](compiler-module-project-authoring.md) |
| Work on stdlib items blocked by compiler features | [Compiler-Bound Standard Library Gaps](compiler-bound-stdlib-gaps.md) |
| Work on current-zone allocation syntax | [Current-Zone Blocks](current-zone-blocks.md) |
| Pick the right focused test bucket | [Compiler Test Authoring](compiler-test-authoring.md), [Feature Test Matrix](test-matrix.md) |
| Understand semantic pass boundaries | [Compiler Pass Contracts](compiler-pass-contracts.md), [Semantic Checker Decomposition](sema-decomposition.md) |
| Work on traits, operators, generics, ownership, or ABI | [Minimum Trait Readiness](trait-minimum-readiness.md), [Trait-Backed Operators](operator-trait-design.md), [Generic Aggregate Monomorphization](generic-aggregate-monomorphization.md), [Ownership Drop Readiness](ownership-drop-readiness.md), [Aggregate ABI Classification](aggregate-abi.md) |
| Understand runtime/backend output | [Runtime And Backend](runtime-backend.md), [Runtime Support Roadmap](runtime-support.md), [Symbol Mangling](symbol-mangling.md) |

## Focused Checks

Use the narrowest check that can fail near the changed layer:

| Change | First Check |
| --- | --- |
| Documentation index or compiler-doc policy | `make check-compiler-docs` |
| Source map or source-location behavior | `make check-source-map-unit` |
| Compiler artifact emitters or artifact golden text | `make check-compiler-artifacts` |
| Diagnostic catalog CLI | `python3 tests/check_compiler_diagnostic_cli.py` |
| Module loader, resolver, metadata, or cache | `make check-modules` |
| Generics | `make check-generics` |
| Traits | `make check-traits` |
| Ownership | `make check-ownership` |
| ABI/layout units | `make check-layout-unit` |

Run full `make check` only for broad handoffs. While iterating, prefer focused
checks.

## Non-Goals

- Do not start writing the Ari compiler in Ari here.
- Do not add `bootstrap/`, `stage1/`, or self-host implementation scaffolding.
- Do not count standard-library maturity as compiler-writing readiness.
- Do not add an Ari-written package/build tool as a prerequisite for current
  compiler readiness.
- Do not add `class` or `interface`; Ari uses `trait`.
- Do not claim a gate improved when only tests were added for unsupported
  compiler behavior.
