# Ari VS Code Extension

The VS Code extension lives in `editors/vscode/`. It registers `.ari` and
`.arih` files, provides a small TextMate grammar, and launches `ari-lsp` through
`vscode-languageclient`. Diagnostics are served by `ari-lsp` through both
publish and pull diagnostic flows, including unsaved buffer text, so VS Code can
show compiler-backed errors in the Problems panel while editing. The extension
also receives `textDocument/documentSymbol` results from `ari-lsp`, which gives
VS Code a first-pass Outline for top-level Ari declarations. Hover is also
served for those top-level declarations, and same-document definition requests
can jump to their declaration lines. Completion suggests those same top-level
symbols in the active document.

## Layout

- `editors/vscode/package.json` declares language, configuration, and extension
  metadata.
- `editors/vscode/extension.js` starts the language client.
- `editors/vscode/commands.js` owns command-palette actions and output-channel
  process execution.
- `editors/vscode/tasks.js` owns VS Code task-provider integration for common
  Ari make targets.
- `editors/vscode/language-configuration.json` defines comments, brackets, and
  auto-closing pairs.
- `editors/vscode/syntaxes/ari.tmLanguage.json` provides lightweight syntax
  highlighting.

## Configuration

- `ari.compilerPath`: path passed to `ari-lsp --ari`
- `ari.lspPath`: path to the `ari-lsp` executable
- `ari.lintPath`: path to the `ari-lint` executable
- `ari.modulePaths`: extra `-I` paths passed to the language server

## Commands

- `Ari: Check Current File` saves the active Ari file and runs
  `ari --check`.
- `Ari: Lint Current File` saves the active Ari file and runs `ari-lint`.

Both commands write to the `Ari` output channel.

## Tasks

The extension contributes Ari tasks for common repository commands:

- `Ari: make`
- `Ari: make check`
- `Ari: make tools`
- `Ari: make check-tools`
- `Ari: make lint`
- `Ari: make lsp`

## Developer Notes

Progress and planned work are tracked in [VS Code Roadmap](dev/roadmap.md).
