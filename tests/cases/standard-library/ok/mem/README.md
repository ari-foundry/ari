# Memory Helpers Standard Library Tests

This folder contains positive tests for the `mem` standard-library feature
group. Keep each `.ari` file focused on one API family or lowering behavior,
and update `tests/Makefile` whenever the fixture should be part of automated
checks.

- `std-mem-value-helpers.ari`: source `replace`/`swap` behavior for scalar and
  plain aggregate values.
- `std-mem-byte-ops.ari`: byte-level `copy_bytes`, `move_bytes`, and
  `set_bytes` behavior plus LLVM memory intrinsic lowering.
