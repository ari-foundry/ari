# Compiler Contributor Guide

This page is the practical entry point for people developing the Ari compiler.
The job today is to make Ari a reliable, pleasant, general-purpose language
implementation with clear diagnostics, predictable modules, stable IR, and
reviewable backend output.

Use this guide when you are about to change compiler behavior and need to know
where to edit, what to test, and how the change improves Ari as a real
language implementation.

## Start Here

For a compiler change, read these in order:

1. [Compiler Development Roadmap](compiler-development-roadmap.md): current
   development phases and non-goals.
2. [Compiler Readiness Inventory](compiler-readiness-inventory.md): current
   strengths, blocking gaps, backlog, and development gates.
3. [Compiler Layer Map](compiler-layer-map.md): which `src/` files own each
   compiler layer and which small check to run first.
4. [Compiler Triage Guide](compiler-triage-guide.md): how to route a symptom
   to the earliest layer, closest test bucket, and smallest check.
5. [Compiler Pipeline](compiler-pipeline.md): how source becomes LLVM output.
6. [Compiler Pass Contracts](compiler-pass-contracts.md): what each pass owns
   and what data it may hand to the next pass.
7. [Feature Test Matrix](test-matrix.md): which feature families already have
   ok, error, IR, and executable coverage.
8. [Build And Test](build-test.md): focused Make targets and direct
   `build/ari` commands.

Use [Compiler Maturity Gates](compiler-maturity-gates.md) as the health
scorecard for normal compiler work.

## Edit Map

| Change | First Files To Read | Usual Tests |
| --- | --- | --- |
| Tokens, literals, comments, escapes | `src/lexer.cpp`, `src/literal.cpp`, `src/token.hpp` | lexer/parser ok and error fixtures, then `--check` |
| Grammar or AST shape | `src/parser.cpp`, `src/ast.hpp`, `src/ast_builders.cpp` | parser/control-flow/struct/module fixtures, malformed syntax diagnostics |
| Names, modules, visibility | `src/module_loader.cpp`, `src/module_path.cpp`, `src/sema.cpp` | `make check-modules` or one module fixture |
| Types, inference, generics | `src/type_semantics.cpp`, `src/type_inference.cpp`, `src/trait_semantics.cpp` | `make check-generics`, `make check-traits`, focused `--emit-llvm` |
| Ownership and borrowing | `src/ownership_semantics.cpp`, `src/borrow_semantics.cpp`, `src/move_semantics.cpp` | focused ownership/borrowing error fixtures |
| Patterns and control flow | `src/pattern_semantics.cpp`, `src/control_flow_semantics.cpp`, `src/loop_state_semantics.cpp` | `make check-control-flow`, `make check-match` |
| IR facts | `src/ir.hpp`, `src/ir_builders.cpp`, `src/sema.cpp` | focused `--emit-llvm` plus IR text checks |
| LLVM output and artifacts | `src/llvm_codegen.cpp`, `src/toolchain.cpp`, `src/driver.cpp` | `--emit-llvm`, `--emit-obj`, linked executable or shared object |
| Source docs and gates | `docs/dev/*.md`, `tests/*manifest*.txt` | `make check-compiler-dev-docs` |

If a backend change needs information that sema already knows, add that fact to
IR instead of making codegen re-resolve source names.

## Target Picker

Choose the smallest target that observes the behavior you changed:

| Change Shape | First Check |
| --- | --- |
| User-facing language docs or navigation | `make check-language-docs` |
| Compiler roadmap, maturity gates, pass contracts, or readiness docs | `make check-compiler-dev-docs` |
| Compiler pass ownership, inputs, outputs, or first artifact routing | `build/ari --list-passes` or `build/ari --explain-pass sema` |
| Compiler fixture placement or test bucket routing | `build/ari --list-test-buckets` or `build/ari --explain-test-bucket compiler-artifact-ok` |
| Stage-plan, capability inventory, token, source-map, syntax, diagnostic catalog, diagnostic, module-graph, declaration, typed-IR, or pass-summary artifacts | `make check-compiler-artifacts` |
| Compiler-shaped Ari model fixtures | `make check-compiler-development` |
| One ordinary Ari program | `build/ari path/to/case.ari --check` |
| One backend lowering shape | `build/ari path/to/case.ari --emit-llvm build/focused/name.ll` |

Do not jump to the full suite while the change is still local. Broad checks are
for handoff or for changes that cross multiple compiler boundaries.

## Development Loop

Use small checks while iterating:

```text
build/ari tests/cases/<area>/ok/<case>.ari --check
build/ari tests/cases/<area>/ok/<case>.ari --emit-llvm build/focused/<case>.ll
build/ari tests/cases/<area>/ok/<case>.ari -o build/focused/<case>.elf
build/ari --explain-pass sema
build/ari --explain-test-bucket compiler-artifact-ok
make check-language-docs
make check-compiler-dev-docs
make check-compiler-development
```

Run broader targets before handoff only when the change crosses a boundary:

- `make check-modules` for module loading, metadata, cache, and visibility
- `make check-generics` for generic calls, impls, and monomorphization
- `make check-traits` for trait dispatch, associated items, and trait objects
- `make check-control-flow` and `make check-match` for ownership flow,
  patterns, loops, and expression-valued control flow
- `make check-cli` for driver, artifact, object, shared-library, and toolchain
  behavior

Sanitizer runs are useful for parser, sema, ownership, and codegen internals,
but they are a separate heavy check and are not required for every small
documentation or fixture slice.

## Test Categories

Tests should explain what they protect from the path alone:

```text
tests/cases/<feature>/ok/
tests/cases/<feature>/errors/
tests/cases/compiler-development/ok/model/
tests/cases/compiler-development/artifact/ok/
tests/cases/compiler-development/artifact/errors/
```

Use `compiler-development` for fixtures that prove normal Ari can model
compiler-shaped data and compiler artifacts.

Use `artifact/ok` and `artifact/errors` for text artifacts and comparison
reports. Use ordinary feature folders such as `modules`, `generics`, `traits`,
`ownership`, or `ffi` when the fixture primarily protects a language feature
instead of compiler-development infrastructure.

Prefer names like:

- `compiler-pass-worklist.ari`
- `source-line-column.ari`
- `errors-result-flow.ari`
- `formatting-artifact-line.ari`
- `compiler-test-classification.ari`

Each test should have one reason to exist. If a file starts proving lexer,
parser, generics, ownership, and backend behavior all at once, split it.

## Natural Design Rule

When compiler work exposes awkward Ari code, fix the normal language or public
library surface. Do not add private compiler-only syntax or runtime hooks.

Good examples:

- use `char` and character literals like `'0'` for byte character intent
- use type aliases for domain types such as `SourceId` and `ByteOffset`
- use `Option[T]` when a value may be absent
- use `Result[T, E]` when a pass may fail normally
- use tuple returns for always-present product values such as
  `(value, overflowed)`
- use named formatting captures for stable artifact text

Bad examples:

- private compiler-only keywords
- hidden global compiler allocation
- source-map APIs in runtime `std`
- codegen rediscovering names that sema resolved
- one giant executable test as the first signal of progress

## What Counts As Progress

A compiler change is healthy when it:

- keeps pass ownership clear
- improves ordinary Ari users and compiler contributors
- adds a focused ok or error test
- adds an IR/object/executable check when behavior reaches the backend
- updates docs when it changes language, ABI, module, diagnostic, or artifact
  behavior
- leaves unsupported features rejected with clear diagnostics

The current practical compiler-development maturity is about **45-46%**. Treat
that number as a health metric tied to normal compiler surfaces, not a separate
project goal.
