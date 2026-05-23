# Test Cases

Feature fixtures for the Ari compiler live here. Most feature folders split
valid programs into `ok/` and diagnostics into `errors/`. Standard library
fixtures have an extra module-style grouping under `standard-library/`.

Keep each test focused on one behavior and wire it into the narrowest target in `tests/Makefile`.

## Naming Rules

Test names should tell reviewers what the file protects before they open it:

- `char-literal-escapes.ari`: lexer/parser spelling or literal behavior.
- `module-private-import.ari`: module resolution or visibility behavior.
- `generic-enum-payload.ari`: generic aggregate behavior.
- `ownership-drop-insertion.ari`: ownership or drop-lowering behavior.
- `diagnostic-parser-expected.ari`: source input for a committed diagnostic
  artifact.

Use `ok/` for programs that should compile, emit an artifact, or run. Use
`errors/` for programs that should fail with a stable diagnostic. Use
`artifact/ok` or `artifact/errors` only when the committed text file is the
thing being compared.

README files stop at this feature-map level. Keep `ok/`, `errors/`, and their
leaf test folders free of README files so the fixture tree stays easy to scan.
Use descriptive `.ari` filenames and the relevant docs page for per-case notes.
