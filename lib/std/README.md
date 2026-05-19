# Standard Library Sources

Each `.arih` file in this folder is a source module loaded by `std.arih`.
Keep APIs natural and capability-oriented: allocation-backed handles should
take an explicit `ref mut Zone`, OS-backed helpers should stay in small modules
such as `target`, `env`, `process`, `thread`, `sync`, `time`, and `fs`, and public names should be
mirrored in the stdlib docs and API manifest.
Source-only algorithm helpers live in `algo` so slice operations can grow
without bloating `Slice[T]` itself.
