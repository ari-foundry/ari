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

1. Prepare source `std` library foundations before broad library expansion.
   Keep the library-facing contracts near-term before adding many owned
   collection, string, or smart-pointer APIs. This keeps the source prelude from
   growing into a pile of one-off compiler hooks. The current source `std`
   child modules now live as package files under `lib/std/`, so the root
   `lib/std.arih` stays focused on root types, root traits, builtin declarations,
   and re-exports while future child modules can start in their own files.
   Zone-provenance rules for source handles and pointer-returning methods now
   stay in focused helpers such as `std_box_semantics`, `std_vec_semantics`,
   `slice_semantics`, and `zone_pointer_semantics` instead of growing more
   bespoke checks in `sema.cpp`. The root `std::Vec[T]` spelling now aliases the
   explicit-zone `std::vec::Vec[T]` source handle rather than the temporary
   compiler-known local vector type, and the `Vec!` macro still lowers to
   `std::vec::new<T>` as constructor sugar. The `std::vec::Vec` same-zone
   method contract lives in `std_vec_semantics`, and source-handle/pointer
   provenance expression tracing for locals, tuple fields, calls, slices, and
   control-flow expressions lives in `zone_pointer_semantics`. Zone
   source/generation assignment, named
   zone-borrow reset recognition, reset invalidation, validity diagnostics,
   temporary-zone escape diagnostics, temporary-zone `zone::destroy` IR cleanup,
   and cleanup-before-exit value materialization for returns, labeled breaks,
   and `init while` continue values are centralized there. The root
   `Box[T]`/`std::Box[T]` spelling currently aliases the explicit-zone source
   `std::boxed::Box<T>` handle; `Unique[T]` stays reserved for policy
   compatibility, and `Shared[T]` / `Weak[T]` are reserved for
   reference-counted ownership. Source `Slice[T]` now has checked
   `first`/`last`/`get`, element search, and borrowed `Slice[T]`
   exact/prefix/suffix comparison methods, plus
   `copy_to(ref mut Zone)` into a target-zone `std::vec::Vec<T>`, so early
   library code can use borrowed views without adding one-off helpers. New
   source `std` APIs are now guarded by `make check-std-api`, which compares the
   extracted public `lib/std` surface with `tests/std_api_manifest.txt` and
   requires a focused coverage note beside the API entry. Generic `Drop` impls
   are now selected during explicit drop lowering, and the existing
   `std::boxed::Box<T>` source seed, including the root `Box[T]` alias, has a
   concrete value-drop contract:
   `drop boxed` consumes the handle binding and runs the pointed-to value
   through normal Drop lowering when the handle is not empty. `boxed.set(value)`
   drops the overwritten value, `boxed.take()` moves the value out and empties
   the handle so a later handle drop skips that value; `boxed.try_take()`
   returns `Option<T>` for the same path; `boxed.clear()` drops
   the current value and empties the handle;
   `boxed.put_in(ref mut zone, value)` refills that empty handle only with the
   same tracked source zone, while storage release remains the
   explicit zone's responsibility through `zone::reset` or `zone::destroy`. Source
   `std::vec::Vec<T>` follows the same explicit-zone value-drop policy:
   `drop vec` drops each current element and leaves buffer release to the zone,
   while overwrite and shrink helpers drop removed elements before reducing the
   live length. `vec.try_pop()` returns `Option<T>` for empty-aware last-element
   move-out. The small goal [std-vec-inferred-zone-growth] is now complete:
   tracked local `std::vec::Vec<T>` / `std::Vec<T>` receiver provenance
   synthesizes the same source-zone argument for `push`, `insert`,
   one-argument `reserve`/`reserve_extra`, `extend_from_slice`, and `resize`.
   Explicit `_in` forms remain available, and untracked manually assembled
   handles still get a targeted diagnostic instead of a hidden allocation
   capability.
   Small goal [source-string-inferred-zone-growth]: consider mirroring the same
   receiver-provenance convenience for source `std::string::String` growth and
   append helpers once the Vec path has settled under library use.
   Source `std::mem::replace<T>` and `std::mem::swap<T>` now provide
   mutable-place value helpers for copyable scalar and plain Ari-layout
   aggregate values, with root prelude re-exports. They lower through the same
   raw-pointer materialization path as `ptr_load` / `ptr_store`, and that path
   now rejects ownership- or borrow-valued values instead of allowing accidental
   raw copies. For the 0.x library-prep surface, move-aware owning-place
   replacement and swapping are deliberately out of scope until Ari has a safe
   generic place-move contract.
   Source `Option[T]` and `Result[T, E]` also have their first ordinary library
   method surface now: borrowed-receiver presence/status predicates that lower
   to tag-only enum probes, consuming panic-style `unwrap`/`expect` helpers,
   `unwrap_or`, and consuming Option `map`/`or`/`or_else`/`and_then` plus Result
   `map`/`map_err`/`and_then`/`or_else`/`unwrap_err`/`expect_err` helpers,
   implemented in focused
   `std::option` and `std::result` child modules while the enum types and cases
   remain at the `std` root. The raw freestanding backend now preserves caller
   pointer bases while aggregate-valued match/control-flow results evaluate
   aggregate-returning callees, so these predicates and combinators run on both
   LLVM-host and raw paths. Statement-position and expression-arm
   `panic`/`todo`/`unreachable` noreturn recognition, including source builtin
   aliases, Ari builtin symbols, and block-wrapped bottom-like values, lives
   with the control-flow helpers rather than adding one-off `sema.cpp` special
   cases, so expression-valued `if`/`match` arms can use those stop calls while
   reachable arms determine the result type.
   Method receiver borrow weakening now lets a `ref mut Self` receiver call
   `ref Self` methods directly, including trait-qualified calls. This keeps
   source `std` internals from duplicating read-only predicates inside mutable
   methods while preserving the stricter explicit borrow rules for ordinary
   borrow-valued bindings.
   Source `std::cmp` now has small generic value helpers
   (`min`, `max`, and `clamp`) over its `cmp::Ord[T]` trait, with root prelude
   re-exports for ordinary library code. This covers another ordinary-library
   API without adding compiler-known hooks.
   The Drop
   trait/method shape checks and shared diagnostics for explicit destructor
   lowering now live in `drop_semantics`, keeping this ownership/destructor
   phase out of the central expression-lowering code.
   Owned string work has a source-handle root spelling now:
   `std::string::alloc_buffer(ref mut Zone, capacity) -> ptr u8` validates a
   non-negative byte capacity, allocates bytes through the explicit zone
   capability, and returns a tracked pointer that is invalidated by
   `zone::reset`/`zone::destroy`. `std::string::RawString` and
   `std::string::String` now wrap that storage in a tracked source handle with
   `data`, `len`, and `capacity` metadata, borrowed receiver lowering for
   metadata, endpoint byte reads, indexed byte reads, byte search,
   `Slice[u8]` exact/prefix/suffix checks, and raw-pointer views,
   fixed-capacity byte push/pop,
   checked get/set/replace, truncate/clear, slice views,
   explicit same-zone growth through `reserve`, `reserve_extra`, `push_in`,
   `insert_in`, `extend_from_slice_in`, and `resize_in`,
   same-zone text construction helpers through `append_string_in`,
   `append_i64_in`, `append_u64_in`, `append_bool_in`, `append_f32_in`, and
   `append_f64_in`; all string growth now goes through private capacity
   helpers that centralize the reallocation/copy path before later string
   builder APIs are added,
   `String.copy_to(ref mut Zone)` and
   `std::string::copy_to(ref value, ref mut Zone)` borrowed-source target-zone
   copying, `std::string::from_slice_in(ref mut Zone, Slice[u8])` target-zone
   construction from a borrowed byte slice, and
   `from_string(ref mut Zone, string)` copying from today's borrowed lowercase
   `string` values. Host line input now has explicit-zone owned helpers
   (`read_line_owned`, `std::io::read_line_owned`, `input_owned`, and
   `std::input::owned_line`) that copy the borrowed line buffer into
   `std::string::String` storage before later input can overwrite it. The root
   `String`/`std::String` spelling now aliases that explicit-zone source
   string handle; storage is still released by `zone::reset`/`zone::destroy`,
   and the current `Drop` impl only ends the binding.
   - [owned-box-unique] define and implement allocator-backed unique `Box[T]`
     ownership and construction before std APIs start returning owning heap
     handles. Today's root `Box[T]`/`std::Box[T]` spelling is an alias for the
     explicit-zone `std::boxed::Box<T>` source seed, with associated
     construction through `Box::new<T>(ref mut Zone, value)`, borrowed
     read-only method receivers, zone-provenance tracking, use-after-drop
     checking, and a generic Drop impl that runs the stored value's Drop path.
     The explicit-zone seed now also has the first value move-out contract:
     `set(value)` drops the overwritten value, `take()` returns the stored value,
     leaves the handle empty, `try_take()` returns `Option<T>` for the same
     path, `is_empty()` exposes that state,
     `put_in(ref mut Zone, value)` refills the empty handle under same-zone
     provenance checks, `clear()` drops and empties a non-empty handle while
     treating an already empty handle as a no-op, and Drop skips empty handles.
     Remaining work is the allocator-backed root handle layout,
     allocator/capability construction, move-only ownership rules for the root
     handle, provenance updates for any future cross-zone handle mutation, and
     integration with heap release.
   - [owned-box-release] connect the allocator-backed unique `Box[T]` Drop path
     to the heap-storage release contract once that root handle exists. The
     current source `std::boxed::Box<T>` / root `Box[T]` value-drop contract
     intentionally leaves storage release with `zone::reset` / `zone::destroy`.
   Explicit-zone formatted strings are now settled for the 0.x source-`std`
   surface: `format_in!(ref mut Zone, "...", values...)` lowers `{}`
   string/signed and unsigned integer/bool/float formatting and `{:.N}` float
   precision to source
   `String` construction plus same-zone append helpers, and evaluates each
   formatted value once before selecting the append target from the lowered
   value type. User-defined value types can implement borrowed-receiver
   `Display::format_in` or `fmt::Display::format_in` to return a source
   `String` in the same explicit zone, so formatting no longer needs new
   macro-only cases for every library type. Plain `format!` remains a reserved
   spelling with a targeted
   no-implicit-allocation-zone diagnostic, keeping Ari away from a magical
   global heap while the library surface stays explicit-capability based.
   `print`/`println` and source IO now have a `u64` runtime formatting path
   through `ari_builtin_write_u64` / `std::io::write_u64`, matching the
   explicit-zone `format_in!` unsigned integer support.

See also [Semantic Checker Decomposition](sema-decomposition.md) for the
maintenance roadmap for splitting `src/sema.cpp` into smaller subsystems.

## Medium-Term Compiler Work

1. Promote IR package-cache summaries when future executable forms outgrow AST
   summaries.
   Source-snapshot module caches currently validate cfg/search-path/source
   hashes/imports and can materialize declaration-safe headers plus
   summary-safe executable dependency bodies from AST summaries without parsing
   the cached source snapshot. The current source-level AST expression,
   statement, and pattern surface is covered by that path. The V0 cache family
   is centralized on `ari-module-metadata-v0`, `ari-module-cache-v0`, and
   `ari-ast-decls-v0`; add IR summaries later for future executable bodies that
   cannot round-trip through the compact AST summary format without changing the
   V0 header by default.
   - [ir-materialize] feed future IR-summary declarations/bodies into the
     module loader for dependencies whose executable function or impl bodies
     use forms outside the AST summary subset
   - [cache-skip] once IR summaries exist, avoid reparsing those dependencies
     when metadata and source hashes match the current source graph and
     cfg/search-path inputs
2. Improve loop fixed-point precision beyond exact snapshots.
   The near-term loop checker now tracks ownership and borrow state at
   `break`, `continue`, zero-iteration, literal-true next-iteration, and
   fallthrough merge points. It rejects incompatible ownership states, live
   loop-local owners across jumps, and borrow-state changes that are not exact
   fixed points. Further work is precision, not basic loop-state tracking.
   - [owner-widen] prove non-trivial ownership fixed points where later
     iterations intentionally start from a changed but compatible state
   - [borrow-widen] merge compatible borrow-state transitions beyond exact
     snapshot equality without weakening source-borrow diagnostics
3. Extend the meta AST evaluator beyond the lint-stable constructor subset.
   Near-Term `ast -> ast` macros can generate declaration, expression, pattern,
   and type AST output through explicit constructors and can inspect declaration
   input by kind, name, count, and visibility. Keep richer compile-time AST
   reflection out of the fixed syntax surface until its policy is explicit.
   - [ast-rich-construction] add structured field/method/type inspection and
     dynamic identifier construction for declaration-generating macros once the
     evaluator's data model is defined
   - [ast-general-values] decide which compile-time value operations are
     allowed for non-declaration AST inputs before broadening expression
     rewriting beyond today's explicit constructors and hygienic substitution

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
    Allocator-backed unique `Box[T]` ownership remains promoted into the
    Near-Term source-`std` library-prep checklist because broad library APIs
    should not return owning heap handles until their construction, provenance,
    move, and drop contracts are explicit. The explicit-zone formatted string
    construction path is already in the Near-Term completed surface through
    `format_in!` and `Display::format_in`.
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
