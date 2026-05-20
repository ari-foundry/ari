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

## Module Tier Policy

Production `std` should make it clear which APIs are always available and which
ones depend on a hosted OS or a specific platform family.

| Tier | Modules | Stability Expectation |
| --- | --- | --- |
| core | `option`, `result`, `cmp`, `convert`, `math`, `bits`, `ascii`, `parse`, `encoding`, root `Slice[T]` | Source-first APIs that should stay portable and easy to test without OS state. |
| alloc | `zone`, `boxed`, `string`, `vec`, `collections`, `iter`, `algo`, `hash`, `random::Prng` | APIs that need explicit `Zone` allocation or collection invariants. They must document provenance and reset/destroy behavior. |
| hosted | `io`, `input`, `env`, `fs`, `process`, `thread`, `sync`, `time`, `random::entropy`, `random::fill` | APIs that require a hosted runtime, libc, OS handles, clocks, threads, or entropy. They must document handle ownership and failure behavior. |
| platform | `target`, `c`, future `os`, future socket handles in `net` | APIs that expose ABI, loader, errno, syscall, descriptor, or target-specific behavior. They must say which target family they describe. |
| experimental | future `net` sockets, future `os`, backtrace, benchmark, fuzzing, async, compression | APIs that should remain roadmap work until ownership, error, and platform policies are tested. |

Moving an API from experimental to usable requires docs, manifest coverage,
focused tests, and a short note explaining the failure and ownership policy.

## Failure Policy

Use natural names for the common path and make the failure shape visible:

- Use `try_*` for `Option`-returning absence, such as missing values, EOF, or
  unsupported metadata.
- Use `Result[T, E]` for recoverable failures once the payload shape is
  supported for the relevant value.
- Use `std::error::Error` for shared OS, IO, filesystem, network, process, and
  runtime categories.
- Use tuples only for product values that are always produced, such as
  `overflowing_add` returning `(value, overflowed)`.
- Use `assert`, `panic`, `todo`, and `unreachable` only for programmer errors,
  internal invariants, or deliberate test assertions.

APIs should not return sentinel integers when `Option`, `Result`, or a small
named struct would make the call site clearer.

## Platform Policy

Platform APIs should be honest instead of pretending every target can do the
same thing:

- Portable APIs belong in core or alloc tiers.
- Hosted APIs should say whether they require Linux/glibc-like behavior today.
- Linux-specific APIs should live behind target facts or future `std::os`
  modules, not under broad portable names.
- Freestanding or kernel-facing work should be documented in runtime/platform
  roadmaps until startup, allocation, panic, atomics, and syscall ABI rules are
  explicit.
- Optional APIs such as `io_uring`, `fanotify`, namespaces, seccomp,
  capabilities, compression, benchmark hooks, and fuzz hooks should stay
  optional until their failure behavior is clear.

## API Acceptance Checklist

A public API is ready for normal use when all of these are true:

- Purpose: the docs say why the API exists and which module owns it.
- Name: the call reads naturally without unnecessary type suffixes.
- Signature: allocation, ownership, borrowing, and platform requirements are
  visible.
- Failure: ordinary absence or OS failure uses `Option`, `Result`, or
  `std::error::Error`.
- Tests: focused ok tests cover the main behavior, and errors tests cover the
  first static misuse when the compiler can reject it.
- Manifest: `tests/std_api_manifest.txt` has a coverage note for the public
  declaration.
- Docs: `docs/stdlib/api-reference.md` and the focused module guide mention
  the API.
- Build: `make check-std-api` and `make check-stdlib-docs` pass.

## Compatibility And Deprecation

Compatibility wrappers are allowed when the language or runtime is still
catching up, but the docs should teach the preferred API first.

Use this migration order:

1. Add the natural API and focused tests.
2. Keep the older compatibility wrapper when existing tests or users need it.
3. Mark the wrapper as compatibility-only in docs or roadmap notes.
4. Remove the wrapper only after a migration note and replacement tests exist.

Examples: keep raw `io::write_i64`-style hooks as low-level compatibility
surfaces while teaching `fmt::write_value`, `fmt::print_value`, and
`println!("{value}")` style formatting as the normal API.

## Non-Goals For Runtime Std

Runtime `std` should not absorb every compiler/tooling idea. These belong in
compiler, tooling, platform, or optional packages instead:

- compiler source maps, source locations, labels, fix-its, and diagnostic
  report builders
- bootstrap-only helpers that ordinary Ari programs would never use
- hidden global heaps or hidden allocation in convenience functions
- raw syscall collections without owned descriptor/error policy
- platform probes that contradict compile-time `std::target` facts
- kernel/freestanding startup internals before the runtime contract is stable

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

## Module Status Snapshot

This is a coarse map for planning, not a replacement for the API manifest:

| Family | Current Level | Next Hardening Slice |
| --- | --- | --- |
| `Option`/`Result`, `cmp`, `convert`, `math`, `bits`, `ascii`, `parse`, `encoding` | usable | More negative tests around trait bounds, overflow, and malformed encodings. |
| `zone`, `boxed`, `string`, `vec`, `collections`, `iter`, `algo`, `hash`, `random::Prng` | usable | More same-zone, reset/destroy, iterator invalidation, and trait-driven constructor tests. |
| `io`, `input`, `env`, `fs`, `process`, `thread`, `sync`, `time` | seed to usable | Stronger owned handle policy, richer `Result` error values, and platform notes. |
| `target`, `c`, `os`, future socket handles in `net` | seed | Split portable facts from raw platform handles and document ABI constraints. |
| backtrace, benchmark, fuzzing, async, compression | planned | Keep out of runtime `std` until runtime, driver, and error contracts are ready. |
