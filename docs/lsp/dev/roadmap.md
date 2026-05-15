# Ari LSP Roadmap

## Status

- [x] Create a separate `tools/lsp/` folder for the language server.
- [x] Implement a minimal stdio JSON-RPC message loop.
- [x] Reply to `initialize` with diagnostic-first capabilities.
- [x] Publish compiler-backed diagnostics for opened, changed, and saved file
      notifications.
- [x] Respond to `textDocument/diagnostic` pull requests using the same
      diagnostic conversion as publish diagnostics.
- [x] Track opened/changed document text and check unsaved buffers through a
      same-directory temporary source file so relative modules keep working.
- [x] Add first-pass top-level `textDocument/documentSymbol` support for VS Code
      Outline using the current document text.
- [x] Add first-pass `textDocument/documentHighlight` support for same-document
      identifier highlighting using shared LSP text-position helpers.
- [x] Add first-pass `workspace/symbol` support by scanning Ari source files
      under the server working directory.
- [x] Add first-pass `textDocument/hover` support for top-level declarations
      using the same symbol scanner.
- [x] Add first-pass same-document `textDocument/definition` support for
      top-level declarations using the same symbol scanner.
- [x] Add first-pass `textDocument/completion` support for top-level symbols in
      the current document using the same symbol scanner.
- [x] Clear diagnostics on `didClose`.
- [x] Route LSP diagnostics through the shared lint checker so compiler
      diagnostics and native lint rules use the same rule settings.
- [x] Accept shared lint rule config files through `--config PATH`.
- [x] Use lint config discovery for saved and unsaved LSP documents when no
      explicit `--config` is supplied.
- [x] Preserve shared diagnostic codes and explicit end spans in LSP diagnostic
      JSON when the tooling layer provides them.
- [x] Add smoke coverage through `tests/tools/lsp_smoke.sh`.

## Near-Term Work

- [ ] Add stable diagnostic codes once the compiler/lint layer exposes them.
- [ ] Replace first-pass text symbol scanning with parser/module-summary-backed
      nested symbols.
- [ ] Replace first-pass document highlights with parser/sema-backed symbol
      identity so unrelated same-name identifiers are not grouped together.
- [ ] Replace first-pass workspace symbols with parser/module-summary-backed
      package indexing.
- [ ] Replace first-pass definition with parser/sema-backed local, module, and
      imported source declaration navigation.
- [ ] Replace first-pass completion with parser/sema-backed locals, imports,
      fields, methods, and std API suggestions.
- [ ] Replace first-pass hover text with parser/sema-backed signatures and std
      API docs.

## Design Notes

The LSP server should own protocol state and editor behavior only. Compiler
analysis stays in the compiler, lint-only policy stays in `tools/lint`, and
shared subprocess/diagnostic utilities stay in `tools/ari_tooling`.

The first server version avoids broad protocol helpers and keeps JSON parsing
bounded to the messages it currently handles. If protocol surface grows, replace
the small helper with a real JSON parser before adding complex features.
