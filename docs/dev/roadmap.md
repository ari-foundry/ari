# Roadmap

This page tracks unfinished work only. Completed compiler milestones are kept in
[Completed Milestones](completed-milestones.md), and the supported language
surface is documented in the language guide and [Feature Test Matrix](test-matrix.md).

Keep roadmap items small enough to land in a 0.x series. Do not describe an
item as 1.0 work unless the whole language release is being scoped.

## Near-Term Compiler Work

These are the next compiler-sized slices that should be possible without
changing the long-term language contract.

1. Keep sema extraction phase-oriented.
   `pattern_semantics` already owns pure pattern tree queries, or-pattern
   expansion, positional product mapping, and runtime-sequence irrefutability
   checks, with the shared pattern-alternative set keeping or-pattern detection
   and expansion together before sema lowers bindings. Continue extracting
   broad modules such as type inference, ownership state, zone provenance, and
   IR lowering helpers. Avoid splitting one tiny file per syntax feature.
2. Finish the remaining pattern binding-mode surface.
   Nested shared reference binding modes now work through local/function
   parameter destructuring plus enum statement/expression `match` and enum
   `if let`/`while let` patterns, including enum `while let` or-pattern
   alternatives. Pattern macro output now feeds the same expanded pattern path
   before match/or-pattern lowering and reference-binding detection. Mutable
   enum payload reference bindings now work in enum statement/expression
   `match`, enum `if let`, and enum `while let` when the matched subject is an
   addressable local, field, or indexed element. Mutable tuple, fixed-array,
   and struct control-flow reference bindings now borrow the original
   addressable subject while hidden product storage drives pattern tests, and
   mutable runtime-sequence control-flow reference bindings now borrow
   addressable `Slice[T]`/`Vec[T]` subjects while hidden sequence storage drives
   length and element tests. Finish ownership-aware binding modes through
   aggregate, enum, slice, and vector patterns.
   Keep `let`/`var`, match, control-flow, for-loop, and function-parameter
   patterns on the same shared binding-mode engine.
3. Expand aggregate enum payload storage.
   Current aggregate enum payloads intentionally cover a narrow ABI-safe
   universe. Add tuple, struct, vector, and owned payload values only after
   their non-local ABI/storage rules are defined. Define payload-bearing
   `@repr(C)` enum layout and C header emission after that ABI is stable.

See [Semantic Checker Decomposition](sema-decomposition.md) for the maintenance
roadmap for splitting `src/sema.cpp` by broad semantic phases.

## Medium-Term Language Work

1. Define owned root collection and smart-pointer handles.
   Define the growable root `Vec[T]` runtime-capacity ABI, permanent root
   method surface, and non-local aggregate layout. Future owning heap-style
   `Box[T]` should build on the same explicit-capability rules rather than
   inventing an ambient heap.
2. Extend trait-object ownership.
   Define durable data-pointer storage for `own` and borrow-valued dyn objects,
   including lifetime rules for objects that outlive hidden stack
   materialization.
3. Add an explicit owner-resolution surface.
   Loop exits that cannot prove a single owner state currently produce
   `maybe-unavailable` locals. A future language form should let users resolve
   those conditional cleanup states intentionally.

## Backend Work

1. Keep LLVM object output library-ready.
   Preserve explicit symbol names, C-header compatibility, and module-cache
   replay behavior for object files produced with `--emit-obj`.

## Small Follow-Ups

- [llvm-object-fixtures] Add one minimal external-link fixture around
  `--emit-obj` when the library ABI surface grows beyond scalar exports.

## Bootstrap Direction

1. Keep the C++ implementation compact while the language design stabilizes.
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
- adding a second backend before the LLVM path is library-ready
