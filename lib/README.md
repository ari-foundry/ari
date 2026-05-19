# Ari Libraries

This folder contains Ari source libraries shipped with the compiler. `std.arih` is the root standard library entry point, and child modules live under `lib/std/`.

When adding public APIs, update `tests/std_api_manifest.txt`, the docs under `docs/stdlib/`, and focused tests under `tests/cases/standard-library/`.

README files belong at library boundaries such as `lib/` and `lib/std/`, where
they explain ownership and navigation for a meaningful source area. Avoid
adding README files to every tiny module or generated/test outcome folder.

## Make Targets

Until Ari has a package manager, this folder owns the source-library build
entry point:

```sh
make -C lib build
make -C lib check
```

From the repository root, use:

```sh
make build-lib
make check-lib
```

`build` compiles the standard-library smoke program, writes module metadata and
a module cache under `build/lib/`, verifies the metadata, and builds a small
shared-library sample with LLVM IR, object, and C-header artifacts. If the
selected `LLVM_CC` exists, it also links `build/lib/libari_sample.so`.

To try a different Ari library source before a package manager exists:

```sh
make -C lib shared-library LIBRARY=path/to/api.ari LIBRARY_NAME=my_api
```

That writes `build/lib/libmy_api.ll`, `.o`, `.h`, and, when an LLVM driver is
available, `.so`. The `LIBRARY` path is relative to the repository root because
the recipe runs Ari from the root for stable implicit-`std` module loading. Set
`LIBRARY_SYMBOL=expected_symbol` to grep for a known export, or
`LIBRARY_SYMBOL=` to skip the symbol check for an ad hoc library.
