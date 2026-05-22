# Ari Test Layout

The test suite is grouped by feature first, then by expected outcome.

| Location | Meaning |
| --- | --- |
| `tests/cases/<feature>/ok/` | Valid Ari programs. Makefile targets compile them, run them, or inspect generated LLVM. |
| `tests/cases/<feature>/errors/` | Invalid Ari programs. Makefile targets assert the expected diagnostic text. |
| `tests/cases/compiler-development/ok/model/` | Normal Ari programs that model compiler data, pass flow, readiness gates, and test classification. |
| `tests/cases/compiler-development/artifact/ok/` | Golden text artifacts and source fixtures that should compare cleanly. |
| `tests/cases/compiler-development/artifact/errors/` | Expected diagnostic artifacts and text-comparator mismatch reports. |
| `tests/cases/compiler-development/errors/` | Invalid compiler-development source fixtures, such as bootstrap-only syntax that must stay rejected. |
| `tests/cases/bootstrap-readiness/` | Compiler-shaped Ari fixtures used before a real `bootstrap/` tree exists. |
| `tests/packages/` | File-backed module and module-cache fixtures. |
| `tests/ffi/` | C helper sources used by FFI and object-linking tests. |
| `tests/fixtures/` | Alternate module roots and intentionally incomplete fixtures. |
| `tests/tools/` | LSP, lint, and editor integration smoke checks. |
| `tests/std_api_manifest.txt` | Public `lib/std` API manifest with coverage notes. |
| `tests/bootstrap_readiness_manifest.txt` | Planned compiler-bootstrap readiness fixture groups. |
| `tests/compiler_development_manifest.txt` | General compiler-development gates and the test style expected for each gate. |
| `tests/check_language_docs.py` | User-facing documentation smoke check for docs-only Ari usage and test navigation. |
| `tests/check_bootstrap_readiness_docs.py` | Documentation smoke check for the production compiler design, bootstrap start-gate, and self-host roadmap links. |

Feature case directories:

- `attributes`
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
- `compiler-<model-or-artifact>.ari` for compiler-development fixtures that
  model normal compiler data, pass flow, test classification, or artifact
  behavior without creating a `bootstrap/` tree.
  Examples: `compiler-readiness-scorecard.ari`,
  `compiler-development-dashboard.ari`, `compiler-test-classification.ari`,
  `compiler-onboarding-workflow.ari`, `bootstrap-class-keyword.ari`.

See `docs/dev/library-testing.md` for the full standard library testing policy.

## Focused Targets

Use the narrowest target that matches the changed surface:

| Target | Scope |
| --- | --- |
| `make check-language-docs` | Language docs, docs-only reading path, and test-layout navigation. |
| `make check-compiler-dev-docs` | Compiler roadmap, maturity gates, pass contracts, project model, source diagnostics, and artifact-testing docs. |
| `make check-source-map-unit` | Direct C++ SourceMap API checks for line/column, EOF, CRLF, UTF-8 byte columns, invalid spans, multi-file ids, and snippets. |
| `make check-layout-unit` | Direct C++ layout and non-local aggregate ABI checks for primitive sizes/alignments, field offsets, aggregate enum storage, and target classifier reasons. |
| `make check-compiler-artifacts` | Deterministic fixture inventory, capability inventory, source-map, token, syntax, diagnostic, module graph, declaration index, typed IR, ownership/drop, pass-summary, LLVM-fragment, object/shared-symbol, and runtime-output artifacts. |
| `make check-compiler-development` | Ari fixtures that model compiler pass worklists, diagnostics, source maps, readiness gates, onboarding, and test classification as ordinary language code. |
| `make check-core-language` | Executable core language smoke tests plus representative stable diagnostics for functions, locals, operators, casts, blocks, branches, loops, `break`, `continue`, and returns. |
| `make check-traits` | Minimum static trait subset plus trait objects, impl conformance, generic trait dispatch, compiler-shaped Eq/Hash/Debug/Ord fixtures, and trait diagnostics. |
| `make check-ownership` | Fast ownership/borrow/drop smoke for aggregate moves, reborrowing, compiler-shaped owner flow, active enum payload drop lowering, and representative ownership diagnostics. |
| `make check-bootstrap-docs` | Later bootstrap start-gate docs and fixture-group manifest. |
| `make check-bootstrap-readiness` | Small pre-bootstrap Ari fixtures under `tests/cases/bootstrap-readiness/`. |

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
`make check-bootstrap-docs` only verifies that the bootstrap readiness guide
keeps its production-language contract, readiness scorecard, start gate, first
implementation slices, fixture groups, roadmap, estimate, and test-plan
sections linked from the developer docs.

`make check-bootstrap-readiness` compiles only the small pre-bootstrap Ari
fixtures under `tests/cases/bootstrap-readiness/`. It is meant to stay fast and
focused; it should prove one compiler-shaped pressure point at a time, not run
the whole test suite.

`make check-compiler-development` is different from
`make check-bootstrap-readiness`: it protects normal compiler-development
language pressure, not a bootstrap implementation. Add fixtures there when a
large Ari tool would need clearer data modeling, error flow, artifact
classification, or pass-state representation.

## README Placement

Keep README files at navigation boundaries such as `tests/`, `tests/cases/`,
`tests/cases/<feature>/`, and `tests/cases/standard-library/`. Do not add
README files inside `ok/`, `errors/`, or their leaf feature folders; those
directories should stay lightweight and be explained by focused test file names,
`tests/Makefile`, `tests/std_api_manifest.txt`, and the standard library docs.
