# Generic Aggregate Monomorphization

This page is the compiler-maintainer contract for generic structs, enums, and
type aliases. The goal is general language support: `Vec`, `Option`, `Result`,
and maps should work because generic aggregate handling is correct, not because
those names are special-cased.

## Current Readiness

| Area | Status | Notes |
| --- | --- | --- |
| Generic structs | supported | Explicit and inferred construction, field substitution, methods, field access, pass/return, and ownership-qualified type arguments are covered. |
| Generic enums | supported | Explicit, inferred, and expected constructors; generic payload substitution; match bindings; aggregate enum layout; and mixed concrete aggregate payload storage are covered. |
| Generic type aliases | supported | Generic aliases expand before identity, layout, and codegen. Recursive aliases are rejected. |
| Nested generic types | supported | Concrete keys and typed IR include nested applications such as `PassResult[Box[Token], Maybe[Diagnostic]]`. |
| Generic methods and receivers | supported | Receiver type identity is concrete; mismatched receivers are rejected. |
| Ownership/drop through generic payloads | supported | `Box[own i64]` style payloads preserve qualifiers, detect use-after-move, and drop active aggregate/enum payloads. |
| Stdlib generic fixtures | stress coverage | `Vec`, `Option`, `Result`, and `HashMap` are fixtures, not correctness targets. |
| Recursive value aggregates | unsupported by design | Direct value recursion is rejected with a stable diagnostic; pointer/zone indirection is required before layout can be finite. |
| External aggregate ABI | partial | Local/codegen paths are covered; public C/extern ABI stays under aggregate ABI rules. |

## Implementation Map

- `src/parser.cpp`: parses generic declarations, type arguments, constructors,
  methods, and aliases.
- `src/sema.cpp`: owns type application checks, substitution, concrete type
  identity checks, generic method selection, enum constructor checking,
  ownership/drop analysis, and typed IR lowering.
- `src/type_semantics.cpp`: `same_type`, assignability, copy/owner/borrow
  classification.
- `src/enum_payload_layout.cpp`: generic/concrete enum payload slot layout.
- `src/layout.cpp`: aggregate size/alignment used by sema and codegen.
- `src/llvm_codegen.cpp`: concrete aggregate and aggregate-enum lowering.
- `src/ir.cpp`: stable human-readable type names used in diagnostics and
  artifacts.

## Rules

Type application must validate:

- exact generic arity
- type arguments on non-generic types
- unknown type arguments and invalid type syntax
- nested type argument validity
- alias arity and recursive alias cycles
- value/type argument confusion at calls and constructors

Substitution must preserve the complete concrete type, including ownership and
borrow qualifiers. A bare `T` field in `Box[T]` becomes `own i64` for
`Box[own i64]`; it must not collapse to plain `i64`.

Concrete identity is:

```text
qualifier + nominal type name + ordered concrete type arguments
```

Aliases expand before comparison. `Box[Token]` and `Box[Span]` are different,
and two different struct names are different even when their fields have the
same shape.

Monomorphization must keep aggregate specialization separate from generic
function specialization. Generic function symbols use specialized function
names; concrete aggregate types use stable type names/layouts and do not need a
public symbol unless codegen emits a helper for a method or function.

Enum layout must be computed from concrete payloads. Generic declaration-time
placeholders may not force final layout decisions; the concrete application
must recompute payload slots after substitution.

## Diagnostics

Prefer diagnostics that name the concrete source-level type:

- `type mismatch: expected Box[Box[Token]], got Box[Box[Span]]`
- `type mismatch: expected Token, got LexError`
- `struct 'Box' has no field 'missing'`
- `unknown method 'id' for type Box[bool]`
- `cannot use moved owned field 'value.0'`
- `recursive aggregate value type 'Node[T]' requires indirection`

These are more valuable than only testing arity errors because they prove
substitution, identity, method receiver selection, owner-path tracking, and
layout-cycle protection.

## Test Inventory

Positive fixtures:

- `tests/cases/generics/ok/generic-aggregate-monomorphization.ari`
- `tests/cases/generics/ok/generic-aggregate-stdlib-stress.ari`
- `tests/cases/generics/ok/generic-aggregate-recursive-pointer.ari`
- `tests/cases/compiler-development/ok/model/compiler-generic-aggregates.ari`

Negative fixtures:

- `tests/cases/generics/errors/generic-aggregate-field-mismatch.ari`
- `tests/cases/generics/errors/generic-aggregate-nested-mismatch.ari`
- `tests/cases/generics/errors/generic-aggregate-enum-payload-mismatch.ari`
- `tests/cases/generics/errors/generic-aggregate-field-access.ari`
- `tests/cases/generics/errors/generic-aggregate-method-receiver.ari`
- `tests/cases/generics/errors/generic-aggregate-use-after-move.ari`
- `tests/cases/generics/errors/generic-aggregate-recursive-struct.ari`
- `tests/cases/generics/errors/generic-aggregate-recursive-enum.ari`
- `tests/cases/generics/errors/generic-aggregate-recursive-growth.ari`

Artifact coverage:

- `tests/cases/compiler-development/artifact/ok/generic-aggregate-monomorphization.ir`
- `tests/cases/compiler-development/artifact/errors/diagnostic-generic-aggregate-nested-mismatch.diagnostic`
- `tests/cases/compiler-development/artifact/errors/diagnostic-generic-aggregate-recursive-struct.diagnostic`

The typed IR artifact is generated with `--no-implicit-std` to keep it focused
on user generic aggregates and to make concrete type names, nested match
payloads, and owned generic fields visible. The diagnostic artifacts pin
source-aware spans for a nested concrete identity mismatch and the intentionally
unsupported direct recursive value layout case.

## Adding A New Case

1. Prefer a user-defined aggregate first. Use stdlib containers only after the
   compiler behavior is already exercised without them.
2. Include at least one nested type application.
3. Include construction, function argument passing, function return, field
   access, and match or method receiver behavior.
4. If ownership is relevant, use a small `own i64` payload and prove
   move/drop/use-after-move behavior.
5. Add a negative diagnostic when the change fixes or protects a semantic
   boundary.
6. Add a typed IR or LLVM check when layout, symbol naming, or emission order
   is part of the behavior.
7. Run focused checks:

```sh
make check-generics
make check-compiler-development
make check-compiler-artifacts
```
