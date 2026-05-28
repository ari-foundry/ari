# Compiler Roadmap

This page tracks active compiler work for the current C++ hosted compiler. It
is deliberately short: detailed implementation rules live in the focused docs
linked from [Developer Overview](README.md).

## Scope

The roadmap is for production compiler behavior:

- source identity, spans, snippets, and multi-file diagnostics
- lexer/parser reliability on malformed input
- semantic diagnostics, module flow, traits, generics, ownership, and ABI
- deterministic artifacts that make compiler regressions reviewable

This page is not a self-host implementation plan. Do not use it to create an
Ari-written compiler tree, bootstrap/stage directories, or a cargo-like build
tool. Standard-library/library maturity is tracked separately.
The few stdlib polish items that are blocked by compiler features are mapped in
[Compiler-Bound Standard Library Gaps](compiler-bound-stdlib-gaps.md).

## Near-Term Order

1. Function parameter patterns: parse, check, and lower pattern shapes in
   function parameter positions without weakening current ownership or
   diagnostics.
2. Runtime strings and floats: finish owned runtime string semantics and full
   float value behavior across typing, constants, formatting, and LLVM output.
3. Raw pointers and allocation zones: expose explicit pointer operations and
   allocation-zone diagnostics without adding a magical global heap.
4. General iterator protocol: move beyond compiler-known `range` loops toward a
   trait-backed iterator lowering path.
5. Closure and environment lowering: model captured values, move/drop paths,
   and call ABI well enough to unlock captured thread entries and broader
   callback APIs.
6. Generic thread results and compiler TLS: support generic `JoinHandle[T]`,
   result storage ownership, send/share diagnostics, and compiler-level
   `thread_local` declarations.
7. Variadic/default-zone formatting: decide between variadic generics,
   continued compiler-known format lowering, or both. Current-zone `format!`
   covers the local temporary allocation case; explicit `format_in!` remains
   the long-lived allocation form.

## Language Ideas Parked For Later

- Capability requirement extensions: associated types, operators, and field
  requirements remain explicit future design.
- `union by` extensions: non-enum selectors and public/stable ABI promises
  remain future design outside the executable enum/bool selector subset.

## What Not To Track Here

- Ari compiler rewrite tasks
- self-host stage plans
- bootstrap/stage1 directory layouts
- package manager or cargo-like tool work
- standard-library maturity
- broad kernel/freestanding roadmaps unless a narrow compiler ABI/backend fix is
  ready to implement now

## Work Loop

For each roadmap item:

1. Find the implementation path and existing focused tests.
2. Classify the gap as implementation missing, implementation buggy, or
   verification missing.
3. Fix the hosted compiler when behavior is missing or wrong.
4. Add one focused fixture or golden only after the behavior exists.
5. Run the narrow check for that layer.
6. Update the focused doc so the next compiler developer can follow the same
   path.

## Current Non-Goals

- class syntax
- hidden inheritance
- garbage collection
- C++ ABI dependency as a source-level FFI surface
- ambient global heap as a language primitive
- adding a second backend during current 0.x compiler stabilization
