# Ari VS Code Extension Roadmap

## Status

- [x] Create a separate `editors/vscode/` folder.
- [x] Register `.ari` and `.arih` as Ari language files.
- [x] Add extension settings for compiler path, LSP path, and module paths.
- [x] Launch `ari-lsp` through `vscode-languageclient`.
- [x] Add a small grammar and language configuration.
- [x] Surface first-pass Ari top-level declaration symbols through the LSP
      Outline path.
- [x] Surface first-pass hover text for top-level Ari declarations through the
      LSP hover path.

## Near-Term Work

- [ ] Add a README section for packaging and local extension development once
      dependency installation policy is decided.
- [ ] Add command-palette actions for running `ari-lint` and `ari --check`.
- [ ] Add workspace task snippets for `make`, `make check`, and `make lint`.
- [ ] Add richer syntax highlighting after the lint/LSP diagnostic path is
      stable.

## Design Notes

The VS Code extension should stay an editor adapter. It should not duplicate
compiler analysis or lint policy. It launches `ari-lsp`, passes configuration,
and presents editor UI around the toolchain.
