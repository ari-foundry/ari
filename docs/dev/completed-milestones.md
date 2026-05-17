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
- Function parameter patterns accept explicit `ref PATTERN: T` and
  `ref mut PATTERN: T`, lowered as function-entry borrows from hidden ABI
  parameter storage.
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
- LLVM and freestanding `dyn Child` trait objects include inherited object-safe
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
  builtin. Freestanding zone allocation remains rejected until a raw-backend
  allocation runtime exists.

## Modules And Cache

- Module cache metadata, source-snapshot cache records, and AST declaration
  summaries use the shared V0 cache-format contract.
- V0 IR package-cache replay materializes lowered statement/expression body
  trees into pre-lowered `IrFunction` bodies for summary-safe dependencies.
- Cache replay can skip semantic body lowering for validated dependency
  functions, append replayed bodies in normal sema order, and report malformed
  replay payloads before backend emission.
- V0 layout descriptors live in the existing cache family. Current descriptors
  cover cache-only local `Vec[T; capacity]` storage as `vector-storage`
  entries, and cache-use tests compare fresh/cache LLVM output byte-for-byte
  for that metadata.

## ABI, Layout, And Backends

- Direct fixed-array exports are implemented for the current 0.x shared LLVM
  surface, including fixed-array fields in public `@repr(C)` structs,
  pointer-to-array parameters, and by-value `[T, N]` exported
  parameters/returns through generated wrapper typedefs.
- `src/layout.cpp` owns shared aggregate-layout predicates, field-list
  selection, field counts, byte sizes, alignments, and field offsets used by
  sema, LLVM, raw backend, and related IR helpers.
- Aggregate enum payload slots support integer, bool, pointer-shaped values,
  one-word enums, nested aggregate enums, and the current mixed payload-word
  plus nested-enum lane rule.
- Aggregate enum payload slot access through `value.0` and `(*raw_enum).0`
  addresses payload slot 0 rather than the hidden tag field on local and
  raw-pointer-backed ABI paths.
- The raw backend materializes supported direct aggregate enum values into
  hidden stack storage before reading tags or payload slots.
- The raw backend lowers lowercase `string` literals into a per-image static
  NUL-terminated byte pool.
- Freestanding trait-object dispatch works for the copyable non-borrow dyn
  surface, including concrete impls, generic impl specializations, multi-arg
  dyn calls, Vec-shaped aggregate argument views, inherited object-safe
  supertrait methods, and dyn-to-supertrait vtable upcasts.
- Freestanding `f32`/`f64` local storage, raw pointer load/store/dereference,
  arithmetic, ordered comparisons, and direct Ari calls are implemented.
- The raw backend can emit native x86-64 ELF relocatable object files through
  `--freestanding --emit-obj path`. These objects contain a `.text` section and
  a symbol table using the same Ari mangled or explicit `@export`/`@no_mangle`
  symbols as raw executable output. C ABI relocations and host linker
  integration remain separate follow-up work.
