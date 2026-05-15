# Ari VS Code Extension

The VS Code extension lives in `editors/vscode/`. It registers `.ari` and
`.arih` files, provides a small TextMate grammar, and launches `ari-lsp` through
`vscode-languageclient`. Diagnostics are served by `ari-lsp` through both
publish and pull diagnostic flows, including unsaved buffer text, so VS Code can
show compiler-backed errors in the Problems panel while editing.

## Layout

- `editors/vscode/package.json` declares language, configuration, and extension
  metadata.
- `editors/vscode/extension.js` starts the language client.
- `editors/vscode/language-configuration.json` defines comments, brackets, and
  auto-closing pairs.
- `editors/vscode/syntaxes/ari.tmLanguage.json` provides lightweight syntax
  highlighting.

## Configuration

- `ari.compilerPath`: path passed to `ari-lsp --ari`
- `ari.lspPath`: path to the `ari-lsp` executable
- `ari.modulePaths`: extra `-I` paths passed to the language server

## Developer Notes

Progress and planned work are tracked in [VS Code Roadmap](dev/roadmap.md).
