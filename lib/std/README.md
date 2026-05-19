# Standard Library Sources

Each `.arih` file in this folder is a source module loaded by `std.arih`.
Keep APIs natural and capability-oriented: allocation-backed handles should
take an explicit `ref mut Zone`, OS-backed helpers should stay in small modules
such as `target`, `env`, `process`, `thread`, `sync`, `time`, and `fs`, and public names should be
mirrored in the stdlib docs and API manifest.
Source-only algorithm helpers live in `algo` so slice operations can grow
without bloating `Slice[T]` itself.
OS entropy and deterministic non-cryptographic PRNG helpers live in `random`;
keep cryptographic streams and richer distribution APIs out of this first
slice until error/result and byte-buffer policies are stronger.

Run `make build-lib` or `make check-lib` from the repository root when a source
library change should prove that `std` still loads through the library build
entry point. Those targets delegate to `lib/Makefile`, which keeps source
library build policy separate from the compiler and tool build rules.
