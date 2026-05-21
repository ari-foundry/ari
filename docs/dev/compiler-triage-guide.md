# Compiler Triage Guide

This page helps contributors route a compiler bug, feature request, or artifact
diff to the first layer that should own it. It is for ordinary hosted compiler
development, not bootstrap implementation.

Treat this as ordinary hosted compiler development: it routes today's C++
compiler work and public Ari behavior, not a private bootstrap tree.

Use it when the symptom is clear but the source file, test bucket, or small
check is not.

## Triage Rule

Start with the earliest layer that can explain the symptom. If an earlier
artifact can show the problem, do not wait for LLVM or executable behavior.

The usual order is:

```text
source path -> tokens -> syntax -> module graph -> declaration index
  -> typed facts -> ownership facts -> typed IR -> LLVM -> object/link/run
```

Executable output is the last signal, not the first one. A frontend regression
should fail near tokens, syntax, diagnostics, modules, declarations, or typed
facts before it reaches a linked binary.

## Symptom Map

| Symptom | First Layer | First Files | First Check |
| --- | --- | --- | --- |
| CLI flag, output path, target, or link invocation is wrong | Driver/toolchain | `src/driver.cpp`, `src/toolchain.cpp` | `make check-cli` or one direct `--emit-llvm` command |
| Bad character, escape, comment, or token spelling | Lexer | `src/lexer.cpp`, `src/token.hpp`, `src/literal.cpp` | one `--emit-tokens` comparison or `make check-compiler-artifacts` |
| Valid syntax is rejected, or invalid syntax parses | Parser | `src/parser.cpp`, `src/ast.hpp`, `src/ast_builders.cpp` | one `--emit-syntax` comparison or a focused parser error fixture |
| File module, import, visibility, or stale metadata is wrong | Module resolver | `src/module_loader.cpp`, `src/module_path.cpp`, `src/module_metadata.cpp` | `make check-modules` or one `--emit-module-graph` comparison |
| Declaration signature, duplicate item, or public surface is wrong | Declaration collection | `src/sema.cpp` plus declaration helpers | one `--emit-declaration-index` comparison |
| Type inference, generic substitution, or aggregate layout is wrong | Type checking | `src/type_semantics.cpp`, `src/type_inference.cpp`, `src/sema.cpp` | `make check-generics` or one `--emit-typed-ir` comparison |
| Trait selection, impl ambiguity, or method dispatch is wrong | Trait checking | `src/trait_semantics.cpp`, `src/type_semantics.cpp` | `make check-traits` plus one focused fixture |
| Move, borrow, drop, or branch state is wrong | Ownership/control flow | `src/ownership_semantics.cpp`, `src/borrow_semantics.cpp`, `src/move_semantics.cpp`, `src/control_flow_semantics.cpp` | one ownership/borrowing error fixture |
| LLVM text is wrong but typed facts look right | IR/backend boundary | `src/ir.hpp`, `src/ir_builders.cpp`, `src/llvm_codegen.cpp` | focused `--emit-llvm` |
| Object, shared library, symbol, or executable behavior is wrong | Backend/toolchain | `src/llvm_codegen.cpp`, `src/symbol_mangle.cpp`, `src/toolchain.cpp` | focused LLVM/object/shared/executable check |
| Docs and tests disagree | Docs/test policy | focused language or dev docs, `tests/Makefile` | `make check-language-docs` or `make check-compiler-dev-docs` |

## Error Placement

Put the test where the first failing layer can see it:

| Failure Kind | Test Bucket |
| --- | --- |
| Accepted program behavior | `tests/cases/<feature>/ok/` |
| Rejected source-level rule | `tests/cases/<feature>/errors/` |
| Compiler-development policy rejection | `tests/cases/compiler-development/errors/` |
| Stage-plan, source-map, token, diagnostic catalog, syntax, module, declaration, typed-IR, or pass-summary text | `tests/cases/compiler-development/artifact/ok/` |
| Expected diagnostic artifact or artifact mismatch report | `tests/cases/compiler-development/artifact/errors/` |
| Compiler-shaped data model pressure | `tests/cases/compiler-development/ok/model/` |

If a test only proves that a linked executable exits differently, ask whether a
token, syntax, module, declaration, typed fact, or LLVM artifact can catch the
same bug earlier.

## Triage Note Template

Use this note shape in issues, handoff notes, or review summaries:

```text
Symptom:
  What failed first?

Earliest layer:
  Which compiler layer can know this behavior?

First file:
  Which source file should be opened first?

Fixture:
  Which ok, errors, artifact, or model test proves the behavior?

Small check:
  Which single command should fail before broad checks?

Non-goal:
  What bootstrap-only shortcut or unrelated refactor is intentionally avoided?
```

The non-goal keeps the fix on ordinary Ari compiler quality. A future
compiler-in-Ari track should benefit from the same public language and
diagnostic improvements as every other Ari tool.

## Escalation Rules

Escalate from a small check only when the symptom crosses a boundary:

- from `--check` to `--emit-llvm` when lowering changed
- from artifact comparison to executable behavior when backend or runtime
  behavior changed
- from one fixture to a focused Make target when the rule affects a feature
  family
- from focused targets to full `make check` only at handoff for broad compiler
  changes

Sanitizer checks are intentionally outside this triage loop. They are useful
for parser, semantic checker, ownership, and codegen internals, but they are
not the first proof for docs, artifacts, or model fixtures.

## Review Checklist

Before closing a triage-driven compiler change, check:

- Did the first failing layer match the symptom?
- Did the fixture live in the closest test bucket?
- Did the diagnostic or artifact make the failure reviewable?
- Did the docs tell the next contributor where to start?
- Did the change avoid bootstrap-only syntax, hidden allocation, and backend
  shortcuts?
