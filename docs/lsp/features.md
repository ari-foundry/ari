# Ari LSP Features

This page describes the current user-facing behavior of `ari-lsp`.

## Diagnostics

The language server supports both publish diagnostics and pull diagnostics.
Diagnostics are produced through the shared lint checker, which runs compiler
`--check` diagnostics and native lint rules.

Open documents are checked from a temporary same-directory file so unsaved text
can be diagnosed while preserving relative module lookup and nearby
`ari-lint.rules` discovery.

Supported diagnostic paths:

- `textDocument/publishDiagnostics`
- `textDocument/diagnostic`

## Code Actions

`textDocument/codeAction` returns quick fixes for native source-text lint rules
that can be fixed with direct text edits:

- remove trailing whitespace
- insert a missing final newline

The server also offers `source.fixAll.ari` when at least one of those edits is
available.

## Symbols

The first-pass symbol scanner recognizes top-level declarations in the current
document:

- `fn`
- `extern fn`
- `struct`
- `enum`
- `trait`
- `impl`
- `mod`

It powers:

- `textDocument/documentSymbol`
- `textDocument/hover`
- `textDocument/definition`
- `textDocument/completion`

These features are intentionally top-level only until parser and module-summary
backing replaces the scanner.

## Workspace Symbols

`workspace/symbol` scans `.ari` and `.arih` files below the server working
directory and skips generated or dependency folders such as `build`, `.git`, and
`node_modules`.

Search matches declaration names and labels with a case-insensitive substring
match.

## Editor Navigation Helpers

The server also provides first-pass editor helpers:

- `textDocument/documentHighlight`: highlights matching same-document
  identifiers.
- `textDocument/foldingRange`: returns brace-block folding ranges.
- `textDocument/selectionRange`: expands from the identifier at the cursor to
  the containing source line.

These helpers are text-based and will become parser-backed as the LSP grows.

## Configuration

Editors can pass:

```sh
build/ari-lsp --ari build/ari
build/ari-lsp --ari build/ari -I modules
build/ari-lsp --ari build/ari --config ari-lint.rules
build/ari-lsp --ari build/ari --rule lint/trailing-whitespace=error
```

Lint config and rule override semantics match `ari-lint`.
