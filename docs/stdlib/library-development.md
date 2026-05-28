# Standard Library Development

This guide is for contributors adding or changing source libraries under
`lib/std`.

## Add An API

1. Decide whether the API can be implemented in Ari source.
2. Put public declarations in `lib/std.arih` or a child module in `lib/std/`.
3. Keep helpers private unless users need the name.
4. Add focused tests in `tests/cases/standard-library/ok/<feature>/`.
5. Add diagnostics in `tests/cases/standard-library/errors/<feature>/` when
   misuse should be rejected at compile time.
6. Add the test to `tests/Makefile`.
7. Update `tests/std_api_manifest.txt`.
8. Update this `docs/stdlib/` folder and any focused language docs.
9. Regenerate `docs/stdlib/generated/api-index.md` when the public API
   manifest changes.
10. Run `make check-stdlib-docs` when the API changes docs, stability policy,
   or module guide coverage.

Compiler changes belong in `src/` only when source Ari cannot model the
primitive. Examples include layout queries, raw pointer load/store,
`extern "ari"` runtime hooks, formatting macro parsing, and zone provenance.

## Source File Layout

Today, `.arih` and `.ari` are both parsed as Ari source. A file-backed
`mod name;` import chooses one module file, and it searches `name.ari` before
`name.arih`. The compiler does not yet merge a `name.arih` declaration file
with a sibling `name.ari` implementation file.

For `std`, keep each module's current public surface and source
implementation in the file that is actually loaded, usually
`lib/std/<module>.arih`. This keeps docs, manifest entries, and module-cache
metadata simple. If a module grows too large, split private implementation
details into a child module rather than assuming an automatic header/source
pair.

The intended longer-term direction is still friendly to paired files:
`<module>.arih` can become the public declaration and ABI surface, while
`<module>.ari` can hold larger private bodies. That compiler work should land
with explicit duplicate-declaration and body-matching diagnostics before std
depends on it.

## API Style

- Use module paths that say what owns the behavior: `std::vec::new`,
  `std::string::from_slice_in`, `std::math::gcd`.
- Do not add a type suffix when the module path, generic parameters, argument
  types, or return type already carry the type. Prefer `math::is_positive`,
  `bits::leading_ones`, or `convert::from<T, U>` over names like
  `is_positive_i64`, `leading_ones_u64`, or `from_i32_to_i64`.
- Keep type suffixes only for current runtime hooks or compatibility surfaces
  that cannot yet be generic, such as `io::write_i64` or `append_i64_in`.
  When the language can express a generic source API, add the natural name and
  keep the suffixed form only as a documented compatibility shim.
- For string formatting extensions, prefer `String.append_value(value)` or
  `append_value_in(ref mut Zone, value)` with `std::fmt::Display` over adding a
  new type-specific `append_*_in` method.
- For Writer-backed output, prefer `fmt::write_value(ref mut writer, ref mut
  zone, value)` with `Display` over adding another type-specific `write_*`
  helper. It returns `Result[(), Error]`; keep `_bool` wrappers for
  compatibility-only call sites that intentionally discard write errors. Keep
  `io::write_i64`-style names only for raw runtime hooks.
- For direct stdout output, prefer `fmt::print_value(ref mut zone, value)` or
  `fmt::println_value(ref mut zone, value)` over raw type-specific IO hooks.
- For equality and ordering in source `std`, prefer `==`, `!=`, `<`, `>`,
  `<=`, and `>=` over direct `.eq(...)` or `.lt(...)` calls. The operators
  preserve the public `cmp::Eq[T]` and `cmp::Ord[T]` contracts while keeping
  library code close to normal user code.
- Use `_in` when a function needs an explicit allocation region/zone.
- In new user-facing docs and APIs, prefer `Region` for choosing a bulk
  lifetime and `Allocator` for follow-up growth from existing region-backed
  handles. Use `std::allocator::from_region(ref mut region)` to pass an
  allocation capability without passing reset/destroy authority. Use `Zone`
  and `ZoneMetadata` only when touching compatibility, compiler/runtime hooks,
  or low-level implementation details.
- Prefer `Option` or `Result` for ordinary absence or recoverable failure.
- Prefer `std::error::Error`/`ErrorKind` for shared OS, runtime, IO,
  filesystem, network, or parser failures. Public library APIs should return
  `Result[T, Error]` when they need a shared failure value. Keep
  `Result[T, i64]` only at runtime, FFI, and compatibility boundaries, and use
  `error::map_raw`/`error::to_raw` when adapting those shapes.
- Use `assert` only for programmer errors and current precondition traps.
- Keep mutating methods on `self: ref mut Self`.
- Keep read-only methods on `self: ref Self`.
- Read-only methods can be chained from non-owning temporary values. Prefer
  `path.as_slice().equals("src")` over introducing a temporary only to satisfy
  a borrowed receiver. Bind a local when the receiver is mutable or owns
  resources.
- Do not add hidden allocation. Every allocation must flow through an explicit
  `Region`/`Zone` owner or an `Allocator` capability derived from one.
- Do not add compiler-tooling-only APIs such as source maps, source locations,
  labels, fix-its, or diagnostic report builders to runtime `std`; keep them
  in compiler/tooling packages.

## Stability Review

Before calling a public API usable or stable, classify it with the production
readiness tiers:

- core: portable source-first APIs
- alloc: explicit-zone or collection APIs
- hosted: APIs that need OS/runtime state
- platform: ABI, C, target, or future raw OS surfaces
- experimental: APIs that need more ownership, error, or runtime design first

Every tier decision should be reflected in `docs/stdlib/production-readiness.md`
or the focused module guide. The detailed labels and deprecation policy live in
`docs/stdlib/stability.md`; platform, CI, and fuzz/property coverage live in
`docs/stdlib/verification-matrix.md`. If the answer is
"only a future compiler would use this", it is probably not a runtime `std`
API.

Every public child module in `lib/std/*.arih` needs a focused guide under
`docs/stdlib/modules/`. `std::option` and `std::result` intentionally share
`option-result.md`; other modules should have a guide named after the module.

## Source Comments

Use comments for invariants that are not obvious from the code, especially:

- why a helper remains compiler-known
- what a private helper preserves during growth, copy, or drop
- why an API is intentionally narrow
- which future language feature should replace a temporary design

Avoid comments that restate a single line of code.

## Public API Manifest

`make check-std-api` compares public declarations against
`tests/std_api_manifest.txt`. When the manifest fails, run:

```sh
python3 tests/check_std_api_manifest.py --print
```

Copy only the new entries you meant to expose, then replace the placeholder
note with the focused test and docs coverage. Keep entries sorted.

Then regenerate and check the browsable API index:

```sh
python3 tools/generate_std_api_docs.py
python3 tools/generate_std_api_docs.py --check
```

## Library Build Entry Point

Until Ari grows a package manager, use the Makefile under `lib/` as the
library build boundary:

```sh
make build-lib
make check-lib
```

`make build-lib` compiles the source `std` smoke program, emits module metadata
and a module cache, verifies the metadata, and builds a tiny shared-library
sample with LLVM IR, object, and C-header artifacts under `build/lib/`.
`make check-lib` also runs the public API manifest check. For a one-off Ari
library source, run:

```sh
make -C lib shared-library LIBRARY=path/to/api.ari LIBRARY_NAME=my_api
```

The `LIBRARY` path is relative to the repository root. This is not a package
format yet; it is the temporary build contract that keeps library work out of
the root compiler Makefile until a Cargo-like tool exists. Set
`LIBRARY_SYMBOL=expected_symbol` for a known export, or `LIBRARY_SYMBOL=` for
an ad hoc library where a symbol check is not useful.

## Review Checklist

- The API has a clear module owner.
- Public names are documented in `docs/stdlib/api-reference.md`.
- Allocation and ownership behavior is visible in the signature.
- Positive behavior has an executable or LLVM check.
- Misuse has a negative test when it can be caught before runtime.
- `make check-std-api` passes.
- `python3 tools/generate_std_api_docs.py --check` passes when the manifest
  changed.
- `make check-stdlib-docs` passes when docs or stability policy changed.
- A narrow build/test target passes before broader `make check`.
