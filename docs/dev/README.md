# Ari Developer Docs

This folder is for people changing the compiler.

## Read First

For normal compiler work, read in this order:

1. [Compiler Development Dashboard](compiler-development-dashboard.md)
2. [Compiler Contributor Guide](compiler-contributor-guide.md)
3. [Compiler Concepts Glossary](compiler-concepts-glossary.md)
4. [Compiler Layer Map](compiler-layer-map.md)
5. [Compiler Triage Guide](compiler-triage-guide.md)
6. [Compiler Source Identity](compiler-source-identity.md)
7. [Compiler Module Project Authoring](compiler-module-project-authoring.md)
8. [Compiler Artifact Authoring](compiler-artifact-authoring.md)
9. [Compiler Diagnostic Authoring](compiler-diagnostic-authoring.md)
10. [Compiler Test Authoring](compiler-test-authoring.md)
11. [Compiler Development Roadmap](compiler-development-roadmap.md)
12. [Compiler Implementation Playbook](compiler-implementation-playbook.md)
13. [Compiler Next Slices](compiler-next-slices.md)
14. [Compiler Change Checklist](compiler-change-checklist.md)
15. [Compiler Readiness Inventory](compiler-readiness-inventory.md)
16. [Build And Test](build-test.md)

Keep this path focused on the compiler that exists today: source loading,
lexing, parsing, sema, IR, LLVM output, diagnostics, modules, artifacts, and
focused tests. Self-hosting notes live in a separate appendix and should not
drive ordinary compiler design.

## Main Pages

- [Architecture](architecture.md)
- [Compiler Pipeline](compiler-pipeline.md)
- [Compiler Development Dashboard](compiler-development-dashboard.md)
- [Compiler Contributor Guide](compiler-contributor-guide.md)
- [Compiler Concepts Glossary](compiler-concepts-glossary.md)
- [Compiler Layer Map](compiler-layer-map.md)
- [Compiler Triage Guide](compiler-triage-guide.md)
- [Compiler Source Identity](compiler-source-identity.md)
- [Compiler Module Project Authoring](compiler-module-project-authoring.md)
- [Compiler Artifact Authoring](compiler-artifact-authoring.md)
- [Compiler Diagnostic Authoring](compiler-diagnostic-authoring.md)
- [Compiler Test Authoring](compiler-test-authoring.md)
- [Compiler Implementation Playbook](compiler-implementation-playbook.md)
- [Compiler Next Slices](compiler-next-slices.md)
- [Compiler Change Checklist](compiler-change-checklist.md)
- [Compiler Readiness Inventory](compiler-readiness-inventory.md)
- [Compiler Pass Contracts](compiler-pass-contracts.md)
- [Build And Test](build-test.md)
- [Aggregate ABI Classification](aggregate-abi.md)
- [Feature Test Matrix](test-matrix.md)
- [Runtime And Backend](runtime-backend.md)
- [Runtime Support Roadmap](runtime-support.md)
- [Symbol Mangling](symbol-mangling.md)
- [Standard Library Roadmap](standard-library-roadmap.md)
- [Compiler Development Roadmap](compiler-development-roadmap.md)
- [Compiler Maturity Gates](compiler-maturity-gates.md)
- [Compiler Project Model](compiler-project-model.md)
- [Compiler Source And Diagnostics](compiler-source-diagnostics.md)
- [Compiler Artifact Testing](compiler-artifact-testing.md)
- [Library Testing](library-testing.md)
- [Standard Library Docs](../stdlib/README.md)
- [Roadmap](roadmap.md)
- [Completed Milestones](completed-milestones.md)
- [Semantic Checker Decomposition](sema-decomposition.md)

## Long-Term Self-Hosting Appendix

These pages are not the active compiler roadmap. Use them only when a change
directly affects future self-hosting constraints:

- [Production Compiler Design](production-compiler-design.md)
- [Compiler Bootstrap Fixture Plan](bootstrap-fixture-plan.md)
- [Bootstrap Readiness](bootstrap-readiness.md)
- [Self-Host Roadmap](self-host-roadmap.md)

## Source Map

```text
src/lexer.cpp    tokenizes Ari source
src/parser.cpp   builds AST nodes from tokens
src/sema.cpp     resolves names, checks types, checks ownership, lowers to IR
src/ir.hpp       typed IR data model
src/symbol_mangle.cpp encodes IR function names as backend symbols
src/llvm_codegen.cpp  emits LLVM IR from IR
src/driver.cpp        CLI pipeline and file IO
```

## Current Backend Contract

The default backend emits LLVM IR, invokes an LLVM driver such as `clang`, and
links a normal glibc-backed executable or shared library on Linux. Relocatable
object output also goes through the LLVM driver.
