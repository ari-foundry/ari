# Compiler Concepts Glossary

This page explains the compiler terms used across the Ari developer docs. It is
for contributors who are new to compiler work or new to this codebase.

This is not a bootstrap implementation plan. These concepts describe the hosted
C++ compiler today and the normal compiler-development shape Ari should keep
improving before any future compiler-in-Ari track starts.

## Reading Rule

When a compiler term appears in a roadmap or test name, ask three questions:

1. Which compiler layer owns this value?
2. Which file or artifact proves it changed correctly?
3. Which user-facing Ari rule became clearer?

If a change cannot answer those questions, it is probably too broad or too
hidden to review well.

## Frontend Terms

| Term | Meaning | Current Home |
| --- | --- | --- |
| Source file | One loaded Ari input with a stable path and bytes. | `src/driver.cpp`, source-map artifacts. |
| `SourceId` | A small identity for a loaded source file. | Planned compiler/tooling source model. |
| Byte span | Half-open byte range in a source file. | Lexer/parser diagnostics and source-map fixtures. |
| Token | The lexer output for one syntactic item, such as an identifier or number. | `src/lexer.cpp`, `--emit-tokens`. |
| AST | The parser output shaped like the source syntax. | `src/parser.cpp`, `src/ast.hpp`, `--emit-syntax`. |
| Declaration | A top-level item surface such as a function, type, module, trait, or impl. | Declaration-index artifacts. |

Frontend work should preserve source facts. Do not make the parser guess type
meaning that belongs to sema.

## Middle-End Terms

| Term | Meaning | Current Home |
| --- | --- | --- |
| Name resolution | Mapping source names to modules, functions, types, fields, methods, and traits. | `src/sema.cpp`. |
| Sema | Semantic analysis: names, types, modules, ownership, traits, and lowering decisions. | `src/sema.cpp`. |
| HIR | A future lowered tree between AST and typed IR. | Planned artifact stage. |
| Type fact | A resolved type decision sema should pass forward. | `src/ir.hpp` and sema lowering. |
| Ownership fact | A move, borrow, drop, or lifetime decision checked before backend lowering. | Sema ownership checks. |
| Diagnostic | Structured error data: code, severity, labels, notes, and source spans. | Diagnostic docs and `--emit-diagnostics`. |

Middle-end work should keep source-level resolution in sema. If codegen needs
to rediscover a source name, sema probably failed to lower enough metadata.

## Backend Terms

| Term | Meaning | Current Home |
| --- | --- | --- |
| Typed IR | Ari's resolved, backend-facing program model. | `src/ir.hpp`, `--emit-typed-ir`. |
| LLVM IR | Textual LLVM program emitted from typed IR. | `src/llvm_codegen.cpp`, `--emit-llvm`. |
| Object file | Relocatable output from the LLVM driver. | `--emit-obj`. |
| Executable | Linked program produced through the LLVM path. | Default `build/ari input.ari -o out`. |
| Shared library | Exportable dynamic library built through the LLVM path. | `--shared`. |
| Symbol mangling | Stable encoding of Ari names into backend symbols. | `src/symbol_mangle.cpp`. |

Backend work should be mechanical: typed IR in, deterministic target artifact
out. Backend tests should inspect LLVM, objects, symbols, or executable behavior
according to the smallest affected layer.

## Testing Terms

| Term | Meaning |
| --- | --- |
| `ok/` fixture | Source that should compile, emit an artifact, or run. |
| `errors/` fixture | Source that should fail with a stable diagnostic. |
| Artifact fixture | Committed expected output from a compiler stage. |
| Golden file | A text artifact checked by exact or normalized comparison. |
| Model fixture | A normal Ari program that models compiler-shaped data or pass flow. |
| Focused check | The smallest command that proves the changed layer. |
| Handoff check | Broader verification before sharing a large change. |

Use `make check-compiler-development` for model fixtures and
`make check-compiler-artifacts` for stage artifacts. Full `make check` belongs
at handoff. Sanitizer checks are intentionally separate.

## Layer Ownership

| Layer | Owns | Should Not Own |
| --- | --- | --- |
| Lexer | bytes, characters, tokens, token spans | type checking |
| Parser | AST shape, grammar errors, syntax recovery | trait selection |
| Resolver | module paths, imports, source item identity | LLVM details |
| Sema | types, traits, ownership, lowering facts | final symbol text |
| Typed IR | resolved backend-facing facts | source grammar decisions |
| LLVM backend | LLVM text, objects, executables, shared libraries | source-level name lookup |

The maintenance rule is simple: each layer should consume the previous layer's
data and produce its own reviewable output. Hidden cross-layer shortcuts make
the compiler harder to test.

## Review Vocabulary

Use these phrases in review notes:

- Source identity: source ids, paths, byte spans, line/column lookup, snippets.
- Diagnostic stability: codes, labels, notes, normalized paths, deterministic
  rendering.
- Pass boundary: the exact input and output of a compiler stage.
- Lowered fact: metadata sema passed to IR so codegen stays mechanical.
- Artifact order: compare source maps, tokens, syntax, diagnostics, HIR, typed
  IR, LLVM, objects, and executable behavior in that order.
- Public language pressure: a compiler-shaped need that should improve Ari for
  all users, not just future self-hosting work.

## Example Change Shape

A good compiler change usually looks like this:

1. Pick one layer, such as parser diagnostics or typed IR metadata.
2. Add or update one focused fixture.
3. Update the user-facing language page or developer doc that explains the
   behavior.
4. Run the smallest matching check.
5. Leave self-hosting readiness as a secondary metric, not the current
   implementation target.

That rhythm keeps Ari moving toward a real compiler while preserving a clean
language design for ordinary programs.
