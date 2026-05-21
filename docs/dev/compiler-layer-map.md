# Compiler Layer Map

This page maps Ari compiler layers to the source files, artifacts, docs, and
small checks a contributor should open first. It is about ordinary hosted
compiler development.

Treat this as an ordinary hosted compiler development map: it points to today's
C++ source files and public Ari behavior.

Use it when you know the compiler behavior that changed but do not yet know
where to edit or how to prove the change.

## How To Use This Map

Start with the earliest layer that can know the behavior:

1. Find the source row for the behavior.
2. Run `build/ari --list-passes` or `build/ari --explain-pass sema` when the
   boundary, owner, input, or first artifact is unclear.
3. Read the linked contract page before editing.
4. Add or update the closest `ok`, `errors`, `artifact`, or model fixture.
5. Run the smallest check listed for that layer.
6. Update user docs if the public Ari rule changed.

If a later layer needs information an earlier layer already knew, move that
information forward through AST, module metadata, typed IR, or artifacts.
Do not make LLVM codegen re-resolve Ari source-level names.

## Source Files At A Glance

| Layer | First Files | Owns | First Artifact |
| --- | --- | --- | --- |
| Driver | `src/driver.cpp`, `src/toolchain.cpp` | CLI flags, file IO, target options, artifact paths, LLVM driver calls, capability inventory. | capability inventory, source map, pass summary, linked output |
| Lexer | `src/lexer.cpp`, `src/token.hpp`, `src/literal.cpp` | token kinds, spelling classes, comments, escapes, lexical byte spans. | token dump |
| Parser | `src/parser.cpp`, `src/ast.hpp`, `src/ast_builders.cpp` | grammar, AST shape, parser recovery, source-shaped syntax. | syntax dump |
| Module loading | `src/module_loader.cpp`, `src/module_path.cpp`, `src/module_metadata.cpp` | file-backed modules, search paths, `.ari`/`.arih`, imports, metadata, cache inputs. | module graph |
| Declaration and names | `src/sema.cpp`, declaration helpers, module helpers | item shells, duplicate names, `pub`, `use`, aliases, qualified paths. | declaration index |
| Type and traits | `src/type_semantics.cpp`, `src/type_inference.cpp`, `src/trait_semantics.cpp` | type facts, generics, trait obligations, associated items, layouts. | typed IR |
| Ownership and control flow | `src/ownership_semantics.cpp`, `src/borrow_semantics.cpp`, `src/move_semantics.cpp`, `src/control_flow_semantics.cpp` | moves, borrows, drops, loop state, branch state, pattern state. | future ownership fact dump |
| IR model | `src/ir.hpp`, `src/ir_builders.cpp` | backend-ready functions, blocks, values, calls, layouts, resolved symbol facts. | typed IR and LLVM smoke |
| LLVM backend | `src/llvm_codegen.cpp`, `src/symbol_mangle.cpp` | LLVM text, ABI lowering, symbol names, object/shared/executable output. | LLVM, object, executable |

The current hosted prototype still has several boundaries inside `src/sema.cpp`.
That is acceptable while the compiler is small, but new behavior should move
toward explicit helper files and forward-flowing data.

## Layer Contracts

| Layer | Should Do | Should Not Do |
| --- | --- | --- |
| Driver | Normalize input paths, choose target options, request artifacts, and invoke the LLVM driver. | Decide whether a name is a type, value, or module item. |
| Lexer | Produce tokens with deterministic spans and lexical diagnostics. | Peek into declarations, imports, types, or traits. |
| Parser | Preserve source shape in AST and reject malformed grammar early. | Ask type, visibility, ownership, or layout questions. |
| Module loading | Resolve file-backed modules and build deterministic module inputs. | Type-check function bodies. |
| Declaration and names | Build visible item shells and resolve import/name surfaces. | Lower backend calls or choose LLVM symbols. |
| Type and traits | Produce type, generic, trait, and layout facts. | Emit LLVM or recreate parser syntax. |
| Ownership and control flow | Track move/borrow/drop state and branch compatibility. | Depend on backend artifacts to detect source-level ownership errors. |
| IR model | Store every resolved fact codegen needs. | Require codegen to walk source AST to rediscover names. |
| LLVM backend | Emit target artifacts from typed IR. | Re-resolve Ari modules, visibility, traits, enum cases, or overload-like choices. |

## First Checks By Layer

| Layer | First Check |
| --- | --- |
| Driver and CLI | `make check-cli` or one direct `build/ari file.ari --emit-llvm build/focused/file.ll` |
| Pass ownership and routing | `build/ari --list-passes` or `build/ari --explain-pass sema` |
| Lexer artifact | `make check-compiler-artifacts` or one `--emit-tokens` comparison |
| Parser artifact | `make check-compiler-artifacts` or one `--emit-syntax` comparison |
| Modules | `make check-modules` or one `--emit-module-graph` comparison |
| Declaration surface | one `--emit-declaration-index` comparison |
| Types and generics | `make check-generics` or one focused `--check` fixture |
| Traits | `make check-traits` or one focused trait fixture |
| Ownership and borrowing | one `tests/cases/ownership/` or `tests/cases/borrowing/` fixture |
| Compiler model pressure | `make check-compiler-development` |
| Compiler artifacts | `make check-compiler-artifacts` |
| Developer docs | `make check-compiler-dev-docs` |

Full `make check` belongs at handoff for broad changes. Sanitizer checks are
useful for low-level compiler internals, but they are intentionally separate
from this small-slice loop.

## Documentation Updates By Layer

| Changed Surface | Update |
| --- | --- |
| User syntax or semantics | language docs plus [Feature Status](../language/feature-status.md) and [Feature Crosswalk](../language/feature-crosswalk.md) |
| Pass boundary or source file ownership | this page and [Compiler Pass Contracts](compiler-pass-contracts.md) |
| Module search, metadata, or project layout | [Compiler Module Project Authoring](compiler-module-project-authoring.md) and [Compiler Project Model](compiler-project-model.md) |
| Diagnostic code, label, note, or renderer | [Compiler Diagnostic Authoring](compiler-diagnostic-authoring.md) and artifact goldens |
| New artifact format | [Compiler Artifact Authoring](compiler-artifact-authoring.md) and [Compiler Artifact Testing](compiler-artifact-testing.md) |
| Test bucket or target | [Compiler Test Authoring](compiler-test-authoring.md), `tests/README.md`, and `tests/Makefile` |
| Maturity estimate or development gate | [Compiler Readiness Inventory](compiler-readiness-inventory.md) and [Compiler Maturity Gates](compiler-maturity-gates.md) |

## Adding A New Layer

Add a new compiler layer only when it removes real ambiguity:

1. Name the input and output data.
2. Decide which existing layer stops owning that responsibility.
3. Add a deterministic artifact or model fixture before broad executable tests.
4. Wire a focused Make target or extend the closest existing one.
5. Update this map, pass contracts, and the readiness inventory.

Good new layers make reviews smaller. A new layer is not healthy if it only
moves code around while keeping the same hidden cross-pass dependencies.

## Review Checklist

Before handing off a compiler-layer change, check:

- Did the earliest capable layer reject the invalid program?
- Did the next layer receive data instead of re-reading source state?
- Is there a small fixture in the closest test bucket?
- Is the artifact order still capability inventory, token, syntax, module,
  declaration, typed facts, LLVM, then executable behavior?
- Did docs tell a new contributor which file and check to use next?
- Is the change normal Ari compiler development rather than private machinery?
