# Compiler Onboarding

This page is the first-day path for changing the Ari compiler. It is for
contributors who need to become useful quickly without reading every design
note first.

The goal is ordinary hosted compiler development: clearer source handling,
lexer/parser behavior, semantic checks, IR facts, LLVM output, diagnostics,
artifacts, and tests. This is not a bootstrap implementation plan.

## First Day Path

Read in this order:

1. [Compiler Development Dashboard](compiler-development-dashboard.md) for the
   current status, next work, and small checks.
2. [Compiler Contributor Guide](compiler-contributor-guide.md) for the edit map
   and target picker.
3. [Compiler Layer Map](compiler-layer-map.md) for the source files and first
   artifacts owned by each compiler layer.
4. [Compiler Test Authoring](compiler-test-authoring.md) before adding or
   moving a fixture.
5. [Compiler Readiness Inventory](compiler-readiness-inventory.md) for the
   current 46-47% compiler-development estimate.

When writing Ari source for fixtures, keep
[Quick Reference](../language/quick-reference.md),
[Feature Status](../language/feature-status.md), and
[Feature Crosswalk](../language/feature-crosswalk.md) open. They are the
docs-only path for using the language surface that exists today.

## Working Loop

Start from the earliest compiler layer that can know the behavior:

```text
source loading -> lexer -> parser -> module graph -> declarations -> sema -> typed IR -> LLVM -> executable
```

Then use the smallest check that observes that layer:

```text
build/ari path/to/case.ari --check
build/ari path/to/case.ari --emit-llvm build/focused/case.ll
build/ari --explain-pass sema
build/ari --explain-test-bucket compiler-artifact-ok
make check-compiler-dev-docs
make check-compiler-development
make check-compiler-artifacts
```

Full `make check` belongs at handoff for broad compiler changes.
Sanitizer checks are intentionally separate from the normal small documentation
and fixture loop.

## Test Bucket Picker

Choose by behavior, not by the implementation file you edited.

| Bucket | Use For | First Check |
| --- | --- | --- |
| `tests/cases/<feature>/ok/` | Valid language behavior that should compile, emit LLVM, link, or run. | `build/ari path/to/case.ari --check` |
| `tests/cases/<feature>/errors/` | Invalid source that should fail with a stable diagnostic. | A focused failing `--check` command plus a diagnostic assertion. |
| `tests/cases/compiler-development/ok/model/` | Ari programs that model compiler-shaped data, pass flow, docs, readiness, or test policy. | `make check-compiler-development` |
| `tests/cases/compiler-development/artifact/ok/` | Deterministic compiler text artifacts that should match committed goldens. | `make check-compiler-artifacts` |
| `tests/cases/compiler-development/artifact/errors/` | Expected diagnostic artifacts or artifact-comparison reports. | `make check-compiler-artifacts` |
| `tests/cases/compiler-development/errors/` | Compiler-development policy rejections, such as unsupported private syntax. | `make check-compiler-development` |

Name the file after the behavior it protects: `module-private-import.ari`,
`diagnostic-parser-expected.diagnostic`, `compiler-onboarding-workflow.ari`.

## Docs-Only Ari Fixture Path

When a fixture is hard to write from docs alone, improve the public docs or the
general language surface. Do not solve the problem with compiler-only syntax.

Use normal Ari modeling tools:

- `struct` for product data whose fields are meaningful together
- `enum` for exclusive states, outcomes, and result-like flows
- `trait` for shared behavior; Ari has no `class` or `interface` keyword
- `Option[T]` for absence
- `Result[T, E]` for expected compiler failures
- `type` aliases for domain names such as `SourceId`, `Span`, and `TypeId`
- explicit `Zone` values for owned compiler data

If the public language makes a compiler-shaped fixture awkward, record that as
compiler-development pressure in the roadmap instead of hiding it in a private
bootstrap path.

## Working Beside Library Changes

Compiler-design work should stay out of `lib/` unless the requested change is
explicitly a library change.

Before editing or committing, run:

```text
git status --short
```

If unrelated library work is present, leave it alone. Stage only the compiler
docs, tests, and source files that belong to the current change. Do not stage `lib/`
or generated library artifacts just because they are in the same worktree.

## Handoff Checklist

Before handing off a compiler change, make sure the note answers:

- Which compiler layer changed?
- Which public Ari behavior became clearer?
- Which fixture bucket did the test use?
- Which focused check passed?
- Which docs changed?
- Which bootstrap-only or library-only work stayed out of scope?

That keeps Ari moving toward a real compiler while preserving the ordinary
language and library work already happening nearby.
