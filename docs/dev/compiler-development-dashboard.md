# Compiler Development Dashboard

This page is the one-page dashboard for ordinary Ari compiler development. It
summarizes where the compiler is now, what should be built next, which small
checks prove progress, and how to keep the work useful for real Ari users and
compiler contributors.

This is not a bootstrap implementation plan. The goal right now is to make the
hosted compiler and the public Ari language strong enough that a future
compiler-in-Ari track would be boring, reviewable, and useful to normal Ari
users too.

## Current Status

Ari is about **38-42% ready** to begin a serious compiler-in-Ari track. Treat
the working seed as **40% ready**, with about **58-62% remaining**.

That percentage is a compiler health metric, not a command to start
bootstrapping. A future bootstrap becomes practical only when normal Ari code
can express large compiler data models, produce stable diagnostics, build
multi-file projects, and compare compiler artifacts before relying on linked
executables.

## Read First

Use this order when joining compiler work:

1. [Compiler Contributor Guide](compiler-contributor-guide.md): where to edit,
   how to choose a focused test, and what counts as progress.
2. [Compiler Concepts Glossary](compiler-concepts-glossary.md): layer terms,
   artifact vocabulary, and review language for first-time compiler work.
3. [Compiler Layer Map](compiler-layer-map.md): which `src/` files own each
   layer, which artifact proves it, and which small check to run first.
4. [Compiler Triage Guide](compiler-triage-guide.md): how to route a symptom,
   bug report, or artifact diff to the first owning compiler layer.
5. [Compiler Source Identity](compiler-source-identity.md): how source files,
   ids, byte spans, line/column lookup, and source-map artifacts should work.
6. [Compiler Module Project Authoring](compiler-module-project-authoring.md):
   how file modules, roots, search paths, metadata, caches, and module graph
   artifacts should be changed.
7. [Compiler Artifact Authoring](compiler-artifact-authoring.md): how to add
   deterministic artifact producers, goldens, normalization, and review rules.
8. [Compiler Diagnostic Authoring](compiler-diagnostic-authoring.md): how to
   choose diagnostic codes, messages, labels, notes, and golden tests.
9. [Compiler Test Authoring](compiler-test-authoring.md): how to choose test
   buckets, names, focused checks, and artifact updates.
10. [Compiler Development Roadmap](compiler-development-roadmap.md): the long
   compiler plan and implementation phases.
11. [Compiler Implementation Playbook](compiler-implementation-playbook.md):
   how to turn roadmap entries into small tickets.
12. [Compiler Next Slices](compiler-next-slices.md): the near-term queue.
13. [Compiler Change Checklist](compiler-change-checklist.md): review and
   handoff checklist for normal compiler changes.
14. [Compiler Readiness Inventory](compiler-readiness-inventory.md): current
   strengths, blocking gaps, scorecard, and development gates.

For language behavior while writing Ari fixtures, pair these with
[Getting Started](../language/getting-started.md),
[Quick Reference](../language/quick-reference.md), and
[Feature Status](../language/feature-status.md). Use
[Feature Crosswalk](../language/feature-crosswalk.md) when you need the exact
example, test family, and small check for a language feature.

## Do Now

| Area | Why It Matters | First Proof |
| --- | --- | --- |
| Source identity | Diagnostics, modules, and artifacts need stable source ownership before the compiler grows more stages. | Source-map golden fixtures and compiler model tests. |
| Diagnostic data | Users and contributors need stable codes, labels, notes, and normalized paths. | `--emit-diagnostics` golden files under `tests/cases/compiler-development/artifact/errors/`. |
| Test classification | Each compiler change should land near the layer it protects. | `ok/model`, `artifact/ok`, `artifact/errors`, and `errors` fixture placement. |
| Artifact comparison | Regressions should fail near tokens, syntax, diagnostics, typed IR, or backend output instead of only at executable behavior. | `make check-compiler-artifacts`. |

## Do Next

| Area | Direction | First Small Check |
| --- | --- | --- |
| File-backed project flow | Harden roots, search paths, `.ari`/`.arih`, visibility, metadata, and cache validation. | One module fixture or `--emit-module-graph` golden. |
| Generic compiler models | Keep nested structs, enums, vectors, maps, and `Result[T, E]` natural in normal Ari. | One fixture under `tests/cases/compiler-development/ok/model/`. |
| Trait and formatting selection | Make `Drop`, `Debug`, formatting, `Eq`, `Ord`, `Hash`, and iterator behavior deterministic. | Focused trait tests plus one compiler-shaped model. |
| HIR and ownership artifacts | Add text artifacts before broadening sema and backend behavior. | Future HIR or ownership-fact golden output. |

## Not Yet

Do not create a real `bootstrap/` tree yet. Do not add bootstrap-only syntax,
hidden global allocation, special compiler-only runtime APIs, or backend hooks
that ordinary Ari programs cannot use.

If compiler-shaped Ari code looks awkward, use that pressure to improve the
general language and compiler:

- use `type` aliases for domain terms like `SourceId`, `Span`, and `TypeId`
- use `Result[T, E]` for expected compiler failures
- use `Option[T]` for absence instead of sentinel values when the type can say
  it directly
- use tuple returns for always-present product values
- use deterministic artifact text before relying on executable behavior

## Small Checks

Run the smallest check that proves the slice you touched:

| Change | Focused Check |
| --- | --- |
| Language docs or feature index | `make check-language-docs` |
| Compiler roadmap, dashboard, or dev docs | `make check-compiler-dev-docs` |
| Compiler-shaped Ari model fixture | `make check-compiler-development` |
| Stage-plan, capability inventory, token, syntax, diagnostic catalog, diagnostic, module, declaration, typed-IR, or pass-summary golden | `make check-compiler-artifacts` |
| Long-term self-host readiness docs only | `make check-bootstrap-docs` |
| One Ari source while iterating | `build/ari path/to/file.ari --check` |

Full `make check` belongs at handoff for broad compiler changes. Sanitizer
checks are intentionally separate, and should be run only when that is the
chosen verification pass.

## Test Classification

Compiler-development tests use these buckets:

| Bucket | Meaning |
| --- | --- |
| `tests/cases/compiler-development/ok/model/` | Ari programs that model compiler-shaped data and pass flow. |
| `tests/cases/compiler-development/artifact/ok/` | Golden artifacts expected to match exactly after normalization. |
| `tests/cases/compiler-development/artifact/errors/` | Golden diagnostics and mismatch reports for expected failures. |
| `tests/cases/compiler-development/errors/` | Source fixtures that should be rejected by the compiler. |

Name each file by the behavior it protects, not by the implementation helper
you happened to edit.

## Compiler Development Gates

Treat these as the gates for making Ari a practical compiler project. They are
useful before any self-hosting plan matters:

| Gate | Green Signal |
| --- | --- |
| Source identity | Every diagnostic and artifact can point to stable files, byte ranges, and line/column positions. |
| Diagnostics | Expected failures have stable codes, labels, notes, and golden rendering. |
| Project flow | A multi-directory Ari tool builds from Make with explicit paths and no hidden stage flags. |
| Compiler data models | Compiler-shaped structs, enums, generics, `Option`, `Result`, and vectors compile naturally. |
| Artifact order | Tokens, syntax, diagnostics, HIR, typed IR, LLVM, object symbols, and executable behavior can be compared in order. |
| Developer loop | A new contributor can find the right source file and run one focused check from docs alone. |

## Handoff Note

For a normal compiler change, leave a short note with:

- the compiler layer changed
- the public Ari behavior clarified
- the focused check run
- the docs or test bucket updated
- the explicit non-goal when a request could drift into unrelated work

That keeps Ari moving toward a real compiler without turning every improvement
into a hidden self-hosting shortcut.
