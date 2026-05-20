# Compiler Implementation Playbook

This page turns the compiler roadmap into small implementation tickets.

It is not a bootstrap plan. Use it when you are changing the hosted compiler
today and want a practical answer to:

- which source files should I read first?
- which artifact should prove the change?
- which small test target should I run before broad checks?
- what makes the work ready for review?

For the larger direction, read
[Compiler Development Roadmap](compiler-development-roadmap.md). For pass
ownership rules, read [Compiler Pass Contracts](compiler-pass-contracts.md).
For the readiness estimate, read
[Compiler Readiness Inventory](compiler-readiness-inventory.md).
For a concrete queue of near-term tickets, read
[Compiler Next Slices](compiler-next-slices.md).
Before handoff, use [Compiler Change Checklist](compiler-change-checklist.md).

## Work Selection

Pick work in this order unless a bug report points somewhere sharper:

1. Make source identity and diagnostics more deterministic.
2. Make one compiler layer observable through a small artifact.
3. Harden module/project behavior that large Ari tools already need.
4. Reduce special cases in generic aggregate and trait-heavy code.
5. Move resolved facts from sema into IR when codegen is re-deriving them.
6. Add focused backend checks only after earlier artifacts explain the change.

Do not start by writing a new compiler in Ari. A future Ari-written compiler is
useful only if the ordinary hosted compiler and public language are already
pleasant enough for large programs.

## Implementation Slices

| Slice | First Files | Artifact Or Fixture | Small Check | Done When |
| --- | --- | --- | --- | --- |
| Source identity | `src/driver.cpp`, `src/source_map_dump.cpp`, module loader code | `--emit-source-map` golden | `make check-compiler-artifacts` | Byte offsets, line starts, filenames, and snippets are deterministic. |
| Diagnostics | lexer/parser/sema throw sites, `src/diagnostic_dump.cpp` | `--emit-diagnostics` golden | `make check-compiler-artifacts` | Expected failures have stable codes and normalized text. |
| Lexer/parser surface | `src/lexer.cpp`, `src/parser.cpp`, `src/ast.hpp` | token or syntax dump | focused `build/ari case.ari --check` | New syntax has ok and error coverage plus docs. |
| Module flow | `src/module_loader.cpp`, `src/module_metadata.cpp`, `src/module_graph_dump.cpp` | module graph or metadata golden | `make check-modules` or one module fixture | Missing/private/stale module behavior is predictable. |
| Sema boundary | `src/sema.cpp` and the relevant `*_semantics.cpp` helper | compiler-model fixture or typed IR | `make check-compiler-development` | One semantic area owns the rule and diagnostics name the source construct. |
| Generic data models | type, trait, layout, enum payload helpers | `tests/cases/compiler-development/ok/model/` fixture | `make check-compiler-development` | Compiler-shaped data uses normal Ari structs, enums, `Result`, and containers. |
| IR contract | `src/ir.hpp`, `src/ir_builders.cpp`, `src/llvm_codegen.cpp` | `--emit-typed-ir` or LLVM text | focused `--emit-llvm` | LLVM codegen consumes resolved facts instead of re-resolving source names. |
| Backend artifact | `src/llvm_codegen.cpp`, `src/toolchain.cpp`, ABI helpers | LLVM/object/shared-library check | focused `--emit-llvm` or `--emit-obj` | ABI, visibility, symbols, and runtime hooks are reviewable as small artifacts. |

## Ticket Template

Use this shape for compiler implementation notes and PR descriptions:

```text
Problem:
  What compiler behavior is unclear, unstable, or hard to test?

Scope:
  Which compiler layer owns the change?

Implementation:
  Which files carry the new source fact, semantic rule, IR fact, or artifact?

Tests:
  Which ok/error/model/artifact fixture proves it?

Docs:
  Which language or developer page changed?

Non-goals:
  What bootstrap-only shortcut or unrelated refactor did this avoid?
```

The `Non-goals` line matters. It keeps compiler development focused on
ordinary Ari language quality instead of drifting into private stage1 behavior.

## Test Placement

Choose the test folder by what the fixture proves:

| What Changed | Put The Test Here |
| --- | --- |
| User-visible language behavior | `tests/cases/<feature>/ok/` or `tests/cases/<feature>/errors/` |
| Compiler-shaped data model | `tests/cases/compiler-development/ok/model/` |
| Deterministic compiler artifact | `tests/cases/compiler-development/artifact/ok/` |
| Expected compiler diagnostic artifact | `tests/cases/compiler-development/artifact/errors/` |
| Later bootstrap start-gate pressure | `tests/cases/bootstrap-readiness/` |

Prefer one reason per fixture. If a test starts proving parser recovery,
generic monomorphization, ownership cleanup, and LLVM layout at once, split it.

## Review Checklist

Before calling a compiler implementation slice done:

- The source-level rule is documented or intentionally internal.
- Unsupported syntax still fails with a clear diagnostic.
- Sema owns source-level name, module, type, trait, and ownership decisions.
- IR carries facts that backend codegen needs mechanically.
- The smallest relevant check passes.
- A broader target is reserved for handoff, not for every edit loop.
- No runtime `std` API was added just for compiler source maps or diagnostics.
- No bootstrap-only keyword, hidden allocation path, or private backend hook was
  added.

## Readiness Impact

This playbook moves the readiness score by making ordinary compiler work
repeatable. It helps most when a change adds one of these:

- a more deterministic source or diagnostic artifact
- a smaller compiler fixture that models real pass data
- a clearer module/project failure mode
- a generic aggregate or trait case that removes awkward compiler-shaped code
- an IR fact that keeps LLVM lowering mechanical

Ari is still about **38-42% ready** to start a serious compiler-in-Ari track.
The useful work today is to make each normal compiler change smaller, clearer,
and easier to test.
