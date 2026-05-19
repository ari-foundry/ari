# Ari Libraries

This folder contains Ari source libraries shipped with the compiler. `std.arih` is the root standard library entry point, and child modules live under `lib/std/`.

When adding public APIs, update `tests/std_api_manifest.txt`, the docs under `docs/stdlib/`, and focused tests under `tests/cases/standard-library/`.

README files belong at library boundaries such as `lib/` and `lib/std/`, where
they explain ownership and navigation for a meaningful source area. Avoid
adding README files to every tiny module or generated/test outcome folder.
