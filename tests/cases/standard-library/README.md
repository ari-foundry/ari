# Standard Library Test Cases

This folder contains focused tests for the source `std` package and its
compiler-known hooks.

## Folders

- `ok/<feature>/`: programs that should compile. Many are also linked and
  executed from `tests/Makefile`.
- `errors/<feature>/`: programs that should fail with a stable diagnostic.

Feature folders use the library family as the directory name, for example
`ok/collections/`, `ok/string/`, `ok/vec/`, `errors/format/`, or
`errors/prelude/`. Keep new files in the narrowest feature folder instead of
adding them directly under `ok/` or `errors/`.

## Name Prefixes

- `std-<module>-<feature>.ari`: source module APIs such as
  `std-vec-try-pop.ari`, `std-math-integer-helpers.ari`, or
  `std-env-vars.ari`.
- `prelude-<feature>.ari`: root prelude names, aliases, macros, and implicit
  `std` loading.
- `format-<feature>.ari`: formatting macro and runtime-lowering behavior.
- `ari-builtin-<feature>.ari`: reserved `extern "ari"` builtin validation.
- `std-library-smoke.ari`: one cross-library integration program.

Prefer one behavior per test file. If a file covers a small method family,
make that family explicit in the name, such as `metadata-methods`,
`integer-helpers`, `slice-compare`, or `growth-paths`.

## Adding Tests

1. Add valid programs under `ok/<feature>/` and diagnostics under
   `errors/<feature>/`.
2. Add the file to the narrowest target in `tests/Makefile`, usually
   `check-prelude` for source `std`.
3. Check generated LLVM when the API has a public symbol or lowers through a
   runtime hook.
4. Run `make check-std-api` whenever public declarations change.
5. Update `tests/std_api_manifest.txt` and `docs/stdlib/` for public APIs.
