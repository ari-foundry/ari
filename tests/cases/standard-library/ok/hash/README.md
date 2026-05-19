# Standard Library Hash Tests

These cases cover source-only hashing helpers in `std::hash`.

Current files:

- `std-hash-basic.ari`: `Hasher` construction/reset/finalization, byte writes,
  byte-slice hashing, generic `Hash[T]` dispatch for primitive values, and the
  `collections::hash_i64` compatibility shim.
