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
  -> LLVM IR codegen
  -> LLVM driver
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

Executable Ari aggregate layout is centralized in `src/layout.cpp`. Sema,
shared IR builders, and the LLVM backend use that service for aggregate layout
predicates, field lists, field counts, sizes, alignments, and byte offsets.
Enum payload storage selection is still handled by `enum_payload_layout`; once
those payload slot field types are present, byte layout comes from the shared
layout service.

## Backends

`src/llvm_codegen.cpp` is the glibc-backed backend. It emits LLVM IR, declares
FFI symbols, initializes Ari runtime context, and lets an LLVM driver such as
`clang` link against glibc and user libraries. `--emit-llvm` writes the IR and
stops; `--emit-obj` asks the LLVM driver to produce a relocatable object.

## Runtime

The host runtime is compiler-emitted LLVM IR:

- initialize `AriContext` with `argc`, `argv`, and main thread id `0`
- enter `@ari_entry`, then call the `@"ari::main"` bridge for source `main`
- shut down the thread-local context
- prelude IO through C stdio
