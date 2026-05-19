# IO Standard Library Tests

This folder contains positive tests for the `io` standard-library feature
group. Keep each `.ari` file focused on one API family or lowering behavior,
and update `tests/Makefile` whenever the fixture should be part of automated
checks.

- `std-io-byte-slice.ari`: raw stdout byte-slice output over the backend
  `write_byte` hook.
- `std-io-traits-cursor.ari`: `Reader`/`Writer`/`Seek`, `Cursor`, `stdin`,
  `stdout`, `read_exact`, `write_all`, and `flush`.
- `std-io-buffered.ari`: `BufReader`/`BufWriter` with caller-provided
  buffers, exact reads, write buffering, explicit flush, and stdout output.
