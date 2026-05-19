# Formatting Standard Library Tests

This folder contains positive tests for the `format` standard-library feature group. Keep each `.ari` file focused on one API family or lowering behavior, and update `tests/Makefile` whenever the fixture should be part of automated checks.

- `std-fmt-format-spec.ari` covers source `std::fmt::FormatSpec` construction, unsigned radix formatting, width/precision/alignment, debug text quoting, explicit-zone strings, and `io::Writer` output.
- `format-print*.ari` covers compiler-assisted formatting macro lowering and runtime output.
