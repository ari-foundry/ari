# Ari LSP

`ari-lsp` is the language-server entry point for editors. The server is
diagnostic-first: it speaks JSON-RPC over stdio, tracks opened document text,
and delegates diagnostics to the shared lint checker, which runs `ari --check`
and native lint-only rules.

The server is intentionally small so it can stabilize protocol behavior before
larger parser-backed features such as references and rename are added.

## Build

```sh
make lsp
```

The executable is written to `build/ari-lsp`.

## Usage

Editors should launch:

```sh
build/ari-lsp --ari build/ari
build/ari-lsp --ari build/ari --config ari-lint.rules
build/ari-lsp --ari build/ari --rule lint/trailing-whitespace=error
```

Additional module search paths can be passed with `-I path`.
Lint rule severities use the same config discovery, `--config PATH`, and
`--rule RULE=SEVERITY` formats as `ari-lint`; command-line rule overrides are
applied after config-file settings. For unsaved buffers, discovery follows the
temporary source file beside the original document path, so nearby
`ari-lint.rules` files still apply while editing.

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
- `workspace/symbol`
- `textDocument/hover`
- `textDocument/definition`
- `textDocument/completion`

For open documents, the server mirrors the current in-memory text into a
temporary file beside the original source path before invoking `ari --check`, so
relative module lookup still follows the edited file's directory. If the editor
asks about a file that is not open, the saved file on disk is checked directly.
Diagnostics include compiler-backed errors and native lint warnings. Diagnostic
serialization preserves shared diagnostic codes and explicit end spans when the
tooling layer provides them.
Document symbols are a first-pass top-level outline for functions, structs,
enums, traits, impls, and modules; richer parser-backed symbol trees are still
planned. Workspace symbols scan `.ari` and `.arih` files below the server
working directory while skipping generated or dependency folders such as
`build`, `.git`, and `node_modules`. Hover uses the same first-pass top-level
declaration scan to show the declaration kind and source line for known symbols,
and same-document definition requests can jump back to those top-level
declarations. Completion uses the same declaration scan to suggest top-level
symbols in the current document.

## Developer Notes

Progress and planned work are tracked in [LSP Roadmap](dev/roadmap.md).
