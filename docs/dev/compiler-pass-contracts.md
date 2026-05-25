# Compiler Pass Contracts

This page defines the contract between Ari compiler passes. These boundaries
make today's compiler easier to maintain and review.

Use this page with [Architecture](architecture.md),
[Compiler Pipeline](compiler-pipeline.md),
[Compiler Layer Map](compiler-layer-map.md),
[Compiler Readiness Inventory](compiler-readiness-inventory.md),
[Compiler Source Identity](compiler-source-identity.md),
[Compiler Diagnostic Authoring](compiler-diagnostic-authoring.md), and
[Compiler Artifact Authoring](compiler-artifact-authoring.md).

## Goals

Compiler code stays maintainable when each pass has a narrow job:

- lexer produces tokens with spans, not syntax decisions
- parser produces source-shaped AST, not type facts
- resolver owns module paths, imports, visibility, and symbol ids
- semantic checking owns types, ownership, traits, and lowering choices
- typed IR carries resolved facts so codegen stays mechanical
- backend codegen emits target artifacts without re-resolving source names
- diagnostics are built from source spans and pass-local facts, not backend
  accidents

If a later pass needs to rediscover something an earlier pass already knew, the
boundary is missing data. Add that data to the pass output instead of reaching
back into source-level state.

## Pass Map

The long-term pass shape should be explicit even while the current compiler
still combines some of these responsibilities inside `src/sema.cpp`:

| Pass | Input | Output | Owns |
| --- | --- | --- | --- |
| Driver | CLI args and source paths | Source text and build options | file IO, target flags, module roots |
| Lexer | Source text | Token stream | token kinds, byte spans, lexical diagnostics |
| Parser | Token stream | AST | grammar, recovery, source-shaped syntax |
| Declaration collection | AST modules | Declaration tables | item names, visibility shells, duplicate checks |
| Module resolver | Declaration tables and module paths | Module graph | imports, `use`, `pub`, search paths, cache inputs |
| HIR lowering | AST plus module graph | HIR | syntax desugaring, stable ids, simpler compiler model |
| Type checking | HIR | Typed HIR or typed IR facts | inference, aliases, generics, traits, layout queries |
| Ownership checking | Typed HIR facts | move/borrow/drop facts | owner states, borrow provenance, destructor insertion |
| IR lowering | Typed facts | typed IR | backend-ready control flow, calls, layouts, runtime hooks |
| LLVM backend | typed IR | LLVM IR/object/executable | target-specific text and driver invocation |

Today, Ari goes from parser AST into semantic checking and directly into typed
IR. That is fine for a hosted prototype, but the named middle passes should
guide refactors so the compiler grows in the right direction.

## Boundary Rules

Use these rules when changing compiler code:

- The lexer must not know whether an identifier is a type, value, module, or
  keyword-like macro name after tokenization.
- The parser must not ask semantic questions. It should keep AST nodes
  source-shaped and preserve enough syntax for diagnostics.
- Declaration collection may discover names and duplicates, but it must not
  type-check function bodies.
- Module resolution owns file-backed modules, package roots, visibility,
  aliases, and import graph diagnostics.
- HIR lowering may simplify syntax, but it should not pick target ABI layouts.
- Type checking owns type aliases, generic substitutions, trait obligations,
  and expected-type propagation.
- Ownership checking owns moves, drops, borrows, and lifetime-like provenance.
- IR lowering owns the decision to turn typed compiler facts into backend-ready
  IR nodes.
- LLVM codegen must not re-resolve Ari source paths, visibility, traits, enum
  cases, or overload-like choices.

The shortest review question is: "Which pass should have known this first?" If
the answer is not the pass being edited, move the data to the pass boundary.

## Data Ownership

Compiler data should flow forward:

```text
SourceFile
  -> Token { kind, span }
  -> AstNode { kind, span, children }
  -> SymbolId / ModuleId / ImportId
  -> HirNode { stable id, source span, resolved names }
  -> TypeId / LayoutId / TraitImplId
  -> OwnershipFact / BorrowFact / DropAction
  -> IrFunction / IrBlock / IrValue
  -> LLVM text and linked artifacts
```

Source-facing ids should remain stable enough for golden tests. Backend ids may
be normalized by artifact tests, but source spans and symbol names should still
trace back to the original file and module.

Avoid storing raw pointers to temporary pass data across pass boundaries. Prefer
stable ids, indexes, or explicitly owned arena values so dumps and diagnostics
can be deterministic.

## Diagnostic Contract

Each pass should report the error closest to the user's source:

- lexer: invalid byte, invalid escape, unterminated literal or comment
- parser: expected token, malformed declaration, unmatched delimiter
- declaration collection: duplicate item, invalid item placement
- module resolver: missing module, private item, ambiguous import, stale cache
- type checker: unknown type, mismatch, uninferred generic, trait failure
- ownership checker: use after move, live owner escape, borrow conflict
- IR lowering: language feature checked but not yet executable
- backend: target ABI, linker, object, or LLVM-driver failure

Diagnostics should carry a code, primary span, optional labels, notes, and a
pass name. The backend should only produce source diagnostics when the failure
is genuinely backend-specific. If a type or visibility error reaches LLVM
codegen, an earlier pass missed its job.

## Artifact Contract

Each major pass should eventually have a stable dump format:

```text
tokens      lexer output
syntax      parser AST output
modules     resolved module graph
hir         lowered source model
typed-hir   type and trait facts
ownership   move/borrow/drop facts
ir          backend-ready typed IR
llvm        normalized LLVM text
symbols     object/shared-library symbol surface
```

Artifact tests should compare earlier dumps before later ones. A syntax
regression should not first appear as an executable failure. See
[Compiler Artifact Authoring](compiler-artifact-authoring.md) for normalization
and golden file policy.

The compiler also exposes the pass contract table directly:

```text
build/ari --list-passes
build/ari --explain-pass sema
```

Use that CLI before widening a test. It records the pass owner, input, output,
first artifact, and first focused check in the same language used by this page.

## Implementation Slices

These slices are safe compiler-development work:

1. Token dump: expose token kind, spelling class, and span in a deterministic
   text artifact.
2. Syntax dump: expose AST shape and source spans without type facts.
3. Module graph dump: expose module ids, resolved paths, imports, visibility,
   and cache hits.
4. Declaration index dump: expose declaration signatures, visibility, module
   names, and source locations before type checking.
5. Declaration table split: move duplicate/name collection toward a separate
   helper while leaving body checking in sema.
6. HIR sketch: define the minimal lowered node vocabulary before implementing
   a full HIR pass.
7. Typed fact dump: expose selected type, layout, trait, and generic
   specialization facts after sema.
8. Ownership fact dump: add an ownership fact dump that exposes owner states,
   borrow sources, and inserted drop actions for focused tests.
9. IR contract audit: find places where `src/llvm_codegen.cpp` re-derives
   source facts and move those facts into `src/ir.hpp`.

These make the compiler easier to test and review now.

## Test Layout

Compiler pass tests should say which pass they protect:

```text
tests/cases/modules/ok/
tests/cases/modules/errors/
tests/cases/ir/ok/
tests/cases/bootstrap-readiness/ok/
tests/cases/compiler-development/artifact/ok/
tests/cases/compiler-development/artifact/errors/
```

Future tools can split into dedicated `lex`, `parse`, `hir`, and `ir` folders
under their own tool tree. Keep current compiler artifacts in
`tests/cases/compiler-development/artifact/` and ordinary language behavior in
the feature folder that owns it.

Use names that encode the behavior:

- `tokens-char-literal.ari`
- `syntax-function-patterns.ari`
- `modules-private-reexport.ari`
- `typed-ir-enum-layout.ari`
- `ownership-drop-insertion.ari`
- `llvm-extern-symbol.ari`

Small focused checks are preferred while editing one pass:

```text
build/ari tests/cases/modules/ok/module-llvm.ari --check
build/ari tests/cases/control-flow/ok/if-expression.ari --emit-llvm build/focused/if.ll
make check-compiler-docs
```

Run broad checks before handoff for broad compiler changes. Sanitizer checks
are intentionally outside this page's required small-slice loop.

## Review Checklist

Before accepting a compiler pass change, check:

- Does the pass output contain every fact the next pass needs?
- Did the change avoid making a later pass re-resolve source names or types?
- Is the diagnostic produced by the earliest pass that can know the error?
- Is there a focused ok or error test for the pass being changed?
- If LLVM text changes, is the source of the change visible in typed IR or a
  documented lowering rule?
- Does the change help ordinary Ari language implementation quality?

## Readiness Impact

Pass-contract work should move the compiler-development maturity score only
when it becomes executable checks. Its main value is making the compiler easier
to split, test, and review.

The estimate should improve only when these contracts become executable checks:
token dumps, syntax dumps, module graph dumps, typed fact dumps, ownership fact
dumps, and IR audits that remove source re-resolution from backend codegen.
