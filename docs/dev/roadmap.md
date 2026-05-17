# Roadmap

## Near-Term Compiler Work

Parser-visible syntax is now stable for linting and language-server tooling.
`init ... while ... next` is the only loop-state spelling; `let ... while` is
rejected with a migration diagnostic. Pattern binding modes (`ref`, `ref mut`,
`&`, `&mut`, `mut` in patterns) are reserved and rejected with diagnostics so
future reference/ownership binding modes do not collide with the current AST.
Both `ref mut T` and `mut ref T` are accepted as equivalent mutable borrow
spellings. `--check` runs parsing, module loading, and semantic lowering
without backend emission for editor tooling.
`[ ... ]` patterns are fixed syntax for both compile-time-length arrays and
runtime sequence subjects. Local `Vec[T]` storage and `Slice[T]` views now
lower those patterns through shared `len == n` / `len >= n` guards and indexed
element bindings in `let`/`var`, `match`, and `if let`/`while let` statement and
expression positions. Runtime sequence rest markers can bind the skipped range
with `name @ ..` as a `Slice[T]` view, and `Slice[T]` views use the same
lowering for function-parameter patterns. Runtime sequence `match` still
requires an irrefutable fallback such as `_` or `[..]`.
Sema maintenance now follows phase-oriented extraction: constant folding stays
in `constant_semantics`, generic binding/unification/substitution lives in
`type_inference`, and future splits should target broad analysis or lowering
stages rather than one file per syntax feature.
Module cache headers are also fixed for lint/tooling stability: metadata,
source-snapshot cache, and AST declaration-summary payloads all use the shared
V0 cache-format contract until a cache version bump is explicitly approved.
User-defined `meta fn` macro syntax is also stable for the current lint-facing
surface: expression, item, type, pattern, and attribute-position
`ident!(...)` expansion now covers the bounded token-stream and explicit AST
constructor subset documented in the language guide.
The first richer AST declaration-reflection slice has also landed for
library-prep macros: declaration-returning `ast -> ast` macros can inspect
generic, parameter, field, enum-case, method, and associated-type counts through
both free helper functions and `input.*_count()` methods. Named declaration
member inspection is also complete for the current summary surface: macros can
test generic, parameter, field, enum-case, method, and associated-type presence
and compare parameter, field, payload, method, return, trait, and associated
type witness summaries before choosing a `decl!(...)` output.
Dynamic declaration identifier construction is also available through
`meta_ident!(...)` inside `decl!(...)`, so declaration-generating macros can
derive new names from `input.name()` / `decl_name(input)` or
`input.kind()` / `decl_kind(input)` plus literal pieces without exposing a
general compile-time string runtime.
The non-declaration AST compile-time value policy is now fixed for the current
0.x macro surface. Expression-position `ast -> ast` macros can branch on the
input expression kind through compile-time-only `input.kind()` /
`ast_kind(input)` and `input.is("kind")` / `ast_is(input, "kind")`
conditions, while generated output still uses explicit expression constructors
and hygienic input/local substitution. Pattern and type AST inputs deliberately
do not expose general compile-time values yet, and expression helpers cannot be
returned as generated runtime AST.
The trait-associated-item and composition goal that used to sit in Medium-Term
is now covered by the current front-end surface. Supertraits parse and require
matching impls, child-trait bounds can statically dispatch inherited methods,
trait-qualified associated function calls use explicit or expected-result
implementing types, associated type projections resolve through unique impl
witnesses and unique generic supertrait applications, and LLVM `dyn Child`
trait objects include inherited object-safe methods plus dyn-to-supertrait
upcasts. Ambiguous associated functions, inherited methods, inherited
associated type names, and unrelated dyn upcasts are rejected with focused
diagnostics.

Allocator-backed vector library prep is no longer tracked as a Near-Term goal.
The current 0.x surface keeps bare `Vec[T]` as fixed local storage plus the
Slice-shaped direct/function-pointer parameter ABI, while `std::Vec[T]` is now
a public alias for the explicit-zone `std::vec::Vec[T]` source handle and
`Vec!(T, ref mut Zone, capacity)` remains constructor sugar for
`std::vec::new<T>`. The remaining owned root `Vec[T]` runtime-capacity ABI and
permanent root method surface are tracked in Medium-Term/Backend work, because
they need the broader non-local aggregate and allocation capability design.

The source `std` library foundations goal is complete for the current 0.x
library-expansion surface and has been removed from active Near-Term work.
The completed surface is documented in `docs/language/prelude.md`,
`docs/language/types.md`, `docs/language/memory.md`, and the Explicit memory
zones / Prelude rows in `docs/dev/test-matrix.md`: source child modules live
under `lib/std/`, public source APIs are guarded by `make check-std-api`,
source Box/String/Vec/Slice/Option/Result/cmp/mem/fmt/IO contracts are covered,
and source Vec/String growth can infer a tracked receiver's zone without
inventing an ambient heap.

Zone allocation-header metadata is fixed for the host 0.x ABI and is no longer
tracked as active Near-Term work. Every non-empty host `zone::alloc` payload has
an 8-byte header immediately before the returned user pointer. The header
stores only the raw zone handle at `ptr - 8`; size and requested alignment are
not pointer metadata. `zone::allocation_zone` exposes that handle as a narrow
Ari builtin, while zero-capacity source handles may still keep null buffer
pointers and reset/destroy diagnostics continue to use semantic provenance.
Freestanding zone allocation remains deliberately rejected until a raw-backend
allocation runtime exists.

The loop fixed-point precision foundation is complete for the current exact
fixed-point surface. The current loop checker tracks ownership and borrow state at
`break`, `continue`, zero-iteration, literal-true next-iteration, and
fallthrough merge points, including conservative same-provenance borrow-release
joins and literal-true `break` exits that merge moved/dropped unavailable owner
states. The next active Near-Term loop goal is the remaining owner-state
dataflow recheck:

1. Prove loop owner-state widening with a loop dataflow recheck.
   Borrow-release widening is safe with the current same-provenance merge
   because it keeps the source conservatively borrowed. Owner-state widening is
   different: a body checked with an `Alive` owner cannot simply be reused for a
   later iteration that starts with that owner moved or dropped. Add a
   revalidation/dataflow pass before accepting non-trivial next-iteration or
   fallthrough ownership fixed points. The plain `while`, `init while`,
   irrefutable aggregate/runtime-sequence `while let`, direct or immutable
   local enum-constructor `while let`, and exact-once range/list/stored-`Vec`
   `for` slices now recheck no-zero or exact-once next-iteration states under
   a candidate `Alive -> moved/dropped` owner widening before accepting them.
   The plain `while` and `init while` slices also treat immutable local bool
   conditions initialized directly from literals like literal `true`/`false`
   as proven loop conditions, and fold those conditions into literal IR branch
   conditions during lowering.
   - [owner-widen] extend the widened-state recheck beyond plain no-zero
     and exact-once loop bodies into remaining refutable enum
     `while let`/multi-iteration iterator-style bodies and any future
     maybe-zero representation that can distinguish definitely-live from
     maybe-unavailable owners after loop fallthrough

IR package-cache replay is complete for the current V0 0.x executable cache
surface and has been removed from active Near-Term work. Validated
`ari-ir-summary-v0` sidecars now materialize lowered statement/expression body
trees into pre-lowered `IrFunction` bodies, replay named struct/enum/fixed
array/vector-capacity type shapes from declarations plus cache-only type
metadata, skip semantic body lowering for cached dependency functions, and
append those replayed bodies to the final IR in the same order as the normal
sema path. The cache-use path can therefore skip parsing summary-safe
dependencies after metadata/source-hash validation while still producing the
same LLVM IR as a fresh sema lowering. Hash-valid but malformed replay
payloads are reported as module-cache IR replay diagnostics tied to the lowered
function being reconstructed, before backend emission. A focused cache layout
guard now keeps cache-only local `Vec[T; capacity]` type metadata covered with
fresh/cache-use LLVM byte-for-byte comparison, so new layout-bearing IR type
metadata must grow that guard instead of relying only on broad cache-body tests.

See also [Semantic Checker Decomposition](sema-decomposition.md) for the
maintenance roadmap for splitting `src/sema.cpp` into smaller subsystems.

## Medium-Term Compiler Work

1. Expand IR package-cache replay after the current V0 free-function surface.
   V0 replay covers non-generic free-function dependency bodies and the
   generated impl specializations needed by those bodies. Keep the cache family
   on 0.x/V0 until a deliberate version bump is approved, then extend the
   sidecar format for richer stable identities and layout descriptors.
   - [ir-replay-generics] replay generic, impl, and trait-specialized bodies
     directly from cache once their stable sidecar identity is versioned
   - [ir-cache-layouts] add explicit cached layout descriptors only when the
     current declaration-shaped replay is no longer enough

## Medium-Term Language Work

1. Extend pattern binding modes beyond value bindings.
   The parser now reserves `ref`, `ref mut`, `&`, `&mut`, and `mut`
   binding-mode spellings as Near-Term syntax-stability work. This Medium-Term
   item is the semantic lowering phase for those reserved forms.
   - [reference] design `ref`, `ref mut`, `&`, and Ari ownership-aware binding modes
   - [ownership] preserve binding modes through aggregate, enum, slice, and vector patterns once ownership-through-aggregates lands
   Tuple, fixed-array, named-struct, and tuple-struct match arms now share
   same-name/same-type or-pattern bindings through the product pattern engine.
   Refutable aggregate `let`/`var` declarations now reuse that engine for
   tuple, fixed-array, named-struct, and tuple-struct literal/range/alias/or
   tests, and preserve `var` mutability for every introduced binding.
   Aggregate `if let` statement/expression and aggregate `while let`
   statement bindings also share the same product match lowering path. Local
   `Vec[T]` storage and `Slice[T]` views now participate in that path through
   length-checked `[ ... ]` runtime sequence patterns; `Slice[T]` function
   parameter patterns reuse the same function-entry binding prelude.
   - [positions] keep `let`/`var`, match, control-flow, for-loop, and
     function-parameter patterns on one shared binding-mode engine; function
     parameters now cover value-mode aggregate and `Slice[T]` runtime sequence
     destructuring, and value alias patterns now work in range, list-literal,
     and stored-vector loop heads when
     the wrapped pattern is irrefutable. Non-iterator loop-head validation now
     lives in `for_pattern_semantics`, with a shared sema lowering helper for
     range, list-literal, and stored-vector loop heads; reference/ownership
     binding modes still need shared lowering.
2. Expand aggregate enum payload storage beyond the nested-enum MVP.
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
3. Lower remaining allocation-backed prelude ADTs. Integer `Range[T]` and
    `RangeInclusive[T]` local values are implemented today. `Option[T]` and
    `Result[T, E]` are source `std` generic enums exposed through the implicit
    prelude and connected to
    `?`/`??` on the LLVM aggregate-enum path; their borrowed-receiver
    predicates and consuming `unwrap_or`/combinator methods have been promoted
    into the Near-Term source-`std` library-prep checklist. `Slice[T]` is now
    a source `std` view struct implemented on both LLVM and the raw
    freestanding backend for
    `slice(data, len)`, `len(view)`, `view.len()`, `view.is_empty()`,
    `view[index]`, `view[index] = value`, local array/Vec `as_slice()`,
    `view[start..end]` / `view[start..=end]` range slicing, checked
    `first`/`last`/`get`, element search, and exact/prefix/suffix Slice
    comparisons. It can also expose the stored raw pointer through
    provenance-preserving `as_ptr()` and copy itself into target-zone
    `std::vec::Vec<T>` storage through `copy_to(ref mut Zone)`. It is
    non-owning and still relies on explicit raw-pointer discipline.
    Nullable `T?` remains a raw-pointer spelling for `ptr T`; non-pointer
    absence stays on the explicit `Option[T]` ADT path.
    Allocator-backed unique `Box[T]` ownership stays Medium-Term: the current
    root `Box[T]` / `std::Box[T]` spelling is the explicit-zone
    `std::boxed::Box<T>` source handle with value-drop, empty-state, and
    same-zone refill semantics, while a future owning heap handle still needs
    root layout, allocator/capability construction, move-only ownership rules,
    provenance updates for cross-zone mutation, and storage-release Drop
    integration. The explicit-zone formatted string construction path is
    already in the completed 0.x source-std surface through `format_in!` and
    `Display::format_in`.
    Slice pattern follow-ups live with the shared pattern binding-mode work
    because they depend on reference/ownership binding policy, not allocator
    ownership.
4. Extend allocator-backed growable `Vec[T]` after the MVP. Non-empty `[...]` now defaults to
    fixed array literals unless a `Vec[T]` expected type is present. Local
    stack-backed vector literal storage, checked indexing, literal reassignment
    with changing runtime length, typed empty local vectors, length queries,
    stored-vector loops, and the temporary fixed-capacity local method surface
    now lower on both LLVM and the raw freestanding backend for supported
    element values. The current local vector storage grows to the largest
    literal, reserve capacity, or compiler-tracked push/insert capacity seen in
    the binding, while the stored length still shrinks and expands per
    assignment. Explicit-zone growth is available through `std::Vec[T]` /
    `std::vec::Vec[T]`; the remaining owned root `Vec[T]` runtime-capacity ABI,
    non-local aggregate layout, and permanent root public API are Medium-Term
    work shared with the Backend vectors item. The temporary compiler-known
    local API should not grow new convenience methods before that owned root
    layout exists.
5. Extend trait-object dispatch beyond the concrete/generic-impl copyable LLVM
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
   - [vectors] define the growable root `Vec[T]` runtime ABI and backend
     lowering after allocator-backed storage exists; fixed-local root `Vec[T]`
     remains rejected at non-local boundaries until then
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
