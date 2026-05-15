# Ari LSP

`ari-lsp` is the language-server entry point for editors. The initial server is
diagnostic-first: it speaks JSON-RPC over stdio, tracks opened document text,
and delegates source checking to `ari --check`.

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
- `textDocument/documentSymbol`

For open documents, the server mirrors the current in-memory text into a
temporary file beside the original source path before invoking `ari --check`, so
relative module lookup still follows the edited file's directory. If the editor
asks about a file that is not open, the saved file on disk is checked directly.
Document symbols are a first-pass top-level outline for functions, structs,
enums, traits, impls, and modules; richer parser-backed symbol trees are still
planned.

## Developer Notes

Progress and planned work are tracked in [LSP Roadmap](dev/roadmap.md).
