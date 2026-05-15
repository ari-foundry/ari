# Ari LSP Roadmap

## Status

- [x] Create a separate `tools/lsp/` folder for the language server.
- [x] Implement a minimal stdio JSON-RPC message loop.
- [x] Reply to `initialize` with diagnostic-first capabilities.
- [x] Publish compiler-backed diagnostics for opened, changed, and saved file
      notifications.
- [x] Clear diagnostics on `didClose`.
- [x] Add smoke coverage through `tests/tools/lsp_smoke.sh`.

## Near-Term Work

- [ ] Track in-memory document text and check unsaved buffers without breaking
      path-sensitive module resolution.
- [ ] Add stable diagnostic codes once the compiler/lint layer exposes them.
- [ ] Add document symbols from parser/module summaries.
- [ ] Add go-to-definition for local modules and source declarations.
- [ ] Add hover text for functions, types, and std APIs.

## Design Notes

The LSP server should own protocol state and editor behavior only. Compiler
analysis stays in the compiler, lint-only policy stays in `tools/lint`, and
shared subprocess/diagnostic utilities stay in `tools/ari_tooling`.

The first server version avoids broad protocol helpers and keeps JSON parsing
bounded to the messages it currently handles. If protocol surface grows, replace
the small helper with a real JSON parser before adding complex features.
