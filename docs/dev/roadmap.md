# Roadmap

This page tracks unfinished work only. Completed compiler milestones are kept in
[Completed Milestones](completed-milestones.md), and the supported language
surface is documented in the language guide and [Feature Test Matrix](test-matrix.md).

Keep roadmap items small enough to land in a 0.x series. Do not describe an
item as 1.0 work unless the whole language release is being scoped.

## Near-Term Compiler Work

These are the next compiler-sized slices that should be possible without
changing the long-term language contract.

1. [ir-replay-generics] Replay generic free functions and generated impl
   specializations from V0 IR sidecars once their stable specialization identity
   is versioned. Keep trait-specialized replay and broader identity descriptor
   expansion behind the same 0.x cache-version policy.
2. [abi-aggregate-classification] Define non-local aggregate ABI
   classification for public tuples, arrays, structs, vectors, and aggregate
   enums. This should settle the policy needed before raw C aggregate imports,
   richer C headers, and library-owned collection handles grow further.

See [Semantic Checker Decomposition](sema-decomposition.md) for the maintenance
roadmap for splitting `src/sema.cpp` by broad semantic phases.

## Medium-Term Compiler Work

1. Expand IR package-cache replay after `[ir-replay-generics]`.
   The current V0 cache family should stay V0 until a deliberate cache version
   bump is approved. The next useful work after generic/impl replay is
   trait-specialized replay and richer identity-bearing descriptors.
2. Keep sema extraction phase-oriented.
   Prefer broad modules such as type inference, pattern semantics, ownership
   state, zone provenance, and IR lowering helpers. Avoid splitting one tiny
   file per syntax feature.

## Medium-Term Language Work

1. Finish the remaining pattern binding-mode surface.
   Implement `&` / `&mut` shorthand, runtime-sequence reference rest bindings,
   enum-payload reference bindings, and ownership-aware binding modes through
   aggregate, enum, slice, and vector patterns. Keep `let`/`var`, match,
   control-flow, for-loop, and function-parameter patterns on the same shared
   binding-mode engine.
2. Expand aggregate enum payload storage.
   Current aggregate enum payloads intentionally cover a narrow ABI-safe
   universe. Add tuple, struct, vector, and owned payload values only after
   their non-local ABI/storage rules are defined. Define payload-bearing
   `@repr(C)` enum layout and C header emission after that ABI is stable.
3. Define owned root collection and smart-pointer handles.
   Define the growable root `Vec[T]` runtime-capacity ABI, permanent root
   method surface, and non-local aggregate layout. Future owning heap-style
   `Box[T]` should build on the same explicit-capability rules rather than
   inventing an ambient heap.
4. Extend trait-object ownership.
   Define durable data-pointer storage for `own` and borrow-valued dyn objects,
   including lifetime rules for objects that outlive hidden stack
   materialization.
5. Add an explicit owner-resolution surface.
   Loop exits that cannot prove a single owner state currently produce
   `maybe-unavailable` locals. A future language form should let users resolve
   those conditional cleanup states intentionally.

## Backend Work

1. Extend raw C imports after the scalar relocatable-object path.
   Add aggregate signatures, varargs, platform float-C ABI, libc discovery, and
   external aggregate-enum FFI once aggregate ABI classification is stable.
2. Add freestanding runtime string features beyond static literals.
   Add line-input buffers, owned-line allocation, allocator-backed string
   construction, and richer hosted IO compatibility.
3. Complete freestanding floating-point lowering.
   Add eventual `f128` values and foreign/platform C float ABI integration.

## Small Follow-Ups

- [raw-object-gnu-stack-note] Add a non-executable-stack marker section to raw
  relocatable objects so external linkers do not warn about a missing
  `.note.GNU-stack` section.

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
- mixing host/glibc codegen assumptions into the raw freestanding backend
