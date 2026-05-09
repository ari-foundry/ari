# Roadmap

## Near-Term Compiler Work

No active near-term compiler work is queued after the concrete trait-object
dispatch slice. Promote the next small Medium-Term item here when it becomes
the next implementation target.

## Medium-Term Language Work

1. Expand product-pattern analysis beyond the finite coverage engine.
   - [symbolic-products] represent high-cardinality integer product coverage without enumerating every value
   - [diagnostics] suggest the smallest missing product shape when a product match is non-exhaustive
   - [enum-payloads] support literal, range, alias, or-pattern, and nested enum-case subpatterns inside aggregate enum payload slots
2. Refine borrow checking beyond lexical named borrows.
   - [nll] shorten named borrows to their last use when control-flow analysis can prove it
   - [reborrow] allow safe reborrowing from existing borrow bindings
   - [borrow-results] allow borrow-valued function returns, `if`/`match`/block
     expression results, and labeled-block break values when lifetimes are valid
   - [aggregate-borrows] track borrow-valued aggregate fields independently so
     assigning unrelated fields or whole aggregate bindings can be checked
   - [loop-state] track ownership and borrow state through loops, init-while
     updates, owning loop bindings, and owning break values instead of rejecting
     all state changes inside loops
3. Move compiler-known prelude traits/functions toward source-level standard
   library modules.
4. Extend pattern binding modes beyond value bindings.
   - [reference] design `ref`, `ref mut`, `&`, and Ari ownership-aware binding modes
   - [ownership] preserve binding modes through aggregate, enum, slice, and vector patterns once ownership-through-aggregates lands
   - [refutable-let] lower refutable enum-case `let`/`var` patterns after the
     failure behavior is designed
   - [or-bindings] support binding unification for or-patterns in all aggregate
     pattern positions
   - [alias-or] support alias patterns wrapped around or-patterns
   - [macro-pattern] allow pattern-position macro expansion after the macro system is real
   - [positions] keep `let`/`var`, match, control-flow, for-loop, and function-parameter patterns on one shared binding-mode engine
5. Implement user-defined compile-time meta expansion for `meta fn`.
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
6. Expand the FFI surface.
   - [repr] finish `repr(C)` aggregate ABI layout, including generic
     aggregates and the policy for ownership/borrow-qualified fields
   - [pointers] finish `repr(C)`-aware aggregate pointer layout; nullable raw-pointer literals, pointer casts, byte-wise pointer offsets, typed scalar/Ari-layout aggregate offsets, scalar/plain-Ari-aggregate load/store helpers, scalar/plain-Ari-aggregate `*pointer` dereference syntax, Ari-layout scalar aggregate field/element pointer access, and `size_of<T>()` / `align_of<T>()` layout queries are implemented
   - [varargs] apply C default argument promotions for non-literal variadic
     arguments and define whether variadic functions can be used as function
     pointer values
   - [generic-extern] keep extern declarations non-generic by design or define
     explicit wrapper monomorphization for generic foreign declarations
   - [abi] represent non-C ABI shims explicitly
7. Lower remaining allocation-backed prelude ADTs. Integer `Range[T]` and
    `RangeInclusive[T]` local values are implemented today.
    - [sum] `Option[T]`, `Maybe[T]`, and `Result[T, E]`, connected to the existing `?`/`??` propagation model
    - [owned] `Box[T]`
    - [strings] add allocator-backed owned runtime strings so APIs such as
      `read_line` can return independent buffers instead of the current host
      reusable line buffer
    - [views] `Slice[T]`, including slice patterns after slice layout and borrowing are defined
8. Add allocator-backed growable `Vec[T]`. Non-empty `[...]` now defaults to
    fixed array literals unless a `Vec[T]` expected type is present. Local
    stack-backed vector literal storage, checked indexing, literal reassignment
    with changing runtime length, typed empty local vectors, and
    `len(value)` / `value.len()` length queries for arrays and vectors are
    implemented on the LLVM backend today. The current local vector storage
    grows to the largest literal capacity seen in the binding, while the stored
    length still shrinks and expands per assignment.
    - [allocator] thread explicit allocator/capability values through creation
    - [ops] lower explicit push, reserve, and iteration primitives for allocator-backed vectors
    - [capacity] replace the current compile-time local max-literal capacity with runtime heap capacity growth
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
11. Add package metadata for file-backed modules.
    - [metadata] write and read compiled package/module summaries
    - [cache] use package metadata to avoid repeatedly reparsing stable module
      dependencies
12. Extend trait-object dispatch beyond the concrete copyable LLVM subset.
    Explicit `dyn Trait[...]` object types, explicit `value as dyn Trait[...]`
    conversions, per-impl vtables, erased receiver thunks, and vtable-slot
    method calls are implemented for concrete, copyable, non-borrow source
    values on the LLVM backend.
    - [generic-impls] build vtables from generic trait impl specializations
    - [generic-methods] decide whether generic trait methods are object-safe or
      must remain statically dispatched
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
