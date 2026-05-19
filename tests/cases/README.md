# Test Cases

Feature fixtures for the Ari compiler live here. Most feature folders split valid programs into `ok/` and diagnostics into `errors/`. Standard library fixtures have an extra module-style grouping under `standard-library/`.

Keep each test focused on one behavior and wire it into the narrowest target in `tests/Makefile`.
