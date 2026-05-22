# Generics Tests

This folder contains focused fixtures for Ari generics behavior. Put valid programs under `ok/` and expected diagnostics under `errors/` when both kinds exist.

Wire new cases into the matching target in `tests/Makefile` and keep each file
centered on one behavior.

Generic aggregate coverage is split on purpose:

- `ok/generic-aggregate-monomorphization.ari` uses only user-defined generic
  structs, enums, aliases, methods, nested payloads, and ownership-qualified
  arguments. This is the primary compiler fixture.
- `ok/generic-aggregate-stdlib-stress.ari` uses `Vec`, `Option`, `Result`, and
  `HashMap` only as large stdlib stress inputs.
- `ok/generic-aggregate-recursive-pointer.ari` proves recursive generic shapes
  are accepted when the recursive edge is an indirection instead of a value
  field.
- `errors/generic-aggregate-*.ari` covers semantic failures such as nested
  identity mismatch, payload mismatch, invalid field access, receiver mismatch,
  use-after-move through a generic payload, and unsupported recursive value
  aggregate layouts.
