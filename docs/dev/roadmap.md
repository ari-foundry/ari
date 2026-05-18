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

Source standard library planning is tracked in
[Standard Library Roadmap](standard-library-roadmap.md). Add compiler work here
only when a library slice needs parser, semantic checker, IR, runtime, or
backend changes that cannot be expressed in Ari source.

No active near-term compiler work is queued right now. Add the next concrete
0.x-sized compiler slice here when it is ready to implement.

## Backend Work

No active backend work is queued right now. Add the next concrete 0.x-sized
backend slice here when it is ready to implement.

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
- adding a second backend during the current 0.x compiler/library stabilization
  work
