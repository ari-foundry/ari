# Environment Standard Library Tests

This folder contains positive tests for the `env` standard-library feature group. Keep each `.ari` file focused on one API family or lowering behavior, and update `tests/Makefile` whenever the fixture should be part of automated checks.

- `std-env-args.ari`: argument count, optional argument, and program-name wrappers.
- `std-env-vars.ari`: current-process environment variable lookup and mutation.
- `std-env-paths.ari`: current directory, executable path, and cwd mutation hooks.
- `std-env-os-path-views.ari`: `OsStr` and `PathBytes` boundary views over env data.
