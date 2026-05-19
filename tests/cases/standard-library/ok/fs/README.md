# std::fs Positive Tests

These cases cover the first `std::fs` slice:

- runtime-backed file existence and removal hooks
- value `File` handles returned by open calls
- `Option[File]` wrappers for fallible open operations
- byte read/write helpers over a filesystem handle
- append-mode opens that preserve existing bytes and create missing files

Keep filesystem tests deterministic. Use paths under `build/prelude/`, clean up
created files, and do not depend on host-specific absolute paths.
