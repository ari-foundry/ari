# Ari VS Code Extension

This package provides local VS Code support for Ari source files. It registers
`.ari` and `.arih`, launches `ari-lsp`, exposes Ari commands, and contributes
common Ari repository tasks.

## Local Package

Build the Ari tools first:

```sh
cd /mnt/c/home/asss
make
make tools
```

Install extension dependencies and create the local VSIX:

```sh
cd /mnt/c/home/asss/editors/vscode
npm install
npm run package
```

The package is written to:

```text
../../build/vscode/ari-vscode-0.1.0.vsix
```

Install later with:

```sh
npm run install:local
```

The installed extension expects the Ari tools at `build/ari`,
`build/ari-lint`, and `build/ari-lsp` by default.
