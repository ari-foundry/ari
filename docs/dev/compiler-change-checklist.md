# Compiler Change Checklist

Use this checklist before handing off a compiler change.

This page is for normal Ari compiler development. It should not justify
private compiler syntax, hidden allocation, or runtime `std` APIs that only
compiler tools need.

## Fast Checklist

Before review, answer these questions:

| Area | Question | Good Answer |
| --- | --- | --- |
| Scope | Which compiler layer owns the change? | One layer is primary; other changes are supporting and explained. |
| Docs | Did user-facing behavior or contributor workflow change? | The focused language or dev page is updated. |
| Tests | Is there one narrow fixture that proves the change? | `ok`, `errors`, `artifact`, or `compiler-development/ok/model` placement is clear. |
| Diagnostics | If failure behavior changed, is the diagnostic stable? | The message has a code-family or golden artifact when appropriate. |
| Sema | Does sema still own source-level facts? | Names, modules, types, traits, and ownership are resolved before backend lowering. |
| IR | Does codegen consume lowered facts mechanically? | Backend code does not re-resolve source-level names or visibility. |
| Non-goals | What did this intentionally avoid? | Private syntax, hidden allocation, and unrelated refactors are explicitly avoided. |

No runtime `std` API should be added only to support compiler source maps,
diagnostics, or golden renderers. Keep those APIs in compiler/tooling layers.

## Test Choice

Use the smallest check that observes the changed behavior:

| Change | First Check |
| --- | --- |
| Docs-only language or navigation update | `make check-language-docs` |
| Compiler roadmap, playbook, readiness, or artifact policy | `make check-compiler-dev-docs` |
| Compiler-shaped Ari model fixture | `make check-compiler-development` |
| Deterministic compiler artifact | `make check-compiler-artifacts` |
| Module loading, visibility, metadata, or cache | `make check-modules` or one module fixture |
| One executable language feature | `build/ari tests/cases/<feature>/ok/<case>.ari --check` |
| One backend lowering shape | `build/ari tests/cases/<feature>/ok/<case>.ari --emit-llvm build/focused/<case>.ll` |

Full `make check` belongs at handoff for broad compiler changes. Sanitizer
checks are valuable for parser, sema, ownership, and codegen internals, but
they are intentionally outside the small checklist loop.

## Documentation Update Rules

Update docs when any of these changes:

- source syntax or accepted forms
- type, ownership, trait, module, or visibility rules
- generated artifact format
- CLI behavior
- ABI, symbol, object, shared-library, or runtime hook behavior
- test placement policy
- compiler maturity estimate or development gate

When a test is clearer than the docs, improve the docs. Tests protect behavior;
docs teach people how to use and maintain it.

## Review Notes Template

```text
Scope:
  <compiler layer and files>

Behavior:
  <user-visible or artifact-visible change>

Tests:
  <narrow command and fixture>

Docs:
  <updated docs or why none needed>

Non-goals:
  <private compiler-only or unrelated work avoided>
```

## Readiness Impact

A change improves the readiness score when it makes normal compiler development
smaller, clearer, or more deterministic. It does not improve the score merely
because it resembles a long-term side project.

Ari is about **45-46% through the current compiler-development maturity work**.
Use this checklist to keep that progress grounded in ordinary compiler quality.
