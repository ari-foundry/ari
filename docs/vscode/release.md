# Ari VS Code Release

This page tracks the local installable VS Code extension package.

## 0.1.0 Scope

The 0.1.0 package is meant for local Ari development. It includes:

- Ari language activation for `.ari` and `.arih`
- syntax highlighting and language configuration
- `ari-lsp` startup through `vscode-languageclient`
- diagnostics, Outline, hover, definition, completion, highlights, folding,
  selection ranges, workspace symbols, and Quick Fix actions from `ari-lsp`
- stable diagnostic codes for native lint and compiler-backed diagnostics
- command-palette actions for `ari --check`, `ari-lint`, and LSP restart
- Ari task entries for common repository `make` targets

Marketplace publishing is intentionally not part of 0.1.0. The package is a
local VSIX that can be installed directly into VS Code.

## Build Prerequisites

From the repository root, build the compiler and editor tools:

```sh
make
make tools
```

From `editors/vscode`, install Node dependencies:

```sh
npm install
```

The package script also requires the `zip` command. The repository currently
uses that command to add the small runtime dependency set needed by
`vscode-languageclient` after `vsce` creates the base VSIX.

## Create The VSIX

Run:

```sh
cd editors/vscode
npm run package
```

The output path, relative to the repository root, is:

```text
build/vscode/ari-vscode-0.1.0.vsix
```

The package script creates a normal `vsce` package and then injects only the
runtime Node packages needed by the extension. This keeps the VSIX installable
without vendoring the whole development dependency tree.

## Install Later

When ready to install the local build:

```sh
cd editors/vscode
npm run install:local
```

Do not run this step when only validating the package. `npm run package` is
enough to produce the VSIX artifact.

## 0.1.0 Validation

Before treating the VSIX as the current local 0.1.0 package, run:

```sh
cd editors/vscode
npm run check
cd ../..
make check-tools
```

For package structure checks, inspect the archive:

```sh
unzip -l build/vscode/ari-vscode-0.1.0.vsix | rg 'extension/node_modules/vscode-languageclient|extension/extension.js'
```
