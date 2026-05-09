# Architecture

## Goal

Ari is a C++17 compiler prototype for a future systems language with:

- value-oriented data
- explicit ownership and borrowing
- no classes
- traits for behavior
- ADT enums and pattern matching
- modules and visibility
- local type inference
- a compiler-known prelude

The implementation is intentionally direct so language behavior stays easy to
inspect.

## Layers

```text
source text
  -> lexer
  -> parser
  -> AST
  -> semantic checker
  -> typed IR
  -> LLVM IR codegen or freestanding x86-64 codegen
  -> LLVM driver or ELF writer
  -> Linux executable or shared library
```

## Front End

The front end accepts more syntax than the backend can lower. This is
intentional.

The parser already gives future features stable AST shapes for structs,
generics, traits, impls, attributes, meta functions, modules, ownership
qualifiers, tuples, vectors, floats, and strings.

The semantic checker decides what is executable today.

## Semantic Checker

`src/sema.cpp` owns most language policy:

- symbol collection
- module path resolution
- `pub` visibility checks
- `use` alias handling
- local type inference
- primitive type checking
- integer literal range checking
- ownership state checking
- temporary borrow checking
- enum layout validation
- match exhaustiveness checking
- prelude builtin injection
- formatting placeholder validation
- AST-to-IR lowering

When adding a language feature, prefer making the checker reject unsupported
runtime cases clearly instead of letting codegen fail with a vague error.

## IR Boundary

`src/ir.hpp` is the contract between language checking and target-specific
lowering.

Good IR additions should carry enough type information for codegen to avoid
re-resolving source-level names. For example, enum constructor expressions carry
case metadata after semantic resolution.

## Backends

`src/llvm_codegen.cpp` is the default glibc-backed backend. It emits LLVM IR,
declares FFI symbols, initializes Ari runtime context, and lets an LLVM driver
such as `clang` link against glibc and user libraries. `--emit-llvm` writes the
IR and stops.

`src/codegen.cpp` is the freestanding backend. It emits raw x86-64 machine code
into a flat code buffer. The ELF writer then wraps that buffer as a minimal
direct-syscall executable and can attach a `.symtab` containing Ari mangled
function symbols.

The freestanding backend currently assumes scalar stack slots are 64 bits.
Source-level integer width is preserved in IR so later lowering can use exact
layouts.

## Runtime

The host runtime is compiler-emitted LLVM IR:

- initialize `AriContext` with `argc`, `argv`, and a thread-id slot
- enter `@ari_entry`, then call the `@"ari::main"` bridge for source `main`
- shut down the thread-local context
- prelude IO through C stdio

The freestanding runtime still uses a raw ELF entry point, calls Ari `main`, and
exits through Linux syscalls.
