# Ari Lint Roadmap

## Status

- [x] Create a separate `tools/lint/` folder for the lint CLI.
- [x] Create shared tooling helpers under `tools/ari_tooling/` for process
      execution, diagnostic parsing, and JSON escaping.
- [x] Add `make lint` and `make check-tools` coverage.
- [x] Support human output for quick terminal use.
- [x] Support JSON output for editor and CI integrations.
- [x] Add native lint-rule registration separate from compiler semantic
      lowering.
- [x] Add rule severity configuration through `--rule RULE=SEVERITY`.
- [x] Add the first native lint rule, `lint/trailing-whitespace`, and document
      its default behavior.
- [x] Add machine-readable `endLine`/`endColumn` spans to lint JSON for
      diagnostics that know a wider source range.

## Near-Term Work

- [ ] Add stable diagnostic codes once compiler diagnostics expose codes.
- [ ] Extend compiler-backed diagnostics with wider spans once the compiler
      exposes them.
- [ ] Keep adding lint-only rules and docs as policy decisions become stable.
- [ ] Add shared lint rule configuration loading once projects need persistent
      per-repository policy.

## Design Notes

`ari-lint` should remain a tool-layer consumer of the compiler front end. Parser
and semantic correctness diagnostics belong in the compiler; style, migration,
unused-code, and editor-oriented warnings belong in lint rules.

Do not put LSP protocol logic in this folder. The LSP server consumes the same
diagnostic helpers but owns JSON-RPC and editor behavior separately.
