# Library Testing

This page explains how standard library tests are organized. It is meant for
contributors adding APIs under `lib/std.arih` or `lib/std/`.

## Test Layers

| Layer | Purpose | Where |
| --- | --- | --- |
| API manifest | Prevent public `std` APIs from changing accidentally. | `tests/std_api_manifest.txt`, checked by `make check-std-api`. |
| Positive source behavior | Compile and run valid library use. | `tests/cases/standard-library/ok/std-<module>-<feature>.ari`, `tests/cases/standard-library/ok/prelude-<feature>.ari`. |
| Negative diagnostics | Lock down misuse and unsupported surfaces. | `tests/cases/standard-library/errors/std-<module>-<feature>.ari`, `tests/cases/standard-library/errors/prelude-<feature>.ari`. |
| Backend checks | Inspect LLVM, symbols, runtime hooks, and executable output. | `tests/Makefile` under the relevant check target. |
| Cross-library smoke | Exercise several library families together. | `tests/cases/standard-library/ok/std-library-smoke.ari`. |
| Docs coverage | Explain user-visible behavior and remaining limits. | `docs/language/standard-library.md`, focused language docs, `docs/dev/test-matrix.md`. |

## Naming Scheme

Use file names that identify both the library family and the behavior under
test:

- `std-vec-try-pop.ari`: source `std::vec` method behavior.
- `std-string-append-u64.ari`: source `std::string` formatting helper.
- `std-math-integer-helpers.ari`: source `std::math` i64 helper behavior.
- `std-bits-mask-helpers.ari`: source `std::bits` u64 mask and alignment
  helper behavior.
- `std-boxed-as-ptr-after-reset.ari`: negative zone provenance diagnostic.
- `prelude-option-result-methods.ari`: root prelude ADT method behavior.
- `prelude-macro-format-no-default-zone.ari`: macro or implicit prelude
  diagnostic.
- `zone-promote-return.ari`: compiler-known zone lifetime behavior.
- `std-library-smoke.ari`: cross-module integration smoke.

Prefer one focused behavior per file. If a test intentionally covers a family
of tiny methods, make that clear in the name, such as `metadata-methods`,
`fixed-ops`, or `slice-compare`.

## Check Targets

Run these depending on the change:

- `make check-std-api`: public API manifest only.
- `make check-prelude`: source `std`, prelude aliases, formatting, zones,
  `Box`, `String`, and source `Vec` coverage.
- `make check-ffi`: C FFI, pointer, layout, and explicit zone backend coverage.
- `make check`: full default suite after compiler or public API changes.
- `make check-sanitize`: preferred for parser, semantic checker, ownership,
  zone provenance, or codegen changes.

## Adding A Library Test

1. Put valid programs under `tests/cases/<feature>/ok/`; put diagnostics under `tests/cases/<feature>/errors/`.
2. Use a file name that starts with the library category.
3. Add the test to the narrowest `tests/Makefile` target.
4. For runtime behavior, compile to an executable and check the exit code or
   stdout.
5. For lowering behavior, emit LLVM and grep for stable function names,
   builtin calls, visibility, or layout operations.
6. If the test introduces a public API, update `tests/std_api_manifest.txt`
   with a coverage note.
7. Update user-facing docs when behavior is visible to Ari programmers.

## What To Test For Libraries

Every public library API should eventually have:

- a simple positive call
- an executable result check
- a source path check through `std::module::name`
- a prelude/root alias check when the API is re-exported
- negative diagnostics for wrong types, wrong arity, invalid ownership, or
  invalid zone use
- LLVM checks when the API depends on runtime hooks, layout, ABI, or symbol
  names
- ownership and borrowing interaction tests when the API moves, drops, borrows,
  or returns references

Do not add broad tests that only prove "the whole library works". Add one
cross-library smoke test for integration confidence, then keep detailed tests
small and local.
