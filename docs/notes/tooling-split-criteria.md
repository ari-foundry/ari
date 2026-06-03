# Tooling Split Criteria

This note defines when Ari tooling may move out of the `ari` repository. It is
a planning note only: this document does not split tooling by itself.

## Current Bundled Status

- ari-lint is currently bundled in the ari repository.
- ari-lsp is currently bundled in the ari repository.
- editor integrations currently live under the ari repository.
- This document does not split tooling by itself.

## Split Candidates

Potential future split candidates are:

- `ari-lint`
- `ari-lsp`
- `ari-vscode` or future editor integration repositories

## General Split Criteria

A tool may move to a separate repository only when:

- it has a stable command-line or protocol boundary
- it can be built and tested independently
- it has a clear dependency relationship with the Ari compiler
- compatibility with ari releases can be documented
- its docs can move with it without breaking Ari docs
- the Ari Foundry portal can link its new project location
- CI can run without relying on undocumented local `ari` repository state

## ari-lint Criteria

Before `ari-lint` splits:

- lint rule registry and severity handling must be stable enough to version
- JSON diagnostic format must be documented
- integration with `ari --check` must be explicit
- tests must be runnable outside the `ari` monorepo or through a documented fixture
- `docs/lint` content must have a clear new home

## ari-lsp Criteria

Before `ari-lsp` splits:

- LSP protocol surface must be stable enough to version
- dependency on `ari-lint` or shared lint checker must be decided
- diagnostics transport must be documented
- editor-facing settings must be documented
- tests must be runnable outside the `ari` monorepo or through documented fixtures
- `docs/lsp` content must have a clear new home

## Editor Integration Criteria

Before editor integrations split:

- `ari-lsp` binary discovery must be stable
- package/release workflow must be defined
- generated assets must be reproducible
- settings and install docs must move with the editor repo
- release cadence must justify separation from `ari`

## Explicit Non-Goals

- Do not split tools/lint in this step.
- Do not split tools/lsp in this step.
- Do not move editors in this step.
- Do not create ari-lint, ari-lsp, or ari-vscode repositories in this step.
- Do not modify Makefile or tests/Makefile in this step.
- Do not create arix in this step.

## Relationship To The Portal

The Ari Foundry portal tracks project status and compatibility. New tooling
repos should only be linked from the portal once they actually exist.
Do not invent future repository links.
