# Ari LSP

`ari-lsp` is the language-server entry point for editors. The initial server is
diagnostic-first: it speaks JSON-RPC over stdio, starts with saved-file
diagnostics, and delegates source checking to `ari --check`.

The server is intentionally small so it can stabilize protocol behavior before
hover, symbols, references, and rename are added.

## Build

```sh
make lsp
```

The executable is written to `build/ari-lsp`.

## Usage

Editors should launch:

```sh
build/ari-lsp --ari build/ari
```

Additional module search paths can be passed with `-I path`.

## Current Capabilities

- `initialize`
- `shutdown`
- `exit`
- `textDocument/didOpen`
- `textDocument/didChange`
- `textDocument/didSave`
- `textDocument/didClose`
- `textDocument/publishDiagnostics`
- `textDocument/diagnostic`

Diagnostics currently use saved files on disk. Unsaved in-memory checking is a
future milestone because Ari module resolution depends on the source file path.

## Developer Notes

Progress and planned work are tracked in [LSP Roadmap](dev/roadmap.md).
