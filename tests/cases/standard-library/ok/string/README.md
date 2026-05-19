# String Standard Library Tests

This folder contains positive tests for the `string` standard-library feature
group. Keep each `.ari` file focused on one API family or lowering behavior,
and update `tests/Makefile` whenever the fixture should be part of automated
checks.

- `std-string-alloc-buffer.ari`: raw string buffer allocation provenance.
- `std-string-handle.ari`: metadata, capacity, fixed byte mutation, and handle
  basics.
- `std-string-first-last.ari`: endpoint byte access.
- `std-string-try-byte-access.ari`: Option-returning byte access and empty pop.
- `std-string-search.ari`: byte search and count helpers.
- `std-string-prefix-suffix.ari`: borrowed slice prefix/suffix checks.
- `std-string-equals.ari`: exact borrowed byte-slice comparison.
- `std-string-ascii-helpers.ari`: ASCII trim views and whole ASCII parsers.
- `std-string-ascii-case-helpers.ari`: ASCII-only case-insensitive comparison
  and search.
- `std-string-prefix-parsers.ari`: ASCII prefix parsers with consumed length.
- `std-string-trim-copy.ari`: owned copies from borrowed ASCII trim views.
- `std-string-grow.ari`: explicit-zone growth, reserve, insert, extend, and
  resize.
- `std-string-append.ari`: string, i64, and bool append helpers.
- `std-string-append-f32.ari`: f32 formatting append helper.
- `std-string-append-u64.ari`: u64 formatting append helper.
- `std-string-from-slice-in.ari`: owned copies from borrowed byte slices.
- `std-string-unicode-helpers.ari`: UTF-8 validation, code-point access, and
  scalar append convenience methods.
