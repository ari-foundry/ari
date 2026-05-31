# Compiler Pipeline

This page describes the current hosted pipeline. See
[Compiler Pass Contracts](compiler-pass-contracts.md) for the long-term
input/output boundary rules between lexer, parser, resolver, sema, IR, and
backend codegen.

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
6. emit LLVM IR
7. invoke the LLVM driver when an executable, shared library, or object is requested
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
```

The default host codegen emits LLVM IR and relies on an LLVM driver such as
`clang` for glibc linking, FFI, and shared-library output. `--emit-llvm` writes
`.ll` and stops. `--emit-obj` asks the LLVM driver to compile the generated IR
into a relocatable object. No backend should need to know about source parser
details.

If an IR kind is front-end only, codegen should reject it with a clear message
until runtime lowering exists.

## Host Platform Boundary

Files:

```text
src/platform.hpp
src/platform.cpp
```

The hosted compiler uses `ari::platform` for host-specific filesystem and
process boundaries: path joining and parent directory extraction, `PATH`
splitting, executable lookup, current executable and working directory
discovery, regular-file and executable checks, executable permission updates,
shell quoting, and shell command execution.

Keep new host conditionals inside `src/platform.cpp` when possible. Driver,
module loading, module cache validation, and toolchain code should call the
platform API instead of adding direct `_WIN32`, `unistd`, `chmod`, `readlink`,
or PATH-separator logic. This boundary only describes the compiler host; it
does not mean Windows target or native-host support is complete.

## Output Writing

The driver writes text IR directly for `--emit-llvm`. For executables, shared
libraries, and object files it writes temporary LLVM IR, invokes the selected
LLVM driver, and leaves the final artifact at the requested output path.
