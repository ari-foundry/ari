# Ari Documentation

Detailed Ari language, compiler, standard library, and developer docs remain in
this `ari` repository. Use this index by audience, then follow links only when
you need deeper detail.

The [Ari Foundry portal](https://ari-foundry.github.io) is the ecosystem entry
point for discovery, releases, install entry points, and compatibility.

## Start Here

- New Ari user: [Getting Started](language/getting-started.md), then
  [Language Tour](language/language-tour.md), then
  [Quick Reference](language/quick-reference.md), then
  [Cookbook](language/cookbook.md), then
  [Feature Crosswalk](language/feature-crosswalk.md).
- Standard library user: [Standard Library Overview](stdlib/README.md), then
  [Module Map](stdlib/overview.md), then
  [API Reference](stdlib/api-reference.md), then
  [Example Index](stdlib/examples.md).
- Compiler user: [Getting Started](language/getting-started.md), then the
  compiler CLI notes there, then [Build And Test](dev/build-test.md), then
  [Compiler Readiness Inventory](dev/compiler-readiness-inventory.md).
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
- Ari-written compiler source work:
  [Ari-Written Compiler](notes/ari-written-compiler.md), then its temporary
  split-out [status](notes/ari-written-compiler-status.md),
  [roadmap](notes/ari-written-compiler-roadmap.md),
  [tasks](notes/ari-written-compiler-tasks.md), and
  [validation/follow-up](notes/ari-written-compiler-validation.md) notes, then
  the source files directly under `compiler/`.
- Ecosystem discovery, releases, installs, and compatibility:
  [Ari Foundry](https://ari-foundry.github.io).

## Ecosystem Portal

[Ari Foundry](https://ari-foundry.github.io) links Ari ecosystem projects,
release information, install entry points, and compatibility information. Keep
using this `ari` repository for detailed Ari language docs, compiler docs,
standard library docs, bundled tooling docs, and developer docs.

## Language Users

Use these Language Docs when writing Ari code:

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
- [Memory Model](language/memory-model.md)
- [Front-End Only Syntax](language/front-end-only.md)

## Standard Library Users

Use these Standard Library Docs when writing or changing Ari libraries. Standard
library maturity is tracked separately from compiler-writing readiness.

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

## Compiler Users

Use these when building Ari, invoking the compiler, linking C libraries, or
reading compiler-visible behavior:

- [Getting Started](language/getting-started.md)
- [Quick Reference](language/quick-reference.md)
- [Examples And Tests](language/examples-and-tests.md)
- [C FFI And Libraries](language/ffi.md)
- [Build And Test](dev/build-test.md)
- [Compiler Artifact Authoring](dev/compiler-artifact-authoring.md)
- [Compiler Readiness Inventory](dev/compiler-readiness-inventory.md)

## Compiler Contributors

Use these Developer Docs when changing the current C++ hosted compiler:

- [Developer Overview](dev/README.md)
- [Architecture](dev/architecture.md)
- [Compiler Pipeline](dev/compiler-pipeline.md)
- [Compiler Layer Map](dev/compiler-layer-map.md)
- [Compiler Concepts Glossary](dev/compiler-concepts-glossary.md)
- [Compiler Triage Guide](dev/compiler-triage-guide.md)
- [Compiler Source Identity](dev/compiler-source-identity.md)
- [Compiler Module Project Authoring](dev/compiler-module-project-authoring.md)
- [Current-Zone Blocks](dev/current-zone-blocks.md)
- [Compiler Diagnostic Authoring](dev/compiler-diagnostic-authoring.md)
- [Compiler Artifact Authoring](dev/compiler-artifact-authoring.md)
- [Compiler Test Authoring](dev/compiler-test-authoring.md)
- [Compiler Readiness Inventory](dev/compiler-readiness-inventory.md)
- [Compiler-Bound Standard Library Gaps](dev/compiler-bound-stdlib-gaps.md)
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

## Bootstrap / Ari-written compiler work

Use these when working on the Ari-written compiler source root under
`compiler/`:

- [Ari-Written Compiler](notes/ari-written-compiler.md)
- [Ari-Written Compiler Status](notes/ari-written-compiler-status.md)
- [Ari-Written Compiler Roadmap](notes/ari-written-compiler-roadmap.md)
- [Ari-Written Compiler Tasks](notes/ari-written-compiler-tasks.md)
- [Ari-Written Compiler Validation And Follow-Ups](notes/ari-written-compiler-validation.md)

## Bundled Tooling Docs

Use these bundled Tooling Docs when changing lint, LSP, and editor tooling.
These docs are currently bundled in `ari`; future stable tooling projects may
carry their docs with them if they split out. Split criteria are tracked in
[Tooling Split Criteria](notes/tooling-split-criteria.md).

- [Ari Lint](lint/README.md) - separate repository initialized at
  https://github.com/ari-foundry/ari-lint; migration is still in progress for
  docs/source.
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
- [Ari-Written Compiler](notes/ari-written-compiler.md)
- [Ari-Written Compiler Status](notes/ari-written-compiler-status.md)
- [Ari-Written Compiler Roadmap](notes/ari-written-compiler-roadmap.md)
- [Ari-Written Compiler Tasks](notes/ari-written-compiler-tasks.md)
- [Ari-Written Compiler Validation And Follow-Ups](notes/ari-written-compiler-validation.md)
- [Codex Project Notes](notes/codex-notes.md)
- [Memory Direction Notes](notes/memory-direction.md)

## Legacy Entry Points

These files are kept so old links and open editor tabs still land somewhere
useful:

- [PROJECT.md](PROJECT.md)
- [SYNTAX.md](SYNTAX.md)
- [MEMORY.md](MEMORY.md)
