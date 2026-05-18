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
   checks, plus product-pattern irrefutability queries used before sema
   materializes hidden match storage. The shared pattern-alternative set keeps
   or-pattern detection and expansion together before sema lowers bindings.
   It also owns runtime-sequence reference-pattern planning for direct rest
   alias constraints, ownership-carrying rest-alias rejection, and known-length
   owner suffix guards before sema lowers the element borrows.
   Continue extracting broad modules such as declaration tables, name
   resolution, ownership state, zone provenance, and IR lowering helpers.
   `attribute_semantics` now owns built-in attribute classification,
   target/argument validation, and `@repr(C)` field/case guards,
   `c_export_semantics` now owns public `@repr(C)` record/enum C-header
   metadata assembly while sema supplies resolved types and enum layout, and
   `vector_semantics` owns root `Vec[T]` parameter ABI lowering into
   Slice-shaped views plus the remaining root-vector runtime ABI guards.
   `ownership_semantics` now owns recursive owned-field state seeding for
   locals and stack-backed vector storage. Avoid splitting one tiny file per
   syntax feature.

See [Semantic Checker Decomposition](sema-decomposition.md) for the maintenance
roadmap for splitting `src/sema.cpp` by broad semantic phases.

2. Define owned root collection and smart-pointer handles.
   Define the growable root `Vec[T]` runtime-capacity ABI and non-local
   aggregate layout before expanding source libraries that depend on
   ownership-stable collections. The stack-backed local root `Vec[T]` method
   surface is frozen, including `as_ptr()` for raw element-buffer access.
   Source `Box[T]`/`std::Box[T]` already follows the explicit-zone handle
   policy; future heap ownership should keep that capability-oriented shape
   rather than inventing an ambient heap.
3. Define dynamic owner pattern paths.
   After runtime-capacity `Vec[T]` and owned enum payload ABI rules are stable,
   define owner moves through enum payload slots, `Slice[T]` element paths,
   owned rest aliases, and dynamic vector suffixes without relying on hidden
   whole-value leaks.
   Non-scalar or owned payload-bearing `@repr(C)` enum C layouts should be
   defined with this ABI work; the current compiler covers public non-generic
   scalar/pointer-slot payload enums in C headers.

## Medium-Term Language Work

1. Extend trait-object ownership.
   Define durable data-pointer storage for `own` and borrow-valued dyn objects,
   including lifetime rules for objects that outlive hidden stack
   materialization.
2. Add an explicit owner-resolution surface.
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
