# Standard Library Roadmap

This page summarizes the library implementation plan. The detailed compiler
roadmap remains in `docs/dev/standard-library-roadmap.md`.

## Phase 1: Stabilize The Current Seed

- Keep `lib/std.arih` as the public root.
- Keep child modules under `lib/std/`.
- Keep all public declarations in `tests/std_api_manifest.txt`.
- Keep focused tests under `tests/cases/standard-library/`.
- Keep this `docs/stdlib/` folder current with every public API.

Current source families: `option`, `result`, `mem`, `zone` raw allocation plus
source typed array allocation, `boxed`, `string` byte access/search/ASCII
helpers including case search and owned trim copies, `ascii` byte classification,
case-insensitive comparison/search, and slice helpers, `vec`, `iter`, `fmt`,
`cmp` comparison helpers, `convert`
identity/from/into helpers, `context` runtime hooks plus the source
`has_arg` helper, `input` runtime hooks plus the source `try_read_byte` EOF
helper, `io` runtime hooks plus source byte-slice output, and the first
`math` arithmetic/division-rounding and `bits` numeric helper slices.

## Phase 2: Pull More Behavior Into Ari Source

- Replace compiler hooks with source code when generic aggregates, trait
  dispatch, and module cache summaries can model them safely.
- Keep compiler-known declarations as compatibility shims only when required.
- Improve diagnostics for partial custom `std` packages.

## Phase 3: Grow Collections And Text

- Stabilize `std::vec::Vec[T]` and document the distinction from bare local
  `Vec[T]`.
- Keep shared collection access vocabulary aligned across `Slice[T]` and
  `std::vec::Vec[T]`, including `try_*` methods for `Option`-based absence.
- Add collection helpers in small slices: slice methods, vector methods,
  iterator adapters, then maps/sets/deques.
- Keep `std::string::String` byte-oriented until a Unicode/text policy is
  designed.
- Expose small `String` conveniences only when they preserve byte-string
  semantics, such as ASCII case comparison/search, borrowed ASCII trim views,
  owned trim copies, and whole-string ASCII parsers.
- Keep ASCII-only helpers in `std::ascii` so byte-oriented classification,
  comparison, search, trimming, and parsing behavior is explicit at call sites.

## Phase 4: Numerics

- Expand `std::math` from i64 signatures to generic numeric helpers when the
  language has the right trait vocabulary. Preserve the existing natural names
  for signs, parity, powers, division rounding, and divisor helpers.
- Expand `std::bits` from u64 signatures to generic integer mask, rotation,
  power-of-two, low-mask, and bit-scan helpers when the same trait vocabulary
  exists.
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
