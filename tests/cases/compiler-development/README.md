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
- `ok/model/compiler-source-map-workflow.ari`: source ids, byte spans,
  line/column lookup, structured source errors, and tuple return flow.
- `artifact/ok/normalize-paths.*.txt`: seed path, temp-name, and pointer
  normalization fixture for future golden artifact checks.
- `artifact/ok/source-map-file-module.map`: source byte, line, newline, and
  snippet golden seed for root plus file-backed child module sources.
- `artifact/ok/declaration-index-basic.*`: declaration signature, visibility,
  module, and source-location golden seed before semantic lowering.
- `artifact/ok/token-dump-basic.*`: lexer, parser, and typed-IR golden seed
  generated from one tiny source file.
- `artifact/ok/module-graph-file-module.graph`: file-backed module graph
  golden seed for resolved sources, imports, and item surfaces.
- `artifact/ok/pass-summary-basic.summary`: driver-level stage count seed for
  lexer, syntax, module loading, and sema boundaries.
- `artifact/errors/diagnostic-*.diagnostic`: lexer, parser, module, type/trait,
  and ownership diagnostic-code golden seeds.
- `artifact/errors/text-line-mismatch.*.txt`: seed mismatch-report fixture for
  text artifact comparisons.
