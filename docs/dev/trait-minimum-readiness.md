# Minimum Trait Readiness

This page defines the production-ready trait subset Ari relies on for normal
compiler-shaped code today. It is not a promise that the whole future trait
system is complete. The goal is narrower: user-defined traits, impl
validation, static dispatch, trait bounds, comparison, hashing, formatting,
and iterator-shaped helpers must be deterministic and testable without
stdlib-specific shortcuts.

## Supported Subset

The minimum supported subset is:

- trait declarations with method signatures
- concrete impls and generic impls
- bare `self` inference to the impl target type
- value, shared-borrow, and mutable-borrow receiver checking
- impl conformance for missing, extra, duplicate, mismatched, and generic
  trait methods
- duplicate and overlapping impl rejection
- concrete method-call static dispatch
- trait-qualified method calls such as `Trait::method(value)`
- trait bounds on generic functions, generic impls, structs, and enums
- static dispatch from a bound generic parameter
- single trait applications in bounds, including trait arguments such as
  `Eq[T]`
- supertrait requirements and unique inherited method lookup
- trait-backed `==`, `!=`, `<`, `>`, `<=`, and `>=` through `eq` and `lt`
- stdlib `cmp::Eq[T]`, `cmp::Ord[T]`, `hash::Hash[T]`, `fmt::Debug`,
  `fmt::Display`, `Iterator[T]`, and `IntoIterator[T]` in their documented
  current forms

This is enough for compiler-shaped Ari code to model token-kind equality,
source-position ordering, symbol hashing, token/diagnostic debug text,
work-item scoring, generic helper functions, and iterator-like traversal.

## Unsupported Or Larger-Scope Forms

The following are intentionally outside the minimum production subset:

- specialization
- negative impls
- blanket impl solving beyond Ari's conservative generic coherence checks
- multiple bounds on one type parameter
- higher-ranked bounds and lifetime solving
- implicit dynamic dispatch
- object-safe dispatch for generic trait methods
- unrelated `dyn`-to-`dyn` casts
- automatic trait-driven default constructors for hash collections

Trait objects, associated type projections, and iterator lowering have focused
tests, but they remain separate capability areas. The minimum subset is the
static trait path that normal compiler data structures should use first.

## Coverage Audit

| Area | Classification | First Checks |
| --- | --- | --- |
| Trait declarations | already supported | `make check-traits` |
| Concrete impl validation | already supported | missing/extra/duplicate/mismatch fixtures in `tests/cases/traits/errors/` |
| Generic impl validation | already supported | `generic-trait-impl`, `generic-impl-bounds`, coherence errors |
| Trait method static dispatch | already supported | `trait-method-call`, `trait-bound-static-dispatch` |
| Trait-qualified calls | already supported | `trait-qualified-call` |
| Generic functions with bounds | already supported | `trait-bound-static-dispatch`, `trait-minimum-compiler-shaped` |
| Generic impl bounds | already supported | `generic-impl-bounds` |
| Generic aggregate bounds | newly covered | `structural-capability-struct-bound`, `structural-capability-enum-bound` |
| User-defined Eq/Hash/Debug/Ord-like fixtures | newly covered | `trait-minimum-compiler-shaped` |
| Stdlib Eq/Ord/Hash/Debug/Iterator-like fixtures | newly covered in the trait target | `trait-minimum-stdlib-like` |
| Compiler-shaped trait data | newly covered | token, source position, symbol, token cursor, work item fixtures |
| Missing impl diagnostics | already supported, artifact-covered | `trait-bound-missing-impl`, `diagnostic-trait-bound-missing-impl` |
| Ambiguous method diagnostics | already supported, artifact-covered | `trait-method-ambiguous`, `diagnostic-trait-method-ambiguous` |
| Receiver mismatch diagnostics | newly covered | `trait-impl-receiver-mismatch` |
| Trait objects | partially supported by design | focused `trait-object-*` fixtures |
| Associated types | supported seed, broader solving partial | associated type projection fixtures |
| Multiple bounds and specialization | unsupported by design | documentation only |

## Dispatch Rules

Trait method calls are statically selected unless the source program
explicitly asks for a `dyn Trait` value. For a concrete receiver, Ari searches
visible inherent and trait impl methods whose receiver type matches the call.
If exactly one trait impl method applies, lowering records the concrete impl
method in typed IR. If multiple visible trait impls provide the same method for
the same receiver, the call is rejected as ambiguous.

For a generic parameter with a bound, the bound is part of the function's
contract. A call such as `value.debug_score()` inside
`fn dump[T: Debuggable](value: T)` resolves through the bound and is checked
again when the function is specialized for a concrete type. If the concrete
type does not implement the required trait application, the call site fails
with a missing-bound diagnostic.

Generic impl bounds behave the same way. `impl[T: Debuggable] Debuggable for
DebugBox[T]` becomes available only for concrete `DebugBox[X]` instantiations
where `X` implements `Debuggable`.

## Diagnostics

Stable negative coverage includes:

- unknown trait in an impl or bound
- wrong trait arity
- duplicate trait declarations and duplicate methods
- duplicate or overlapping impls
- impl missing a required method
- impl declaring an extra method
- parameter, receiver, generic-parameter, and return-type mismatches
- missing impl for a generic bound
- ambiguous method calls and ambiguous trait-qualified calls
- supertrait requirements without the required parent impl
- non-object-safe generic trait methods under `dyn` dispatch

Diagnostic artifacts under `tests/cases/compiler-development/artifact/errors/`
lock the missing-bound and ambiguous-call shapes so tooling can compare stable
source locations and messages.

## Adding Tests

Put focused positive trait programs under `tests/cases/traits/ok/` and
diagnostic programs under `tests/cases/traits/errors/`. Use compiler-shaped
names when the fixture models token, span, source-position, symbol, diagnostic,
or pass-worklist behavior. Wire each case into `make check-traits`.

Add typed IR artifacts only when the selected impl method or generic
specialization is the behavior under review. Add diagnostic artifacts when the
source location or message shape matters for contributor tooling.
