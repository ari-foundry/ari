# Ari Documentation

The docs are split by audience. Start with the smallest path that matches what
you are doing, then follow links only when you need deeper detail.

## Start Here

- New Ari user: [Getting Started](language/getting-started.md), then
  [Language Tour](language/language-tour.md), then
  [Quick Reference](language/quick-reference.md), then
  [Cookbook](language/cookbook.md), then
  [Feature Crosswalk](language/feature-crosswalk.md).
- New compiler contributor:
  [Developer Overview](dev/README.md), then
  [Architecture](dev/architecture.md), then
  [Compiler Pipeline](dev/compiler-pipeline.md), then
  [Compiler Layer Map](dev/compiler-layer-map.md), then
  [Compiler Source Identity](dev/compiler-source-identity.md), then
  [Compiler Diagnostic Authoring](dev/compiler-diagnostic-authoring.md), then
  [Compiler Artifact Authoring](dev/compiler-artifact-authoring.md), then
  [Compiler Test Authoring](dev/compiler-test-authoring.md), then
  [Build And Test](dev/build-test.md), then
  [Compiler Readiness Inventory](dev/compiler-readiness-inventory.md), then
  [Roadmap](dev/roadmap.md).

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

## Developer Docs

Use these when changing the current C++ hosted compiler:

- [Developer Overview](dev/README.md)
- [Architecture](dev/architecture.md)
- [Compiler Pipeline](dev/compiler-pipeline.md)
- [Compiler Layer Map](dev/compiler-layer-map.md)
- [Compiler Concepts Glossary](dev/compiler-concepts-glossary.md)
- [Compiler Triage Guide](dev/compiler-triage-guide.md)
- [Compiler Source Identity](dev/compiler-source-identity.md)
- [Compiler Module Project Authoring](dev/compiler-module-project-authoring.md)
- [Compiler Diagnostic Authoring](dev/compiler-diagnostic-authoring.md)
- [Compiler Artifact Authoring](dev/compiler-artifact-authoring.md)
- [Compiler Test Authoring](dev/compiler-test-authoring.md)
- [Compiler Readiness Inventory](dev/compiler-readiness-inventory.md)
- [Compiler Pass Contracts](dev/compiler-pass-contracts.md)
- [Core Language Readiness](dev/core-language-readiness.md)
- [Minimum Trait Readiness](dev/trait-minimum-readiness.md)
- [Ownership Drop Readiness](dev/ownership-drop-readiness.md)
- [Generic Aggregate Monomorphization](dev/generic-aggregate-monomorphization.md)
- [Trait-Backed Operators](dev/operator-trait-design.md)
- [Aggregate ABI Classification](dev/aggregate-abi.md)
- [Symbol Mangling](dev/symbol-mangling.md)
- [Runtime And Backend](dev/runtime-backend.md)
- [Runtime Support Roadmap](dev/runtime-support.md)
- [Semantic Checker Decomposition](dev/sema-decomposition.md)
- [Feature Test Matrix](dev/test-matrix.md)
- [Build And Test](dev/build-test.md)
- [Roadmap](dev/roadmap.md)

## Standard Library Docs

Use these when writing or changing Ari libraries. Standard library maturity is
tracked separately from compiler-writing readiness.

- [Standard Library Overview](stdlib/README.md)
- [Standard Library Module Map](stdlib/overview.md)
- [Standard Library API Reference](stdlib/api-reference.md)
- [Generated Standard Library API Index](stdlib/generated/api-index.md)
- [Standard Library Example Index](stdlib/examples.md)
- [Standard Library Completion Status](stdlib/completion-status.md)
- [Standard Library Stability Policy](stdlib/stability.md)
- [Standard Library Value Movement Contracts](stdlib/value-contracts.md)
- [Standard Library Platform Notes](stdlib/platform/README.md)
- [Standard Library Verification Matrix](stdlib/verification-matrix.md)
- [Standard Library Development](stdlib/library-development.md)
- [Standard Library Production Readiness](stdlib/production-readiness.md)
- [Standard Library Testing](stdlib/testing.md)
- [Standard Library Roadmap](stdlib/roadmap.md)

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
