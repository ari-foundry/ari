# ari-lint CLI and Diagnostic Contract

## Purpose

This document records the current public CLI and diagnostic contract for
`ari-lint` before any future repository split.

This document does not move files, create repositories, or change build
behavior. It describes the current tool surface so a future split can preserve
known behavior and identify gaps before promising compatibility.

## Current Status

- ari-lint is currently bundled in the ari repository.
- ari-lint is built by the ari repository build system.
- ari-lint currently delegates compiler-backed checking to ari --check.
- This contract is descriptive unless the inspected tests/docs prove a behavior
  is stable.

## CLI Surface

Current usage from `tools/lint/main.cpp`:

```text
usage: ari-lint [--ari PATH] [--json] [--list-rules] [--config PATH] [--rule RULE=SEVERITY] [-I DIR] FILE...
```

Supported inputs and options:

| Input or option | Purpose | Documented behavior | Confirmation |
| --- | --- | --- | --- |
| Positional source file input | Select one or more Ari source files to lint. | `docs/lint/README.md` shows source file arguments; source requires at least one file unless `--list-rules` is used. | Source and docs. |
| `--json` | Emit machine-readable results. | JSON output is intended for editor and CI integration. | Source, docs, and `tests/Makefile`. |
| `--ari PATH` | Choose the Ari compiler executable used for compiler-backed checks. | `docs/lint/README.md` documents `--ari build/ari`; source stores the provided path as `config.ari_path`. | Source, docs, and tests. |
| `-I DIR` and `-IDIR` | Pass module include paths to the compiler invocation. | `docs/lint/README.md` and `docs/lint/features.md` show `-I`; source forwards each module path before the source file and `--check`. | Source and docs; standalone path policy needs follow-up. |
| `--list-rules` | Print registered lint rules and default severities. | Source prints each registered rule with `default=` and exits successfully. | Source and `tests/Makefile`. |
| `--config PATH` and `--config=PATH` | Load a specific rule config file. | `docs/lint/README.md` says this disables discovery; source loads the file before CLI rule overrides. | Source and docs. |
| `--rule RULE=SEVERITY` and `--rule=RULE=SEVERITY` | Override a rule severity. | Full rule codes and short rule names are accepted; CLI overrides are applied after config-file settings. | Source, docs, and `tests/Makefile`. |
| `ARI_COMPILER` | Provide the default compiler executable when `--ari` is not passed. | `docs/lint/README.md` documents this fallback; source checks the environment before falling back to `build/ari`. | Source and docs; explicit precedence fixtures need follow-up. |

Do not invent options. Any future split should keep unsupported options rejected
or document an intentional change.

## Config File Contract

The rule config file is named `ari-lint.rules`.

Current behavior:

- When `--config PATH` is not provided, `ari-lint` searches upward from each
  source file's directory for `ari-lint.rules`.
- `--config PATH` uses that file explicitly and disables discovery.
- The config file accepts one `RULE = SEVERITY` setting per line.
- Blank lines are allowed.
- `#` comments are supported.
- Command-line `--rule` overrides are applied last.

These behaviors are documented in `docs/lint/README.md` and
`docs/lint/features.md`, and are implemented in `tools/lint/config.cpp` and
`tools/lint/checker.cpp`. Config discovery behavior outside the `ari`
repository needs follow-up before a split.

## Rule and Severity Contract

Current lint rule identifiers:

- `lint/trailing-whitespace`
- `trailing-whitespace` short name
- `lint/missing-final-newline`
- `missing-final-newline` short name

Current severities accepted by `--rule` and `ari-lint.rules`:

- `off`
- `hint`
- `note`
- `warning`
- `error`

Both current rules default to `warning`, as documented in `docs/lint/README.md`
and `docs/lint/features.md`, and implemented in `tools/lint/rules.cpp`.

The source also accepts `info` and `information` as aliases for the internal
information severity. Those aliases are implementation behavior and need
follow-up before they are treated as part of the public contract.

## Diagnostic Output Contract

Human-readable output is the default. When a file has no diagnostics and the
compiler check exits successfully, source prints:

```text
path/to/file.ari: ok
```

For diagnostics, human output prints file, line, column, severity, an optional
code in brackets, and the message. If the compiler process fails without a
parsed diagnostic, human output reports `compiler check failed`.

`--json` prints one JSON document containing a `files` array. Current source
emits per-file `path`, `exitCode`, and `diagnostics` fields. Current diagnostic
objects emit `file`, `line`, `column`, `endLine`, `endColumn`, `severity`,
`message`, `source`, and optional `code`.

The exact JSON schema is not fully documented and needs follow-up instead of an
invented guarantee. Existing docs state that JSON diagnostics include `line`,
`column`, `endLine`, `endColumn`, `severity`, `source`, and `code`.

Line and column values parsed from compiler diagnostics are normalized to at
least `1`. JSON `endLine` and `endColumn` are emitted for every diagnostic;
when a diagnostic does not provide an explicit end span, source falls back to
the start line and the next column.

Native lint diagnostics currently use `source` value `ari-lint` and rule codes
such as `lint/trailing-whitespace` or `lint/missing-final-newline`.
Compiler-backed diagnostics come from `ari --check`, use `source` value `ari`,
and keep explicit compiler codes when present. When no explicit compiler code
is parsed, the fallback code is `ari/compiler`. If compiler checking fails and
no diagnostic can be parsed, source emits `ari/compiler-check-failed`.

## Exit Status Contract

Current exit behavior from source and tests:

- Success exit status: `0` when all checked files have no diagnostics and the
  compiler check exits successfully. `--help` and `--list-rules` also return
  `0`.
- Compiler-check failure exit status: the top-level CLI returns `1` when the
  compiler process exits nonzero. The per-file JSON `exitCode` records the raw
  compiler process exit code reported by `tools/ari_tooling/process.cpp`.
- Lint-only diagnostic exit status: the top-level CLI returns `1` when any
  diagnostic is present, including diagnostics whose severity is `warning`.
  `tests/Makefile` confirms nonzero behavior for native lint diagnostics when
  rules are configured as `error`.
- Invalid CLI/config exit status: missing arguments, unknown options, config
  load errors, and invalid rule settings return `2` in source.

Unexpected process or runtime errors return `1` from the exception handler.
The exact public exit code policy needs follow-up before a future standalone
tool promises versioned behavior.

## Compiler Boundary

`ari-lint` delegates compiler-backed checking to `ari --check`.
`tools/lint/checker.cpp` builds the compiler command from the configured Ari
compiler path, every `-I` module path, the source file, and `--check`, then
invokes it through `tools/ari_tooling/process.cpp`.

`--ari PATH` sets the compiler executable path. If `--ari` is not provided,
source uses the `ARI_COMPILER` environment variable when present, otherwise
`build/ari`. This establishes `--ari` as the observed source-level override
after the environment fallback, but explicit precedence fixtures need
follow-up.

`-I` module paths are forwarded to the compiler process before the source file.
Standalone include path behavior outside the `ari` repository needs follow-up.

`ari-lint` currently shells out to the Ari compiler process. It does not link
the compiler implementation from `src/*.cpp`. It does link lint checker code
and shared tooling helper code under `tools/ari_tooling`. `ari-lsp` currently
links the same lint checker library, so LSP diagnostics may rely on the same
diagnostic shape.

## JSON Compatibility Risks

- JSON schema not fully documented.
- Compiler diagnostics and lint diagnostics may share output shape.
- LSP may rely on the same diagnostic shape.
- Rule severity names must remain compatible.
- Config discovery may depend on source file layout.
- Include path behavior may depend on ari repo conventions.

## Follow-up Checklist

- [ ] Document exact JSON schema
- [ ] Add or identify golden JSON diagnostic fixtures
- [ ] Define exit code policy
- [ ] Confirm compiler-check vs lint-only failure behavior
- [ ] Confirm config discovery behavior outside the ari repository
- [ ] Confirm ARI_COMPILER and --ari precedence
- [ ] Confirm -I behavior for standalone use
- [ ] Decide whether CLI contract is stable enough for versioning

## Explicit Non-Goals

- Do not move tools/lint in this step.
- Do not move docs/lint in this step.
- Do not create ari-foundry/ari-lint in this step.
- Do not modify Makefile or tests/Makefile in this step.
- Do not modify compiler source in this step.
- Do not change ari-lint behavior in this step.
- Do not invent future repository links.
- Do not modify ari-foundry.github.io in this step.
