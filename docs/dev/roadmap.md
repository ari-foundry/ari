# Roadmap

## Near-Term Compiler Work

Parser-visible syntax is now stable for linting and language-server tooling.
`init ... while ... next` is the only loop-state spelling; `let ... while` is
rejected with a migration diagnostic. Declaration-level reference pattern
binding modes are stable for the first local slice: `let ref PATTERN = value`
and `let ref mut PATTERN = value` now introduce named borrow bindings for
plain local/path initializers and for tuple, fixed-array, and struct
destructuring over those tracked paths. `&`/`&mut` shorthand, function
parameter reference patterns, and ownership-aware aggregate binding modes
remain reserved for the broader shared binding-mode engine.
The mutable declaration binding-mode slice promoted from Medium-Term is
complete for the current 0.x surface: `let mut PATTERN = value` now parses as
declaration-level mutability for every binding introduced by the pattern and
reuses the existing `var PATTERN` lowering path.
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

The loop fixed-point precision foundation is complete for the current 0.x
surface and has been removed from active Near-Term work. The loop checker tracks
ownership and borrow state at `break`, `continue`, zero-iteration,
literal-true next-iteration, and fallthrough merge points, including
conservative same-provenance borrow-release joins and literal-true `break` exits
that merge moved/dropped unavailable owner states. Owner-state widening now
requires an explicit body recheck before any non-trivial next-iteration or
fallthrough fixed point is accepted: plain `while`, `init while`, irrefutable
aggregate/runtime-sequence `while let`, direct or immutable local/alias
enum-constructor `while let` with statically satisfied payload
literal/range/nested-enum tests, exact-once range/list/stored-`Vec` `for`
slices, runtime-dependent refutable enum `while let`, and direct
`Iterator[T]`/`IntoIterator[T]` loops all revalidate candidate
`Alive -> moved/dropped` next-iteration states by rechecking the body under the
widened state. Known-nonempty range/list/stored-`Vec` `for` loops also drop the
zero-iteration exit from post-loop owner merges, and runtime-dependent
maybe-zero exits merge `Alive` with moved/dropped owner states into an explicit
`maybe-unavailable` local state. Later use, overwrite, return, scope exit, and
aggregate owned-field overwrite reject that state instead of pretending the
owner is definitely live or definitely unavailable. Plain `while` and
`init while` also treat immutable local bool conditions whose initializers
resolve through immutable local aliases to literals like `true`/`false` as
proven loop conditions, and fold those conditions into literal IR branch
conditions during lowering.

IR package-cache replay is complete for the current V0 0.x executable cache
surface and has been removed from active Near-Term work. Validated
`ari-ir-summary-v0` sidecars now materialize lowered statement/expression body
trees into pre-lowered `IrFunction` bodies, replay named struct/enum/fixed
array/vector-capacity type shapes from declarations plus cache-only type
metadata, skip semantic body lowering for cached dependency functions, and
append those replayed bodies to the final IR in the same order as the normal
sema path. The V0 sidecar now also carries an explicit layout-descriptor section
inside the existing cache family: `L;...D;...` entries currently describe
cache-only local `Vec[T; capacity]` storage as `vector-storage` descriptors
with the normalized vector type, element type, and slot count. Cache parsing
rejects missing, unused, duplicate, unknown, or mismatched layout descriptors
before replay, so layout-bearing metadata no longer has to be inferred from
body replay accidents. The cache-use path can therefore skip parsing
summary-safe dependencies after metadata/source-hash validation while still
producing the same LLVM IR as a fresh sema lowering. Hash-valid but malformed
replay payloads are reported as module-cache IR replay diagnostics tied to the
lowered function being reconstructed, before backend emission. The focused
cache layout guard now checks both the descriptor payload and fresh/cache-use
LLVM byte-for-byte output for cache-only local `Vec[T; capacity]` metadata, so
new layout-bearing IR type metadata must add a descriptor and grow that guard
instead of relying only on broad cache-body tests.

The direct fixed-array export ABI slice promoted from Medium-Term is complete
for the current 0.x shared LLVM surface and has been removed from active
Near-Term work. The C header emitter handles fixed-size array fields in public
`@repr(C)` structs, explicit `ptr/ref/ref mut [T, N]` pointer-to-array
parameters, and direct by-value `[T, N]` exported parameters/returns through
generated wrapper typedefs such as `AriArray_i64_2`. Those direct fixed-array
exports share the same 64-bit Unix, 16-byte, 8-byte-alignment guard as other
direct C aggregate ABI values. Raw/freestanding imported C array calls stay
with the aggregate follow-ups under Backend Work `[raw-c-imports]`, after the
scalar/raw-pointer import path promoted below lands.

Mixed scalar/pointer-shaped and nested aggregate-enum payload slots are fixed
for the current aggregate enum ABI and are no longer tracked as active
Near-Term work. When one payload position mixes ordinary payload-word values
(`i64`/`u64`, smaller integers, bool, `string`, `ptr T`, `fn(...)`, or a
one-word enum) with one nested aggregate enum type, Ari stores the outer slot
as that nested aggregate enum layout. Payload-word cases zero-initialize the
nested storage and write their value into the nested enum's first payload word
lane; nested enum cases store the full nested value. This gives LLVM and the
freestanding local/pointer-backed paths one precise layout rule without
opening tuple, struct, vector, owned, or multiple-nested-enum payload storage.

Aggregate enum payload field access is fixed for the current local and
raw-pointer-backed ABI and is no longer tracked as active Near-Term work. A
tuple-index expression on an aggregate enum, such as `value.0` or
`(*raw_enum).0`, addresses payload slot 0, not the hidden tag field. The
lowering maps that source payload slot to the stored tag-plus-payload layout,
so local reads/writes, LLVM raw-pointer field GEPs, and freestanding
local/pointer-backed raw-place loads, stores, and aggregate slot copies all use
the same checked slot-index rule. This remains a low-level ABI access: it does
not check the current case tag, and scalar/pointer-shaped slots expose their
stored payload word.

Freestanding direct aggregate enum value materialization is fixed for the
current supported layout and is no longer tracked as active Near-Term work.
When a multi-payload aggregate enum value is needed as a direct `match` input
or tag source, the raw backend now materializes constructors, aggregate-returning
direct/function-pointer calls, block/if/match results, and raw-pointer-backed
loads into hidden stack storage, then reads the tag and payload slots from that
storage. This keeps the raw backend's no-register-aggregate rule intact while
letting direct expression inputs use the same tag/payload lowering as locals.

Active Near-Term work promoted from Medium-Term and Backend Work:

1. [aggregate-layout-service] finish sharing field-layout and aggregate layout
   decisions between sema and both backends. This should be a broad layout
   service/refactor, not another syntax-specific sema helper.
2. [raw-c-imports-scalar] implement the first real raw/freestanding imported
   `extern "C"` path for scalar and raw-pointer signatures only. Keep
   aggregate, varargs, platform float-C ABI, and libc discovery outside this
   slice until the scalar link/call path is boring.
3. [raw-runtime-strings] add freestanding runtime string storage sufficient for
   string literals, byte writes, and the currently documented line-input
   rejection boundary without depending on the host C runtime.
4. [raw-dyn-dispatch] lower copyable LLVM-supported trait-object values and
   vtable-slot dispatch in the raw backend, leaving `own` and borrow-valued dyn
   data-pointer policy in Medium-Term.
5. [raw-relocatable-objects] emit native relocatable object files for the raw
    backend using the current Ari symbol table/export model. Treat C ABI
    relocations and host linker integration as follow-ups to `[raw-c-imports-scalar]`.

See also [Semantic Checker Decomposition](sema-decomposition.md) for the
maintenance roadmap for splitting `src/sema.cpp` into smaller subsystems.

## Medium-Term Compiler Work

1. Expand IR package-cache replay after the current V0 free-function surface.
   V0 replay covers non-generic free-function dependency bodies and the
   generated impl specializations needed by those bodies. Keep the cache family
   on 0.x/V0 until a deliberate version bump is approved, then extend the
   sidecar format for richer stable identities. V0 local vector-storage layout
   descriptors are complete in the current IR sidecar; richer generic/impl
   replay and any identity-bearing descriptor expansion still wait for a
   deliberate cache identity/version bump.
   - [ir-replay-generics] replay generic, impl, and trait-specialized bodies
     directly from cache once their stable sidecar identity is versioned

## Medium-Term Language Work

1. Extend pattern binding modes beyond value bindings.
   Declaration-level `let mut PATTERN = value` is implemented as the mutable
   value-binding slice. The first local reference binding slice is implemented
   for `let ref` / `let ref mut` over tracked local/path initializers and
   tuple, fixed-array, and struct destructuring. This Medium-Term item now
   tracks shorthand, function-parameter, and ownership-aware binding forms that
   need broader source-path and aggregate ownership policy.
   - [reference-shorthand] design `&` and `&mut` pattern shorthand after
     explicit `ref`/`ref mut` local patterns are stable
   - [reference-params] carry explicit reference binding modes through function
     parameter patterns once the parameter ABI and source-path contracts are
     shared with local declarations
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
   - [owner-resolution] design an explicit conditional cleanup/resolution form
     for `maybe-unavailable` owners once runtime owner-state values are part of
     the language surface; until then, loop exits that cannot prove a single
     owner state remain intentionally unusable after the loop.
2. Expand aggregate enum payload storage beyond the nested-enum MVP.
   Aggregate enum payload slots support integer, bool, pointer-shaped values
   such as `string`, `ptr T`, and `fn(...) -> ...`, one-word enum values, and
   nested aggregate-enum values today. A slot may also mix payload-word cases
   with a single nested aggregate enum when that nested layout has a first
   payload-word lane. Nested enum-case subpatterns can inspect one-word enum
   payload slots on the LLVM and freestanding local-value paths, and can
   inspect homogeneous or mixed-lane nested aggregate-enum payload slots on the
   LLVM and freestanding paths. The freestanding backend can store/copy local
   and pointer-backed aggregate enum values, including homogeneous or mixed
   nested aggregate-enum payload values and direct enum-constructor stores
   through raw pointers. Direct payload slot access through `value.0` and
   `(*ptr).0` is also implemented for local and raw-pointer-backed aggregate
   enum values; it exposes the ABI storage slot directly and does not check the
   active tag. Freestanding direct match/tag inputs can materialize supported
   aggregate enum constructors, aggregate-returning calls, block/if/match
   results, and raw-pointer-backed loads into hidden stack storage before
   reading their tag/payload slots. The stored payload universe is still
   intentionally narrow.
   - [aggregate-values] allow tuple, struct, vector, and owned payload values
     after their non-local ABI/storage rules are defined
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
      borrow-valued source types; copyable raw backend dispatch moved to
      Near-Term `[raw-dyn-dispatch]`

## Backend Work

1. Emit relocatable object files for the native backend.
   The first Ari-symbol-table-only slice moved to Near-Term
   `[raw-relocatable-objects]`; keep C ABI relocations and host linker polish
   tied to the raw C import path.
2. Define non-local aggregate ABI layouts for external ABI surfaces and
   vectors. Fixed-size local tuple/array/vector stack layout,
   direct raw aggregate parameters/returns through hidden pointers, LLVM
   aggregate enum layout, tuple/array/vector index access, raw aggregate
   expression results, and raw `@export`/`@no_mangle` ELF symbols are
   implemented.
   - [structs] broad field-layout sharing moved to Near-Term
     `[aggregate-layout-service]`
   - [raw-c-imports] scalar/raw-pointer imported `extern "C"` support moved to
     Near-Term `[raw-c-imports-scalar]`; aggregate, varargs, platform float-C
     ABI, and libc discovery remain here until the scalar path exists
   - [vectors] define the growable root `Vec[T]` runtime ABI and backend
     lowering after allocator-backed storage exists; fixed-local root `Vec[T]`
     remains rejected at non-local boundaries until then
   - [enums] external aggregate-enum FFI ABI remains after Near-Term
     `[aggregate-layout-service]`
3. Add freestanding runtime string storage so raw ELF output can lower string
   values and line input helpers without relying on the host C runtime.
   The first storage/runtime slice moved to Near-Term `[raw-runtime-strings]`;
   keep richer hosted IO compatibility and allocator integration as follow-ups.
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
