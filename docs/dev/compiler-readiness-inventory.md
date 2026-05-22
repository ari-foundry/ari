# Compiler Readiness Inventory

This page is the inventory for Ari compiler development. It answers one
question: what has to become true for Ari to feel like a practical compiler
project?

Use it to keep the hosted compiler reliable, make the language pleasant for
large programs, and make compiler behavior easy to test in small pieces.

## Current Readiness

Ari is about **48-49% through the current compiler-development maturity work**.

The number is conservative because Ari already compiles useful programs, but a
compiler is a large program with hard requirements: stable diagnostics,
multi-file project flow, data-heavy generic models, deterministic artifacts,
and tests that fail near the layer that regressed.

## Compiler Health Scorecard

The estimate is a weighted engineering score. The current seed fixture
`tests/cases/compiler-development/ok/model/compiler-readiness-scorecard.ari`
models the same shape and returns `49`, keeping the scorecard executable as
ordinary Ari data.

| Gate | Weight | Current Score | What Moves It |
| --- | --- | --- | --- |
| Frontend reliability | 10 | 55 | Span-aware lexer/parser errors, recovery fixtures, and syntax docs that match parser behavior. |
| Source identity | 12 | 30 | Owned source maps/files, canonical/display paths, stable `SourceId`, byte spans, line tables, EOF offsets, and snippets. |
| Diagnostics | 13 | 30 | Diagnostic codes, labels, notes, source rendering, and normalized golden output. |
| Module projects | 12 | 55 | Predictable roots/search paths, `.ari`/`.arih` policy, visibility errors, cycles, source identity, metadata, and cache invalidation. |
| Compiler data models | 15 | 60 | Nested generic aggregates, `Result` payloads, vectors/maps/sets, compiler-shaped ownership patterns, and stable rejection of infinite value layouts. |
| Trait selection | 12 | 45 | The minimum static subset is locked; broader trait objects, associated-type solving, and collection defaults can still deepen it. |
| Artifact comparison | 24 | 65 | Token, syntax, diagnostic, source-map, module graph, declaration, typed IR, LLVM-fragment, runtime stdout, HIR, object, and executable comparison order. |
| Tool build flow | 10 | 35 | Focused Make targets for one Ari tool, fixture roots, and golden comparison without hidden flags. |

Weighted together, this lands in the high-40s. Treat each row as normal compiler
development: a row moves when ordinary Ari programs, diagnostics, or artifacts
get more reliable, not when a private shortcut is added.

## Already Strong

| Area | Current Strength | Why It Matters |
| --- | --- | --- |
| Hosted LLVM backend | Ari emits LLVM IR, objects, executables, and shared libraries through the LLVM path. | Compiler work can be validated as real artifacts instead of toy interpreter behavior. |
| Core executable language | Functions, locals, scalar operators, casts, blocks, branches, loops, `break`, `continue`, and returns are locked by `make check-core-language`; richer enums, structs, modules, FFI, and formatting are covered by their focused suites. | Compiler-shaped fixtures can be written as ordinary Ari programs on top of a stable scalar/control-flow base. |
| Generic calls and ADTs | Generic functions, generic structs, generic enums, generic aliases, nested aggregate payloads, stdlib generic stress cases, and compiler-shaped generic models are locked by `make check-generics` and `make check-compiler-development`. | Source models, tokens, AST nodes, pass results, diagnostics, and expected failures can use natural types without stdlib name-specific shortcuts. |
| Generic aggregate scale | Nested structs, enums, aliases, vectors, maps, `Result` payloads, ownership-qualified generic fields, and recursive-value diagnostics have production-focused coverage. | Compiler data models can grow through the general generic aggregate path rather than one-off container or bootstrap scaffolds. |
| Minimum static traits | Trait declarations, impl conformance, deterministic static dispatch, generic bounds, Eq/Ord/Hash/Debug-like fixtures, iterator-shaped helpers, and missing/ambiguous diagnostics are locked by `make check-traits`. | Compiler-shaped data can compare, hash, format, and traverse values through normal trait behavior instead of name-specific shortcuts. |
| SourceMap and diagnostics | `SourceMap`, `SourceId`, `SourceFile`, `Span`, `SourceLocation`, line/column lookup, snippets, diagnostic codes, labels, notes, and source-aware golden artifacts are locked by `make check-source-map-unit` and `make check-compiler-artifacts`. | User-facing compiler errors keep source identity and deterministic artifact rows across lexer, parser, module, semantic, trait, and ownership paths. |
| File-backed projects | Entry-file roots, explicit `-I`/`--module-path` roots, `.ari`/`.arih` candidate policy, aliases, package-style child directories, visibility, cycles, duplicate source identities, imported-file diagnostics, metadata, cache invalidation, and module graph artifacts are locked by `make check-modules` and `make check-compiler-artifacts`. | Multi-file Ari tools can be structured as ordinary source/diagnostic/symbol/parser modules without package-manager or bootstrap scaffolding. |
| Ownership checks | Move, borrow, drop, aggregate field moves, compiler-shaped owner movement, active enum payload cleanup, and explicit-zone checks are locked by `make check-ownership`, `make check-errors`, `make check-variables`, and ownership artifact goldens. | Large compiler graphs can be kept explicit instead of hiding allocation in globals. |
| Focused test culture | Many feature folders already separate `ok` and `errors` tests. | New compiler behavior can be guarded with one small fixture at a time. |

## Blocking Gaps

| Gap | Needed State | First Work |
| --- | --- | --- |
| Trait selection beyond the minimum subset | Trait objects, associated-type solving, trait-driven collection defaults, and richer iterator ownership policies need the same stability as the static subset. | Keep minimum-subset fixtures green while adding one focused advanced trait fixture at a time. |
| Pass artifacts | HIR plus richer object/header/relocation and broader executable-output goldens still need the same depth as token, syntax, typed IR, LLVM-fragment, stdout, and seeded object/shared-symbol artifacts. | Add normalized text dumps before broad executable checks. |
| Build ergonomics | Large Ari tools need boring Make targets and project fixtures before a package manager exists. | Keep `make check-compiler-development` small and add one target per compiler slice. |

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

Generic aggregate monomorphization is now treated as implemented for local
executable/compiler-model code. User-defined fixtures cover generic structs,
enums, aliases, nested payloads, methods, match bindings, ownership-qualified
fields, and stable recursive-value diagnostics; stdlib fixtures use `Vec`,
`Option`, `Result`, `HashMap`, and diagnostic vectors only as stress cases for
the same general machinery.

The hosted compiler now also has the first artifact producers:
`--emit-stage-plan path`, `--emit-capability-inventory path`,
`--emit-source-map path`, `--emit-tokens path`, `--emit-syntax path`,
`--emit-diagnostics path`, `--emit-diagnostic-catalog path`,
`--emit-module-graph path`, `--emit-declaration-index path`, `--emit-typed-ir path`, and
`--emit-pass-summary path`. They write deterministic stage order and first-check
routing, compiler capability status tables, source byte/line tables, lexer token text,
parser tree text, expected-failure diagnostic text,
diagnostic code ownership tables,
file-backed source/import/item graph text, declaration signature text,
sema-lowered typed IR, and stage-boundary counts. `make check-compiler-artifacts`
also extracts review-sized LLVM fragments, compares seeded object/shared symbol
inventories, and compares stdout goldens. This is the current
stage-comparison path for normal compiler development: when source loading,
lexer, parser, diagnostic, module, declaration surface, typed lowering,
backend lowering, ABI visibility, or runtime output changes, reviewers can
inspect a focused golden diff before reaching for a broad suite run.
Diagnostic artifacts now include stable codes, explicit layer families,
source ids, parseable label byte spans, note/help location policy, and
snippets such as
`Label index=0 role=primary source_id=0 source="file.ari" line=1 column=19 end_line=1 end_column=31 byte_start=18 byte_end=30`,
so triage can route failures without reading the classifier implementation
first.
The hosted compiler also exposes `ari --list-capabilities` and
`ari --explain-capability <name>` for the same capability table, which makes
roadmap triage available from the binary without first creating an artifact
file.

## Development Backlog

Use this order for general compiler development:

1. Frontend contracts: lexer/parser span accuracy, literal behavior, recovery,
   and malformed syntax diagnostics.
2. Source model maintenance: keep `SourceMap`, `SourceFile`, `SourceId`,
   `Span`, canonical/display paths, byte offsets, line tables, EOF offsets,
   line/column conversion, and snippet extraction covered as new compiler paths
   are added.
3. Diagnostic model polish: keep diagnostic codes, severity,
   primary/secondary labels, notes, stable sorting, and path normalization
   covered while expanding rule-specific codes and retiring transitional string
   constructors.
4. Module projects: file-backed modules, project roots, header/source
   separation, module metadata, and module caches.
5. Type and trait maturity: advanced generic constraints, associated types,
   trait selection beyond the static subset, and clear ambiguity diagnostics.
6. Ownership scale: arena/zone patterns for compiler graphs, borrowed views,
   scratch reset rules, and predictable drops.
7. IR contract: lower resolved facts into typed IR so LLVM codegen is mostly a
   mechanical emitter.
8. Artifact testing: normalize and compare capability inventory, token,
   diagnostic, syntax, HIR, typed IR, LLVM, object symbol, and executable
   outputs.

## Compiler Development Gates

Use these gates to decide whether Ari is becoming a practical compiler project.
Their job is to keep today's compiler reliable and pleasant to extend:

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

Do not add private compiler-only keywords, hidden global allocation, or backend
hooks that ordinary Ari programs cannot use.

## Test Inventory

Current compiler-development tests:

- `tests/cases/compiler-development/ok/model/compiler-development-dashboard.ari`:
  one-page compiler-development dashboard status, next-action categories, and
  the 40% readiness seed as normal Ari data.
- `tests/cases/compiler-development/ok/model/compiler-onboarding-workflow.ari`:
  first-day compiler contributor path, layer choice, fixture bucket choice,
  focused checks, and non-bootstrap scope as normal Ari data.
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
  becoming a private abstraction shortcut.
- `tests/cases/compiler-development/errors/bootstrap-interface-keyword.ari`:
  compiler-development policy fixture proving `interface` is rejected and
  compiler abstraction boundaries should use `trait`.
  Together these prove class/interface private syntax stays rejected in ordinary
  compiler-development fixtures.
- `tests/cases/compiler-development/artifact/ok/token-dump-basic.ari`,
  `token-dump-lexical-surface.ari`, and `token-dump-crlf.tokens`: lexer
  artifact fixtures checked through `--emit-tokens`.
- `tests/cases/compiler-development/artifact/ok/source-map-file-module.map`:
  source byte/line table golden checked through `--emit-source-map`.
- `tests/cases/compiler-development/artifact/ok/source-map-empty.map`,
  `source-map-crlf.map`, and `source-map-utf8.map`: empty-file, CRLF, and
  UTF-8 byte/line table goldens checked through `--emit-source-map`.
- `tests/cases/compiler-development/artifact/ok/syntax-dump-basic.syntax` and
  `syntax-declarations.syntax`: parser artifact goldens checked through
  `--emit-syntax`.
- `tests/cases/compiler-development/artifact/ok/syntax-control-flow.syntax`:
  parser control-flow artifact golden checked through `--emit-syntax`.
- `tests/cases/compiler-development/artifact/ok/module-graph-file-module.graph`:
  file-backed source/import/item graph golden checked through
  `--emit-module-graph`.
- `tests/cases/compiler-development/artifact/ok/declaration-index-basic.decls`:
  declaration signature, visibility, and source-location golden checked through
  `--emit-declaration-index`.
- `tests/cases/compiler-development/artifact/ok/declaration-index-project-compiler.decls`:
  compiler-shaped file-backed project declaration inventory checked through
  `--emit-declaration-index`.
- `tests/cases/compiler-development/artifact/ok/declaration-index-generic-aggregate.decls`:
  generic aggregate aliases, impls, nested payloads, and owned-field
  declaration inventory checked through `--emit-declaration-index`.
- `tests/cases/compiler-development/artifact/ok/stage-plan-basic.plan`:
  stage order, owner, first-check, and development-gate golden checked through
  `--emit-stage-plan`.
- `tests/cases/compiler-development/artifact/ok/capability-inventory.inventory`:
  implemented, partial, planned, and rejected compiler capability status
  checked through `--emit-capability-inventory`.
- `tests/cases/compiler-development/artifact/ok/diagnostic-catalog.catalog`:
  diagnostic code, family, owner, and fallback-policy golden checked through
  `--emit-diagnostic-catalog`.
- `tests/cases/compiler-development/artifact/ok/typed-ir-basic.ir`: sema and
  typed-IR artifact golden checked through `--emit-typed-ir`.
- `tests/cases/compiler-development/artifact/ok/project-compiler.ir`:
  compiler-shaped file-backed module typed-IR artifact golden checked through
  `--emit-typed-ir`.
- `tests/cases/compiler-development/artifact/ok/ownership-aggregate-field-move.ir`:
  ownership/drop typed-IR artifact golden checked through `--emit-typed-ir`.
- `tests/cases/compiler-development/artifact/ok/backend-ownership-drop-aggregate.llvm-frag`
  and `backend-ownership-drop-runtime-enum.llvm-frag`: review-sized LLVM
  fragments for aggregate field drop calls and runtime-tagged active enum
  payload cleanup.
- `tests/cases/compiler-development/artifact/ok/backend-ownership-compiler-shaped.llvm-frag`:
  review-sized LLVM fragment for the compiler-shaped ownership fixture covering
  generic aggregate field moves, local `Vec[WorkItem]` owner moves, result-like
  enum matching, and deterministic `Drop` calls.
- `tests/cases/compiler-development/artifact/errors/diagnostic-assignment-while-borrowed.diagnostic`,
  `diagnostic-field-assignment-while-borrowed.diagnostic`,
  `diagnostic-return-live-owner.diagnostic`,
  `diagnostic-loop-break-live-owner.diagnostic`,
  `diagnostic-loop-continue-live-owner.diagnostic`,
  `diagnostic-borrow-after-move.diagnostic`, `diagnostic-double-move.diagnostic`,
  and `diagnostic-enum-payload-invalid-move.diagnostic`: source-aware
  ownership diagnostic goldens for borrow conflicts, live-owner control-flow
  exits, repeated moves, and invalid enum payload moves.
- `tests/cases/compiler-development/artifact/ok/pass-summary-basic.summary`:
  pass-boundary count golden checked through `--emit-pass-summary`.
- `tests/cases/compiler-development/artifact/ok/backend-core.llvm-frag`,
  `backend-generic-aggregate.llvm-frag`, and `backend-trait-dispatch.llvm-frag`:
  extracted LLVM backend fragments checked after full `--emit-llvm` generation.
- `tests/cases/compiler-development/artifact/ok/object-library-export.symbols`
  and `shared-visibility.symbols`: normalized object and linked shared-library
  symbol inventories checked through `tests/extract_symbol_names.py`.
- `tests/cases/compiler-development/artifact/ok/runtime-output-basic.stdout` and
  `runtime-output-trait.stdout`: executable stdout goldens checked when the LLVM
  driver is available.
- `tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.diagnostic`:
  lexer diagnostic golden checked through `--emit-diagnostics`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-parser-expected.diagnostic`:
  parser diagnostic-code golden checked through `--emit-diagnostics`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-missing-module.diagnostic`:
  module diagnostic-code golden checked through `--emit-diagnostics`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-ambiguous-module.diagnostic`
  and `diagnostic-cyclic-module.diagnostic`: module graph validation
  diagnostic goldens checked through `--emit-diagnostics`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-unknown-trait.diagnostic`:
  type and trait diagnostic-code golden checked through `--emit-diagnostics`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-type-assignment.diagnostic`:
  assignment type diagnostic span golden checked through `--emit-diagnostics`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-borrow-conflict.diagnostic`:
  ownership diagnostic-code golden checked through `--emit-diagnostics`.
- `tests/cases/compiler-development/artifact/errors/diagnostic-use-after-move.diagnostic`,
  `tests/cases/compiler-development/artifact/errors/diagnostic-return-live-owner.diagnostic`,
  `tests/cases/compiler-development/artifact/errors/diagnostic-loop-break-live-owner.diagnostic`,
  `tests/cases/compiler-development/artifact/errors/diagnostic-loop-continue-live-owner.diagnostic`,
  `tests/cases/compiler-development/artifact/errors/diagnostic-move-borrowed-owner.diagnostic`,
  `tests/cases/compiler-development/artifact/errors/diagnostic-ownership-partial-move.diagnostic`,
  and `tests/cases/compiler-development/artifact/errors/diagnostic-ownership-vector-dynamic-move.diagnostic`:
  source-aware ownership diagnostics for representative move, borrow,
  control-flow live-owner, partial-move, and unsupported container-element
  ownership failures.

The first command to run after changing this area is:

```text
make check-compiler-development
```

Run `make check-compiler-dev-docs` when changing the roadmap, maturity gates,
or this inventory.
