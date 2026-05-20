# Ari Developer Docs

This folder is for people changing the compiler.

## Main Pages

- [Architecture](architecture.md)
- [Compiler Pipeline](compiler-pipeline.md)
- [Build And Test](build-test.md)
- [Aggregate ABI Classification](aggregate-abi.md)
- [Feature Test Matrix](test-matrix.md)
- [Runtime And Backend](runtime-backend.md)
- [Runtime Support Roadmap](runtime-support.md)
- [Symbol Mangling](symbol-mangling.md)
- [Standard Library Roadmap](standard-library-roadmap.md)
- [Compiler Development Roadmap](compiler-development-roadmap.md)
- [Compiler Maturity Gates](compiler-maturity-gates.md)
- [Compiler Source And Diagnostics](compiler-source-diagnostics.md)
- [Compiler Artifact Testing](compiler-artifact-testing.md)
- [Production Compiler Design](production-compiler-design.md)
- [Compiler Bootstrap Fixture Plan](bootstrap-fixture-plan.md)
- [Bootstrap Readiness](bootstrap-readiness.md)
- [Self-Host Roadmap](self-host-roadmap.md)
- [Library Testing](library-testing.md)
- [Standard Library Docs](../stdlib/README.md)
- [Roadmap](roadmap.md)
- [Completed Milestones](completed-milestones.md)
- [Semantic Checker Decomposition](sema-decomposition.md)

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
