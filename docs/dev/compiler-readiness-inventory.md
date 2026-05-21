# Compiler Readiness Inventory

This page is the inventory for Ari compiler development. It answers one
question: what has to become true before starting an Ari-written compiler track
is a good engineering decision?

This is not a bootstrap implementation plan. The immediate work is still normal
compiler development: make the hosted compiler reliable, make the language
pleasant for large programs, and make compiler behavior easy to test in small
pieces.

In short: normal compiler development comes first.

## Current Readiness

Ari is about **38-42% ready** to begin a serious compiler-in-Ari track. That
leaves about **58-62% remaining** before self-hosting work is likely to be
productive.

The number is conservative because Ari already compiles useful programs, but a
compiler is a large program with hard requirements: stable diagnostics,
multi-file project flow, data-heavy generic models, deterministic artifacts,
and tests that fail near the layer that regressed.

## Readiness Scorecard

The 38-42% estimate is a weighted engineering score, not a promise that
self-hosting is almost ready. The current seed fixture
`tests/cases/compiler-development/ok/model/compiler-readiness-scorecard.ari`
models the same shape and returns `40`, keeping the estimate executable as
ordinary Ari data.

| Gate | Weight | Current Score | What Moves It |
| --- | --- | --- | --- |
| Frontend reliability | 10 | 55 | Span-aware lexer/parser errors, recovery fixtures, and syntax docs that match parser behavior. |
| Source identity | 12 | 30 | Owned source files, stable `SourceId`, byte spans, line/column lookup, and snippets. |
| Diagnostics | 13 | 30 | Diagnostic codes, labels, notes, source rendering, and normalized golden output. |
| Module projects | 12 | 45 | Predictable package roots, `.ari`/`.arih` policy, visibility errors, metadata, and cache invalidation. |
| Compiler data models | 15 | 50 | Nested generic aggregates, `Result` payloads, vectors/maps/sets, and compiler-shaped ownership patterns. |
| Trait selection | 12 | 35 | Deterministic `Drop`, `Debug`, formatting, `Eq`, `Ord`, `Hash`, and iterator dispatch. |
| Artifact comparison | 16 | 45 | Token, syntax, diagnostic, module graph, declaration, typed IR, HIR, LLVM, object, and executable comparison order. |
| Tool build flow | 10 | 35 | Focused Make targets for one Ari tool, fixture roots, and golden comparison without hidden flags. |

Weighted together, this lands at roughly 40%. Treat each row as normal compiler
development: a row moves when ordinary Ari programs, diagnostics, or artifacts
get more reliable, not when a private bootstrap shortcut is added.

## Already Strong

| Area | Current Strength | Why It Matters |
| --- | --- | --- |
| Hosted LLVM backend | Ari emits LLVM IR, objects, executables, and shared libraries through the LLVM path. | Compiler work can be validated as real artifacts instead of toy interpreter behavior. |
| Core executable language | Functions, locals, control flow, integers, bools, enums, structs, modules, FFI, and formatting are available. | Compiler-shaped fixtures can be written as ordinary Ari programs. |
| Generic calls and ADTs | Generic functions, generic structs, generic enums, `Option[T]`, and `Result[T, E]` exist. | Source models, tokens, AST nodes, and expected failures can use natural types. |
| Ownership checks | Move, borrow, drop, and explicit-zone checks catch many unsafe flows. | Large compiler graphs can be kept explicit instead of hiding allocation in globals. |
| Focused test culture | Many feature folders already separate `ok` and `errors` tests. | New compiler behavior can be guarded with one small fixture at a time. |

## Blocking Gaps

| Gap | Needed State | First Work |
| --- | --- | --- |
| Source identity | Stable `SourceId`, byte spans, line/column lookup, and snippets for every diagnostic. | Add compiler/tooling source-map fixtures and golden source rendering checks. |
| Diagnostics | Stable codes, labels, notes, and normalized golden output. | Move errors toward data-first diagnostics before polishing renderer text. |
| File-backed projects | Predictable module roots, `.ari`/`.arih` policy, metadata, cache invalidation, and Makefile flows. | Harden module search and add stale/private/missing file diagnostics. |
| Generic aggregate scale | Nested structs, enums, vectors, maps, sets, and `Result` payloads need fewer edge cases. | Keep adding compiler-shaped model fixtures under `tests/cases/compiler-development/ok/model/`. |
| Trait selection | `Eq`, `Ord`, `Hash`, `Debug`, formatting, `Drop`, and iterator dispatch need stable ambiguity errors. | Add positive dispatch tests and negative ambiguous-impl tests. |
| Pass artifacts | Token, syntax, HIR, typed IR, LLVM, object, executable, and shared outputs need a comparison order. | Add normalized text dumps before broad executable checks. |
| Build ergonomics | Large Ari tools need boring Make targets before a package manager exists. | Keep `make check-compiler-development` small and add one target per compiler slice. |

## Recent Compiler Support

Mixed aggregate enum payload slots now use byte storage when no single payload
type can safely represent every case. This lets normal compiler-shaped values
such as `Result[Span, DiagnosticBuildError]` compile without replacing rich
payloads with numeric sentinels. The LLVM backend materializes those slots by
storing the source payload into scratch byte storage and loading the active
payload type back out only after the enum tag has selected the case.

Postfix `?` can now propagate residual payloads through aggregate enum layouts.
This lets compiler-shaped functions keep natural code such as
`let checked = validate_span(file, span)?;` even when the success payload type
changes between `Result[Span, E]` and `Result[LineColumnRange, E]`. The backend
extracts the active residual payload from the operand enum slot, re-materializes
the function return enum, and uses the same byte-storage cast path as match
payload bindings.

This is deliberately a general language feature. It is useful for any large
Ari program that returns structured errors, not only for a future compiler
written in Ari.

The hosted compiler now also has the first artifact producers:
`--emit-stage-plan path`, `--emit-source-map path`, `--emit-tokens path`, `--emit-syntax path`,
`--emit-diagnostics path`, `--emit-module-graph path`,
`--emit-declaration-index path`, `--emit-typed-ir path`, and
`--emit-pass-summary path`. They write deterministic stage order and first-check
routing, source byte/line tables, lexer token text, parser tree text, expected-failure diagnostic text,
file-backed source/import/item graph text, declaration signature text,
sema-lowered typed IR, and stage-boundary counts, all checked by
`make check-compiler-artifacts`. This is the first small stage-comparison step
for normal compiler development: when source loading, lexer, parser,
diagnostic, module, declaration surface, or typed lowering behavior changes,
reviewers can inspect a tiny golden diff before LLVM or executable behavior
changes are involved.
Diagnostic artifacts now include both stable codes and explicit layer families
such as `code=T0001 family=type`, so triage can route failures without reading
the classifier implementation first.

## Development Backlog

Use this order for general compiler development:

1. Frontend contracts: lexer/parser span accuracy, literal behavior, recovery,
   and malformed syntax diagnostics.
2. Source model: `SourceFile`, `SourceId`, `Span`, byte offsets, line/column
   conversion, and snippet extraction as compiler/tooling concepts.
3. Diagnostic model: diagnostic codes, severity, primary/secondary labels,
   notes, stable sorting, and path normalization.
4. Module projects: file-backed modules, project roots, header/source
   separation, module metadata, and module caches.
5. Type and trait maturity: generic aggregate monomorphization, associated
   types, trait selection, and clear ambiguity diagnostics.
6. Ownership scale: arena/zone patterns for compiler graphs, borrowed views,
   scratch reset rules, and predictable drops.
7. IR contract: lower resolved facts into typed IR so LLVM codegen is mostly a
   mechanical emitter.
8. Artifact testing: normalize and compare token, diagnostic, syntax, HIR,
   typed IR, LLVM, object symbol, and executable outputs.

## Compiler Development Gates

Use these gates to decide whether Ari is becoming a practical compiler project.
They also inform a future compiler-in-Ari track, but their first job is to keep
today's compiler reliable and pleasant to extend:

| Gate | Green Signal |
| --- | --- |
| Frontend reliability | New syntax has ok tests, error tests, docs, and span-aware diagnostics. |
| Compiler data models | Compiler-shaped structs/enums/generic containers compile without awkward casts or hidden runtime hooks. |
| Diagnostic stability | Expected failures can be reviewed as normalized text artifacts. |
| Module project flow | A multi-directory Ari tool builds from Make with explicit search paths and no hidden stage flags. |
| Artifact comparison | The project can compare earlier artifacts before relying on linked executable behavior. |
| Developer loop | A contributor can find the right source file and run one focused check from docs alone. |

## Natural Syntax Pressure

When compiler code looks ugly, prefer improving the general language surface:

- use `type` aliases for domain terms such as `SourceId`, `ByteOffset`,
  `SymbolId`, and `TypeId`
- use `char` literals like `'0'` instead of integer byte constants
- use `Result[T, E]` for expected compiler failures
- use tuple returns for always-present product values such as
  `(value, overflowed)`
- use `Option[T]` for absence instead of sentinel values when the type can be
  expressed cleanly
- use named formatting captures for stable artifact text
- keep compiler/tooling diagnostics outside runtime `std`

Do not add bootstrap-only keywords, hidden global allocation, or backend hooks
that ordinary Ari programs cannot use.

## Test Inventory

Current readiness tests:

- `tests/cases/compiler-development/ok/model/compiler-development-dashboard.ari`:
  one-page compiler-development dashboard status, next-action categories, and
  the 40% readiness seed as normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-concepts-glossary.ari`:
  compiler layer concepts, review vocabulary, and artifact ownership as normal
  Ari data.
- `tests/cases/compiler-development/ok/model/compiler-layer-map.ari`:
  compiler source-layer ownership, first artifact, and focused-check routing as
  normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-triage-guide.ari`:
  symptom-to-layer routing, first artifact choice, and smallest-check selection
  as normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-source-identity.ari`:
  source kinds, stable source ids, byte span states, and source-map policy as
  normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-module-project-authoring.ari`:
  module surfaces, file-backed project state, metadata, cache, and module graph
  policy as normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-artifact-authoring.ari`:
  artifact stage order, golden policy, handoff boundary, and executable-last
  review policy as normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-diagnostic-authoring.ari`:
  diagnostic code families, labels, notes, source spans, and golden-test policy
  as normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-test-authoring.ari`:
  compiler test bucket selection, expected results, and focused-check policy as
  normal Ari data.
- `tests/cases/bootstrap-readiness/ok/model/model-token-span.ari`: token/span
  model seed fixture.
- `tests/cases/bootstrap-readiness/ok/source/source-line-column.ari`: source
  lookup seed fixture.
- `tests/cases/bootstrap-readiness/ok/errors/errors-result-flow.ari`:
  `Result[T, E]` expected-failure fixture.
- `tests/cases/bootstrap-readiness/ok/formatting/formatting-artifact-line.ari`:
  stable artifact text seed fixture.
- `tests/cases/compiler-development/ok/model/compiler-pass-worklist.ari`:
  normal compiler pass/worklist data model.
- `tests/cases/compiler-development/ok/model/compiler-diagnostic-workflow.ari`:
  normal diagnostic data flow as Ari values.
- `tests/cases/compiler-development/ok/model/compiler-change-checklist.ari`:
  review checklist areas, focused-test choice, and non-goal tracking as normal
  Ari data.
- `tests/cases/compiler-development/ok/model/compiler-source-map-workflow.ari`:
  normal source identity, span validation, line/column lookup, structured
  source errors, and tuple return flow as Ari values.
- `tests/cases/compiler-development/ok/model/compiler-implementation-slices.ari`:
  implementation playbook slices, first-check selection, artifact choice, and
  review readiness as normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-next-slices.ari`:
  near-term compiler-development queue, readiness blockers, and first artifact
  choices as normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-stage-gates.ari`:
  compiler-development readiness gates, percent windows, enum state payloads,
  tuple returns, and `Result`-based not-ready flow as normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-readiness-scorecard.ari`:
  weighted readiness gates, current percentage scoring, and compiler pressure
  as normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-test-classification.ari`:
  compiler test categories, artifact families, backend/runtime distinction, and
  fixture naming policy as normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-doc-crosswalk.ari`:
  language docs-to-tests navigation, focused checks, and feature-family
  coverage as normal Ari data.
- `tests/cases/compiler-development/errors/bootstrap-class-keyword.ari`:
  compiler-development policy fixture proving `class` is rejected instead of
  becoming a bootstrap-only abstraction shortcut.
- `tests/cases/compiler-development/errors/bootstrap-interface-keyword.ari`:
  compiler-development policy fixture proving `interface` is rejected and
  compiler abstraction boundaries should use `trait`.
  Together these prove class/interface bootstrap-only syntax stays rejected in
  ordinary compiler-development fixtures.
- `tests/cases/compiler-development/artifact/ok/token-dump-basic.ari`:
  lexer artifact fixture checked through `--emit-tokens`.
- `tests/cases/compiler-development/artifact/ok/source-map-file-module.map`:
  source byte/line table golden checked through `--emit-source-map`.
- `tests/cases/compiler-development/artifact/ok/syntax-dump-basic.syntax`:
  parser artifact golden checked through `--emit-syntax`.
- `tests/cases/compiler-development/artifact/ok/module-graph-file-module.graph`:
  file-backed source/import/item graph golden checked through
  `--emit-module-graph`.
- `tests/cases/compiler-development/artifact/ok/declaration-index-basic.decls`:
  declaration signature, visibility, and source-location golden checked through
  `--emit-declaration-index`.
- `tests/cases/compiler-development/artifact/ok/stage-plan-basic.plan`:
  stage order, owner, first-check, and development-gate golden checked through
  `--emit-stage-plan`.
- `tests/cases/compiler-development/artifact/ok/typed-ir-basic.ir`: sema and
  typed-IR artifact golden checked through `--emit-typed-ir`.
- `tests/cases/compiler-development/artifact/ok/pass-summary-basic.summary`:
  pass-boundary count golden checked through `--emit-pass-summary`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.diagnostic`:
  lexer diagnostic golden checked through `--emit-diagnostics`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-parser-expected.diagnostic`:
  parser diagnostic-code golden checked through `--emit-diagnostics`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-missing-module.diagnostic`:
  module diagnostic-code golden checked through `--emit-diagnostics`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-unknown-trait.diagnostic`:
  type and trait diagnostic-code golden checked through `--emit-diagnostics`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-borrow-conflict.diagnostic`:
  ownership diagnostic-code golden checked through `--emit-diagnostics`.

The first command to run after changing this area is:

```text
make check-compiler-development
```

Run `make check-compiler-dev-docs` when changing the roadmap, maturity gates,
or this inventory.
