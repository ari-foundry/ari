# Compiler Pipeline

## Driver

`src/driver.cpp` implements:

```text
ari <input.ari> [--check] [-o output.elf]
```

Pipeline:

1. read the source file
2. lex source into tokens
3. parse tokens into AST
4. check AST and lower into IR
5. with `--check`, stop after diagnostics without backend output
6. emit LLVM IR or freestanding x86-64 code bytes
7. invoke the LLVM driver or wrap bytes in ELF64
8. write the output file

## Lexer

Files:

```text
src/lexer.cpp
src/lexer.hpp
src/token.hpp
```

The lexer handles:

- identifiers and keywords
- integer literals
- float literals
- string literals
- line comments
- nested block comments
- multi-character tokens such as `::`, `=>`, `&&`, `||`, `<<`, `>>`

When adding syntax, update `token.hpp`, the keyword table in `lexer.cpp`, and
parser expectations together.

## Parser

Files:

```text
src/parser.cpp
src/parser.hpp
src/ast.hpp
```

The parser builds source-shaped AST nodes. It intentionally keeps some features
that are not executable yet, such as generic structs and meta functions.

Parser errors should describe the expected syntax shape. Semantic errors should
describe language rule violations.

## Semantic Checking

Files:

```text
src/sema.cpp
src/sema.hpp
src/types.hpp
src/ir.hpp
```

The checker turns source-shaped AST into typed IR. This is where feature
support becomes concrete.

Common semantic tasks:

- resolve unqualified and qualified names
- resolve imported aliases from `use`
- check `pub` before cross-module access
- infer local binding types
- reject unsupported type combinations
- check ownership and borrowing
- lower only backend-supported expressions to IR

## Code Generation

Files:

```text
src/llvm_codegen.cpp
src/llvm_codegen.hpp
src/codegen.cpp
src/codegen.hpp
```

The default host codegen emits LLVM IR and relies on an LLVM driver such as
`clang` for glibc linking, FFI, and shared-library output. `--emit-llvm` writes
`.ll` and stops. The freestanding codegen emits x86-64 machine code directly.
`--freestanding --emit-obj` wraps that raw code stream as an ELF64 relocatable
object instead of an executable. No backend should need to know about source
parser details.

If an IR kind is front-end only, codegen should reject it with a clear message
until runtime lowering exists.

## ELF Writing

Files:

```text
src/elf.cpp
src/elf.hpp
```

The ELF writer creates minimal Linux x86-64 ET_EXEC files with executable load
segments and ET_REL object files with `.text`, optional `.rela.text`,
`.symtab`, `.strtab`, and `.shstrtab` sections.

The driver writes freestanding ELF outputs with normal executable permissions,
so no manual `chmod +x` step is needed. Relocatable object outputs are ordinary
non-executable binary files. Supported raw imported `extern "C"`
scalar/raw-pointer calls lower to undefined symbols plus x86-64 call
relocations in object output; raw executable output still has no linker phase.
