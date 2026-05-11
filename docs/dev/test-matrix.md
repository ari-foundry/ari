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
| Functions | `main`, calls, named params, unit-returning functions, tuple/fixed-array/struct/tuple-struct/alias/enum/wildcard parameter patterns, return values, recursion, void functions, function pointer values/calls, generics surface | missing `main`, wrong `main`, wrong arg count, duplicate concrete function paths, bad parameter pattern type, missing return, bad return type, private module function | host and freestanding calls, function-entry parameter destructuring, indirect calls, unit return lowering, freestanding narrow scalar return normalization, freestanding direct `f32`/`f64` parameters and returns, freestanding direct/function-pointer aggregate parameters and returns through hidden pointers | good first pass |
| Variables | `let`, `var`, annotations, assignment, scalar inference, unit `()` values, fixed-size array locals, default non-empty `[]` array literals, constant and local dynamic array indexing with runtime bounds checks, typed empty local `Vec[T]` literals, local `Vec[T]` literal reassignment with changing runtime length, fixed-capacity local literal/const/static-expr/known-local/runtime-checked `Vec.reserve(n)` / static-length-tracked `Vec.truncate(n)` / `Vec.push(value)` / `Vec.insert(index, value)` / `Vec.pop()` / `Vec.remove(index)` / `Vec.first()` / `Vec.last()` / `Vec.get(index)` / `Vec.contains(value)` / `Vec.index_of(value)` / `Vec.count(value)` / `Vec.capacity()` / `Vec.is_empty()` / `Vec.clear()` / `Vec.set(index, value)` / `Vec.swap(a, b)`, `len(value)` / `value.len()` array, vector, and `Slice[T]` length queries, mutable local array/Vec `as_slice()` view creation, `Slice[T]` indexing, indexed assignment, and range slicing with runtime bounds checks, tuple/wildcard/unit/array/struct `let` and `var` pattern bindings, refutable tuple/fixed-array/named-struct/tuple-struct or-pattern `let`/`var` bindings, refutable single-payload enum `let`/`var` bindings, refutable enum or-pattern `let`/`var` bindings, match payload bindings | immutable assignment, shadowing, type mismatch, fixed array size/index/pattern mismatch, untyped empty `[]` literals, vector length/type mismatch, vector reserve/shrink bounds trap, tuple pattern arity/type mismatch, struct pattern non-struct/field mismatch | stack/local lowering, host string/float slots, unit `zeroinitializer`, fixed arrays, tuple/array/struct destructuring on host and freestanding, refutable aggregate and enum binding panic paths, vector LLVM storage and freestanding backend rejection | good first pass |
| Constants | top-level scalar constants, file-backed module constants, module-cache scalar and aggregate constant summary materialization, module paths/imports, scalar folding, scalar constant patterns, tuple/struct/tuple-struct/fixed-array/enum constant values, generic enum constant values from concrete annotations | cycle path diagnostics, private constants, scalar pattern type mismatch, aggregate arity mismatch, invalid constant shifts, generic enum constant type mismatch, ownership/borrow-valued aggregate constants | inline scalar and aggregate IR lowering on LLVM and freestanding where the aggregate backend is supported | good first pass |
| Generics | generic functions, explicit `<T>` generic call type arguments, repeated function specializations, generic function names specialized as function pointer values from expected `fn(...) -> ...` types, generic structs, explicit and inferred generic struct literal / tuple-struct construction, generic inherent impl methods with `self` receivers, explicit and inferred method-level generic calls on inherent impl methods, generic inherent associated functions, explicit and inferred method-level generic associated calls, generic trait impl methods with `self` receivers, explicit and inferred generic trait method calls, method-level bounds on generic trait methods, generic trait impl associated functions, trait-qualified associated function calls with explicit implementing type arguments and expected-result inference, generic impl bounds, generic trait impl coherence, generic traits, generic ADT surface, explicit/payload-inferred/expected-type generic enum constructors, generic enum constants, generic enum `?` / `??`, generic enum match and refutable payload bindings | inference conflicts, uninferred parameters, uninferred generic function pointer parameters, explicit type-argument arity/mismatch, duplicate generic parameters, unsatisfied generic impl bounds, overlapping generic/concrete trait impls, generic method arity mismatch, uninferred generic method parameters, generic trait method bound mismatch, generic associated arity mismatch, uninferred generic associated parameters, ambiguous trait associated functions, generic enum constructor errors, generic enum `??` fallback type mismatch, generic enum payload pattern type mismatch, generic enum constant mismatch | monomorphized function calls, specialized generic function pointer refs, specialized generic impl method and associated calls, aggregate surface validation, generic enum LLVM aggregate construction, try/coalesce extraction, compact payload pattern extraction, and inline generic enum constants | partial |
| Primitive types | signed/unsigned widths, suffix literals, bool, float/string host values, freestanding local `f32`/`f64` literal storage and raw pointer copies | literal ranges, invalid casts, invalid suffixes | exact host types, width-aware freestanding scalar local loads/stores, byte-packed standalone scalar locals, raw `f32`/`f64` literal and pointer bit storage, raw backend tuple/struct/fixed-array byte layout, local aggregate enum layout, external ABI still pending | partial |
| Operators and literals | arithmetic, modulo, bit ops, bitwise-not, shifts, comparisons, logical ops, postfix `?` with compatible residual conversion, generic prelude `Option`/`Result` and non-generic Option/Result-style `??`, integer base prefixes, C/Unicode string escapes, integer, float-width, and integer/float explicit casts, `f32`/`f64`/`f128` LLVM ops, `f32`/`f64` freestanding arithmetic/comparisons/casts | non-bool logic, non-integer bit ops, invalid `?`, incompatible `?` residuals, invalid `??`, invalid base/escape literals, invalid casts, freestanding f128 ops | LLVM IR and freestanding x86-64, with raw `f32`/`f64` support through local literal, raw pointer bit storage, SSE arithmetic, ordered comparisons, width casts, integer/float casts, and direct call scalar ABI slots | good first pass |
| Control flow | `if`/`else if`, expression-valued `if`, expression-valued enum and aggregate `if let`, enum `if let` and `while let` with alias-wrapped or-pattern alternatives, aggregate `if let` statement/expression and aggregate `while let` or-pattern alternatives for tuple/fixed-array/named-struct/tuple-struct bindings, value-producing block expressions, aggregate-valued `if`/`match`/block expression results with expected result type propagation into result arms, final-expression function returns, required `;` for non-final expression statements and non-block statements, typed `break label value`, tuple/struct-valued labeled block results, `if let`, `while`, `for range`, integer `Range[T]` and `RangeInclusive[T]`, `0..end`, `0..=end`, range `for` loops with irrefutable alias heads, list literal and stored-vector `for` loops with irrefutable alias/tuple/struct/tuple-struct heads and source-known Vec control-flow loop bounds, direct `Iterator[T]` and `IntoIterator[T]` for loops including mutable `Iterator.next` receivers, iterator item scalar literal/range/or, fieldless enum-case, compact enum payload, and nested aggregate enum multi-payload item patterns with nested or-pattern slots, `init while`, `let while`, labeled loop/block break | bad conditions, missing `;`, if-expression branch type mismatch, if-let-expression branch type mismatch, irrefutable aggregate if-let with else, final value accidentally terminated with `;`, labeled-block break type mismatch, labeled-block ownership exit mismatch, non-integer range types, continue outside loop, bad labels, unsupported iterator/refutable for-loop shapes | host and freestanding loops plus aggregate control-flow result materialization, signed and unsigned range compares, multi-alternative if-let/while-let dispatch, aggregate product if-chain lowering, iterator `while let Some(pattern)` lowering | good first pass |
| Ownership | `own`, move, explicit `drop`, `Drop::drop` destructor lowering, aggregate field destructor lowering, non-owned field read/assignment from owning structs, owned struct/tuple-struct field move and reinitialization, nested owned field paths, constant fixed-array/vector element move and reinitialization, return ownership, whole-value tracking for aggregates containing owned fields | use-after-move, owned field/element use-after-move, dynamic indexed owned element move, temporary owned field/element move, partial aggregate move, undropped owner, overwrite live owner, undropped owning aggregate | sema-focused, LLVM destructor call, freestanding destructor call, LLVM aggregate storage, moved-field destructor skip | good first pass |
| Borrowing | `ref`, `ref mut`, call borrows, named borrow bindings, lexical block release, aggregate bindings that retain borrowed sources, direct struct/tuple/constant array/vector field borrows, independent field-path borrow tracking | mut borrow immutable, overlapping field borrow, borrowed field assignment, source assignment while named-borrowed or aggregate-borrowed, return/discard borrow, borrow binding reassignment | host pointer lowering, freestanding address lowering, LLVM aggregate field pointers | good first pass |
| Structs and tuples | comma-separated fields, named struct literals, generic struct declarations, tuple structs, inherent associated constructors, module-visible field types, field `mut` assignment, unit/empty tuple type and literal, fixed-size local tuples, tuple index access, struct field/index access | semicolon field separators, duplicate structs, duplicate fields, unknown/private field types, bad generic arity, immutable field assignment, immutable binding field assignment, unknown/private associated constructors, unknown fields, tuple index range | front-end validation, tuple stack lowering, empty tuple lowering, local struct stack lowering on LLVM and freestanding, LLVM aggregate-return constructor calls, freestanding direct aggregate returns | good first pass |
| Enums | comma-separated cases, constructors, compact payloads, LLVM aggregate layout for multi-payload and i64/u64 payload cases, aggregate enum pointer-shaped payloads (`string`, `ptr T`, `fn(...) -> ...`), homogeneous nested aggregate-enum payload slots on LLVM and freestanding, equality for compact enums, generic ADT surface, generic enum type applications, explicit, payload-inferred, and expected-type generic enum constructors, generic enum constants | semicolon case separators, payload count, unsupported aggregate payload types, mixed scalar/nested aggregate enum payload slots, duplicate cases, generic enum constructor arity, uninferred generic enum constructor type arguments, generic enum constructor type mismatch, generic enum constant mismatch | tagged-union lowering, LLVM aggregate-enum lowering with pointer-to-slot casts, freestanding local aggregate-enum constructor/copy storage, aggregate-enum parameter passing, pointer-backed aggregate-enum copies, direct aggregate-enum constructor stores through raw pointers, and tag/payload-binding/literal/range/nested compact or homogeneous aggregate-enum matches, external aggregate enum ABI still pending | partial |
| Pattern matching | exhaustive enum match, generic and non-generic enum payload bindings, wildcard, grouped patterns, unit `()` patterns, or-patterns with same-name/same-type bindings across enum, tuple, fixed-array, named-struct, and tuple-struct match arms, alias patterns, alias-wrapped or-patterns, payload binding, enum payload literal/range/or/alias patterns, aggregate enum payload literal/range/or/alias patterns on LLVM, local aggregate enum payload literal/range/or/alias patterns on freestanding, nested compact-enum subpatterns inside aggregate enum payloads on LLVM and local freestanding values, homogeneous nested aggregate-enum subpatterns inside aggregate enum payloads on LLVM and freestanding, local aggregate enum positional/rest payload bindings on freestanding, bool payload literal exhaustiveness, multi-payload enum positional/rest binding patterns, module-qualified cases, integer/bool literal and range patterns, ordered first-match range overlap, scalar shadow warnings, finite integer range exhaustiveness, nested tuple match patterns, tuple `..` rest patterns, named struct patterns, tuple-struct patterns, finite product exhaustiveness, symbolic high-cardinality product exhaustiveness, aggregate shadow warnings, expression-valued match | non-exhaustive enum/integer/bool/tuple/struct matches, missing payload pattern, duplicate payload literals, invalid range order, mismatched or-pattern bindings, incomplete integer range coverage, incomplete bool payload coverage, incomplete product coverage with missing-case hints, slice patterns still planned, incompatible expression arms, wrong literal kind | branch lowering, expression phi/scalar and aggregate result lowering, aggregate enum LLVM extraction, raw local aggregate enum tag tests and payload binding/literal/range/nested compact or homogeneous aggregate-enum loads | good first pass |
| Modules | inline `mod`, file `mod`, `.arih`, package search paths, module load cache, compact module metadata emission/read-back validation, source content hashes, metadata format-version gating, source-snapshot module cache emission/use, per-source AST-summary cache records, declaration fingerprints, self-validated declaration-summary payloads, `pub`, `pub mod`, `use`, use groups, glob imports, module aliases, `pub use` re-exports, `A::B`, `self::`/`super::` relative paths | private access, private nested modules, duplicate aliases, missing file modules, stale module metadata, stale source hash, stale module cache cfg/source/import/AST-summary resolution, old metadata/cache versions without hashes or AST summaries, private re-export aliases, root `super::` | name resolution, module graph summary serialization, module cache source snapshot and AST-summary validation | good first pass |
| Prelude IO/format/input/assert | `print`, `println`, `std::print`, `std::println`, `std`-alias qualified print names, `print!`, `println!`, `matches!`, IO aliases, `read_byte`, host `read_line`/`input`, source `Option[T]` / `Result[T, E]` / `Slice[T]` and `slice(data, len)`, Slice `len`/`is_empty`/indexing/indexed assignment/range slicing, mutable local array/Vec `as_slice()` view creation, assertion function and macro helpers, escaped braces | bad placeholder count, nonliteral format, planned `format!`, freestanding line input, bad assert type, bad assert macro type, bad `matches!` arity/patterns, removed `Maybe[T]` alias | stdio, getchar/fgets, syscall byte IO, assertion exits, pattern-engine macro lowering | partial |
| Context runtime | `context::argc`, `context::arg`, aliases, `@ari_entry`/`ari::main` host wrapper | out-of-range behavior, shared-library context use | host runtime init | partial |
| C FFI | libc calls, aliases, module externs, concrete C ABI bindings only, C ABI type aliases, C varargs with default promotions, C callback/function-pointer params, helper library, `ref mut` pointers, `ptr c_char` strings, `@repr(C)` structs with value/raw-pointer/borrow fields, generic `@repr(C)` structs with value and pointer-sized generic fields, generic fieldless `@repr(C)` enums, `null` raw-pointer literals, nullable `T?` raw-pointer type spelling, raw-pointer casts, byte-wise `ptr_offset` with inferred or explicit `<T>`, typed scalar/aggregate `ptr_add` with inferred or explicit `<T>`, natural Ari aggregate `size_of<T>()` / `align_of<T>()`, scalar/plain-aggregate `ptr_load`/`ptr_store` with inferred or explicit `<T>`, `f32`/`f64` raw-pointer load/store bit copies, scalar/plain-aggregate `*pointer` dereference load/store, scalar struct/tuple/fixed-array field and element access through raw pointers, scalar/raw-pointer exported C header prototypes, public non-generic `@repr(C)` struct declarations with `ref` as `const` C pointers, by-value public non-generic `@repr(C)` struct export prototypes for direct 64-bit Unix aggregate ABI values, opaque public generic `@repr(C)` struct typedefs for pointer-only APIs, concrete public generic `@repr(C)` struct typedefs for by-value exported instantiations, and public generic/non-generic fieldless enum tag typedefs in C headers, `c_void` returns | rejected ABI, body, generic extern declarations, bad link symbol, by-value `c_void` param, non-extern varargs, unsupported aggregate varargs, variadic extern function pointer values, `@repr(C)` owning fields, payload-bearing `@repr(C)` enums, oversized or non-Unix by-value C-header aggregates, null used as non-pointer, nullable `?` combined with `own`/`ref`/`ptr`, non-pointer value initializers for `T?`, bad layout query arity, dereference of non-pointer values, whole raw-pointer copies of ownership/borrow-valued aggregates, freestanding C extern calls, C header prototypes for Ari-only/unsupported aggregate values | LLVM declarations, explicit IR ABI split for Ari builtins vs C externs, function pointer params as `ptr`, vararg bool/narrow-int/f32 promotion casts, `ptrtoint`/`inttoptr`, `getelementptr i8`, typed scalar and aggregate pointer GEP, scalar and natural aggregate layout literals, scalar and plain-aggregate raw pointer load/store, float raw-pointer bit load/store, pointer dereference load/store, aggregate pointer GEP, raw backend natural byte field/array addressing, raw backend address add, raw backend explicit C extern rejection, host linker, and C header emitter | good first pass |
| Explicit memory zones | `Zone`, `zone::create`, lexical `zone::temp`, raw `zone::alloc`, typed `zone::alloc<T>`, placement `zone::new<T>`, local scratch `zone::scratch<T>`, explicit scratch promotion `zone::promote<T>`, zone-backed associated `T::new(ref mut Zone, ...)`, `zone::reset`, `zone::destroy`, raw `ptr u8` and typed `ptr T` allocation used with layout queries and pointer load/store | missing destroy for non-temporary `own Zone`, rejected `drop zone`, rejected movement of temporary zones, rejected temporary-zone pointer escape through returns, outer bindings, aggregates, or call arguments, `zone::scratch<T>` rejected outside local pointer binding initializers, bad typed allocation arity, zero-sized typed allocation, bad placement-construction/promotion arity, ownership/borrow-valued placement/promotion rejection, direct local and single-zone wrapper zone-pointer use after reset/destroy, reset invalidation after `if`/`match`/loop joins, aggregate/FFI/multi-zone zone-pointer escape rejection, freestanding zone allocation rejection | LLVM host runtime backed by `malloc`/`free`, ownership move into `zone::destroy`, automatic destroy insertion for temporary-zone scope fallthrough, returns, and escaping `break`/`continue`/labeled-block exits, `zone::scratch<T>` lowered to hidden `zone::temp` plus placement `zone::new<T>`, `zone::promote<T>` lowered to pointer load plus placement `zone::new<T>` in the target zone, typed allocation lowered to raw allocation with compile-time layout, placement construction lowered to allocation plus store, inherent associated call to zone placement, direct and single-zone wrapper pointer provenance invalidation with named temporary-zone escape diagnostics, control-flow zone generation merge, deliberate host-only zone policy | good first pass |
| Shared libraries | compile library, Ari-mangled symbols, `@export`, `@export("symbol")`, `@no_mangle`, `--shared` public/default and private/hidden LLVM visibility, raw ELF export/no-mangle symbol table names | main not required only with `--shared`, invalid export symbols, export-vs-mangled symbol collisions | dynamic symbol table and raw static symbol table | good first pass |
| Comments | line and nested block comments | unterminated block comment | lexer | partial |
| Attributes and meta | built-in attributes, `@repr(C)` layout guards including borrow-field, value/pointer-sized generic slots, generic fieldless enums, and emitted public non-generic struct with const-ref header slots, direct by-value struct header ABI guards, opaque and concrete generic-struct typedefs, plus generic/non-generic fieldless-enum declarations in C headers, `@cfg(true/false)` declaration pruning, boolean/target/feature cfg predicates, command-line cfg feature flags, deprecated use warnings, `@test` runner generation, `@export`/`@no_mangle` symbol controls, exported C header prototypes, user-reserved attribute names via concrete non-generic `meta fn`, meta signatures over `token_stream`/`ast`/`type` | unknown attributes, bad attribute placement, bad `repr`, `@repr(C)` owning fields, payload-bearing `@repr(C)` enums, oversized or non-Unix by-value C-header aggregates, bad `@cfg` predicate, bad deprecated/export arguments, bad export symbol collisions, bad `@test` signature, bad meta signatures, generic meta functions, planned macro invocation | parser pruning, sema validation, warning emission, freestanding test runner, LLVM/shared/raw symbol selection, C header emitter | good first pass |
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
- [x] `--freestanding` rejects direct `extern "C"` calls instead of treating
      them as missing Ari symbols
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
- [x] emit hidden LLVM visibility for private Ari helpers under `--shared`
- [x] inspect linked shared-library dynamic symbols for public/exported-only
      user ABI surface
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
- [x] support freestanding direct function returns for tuple, struct,
      fixed-array, and aggregate enum values through hidden result pointers
- [x] support freestanding direct and function-pointer call parameters and
      returns for tuple, struct, fixed-array, and aggregate enum values through
      hidden argument/result pointers copied into callee-local storage or
      caller-provided result storage
- [x] materialize discarded freestanding aggregate-returning direct and
      function-pointer calls into hidden temporaries
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
- [x] reject argument-based source-level function overloading by arity or
      parameter type
- [x] allow explicit generic companions such as `alloc<T>(...)` beside a
      concrete same-path function
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
- [x] aggregate-valued `if`, `match`, and block expression results on the
      freestanding backend
- [x] integer-to-bool condition coercion as `value != 0`
- [x] `if let EnumCase(payload) = value`
- [x] `while let EnumCase(payload) = value`
- [x] `for value in range(start, end)`
- [x] `for value in start..end`
- [x] `for value in start..=end`
- [x] `for value in range_inclusive(start, end)`
- [x] integer `Range[T]` and `RangeInclusive[T]` local values with field and tuple-index access
- [x] `for value in stored_range`
- [x] irrefutable alias loop heads for range calls, range operators, and stored range values
- [x] `for _ in range(start, end)` wildcard pattern
- [x] empty ranges do not run the loop body
- [x] `iter::range(start, end)` qualified prelude form
- [x] `for value in [a, b, c]` list literal iteration
- [x] `for value in stored_vec` local vector iteration on the LLVM backend,
      using the current vector length rather than reserved capacity
- [x] `for _ in [a, b, c]` list literal wildcard pattern
- [x] irrefutable tuple, named struct, tuple-struct, and alias loop heads for list literal iteration
- [x] irrefutable tuple, named struct, tuple-struct, and alias loop heads for stored local vector iteration
- [x] list literal `for` with `break` and `continue`
- [x] stack-backed local `Vec[T]` literals on the LLVM backend
- [x] typed empty local `Vec[T]` literals through annotations and assignments
- [x] checked dynamic indexing of stored local `Vec[T]`, including
      compiler-known current-length static index diagnostics and known-empty
      dynamic index rejection, plus direct control-flow Vec expression static
      index diagnostics from source-known local branch lengths
- [x] local `Vec[T]` literal reassignment with changing runtime length
- [x] local `Vec[T]` initialization and assignment from another local vector
      preserve compiler-known current length when the source length is precise
- [x] local `Vec[T]` assignment from another local vector widens the target's
      fixed local storage to the source storage capacity
- [x] local `Vec[T]` assignment/initialization from Vec-valued `if`, block,
      and labeled-block expressions preserves nested fixed storage capacity and
      same-length literal, block, typed-break, or local-binding known length
- [x] local `Vec[T]` initialization from Vec-valued `match` and `if let`
      expressions preserves fixed storage capacity and same-length literal,
      block, labeled-block, typed-break, or local-binding known length across
      branch result materialization
- [x] `len(value)` and `value.len()` for local vectors, fixed arrays,
      constant-folded literal length, and compiler-known local Vec length,
      including local Vec `as_slice()` view length construction and
      source-known Vec control-flow expression result lengths
- [x] fixed-capacity local literal/const/static-signed-expr/known-local/runtime-checked `Vec.reserve(n)`, plus auto-widening local `Vec.push(value)` on the LLVM backend
- [x] checked local `Vec.insert(index, value)` on the LLVM backend
- [x] checked local `Vec.pop()` on the LLVM backend
- [x] checked local `Vec.remove(index)` on the LLVM backend, including
      known-empty runtime-index rejection
- [x] checked local `Vec.first()` and `Vec.last()` on the LLVM backend
- [x] checked local `Vec.get(index)` on the LLVM backend, including known-empty
      runtime-index rejection
- [x] checked local `Vec.contains(value)`, `Vec.index_of(value)`, and `Vec.count(value)` on the LLVM backend
- [x] fixed-capacity local `Vec.capacity()` on the LLVM backend
- [x] `value.is_empty()` for local vectors, fixed arrays, constant-folded
      literals, and compiler-known local Vec length
- [x] fixed-capacity local `Vec.clear()` on the LLVM backend
- [x] fixed-capacity local `Vec.truncate(n)` on the LLVM backend, including static/known-local length tracking for later local growth
- [x] checked local `Vec.set(index, value)` on the LLVM backend, including
      known-empty runtime-index rejection
- [x] checked local `Vec.swap(a, b)` on the LLVM backend, including known-empty
      runtime-index rejection
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
- [x] lower direct copyable `Iterator[T]` for-loop values through `next`
- [x] lower direct copyable `Iterator[T]` for-loop values whose `next` receiver
      is `self: ref mut Self`
- [x] lower explicit `ref mut` `Iterator[T]` for-loop values by advancing the
      original iterator binding and releasing the hidden borrow after the loop
- [x] lower self-returning copyable `IntoIterator[T]` for-loop values through
      `into_iter` and `next`
- [x] lower distinct-result copyable `IntoIterator[T]` for-loop values when
      `into_iter` returns another `Iterator[T]`
- [x] lower copyable `IntoIterator[T]` for-loop values whose `into_iter`
      receiver is `self: ref mut Self`
- [x] lower owning `Iterator[T]` values by moving them into hidden loop storage,
      calling `next(self: ref mut Self)`, and dropping the hidden owner at loop
      exit
- [x] drop hidden owning iterator storage before `return` exits a function from
      inside the loop
- [x] drop hidden owning iterator storage before postfix `?` residual returns
      from inside the loop
- [x] lower refutable scalar literal/range, or-pattern, fieldless enum-case,
      compact enum payload, homogeneous nested aggregate-enum payload, and
      nested aggregate-enum multi-payload item patterns with nested
      or-pattern slots for `Iterator[T]` loops with stop-on-first-mismatch
      semantics
- [x] lower `for let pattern in iterator` as a filter loop that skips
      non-matching `Some(_)` items
- [x] reject `for let` filter syntax on range/list/vector loops until those
      collections lower through `Iterator[T]`
- [x] reject unsupported iterator for-loop shapes with trait-aware planned
      diagnostics
- [x] reject enum-case list/vector `for` patterns with a planned diagnostic
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
- [x] scalar constant initializer arithmetic, bitwise/shift, bool, and comparison folding
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
      search paths, cfg features, selected target triples, implicit `std`,
      source hashes, and import resolution
- [x] validated source-snapshot module cache use resolves module imports from
      the cached import table instead of rediscovering candidate files
- [x] source-snapshot module cache use rechecks the parsed cached source graph
      against the cache's embedded metadata
- [x] source-snapshot module cache parsing rejects duplicate source snapshot records
- [x] source-snapshot module cache emits and reloads per-source AST summary records
- [x] AST summary records include declaration fingerprints and declaration
      payloads, then reject tampered fingerprints, payloads, or count/payload
      disagreement
- [x] AST summary declaration payloads materialize back into declaration-only
      `Program` records and round-trip before cache use
- [x] AST summary declaration payloads materialize scalar constant
      initializers for header-like cached modules
- [x] AST summary declaration payloads materialize tuple, struct, fixed-array,
      enum, and generic enum constant initializers for header-like cached modules
- [x] AST summary declaration payloads preserve explicit casts plus arithmetic,
      bitwise, unary bitwise-not, and shift constant expressions for
      header-like cached modules
- [x] AST summary declaration payloads preserve field, tuple-index, and
      fixed-array index access over materialized aggregate constants
- [x] cached header-like dependencies can load declaration-safe materialized
      summaries instead of reparsing cached source text
- [ ] AST/IR package cache summaries skip dependency parsing after validation
- [x] AST/IR scalar payload packing preserves integer, bool, float, tuple-index,
      and indexed-assignment lowering paths
- [x] parser and sema share union-safe AST expression clone helpers for
      compound assignment targets and synthetic borrow receivers
- [x] AST pattern literal payload packing preserves integer, bool, signed range,
      or-pattern, alias, and product-pattern match lowering paths
- [x] sema iterator filter rewrites reuse the shared union-safe pattern clone
      helper
- [x] IR match-arm and nested enum-payload literal packing preserves scalar
      match lowering, aggregate enum nested-payload tests, and raw/LLVM backends
- [x] AST expression builders preserve compound-assignment name cloning,
      matches! bool arms, tuple-index borrowing, and negative scalar literals
- [x] AST composite builders preserve string/null, tuple/vector/struct literal,
      labeled-block, and match expression lowering paths
- [x] IR literal builders preserve integer sign, float, string, bool, null,
      pointer fallback defaults, product-match fallback defaults, and tuple
      default lowering paths
- [x] IR access builders preserve struct field assignment/access, tuple-index
      reads, fixed-array static/dynamic indexing, and destructor call lowering
- [x] IR aggregate builders preserve tuple, struct, tuple-struct, expected
      aggregate, and range constructor lowering paths
- [x] IR call builders preserve direct, generic, inherent associated, trait
      associated, trait-qualified method, method, and zone helper call lowering
      paths
- [x] IR control-flow builders preserve aggregate/product if-let and match
      expression chain lowering paths
- [x] rare AST/IR vector payload packing preserves macro token parsing and
      format-print string-part lowering paths
- [x] AST condition-pattern payload packing preserves enum and aggregate
      if-let/while-let statement and expression lowering paths
- [x] AST for-loop pattern payload packing preserves range, list literal,
      stored-vector, and iterator filter lowering paths
- [x] AST/IR match statement arm payload packing preserves scalar, enum,
      aggregate, while-let, LLVM, and freestanding lowering paths
- [x] AST/IR explicit-drop name payload packing preserves explicit destructor
      lowering on LLVM and freestanding paths
- [x] AST/IR break statement payload packing preserves unlabeled break,
      labeled-loop break, and labeled-block value break lowering paths
- [x] AST/IR assignment statement payload packing preserves direct binding,
      compound, field, and indexed assignment lowering paths
- [x] AST/IR statement label payload packing preserves labeled block, loop,
      and range-for break lowering paths
- [x] AST/IR statement body-vector payload packing preserves block, if/else,
      while, range-for, init-while, and tuple-match if-chain lowering paths
- [x] AST/IR block-expression payload packing preserves nested block values,
      labeled block-expression breaks, and raw/LLVM lowering paths
- [x] AST/IR if-expression payload packing preserves plain `if`, `else if`,
      and expression-valued `if let` lowering paths
- [x] AST/IR match-expression payload packing preserves enum, integer, and
      bool expression-valued match lowering paths
- [x] IR trait-object call parameter payload packing preserves multi-argument
      LLVM dyn dispatch
- [x] AST receiver type-argument payload packing preserves trait-qualified
      associated calls with explicit and inferred result type arguments
- [x] AST struct-literal field-name payload packing preserves out-of-order
      generic struct literal mapping for constants and locals
- [x] AST direct type-argument payload packing preserves explicit generic
      function calls, generic struct literals, and generic enum constructors
- [x] AST argument-vector payload packing preserves tuple, vector, struct
      literal, and call argument lowering on LLVM and freestanding paths
- [x] IR argument-vector payload packing preserves tuple, vector, struct literal,
      call, and stored-vector for-loop lowering on LLVM and freestanding paths
- [x] AST operand child payload packing preserves unary, binary, cast, try,
      tuple-index, field, index, call, compound-assignment clone, and synthetic
      borrow receiver lowering on LLVM and freestanding paths, including
      method-call receiver lowering through sema helper accessors
- [x] IR operand child payload packing preserves pointer casts/load/store/
      addition, pointer dereference assignment, indirect calls, and binary
      lowering on LLVM and freestanding paths
- [x] IR rare expression payload packing preserves compact enum constructors
      and vector set/swap/insert/search side-input lowering paths
- [x] IR string payload packing preserves string literals, local loads,
      borrows, direct calls, function pointer refs, and indirect calls through
      `ir_builders`, sema, and LLVM lowering, plus trait-object cast/call
      builder paths
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
- [x] reserve `Iterator[T]` with `next(self: ref mut Self) -> Option[T]`
- [x] keep value-self `next(self) -> Option[T]` impls accepted as a copyable
      compatibility subset
- [x] lower direct copyable `Iterator[T]` `for` loops through `next`
- [x] lower direct `ref mut` `Iterator[T]` `for` loops through `next`
- [x] reserve `IntoIterator[T]` with
      `into_iter(self: ref mut Self) -> Self`
- [x] keep value-self `into_iter(self)` impls accepted as a copyable
      compatibility subset
- [x] lower self-returning copyable `IntoIterator[T]` `for` loops through
      `into_iter` and `next`
- [x] allow `IntoIterator[T].into_iter` impls to return a distinct iterator
      type and validate that result at `for` lowering sites
- [x] lower generic `IntoIterator[T]` impls that return a distinct generic
      iterator type
- [x] reject `IntoIterator[T].into_iter` impls whose concrete result does not
      implement `Iterator[T]`
- [x] keep generic direct `Iterator[T]` impls valid while the
      `IntoIterator[T]` result contract pass scans generic impl headers
- [x] reject generic `IntoIterator[T]` impls whose placeholder result type does
      not implement `Iterator[T]`
- [x] lower iterator item literal/range/or, fieldless enum-case, compact enum
      payload, homogeneous nested aggregate-enum payload, and nested
      aggregate-enum multi-payload patterns with nested or-pattern slots
      through the `while let Some(pattern) = next()` path
- [x] reserve `iter::Iterable[T]`
- [x] reserve `iter::Iterator[T]`
- [x] reserve `iter::IntoIterator[T]`
- [x] accept impls against prelude iterator traits
- [x] reject unknown trait impls
- [x] reject wrong trait type-argument counts
- [x] reject duplicate trait impls for the same trait/type pair
- [x] validate trait impl method completeness and signatures
- [x] parse `trait Child: Parent` supertraits and require matching
      supertrait impls for the same receiver type
- [x] allow generic bounds on a child trait to statically dispatch methods
      declared by its supertraits
- [x] lower trait-qualified calls such as `Trait::method(receiver, ...)` and
      `Trait<T>::method(receiver, ...)` for explicit static dispatch
- [x] lower trait-qualified associated function calls such as
      `Trait::make<SelfType>(...)` and `Trait<T>::make<SelfType>(...)`, plus
      expected-result forms such as `let x: SelfType = Trait<T>::make(...)`
      in control-flow result arms, aggregate literal elements, and generic enum
      constructor payloads, including primitive `Self` impls, to disambiguate
      same-named trait associated functions
- [x] include object-safe supertrait methods in LLVM `dyn Child` vtables and
      reject ambiguous inherited dyn method names
- [x] allow LLVM dyn-to-dyn upcasts from a child trait object to the same trait
      or a supertrait, and reject unrelated dyn-to-dyn casts
- [x] accept bare `self` in trait/impl method signatures as `Self`
- [x] lower concrete `value.method(...)` calls through matching impl methods
- [x] reject unknown and ambiguous method calls
- [x] expose source `Option[T]` and `Result[T, E]` through the source prelude
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
