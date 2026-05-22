# Ari Project Orientation

This file is the quick project map for coding agents.

## Project Snapshot

Ari is a C++17 compiler prototype for a small systems language. The default
backend emits LLVM IR and invokes an LLVM driver such as `clang`, so generated
Linux executables link glibc without a generated-C++ step. Object output also
uses the LLVM driver.

## Commands

```sh
make
make check
make debug
make sanitize
make check-sanitize
```

Run `make check` after compiler code changes. Prefer `make check-sanitize` for
parser, semantic checker, ownership, or codegen changes.

## Source Map

```text
src/lexer.cpp    tokenizer
src/parser.cpp   AST construction
src/ast.hpp      parsed syntax model
src/sema.cpp     names, modules, types, ownership, IR lowering
src/ir.hpp       typed IR model
src/llvm_codegen.cpp LLVM IR emitter
src/driver.cpp   CLI pipeline
```

## Current Language Surface

Implemented executable subset:

- functions and `main() -> i64`
- `let`, `var`, assignment, `return`
- `if`, `while`, `break`, `continue`, `init ... while ... next`
- `for value in range(start, end)`
- integer and bool scalar values
- `i8/i16/i32/i64`, `u8/u16/u32/u64`
- explicit integer casts with `as`
- arithmetic, modulo, bit operations, comparisons, `&&`, `||`, `!`
- ownership checks for `own`, `drop`, move state, and temporary borrows
- non-generic ADT enums as one-word tagged unions
- statement `match` over enums
- inline modules with `mod`, `pub`, `use`, and `A::B`
- compiler-known prelude IO plus formatting `print` and `println`
- `extern "C"` FFI declarations
- explicit external C link names with `= "symbol"`
- `--shared` library builds
- context initialization with argc/argv and thread id storage
- simple generic function-call monomorphization
- Ari mangled function symbols in LLVM IR and LLVM object output
- LLVM IR emission with `--emit-llvm`

Front-end or planned surface:

- structs and field layout
- generic aggregate/type monomorphization
- trait resolution and dispatch
- function parameter patterns
- tuple/vector runtime lowering
- runtime strings and floats
- raw pointer operations and explicit allocation-zone diagnostics
- file-backed modules
- general iterator protocol beyond `range`

## Design Rules

- Ari has no `class` keyword and no `interface` keyword. Use `trait`.
- Keep unsupported features rejected with clear diagnostics.
- Do not make codegen re-resolve source-level names if sema can lower metadata
  into IR.
- Keep all executable and object emission on the LLVM path.
- Keep allocation explicit and capability-oriented. Avoid a magical global heap.
- For zone-backed stdlib containers, entries, iterators, and helper views, do
  not store redundant `zone`, `allocator`, `arena`, or metadata handles when a
  stable zone allocation pointer already exists. Recover the zone from the
  allocation header via `zone::of(ptr)` or `ptr.zone()` at growth/allocation
  sites. Normal payload access must keep using the returned user pointer
  directly; only allocator/zone recovery code may inspect the fixed header at
  `ptr - 8`. Empty containers need a real fallback such as a zone-owned
  sentinel/backing allocation or another owning zone-backed pointer, not a
  per-entry/per-helper cached zone field.
- Preserve user changes in a dirty worktree; do not revert unrelated files.

## Docs Map

- `docs/language`: user-facing language documentation
- `docs/dev`: compiler implementation documentation
- `docs/dev/test-matrix.md`: feature-by-feature test plan
- `docs/notes`: design and handoff notes
- `docs/README.md`: complete docs index
