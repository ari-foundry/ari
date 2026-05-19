# std::fs Positive Tests

These cases cover the first `std::fs` slice:

- runtime-backed file existence and removal hooks
- value `File` handles returned by mode-string open calls
- `Option[File]` wrappers for fallible mode-string open operations
- byte read/write helpers over a filesystem handle
- append-mode opens that preserve existing bytes and create missing files
- read/write mode strings such as `"rw"`, plus familiar `"r+"`, `"w+"`,
  and `"a+"` aliases

Keep filesystem tests deterministic. Use paths under `build/prelude/`, clean up
created files, and do not depend on host-specific absolute paths.
