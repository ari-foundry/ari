# Standard Library Development

This guide is for contributors adding or changing source libraries under
`lib/std`.

## Add An API

1. Decide whether the API can be implemented in Ari source.
2. Put public declarations in `lib/std.arih` or a child module in `lib/std/`.
3. Keep helpers private unless users need the name.
4. Add focused tests in `tests/cases/standard-library/ok/`.
5. Add diagnostics in `tests/cases/standard-library/errors/` when misuse
   should be rejected at compile time.
6. Add the test to `tests/Makefile`.
7. Update `tests/std_api_manifest.txt`.
8. Update this `docs/stdlib/` folder and any focused language docs.

Compiler changes belong in `src/` only when source Ari cannot model the
primitive. Examples include layout queries, raw pointer load/store,
`extern "ari"` runtime hooks, formatting macro parsing, and zone provenance.

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
- Use `_in` when a function needs an explicit allocation zone.
- Prefer `Option` or `Result` for ordinary absence or recoverable failure.
- Use `assert` only for programmer errors and current precondition traps.
- Keep mutating methods on `self: ref mut Self`.
- Keep read-only methods on `self: ref Self`.
- Do not add hidden allocation. Every allocation must flow through `Zone`.

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

## Review Checklist

- The API has a clear module owner.
- Public names are documented in `docs/stdlib/api-reference.md`.
- Allocation and ownership behavior is visible in the signature.
- Positive behavior has an executable or LLVM check.
- Misuse has a negative test when it can be caught before runtime.
- `make check-std-api` passes.
- A narrow build/test target passes before broader `make check`.
