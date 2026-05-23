# FFI Tests

This folder contains focused fixtures for Ari ffi behavior. Put valid programs under `ok/` and expected diagnostics under `errors/` when both kinds exist.

Wire new cases into the matching target in `tests/Makefile` and keep each file centered on one behavior.

ABI/layout boundary failures should also get a source-aware diagnostic artifact
when the message is part of the public compiler contract. Current examples
include
`tests/cases/compiler-development/artifact/errors/diagnostic-ffi-extern-abi.diagnostic`,
`diagnostic-ffi-extern-body.diagnostic`,
`diagnostic-ffi-extern-generic.diagnostic`,
`diagnostic-ffi-extern-invalid-link-name.diagnostic`,
the `diagnostic-ffi-extern-varargs-*` fixtures,
`diagnostic-ffi-extern-void-param.diagnostic`,
`diagnostic-ffi-nonrepr-aggregate-import.ari`,
`diagnostic-ffi-large-aggregate-import.diagnostic`, and
`diagnostic-ffi-target-aggregate-import.diagnostic`, which lock `A0001`
diagnostics for rejecting invalid extern declarations, invalid external link
symbols, unsupported varargs surfaces, `c_void` parameters, and non-`@repr(C)`,
oversized, or target-unsupported by-value aggregates in `extern "C"` imports.
