# Compiler Development Tests

These fixtures protect ordinary Ari compiler-development work. They are not a
bootstrap implementation tree.

Use this folder for small programs that prove Ari can model compiler-shaped
data, pass results, artifact behavior, and expected compiler diagnostics with
the same language surface that normal Ari users get.

## Layout

- `ok/model/`: data-model and pass-flow fixtures.
- `artifact/ok/`: expected/actual artifact pairs that should compare cleanly.
- `artifact/errors/`: expected mismatches and report shapes for artifact tools.
- `errors/`: focused compiler diagnostics for unsupported or invalid surfaces.

## Current Fixtures

- `ok/model/compiler-pass-worklist.ari`: pass ids, work items, dependency
  state, and `Result`-based pass errors.
- `ok/model/compiler-diagnostic-workflow.ari`: diagnostic values, severities,
  source spans, rich enum error payloads, and explicit vectors.
- `ok/model/compiler-development-dashboard.ari`: one-page dashboard status,
  next-action categories, small-check pressure, and readiness seed as normal Ari
  data.
- `ok/model/compiler-concepts-glossary.ari`: compiler layer concepts, artifact
  ownership, and review vocabulary as normal Ari data.
- `ok/model/compiler-layer-map.ari`: compiler source-layer ownership, first
  artifact, and focused-check routing as normal Ari data.
- `ok/model/compiler-triage-guide.ari`: symptom-to-layer routing, first
  artifact choice, and smallest-check selection as normal Ari data.
- `ok/model/compiler-source-identity.ari`: source kinds, stable source ids,
  byte span states, and source-map policy as normal Ari data.
- `ok/model/compiler-module-project-authoring.ari`: module surfaces,
  file-backed project state, metadata, cache, and module graph policy as normal
  Ari data.
- `ok/model/compiler-artifact-authoring.ari`: artifact stage order, golden
  policy, handoff boundary, and executable-last review policy as normal Ari
  data.
- `ok/model/compiler-diagnostic-authoring.ari`: diagnostic code families,
  labels, notes, source spans, and golden-test policy as normal Ari data.
- `ok/model/compiler-test-authoring.ari`: compiler test bucket selection,
  expected results, artifact update policy, and focused checks as normal Ari
  data.
- `ok/model/compiler-change-checklist.ari`: review checklist areas,
  focused-test choice, and non-goal tracking as normal Ari data.
- `ok/model/compiler-source-map-workflow.ari`: source ids, byte spans,
  line/column lookup, structured source errors, and tuple return flow.
- `ok/model/compiler-implementation-slices.ari`: implementation playbook
  slices, first-check selection, and review readiness as normal Ari data.
- `ok/model/compiler-next-slices.ari`: near-term compiler-development queue,
  readiness blockers, and first artifact choices as normal Ari data.
- `ok/model/compiler-stage-gates.ari`: readiness gates, percent windows,
  enum state payloads, and `Result`-based not-ready flow for normal compiler
  development planning.
- `ok/model/compiler-readiness-scorecard.ari`: weighted readiness gates and
  the current 40% seed estimate as normal Ari data.
- `ok/model/compiler-test-classification.ari`: compiler test categories,
  artifact kinds, backend/runtime distinction, and naming policy as normal Ari
  data.
- `ok/model/compiler-doc-crosswalk.ari`: language docs-to-tests navigation,
  focused checks, and feature-family coverage as normal Ari data.
- `ok/model/compiler-onboarding-workflow.ari`: first-day compiler contributor
  path, layer choice, fixture bucket choice, focused checks, and non-bootstrap
  scope as normal Ari data.
- `ok/model/compiler-generic-aggregates.ari`: TokenStream, ParserState,
  AstNode, AstArena, DiagnosticBuilder, SymbolTable, PassWorklist, WorkItem,
  PassError, and Result-like pass outputs built from normal generic
  aggregates.
- `errors/bootstrap-class-keyword.ari`: rejects `class` as a compiler-development
  shortcut; compiler code should use normal Ari structs, enums, functions, and
  traits.
- `errors/bootstrap-interface-keyword.ari`: rejects `interface` as a
  compiler-development shortcut; Ari uses `trait`.
- `artifact/ok/normalize-paths.*.txt`: seed path, temp-name, and pointer
  normalization fixture for golden artifact checks.
- `artifact/ok/artifact-fixtures.inventory`: deterministic inventory of
  compiler artifact ok/error fixtures, groups, and artifact kinds.
- `tests/source_map_unit.cpp`: direct C++ SourceMap test for empty files,
  one-line and multi-line files, EOF, CRLF, UTF-8 byte columns, invalid spans,
  multi-file ids, source replacement identity, generated sources, missing
  source fallback, and snippets.
- `artifact/ok/source-map-file-module.map`, `source-map-empty.map`,
  `source-map-crlf.map`, and `source-map-utf8.map`: source byte, line, EOF,
  newline, CRLF, and UTF-8 golden seeds for root plus file-backed child module
  sources. The CRLF input is materialized into `build/` from
  `source-map-crlf-template.ari` so committed fixtures remain LF-normalized.
- `artifact/ok/declaration-index-basic.*`: declaration signature, visibility,
  module, and source-location golden seed before semantic lowering.
- `artifact/ok/declaration-index-project-compiler.decls`: declaration
  inventory golden for a compiler-shaped file-backed project with public and
  private functions, imports, nested modules, generic parser state, and enum
  payloads.
- `artifact/ok/declaration-index-generic-aggregate.decls`: declaration
  inventory golden for user-defined generic structs, enums, aliases, impls,
  nested payloads, and owned generic fields.
- `artifact/ok/stage-plan-basic.plan`: compiler artifact order, layer owner,
  first-check, and development-gate golden seed emitted by the driver.
- `artifact/ok/capability-inventory.inventory`: implemented, partial, planned,
  and rejected compiler capability status emitted by the driver.
- `artifact/ok/diagnostic-catalog.catalog`: diagnostic code, family, owner,
  and fallback-policy golden seed emitted by the driver.
- `artifact/ok/token-dump-basic.*`, `token-dump-lexical-surface.*`, and
  `token-dump-crlf.tokens`: lexer token spans, comments, strings, literal
  suffixes, compound operators, EOF, CRLF byte offsets, parser AST spans, and
  typed-IR golden seeds.
- `artifact/ok/syntax-declarations.*`: parser declaration-surface golden for
  inline modules, use aliases, generic structs/enums, traits, impls, and match.
- `artifact/ok/syntax-control-flow.*`: parser control-flow golden for
  `init while`, `continue`, `break`, `else`, and match expression shape.
- `artifact/ok/module-graph-file-module.graph`: file-backed module graph
  golden seed for resolved sources, imports, and item surfaces.
- `artifact/ok/pass-summary-basic.summary`: driver-level stage count seed for
  lexer, syntax, module loading, and sema boundaries.
- `artifact/ok/generic-aggregate-monomorphization.ir`: typed IR golden for
  concrete user-defined generic aggregate instantiations, nested enum payloads,
  method specialization, and owned generic fields.
- `artifact/ok/project-compiler.ir`: typed IR golden for compiler-shaped
  file-backed modules, cross-file calls, qualified types, generic module
  payloads, and enum matching.
- `artifact/ok/ownership-aggregate-field-move.ir`: typed IR golden for `own`
  field moves, field replacement, explicit `drop`, and aggregate storage.
- `artifact/ok/backend-*.llvm-frag`: extracted LLVM function fragments for core
  control flow, generic aggregate backend lowering, ownership/drop lowering,
  and static trait dispatch, kept smaller than full runtime-heavy LLVM files.
- `artifact/ok/object-library-export.symbols` and
  `shared-visibility.symbols`: normalized `nm` symbol inventories for object
  and linked shared-library export surfaces.
- `artifact/ok/runtime-output-basic.*` and `runtime-output-trait.*`:
  executable stdout goldens that run only after earlier frontend, typed IR, and
  backend artifacts are stable.
- `artifact/errors/diagnostic-*.diagnostic`: lexer, parser, module
  missing/ambiguous/cyclic validation, unknown-name, duplicate-name,
  wrong-arity, wrong-argument, invalid-return, invalid-assignment, trait, and
  ownership diagnostic code/family/span golden seeds, including use-after-move,
  moving borrowed owners, invalid partial moves, and dynamic owner element
  moves.
- `artifact/errors/text-line-mismatch.*.txt`: seed mismatch-report fixture for
  text artifact comparisons.
