# Ari Libraries

This folder contains Ari source libraries shipped with the compiler. `std.arih` is the root standard library entry point, and child modules live under `lib/std/`.

When adding public APIs, update `tests/std_api_manifest.txt`, the docs under `docs/stdlib/`, and focused tests under `tests/cases/standard-library/`.

README files belong at broad source-library boundaries such as `lib/`, where
they explain ownership and navigation for a meaningful source area. Avoid
adding README files to child module folders or generated/test outcome folders.

`lib/std/` is intentionally covered by this README. Each `.arih` file there is
a source module loaded by `std.arih`. Keep APIs natural and
capability-oriented: allocation-backed handles should take an explicit
`ref mut Zone`, OS-backed helpers should stay in focused modules such as
`target`, `env`, `process`, `thread`, `sync`, `time`, and `fs`, and public
names should be mirrored in the stdlib docs and API manifest. Source-only
algorithm helpers live in `algo` so slice operations can grow without bloating
`Slice[T]` itself. OS entropy and deterministic non-cryptographic PRNG helpers
live in `random`; keep cryptographic streams and richer distribution APIs out
of this first slice until error/result and byte-buffer policies are stronger.

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
