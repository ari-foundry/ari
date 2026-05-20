# Standard Library Production Readiness

This page defines the quality bar for moving Ari's standard library from a
broad prototype into a dependable systems-programming library. It complements
the API reference and roadmap: the roadmap says what to build, while this page
says what "done enough to rely on" means.

## Release Bar

Every public `std` API should satisfy these rules before it is treated as
stable:

- The API name reads naturally at call sites and avoids type suffixes when the
  module path or generic type already carries the meaning.
- Ordinary failure returns `Option` or `Result`; `panic`, `assert`, `todo`, and
  `unreachable` are reserved for programmer errors or deliberately asserting
  helpers.
- Owned resources have explicit ownership, close/drop/reset behavior, and
  documented invalid-handle behavior.
- Zone-backed values keep their provenance tracked through wrappers,
  `Option`, and `Result`-style enum payloads so reset/destroy diagnostics stay
  reliable.
- Platform APIs document whether they are portable, Linux-specific, hosted
  libc-backed, or future freestanding/kernel-only work.
- Each API has a focused executable or IR test under
  `tests/cases/standard-library/`, a manifest entry in
  `tests/std_api_manifest.txt`, and user-facing docs in `docs/stdlib/`.
- Tests cover both the happy path and the first ordinary failure path.
- Compatibility wrappers may exist, but docs should teach the preferred
  natural API first.

## Stability Levels

Use these labels in roadmap notes when a module is being hardened:

| Level | Meaning |
| --- | --- |
| seed | The module exists and its shape is useful, but edge cases and resource policy are still moving. |
| usable | The main APIs have focused tests, docs, and ordinary failure behavior. |
| hardened | Resource lifetimes, platform differences, invalid inputs, and representative edge cases are tested. |
| stable | The API should not change without migration notes and compatibility wrappers. |

## Current Priorities

1. Prefer fallible value-returning APIs over validation-plus-panicking pairs for
   user input, while keeping asserting helpers for trusted/internal paths.
2. Harden resource handles in `fs`, `io`, `process`, `thread`, and future
   `net`/`os` with explicit invalid, close, ownership, and error behavior.
3. Keep text/path/C string/OS string boundaries distinct and document
   conversions before adding convenience wrappers.
4. Split freestanding-safe `core` behavior from hosted `std` behavior before
   kernel-grade work starts.
5. Turn broad roadmap items into small focused slices with a test, docs, and a
   commit for each slice.
