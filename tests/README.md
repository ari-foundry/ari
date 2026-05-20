# Ari Test Layout

The test suite is grouped by feature first, then by expected outcome.

| Location | Meaning |
| --- | --- |
| `tests/cases/<feature>/ok/` | Valid Ari programs. Makefile targets compile them, run them, or inspect generated LLVM. |
| `tests/cases/<feature>/errors/` | Invalid Ari programs. Makefile targets assert the expected diagnostic text. |
| `tests/packages/` | File-backed module and module-cache fixtures. |
| `tests/ffi/` | C helper sources used by FFI and object-linking tests. |
| `tests/fixtures/` | Alternate module roots and intentionally incomplete fixtures. |
| `tests/tools/` | LSP, lint, and editor integration smoke checks. |
| `tests/std_api_manifest.txt` | Public `lib/std` API manifest with coverage notes. |
| `tests/check_bootstrap_readiness_docs.py` | Documentation smoke check for the production compiler design, bootstrap start-gate, and self-host roadmap links. |

Feature case directories:

- `attributes`
- `borrowing`
- `constants`
- `control-flow`
- `ffi`
- `functions`
- `generics`
- `ir`
- `literals`
- `match`
- `memory`
- `meta`
- `modules`
- `operators`
- `ownership`
- `patterns`
- `standard-library`
- `structs`
- `traits`
- `variables`
- `vectors`

Within each feature directory, tests still use readable file prefixes:

- `std-<module>-<feature>.ari` for source `std` module behavior.
- `prelude-<feature>.ari` for implicit root aliases, macros, and root ADTs.
- `zone-<feature>.ari` for allocation, lifetime, and provenance hooks.
- `std-library-smoke.ari` for a small cross-library integration test.

See `docs/dev/library-testing.md` for the full standard library testing policy.

Documentation checks are intentionally small. For example,
`make check-bootstrap-docs` only verifies that the bootstrap readiness guide
keeps its production-language contract, estimate, start gate, roadmap, and
test-plan sections linked from the developer docs.

## README Placement

Keep README files at navigation boundaries such as `tests/`, `tests/cases/`,
`tests/cases/<feature>/`, and `tests/cases/standard-library/`. Do not add
README files inside `ok/`, `errors/`, or their leaf feature folders; those
directories should stay lightweight and be explained by focused test file names,
`tests/Makefile`, `tests/std_api_manifest.txt`, and the standard library docs.
