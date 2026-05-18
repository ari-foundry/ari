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
- Built-in attribute-name classification, built-in attribute target/argument
  validation, and declaration-level `@repr(C)` field/case guards live in
  `attribute_semantics`, keeping those front-end validation rules out of the
  central semantic coordinator.
- Public `@repr(C)` record and enum C-header metadata assembly lives in
  `c_export_semantics`; sema still owns declaration traversal, type resolution,
  and enum-layout lookup for that export phase.
- Ari builtin metadata now records expected source signatures for
  `extern "ari"` declarations, and sema rejects wrong arity, parameter types,
  or return types before any builtin call reaches IR lowering.
- User-defined `meta fn` macro syntax covers expression, item, type, pattern,
  and attribute-position `ident!(...)` expansion for the documented bounded
  token-stream and explicit AST-constructor subset.
- Pattern-position macro output feeds the shared pattern lowering path after
  expansion, including match-arm or-pattern normalization, coverage, payload
  binding, and reference-binding detection.
- Pattern or-detection and expanded alternative lists share one
  `pattern_semantics` result object, so sema keeps match/control-flow lowering
  wired to a phase-level pattern API instead of syntax-local helper pairs.
- Root `Vec[T]` function, method, trait-method, and function-pointer parameter
  ABI lowering plus impl receiver view lowering is owned by `vector_semantics`:
  parameters and receivers lower to the Slice-shaped view ABI. Bare root
  `Vec[T]` remains a local-storage and borrowed-view type; non-local ownership
  contexts such as returns, struct fields, trait-method returns, and extern C
  signatures produce a dedicated policy diagnostic instead of implying a hidden
  heap.
- Raw pointer helpers, layout queries, and `std::mem` value helpers lower
  through `pointer_memory_semantics`, keeping type-argument checks,
  mutable-place validation, hidden temporary locals, and final pointer IR block
  assembly out of central expression dispatch.
- Typed zone allocation, placement construction, promotion, and lexical
  temporary-zone calls lower through `zone_allocation_semantics`, so layout
  checks and runtime allocation-call assembly are kept out of central
  expression dispatch.
- Phase-oriented sema decomposition is no longer tracked as a finite near-term
  roadmap item. Ongoing extraction policy and candidate boundaries live in
  `docs/dev/sema-decomposition.md` after the attribute, C export, vector,
  pointer/memory, zone allocation, ownership, pattern, and IR helper phases
  were split out far enough for roadmap cleanup.
- The stack-backed local root `Vec[T]` method surface includes `as_ptr()` for
  raw element-buffer access alongside the fixed-capacity read/search/mutation
  helpers.
- Source `std::Vec[T]`/`std::vec::Vec[T]` is the supported growable collection
  handle for library-facing ownership. It uses explicit `Zone` capabilities for
  allocation, same-zone growth, `Vec!` construction sugar, drop of live elements,
  tracked `Slice` views, element borrows, and raw element-buffer views. Source
  `Box[T]`/`std::Box[T]` follows the same explicit-zone handle policy for single
  values through `Box!(T, ref mut Zone, value)`, `as_ref()` / `as_mut()`, and raw
  pointer views.
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
- Local/path `let ref` and `let ref mut` patterns can destructure
  ownership-carrying tuple, fixed-array, and struct values. Owned fields may be
  skipped, and borrowed owned fields still use the tracked owned-field liveness
  checks before the borrow binding is introduced.
- Exact local `Vec[T]` reference patterns without `..` can borrow
  ownership-carrying element slots, including nested owned fields inside
  aggregate elements, when each element path is statically known; moved element
  slots still report the owned-field liveness diagnostic.
- Local `Vec[T]` reference patterns with `..` can borrow ownership-carrying
  prefix elements, and suffix elements when the direct local vector's current
  length is known, as long as the pattern does not bind the rest as a Slice.
- Unknown-length local `let ref` and `let ref mut` suffix patterns over direct
  `Vec[own T]` storage use synthetic suffix owner paths after proving all
  tracked owned elements are still live. These dynamic suffix borrows allow
  distinct prefix/suffix bindings such as `let ref mut [first, .., last]` while
  still blocking whole-vector mutation and drop until the borrow bindings leave
  scope.
- Shared `Slice[T]` reference sequence patterns can destructure tuple,
  fixed-array, and struct elements through the same access-path helper used by
  local aggregate reference patterns.
- Dynamic runtime-sequence reference patterns keep distinct element borrow paths
  for nested tuple, fixed-array, and struct subpatterns, so `ref mut` nested
  element borrows can coexist when the sequence length guard proves the matched
  elements are distinct.
- Runtime-sequence pattern irrefutability checks live in `pattern_semantics`,
  keeping `sema.cpp` focused on lowering length guards and element bindings.
- Runtime-sequence reference pattern planning also lives in
  `pattern_semantics`, including direct rest-alias constraints,
  ownership-carrying rest-alias rejection, known-length owner suffix paths, and
  synthetic dynamic owner suffix planning before sema emits shared or mutable
  borrow bindings.
- Enum-case reference patterns borrow addressable aggregate enum payload slots,
  including 64-bit payload-word slots and nested aggregate-enum payload slots.
- Enum statement/expression `match`, enum `if let`, and enum `while let` can
  use mutable reference payload patterns when the matched enum subject is an
  addressable local, field, or indexed element. Control-flow lowering still
  uses hidden value storage for tag tests while the borrow binding points at
  the original subject.
- Tuple, fixed-array, and struct statement/expression `match`, aggregate
  `if let`, and aggregate `while let` can use `ref mut` field patterns when
  the matched subject is an addressable local, field, or indexed element.
  Pattern tests still use hidden product storage while mutable field borrows
  point at the original subject.
- Runtime-sequence statement/expression `match`, `if let`, and `while let`
  patterns over `Vec[T]` storage and `Slice[T]` views can use `ref mut`
  element patterns when the matched subject is addressable. Length and element
  tests still use hidden sequence storage while mutable element borrows point
  at the original subject.
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
- Local and function-parameter value patterns can move ownership-carrying
  tuple, fixed-array, struct, and tuple-struct slots into bindings from tracked
  hidden storage. The binding engine marks the moved owner subpaths so skipped
  live owned fields still follow normal cleanup.
- Local `Vec[own T]` value patterns can move exact element bindings and
  known-length suffix element bindings after `..` from tracked hidden Vec
  storage. Selected `_` elements and known skipped rest-gap elements are lowered
  as explicit element drops, so hidden Vec storage has no leaked live owner
  slots after the pattern body. Known-length owned rest aliases lower to
  non-owning `Slice[own T]` views; the source Vec is borrowed while the view is
  live, and compiler-owned hidden pattern storage drops any still-owned slots at
  scope exit.
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
- `Box!(T, ref mut Zone, value)` is constructor sugar for the explicit-zone
  `std::boxed::new<T>` source handle.
- Source `Box[T]` exposes `as_ref()` and `as_mut()` borrowed value views in
  addition to tracked raw pointer views.
- Source `std::vec::Vec[T]` exposes tracked `as_ptr()` and `as_mut_ptr()`
  raw element-buffer views.
- Source `std::vec::Vec[T]` exposes `get_ref(index)` and `get_mut(index)`
  element borrow views over the zone-backed buffer.
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
  summaries use the shared `v0` cache-format contract.
- `v0` IR package-cache replay materializes lowered statement/expression body
  trees into pre-lowered `IrFunction` bodies for summary-safe dependencies.
- Cache replay can skip semantic body lowering for validated dependency
  functions, append replayed bodies in normal sema order, and report malformed
  replay payloads before backend emission.
- `v0` IR sidecars record stable specialization metadata for replayed generic
  free-function and generated impl-method specializations. Cache-use semantic
  checking recognizes those lowered names and does not re-lower duplicate
  specializations before the replayed bodies are appended. Cache loading also
  validates specialization origins, generic argument names, and locally known
  trait-vs-inherent impl origin keys against the cached AST surface before
  replay. Replayed IR bodies also validate cached `impl::...` call targets
  against the functions carried by the same sidecar, so corrupted body call
  identities fail during module-cache loading instead of reaching backend
  symbol lookup.
- `v0` layout descriptors live in the existing cache family. Current descriptors
  cover fixed-capacity `Vec[T; capacity]` storage as `vector-storage`
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
- Public non-generic payload-bearing `@repr(C)` enums are emitted in C headers
  as named tagged structs with `int32_t tag` plus payload slots. Scalar and
  pointer-shaped payloads use raw `uint64_t payloadN` slots, while non-scalar
  plain aggregate payloads use generated wrapper typedefs such as
  `AriTuple_*`. Direct-ABI exported functions use the public enum type name
  instead of a generated `AriEnum_*` wrapper.
- `src/layout.cpp` owns shared aggregate-layout predicates, field-list
  selection, field counts, byte sizes, alignments, and field offsets used by
  sema, the LLVM backend, and related IR helpers.
- Aggregate enum payload slots support integer, bool, pointer-shaped values,
  one-word enums, nested aggregate enums, and the current mixed payload-word
  plus nested-enum lane rule.
- Aggregate enum payload slots preserve `own i64` and `own u64` payload storage
  for direct temporary constructor matches, so a `match` arm can bind and drop
  the owned payload value without erasing the ownership qualifier.
- Direct aggregate-enum constructors stored in locals or assigned to whole
  locals seed only the active `own i64`/`own u64` payload slots as live owner
  paths. Fieldless cases such as `None` do not require a payload drop, and
  moving or dropping a stored payload-bearing case uses the active payload
  owner path.
- Runtime-dependent aggregate enum locals, parameters, and call/return results
  with owner-carrying payload slots can be explicitly dropped as whole values.
  The lowered cleanup reads the runtime tag and only calls payload destructors
  for the active case.
- Aggregate enum payload slots can store plain Ari-layout tuple, fixed-array,
  struct, and explicit fixed-capacity `Vec[T; N]` payload values inline. Match
  payload bindings and direct payload slot access expose the full aggregate
  value while general tag-aware ownership through stored enum payloads stays
  planned behind its ABI rules.
- Enum patterns can destructure inline plain-aggregate payload slots with
  tuple, fixed-array, and struct subpatterns for value bindings, aliases,
  wildcards, and nested product subpatterns.
- Enum patterns can destructure explicit fixed-capacity `Vec[T; N]` payload
  slots with exact array-style element patterns. The lowered arm checks the
  vector's current length before extracting inline element fields.
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
- LLVM object fixtures now cover direct by-value `@repr(C)` aggregate exports
  that also reference unresolved `extern "C"` helpers, so symbol tables and
  relocation records stay checked as the library ABI surface grows.
