# Standard Library Roadmap

This page summarizes the library implementation plan. The detailed compiler
roadmap remains in `docs/dev/standard-library-roadmap.md`.

## Phase 1: Stabilize The Current Seed

- Keep `lib/std.arih` as the public root.
- Keep child modules under `lib/std/`.
- Keep all public declarations in `tests/std_api_manifest.txt`.
- Keep focused tests under `tests/cases/standard-library/`.
- Keep this `docs/stdlib/` folder current with every public API.

Current source families: `option`, `result`, `mem`, `zone`, `boxed`,
`string`, `vec`, `iter`, `fmt`, `cmp`, `convert`, `context`, `input`, `io`,
and the first `math` integer helper slice.

## Phase 2: Pull More Behavior Into Ari Source

- Replace compiler hooks with source code when generic aggregates, trait
  dispatch, and module cache summaries can model them safely.
- Keep compiler-known declarations as compatibility shims only when required.
- Improve diagnostics for partial custom `std` packages.

## Phase 3: Grow Collections And Text

- Stabilize `std::vec::Vec[T]` and document the distinction from bare local
  `Vec[T]`.
- Add collection helpers in small slices: slice methods, vector methods,
  iterator adapters, then maps/sets/deques.
- Keep `std::string::String` byte-oriented until a Unicode/text policy is
  designed.

## Phase 4: Numerics

- Expand `std::math` from i64 helpers to width-specific integer helpers.
- Add checked, saturating, and wrapping operations only after overflow policy
  is documented.
- Add floating helpers only where LLVM lowering and runtime behavior are
  stable.

## Phase 5: OS-Facing Libraries

- Add `std::env`, `std::fs`, `std::time`, and `std::process` as thin explicit
  wrappers after C FFI conventions are stable.
- Keep handles visible and owned; do not hide OS resources behind global state.

## Phase 6: Library Developer Experience

- Add source-level test helpers when the language can express them.
- Build richer `@test` integration around library test cases.
- Keep examples short enough to serve as contributor templates.
