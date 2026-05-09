# Feature Test Matrix

This is the project checklist for testing one language feature at a time from
multiple directions. Every feature should eventually have positive, negative,
cross-feature, and backend tests.

## Per-Feature Checklist

For each feature:

- [ ] syntax parses the intended surface
- [ ] minimal positive program compiles and runs
- [ ] result is checked by exit code or stdout
- [ ] invalid syntax or semantic misuse has a negative test
- [ ] LLVM/glibc backend behavior is covered
- [ ] `--freestanding` behavior is covered when the feature is supported there
- [ ] interaction with modules or visibility is covered when names are involved
- [ ] interaction with ownership/borrowing is covered when values move or borrow
- [ ] generated LLVM IR is inspected when ABI or linking is involved
- [ ] docs mention the behavior and known limits

## Feature Plan

| Feature | Positive Directions | Negative Directions | Backend Directions | Status |
| --- | --- | --- | --- | --- |
| CLI/build modes | explicit output, `--emit-llvm`, `--module-path`/`-I`, `--shared`, `--freestanding` | incompatible flags, removed C++ options, missing input/output/compiler/search-path arg, unexpected arg | LLVM IR, optional LLVM link, and raw ELF | good first pass |
| Functions | `main`, calls, named params, unit-returning functions, tuple/array/struct/enum/wildcard parameter patterns, return values, recursion, void functions, function pointer values/calls, generics surface | missing `main`, wrong `main`, wrong arg count, bad parameter pattern type, missing return, bad return type, private module function | host and freestanding calls, function-entry parameter destructuring, indirect calls, unit return lowering | good first pass |
| Variables | `let`, `var`, annotations, assignment, scalar inference, unit `()` values, fixed-size array locals, default non-empty `[]` array literals, constant and local dynamic array indexing with runtime bounds checks, typed empty local `Vec[T]` literals, local `Vec[T]` literal reassignment with changing runtime length, fixed-capacity local `Vec.reserve(n)` / `Vec.push(value)` / `Vec.insert(index, value)` / `Vec.pop()` / `Vec.remove(index)` / `Vec.first()` / `Vec.last()` / `Vec.get(index)` / `Vec.contains(value)` / `Vec.index_of(value)` / `Vec.count(value)` / `Vec.capacity()` / `Vec.is_empty()` / `Vec.clear()` / `Vec.truncate(n)` / `Vec.set(index, value)` / `Vec.swap(a, b)`, `len(value)` / `value.len()` array, vector, and `Slice[T]` length queries, mutable local array/Vec `as_slice()` view creation, `Slice[T]` indexing, indexed assignment, and range slicing with runtime bounds checks, tuple/wildcard/unit/array/struct `let` and `var` pattern bindings, refutable single-payload enum `let`/`var` bindings, refutable enum or-pattern `let`/`var` bindings, match payload bindings | immutable assignment, shadowing, type mismatch, fixed array size/index/pattern mismatch, untyped empty `[]` literals, vector length/type mismatch, vector shrink bounds trap, tuple pattern arity/type mismatch, struct pattern non-struct/field mismatch | stack/local lowering, host string/float slots, unit `zeroinitializer`, fixed arrays, tuple/array/struct destructuring on host and freestanding, refutable enum binding panic path, vector LLVM storage and freestanding backend rejection | good first pass |
| Constants | top-level scalar constants, module paths/imports, scalar folding, scalar constant patterns, tuple/struct/tuple-struct/fixed-array/enum constant values, generic enum constant values from concrete annotations | cycle path diagnostics, private constants, scalar pattern type mismatch, aggregate arity mismatch, generic enum constant type mismatch, ownership/borrow-valued aggregate constants | inline scalar and aggregate IR lowering on LLVM and freestanding where the aggregate backend is supported | good first pass |
| Generics | generic functions, explicit `<T>` generic call type arguments, repeated function specializations, generic function names specialized as function pointer values from expected `fn(...) -> ...` types, generic structs, explicit and inferred generic struct literal / tuple-struct construction, generic inherent impl methods with `self` receivers, explicit and inferred method-level generic calls on inherent impl methods, generic inherent associated functions, explicit and inferred method-level generic associated calls, generic trait impl methods with `self` receivers, explicit and inferred generic trait method calls, method-level bounds on generic trait methods, generic trait impl associated functions, generic impl bounds, generic trait impl coherence, generic traits, generic ADT surface, generic enum constructors, generic enum constants, generic enum `?` / `??`, generic enum match and refutable payload bindings | inference conflicts, uninferred parameters, uninferred generic function pointer parameters, explicit type-argument arity/mismatch, duplicate generic parameters, unsatisfied generic impl bounds, overlapping generic/concrete trait impls, generic method arity mismatch, uninferred generic method parameters, generic trait method bound mismatch, generic associated arity mismatch, uninferred generic associated parameters, ambiguous trait associated functions, generic enum constructor errors, generic enum `??` fallback type mismatch, generic enum payload pattern type mismatch, generic enum constant mismatch | monomorphized function calls, specialized generic function pointer refs, specialized generic impl method and associated calls, aggregate surface validation, generic enum LLVM aggregate construction, try/coalesce extraction, compact payload pattern extraction, and inline generic enum constants | partial |
| Primitive types | signed/unsigned widths, suffix literals, bool, float/string host values | literal ranges, invalid casts, invalid suffixes | exact host types, freestanding scalar slots | partial |
| Operators and literals | arithmetic, modulo, bit ops, bitwise-not, shifts, comparisons, logical ops, postfix `?` with compatible residual conversion, generic prelude `Option`/`Result` and non-generic Maybe/Result-style `??`, integer base prefixes, C/Unicode string escapes, `f32`/`f64`/`f128` LLVM ops | non-bool logic, non-integer bit ops, invalid `?`, incompatible `?` residuals, invalid `??`, invalid base/escape literals | LLVM IR and freestanding x86-64 | good first pass |
| Control flow | `if`/`else if`, expression-valued `if`, expression-valued enum and aggregate `if let`, enum `if let` and `while let` with alias-wrapped or-pattern alternatives, value-producing block expressions, final-expression function returns, required `;` for non-final expression statements and non-block statements, typed `break label value`, tuple/struct-valued labeled block results, `if let`, `while`, `for range`, integer `Range[T]` and `RangeInclusive[T]`, `0..end`, `0..=end`, list literal and stored-vector `for` loops with irrefutable alias/tuple/struct/tuple-struct heads, `init while`, `let while`, labeled loop/block break | bad conditions, missing `;`, if-expression branch type mismatch, if-let-expression branch type mismatch, irrefutable aggregate if-let with else, final value accidentally terminated with `;`, labeled-block break type mismatch, labeled-block ownership exit mismatch, non-integer range types, continue outside loop, bad labels, planned iterator/refutable for patterns | host and freestanding loops plus aggregate block result binding, signed and unsigned range compares, multi-alternative if-let/while-let dispatch | good first pass |
| Ownership | `own`, move, explicit `drop`, `Drop::drop` destructor lowering, aggregate field destructor lowering, non-owned field read/assignment from owning structs, owned struct/tuple-struct field move and reinitialization, nested owned field paths, constant fixed-array/vector element move and reinitialization, return ownership, whole-value tracking for aggregates containing owned fields | use-after-move, owned field/element use-after-move, dynamic indexed owned element move, temporary owned field/element move, partial aggregate move, undropped owner, overwrite live owner, undropped owning aggregate | sema-focused, LLVM destructor call, freestanding destructor call, LLVM aggregate storage, moved-field destructor skip | good first pass |
| Borrowing | `ref`, `ref mut`, call borrows, named borrow bindings, lexical block release, aggregate bindings that retain borrowed sources, direct struct/tuple/constant array/vector field borrows, independent field-path borrow tracking | mut borrow immutable, overlapping field borrow, borrowed field assignment, source assignment while named-borrowed or aggregate-borrowed, return/discard borrow, borrow binding reassignment | host pointer lowering, freestanding address lowering, LLVM aggregate field pointers | good first pass |
| Structs and tuples | comma-separated fields, named struct literals, generic struct declarations, tuple structs, inherent associated constructors, module-visible field types, field `mut` assignment, unit/empty tuple type and literal, fixed-size local tuples, tuple index access, struct field/index access | semicolon field separators, duplicate structs, duplicate fields, unknown/private field types, bad generic arity, immutable field assignment, immutable binding field assignment, unknown/private associated constructors, unknown fields, tuple index range | front-end validation, tuple stack lowering, empty tuple lowering, local struct stack lowering on LLVM and freestanding, LLVM aggregate-return constructor calls | good first pass |
| Enums | comma-separated cases, constructors, compact payloads, LLVM aggregate layout for multi-payload and i64/u64 payload cases, aggregate enum pointer-shaped payloads (`string`, `ptr T`, `fn(...) -> ...`), equality for compact enums, generic ADT surface, generic enum type applications, explicit and payload-inferred generic enum constructors, generic enum constants | semicolon case separators, payload count, unsupported aggregate payload types, duplicate cases, generic enum constructor arity, uninferred generic enum constructor type arguments, generic enum constructor type mismatch, generic enum constant mismatch | tagged-union lowering, LLVM aggregate-enum lowering with pointer-to-slot casts, freestanding aggregate-enum rejection | partial |
| Pattern matching | exhaustive enum match, generic and non-generic enum payload bindings, wildcard, grouped patterns, unit `()` patterns, or-patterns with same-name/same-type bindings, alias patterns, alias-wrapped or-patterns, payload binding, enum payload literal/range/or/alias patterns, aggregate enum payload literal/range/or/alias patterns on LLVM, nested compact-enum subpatterns inside aggregate enum payloads on LLVM, bool payload literal exhaustiveness, multi-payload enum positional/rest binding patterns, module-qualified cases, integer/bool literal and range patterns, ordered first-match range overlap, scalar shadow warnings, finite integer range exhaustiveness, nested tuple match patterns, tuple `..` rest patterns, named struct patterns, tuple-struct patterns, finite product exhaustiveness, symbolic high-cardinality product exhaustiveness, aggregate shadow warnings, expression-valued match | non-exhaustive enum/integer/bool/tuple/struct matches, missing payload pattern, duplicate payload literals, invalid range order, mismatched or-pattern bindings, incomplete integer range coverage, incomplete bool payload coverage, incomplete product coverage with missing-case hints, slice patterns still planned, incompatible expression arms, wrong literal kind | branch lowering, expression phi/scalar result lowering, aggregate match if-chain lowering, aggregate enum LLVM extraction | good first pass |
| Modules | inline `mod`, file `mod`, `.arih`, package search paths, module load cache, compact module metadata emission/read-back validation, source content hashes, metadata format-version gating, source-snapshot module cache emission/use, per-source AST-summary cache records, declaration fingerprints, self-validated declaration-summary payloads, `pub`, `pub mod`, `use`, use groups, glob imports, module aliases, `pub use` re-exports, `A::B`, `self::`/`super::` relative paths | private access, private nested modules, duplicate aliases, missing file modules, stale module metadata, stale source hash, stale module cache cfg/source/import/AST-summary resolution, old metadata/cache versions without hashes or AST summaries, private re-export aliases, root `super::` | name resolution, module graph summary serialization, module cache source snapshot and AST-summary validation | good first pass |
| Prelude IO/format/input/assert | `print`, `println`, `std::print`, `std::println`, `std`-alias qualified print names, `print!`, `println!`, `matches!`, IO aliases, `read_byte`, host `read_line`/`input`, source `Option[T]` / `Maybe[T]` alias / `Result[T, E]` / `Slice[T]` and `slice(data, len)`, Slice `len`/`is_empty`/indexing/indexed assignment/range slicing, mutable local array/Vec `as_slice()` view creation, assertion function and macro helpers, escaped braces | bad placeholder count, nonliteral format, planned `format!`, freestanding line input, bad assert type, bad assert macro type, bad `matches!` arity/patterns | stdio, getchar/fgets, syscall byte IO, assertion exits, pattern-engine macro lowering | partial |
| Context runtime | `context::argc`, `context::arg`, aliases, `@ari_entry`/`ari::main` host wrapper | out-of-range behavior, shared-library context use | host runtime init | partial |
| C FFI | libc calls, aliases, module externs, concrete C ABI bindings only, C ABI type aliases, C varargs with default promotions, C callback/function-pointer params, helper library, `ref mut` pointers, `ptr c_char` strings, `@repr(C)` structs with value/raw-pointer/borrow fields, generic `@repr(C)` structs with pointer-sized generic fields, generic fieldless `@repr(C)` enums, `null` raw-pointer literals, nullable `T?` raw-pointer type spelling, raw-pointer casts, byte-wise `ptr_offset`, typed scalar/aggregate `ptr_add`, `size_of<T>()` / `align_of<T>()`, scalar/plain-aggregate `ptr_load`/`ptr_store`, scalar/plain-aggregate `*pointer` dereference load/store, scalar struct/tuple/fixed-array field and element access through raw pointers, `c_void` returns | rejected ABI, body, generic extern declarations, bad link symbol, by-value `c_void` param, non-extern varargs, unsupported aggregate varargs, variadic extern function pointer values, `@repr(C)` owning fields, `@repr(C)` value-stored generic fields, payload-bearing `@repr(C)` enums, null used as non-pointer, nullable `?` combined with `own`/`ref`/`ptr`, non-pointer value initializers for `T?`, bad layout query arity, dereference of non-pointer values, whole raw-pointer copies of ownership/borrow-valued aggregates | LLVM declarations, explicit IR ABI split for Ari builtins vs C externs, function pointer params as `ptr`, vararg bool/narrow-int/f32 promotion casts, `ptrtoint`/`inttoptr`, `getelementptr i8`, typed scalar and aggregate pointer GEP, scalar and aggregate layout literals, scalar and plain-aggregate raw pointer load/store, pointer dereference load/store, aggregate pointer GEP, raw backend pointer-backed field/array addressing, raw backend address add, and host linker | good first pass |
| Explicit memory zones | `Zone`, `zone::create`, lexical `zone::temp`, raw `zone::alloc`, typed `zone::alloc<T>`, placement `zone::new<T>`, local scratch `zone::scratch<T>`, explicit scratch promotion `zone::promote<T>`, zone-backed associated `T::new(ref mut Zone, ...)`, `zone::reset`, `zone::destroy`, raw `ptr u8` and typed `ptr T` allocation used with layout queries and pointer load/store | missing destroy for non-temporary `own Zone`, rejected `drop zone`, rejected movement of temporary zones, rejected temporary-zone pointer escape through returns, outer bindings, aggregates, or call arguments, `zone::scratch<T>` rejected outside local pointer binding initializers, bad typed allocation arity, zero-sized typed allocation, bad placement-construction/promotion arity, ownership/borrow-valued placement/promotion rejection, direct local and single-zone wrapper zone-pointer use after reset/destroy, reset invalidation after `if`/`match`/loop joins, aggregate/FFI/multi-zone zone-pointer escape rejection, freestanding zone allocation rejection | LLVM host runtime backed by `malloc`/`free`, ownership move into `zone::destroy`, automatic destroy insertion for temporary-zone scope fallthrough, returns, and escaping `break`/`continue`/labeled-block exits, `zone::scratch<T>` lowered to hidden `zone::temp` plus placement `zone::new<T>`, `zone::promote<T>` lowered to pointer load plus placement `zone::new<T>` in the target zone, typed allocation lowered to raw allocation with compile-time layout, placement construction lowered to allocation plus store, inherent associated call to zone placement, direct and single-zone wrapper pointer provenance invalidation with named temporary-zone escape diagnostics, control-flow zone generation merge, deliberate host-only zone policy | good first pass |
| Shared libraries | compile library, Ari-mangled symbols, `@export`, `@export("symbol")`, `@no_mangle` | main not required only with `--shared`, invalid export symbols | dynamic symbol table | partial |
| Comments | line and nested block comments | unterminated block comment | lexer | partial |
| Attributes and meta | built-in attributes, `@repr(C)` layout guards including borrow-field and pointer-sized generic slots plus generic fieldless enums, `@cfg(true/false)` declaration pruning, boolean/target/feature cfg predicates, command-line cfg feature flags, deprecated use warnings, `@test` runner generation, `@export`/`@no_mangle` symbol controls, user-reserved attribute names via concrete non-generic `meta fn`, meta signatures over `token_stream`/`ast`/`type` | unknown attributes, bad attribute placement, bad `repr`, `@repr(C)` owning fields, `@repr(C)` value-stored generic fields, payload-bearing `@repr(C)` enums, bad `@cfg` predicate, bad deprecated/export arguments, bad `@test` signature, bad meta signatures, generic meta functions, planned macro invocation | parser pruning, sema validation, warning emission, freestanding test runner, LLVM/shared symbol selection | good first pass |
| Front-end surfaces | structs, traits, trait generics, `dyn Trait[...]` trait-object type syntax, explicit `value as dyn Trait[...]` impl checks, concrete and generic-impl-specialized copyable LLVM dyn dispatch, impl conformance, concrete method dispatch, generic function trait bounds, constrained static dispatch, generics, meta syntax, Rust-like prelude trait names and ADTs, required `Drop::drop` method | removed class/interface syntax, unknown traits and trait bounds, invalid trait object arity/qualifiers, implicit concrete-to-dyn assignment, missing dyn-conversion impls, dyn-to-dyn upcasts, non-object-safe generic trait methods under dyn dispatch, duplicate impls, missing/mismatched trait methods, missing impls for generic trait bounds, unknown/ambiguous method calls, planned aggregate destructuring syntax, still-planned owned prelude ADTs | parser/sema validation, static method-call lowering, constrained generic method selection, LLVM dyn vtable globals and erased receiver thunks for concrete and generic impls; freestanding dyn dispatch remains planned | partial |
| Unsupported aggregates | vector type checking, list literal constant indexing, fixed-size array surface, non-local aggregate ABI gaps | backend rejection, vector index bounds, non-local aggregate ABI rejection | clear diagnostics | partial |

## Completed Sprint: C FFI

Goal: C FFI must be boring and reliable. Ari supports only C ABI for foreign
libraries at the source level; C++ interop should go through explicit C wrapper
functions. Ari-owned runtime hooks use the separate reserved `extern "ari"`
ABI and validated `ari_builtin_*` symbols.

Checklist:

- [x] `extern "C" fn puts(...)`
- [x] `extern fn puts(...)` shorthand
- [x] `extern "C" fn local(...) = "external_symbol"`
- [x] extern declarations inside modules
- [x] generated LLVM IR declares external C symbols with explicit link names
- [x] external numeric arguments and return values
- [x] external string arguments
- [x] external `ref mut` argument lowered as pointer
- [x] nullable raw-pointer literals for `ptr T`
- [x] nullable raw-pointer type suffix `T?`
- [x] raw-pointer casts lower to LLVM `ptrtoint`/`inttoptr`
- [x] byte-wise `ptr_offset` lowers to LLVM `getelementptr i8` and raw address add
- [x] typed scalar `ptr_add` lowers to LLVM typed `getelementptr` and raw scaled address add
- [x] typed aggregate `ptr_add` lowers to LLVM aggregate `getelementptr` and raw Ari-layout scaled address add
- [x] `size_of<T>()` and `align_of<T>()` expose scalar and Ari-layout aggregate byte counts
- [x] scalar `ptr_load`/`ptr_store` lower on LLVM and raw freestanding backends
- [x] plain aggregate `ptr_load`/`ptr_store` and `*pointer` whole-copy lower on LLVM and raw freestanding backends
- [x] scalar field/element access through raw aggregate pointers lowers on LLVM and raw freestanding backends
- [x] C variadic extern declarations, default promotions, calls, and function-pointer rejection
- [x] C callback/function-pointer parameters lower as `ptr` and accept Ari function names
- [x] reserved `extern "ari"` builtin declarations for the source `std` header
- [x] `extern "ari"` declarations lower through a distinct IR ABI and do not
      emit foreign C declarations for builtin hooks
- [x] reject `extern "C++"`
- [x] reject extern functions with bodies
- [x] reject generic extern declarations as a permanent concrete-C-symbol policy
- [x] reject invalid external link symbols

## Completed Sprint: CLI And Build Modes

Goal: compiler invocation should fail early and predictably, and every output
mode should leave an artifact that can be checked by exit code, stdout, or file
presence.

Checklist:

- [x] compile a normal glibc-backed executable with `-o`
- [x] execute the normal output and check its exit code
- [x] emit generated LLVM IR with `--emit-llvm`
- [x] inspect generated LLVM IR for Ari `main` and host wrapper
- [x] reject removed generated-C++ options
- [x] compile and run a raw Linux executable with `--freestanding`
- [x] freestanding output is executable without a test-side `chmod +x`
- [x] compile a shared library with `--shared`
- [x] reject missing input
- [x] reject missing `-o` path
- [x] reject missing `--emit-llvm` path
- [x] reject missing `--module-path` and `-I` paths
- [x] reject missing `--llvm-cc` compiler path
- [x] reject removed `--backend`
- [x] reject removed `--cc`
- [x] reject `--freestanding` combined with LLVM output options
- [x] reject unexpected extra input arguments

## Completed Sprint: Functions

Goal: function calls should be trustworthy before widening the language around
methods, trait-bound dispatch, generics, and richer patterns.

Checklist:

- [x] require executable `main`
- [x] reject `main` parameters
- [x] require `main -> i64`
- [x] require non-void functions to return on all paths
- [x] check return expression type against the function return type
- [x] accept functions with omitted return type as `void`
- [x] accept bare `return` in void functions
- [x] reject value returns from void functions
- [x] check function argument count
- [x] check function argument types
- [x] support direct recursion on LLVM IR backend
- [x] support direct recursion on freestanding backend
- [x] support freestanding calls beyond six arguments with stack slots
- [x] confirm seven-argument calls work
- [x] accept generic function syntax as a parsed surface
- [x] monomorphize simple generic function calls
- [x] monomorphize explicit generic function calls such as `identity<i64>(value)`
- [x] specialize generic function names into function pointer values from expected `fn(...) -> ...` types
- [x] lower explicit generic struct literals and tuple-struct constructors
- [x] infer generic struct literal and tuple-struct constructor type arguments from field/argument values
- [x] specialize generic inherent impl methods with `self` receivers
- [x] specialize generic trait impl methods with `self` receivers
- [x] enforce generic impl bounds at specialization sites
- [x] specialize generic inherent associated functions with explicit associated call type arguments
- [x] reject overlapping concrete and generic trait impls
- [x] specialize method-level generic inherent impl methods with explicit method call type arguments
- [x] infer method-level generic inherent impl arguments from call arguments
- [x] specialize method-level generic inherent associated functions with explicit or inferred call arguments
- [x] emit mangled function symbols in LLVM IR and raw ELF `.symtab`
- [x] lower function parameter patterns at function entry
- [x] call `extern "C"` functions from normal Ari functions
- [x] expose public module functions
- [x] hide private module functions from outside modules

## Completed Sprint: Control Flow

Goal: loops and branches should be easy to trust before the language grows more
syntax around iterators and pattern-driven control flow.

Checklist:

- [x] `while` with `continue`
- [x] `while` with `break`
- [x] `if` / `else if` / `else` branch chains
- [x] expression-valued `if` / `else if` / `else` with branch type unification
- [x] expression-valued `if let` with enum payload binding
- [x] integer-to-bool condition coercion as `value != 0`
- [x] `if let EnumCase(payload) = value`
- [x] `while let EnumCase(payload) = value`
- [x] `for value in range(start, end)`
- [x] `for value in start..end`
- [x] `for value in start..=end`
- [x] `for value in range_inclusive(start, end)`
- [x] integer `Range[T]` and `RangeInclusive[T]` local values with field and tuple-index access
- [x] `for value in stored_range`
- [x] `for _ in range(start, end)` wildcard pattern
- [x] empty ranges do not run the loop body
- [x] `iter::range(start, end)` qualified prelude form
- [x] `for value in [a, b, c]` list literal iteration
- [x] `for value in stored_vec` local vector iteration on the LLVM backend
- [x] `for _ in [a, b, c]` list literal wildcard pattern
- [x] irrefutable tuple, named struct, tuple-struct, and alias loop heads for list literal iteration
- [x] irrefutable tuple, named struct, tuple-struct, and alias loop heads for stored local vector iteration
- [x] list literal `for` with `break` and `continue`
- [x] stack-backed local `Vec[T]` literals on the LLVM backend
- [x] typed empty local `Vec[T]` literals through annotations and assignments
- [x] checked dynamic indexing of stored local `Vec[T]`
- [x] local `Vec[T]` literal reassignment with changing runtime length
- [x] `len(value)` and `value.len()` for local vectors, fixed arrays, and constant-folded literal length
- [x] fixed-capacity local `Vec.reserve(n)` and auto-widening local `Vec.push(value)` on the LLVM backend
- [x] checked local `Vec.insert(index, value)` on the LLVM backend
- [x] checked local `Vec.pop()` on the LLVM backend
- [x] checked local `Vec.remove(index)` on the LLVM backend
- [x] checked local `Vec.first()` and `Vec.last()` on the LLVM backend
- [x] checked local `Vec.get(index)` on the LLVM backend
- [x] checked local `Vec.contains(value)`, `Vec.index_of(value)`, and `Vec.count(value)` on the LLVM backend
- [x] fixed-capacity local `Vec.capacity()` on the LLVM backend
- [x] `value.is_empty()` for local vectors, fixed arrays, and constant-folded literals
- [x] fixed-capacity local `Vec.clear()` on the LLVM backend
- [x] fixed-capacity local `Vec.truncate(n)` on the LLVM backend
- [x] checked local `Vec.set(index, value)` on the LLVM backend
- [x] checked local `Vec.swap(a, b)` on the LLVM backend
- [x] frozen local `Vec[T]` API rejects unsupported compiler-known methods with allocator-backed std guidance
- [x] keep stored local `Vec[T]` rejected on the raw freestanding backend
- [x] `init ... while ... next ...` normal update path
- [x] `let ... while ... next ...` loop-state spelling
- [x] `continue value, ...` inside init-while
- [x] labeled loop `break label`
- [x] labeled block `break label`
- [x] LLVM/glibc backend loop lowering
- [x] freestanding loop execution across while, while-let, range-for, vector-for, and init-while
- [x] reject non-bool/non-integer loop conditions
- [x] reject `break` outside loops
- [x] reject unknown or duplicate active loop labels
- [x] reject `continue` outside loops
- [x] reject value `continue` outside init-while
- [x] reject non-`range` for iterables
- [x] reject general `Iterator[T]` for-loop lowering with a planned diagnostic
- [x] reject enum-case `for` patterns with a planned diagnostic
- [x] reject wrong `range` arity
- [x] reject direct untyped empty `[]` iteration
- [x] reject mixed-type list literal iteration
- [x] reject wrong init-while `next` arity
- [x] reject wrong init-while `continue` arity

## Completed Sprint: Pattern Matching

Goal: enum `match` should be predictable across payload extraction, wildcard
coverage, branch flow, and backend lowering.

Checklist:

- [x] exhaustive enum match with all arms returning
- [x] payload binding in a match arm
- [x] payload ignore with `_`
- [x] multi-payload enum binding and `..` rest payload ignore on LLVM
- [x] wildcard arm covers remaining cases
- [x] match where one arm returns and another continues
- [x] match over module-qualified enum cases
- [x] scalar constant patterns in integer and bool matches
- [x] module-qualified constant patterns in enum payloads
- [x] scalar constant initializer arithmetic, bool, and comparison folding
- [x] aggregate constant values for tuples, structs, tuple structs, enums, and fixed arrays
- [x] constant cycle diagnostics report the full dependency path
- [x] fixed-size array binding and match patterns with `..` rest
- [x] expression-valued `match` can bind payloads and produce a scalar result
- [x] expression-valued `match` lowers on LLVM/glibc and freestanding backends
- [x] LLVM/glibc backend match lowering
- [x] freestanding backend match execution
- [x] reject empty match
- [x] reject non-exhaustive match
- [x] reject missing payload pattern
- [x] reject duplicate enum case arm
- [x] reject unreachable arm after wildcard
- [x] reject payload pattern on no-payload case
- [x] reject case from a different enum
- [x] reject match over non-enum values
- [x] reject incompatible ownership states after match arms
- [x] reject incompatible ownership states after match expression arms

## Completed Sprint: Modules

Goal: module namespaces should stay boring as more language features start to
create names. Public access, aliases, and qualified names should resolve the
same way across functions and enum constructors.

Checklist:

- [x] `mod` creates a qualified namespace
- [x] public module functions are callable with `A::B`
- [x] private module functions are callable from the same module
- [x] private module functions are rejected from outside
- [x] root-scope `use` imports a function
- [x] root-scope `use ... as ...` aliases a function
- [x] root-scope `use A::{x, y as z}` imports selected items
- [x] root-scope `use A::*` imports public constants, functions, enum types, and enum cases
- [x] module path aliases work as qualified prefixes
- [x] module-scope `use ... as ...` stays local to that module
- [x] `pub use` re-exports functions, enum types, and enum cases
- [x] `pub use A::*` re-exports public module surfaces
- [x] file-backed `mod source as Alias;` loads one file under another module path
- [x] nested module functions are callable with full `A::B::C`
- [x] `pub mod` exposes nested module paths outside their parent module
- [x] `self::` and `super::` resolve in item, type, pattern, and `use` paths
- [x] public module enums are usable with qualified enum types
- [x] enum type aliases through `use ... as ...`
- [x] enum constructor aliases through `use ... as ...`
- [x] qualified enum constructors still work beside aliases
- [x] LLVM/glibc backend module name lowering
- [x] freestanding backend module name lowering
- [x] file-backed `mod name;` loads `name.ari`
- [x] header-like `mod name;` loads `name.arih`
- [x] package search paths work with `--module-path`, `-I path`, and `-Ipath`
- [x] repeated file-backed module imports are cached by resolved module name
- [x] compact module metadata can be emitted and read back for source-graph validation
- [x] metadata records stable source content hashes for cache invalidation
- [x] metadata check rejects old v1 summaries that cannot validate source hashes
- [x] metadata/cache parsing rejects duplicate source, import, item summary,
      and AST-summary records
- [x] stale module metadata reports changed cfg, source, import, and item records
- [x] source-snapshot module caches can be emitted and reused after validating
      search paths, cfg features, implicit `std`, source hashes, and import
      resolution
- [x] validated source-snapshot module cache use resolves module imports from
      the cached import table instead of rediscovering candidate files
- [x] source-snapshot module cache use rechecks the parsed cached source graph
      against the cache's embedded metadata
- [x] source-snapshot module cache parsing rejects duplicate source snapshot records
- [x] source-snapshot module cache emits and reloads per-source AST summary records
- [x] AST summary records include declaration fingerprints and declaration
      payloads, then reject tampered fingerprints, payloads, or count/payload
      disagreement
- [ ] AST/IR package cache summaries skip dependency parsing after validation
- [x] reject duplicate `use` aliases in one module scope
- [x] reject duplicate aliases introduced by glob imports
- [x] reject private function access through a `use` alias
- [x] reject public items behind private nested modules from outside the parent
- [x] reject private non-`pub use` aliases through qualified access
- [x] private items are not imported by glob use from outside the module
- [x] reject private enum type access
- [x] reject private enum case access
- [x] reject private constant access
- [x] reject missing file-backed modules
- [x] reject `super::` paths that escape the root module

## Completed Sprint: Prelude Surface

Goal: make common Rust-like prelude names part of the compiler-known ABI
surface so library code can refer to one stable spelling before trait-bound
dispatch, source standard modules, macro expansion, and allocator-backed types are
implemented.

Checklist:

- [x] add byte input builtins: `read_byte`, `io::read_byte`, `input::read_byte`
- [x] add host line input builtins: `read_line`, `io::read_line`, `input`,
      `input::line`
- [x] add assertion/stop function builtins: `assert`, `debug_assert`,
      `assert_eq_i64`, `assert_ne_i64`, `assert_eq_bool`, `assert_ne_bool`,
      `panic`, `todo`, `unreachable`
- [x] add executable prelude macro forms: `assert!`, `debug_assert!`,
      `assert_eq!`, `assert_ne!`, `panic!`, `todo!`, `unreachable!`,
      `print!`, and `println!`
- [x] lower `matches!` through the pattern engine
- [x] reserve `format!` with targeted planned diagnostics
- [x] reserve Rust-like standard traits: debug/display/default/clone/copy/drop,
      equality/order, conversion, iterator, string/owned surfaces
- [x] reserve `Iterable[T]`
- [x] reserve `Iterator[T]`
- [x] reserve `IntoIterator[T]`
- [x] reserve `iter::Iterable[T]`
- [x] reserve `iter::Iterator[T]`
- [x] reserve `iter::IntoIterator[T]`
- [x] accept impls against prelude iterator traits
- [x] reject unknown trait impls
- [x] reject wrong trait type-argument counts
- [x] reject duplicate trait impls for the same trait/type pair
- [x] validate trait impl method completeness and signatures
- [x] accept bare `self` in trait/impl method signatures as `Self`
- [x] lower concrete `value.method(...)` calls through matching impl methods
- [x] reject unknown and ambiguous method calls
- [x] expose source `Option[T]`, `Maybe[T]` as an `Option[T]` alias, and
      `Result[T, E]` through the source prelude
- [x] expose source `Slice[T]` and `slice(data, len)` as a non-owning
      pointer/length view through the source prelude, with `len(view)`,
      `view.len()`, `view.is_empty()`, `view[index]`, and `view[index] = value`
      helpers, mutable local array/Vec `as_slice()` view creation, and
      `view[start..end]` / `view[start..=end]` range slicing
- [x] reject reserved `Box` surfaces with roadmap-backed diagnostics
- [x] reject non-`i64` prelude range bounds until generic range lowering exists
- [x] add `lib/std.arih` source declarations for the stable
      declaration-shaped prelude surface, auto-load it as `std`, and verify
      explicit `std::...` calls through the LLVM backend
- [x] support `--no-implicit-std` so the same header can be tested only through
      ordinary `mod std;` file-backed module loading
- [x] reject compiler-known prelude helpers under `--no-implicit-std` until a
      real source `std` module is loaded
- [x] reject bare and prelude-path compiler-known function signatures such as
      `write_i64` and `io::write_i64` under `--no-implicit-std` until source
      `std` is loaded
- [x] reject compiler-known prelude trait names such as `Debug` under
      `--no-implicit-std` until source `std` is loaded
- [x] require source generic declarations before lowering declaration-shaped
      `std::mem`, `std::iter`, and typed `std::zone` helpers through compiler
      hooks
- [x] remove compiler-injected concrete prelude function signatures and trait
      fallback declarations; ordinary calls and impls now resolve against
      source `std` declarations plus implicit aliases
- [x] split Ari-owned builtins from C FFI with `extern "ari"` declarations and
      reject unknown `ari_builtin_*` symbols
- [x] add Rust-like implicit prelude aliases for public `std` root items and
      root re-exports, including type/trait names, IO helpers, zone helpers,
      and sema-lowered helpers while preserving local shadowing
- [x] add implicit aliases for public `std` child modules so nested prelude
      forms such as `fmt::Display` and `iter::Iterator[T]` resolve through
      source `std` declarations
- [x] allow root re-exports such as `input()` to coexist with child-module
      paths such as `input::read_byte()`
- [x] centralize Ari builtin source aliases so LLVM and freestanding lowering
      accept the same root `std::...` re-export spellings
- [x] keep `Hash` out of the prelude
