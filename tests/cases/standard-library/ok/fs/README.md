# std::fs Positive Tests

These cases cover the first `std::fs` slice:

- runtime-backed file existence and removal hooks
- value `File` handles returned by mode-string open calls
- `Option[File]` wrappers for fallible mode-string open operations
- byte read/write helpers over a filesystem handle
- source whole-file convenience helpers for byte-slice write, append, and
  byte-string reads
- create/truncate/copy/read convenience helpers built over mode-string opens
- append-mode opens that preserve existing bytes and create missing files
- read/write mode strings such as `"rw"`, plus familiar `"r+"`, `"w+"`,
  and `"a+"` aliases

Keep filesystem tests deterministic. Use paths under `build/prelude/`, clean up
created files, and do not depend on host-specific absolute paths.

Files in this folder:

- `std-fs-basic.ari`: raw handle existence/open/read/write/close/remove
  behavior.
- `std-fs-append.ari`: append mode and preservation of existing bytes.
- `std-fs-open-modes.ari`: supported and rejected mode-string behavior.
- `std-fs-read-write.ari`: source `write`, `append`, and `read_to_string`
  whole-file conveniences.
- `std-fs-create-truncate-copy.ari`: source `create`, `try_create`, `read`,
  `truncate`, and `copy` helpers.
