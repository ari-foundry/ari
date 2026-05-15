# Ari VS Code Features

This page describes the current editor features exposed by the Ari VS Code
extension.

## Language Activation

The extension activates for `.ari` and `.arih` files and registers the `ari`
language id.

## Syntax Highlighting

The bundled TextMate grammar highlights:

- comments
- strings
- attributes
- declarations
- macros
- keywords
- constants
- primitive and support types
- numeric literals
- operators

## Diagnostics

The extension launches `ari-lsp`, which reports compiler-backed diagnostics and
native lint diagnostics in the Problems panel. Unsaved buffer text is sent to the
language server, so diagnostics update while editing.

## Navigation And Editing

Current LSP-powered editor features:

- Outline symbols for top-level Ari declarations.
- Workspace symbol search for top-level Ari declarations in `.ari` and `.arih`
  files.
- Hover text for top-level declarations.
- Same-document go-to-definition for top-level declarations.
- Completion for top-level declarations in the active document.
- Same-document identifier highlights.
- Brace-block folding ranges.
- Selection range expansion from identifier to source line.

## Commands

The command palette exposes:

- `Ari: Check Current File`
- `Ari: Lint Current File`
- `Ari: Restart Language Server`

Check and lint commands save the active Ari file before invoking the configured
tool and write output to the `Ari` output channel.

## Tasks

The extension contributes Ari tasks for common repository workflows:

- `Ari: make`
- `Ari: make check`
- `Ari: make tools`
- `Ari: make check-tools`
- `Ari: make lint`
- `Ari: make lsp`

## Settings

The extension exposes settings for tool paths, module paths, lint config files,
and lint rule severity overrides. Changing a toolchain setting restarts the
language client so editor features use the new configuration.
