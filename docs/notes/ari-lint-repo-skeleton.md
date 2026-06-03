# ari-lint Repository Skeleton

## Purpose

This document proposes a future standalone `ari-lint` repository layout.

This document does not create a repository, move files, add CI, or change build
behavior. It is a planning note for split approval and handoff work.

## Current Status

- ari-lint is currently bundled in the ari repository.
- tools/lint remains in ari.
- docs/lint remains in ari.
- The preferred near-term dependency model is invoking ari --check.
- The standalone test plan expects an external Ari compiler binary.

## Proposed Future Repository

The future repository name should only be finalized when the split is approved.
Until then, this note uses the placeholder: future ari-lint repository.

Do not link to a guessed future GitHub repository.

## Proposed Directory Layout

Proposed planning skeleton:

```text
ari-lint/
  README.md
  AGENTS.md
  LICENSE
  docs/
    README.md
    features.md
    dev/
      roadmap.md
  src/
  tests/
    fixtures/
    golden/
  examples/
  scripts/
  .github/
    workflows/
      check.yml
```

This is a planning layout, not an instruction to create files now.

## Source Migration Candidates

The current boundary inventory identifies the lint implementation under
`tools/lint` and shared helper code under `tools/ari_tooling`.

tools/lint is the expected source migration area.

Any shared checker or diagnostic code requires a separate dependency decision.
The inventory notes that `ari-lint` currently uses shared tooling helpers and
that `ari-lsp` links the lint checker library, so moving shared code cannot be
treated as automatic.

Dependencies that are unclear must remain marked as needs follow-up.

Do not claim files are ready to move unless the inventory proves it.

## Documentation Migration Candidates

docs/lint/README.md should become the future repo README or docs entry point.
docs/lint/features.md should move with ari-lint docs.
docs/lint/dev/roadmap.md should move with ari-lint planning docs.

docs links in ari must become redirects or handoff links only after the future
repository exists.

## Test And Fixture Layout

The standalone test plan should drive the future layout:

- tests/fixtures should contain source fixtures.
- tests/golden should contain expected JSON outputs once schema is stable.
- tests should avoid absolute paths.
- tests should use an explicit --ari compiler path in CI.
- Config fixtures should cover ari-lint.rules and --rule overrides.

## CI Expectations

Future CI should:

- Build ari-lint.
- Provision or receive a compatible Ari compiler binary.
- Run CLI smoke tests.
- Run rule tests.
- Run config tests.
- Run JSON diagnostic tests.
- Run compiler-boundary tests.

Do not add CI now.

## Compatibility And Release Metadata

The future repository should eventually document:

- compatible Ari compiler versions
- ari-lint release version
- JSON diagnostic schema version if needed
- rule registry/severity compatibility
- config format compatibility

The Ari Foundry portal compatibility matrix should be updated only after the
repository and release plan actually exist.

## AGENTS.md Expectations

A future ari-lint repository should have AGENTS.md that says:

- ari-lint is tooling, not the Ari compiler.
- Compiler behavior belongs to ari.
- ari-lint invokes ari --check unless the dependency model changes.
- Do not copy broad Ari language/compiler docs into ari-lint.
- Keep docs focused on lint CLI, rules, diagnostics, config, tests, and
  releases.

## Split Readiness Checklist

- [ ] Source migration candidates confirmed
- [ ] Documentation migration candidates confirmed
- [ ] Standalone test fixtures defined
- [ ] JSON golden fixture policy accepted
- [ ] External Ari compiler provisioning decided
- [ ] Compatibility matrix update plan defined
- [ ] Future repository name approved
- [ ] Future repository creation approved
- [ ] Ari repo handoff links planned
- [ ] Ari Foundry portal update planned

## Explicit Non-Goals

- Do not move `tools/lint` in this step.
- Do not move `docs/lint` in this step.
- Do not create `ari-foundry/ari-lint` in this step.
- Do not modify `Makefile` or `tests/Makefile` in this step.
- Do not modify compiler source in this step.
- Do not change ari-lint behavior in this step.
- Do not add standalone tests in this step.
- Do not add CI in this step.
- Do not invent future repository links.
- Do not modify `ari-foundry.github.io` in this step.
