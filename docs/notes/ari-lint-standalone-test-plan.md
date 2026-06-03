# ari-lint Standalone Test Plan

## Purpose

This document defines a test strategy for future standalone `ari-lint` repository
split planning.

This document does not move files, create repositories, add tests, or change build
behavior. It records what standalone coverage should exist before any future
split.

## Current Status

- `ari-lint` is currently bundled in the `ari` repository.
- `ari-lint` currently uses the `ari` repository build and test environment.
- The preferred near-term dependency model is invoking `ari --check`.
- Standalone testing must account for an external Ari compiler binary.

## Current Test Inventory

Current lint-related tests and fixtures are mixed into broader repository tests.
There is not a separate source-controlled ari-lint-only test manifest today.

Source-controlled paths that currently participate in lint-related coverage:

- `tests/Makefile`: the `check-tools` target builds and runs `ari-lint` and
  `ari-lsp`. It checks a valid file, `--list-rules`, native lint diagnostics,
  config loading, rule overrides, JSON output, and compiler-backed diagnostics.
- `tests/tools/lsp_smoke.sh`: exercises `ari-lsp` with shared lint behavior and
  checks `ari-lint` diagnostics for `lint/trailing-whitespace`.
- `examples/count.ari`: used by `tests/Makefile` as the current valid Ari file
  for `ari-lint`.
- `tests/cases/standard-library/errors/format/prelude-macro-format-no-default-zone.ari`:
  used by `tests/Makefile` to verify compiler-backed diagnostics through
  `ari-lint --json`.

The `check-tools` recipe currently creates temporary source, config, and JSON
outputs under the build directory while it runs. Those are generated build
outputs, not source-controlled standalone fixtures.

`find tests -type f | grep -i lint` currently returns no source-controlled file
paths. That confirms the lint coverage is embedded in broader test files rather
than isolated by filename.

Needs follow-up: the future standalone project must decide whether to copy,
rewrite, or replace the generated `check-tools` fixture flow with permanent
source-controlled fixtures.

## Standalone Fixture Goals

Future standalone fixtures should cover:

- Minimal valid Ari source file.
- Source file with trailing whitespace.
- Source file missing final newline.
- Source file that triggers compiler-backed diagnostics.
- Config file fixture using `ari-lint.rules`.
- Fixture using `--rule` overrides.
- Fixture using `--json` output.
- Fixture using include paths with `-I` if supported.

## Compiler Binary Strategy

Future standalone tests need a clear way to obtain an Ari compiler binary.
Candidate strategies:

- Use a prebuilt ari compiler artifact.
- Build ari from a pinned source checkout.
- Accept an ari binary path through `--ari`.
- Accept `ARI_COMPILER` if supported.

The first standalone split should prefer an explicit `--ari` path in CI because
it is easiest to reason about. `ARI_COMPILER` fallback behavior is implemented
and documented today, but standalone precedence and fallback tests need
follow-up before relying on it as the primary CI path.

Needs follow-up: missing compiler binary behavior, compiler version selection,
and whether tests should ever search `PATH` or other discovery locations are not
yet standalone guarantees.

## Test Categories

### CLI smoke tests

- ari-lint runs on a valid file.
- ari-lint reports usage or errors for invalid arguments.
- `--list-rules` works if supported.

### Rule tests

- `lint/trailing-whitespace`.
- `lint/missing-final-newline`.
- Severity override behavior.

### Config tests

- `ari-lint.rules` discovery.
- `--config PATH` behavior.
- Command-line `--rule` overriding config.

Needs follow-up: config discovery outside the `ari` repository is not yet proven
by standalone fixtures.

### Compiler boundary tests

- Missing compiler binary.
- Compiler check failure.
- Include path forwarding through `-I`.
- Compiler diagnostics plus lint diagnostics.

### JSON diagnostic tests

- `--json` output shape.
- Line/column fields.
- `endLine`/`endColumn` fields if supported.
- Lint rule code fields.
- Mixed compiler/lint diagnostic output.

Needs follow-up: the exact JSON schema is not fully documented yet, so golden
fixtures should wait for that contract.

## Golden Fixture Policy

- JSON outputs should use golden fixtures once the schema is documented.
- Human-readable output should be tested only where stable enough.
- Tests should avoid depending on absolute paths when possible.
- Diagnostics should normalize paths if needed.

Needs follow-up: exit-code expectations should be documented in tests before
standalone CI treats them as stable guarantees.

## CI Requirements

Future standalone CI should:

- Run ari-lint tests without relying on undocumented monorepo layout.
- Provision a compatible ari compiler binary.
- Run CLI, config, JSON, and compiler-boundary tests.
- Publish compatibility expectations with Ari compiler versions.

## Split Risks

- Test fixtures may depend on ari repository layout.
- Compiler binary provisioning may be fragile.
- JSON diagnostic schema may not be fully stable.
- Config discovery may depend on source file locations.
- Human-readable diagnostics may be unstable.
- LSP may depend on shared diagnostic shape.

## Follow-up Checklist

- [ ] Identify current lint test files and fixtures
- [ ] Decide standalone fixture directory layout
- [ ] Define compiler binary provisioning strategy
- [ ] Add golden JSON diagnostic fixtures
- [ ] Document exit-code expectations in tests
- [ ] Test missing compiler binary behavior
- [ ] Test compiler failure vs lint-only diagnostics
- [ ] Test config discovery outside the ari repository
- [ ] Define CI workflow expectations for a future ari-lint repo

## Explicit Non-Goals

- Do not move `tools/lint` in this step.
- Do not move `docs/lint` in this step.
- Do not create `ari-foundry/ari-lint` in this step.
- Do not modify `Makefile` or `tests/Makefile` in this step.
- Do not modify compiler source in this step.
- Do not change `ari-lint` behavior in this step.
- Do not add standalone tests in this step.
- Do not invent future repository links.
- Do not modify `ari-foundry.github.io` in this step.
