# Generics Tests

This folder contains focused fixtures for Ari generics behavior. Put valid programs under `ok/` and expected diagnostics under `errors/` when both kinds exist.

Wire new cases into the matching target in `tests/Makefile` and keep each file
centered on one behavior.

Generic aggregate coverage is split on purpose:

- `ok/generic-function-compiler-shaped.ari` uses compiler-shaped `Span`,
  `Token`, `Diagnostic`, `Box`, `Maybe`, and `PassResult` values to exercise
  generic function declarations, explicit and inferred type arguments, nested
  generic specialization keys, generic locals, generic aggregate parameters and
  returns, repeated specialization reuse, typed IR, LLVM symbols, and runtime
  behavior.
- `ok/generic-function-declaration-policy.ari` locks declaration edges that are
  easy to forget: unused type parameters are allowed but require explicit type
  arguments when they cannot be inferred, return-only type parameters require
  explicit type arguments at call sites, and every explicit type argument still
  participates in the backend specialization key.
- `ok/generic-function-modules.ari` keeps generic function names identical
  across two inline modules and checks public module-qualified inferred and
  explicit specializations produce distinct stable backend symbols.
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
- `errors/generic-*.ari` also covers duplicate function type parameters,
  explicit argument count and mismatch errors, insufficient and conflicting
  inference, generic function pointer inference/signature failures, non-generic
  calls with type arguments, return-context inference limits, declaration-time
  rejection of unbound generic signature types, generated specialization name
  collisions, and private generic function access.
