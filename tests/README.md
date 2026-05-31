# Ari Test Layout

The test suite is grouped by feature first, then by expected outcome.

| Location | Meaning |
| --- | --- |
| `tests/cases/<feature>/ok/` | Valid Ari programs. Makefile targets compile them, run them, or inspect generated LLVM. |
| `tests/cases/<feature>/errors/` | Invalid Ari programs. Makefile targets assert the expected diagnostic text. |
| `tests/cases/compiler-development/artifact/ok/` | Golden text artifacts and source fixtures that should compare cleanly. |
| `tests/cases/compiler-development/artifact/errors/` | Expected diagnostic artifacts and text-comparator mismatch reports. |
| `tests/cases/ari-compiler-bootstrap/` | Source-root smoke tests for the Ari-written compiler under `compiler/`. |
| `tests/cases/bootstrap-readiness/` | Small compiler-shaped Ari fixtures for hosted-compiler readiness; the Ari-written compiler implementation tree is `compiler/`. |
| `tests/packages/` | File-backed module and module-cache fixtures. |
| `tests/ffi/` | C helper sources used by FFI and object-linking tests. |
| `tests/fixtures/` | Alternate module roots and intentionally incomplete fixtures. |
| `tests/tools/` | LSP, lint, and editor integration smoke checks. |
| `tests/std_api_manifest.txt` | Public `lib/std` API manifest with coverage notes. |
| `tests/bootstrap_readiness_manifest.txt` | Hosted compiler-writing readiness fixture groups. |
| `tests/check_language_docs.py` | User-facing documentation smoke check for docs-only Ari usage and test navigation. |
| `tests/check_compiler_docs.py` | Documentation smoke check for the current C++ hosted compiler developer path. |

Feature case directories:

- `attributes`
- `ari-compiler-bootstrap`
- `bootstrap-readiness`
- `borrowing`
- `constants`
- `core-language`
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
- `compiler-<artifact>.ari` for source inputs that produce committed compiler
  artifacts such as token, syntax, diagnostic, source-map, typed IR, or LLVM
  fragments.

See `docs/stdlib/testing.md` for the full standard library testing policy.

## Focused Targets

Use the narrowest target that matches the changed surface:

| Target | Scope |
| --- | --- |
| `make check-language-docs` | Language docs, docs-only reading path, and test-layout navigation. |
| `make check-source-map-unit` | Direct C++ SourceMap API checks for line/column, EOF, CRLF, UTF-8 byte columns, invalid spans, multi-file ids, and snippets. |
| `make check-layout-unit` | Direct C++ layout and non-local aggregate ABI checks for primitive sizes/alignments, field offsets, aggregate enum storage, and target classifier reasons. |
| `make check-compiler-artifacts` | Deterministic fixture inventory, capability inventory, source-map, token, syntax, diagnostic, module graph, declaration index, typed IR, ownership/drop, pass-summary, LLVM-fragment, object/shared-symbol, and runtime-output artifacts. |
| `make check-core-language` | Executable core language smoke tests plus representative stable diagnostics for functions, locals, operators, casts, blocks, branches, loops, `break`, `continue`, and returns. |
| `make check-traits` | Minimum static trait subset plus trait objects, impl conformance, generic trait dispatch, compiler-shaped Eq/Hash/Debug/Ord fixtures, and trait diagnostics. |
| `make check-ownership` | Fast ownership/borrow/drop smoke for aggregate moves, reborrowing, compiler-shaped owner flow, active enum payload drop lowering, and representative ownership diagnostics. |
| `make check-compiler-docs` | Current compiler developer docs, roadmap, and readiness inventory. |
| `make check-ari-compiler-bootstrap` | Actual Ari-written compiler source-root checks under `compiler/` plus a small module-path smoke fixture. |
| `make check-bootstrap-readiness` | Small compiler-shaped Ari fixtures under `tests/cases/bootstrap-readiness/`. |

Use `python3 tests/check_compiler_artifact_cli.py` when changing artifact CLI
help, artifact listing, per-artifact explanations, combination rules, or error
text.
Use `python3 tests/check_compiler_capability_cli.py` when changing compiler
capability inventory listing or per-capability explanations.
Use `python3 tests/check_compiler_pass_cli.py` when changing compiler pass
catalog listing, per-pass explanations, or pass information command errors.
Use `python3 tests/check_compiler_test_bucket_cli.py` when changing compiler
test-bucket listing, per-bucket explanations, or fixture-placement guidance.
Use `python3 tests/check_compiler_work_item_cli.py` when changing compiler
work-item listing, per-item explanations, or implementation-roadmap guidance.
Use `python3 tests/check_compiler_diagnostic_cli.py` when changing diagnostic
catalog listing or diagnostic-code explanation behavior.
Use `python3 tests/check_compiler_target_cli.py` when changing target triple,
ABI fact, or `target("...")` predicate reporting.

Documentation checks are intentionally small. For example,
`make check-compiler-docs` verifies that the compiler developer overview,
roadmap, readiness inventory, test matrix, and docs index still point at the
current C++ hosted compiler workflow instead of stale self-host planning pages.

`make check-ari-compiler-bootstrap` checks the actual Ari-written compiler
source root under `compiler/` and compiles a tiny fixture against it with
`-Icompiler`.

`make check-bootstrap-readiness` compiles only the small compiler-shaped Ari
fixtures under `tests/cases/bootstrap-readiness/`. It is meant to stay fast and
focused; it should prove one hosted-compiler pressure point at a time, not run
the whole test suite.

## README Placement

Keep README files at navigation boundaries such as `tests/`, `tests/cases/`,
`tests/cases/<feature>/`, and `tests/cases/standard-library/`. Do not add
README files inside `ok/`, `errors/`, or their leaf feature folders; those
directories should stay lightweight and be explained by focused test file names,
`tests/Makefile`, `tests/std_api_manifest.txt`, and the standard library docs.
