# Ari Documentation

The docs are split by audience.

## Start Here

- New Ari user: [Getting Started](language/getting-started.md), then
  [Language Tour](language/language-tour.md), then
  [Quick Reference](language/quick-reference.md), then
  [Cookbook](language/cookbook.md), then
  [Feature Crosswalk](language/feature-crosswalk.md).
- New compiler contributor:
  [Compiler Development Dashboard](dev/compiler-development-dashboard.md), then
  [Compiler Onboarding](dev/compiler-onboarding.md), then
  [Compiler Contributor Guide](dev/compiler-contributor-guide.md), then
  [Compiler Concepts Glossary](dev/compiler-concepts-glossary.md), then
  [Compiler Layer Map](dev/compiler-layer-map.md), then
  [Compiler Triage Guide](dev/compiler-triage-guide.md), then
  [Compiler Source Identity](dev/compiler-source-identity.md), then
  [Compiler Module Project Authoring](dev/compiler-module-project-authoring.md),
  then
  [Compiler Artifact Authoring](dev/compiler-artifact-authoring.md), then
  [Compiler Diagnostic Authoring](dev/compiler-diagnostic-authoring.md), then
  [Compiler Test Authoring](dev/compiler-test-authoring.md), then
  [Compiler Development Roadmap](dev/compiler-development-roadmap.md), then
  [Compiler Implementation Playbook](dev/compiler-implementation-playbook.md),
  then [Compiler Next Slices](dev/compiler-next-slices.md), then
  [Compiler Change Checklist](dev/compiler-change-checklist.md), then
  [Core Language Readiness](dev/core-language-readiness.md), then
  [Minimum Trait Readiness](dev/trait-minimum-readiness.md), then
  [Build And Test](dev/build-test.md).

## Language Docs

Use these when writing Ari code:

- [Language Overview](language/README.md)
- [Getting Started](language/getting-started.md)
- [Language Tour](language/language-tour.md)
- [Quick Reference](language/quick-reference.md)
- [Cookbook](language/cookbook.md)
- [Feature Status](language/feature-status.md)
- [Feature Crosswalk](language/feature-crosswalk.md)
- [Examples And Tests](language/examples-and-tests.md)
- [Functions](language/functions.md)
- [Variables](language/variables.md)
- [Literals](language/literals.md)
- [Types](language/types.md)
- [Generic Aggregates](language/generic-aggregates.md)
- [Operators](language/operators.md)
- [Control Flow](language/control-flow.md)
- [Attributes](language/attributes.md)
- [Modules](language/modules.md)
- [Enums And Pattern Matching](language/enums-patterns.md)
- [Traits](language/traits.md)
- [Prelude And Formatting](language/prelude.md)
- [Standard Library](language/standard-library.md)
- [C FFI And Libraries](language/ffi.md)
- [Memory And Ownership](language/memory.md)
- [Front-End Only Syntax](language/front-end-only.md)

## Standard Library Docs

Use these when writing or changing Ari libraries:

- [Standard Library Overview](stdlib/README.md)
- [Standard Library Module Map](stdlib/overview.md)
- [Standard Library API Reference](stdlib/api-reference.md)
- [Generated Standard Library API Index](stdlib/generated/api-index.md)
- [Standard Library Example Index](stdlib/examples.md)
- [Standard Library Stability Policy](stdlib/stability.md)
- [Standard Library Value Movement Contracts](stdlib/value-contracts.md)
- [Standard Library Platform Notes](stdlib/platform/README.md)
- [Standard Library Verification Matrix](stdlib/verification-matrix.md)
- [Standard Library Development](stdlib/library-development.md)
- [Standard Library Production Readiness](stdlib/production-readiness.md)
- [Standard Library Testing](stdlib/testing.md)
- [Standard Library Roadmap](stdlib/roadmap.md)

## Developer Docs

Use these when changing the compiler:

- [Developer Overview](dev/README.md)
- [Architecture](dev/architecture.md)
- [Compiler Pipeline](dev/compiler-pipeline.md)
- [Compiler Development Dashboard](dev/compiler-development-dashboard.md)
- [Compiler Onboarding](dev/compiler-onboarding.md)
- [Compiler Contributor Guide](dev/compiler-contributor-guide.md)
- [Compiler Concepts Glossary](dev/compiler-concepts-glossary.md)
- [Compiler Layer Map](dev/compiler-layer-map.md)
- [Compiler Triage Guide](dev/compiler-triage-guide.md)
- [Compiler Source Identity](dev/compiler-source-identity.md)
- [Compiler Module Project Authoring](dev/compiler-module-project-authoring.md)
- [Compiler Artifact Authoring](dev/compiler-artifact-authoring.md)
- [Compiler Diagnostic Authoring](dev/compiler-diagnostic-authoring.md)
- [Compiler Test Authoring](dev/compiler-test-authoring.md)
- [Compiler Implementation Playbook](dev/compiler-implementation-playbook.md)
- [Compiler Next Slices](dev/compiler-next-slices.md)
- [Compiler Change Checklist](dev/compiler-change-checklist.md)
- [Compiler Readiness Inventory](dev/compiler-readiness-inventory.md)
- [Minimum Trait Readiness](dev/trait-minimum-readiness.md)
- [Compiler Pass Contracts](dev/compiler-pass-contracts.md)
- [Core Language Readiness](dev/core-language-readiness.md)
- [Generic Aggregate Monomorphization](dev/generic-aggregate-monomorphization.md)
- [Trait-Backed Operators](dev/operator-trait-design.md)
- [Build And Test](dev/build-test.md)
- [Aggregate ABI Classification](dev/aggregate-abi.md)
- [Symbol Mangling](dev/symbol-mangling.md)
- [Feature Test Matrix](dev/test-matrix.md)
- [Runtime And Backend](dev/runtime-backend.md)
- [Runtime Support Roadmap](dev/runtime-support.md)
- [Standard Library Roadmap](dev/standard-library-roadmap.md)
- [Compiler Development Roadmap](dev/compiler-development-roadmap.md)
- [Compiler Maturity Gates](dev/compiler-maturity-gates.md)
- [Compiler Project Model](dev/compiler-project-model.md)
- [Compiler Source And Diagnostics](dev/compiler-source-diagnostics.md)
- [Compiler Artifact Testing](dev/compiler-artifact-testing.md)
- [Library Testing](dev/library-testing.md)
- [Roadmap](dev/roadmap.md)
- [Completed Milestones](dev/completed-milestones.md)
- [Semantic Checker Decomposition](dev/sema-decomposition.md)

Long-term self-hosting appendix:

- [Production Compiler Design](dev/production-compiler-design.md)
- [Compiler Bootstrap Fixture Plan](dev/bootstrap-fixture-plan.md)
- [Bootstrap Readiness](dev/bootstrap-readiness.md)
- [Self-Host Roadmap](dev/self-host-roadmap.md)

## Tooling Docs

Use these when changing editor and lint tooling:

- [Ari Lint](lint/README.md)
- [Ari Lint Features](lint/features.md)
- [Ari LSP](lsp/README.md)
- [Ari LSP Features](lsp/features.md)
- [Ari VS Code Extension](vscode/README.md)
- [Ari VS Code Features](vscode/features.md)
- [Ari VS Code Usage](vscode/usage.md)
- [Ari VS Code Release](vscode/release.md)

## Notes

Use these for design direction and handoff context:

- [Notes Overview](notes/README.md)
- [Codex Project Notes](notes/codex-notes.md)
- [Memory Direction Notes](notes/memory-direction.md)

## Legacy Entry Points

These files are kept so old links and open editor tabs still land somewhere
useful:

- [PROJECT.md](PROJECT.md)
- [SYNTAX.md](SYNTAX.md)
- [MEMORY.md](MEMORY.md)
