# FFI Tests

This folder contains focused fixtures for Ari ffi behavior. Put valid programs under `ok/` and expected diagnostics under `errors/` when both kinds exist.

Wire new cases into the matching target in `tests/Makefile` and keep each file centered on one behavior.

ABI/layout boundary failures should also get a source-aware diagnostic artifact
when the message is part of the public compiler contract. The seeded example is
`tests/cases/compiler-development/artifact/errors/diagnostic-ffi-nonrepr-aggregate-import.ari`,
which locks the `A0001` diagnostic for rejecting a non-`@repr(C)` by-value
aggregate in an `extern "C"` import.
