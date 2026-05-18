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

1. Define dynamic owner pattern paths.
   Static local/hidden-storage `Vec[own T]` value patterns can now move exact
   elements and known-length suffix elements through tracked owner paths, and
   they drop selected `_` elements plus known skipped rest-gap elements. Local
   `let ref` / `let ref mut` suffix patterns over unknown-length direct
   `Vec[own T]` storage now use synthetic suffix owner paths when the vector is
   not partially moved, so distinct prefix/suffix mutable borrows can coexist.
   Remaining work is the genuinely dynamic owner-move surface: owner moves
   through enum payload slots, `Slice[T]` element paths, owned rest aliases, and
   unknown-length value vector suffixes without relying on hidden whole-value
   leaks.
   Owned payload-bearing `@repr(C)` enum C layouts should be defined with this
   ABI work; public non-generic plain payload enums already emit C headers for
   scalar, pointer-shaped, and generated wrapper-backed non-scalar payload
   slots.
2. Extend trait-object ownership.
   Define durable data-pointer storage for `own` and borrow-valued dyn objects,
   including lifetime rules for objects that outlive hidden stack
   materialization.
3. Add an explicit owner-resolution surface.
   Loop exits that cannot prove a single owner state currently produce
   `maybe-unavailable` locals. A future language form should let users resolve
   those conditional cleanup states intentionally.

## Medium-Term Language Work

Medium-term is currently empty; promote work here only after the near-term owner
and trait-object slices become too large for one 0.x step.

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
