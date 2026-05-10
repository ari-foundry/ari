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
   library design lands. Unsupported local `Vec` method calls now get a
   dedicated diagnostic that points users at the future allocator-backed std
   collection APIs instead of falling through to the generic unknown-method
   error. Vec storage helper logic is split out of `sema.cpp` into
   `vector_semantics` so the allocator-backed work can grow outside the main
   semantic checker. Introduce the explicit allocation/capability path before
   broadening vector patterns or std collection APIs.
   - [allocator] thread explicit allocator/capability values through creation
   - [capacity] replace local literal/reserve capacity with runtime heap
     capacity growth
   - [ops-runtime] port the existing `push`, `insert`, `pop`, `remove`,
     `first`, `last`, `get`, `swap`, `contains`, `index_of`, `count`, and
     `reserve` operations to allocator-backed storage instead of fixed
     local-capacity traps
2. Reuse AST/IR summary package caches for file-backed modules.
   The source-snapshot cache goal is complete: compact module metadata and
   source-snapshot module caches can be emitted, checked, and invalidated with
   cfg/search-path/source/import/item-specific stale diagnostics. Source records
   carry stable content hashes, cache validation catches body changes even when
   declaration summaries stay the same, import resolution is rechecked against
   the current package layout, old metadata summaries without source hashes are
   rejected, malformed metadata with duplicate source/import/item records is
   rejected, and malformed caches with duplicate source or AST-summary records
   are rejected before validation. Module caches now carry v5 per-source AST
   summary records with declaration fingerprints and declaration-summary
   payloads, parse those payloads during cache loading, and validate their
   internal hashes, top-level counts, scalar and aggregate constant
   initializers, and declaration materialization round-trips before rechecking
   them against the parsed cached source snapshot. Header-like cached
   dependencies whose summaries contain only declaration-safe items, including
   constants, can now feed materialized declarations directly into the loader
   without reparsing the cached source snapshot. The remaining package-cache
   work is to extend that path to future IR summaries for dependencies with
   executable bodies, then skip dependency source parsing whenever validation
   succeeds.
   - [ir-materialize] feed future IR-summary declarations/bodies into the
     module loader for dependencies with executable function or impl bodies
   - [cache-skip] avoid reparsing dependencies when the metadata summary and
     source hashes still match the current source graph and cfg/search-path
     inputs
See also [Semantic Checker Decomposition](sema-decomposition.md) for the
maintenance roadmap for splitting `src/sema.cpp` into smaller subsystems.

## Medium-Term Language Work

1. Extend associated items on traits.
   The near-term trait-composition surface is complete for method lookup:
   `trait Child: Parent` parses and checks, supertrait impls are required,
   generic child-trait bounds can statically dispatch parent methods,
   trait-qualified method calls disambiguate static dispatch, and LLVM
   `dyn Child` values include object-safe supertrait methods with dyn-to-dyn
   upcasts to inherited supertraits. Keep Ari on trait composition instead of
   struct inheritance: structs remain explicit data layouts with field
   embedding rather than hidden base-object layout.
   - [trait-qualified-associated] design an explicit syntax for selecting a
     trait impl's associated function when multiple traits provide the same
     associated function for one receiver type; avoid copying Rust's full
     `<T as Trait>::item` spelling unless Ari really needs that shape
   - [associated-types] add associated type declarations, impl witnesses, and
     projection syntax for generic APIs such as iterator item types
   - [generic-supertrait-inference] handle richer generic supertrait
     applications once associated types and projections exist
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
3. Extend pattern binding modes beyond value bindings.
   - [reference] design `ref`, `ref mut`, `&`, and Ari ownership-aware binding modes
   - [ownership] preserve binding modes through aggregate, enum, slice, and vector patterns once ownership-through-aggregates lands
   Tuple, fixed-array, named-struct, and tuple-struct match arms now share
   same-name/same-type or-pattern bindings through the product pattern engine.
   Refutable aggregate `let`/`var` declarations now reuse that engine for
   tuple, fixed-array, named-struct, and tuple-struct literal/range/alias/or
   tests, and preserve `var` mutability for every introduced binding.
   Aggregate `if let` statement/expression and aggregate `while let`
   statement bindings also share the same product match lowering path.
   - [slice-patterns] lower `Slice[T]` and `Vec[T]` length-checked patterns
     after the shared binding-mode engine decides reference/ownership behavior
   - [macro-pattern] allow pattern-position macro expansion after the macro system is real
   - [positions] keep `let`/`var`, match, control-flow, for-loop, and
     function-parameter patterns on one shared binding-mode engine; value alias
     patterns now work in list-literal and stored-vector loop heads when the
     wrapped pattern is irrefutable, but reference/ownership binding modes
     still need shared lowering. Future `for let` filters over vector and slice
     values should reuse the same product-pattern binding path once vector/slice
     patterns have length-checked lowering.
4. Implement user-defined compile-time meta expansion for `meta fn`.
   The built-in `matches!` macro lowers through the pattern engine today.
   Meta functions are intentionally non-generic; define concrete meta entry
   points over `token_stream`, `ast`, or `type` instead of instantiating them.
   - [tokens] support `token_stream` input/output rewrites
   - [ast] support `ast` input/output rewrites
   - [calls] expand user-defined Rust-style `name!(...)` expression calls
   - [items] expand item-position macro invocations
   - [attributes] allow attribute macros to rewrite or insert AST nodes
   - [derive] expand built-in derives such as `Debug` where the trait surface exists
   - [format] lower `format!` after owned runtime strings exist
5. Expand aggregate enum payload storage beyond the nested-enum MVP.
   Aggregate enum payload slots support integer, bool, pointer-shaped values
   such as `string`, `ptr T`, and `fn(...) -> ...`, one-word enum values, and
   LLVM homogeneous nested aggregate-enum values today. Nested enum-case
   subpatterns can inspect one-word enum payload slots on the LLVM and
   freestanding local-value paths, and can inspect homogeneous nested
   aggregate-enum payload slots on the LLVM and freestanding paths. The
   freestanding backend can store/copy local and pointer-backed aggregate enum
   values, including homogeneous nested aggregate-enum payload values and
   direct enum-constructor stores through raw pointers. The stored payload
   universe is still intentionally narrow.
   - [mixed-slots] define an explicit ABI rule for payload slots that mix
     nested aggregate enums with scalar or pointer-shaped values
   - [aggregate-values] allow tuple, struct, vector, and owned payload values
     after their non-local ABI/storage rules are defined
   - [payload-pointers] lower direct payload field pointer access for
     aggregate enum payloads after payload ABI and aliasing rules are explicit
   - [repr-c-payloads] define C tagged-union layout and C header emission for
     payload-bearing `@repr(C)` enums after aggregate enum payload ABI is stable
6. Lower remaining allocation-backed prelude ADTs. Integer `Range[T]` and
    `RangeInclusive[T]` local values are implemented today. `Option[T]` and
    `Result[T, E]` are source `std` generic enums exposed through the implicit
    prelude and connected to
    `?`/`??` on the LLVM aggregate-enum path. `Slice[T]` is a source `std`
    view struct with `data: ptr T` and `len: i64`; `slice(data, len)` builds
    that view from a raw pointer and length, while `len(view)`, `view.len()`,
    `view.is_empty()`, `view[index]`, and `view[index] = value` use the stored
    view metadata with runtime bounds checks. It is non-owning and still relies
    on explicit raw-pointer discipline.
    Nullable `T?` remains a raw-pointer spelling for `ptr T`; non-pointer
    absence stays on the explicit `Option[T]` ADT path.
    - [owned] `Box[T]`
    - [strings] add allocator-backed owned runtime strings so APIs such as
      `read_line` can return independent buffers instead of the current host
      reusable line buffer
    Slice pattern follow-ups live with the shared pattern binding-mode work
    because they depend on reference/ownership binding policy, not allocator
    ownership.
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
9. Extend trait-object dispatch beyond the concrete/generic-impl copyable LLVM
    subset.
    Explicit `dyn Trait[...]` object types, explicit `value as dyn Trait[...]`
    conversions, per-impl vtables, erased receiver thunks, and vtable-slot
    method calls are implemented for concrete and generic-impl-specialized,
    copyable, non-borrow source values on the LLVM backend. Dyn-to-dyn upcasts
    to the same trait or an inherited supertrait are implemented on LLVM by
    preserving the data pointer and offsetting the vtable pointer; unrelated
    dyn casts remain rejected.
    - [ownership] define dyn object data-pointer ownership for `own` and
      borrow-valued source types
    - [freestanding] lower trait-object values and dispatch in the raw backend

## Backend Work

1. Emit relocatable object files for the native backend.
2. Define non-local aggregate ABI layouts for external ABI surfaces and
   vectors. Fixed-size local tuple/array/vector stack layout,
   direct raw aggregate parameters/returns through hidden pointers, LLVM
   aggregate enum layout, tuple/array/vector index access, raw aggregate
   expression results, and raw `@export`/`@no_mangle` ELF symbols are
   implemented.
   - [structs] finish sharing all field-layout decisions between sema and
     backends
   - [arrays] pass fixed-size arrays across FFI boundaries
   - [raw-c-imports] define a real C ABI/link path for imported `extern "C"`
     symbols on raw/freestanding targets; direct C extern calls remain rejected
     there until this exists
   - [vectors] define non-local and growable vector ABI after allocator-backed `Vec[T]` storage exists
   - [enums] finish direct value materialization for multi-payload aggregate
     enums in the freestanding backend and define their external FFI ABI
3. Add freestanding runtime string storage so raw ELF output can lower string
   values and line input helpers without relying on the host C runtime.
4. Lower floating-point scalar values and calls in the freestanding backend.
   Raw local `f32`/`f64` literals and assignments can now be materialized as
   IEEE bit-pattern scalar storage, and raw pointer load/store/dereference of
   `ptr f32` and `ptr f64` uses the same scalar bit storage path. Freestanding
   `f32`/`f64` arithmetic and ordered comparisons now lower with SSE scalar
   instructions, and direct Ari calls can pass or return `f32`/`f64` as raw
   scalar ABI bit patterns. This intentionally does not pretend to be complete
   native float lowering yet: `f128` still fails with dedicated freestanding
   diagnostics, and foreign/platform C float ABI work remains tied to the raw
   C-import path.
   - [values] materialize eventual `f128` scalar expressions
   - [abi-c] define foreign/platform C float ABI once raw C extern calls exist
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
