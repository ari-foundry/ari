# Match And Patterns Tests

This folder contains focused fixtures for Ari match and patterns behavior. Put
valid programs under `ok/` and expected diagnostics under `errors/` when both
kinds exist. Representative enum constructor, exhaustiveness, unknown-case, and
struct-pattern failures also have golden diagnostic artifacts under
`tests/cases/compiler-development/artifact/errors/`.

Wire new cases into the matching target in `tests/Makefile` and keep each file centered on one behavior.
