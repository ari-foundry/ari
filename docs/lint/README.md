# Ari Lint

`ari-lint` is the standalone lint entry point for Ari tooling. It delegates
parser, module-loader, and semantic diagnostics to `ari --check`, then runs
lint-only source rules and normalizes diagnostics into either human-readable
text or JSON.

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
build/ari-lint --json tests/cases/standard-library/errors/format/prelude-macro-format-no-default-zone.ari
build/ari-lint --ari build/ari -I path/to/modules source.ari
build/ari-lint --list-rules
build/ari-lint --config ari-lint.rules source.ari
build/ari-lint --rule lint/trailing-whitespace=error source.ari
build/ari-lint --rule lint/missing-final-newline=off generated.ari
```

`ARI_COMPILER` can also point the tool at a compiler binary when `--ari` is not
provided.

`--rule RULE=SEVERITY` accepts `off`, `hint`, `note`, `warning`, or `error`.
Rule names may use the full code, such as `lint/trailing-whitespace`, or the
short lint name, such as `trailing-whitespace`.

When `--config PATH` is not provided, `ari-lint` searches from each source file's
directory upward for `ari-lint.rules`. `--config PATH` uses that file explicitly
and disables discovery. In both cases, command-line `--rule` overrides are
applied last.

The config format is intentionally small: one `RULE = SEVERITY` setting per
line, blank lines allowed, and `#` comments supported.

```text
# ari-lint.rules
lint/trailing-whitespace = error
lint/missing-final-newline = warning
```

The VS Code extension's `Ari: Lint Current File` command invokes this tool
through its `ari.lintPath` setting.

`ari-lsp` links the lint checker library directly, so editor diagnostics and
the CLI share the same rule registry and severity handling.

## Current Scope

- Uses saved source files, not editor buffers.
- Runs the compiler in `--check` mode.
- Runs native lint rules after compiler checking.
- Emits normalized line/column diagnostics, with `endLine`/`endColumn` in JSON
  when tools need a machine-readable span.
- Provides a JSON shape shared by the LSP server.
- Emits lint rule codes when a diagnostic comes from a native lint rule.

## Feature Guide

The current lint feature set is documented in [Ari Lint Features](features.md).

## Rules

### `lint/trailing-whitespace`

Reports spaces or tabs at the end of a source line. The default severity is
`warning`. Use `--rule lint/trailing-whitespace=off` to disable it for generated
or transitional sources, or set it to `error` when a CI job should reject it.

### `lint/missing-final-newline`

Reports non-empty source files that do not end with a newline. The default
severity is `warning`. Use `--rule lint/missing-final-newline=off` for generated
files that intentionally omit the final newline, or set it to `error` when CI
should enforce repository text-file hygiene.

## Developer Notes

Progress and planned work are tracked in [Lint Roadmap](dev/roadmap.md).
