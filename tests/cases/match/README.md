# Match And Patterns Tests

This folder contains focused fixtures for Ari match and patterns behavior. Put
valid programs under `ok/` and expected diagnostics under `errors/` when both
kinds exist. Representative enum constructor, match expression typing,
exhaustiveness, duplicate/unreachable arm validation, payload-pattern arity,
or-pattern binding validation, non-matchable scrutinees, unknown/wrong enum
cases, tuple-struct positional arity, struct-pattern failures, and enum payload
ownership/drop failures also have golden diagnostic artifacts under
`tests/cases/compiler-development/artifact/errors/`.

Wire new cases into the matching target in `tests/Makefile` and keep each file centered on one behavior.
