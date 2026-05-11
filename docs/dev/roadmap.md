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
   Runtime `reserve(n)` values now lower to a local-capacity guard, so the
   call no longer requires a literal. Integer constants, static integer
   arithmetic/bitwise/shift expressions over constants and literals, and
   immutable local integer bindings initialized from those static expressions
   are also folded into the local capacity decision. Static truncate lengths,
   including immutable locals initialized from those expressions, keep the
   compiler-known local length precise for later `push`/`insert` capacity
   decisions, and negative static truncate lengths now use the shared
   non-negative operand diagnostic. This still does not allocate or grow beyond
   the fixed local stack storage.
   This local API is now frozen as a temporary executable subset: do not add
   more compiler-known `Vec` convenience methods before the allocator-backed
   library design lands. Unsupported local `Vec` method calls now get a
   dedicated diagnostic that points users at the future allocator-backed std
   collection APIs instead of falling through to the generic unknown-method
   error. Vec storage helper logic is split out of `sema.cpp` into
   `vector_semantics`, including local storage type construction, typed empty
   vector literal construction, the frozen temporary local method list used by
   sema dispatch, local method shape diagnostics, and the shared `len`,
   `is_empty`, and `as_slice` builtin/method shape checks. Shared collection
   `len` lowering for arrays, local Vec storage, and Slice views is centralized
   there as well, along with `as_slice` data-pointer, Vec storage view, and
   Slice view construction. Local Vec integer and non-negative operand
   diagnostics for index/capacity/length arguments are also centralized there;
   static negative and known-out-of-range method indexes now use those shared
   diagnostics before lowering to runtime bounds checks. Known-empty
   `first`, `last`, `pop`, runtime-indexed `get`, `set`, `swap`, and
   `remove` calls, and direct `values[index]` expressions are rejected before
   lowering instead of deferring to guaranteed runtime empty-vector checks.
   Direct local `Vec` indexing also uses compiler-known current length for
   static out-of-range diagnostics after operations such as `truncate` and
   `clear`.
   Local Vec IR construction helpers include `first`, `last`, and `push`
   alongside the other method lowerings. Known-length updates for local `push`,
   `insert`, `pop`, `remove`, `clear`, and `truncate` now use small
   `vector_semantics` transition helpers, and local `Vec` `len`/`is_empty`
   calls plus `as_slice` view lengths and stored-vector `for` loop bounds fold
   to constants when the current length is compiler-known. `len(...)`,
   `.len()`, `.is_empty()`, and direct indexing of Vec-valued control-flow
   expressions also use source-known local branch lengths for constants and
   static out-of-range diagnostics, and stored-vector `for` loops use the same
   source-known branch lengths for constant loop bounds. Local `Vec`
   initialization and assignment from another local `Vec` also preserve that
   compiler-known current length when the source binding is still precise, and
   assignment from another local `Vec` widens the fixed local target storage to
   the source storage capacity before the copy. Vec-valued `if`, block, and
   labeled-block expressions now feed their nested fixed local storage capacity
   and same-length result paths into the same assignment/initialization
   tracking, including `break label value` paths and branch results that copy
   local Vec bindings whose current lengths are still compiler-known;
   Vec-valued `match` and `if let` expression arms do the same after sema sizes
   their expected result storage before branch result materialization, including
   arms whose result is a labeled block with typed Vec breaks.
   The raw freestanding backend now materializes fixed-capacity stored local
   `Vec[T]` values as Ari-layout local aggregates, so direct literals, local
   copies, checked scalar indexing, `len`/`is_empty`, read-only
   `capacity`/`first`/`last`/`get`, `contains`/`index_of`/`count`, and
   the fixed-capacity `reserve`/`push`/`insert`/`pop`/`remove`/`clear`/
   `truncate`/`set`/`swap` method surface work there, along with
   stored-vector `for` loops. Raw `Slice[T]` views over mutable local arrays,
   local vectors, and explicit `slice(data, len)` values also lower through
   their stored pointer/length metadata for `len`, `is_empty`, checked scalar
   indexing, indexed assignment, and exclusive/inclusive range slicing. These
   fixed-local methods still need an allocator-backed runtime storage path once
   `Vec[T]` stops being a fixed local buffer.
   The
   shared constant
   value model,
   constant-to-IR literal
   construction, scalar literal folding, constant binary result evaluation, and
   static integer folding for local Vec capacity and length decisions now live in
   `constant_semantics`;
   this keeps
   allocator-backed work from growing the main semantic checker. Introduce the
   explicit allocation/capability path before broadening vector patterns or std
   collection APIs.
   - [allocator] thread explicit allocator/capability values through creation
   - [capacity] replace local literal/const/static-expr/known-local/runtime-checked
     reserve capacity with runtime heap capacity growth
   - [ops-runtime] port the existing temporary fixed-local Vec API to
     allocator-backed storage instead of fixed local-capacity traps
2. Extend `BorrowContext` path provenance through reborrowed places.
   Direct local borrow bindings and borrow-valued control-flow results now
   preserve source/path/mode metadata. The next focused borrow slice is to carry
   that same metadata through places reached from an existing borrow binding,
   without weakening the current field-path conflict diagnostics.
   - [reborrow-paths] allow reborrowing fields/elements through borrow bindings
     after path provenance and backend lvalue lowering for those forms are
     explicit
See also [Semantic Checker Decomposition](sema-decomposition.md) for the
maintenance roadmap for splitting `src/sema.cpp` into smaller subsystems.

## Medium-Term Compiler Work

1. Promote IR package-cache summaries when future executable forms outgrow AST
   summaries.
   Source-snapshot module caches currently validate cfg/search-path/source
   hashes/imports and can materialize declaration-safe headers plus
   summary-safe executable dependency bodies from AST summaries without parsing
   the cached source snapshot. The current source-level AST expression,
   statement, and pattern surface is covered by that path. Add IR summaries
   later for future executable bodies that cannot round-trip through the compact
   AST summary format.
   - [ir-materialize] feed future IR-summary declarations/bodies into the
     module loader for dependencies whose executable function or impl bodies
     use forms outside the AST summary subset
   - [cache-skip] once IR summaries exist, avoid reparsing those dependencies
     when metadata and source hashes match the current source graph and
     cfg/search-path inputs

## Medium-Term Language Work

1. Extend associated items on traits.
   The near-term trait-composition surface is complete for method lookup:
   `trait Child: Parent` parses and checks, supertrait impls are required,
   generic child-trait bounds can statically dispatch parent methods,
   trait-qualified method calls disambiguate static dispatch, and LLVM
   `dyn Child` values include object-safe supertrait methods with dyn-to-dyn
   upcasts to inherited supertraits. Trait-qualified associated function calls
   such as `Trait::make<SelfType>(...)`, `Trait<T>::make<SelfType>(...)`, and
   expected-result forms like `let x: SelfType = Trait<T>::make(...)` now
   disambiguate same-named trait associated functions without adopting Rust's
   full `<T as Trait>::item` spelling, including expected-result propagation
   through `if`, `if let`, `match`, block, and labeled-block expression result
   arms, plus tuple, named-struct, tuple-struct, fixed-array, and `Vec` literal
   elements, plus generic enum constructor payloads such as `Some(...)` and
   `Ok(...)`. Keep Ari on trait composition instead of struct inheritance:
   structs remain explicit data layouts with field embedding rather than hidden
   base-object layout.
   - [associated-types] add associated type declarations, impl witnesses, and
     projection syntax for generic APIs such as iterator item types
   - [generic-supertrait-inference] handle richer generic supertrait
     applications once associated types and projections exist
2. Refine borrow checking beyond lexical named borrows.
   Direct local reborrows and borrow-valued block, `if`, `match`, and
   labeled-block expression results now use `BorrowContext` source tracking.
   The next refinements should preserve that source/path/mode model.
   - [nll] shorten named borrows to their last use when control-flow analysis can prove it
   - [borrow-returns] allow borrow-valued function returns when lifetimes are
     valid after the near-term expression-result shape checks land
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
     patterns now work in range, list-literal, and stored-vector loop heads when
     the wrapped pattern is irrefutable. Non-iterator loop-head validation now
     lives in `for_pattern_semantics`, with a shared sema lowering helper for
     range, list-literal, and stored-vector loop heads; reference/ownership
     binding modes still need shared lowering. Future `for let` filters over
     vector and slice values should reuse the same product-pattern binding path
     once vector/slice patterns have length-checked lowering.
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
    `?`/`??` on the LLVM aggregate-enum path. `Slice[T]` is now a source `std`
    view struct implemented on both LLVM and the raw freestanding backend for
    `slice(data, len)`, `len(view)`, `view.len()`, `view.is_empty()`,
    `view[index]`, `view[index] = value`, local array/Vec `as_slice()`, and
    `view[start..end]` / `view[start..=end]` range slicing. It is non-owning
    and still relies on explicit raw-pointer discipline.
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
    for common heap and shared-resource patterns. The explicit move surface
    itself is now available through prelude `move(value)` and `take(place)`;
    remaining work is the pointer family and its clone/drop behavior.
    - [unique] define `Unique[T]` / `Box[T]` as unique heap owners with explicit
      zone or allocator capability construction
    - [shared] define `Shared[T]` and `Weak[T]` reference-counted handles,
      including whether counts are atomic or single-threaded by default
    - [clone-drop] define `Clone`/`Drop` interaction for smart pointers,
      ref-count increments, and deterministic release
    - [interop] decide how smart pointers expose raw pointers for FFI without
      pretending Ari has a globally safe borrow model
8. Extend allocator-backed growable `Vec[T]` after the MVP. Non-empty `[...]` now defaults to
    fixed array literals unless a `Vec[T]` expected type is present. Local
    stack-backed vector literal storage, checked indexing, literal reassignment
    with changing runtime length, typed empty local vectors, length queries,
    stored-vector loops, and the temporary fixed-capacity local method surface
    now lower on both LLVM and the raw freestanding backend for supported
    element values. The current local vector storage grows to the largest
    literal, reserve capacity, or compiler-tracked push/insert capacity seen in
    the binding, while the stored length still shrinks and expands per
    assignment. The allocator, runtime capacity, and real grow-on-push behavior
    remain near-term.
    - [std-api] define ergonomic std collection methods only after generic
      impls, allocator capabilities, and runtime growth are in place, so the
      temporary compiler-known local API does not become permanent surface area
    - [iteration] lower iterator primitives for allocator-backed vectors
    - [patterns] connect fixed-length and rest vector patterns such as `[head, tail @ ..]` to stored vectors after runtime layout exists
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
