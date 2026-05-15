# Ari Lint Features

This page describes the current user-facing behavior of `ari-lint`.

## Compiler-Backed Diagnostics

`ari-lint` runs the Ari compiler in `--check` mode before native lint rules.
Parser, module-loader, semantic, ownership, and lowering diagnostics therefore
come from the same compiler path used by normal builds.

Compiler diagnostics keep explicit compiler-provided codes when present. Until
the compiler emits more specific structured codes, compiler-backed diagnostics
use the stable fallback code `ari/compiler`.

Useful commands:

```sh
build/ari-lint source.ari
build/ari-lint --ari build/ari source.ari
build/ari-lint -I modules source.ari
```

## Native Source Rules

Native lint rules run after compiler checks and use `ari-lint` diagnostic codes.

Current rules:

- `lint/trailing-whitespace`: reports spaces or tabs at the end of a source
  line.
- `lint/missing-final-newline`: reports non-empty source files that do not end
  with a newline.

Both rules default to `warning`.

## Rule Configuration

Rule severities can be set on the command line:

```sh
build/ari-lint --rule lint/trailing-whitespace=error source.ari
build/ari-lint --rule missing-final-newline=off source.ari
```

Allowed severities are `off`, `hint`, `note`, `warning`, and `error`. Short rule
names such as `trailing-whitespace` are normalized to `lint/trailing-whitespace`.

When `--config PATH` is not provided, `ari-lint` searches upward from each source
file for `ari-lint.rules`.

```text
lint/trailing-whitespace = error
lint/missing-final-newline = warning
```

Command-line `--rule` settings are applied after config-file settings.

## Output Modes

Human output is the default and is intended for terminal use.

```sh
build/ari-lint examples/count.ari
```

JSON output is intended for editor and CI integration.

```sh
build/ari-lint --json source.ari
```

JSON diagnostics include `line`, `column`, `endLine`, `endColumn`, `severity`,
`source`, and `code`.

## Tool Integration

`ari-lsp` links the same lint checker library, so editor diagnostics use the
same compiler checks, native rules, config discovery, and severity overrides as
the standalone CLI.
