# Ari VS Code Extension

The VS Code extension lives in `editors/vscode/`. It registers `.ari` and
`.arih` files, provides a small TextMate grammar, and launches `ari-lsp` through
`vscode-languageclient`. Diagnostics are served by `ari-lsp` through both
publish and pull diagnostic flows, including unsaved buffer text, so VS Code can
show compiler-backed errors in the Problems panel while editing. The extension
also receives `textDocument/documentSymbol` results from `ari-lsp`, which gives
VS Code a first-pass Outline for top-level Ari declarations. Hover is also
served for those top-level declarations, and same-document definition requests
can jump to their declaration lines. Workspace symbol search can find top-level
Ari declarations across workspace files. Document highlights mark matching
same-document identifiers, brace blocks provide editor folding ranges, selection
range support expands from identifiers to source lines, and completion suggests
top-level declarations in the active document. Quick Fix can apply direct text
edits for simple native lint rules exposed by `ari-lsp`.

## Layout

- `editors/vscode/package.json` declares language, configuration, and extension
  metadata.
- `editors/vscode/extension.js` wires the extension subsystems together.
- `editors/vscode/lsp.js` owns the language-client lifecycle and restarts
  `ari-lsp` when relevant Ari settings change.
- `editors/vscode/config.js` converts Ari extension settings into shared
  command-line arguments for lint and LSP tools.
- `editors/vscode/commands.js` owns command-palette actions and output-channel
  process execution.
- `editors/vscode/tasks.js` owns VS Code task-provider integration for common
  Ari make targets.
- `editors/vscode/paths.js` owns workspace-relative path resolution shared by
  commands and LSP startup.
- `editors/vscode/scripts/package-vsix.js` creates the local 0.1.0 VSIX and
  injects the runtime Node packages needed by the language client.
- `editors/vscode/language-configuration.json` defines comments, brackets, and
  auto-closing pairs.
- `editors/vscode/syntaxes/ari.tmLanguage.json` provides lightweight syntax
  highlighting for Ari 0.1.0 comments, strings, `@` attributes, declarations,
  macros, keywords, constants, primitive/support types, numeric literals,
  operators, punctuation, and type-like identifiers.

## Configuration

- `ari.compilerPath`: path passed to `ari-lsp --ari`
- `ari.lspPath`: path to the `ari-lsp` executable
- `ari.lintPath`: path to the `ari-lint` executable
- `ari.lintConfigPath`: optional explicit lint config path passed as `--config`;
  leave empty to let `ari-lint` and `ari-lsp` discover the nearest
  `ari-lint.rules`
- `ari.modulePaths`: extra `-I` paths passed to the language server
- `ari.lintRules`: lint severity overrides passed to both `ari-lint` and
  `ari-lsp`, for example `{ "lint/trailing-whitespace": "error" }`

## Commands

- `Ari: Check Current File` saves the active Ari file and runs
  `ari --check`.
- `Ari: Lint Current File` saves the active Ari file and runs `ari-lint`.
- `Ari: Restart Language Server` stops and starts the Ari language client with
  the current extension settings.

The check and lint commands write to the `Ari` output channel.

## Usage

Local run instructions, feature expectations, and troubleshooting are documented
in [Ari VS Code Usage](usage.md).

The current editor feature set is documented in
[Ari VS Code Features](features.md).

The current local installable package workflow is documented in
[Ari VS Code Release](release.md).

## Tasks

The extension contributes Ari tasks for common repository commands:

- `Ari: make`
- `Ari: make check`
- `Ari: make tools`
- `Ari: make check-tools`
- `Ari: make lint`
- `Ari: make lsp`

## LSP Restart Policy

Changing `ari.compilerPath`, `ari.lspPath`, `ari.modulePaths`, or
`ari.lintConfigPath`, or `ari.lintRules` restarts the language client so
diagnostics and editor features use the new toolchain settings.

## Developer Notes

Progress and planned work are tracked in [VS Code Roadmap](dev/roadmap.md).
