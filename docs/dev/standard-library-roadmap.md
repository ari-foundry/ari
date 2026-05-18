# Standard Library Roadmap

This roadmap tracks source standard library work. General compiler work remains
in [Roadmap](roadmap.md), and coverage details remain in
[Feature Test Matrix](test-matrix.md).

## Goal

Ari's libraries should be ordinary Ari source whenever possible. The compiler
should only know about primitives that the current language cannot express:
layout queries, raw pointer operations, zone runtime hooks, formatting macros,
temporary-zone lowering, and backend-specific entry/runtime glue.

The library contract is explicit and capability-oriented:

- no hidden global heap
- no source-level C++ ABI dependency
- allocations flow through visible capabilities such as `Zone`
- ownership, borrowing, and zone provenance are checked before LLVM lowering
- public APIs are tracked in `tests/std_api_manifest.txt`
- every public API has focused positive, negative, and backend coverage where
  applicable

## Current Baseline

The current `std` package already provides:

- prelude ADTs: `Option`, `Result`, `Slice`, `Range`, `RangeInclusive`
- source `Option`/`Result` predicates, combinators, conversions, nested option
  filtering, flattening, bidirectional option-result transposition, and lazy
  fallback helpers, including consuming payload predicate helpers
- assertion, panic, `move`, and `take` helpers
- IO/input/context builtin declarations plus source helpers such as
  `io::write_bytes`, `input::try_read_byte`, and `context::has_arg`
- layout and pointer helpers in `std::mem`
- explicit-zone allocation in `std::zone`, including the source
  `alloc_array<T>` raw buffer helper
- source handles for `Box`, `String`, and `Vec`
- `Slice[T]` and `std::vec::Vec[T]` metadata, asserting element access, and
  `Option`-returning element access helpers
- `std::string::String` empty-safe byte access, byte search, comparison,
  ASCII case-insensitive comparison/search, borrowed and owned ASCII trim
  helpers, and whole/prefix ASCII parsing helpers
- range/iterator traits and the `std::vec::Iter` implementation
- comparison, formatting, and conversion trait surfaces, plus source
  comparison value helpers: `min`, `max`, `clamp`, and `is_between`
- `std::convert` source helpers: `identity`, `from`, and `into`
- `std::math` integer helpers implemented in Ari source with natural names:
  `abs`, `sign`, `is_positive`, `is_negative`, `is_zero`, `is_even`,
  `is_odd`, `pow`, `div_floor`, `div_ceil`, `mod_floor`, `gcd`, and `lcm`
- `std::ascii` byte classification, printable/control predicates, case
  conversion, borrowed-slice case-insensitive comparison/search, trimming,
  whole-slice digit parsing, and prefix digit parsing helpers
- `std::bits` `u64` mask, rotation, power-of-two, low-mask, alignment, and
  zero/one-run bit-scan helpers

This baseline is useful, but it is still a seed. Some APIs are compiler hooks
with source declarations, and some names exist mainly so user code can start
depending on stable module paths.

## Phases

### Phase 1: Stabilize The Current Source `std`

- Keep `lib/std.arih` as the single public root.
- Keep child modules file-backed under `lib/std/`.
- Maintain `tests/std_api_manifest.txt` for every public declaration.
- Keep the user-facing API guide in
  [Standard Library](../language/standard-library.md).
- Split library tests by purpose using the naming scheme in
  [Library Testing](library-testing.md).
- Prefer source implementations for helper methods and combinators.

Exit criteria:

- `make check-std-api` passes.
- `make check-prelude` covers every public API family.
- New contributors can find module purpose, API shape, and tests from docs.

### Phase 2: Pull More Behavior Into Source Ari

- Move helper logic out of compiler hooks when structs, generic aggregates, and
  trait dispatch can express it safely.
- Keep compiler-known declarations as compatibility shims only when source
  lowering cannot model the primitive yet.
- Do not make LLVM codegen re-resolve source names; sema should lower the IR
  metadata needed by the backend.

Likely compiler work:

- generic aggregate/type monomorphization cleanup
- stronger trait-bound dispatch for reusable helper impls
- richer module-cache summaries for source library bodies
- clearer diagnostics when a partial custom `std` omits required helpers

### Phase 3: Collections And Allocation

- Stabilize `std::vec::Vec[T]` as the source growable vector handle.
- Keep root bare `Vec[T]` and source `std::Vec[T]` distinctions documented
  until the compiler-known local vector model is unified or retired.
- Add collection APIs only when ownership, borrowing, and zone provenance can
  be tested in focused slices.
- Grow collection families in this order: slice helpers, vector methods,
  iterator adapters, maps/sets/deques.

Likely compiler work:

- generic aggregate monomorphization for richer collection layouts
- iterator protocol diagnostics beyond direct `range` and current `Vec` support
- allocation-zone diagnostics for nested and generic wrapper types

### Phase 4: Text, Formatting, And Diagnostics

- Keep `std::string::String` as a byte string until a deliberate Unicode/text
  policy is designed.
- Add formatting APIs through `std::fmt` only when `Display` and `Debug`
  behavior can be expressed or cleanly lowered.
- Prefer `format_in!(ref mut zone, ...)` for owned formatted strings.
- Add richer panic/assert messages only after string lifetime and allocation
  behavior stays predictable.

Likely compiler work:

- reduce formatting macro special cases as trait dispatch matures
- owned runtime string diagnostics for invalid zone or type usage
- optional float/text runtime helpers behind Ari builtin declarations

### Phase 5: OS-Facing Libraries

- Add thin wrappers for environment, file, time, and process APIs only after
  C FFI wrapper conventions are stable.
- Keep OS resources explicit. File handles, process handles, and buffers should
  be visible owners or zone-backed handles.
- Prefer small modules: `std::env`, `std::fs`, `std::time`, `std::process`.

Likely compiler work:

- no new syntax should be needed
- runtime or C wrapper declarations may be needed for platform-specific calls
- shared-library and object-output behavior must stay on the LLVM path

### Phase 6: Library Developer Experience

- Add source-level test helpers when the language can express them.
- Build a library test runner around existing `@test` support when stable.
- Keep docs and test names readable enough that a new contributor can copy a
  nearby pattern.

## Module Backlog

| Module | Next Useful Slice | Tests To Add First | Compiler Work If Needed |
| --- | --- | --- | --- |
| `std::option` | Inspect-style helpers after borrowed function-pointer ergonomics are clear. | Predicate/filter/combinator/conversion/flatten/transpose behavior plus wrong-payload negative tests. | Generic enum method specialization diagnostics. |
| `std::result` | Error conversion helpers that use `From`/`Into` after trait impl patterns mature. | `Result` projection/conversion, transpose, eager/lazy fallback, and `?` residual tests. | Residual conversion and trait-bound selection. |
| `std::mem` | Safer copy/fill helpers for copyable values. | Scalar, aggregate, and owner-rejection tests. | Layout service and ownership-aware raw memory checks. |
| `std::zone` | Scoped allocation helpers after the raw `alloc_array<T>` buffer helper. | Reset/destroy provenance, raw array allocation, and escape diagnostics. | Zone lifetime/state merge rules. |
| `std::boxed` | Clarify final unique-owner direction. | Empty-handle, drop, same-zone, and pointer-provenance tests. | Generic drop and allocation-zone wrapper tracking. |
| `std::string` | Add signed/checked parsers only after text and numeric policies are documented. | Search, growth, append, copy, ASCII case comparison/search, ASCII trim/parse, prefix parse, owned trim copy, and after-reset tests. | Formatting/string runtime hooks. |
| `std::ascii` | Add signed parsers only after numeric sign and overflow policy is documented. | Byte classification behavior, case-insensitive comparison/search, slice trimming/parsing, prefix parser consumed-length behavior, source symbol checks, and future parser edge cases. | None for current whole-slice and prefix helpers; signed/checked parsers may need overflow diagnostics. |
| `std::vec` | Iterator/adaptor growth and root/source Vec unification plan after safe accessors. | Method, `try_*` access, iterator, borrow, owner-drop, and same-zone tests. | Iterator lowering and generic aggregate monomorphization. |
| `std::iter` | Adapter traits after collection iterators are stable. | Direct iterator, `IntoIterator`, and refutable-pattern diagnostics. | General iterator protocol lowering. |
| `std::fmt` | Source trait impls for common values. | `format_in!`, `Display`, unsupported-type diagnostics. | Macro-to-trait lowering cleanup. |
| `std::cmp` | Derived comparison impl coverage for more aggregate shapes. | Generic helper, inclusive range predicate, and derive interaction tests. | Trait-bound static dispatch and derive expansion. |
| `std::convert` | Concrete `From`/`Into` impl patterns and fallible conversion policy. | Identity/from/into behavior, explicit associated calls, and residual conversions. | Trait coherence and inference diagnostics. |
| `std::math` | Grow natural helper names from i64 signatures into documented numeric policy slices. | Sign predicate behavior, integer helper behavior, signed division rounding, overflow-policy diagnostics, and future checked/wrapping helpers. | Overflow intrinsics or diagnostics only after the source policy is designed. |
| `std::bits` | Grow natural helper names from u64 signatures into generic integer helpers. | Mask behavior, rotate count handling, power-of-two rounding, low-mask widths, alignment preconditions, zero/one-run scan edge cases, and future overflow-policy diagnostics. | Optional bit-scan intrinsics only after the source policy is stable. |

## API Landing Checklist

Before landing a public library API:

- Add or update source in `lib/std.arih` or `lib/std/<module>.arih`.
- Add focused positive tests in `tests/cases/standard-library/ok/`.
- Add negative diagnostics in `tests/cases/standard-library/errors/` for misuse.
- Add IR or executable checks in `tests/Makefile` when behavior reaches the
  backend.
- Update `tests/std_api_manifest.txt`.
- Update [Standard Library](../language/standard-library.md) or a focused
  language page.
- Update [Feature Test Matrix](test-matrix.md) when compiler semantics,
  backend lowering, ownership, borrowing, or ABI behavior changes.
- Run `make check-std-api` and the narrowest affected check target; prefer
  `make check-sanitize` for parser, sema, ownership, or codegen changes.
