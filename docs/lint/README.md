# Ari Lint

`ari-lint` is the standalone lint entry point for Ari tooling. The first
version is intentionally thin: it delegates parser, module-loader, and semantic
diagnostics to `ari --check`, then normalizes those diagnostics into either
human-readable text or JSON.

This keeps lint diagnostics aligned with the compiler while the lint-specific
rule engine is still being designed.

## Build

```sh
make lint
```

The executable is written to `build/ari-lint`.

## Usage

```sh
build/ari-lint examples/count.ari
build/ari-lint --json tests/errors/prelude-macro-format-planned.ari
build/ari-lint --ari build/ari -I path/to/modules source.ari
```

`ARI_COMPILER` can also point the tool at a compiler binary when `--ari` is not
provided.

The VS Code extension's `Ari: Lint Current File` command invokes this tool
through its `ari.lintPath` setting.

## Current Scope

- Uses saved source files, not editor buffers.
- Runs the compiler in `--check` mode.
- Emits normalized line/column diagnostics.
- Provides a JSON shape shared by the LSP server.

Native lint rules will live here later instead of being embedded in the
compiler's semantic lowering code.

## Developer Notes

Progress and planned work are tracked in [Lint Roadmap](dev/roadmap.md).
