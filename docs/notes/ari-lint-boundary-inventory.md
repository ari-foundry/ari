# ari-lint Boundary Inventory

## Purpose

This document inventories the current `ari-lint` boundary before any future
repository split. It supports the planning tracked in Issue #7 and the split
criteria in [Tooling Split Criteria](tooling-split-criteria.md).

This document does not move files, create repositories, or change build
behavior.

## Current Status

- `ari-lint` is currently bundled in the `ari` repository.
- `ari-lint` is built by the `ari` repository build system.
- `ari-lint` currently delegates compiler-backed checking to `ari --check`.
- lint-specific docs remain under `docs/lint` until a future split.

## Source Inventory

Current ari-lint implementation files:

- `tools/lint/README.md`
- `tools/lint/main.cpp`
- `tools/lint/checker.hpp`
- `tools/lint/checker.cpp`
- `tools/lint/config.hpp`
- `tools/lint/config.cpp`
- `tools/lint/rules.hpp`
- `tools/lint/rules.cpp`
- `tools/lint/source_rules.hpp`
- `tools/lint/source_rules.cpp`

Shared tooling helper files used by ari-lint:

- `tools/ari_tooling/diagnostic.hpp`
- `tools/ari_tooling/diagnostic.cpp`
- `tools/ari_tooling/process.hpp`
- `tools/ari_tooling/process.cpp`

Direct dependencies discovered while inspecting the code:

- `Makefile` builds `build/ari-lint` from `tools/lint/*.cpp` plus
  `tools/ari_tooling/*.cpp`.
- `Makefile` builds `build/ari-lsp` with the lint library objects from
  `tools/lint/*.cpp` except `tools/lint/main.cpp`.
- `tools/lsp/main.cpp` includes `../lint/checker.hpp` and
  `../lint/config.hpp`, so LSP diagnostics share the lint checker library.
- `tools/lint/checker.cpp` depends on an Ari compiler executable path and runs
  that process with `--check`.
- `tools/lint/main.cpp` defaults the compiler executable to `build/ari` or the
  `ARI_COMPILER` environment variable unless `--ari` is provided.

Needs follow-up: the future split boundary must decide whether `ari-lint` keeps
calling `ari --check` as a process boundary or moves to a documented shared
library/API boundary.

## Test Inventory

Source-controlled tests and fixtures that directly exercise `ari-lint`:

- `tests/Makefile`: the `check-tools` target builds and exercises
  `$(LINT_TARGET)` with `--ari`, `--list-rules`, `--config`, `--rule`, and
  `--json`.
- `tests/cases/standard-library/errors/format/prelude-macro-format-no-default-zone.ari`:
  used by `check-tools` to verify compiler-backed diagnostics are surfaced
  through `ari-lint` JSON output.

Source-controlled tests that exercise the shared lint checker through LSP:

- `tests/tools/lsp_smoke.sh`: writes `build/tools/ari-lint.rules` and verifies
  `ari-lsp` reports `ari-lint` diagnostics for `lint/trailing-whitespace`.

The lint tests are currently mixed into the repository-level `check-tools`
target. There is not a separate source-controlled ari-lint-only test manifest.

Local generated artifacts observed in this checkout:

- `build/tools/ari-lint.rules`
- `build/tools/lint-trailing-whitespace.ari`
- `build/tools/lint-trailing.json`
- `build/tools/lint-missing-final-newline.ari`
- `build/tools/lint-missing-final-newline.json`
- `build/tools/lint-format.json`
- `build/tools/lsp-smoke.ari`

These generated files are build outputs, not source-controlled fixtures.

## Documentation Inventory

Current `ari-lint` documentation files:

- `docs/lint/README.md`
- `docs/lint/features.md`
- `docs/lint/dev/README.md`
- `docs/lint/dev/roadmap.md`

## Compiler Boundary

`tools/lint/checker.cpp` constructs a compiler command from `config.ari_path`,
configured `-I` module paths, the source file path, and `--check`, then invokes
that command through `tools/ari_tooling/process.cpp`.

`ari-lint` does not currently link the compiler implementation from `src/*.cpp`.
It links lint implementation files and shared tooling helper files. Compiler
diagnostics are parsed after `ari --check` runs by
`tools/ari_tooling/diagnostic.cpp`, then `ari-lint` runs native source rules and
combines the diagnostics.

Diagnostics are normalized after compiler checking into
`ari::tooling::Diagnostic`. JSON output is printed by `tools/lint/main.cpp`.
The JSON diagnostic shape is shared with LSP in practice because `ari-lsp` links
the lint checker library directly and serializes the same diagnostic model for
editor diagnostics.

This is a current process-and-shared-tooling boundary, not a proven stable split
boundary.

## CLI Boundary

Documented and locally observed help output currently exposes:

- `--json`
- `--ari PATH`
- `-I DIR`
- `--list-rules`
- `--config PATH`
- `--rule RULE=SEVERITY`

`docs/lint/README.md` also documents `ARI_COMPILER` as an environment fallback
for the compiler executable when `--ari` is not provided.

Local help output from `build/ari-lint --help` is:

```text
usage: ari-lint [--ari PATH] [--json] [--list-rules] [--config PATH] [--rule RULE=SEVERITY] [-I DIR] FILE...
```

## Rule / Severity Boundary

Current registered lint rules:

- `lint/trailing-whitespace`
- `lint/missing-final-newline`

Current severity settings accepted by `--rule` and `ari-lint.rules`:

- `off`
- `hint`
- `note`
- `warning`
- `error`

Rule names may be provided as full codes such as `lint/trailing-whitespace` or
short names such as `trailing-whitespace`; short names are normalized to the
`lint/` namespace. When `--config PATH` is not provided, `ari-lint` searches
upward from each source file for `ari-lint.rules`.

## Split Risks

- dependency on `ari --check`
- shared diagnostics with `ari-lsp`
- test fixtures coupled to `ari` repo layout
- Makefile integration
- `docs/lint` migration
- compatibility with Ari compiler releases

## Follow-up Checklist

- [ ] Confirm source inventory is complete
- [ ] Confirm lint-specific tests are complete
- [ ] Confirm docs/lint inventory is complete
- [ ] Decide `ari --check` vs shared library boundary
- [ ] Document JSON diagnostic format
- [ ] Document standalone test strategy
- [ ] Define compatibility matrix expectations
- [ ] Decide future repository name only when ready

## Explicit Non-Goals

- Do not move `tools/lint` in this step.
- Do not move `docs/lint` in this step.
- Do not create `ari-foundry/ari-lint` in this step.
- Do not modify `Makefile` or `tests/Makefile` in this step.
- Do not modify compiler source in this step.
- Do not invent future repository links.
- Do not modify `ari-foundry.github.io` in this step.
