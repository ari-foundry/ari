# Ari VS Code Extension Roadmap

## Status

- [x] Create a separate `editors/vscode/` folder.
- [x] Register `.ari` and `.arih` as Ari language files.
- [x] Add extension settings for compiler path, LSP path, and module paths.
- [x] Launch `ari-lsp` through `vscode-languageclient`.
- [x] Add a small grammar and language configuration.
- [x] Surface first-pass Ari top-level declaration symbols through the LSP
      Outline path.
- [x] Surface first-pass same-document identifier highlights through the LSP
      document-highlight path.
- [x] Surface first-pass workspace symbol search through the LSP
      `workspace/symbol` path.
- [x] Surface first-pass hover text for top-level Ari declarations through the
      LSP hover path.
- [x] Surface first-pass same-document go-to-definition for top-level Ari
      declarations through the LSP definition path.
- [x] Surface first-pass top-level Ari declaration completion through the LSP
      completion path.
- [x] Add command-palette actions for running `ari-lint` and `ari --check` on
      the active Ari file.
- [x] Add VS Code task-provider entries for `make`, `make check`, `make tools`,
      `make check-tools`, `make lint`, and `make lsp`.
- [x] Split VS Code LSP client lifecycle into `lsp.js`, share path resolution
      through `paths.js`, and restart the client when Ari toolchain settings
      change.
- [x] Add shared VS Code argument building for module paths and lint rule
      severity overrides, and pass `ari.lintRules` to both lint commands and
      LSP startup.
- [x] Add `ari.lintConfigPath` so VS Code can pass persistent lint rule config
      files to both lint commands and LSP startup.
- [x] Leave lint config path optional so CLI/LSP-side `ari-lint.rules`
      discovery works naturally in editor sessions.
- [x] Add `Ari: Restart Language Server` so extension users can manually
      restart `ari-lsp` after rebuilding tools or changing local setup.
- [x] Expand TextMate syntax highlighting for comments, attributes,
      declarations, macros, keywords, built-in types, numbers, and operators.

## Near-Term Work

- [ ] Add a README section for packaging and local extension development once
      dependency installation policy is decided.
- [ ] Add semantic tokens once parser-backed LSP symbol/type data exists.

## Design Notes

The VS Code extension should stay an editor adapter. It should not duplicate
compiler analysis or lint policy. It launches `ari-lsp`, passes configuration,
and presents editor UI around the toolchain.
