# Completed Milestones

This page keeps completed roadmap history out of the active roadmap. It is not
a second task list; use [Roadmap](roadmap.md) for unfinished work and
[Feature Test Matrix](test-matrix.md) for coverage.

## Front End And Tooling

- Parser-visible syntax is stable for linting and language-server tooling.
  `init ... while ... next` is the loop-state spelling, and old `let ... while`
  syntax is rejected with a migration diagnostic.
- `--check` runs parsing, module loading, and semantic lowering without backend
  emission for editor tooling.
- Mutable borrow type spellings accept both `ref mut T` and `mut ref T`.
- User-defined `meta fn` macro syntax covers expression, item, type, pattern,
  and attribute-position `ident!(...)` expansion for the documented bounded
  token-stream and explicit AST-constructor subset.
- Declaration-returning `ast -> ast` macros can inspect generic, parameter,
  field, enum-case, method, associated-type, trait, return, and witness
  summaries. `meta_ident!(...)` inside `decl!(...)` supports generated
  declaration names from declaration metadata plus literal pieces.
- Expression-position `ast -> ast` macros can branch on input expression kind
  through compile-time-only helpers. Pattern and type AST inputs intentionally
  do not expose general compile-time values yet.

## Patterns And Control Flow

- Declaration-level `let mut PATTERN = value` parses as mutability for every
  binding introduced by the pattern and reuses the `var PATTERN` lowering path.
- `let ref PATTERN = value` and `let ref mut PATTERN = value` introduce borrow
  bindings over tracked local/path initializers and tuple, fixed-array, and
  struct destructuring.
- `let &PATTERN = value`, `let &mut PATTERN = value`, `&PATTERN: T`, and
  `&mut PATTERN: T` are shorthand for the same local and function-parameter
  reference-pattern binding modes.
- Direct local `Vec[T]` storage and `Slice[T]` view reference patterns support
  runtime length guards, plain prefix/suffix element borrows, and `name @ ..`
  rest bindings that produce `Slice[T]` views.
- Shared `Slice[T]` reference sequence patterns can destructure tuple,
  fixed-array, and struct elements through the same access-path helper used by
  local aggregate reference patterns.
- Dynamic runtime-sequence reference patterns keep distinct element borrow paths
  for nested tuple, fixed-array, and struct subpatterns, so `ref mut` nested
  element borrows can coexist when the sequence length guard proves the matched
  elements are distinct.
- Runtime-sequence pattern irrefutability checks live in `pattern_semantics`,
  keeping `sema.cpp` focused on lowering length guards and element bindings.
- Enum-case reference patterns borrow addressable aggregate enum payload slots,
  including 64-bit payload-word slots and nested aggregate-enum payload slots.
- Compact and otherwise non-addressable enum payload reference patterns are
  rejected with payload-specific value-only diagnostics instead of falling
  through to a vague layout error.
- Function parameter reference patterns lower as function-entry borrows from
  hidden ABI parameter storage.
- `[ ... ]` patterns work for compile-time fixed arrays and runtime sequence
  subjects. Local `Vec[T]` storage and `Slice[T]` views share length guards,
  indexed element bindings, and `name @ ..` Slice rest binding where supported.
- Product-pattern lowering is shared across tuple, fixed-array, named-struct,
  tuple-struct, `let`/`var`, match, aggregate `if let`, aggregate `while let`,
  and supported function parameter patterns.
- Loop fixed-point checking tracks ownership and borrow states at `break`,
  `continue`, zero-iteration, literal-true next-iteration, and fallthrough
  merge points. Ambiguous loop exits produce explicit `maybe-unavailable`
  owner states instead of pretending the owner is live or unavailable.

## Traits And Dispatch

- Supertraits parse and require matching impls.
- Child-trait bounds can statically dispatch inherited methods.
- Trait-qualified associated function calls resolve through explicit or
  expected-result implementing types.
- Associated type projections resolve through unique impl witnesses and unique
  generic supertrait applications.
- LLVM `dyn Child` trait objects include inherited object-safe
  methods and dyn-to-supertrait upcasts.
- Ambiguous associated functions, inherited methods, inherited associated type
  names, and unrelated dyn upcasts are rejected with focused diagnostics.

## Source Std And Memory

- Source `std` foundations are in place under `lib/std/`, guarded by
  `make check-std-api`.
- Source Box, String, Vec, Slice, Option, Result, cmp, mem, fmt, IO, iterator,
  and explicit-zone contracts are covered by tests and language docs.
- Bare `Vec[T]` remains fixed local storage plus Slice-shaped direct and
  function-pointer parameter ABI. `std::Vec[T]` is a public alias for the
  explicit-zone `std::vec::Vec[T]` source handle, and
  `Vec!(T, ref mut Zone, capacity)` is constructor sugar for
  `std::vec::new<T>`.
- Source Vec and String growth can infer the tracked receiver's source zone for
  common same-zone operations without adding an ambient heap.
- Host zone allocation metadata is fixed for the 0.x ABI: every non-empty
  `zone::alloc` payload has an 8-byte header immediately before the user
  pointer, storing only the raw zone handle at `ptr - 8`. Size and requested
  alignment are not pointer metadata.
- `zone::allocation_zone` exposes the allocation's zone handle as a narrow
  builtin. Zone allocation is implemented on the LLVM runtime path through
  explicit `malloc`/`free` calls and the fixed 8-byte zone-handle header.

## Modules And Cache

- Module cache metadata, source-snapshot cache records, and AST declaration
  summaries use the shared V0 cache-format contract.
- V0 IR package-cache replay materializes lowered statement/expression body
  trees into pre-lowered `IrFunction` bodies for summary-safe dependencies.
- Cache replay can skip semantic body lowering for validated dependency
  functions, append replayed bodies in normal sema order, and report malformed
  replay payloads before backend emission.
- V0 IR sidecars record stable specialization metadata for replayed generic
  free-function and generated impl-method specializations. Cache-use semantic
  checking recognizes those lowered names and does not re-lower duplicate
  specializations before the replayed bodies are appended. Cache loading also
  validates specialization origins and generic argument names against the
  cached AST surface before replay.
- V0 layout descriptors live in the existing cache family. Current descriptors
  cover cache-only local `Vec[T; capacity]` storage as `vector-storage`
  entries, and cache-use tests compare fresh/cache LLVM output byte-for-byte
  for that metadata.

## ABI, Layout, And Backends

- Non-local aggregate ABI classification is shared in `src/aggregate_abi.*`.
  It recognizes value tuples, fixed arrays, structs, fixed-capacity vector
  storage values, and aggregate-layout enums, and classifies 64-bit Unix
  values as direct when they are non-zero, at most 16 bytes, and at most
  8-byte aligned. Larger/aligned values are indirect, and unsupported
  target/layout/zero-sized cases are reported explicitly. C-header emission
  uses this shared classifier before rendering exported by-value prototypes.
- Direct imported C aggregate calls use the shared classifier for by-value
  `extern "C"` parameters and returns. The first supported import surface is
  classifier-approved `@repr(C)` structs on 64-bit Unix targets; larger,
  target-unsupported, and non-`repr(C)` aggregate spellings are rejected with
  pointer-ABI diagnostics.
- Direct fixed-array exports are implemented for the current 0.x shared LLVM
  surface, including fixed-array fields in public `@repr(C)` structs,
  pointer-to-array parameters, and by-value `[T, N]` exported
  parameters/returns through generated wrapper typedefs.
- Generated C-header wrappers expose classifier-approved Ari-only aggregate
  exports without changing the LLVM ABI: tuples use `AriTuple_*` field wrappers,
  fixed-capacity vector storage values use `AriVec_*` `len`/`data[N]` wrappers,
  and aggregate-layout enums use `AriEnum_*` `tag`/`payloadN` storage wrappers.
- `src/layout.cpp` owns shared aggregate-layout predicates, field-list
  selection, field counts, byte sizes, alignments, and field offsets used by
  sema, the LLVM backend, and related IR helpers.
- Aggregate enum payload slots support integer, bool, pointer-shaped values,
  one-word enums, nested aggregate enums, and the current mixed payload-word
  plus nested-enum lane rule.
- Aggregate enum payload slot access through `value.0` and `(*raw_enum).0`
  addresses payload slot 0 rather than the hidden tag field on local and
  raw-pointer-backed ABI paths.
- The LLVM backend materializes supported direct aggregate enum values into
  hidden stack storage before reading tags or payload slots.
- The LLVM backend lowers lowercase `string` literals into a per-image static
  NUL-terminated byte pool.
- LLVM trait-object dispatch works for the copyable non-borrow dyn surface,
  including concrete impls, generic impl specializations, multi-arg dyn calls,
  Vec-shaped aggregate argument views, inherited object-safe supertrait
  methods, and dyn-to-supertrait vtable upcasts.
- LLVM `f32`/`f64` local storage, raw pointer load/store/dereference,
  arithmetic, ordered comparisons, and direct Ari calls are implemented.
- Ari now has a single LLVM-backed codegen path for executables, shared
  libraries, LLVM IR, and object output.
- The LLVM backend can emit LLVM-driver relocatable object files through
  `--emit-obj path`. These objects contain a `.text` section and
  a symbol table using the same Ari mangled or explicit `@export`/`@no_mangle`
  symbols as LLVM executable output.
- LLVM relocatable object output supports direct imported `extern "C"` calls for
  integer, bool, string/function-pointer, raw-pointer/reference, and void-return
  scalar signatures by emitting undefined C symbols plus `R_X86_64_PLT32`
  `.rela.text` relocations. LLVM executable output still rejects imported C
  symbols because it has no linker phase.
