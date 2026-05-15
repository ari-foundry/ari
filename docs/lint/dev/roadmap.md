# Ari Lint Roadmap

## Status

- [x] Create a separate `tools/lint/` folder for the lint CLI.
- [x] Create shared tooling helpers under `tools/ari_tooling/` for process
      execution, diagnostic parsing, and JSON escaping.
- [x] Add `make lint` and `make check-tools` coverage.
- [x] Support human output for quick terminal use.
- [x] Support JSON output for editor and CI integrations.

## Near-Term Work

- [ ] Add stable diagnostic codes once compiler diagnostics expose codes.
- [ ] Add native lint-rule registration separate from semantic lowering.
- [ ] Add rule severity configuration.
- [ ] Add machine-readable spans for multi-token diagnostics.
- [ ] Add docs for each lint rule once real lint-only rules exist.

## Design Notes

`ari-lint` should remain a tool-layer consumer of the compiler front end. Parser
and semantic correctness diagnostics belong in the compiler; style, migration,
unused-code, and editor-oriented warnings belong in lint rules.

Do not put LSP protocol logic in this folder. The LSP server consumes the same
diagnostic helpers but owns JSON-RPC and editor behavior separately.
