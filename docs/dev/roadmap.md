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

1. Define owned root collection and smart-pointer handles.
   Define the growable root `Vec[T]` runtime-capacity ABI and non-local
   aggregate layout before expanding source libraries that depend on
   ownership-stable collections. The stack-backed local root `Vec[T]` method
   surface is frozen, including `as_ptr()` for raw element-buffer access.
   Root `Vec[T]` function parameters and impl receivers lower through the
   existing Slice-shaped view ABI; root `Vec[T]` returns, struct fields, trait
   method returns, and extern signatures still need the runtime-capacity ABI.
   Source `std::Vec[T]`/`std::vec::Vec[T]` already follows the explicit-zone
   handle policy for allocation, same-zone growth, `Vec!` construction sugar,
   Drop of live elements, tracked `Slice` views, `get_ref()` / `get_mut()`
   element borrows, and `as_ptr()` / `as_mut_ptr()` raw element-buffer views.
   Source `Box[T]`/`std::Box[T]` already follows the explicit-zone handle
   policy, including `Box!(T, ref mut Zone, value)` construction sugar and
   tracked `as_ref()` / `as_mut()` value borrows plus `as_ptr()` /
   `as_mut_ptr()` raw views; future heap ownership should keep that
   capability-oriented shape rather than inventing an ambient heap.
2. Define dynamic owner pattern paths.
   Static local/hidden-storage `Vec[own T]` value patterns can now move exact
   elements and known-length suffix elements through tracked owner paths when
   every owned element is bound. Remaining work is the genuinely dynamic
   surface: owner moves through enum payload slots, `Slice[T]` element paths,
   owned rest aliases, owned-element skips, and unknown-length vector suffixes
   without relying on hidden whole-value leaks.
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
