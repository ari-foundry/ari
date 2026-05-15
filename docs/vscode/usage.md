# Ari VS Code Usage

This page covers running the Ari VS Code extension from the repository and the
features that should be visible in the Extension Development Host.

## Local Development Run

Build the compiler and tools from the repository root:

```sh
cd /mnt/c/home/asss
make
make tools
```

Install the extension dependencies and open the extension folder:

```sh
cd /mnt/c/home/asss/editors/vscode
npm install
code .
```

Press `F5` and choose `Run Ari Extension` if VS Code asks for a launch
configuration. The launch configuration opens `/mnt/c/home/asss` in the
Extension Development Host so the default tool paths resolve to:

- `build/ari`
- `build/ari-lint`
- `build/ari-lsp`

Open an `.ari` file such as `examples/count.ari` in the Extension Development
Host to activate the extension.

## Available Features

- Syntax highlighting for Ari source and header files.
- Problems diagnostics from `ari-lsp`, including compiler checks and native lint
  rules.
- Pull diagnostics when VS Code asks for current document diagnostics.
- Outline symbols for top-level declarations.
- Workspace symbol search for top-level declarations in `.ari` and `.arih`
  files.
- Hover text for top-level declarations.
- Same-document go-to-definition for top-level declarations.
- Completion for top-level declarations in the active document.
- Same-document document highlights for matching identifiers.
- Brace-block folding ranges.
- Selection range expansion from identifier to line.
- Command palette actions:
  - `Ari: Check Current File`
  - `Ari: Lint Current File`
  - `Ari: Restart Language Server`
- Ari task entries for common `make` targets.

## Troubleshooting

If VS Code reports that it cannot find `vscode-languageclient/node`, run
`npm install` from `editors/vscode`.

If the language server fails to start, check that `make` and `make tools` have
created `build/ari`, `build/ari-lint`, and `build/ari-lsp` from the repository
root.

If the tools were built inside WSL, run the extension from a VS Code Remote -
WSL window. A normal Windows extension host cannot execute the Linux
`build/ari-lsp` binary.

If the wrong tool path is being used, open VS Code settings and adjust
`ari.compilerPath`, `ari.lintPath`, or `ari.lspPath`, then run
`Ari: Restart Language Server`.
