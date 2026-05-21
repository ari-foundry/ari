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
- `errors/bootstrap-class-keyword.ari`: rejects `class` as a compiler-development
  shortcut; compiler code should use normal Ari structs, enums, functions, and
  traits.
- `errors/bootstrap-interface-keyword.ari`: rejects `interface` as a
  compiler-development shortcut; Ari uses `trait`.
- `artifact/ok/normalize-paths.*.txt`: seed path, temp-name, and pointer
  normalization fixture for future golden artifact checks.
- `artifact/ok/source-map-file-module.map`: source byte, line, newline, and
  snippet golden seed for root plus file-backed child module sources.
- `artifact/ok/declaration-index-basic.*`: declaration signature, visibility,
  module, and source-location golden seed before semantic lowering.
- `artifact/ok/stage-plan-basic.plan`: compiler artifact order, layer owner,
  first-check, and development-gate golden seed emitted by the driver.
- `artifact/ok/token-dump-basic.*`: lexer, parser, and typed-IR golden seed
  generated from one tiny source file.
- `artifact/ok/module-graph-file-module.graph`: file-backed module graph
  golden seed for resolved sources, imports, and item surfaces.
- `artifact/ok/pass-summary-basic.summary`: driver-level stage count seed for
  lexer, syntax, module loading, and sema boundaries.
- `artifact/errors/diagnostic-*.diagnostic`: lexer, parser, module, type/trait,
  and ownership diagnostic code/family golden seeds.
- `artifact/errors/text-line-mismatch.*.txt`: seed mismatch-report fixture for
  text artifact comparisons.
