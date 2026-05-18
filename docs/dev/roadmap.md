# Roadmap

This page tracks unfinished work only. Completed compiler milestones are kept in
[Completed Milestones](completed-milestones.md), and the supported language
surface is documented in the language guide and [Feature Test Matrix](test-matrix.md).

Keep roadmap items small enough to land in a 0.x series. Do not describe an
item as 1.0 work unless the whole language release is being scoped.

## Near-Term Compiler Work

These are the next compiler-sized slices that should be possible without
changing the long-term language contract.

Phase-oriented sema decomposition is now tracked as ongoing maintenance in
[Semantic Checker Decomposition](sema-decomposition.md) instead of as a finite
near-term deliverable.

1. Add owner-aware `Slice[T]` element paths.
   Treat `Slice[own T]` as a non-owning view while still preserving borrow and
   move diagnostics for element paths that are reached through the slice.
   Label: `slice-owner-element-paths`.
2. Add dynamic value-vector suffix owner paths.
   Support value patterns that move unknown-length `Vec[own T]` suffix elements
   without relying on hidden whole-value leaks.
   Label: `dynamic-value-vector-suffixes`.
3. Extend trait-object ownership.
   Define durable data-pointer storage for `own` and borrow-valued dyn objects,
   including lifetime rules for objects that outlive hidden stack
   materialization.
4. Add an explicit owner-resolution surface.
   Loop exits that cannot prove a single owner state currently produce
   `maybe-unavailable` locals. A future language form should let users resolve
   those conditional cleanup states intentionally.

## Backend Work

1. Keep LLVM object output library-ready.
   Preserve explicit symbol names, C-header compatibility, and module-cache
   replay behavior for object files produced with `--emit-obj`.

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
- non-local ownership for bare root `Vec[T]` without an explicit allocation
  capability
- adding a second backend before the LLVM path is library-ready
