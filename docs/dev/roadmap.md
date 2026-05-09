# Roadmap

## Near-Term Compiler Work

1. Start allocator-backed growable `Vec[T]`.
   Local vector literal storage and local `Vec.reserve(n)`/`Vec.push(value)` /
   `Vec.pop()` / `Vec.first()` / `Vec.last()` / `Vec.capacity()` /
   `Vec.is_empty()` / `Vec.clear()` / `Vec.truncate(n)` /
   `Vec.get(index)` / `Vec.set(index, value)` / `Vec.swap(a, b)` /
   `Vec.remove(index)` / `Vec.insert(index, value)` /
   `Vec.contains(value)` / `Vec.index_of(value)` / `Vec.count(value)` lower
   today on the LLVM backend as stack-backed values with compile-time capacity,
   runtime length checks, and linear search over copyable comparable elements.
   Local `push` and `insert` now auto-widen stack storage when sema can track
   the current length, so empty `Vec[T]` locals can grow through
   straight-line appends/inserts without an explicit `reserve`.
   This local API is now frozen as a temporary executable subset: do not add
   more compiler-known `Vec` convenience methods before the allocator-backed
   library design lands. Vec storage helper logic is split out of `sema.cpp`
   into `vector_semantics` so the allocator-backed work can grow outside the
   main semantic checker. Introduce the explicit allocation/capability path
   before broadening vector patterns or std collection APIs.
   - [allocator] thread explicit allocator/capability values through creation
   - [capacity] replace local literal/reserve capacity with runtime heap
     capacity growth
   - [api-freeze] keep the current compiler-known local methods only as
     compatibility coverage; future Vec methods belong in the std collection
     library after generic impls and allocator capabilities are real
   - [ops-runtime] port the existing `push`, `insert`, `pop`, `remove`,
     `first`, `last`, `get`, `swap`, `contains`, `index_of`, `count`, and
     `reserve` operations to allocator-backed storage instead of fixed
     local-capacity traps
2. Add AST/IR summary package caches for file-backed modules.
   The source-snapshot cache goal is complete: compact module metadata and
   source-snapshot module caches can be emitted, checked, and invalidated with
   cfg/search-path/source/import/item-specific stale diagnostics. Source records
   carry stable content hashes, cache validation catches body changes even when
   declaration summaries stay the same, import resolution is rechecked against
   the current package layout, old metadata summaries without source hashes are
   rejected, malformed metadata with duplicate source/import/item records is
   rejected, and malformed caches with duplicate source records are rejected
   before validation. The remaining package-cache work is to replace dependency
   source parsing with a cached AST or IR summary once validation succeeds.
   - [ast-summary] define a cached AST or IR summary format that can be loaded
     after metadata validation succeeds
   - [cache-skip] avoid reparsing dependencies when the metadata summary and
     source hashes still match the current source graph and cfg/search-path
     inputs
See also [Semantic Checker Decomposition](sema-decomposition.md) for the
maintenance roadmap for splitting `src/sema.cpp` into smaller subsystems.

## Medium-Term Language Work

1. Refine borrow checking beyond lexical named borrows.
   - [nll] shorten named borrows to their last use when control-flow analysis can prove it
   - [reborrow] allow safe reborrowing from existing borrow bindings
   - [borrow-results] allow borrow-valued function returns, `if`/`match`/block
     expression results, and labeled-block break values when lifetimes are valid
   - [aggregate-borrows] track borrow-valued aggregate fields independently so
     assigning unrelated fields or whole aggregate bindings can be checked
   - [loop-state] track ownership and borrow state through loops, init-while
     updates, owning loop bindings, and owning break values instead of rejecting
     all state changes inside loops
2. Extend pattern binding modes beyond value bindings.
   - [reference] design `ref`, `ref mut`, `&`, and Ari ownership-aware binding modes
   - [ownership] preserve binding modes through aggregate, enum, slice, and vector patterns once ownership-through-aggregates lands
   - [or-bindings] support binding unification for or-patterns in all aggregate
     pattern positions
   - [macro-pattern] allow pattern-position macro expansion after the macro system is real
   - [positions] keep `let`/`var`, match, control-flow, for-loop, and function-parameter patterns on one shared binding-mode engine
3. Implement user-defined compile-time meta expansion for `meta fn`.
   The built-in `matches!` macro lowers through the pattern engine today.
   - [tokens] support `token_stream` input/output rewrites
   - [ast] support `ast` input/output rewrites
   - [calls] expand user-defined Rust-style `name!(...)` expression calls
   - [items] expand item-position macro invocations
   - [attributes] allow attribute macros to rewrite or insert AST nodes
   - [generics] decide whether `meta fn` can be generic and how generic meta
     signatures are represented
   - [derive] expand built-in derives such as `Debug` where the trait surface exists
   - [format] lower `format!` after owned runtime strings exist
4. Expand the FFI surface.
   - [repr] finish `repr(C)` aggregate ABI layout, including generic
     aggregates and the policy for ownership/borrow-qualified fields
   - [pointers] finish `repr(C)`-aware aggregate pointer layout; nullable raw-pointer literals, nullable `T?` raw-pointer type suffixes, pointer casts, byte-wise pointer offsets, typed scalar/Ari-layout aggregate offsets, scalar/plain-Ari-aggregate load/store helpers, scalar/plain-Ari-aggregate `*pointer` dereference syntax, Ari-layout scalar aggregate field/element pointer access, and `size_of<T>()` / `align_of<T>()` layout queries are implemented
   - [abi] represent non-C ABI shims explicitly
5. Expand aggregate enum payload storage.
   Aggregate enum payload slots support integer, bool, and one-word enum values
   today. Nested enum-case subpatterns can inspect one-word enum payload slots
   on the LLVM backend, but the stored payload universe is still intentionally
   narrow.
   - [nested-aggregate-enums] allow aggregate enum payload slots to store
     nested aggregate-enum values once the ABI and copy rules are explicit
   - [aggregate-values] allow string, tuple, struct, vector, and owned payload
     values after their non-local ABI/storage rules are defined
   - [freestanding] lower aggregate enum payload storage and tests in the raw backend
6. Lower remaining allocation-backed prelude ADTs. Integer `Range[T]` and
    `RangeInclusive[T]` local values are implemented today.
    - [sum] `Option[T]`, `Maybe[T]`, and `Result[T, E]`, connected to the existing `?`/`??` propagation model
    - [nullable-values] decide whether value-level `T?` should remain raw-pointer-only syntax or grow an `Option[T]`/`Maybe[T]` lowering for non-pointer values
    - [owned] `Box[T]`
    - [strings] add allocator-backed owned runtime strings so APIs such as
      `read_line` can return independent buffers instead of the current host
      reusable line buffer
    - [views] `Slice[T]`, including slice patterns after slice layout and borrowing are defined
7. Design `std` smart-pointer and explicit move surfaces.
    Ari's core memory model is zone/capability-oriented rather than strictly
    borrow-safe, but the standard library still needs clear ownership helpers
    for common heap and shared-resource patterns.
    - [unique] define `Unique[T]` / `Box[T]` as unique heap owners with explicit
      zone or allocator capability construction
    - [shared] define `Shared[T]` and `Weak[T]` reference-counted handles,
      including whether counts are atomic or single-threaded by default
    - [move] add explicit `move(value)` / `take(place)` style helpers for APIs
      that should visibly consume bindings instead of relying on read syntax
    - [clone-drop] define `Clone`/`Drop` interaction for smart pointers,
      ref-count increments, and deterministic release
    - [interop] decide how smart pointers expose raw pointers for FFI without
      pretending Ari has a globally safe borrow model
8. Extend allocator-backed growable `Vec[T]` after the MVP. Non-empty `[...]` now defaults to
    fixed array literals unless a `Vec[T]` expected type is present. Local
    stack-backed vector literal storage, checked indexing, literal reassignment
    with changing runtime length, typed empty local vectors, and
    `len(value)` / `value.len()` length queries for arrays and vectors are
    implemented on the LLVM backend today. Fixed-capacity local
    `reserve(n)`/`push(value)` also lowers on LLVM for copyable element
    vectors. The current local vector storage grows to the largest literal or
    reserve capacity seen in the binding, while the stored length still shrinks
    and expands per assignment. The allocator, runtime capacity, and real
    grow-on-push behavior remain near-term.
    - [std-api] define ergonomic std collection methods only after generic
      impls, allocator capabilities, and runtime growth are in place, so the
      temporary compiler-known local API does not become permanent surface area
    - [iteration] lower iterator primitives for allocator-backed vectors
    - [patterns] connect fixed-length and rest vector patterns such as `[head, tail @ ..]` to stored vectors after runtime layout exists
    - [freestanding] lower stored local vector values in the raw backend
9. Lower general `Iterator[T]`-based `for` loops. Range loops, list literal
    loops, and stored local vector loops lower today without trait dispatch; the
    general iterator model needs trait-bound resolution plus generic
    `Option[T]`/`Maybe[T]` result lowering for `next`.
    - [trait] resolve `IntoIterator[T]`/`Iterator[T]`
    - [loop] lower `next`-style iteration state
    - [pattern] bind refutable enum-case loop-head patterns after the iterator failure/skip semantics are designed
10. Track move-only aggregate elements more precisely.
    - [fields] move owned fields out of local aggregate values without moving
      unrelated fields
    - [temporary-fields] define whether owned fields can be moved out of
      temporary aggregate values
    - [dynamic-indexes] support or deliberately reject moving owning aggregate
      elements through dynamic vector/array indexes with clear semantics
11. Extend trait-object dispatch beyond the concrete/generic-impl copyable LLVM
    subset.
    Explicit `dyn Trait[...]` object types, explicit `value as dyn Trait[...]`
    conversions, per-impl vtables, erased receiver thunks, and vtable-slot
    method calls are implemented for concrete and generic-impl-specialized,
    copyable, non-borrow source values on the LLVM backend.
    - [ownership] define dyn object data-pointer ownership for `own` and
      borrow-valued source types
    - [upcasts] support or reject `dyn SubTrait as dyn SuperTrait`
    - [freestanding] lower trait-object values and dispatch in the raw backend

## Backend Work

1. Refine shared-library export policy. Path-only v0 Ari symbol mangling is
   implemented, and source declarations can opt into explicit C symbols with
   `@export`, `@export("symbol")`, or `@no_mangle`.
   - [visibility] select only exported/public symbols for dynamic export once object visibility is controlled
2. Add exact integer-width stack and ABI layout in `--freestanding`.
3. Emit relocatable object files for the native backend.
4. Define non-local aggregate ABI layouts for tuple parameters/returns,
   structs, fixed arrays, and vectors. Fixed-size local tuple/array/vector
   stack layout, LLVM aggregate enum layout, and tuple/array/vector index access
   are implemented on the LLVM backend.
   - [tuples] pass and return tuple values across function boundaries
   - [structs] share field layout between sema and backends
   - [arrays] pass and return fixed-size arrays across function and FFI boundaries
   - [expr-results] materialize aggregate-valued `if`, `match`, and block
     expressions in the freestanding backend
   - [vectors] define non-local and growable vector ABI after allocator-backed `Vec[T]` storage exists
   - [enums] lower multi-payload aggregate enum layout in the freestanding backend and define its FFI ABI
5. Add freestanding runtime string storage so raw ELF output can lower string
   values and line input helpers without relying on the host C runtime.
6. Lower floating-point scalar values and calls in the freestanding backend.
   - [values] materialize `f32`, `f64`, and eventual `f128` scalar expressions
   - [abi] pass and return supported floats with the platform calling convention
   - [ops] lower arithmetic, comparisons, casts, and raw pointer load/store for
     float pointee types
7. Expand FFI type coverage beyond the x86-64 Linux C aliases.
8. Decide whether source-level function overloading belongs in Ari. The current
   v0 mangling intentionally omits parameter types because overloading is not
   supported.

## Bootstrap Direction

1. Keep C++ implementation compact while the language design stabilizes.
2. Reimplement isolated front-end pieces in Ari once structs, strings, and
   vectors lower.
3. Reimplement parser and semantic passes in Ari.
4. Compile the Ari compiler with Ari.
5. Compare outputs from the current compiler and self-hosted compiler.

## Non-Goals For The Current Milestone

- class syntax
- hidden inheritance
- garbage collection
- C++ ABI dependency as a source-level FFI surface
- ambient global heap as a language primitive
- mixing host/glibc codegen assumptions into the raw freestanding backend
