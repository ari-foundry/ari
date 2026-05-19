# Vec And Slice Standard Library Tests

This folder contains positive tests for the `vec` and root `Slice[T]`
standard-library feature group. Keep each `.ari` file focused on one API
family or lowering behavior, and update `tests/Makefile` whenever the fixture
should be part of automated checks.

- `prelude-slice-methods.ari`: root slice access, search, and exact
  prefix/suffix/equality helpers.
- `prelude-slice-sequence.ari`: root slice subslicing, split views,
  subsequence search, lexicographic compare, chunks, windows, and delimiter
  splitting.
- `prelude-slice-option-access.ari`: `Option`-returning empty/out-of-range
  access.
- `prelude-slice-copy-to.ari`: borrowed slice copy into a target zone-backed
  vector.
