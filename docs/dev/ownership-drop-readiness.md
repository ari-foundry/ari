# Ownership Drop Readiness

This page is the current contract for Ari ownership, borrow, and drop behavior.
It describes what is production-ready for the hosted compiler today, what is
artifact-covered, and which larger ownership shapes are still intentionally
rejected.

## Goals

Ownership checking should make large Ari programs boring to reason about:

- moves are explicit and use-after-move is rejected
- shared and mutable borrows use path-aware conflict checks
- owned aggregate fields are moved or dropped exactly once
- runtime enum payload cleanup drops only the active payload
- generic aggregate ownership follows the monomorphized concrete fields
- unsupported ownership shapes fail with source-aware diagnostics

## Production Contract

Scalar copy types such as integers and booleans remain usable after assignment
or argument passing. `own T` values and aggregates containing owned fields move
when they are passed, assigned, returned, pattern-bound by value, or explicitly
dropped. A moved owner cannot be used again unless the current language surface
supports reinitializing that exact place.

Borrow checking is path-aware. Multiple shared borrows can coexist, a mutable
borrow excludes shared and other mutable borrows of the same path, and field or
element reborrows keep the source borrowed until the returned borrow's visible
use ends. The checker intentionally remains conservative across loops and
control-flow joins.

`drop value;` consumes an owner. If the value type has a matching `Drop` impl,
the backend emits that call; otherwise primitive owners are semantically
dropped without a runtime call. Aggregates drop owned fields in a deterministic
field order. Runtime-dependent owner-carrying enum values test the active tag
before cleaning payload storage so inactive variants are not dropped.

## Coverage Inventory

| Area | Status | Coverage |
| --- | --- | --- |
| Local moves and use-after-move | complete | `make check-errors`, `diagnostic-use-after-move.diagnostic`, live-owner return/break/continue diagnostics |
| Copy vs move behavior | complete | scalar copy fixtures and owning `Token`/`Diagnostic` fixtures |
| Shared and mutable borrows | complete for current path model | `borrow-reborrow-paths.ari`, borrow conflict diagnostics |
| Field moves and partial moves | complete for supported local aggregate paths | `ownership-aggregate-field-move.ari`, partial-move diagnostics |
| Fixed array and local `Vec[own T]` element moves | complete for constant and tracked pattern paths | variables/ownership fixtures plus dynamic-index rejection |
| Whole aggregate moves | complete | `ownership-compiler-shaped.ari`, generic aggregate fixtures |
| Enum payload moves | partial | direct owner payloads and runtime active payload cleanup are covered; owning aggregate enum payloads remain rejected |
| Pattern and match ownership | complete for current pattern surface | match, `if let`, `while let`, vector pattern fixtures, and compact enum payload ref-pattern rejection |
| Generic aggregate ownership | complete for monomorphized fields in supported aggregate layouts | `Box[Token]`, `Slot[Token]`, and generic aggregate fixtures |
| Container ownership | partial | local `Vec` storage and std collection handle drops are covered; arbitrary dynamic element owner moves are rejected |
| Drop insertion and order | complete for current destructor model | LLVM fragments and focused runtime drop fixtures |
| Custom destructors | limited to `Drop::drop` impls | no separate destructor syntax exists |
| Ownership artifacts | partial by design | typed IR and LLVM fragments exist; no standalone move-analysis dump exists yet |

## Unsupported Patterns

These are rejected instead of silently weakening ownership rules:

- moving owning aggregate elements through dynamic indexes
- moving owners out of non-owning `Slice[own T]` views
- unknown-length owning vector rest aliases
- using a partially moved owning aggregate as a whole value
- aggregate enum payloads that themselves contain nested owned fields
- returning borrows from local values or ambiguous borrow sources
- whole raw-pointer copies of ownership-valued values

Unsupported cases should keep stable diagnostics in `tests/cases/ownership/errors/`
or `tests/cases/compiler-development/artifact/errors/`.

## Artifact Coverage

The artifact suite currently locks these ownership/drop surfaces:

- `ownership-aggregate-field-move.ir` for typed IR after sema has resolved
  field moves
- `diagnostic-use-after-move.diagnostic`
- `diagnostic-borrow-after-move.diagnostic`
- `diagnostic-double-move.diagnostic`
- `diagnostic-move-borrowed-owner.diagnostic`
- `diagnostic-assignment-while-borrowed.diagnostic`
- `diagnostic-field-assignment-while-borrowed.diagnostic`
- `diagnostic-return-live-owner.diagnostic`
- `diagnostic-loop-break-live-owner.diagnostic`
- `diagnostic-loop-continue-live-owner.diagnostic`
- `diagnostic-enum-payload-invalid-move.diagnostic`
- `diagnostic-compact-enum-payload-ref.diagnostic`
- `diagnostic-ownership-partial-move.diagnostic`
- `diagnostic-ownership-vector-dynamic-move.diagnostic`
- `diagnostic-ownership-temporary-element-move.diagnostic`
- `backend-ownership-drop-aggregate.llvm-frag`
- `backend-ownership-drop-runtime-enum.llvm-frag`
- `backend-ownership-compiler-shaped.llvm-frag`

`backend-ownership-compiler-shaped.llvm-frag` is extracted from
`ownership-compiler-shaped.ari` and covers a compiler-like mix of parser-state
borrows, generic `Box`/`Slot` field ownership, `Vec[WorkItem]` owner moves and
reinitialization, result-like enum matching, and deterministic `Drop` calls.
The diagnostic goldens above lock assignment-while-borrowed, field assignment
while a subpath is borrowed, borrow-after-move, double-move, live-owner
return/break/continue exits, invalid enum payload-slot moves, partial aggregate
moves, and dynamic indexed container owner moves as source-aware ownership
errors. The temporary aggregate element owner moves use the same O0001 artifact
family.

## Adding Tests

Prefer one narrow fixture per behavior:

- positive ownership and drop behavior: `tests/cases/ownership/ok/`
- borrow-only behavior: `tests/cases/borrowing/`
- stable diagnostics: `tests/cases/ownership/errors/`
- normalized source-aware diagnostics:
  `tests/cases/compiler-development/artifact/errors/`
- typed IR or backend shape:
  `tests/cases/compiler-development/artifact/ok/`

Use `make check-ownership` for the fast smoke, `make check-errors` for the
broader diagnostic matrix, and `make check-compiler-artifacts` when adding or
refreshing ownership artifacts.
