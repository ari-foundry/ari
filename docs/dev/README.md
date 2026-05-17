# Ari Developer Docs

This folder is for people changing the compiler.

## Main Pages

- [Architecture](architecture.md)
- [Compiler Pipeline](compiler-pipeline.md)
- [Build And Test](build-test.md)
- [Feature Test Matrix](test-matrix.md)
- [Runtime And Backend](runtime-backend.md)
- [Symbol Mangling](symbol-mangling.md)
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
