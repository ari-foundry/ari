# Feature Status

This page is the docs-only map for Ari language features. Use it when you want
to know whether a feature is executable today, front-end only, or planned, and
which focused page or test family to inspect next. Use
[Feature Crosswalk](feature-crosswalk.md) when you need the matching example,
test bucket, and smallest check command for a feature.

Status meanings:

- **implemented**: parsed, checked, lowered, and covered by at least one
  executable or artifact path.
- **partial**: useful today, but some combinations still need diagnostics,
  lowering, ABI, or ergonomic work.
- **front-end only**: parsed or checked, but not fully executable.
- **planned**: reserved design direction.

## Executable Core

| Feature | Status | Read | Tests |
| --- | --- | --- | --- |
| `fn main() -> i64`, calls, recursion, returns | implemented | [Functions](functions.md) | `tests/cases/functions/`, `make check-core-language` |
| `let`, `var`, assignment, local inference | implemented | [Variables](variables.md) | `tests/cases/variables/`, `make check-core-language` |
| integers, bool, `char`, strings, floats | partial | [Literals](literals.md), [Types](types.md) | `tests/cases/literals/`, `tests/cases/operators/` |
| arithmetic, bit, comparison, logical operators | implemented | [Operators](operators.md) | `tests/cases/operators/`, `make check-core-language` |
| explicit casts with `as` | implemented | [Operators](operators.md), [Types](types.md) | `tests/cases/operators/`, `make check-core-language` |
| blocks, `if`, `while`, `break`, `continue` | implemented | [Control Flow](control-flow.md) | `tests/cases/control-flow/`, `make check-core-language` |
| ranges and `for` loops | implemented | [Control Flow](control-flow.md), [Cookbook](cookbook.md) | `tests/cases/control-flow/`, `make check-core-language` |
| `Option`, `Result`, `?`, `??` | implemented | [Operators](operators.md), [Cookbook](cookbook.md) | `tests/cases/operators/`, `tests/cases/generics/` |

## Data Modeling

| Feature | Status | Read | Tests |
| --- | --- | --- | --- |
| structs, tuple structs, field access | implemented for local/runtime-supported shapes | [Types](types.md), [Front-End Only Syntax](front-end-only.md) | `tests/cases/structs/`, `make check-structs` |
| field `mut` assignment checks | implemented | [Types](types.md), [Variables](variables.md) | `tests/cases/structs/`, `tests/cases/variables/` |
| non-generic enums and `match` | implemented | [Enums And Pattern Matching](enums-patterns.md) | `tests/cases/match/` |
| generic structs, enums, and aliases | implemented for local/codegen-supported aggregate monomorphization | [Generic Aggregates](generic-aggregates.md), [Types](types.md), [Generics sections in Quick Reference](quick-reference.md) | `tests/cases/generics/`, `make check-generics` |
| tuple, fixed array, local `Vec[T]`, `Slice[T]` | partial | [Types](types.md), [Variables](variables.md) | `tests/cases/variables/`, `tests/cases/vectors/` |
| aggregate destructuring | partial | [Variables](variables.md), [Front-End Only Syntax](front-end-only.md) | `tests/cases/patterns/` |

## Names And Modules

| Feature | Status | Read | Tests |
| --- | --- | --- | --- |
| inline `mod`, `pub`, `use`, aliases | implemented | [Modules](modules.md) | `tests/cases/modules/` |
| file-backed modules with `mod name;` | implemented, still hardening project ergonomics | [Modules](modules.md), [Getting Started](getting-started.md) | `tests/cases/modules/`, `tests/packages/` |
| module metadata and cache | partial | [Compiler Module Project Authoring](../dev/compiler-module-project-authoring.md) | `tests/cases/modules/`, `tests/packages/` |
| package manager | planned later, outside compiler start readiness | [Roadmap](../dev/roadmap.md) | not yet |

## Traits And Methods

| Feature | Status | Read | Tests |
| --- | --- | --- | --- |
| trait declarations and concrete impls | implemented | [Traits](traits.md) | `tests/cases/traits/` |
| minimum static trait subset | complete for current executable subset | [Traits](traits.md), [Minimum Trait Readiness](../dev/trait-minimum-readiness.md) | `make check-traits` |
| constrained static dispatch | implemented for supported bounds | [Traits](traits.md), [Minimum Trait Readiness](../dev/trait-minimum-readiness.md) | `tests/cases/traits/`, `tests/cases/generics/` |
| generic impls and generic trait methods | implemented for static dispatch; advanced solving still scoped separately | [Traits](traits.md), [Front-End Only Syntax](front-end-only.md) | `tests/cases/traits/`, `tests/cases/generics/` |
| `dyn Trait` | partial/front-end only depending on shape | [Traits](traits.md), [Front-End Only Syntax](front-end-only.md) | `tests/cases/traits/` |
| associated type projections and broader trait resolution | implemented seed; broader solving partial | [Traits](traits.md) | `tests/cases/traits/` |

## Memory And Runtime Boundaries

| Feature | Status | Read | Tests |
| --- | --- | --- | --- |
| `own`, move, borrow, drop checks | implemented for supported values | [Memory And Ownership](memory.md) | `tests/cases/ownership/`, `tests/cases/borrowing/` |
| explicit `Zone` allocation | implemented, still growing ergonomics | [Memory And Ownership](memory.md), [Cookbook](cookbook.md) | `tests/cases/memory/` |
| raw pointers and layout helpers | partial | [Memory And Ownership](memory.md), [C FFI And Libraries](ffi.md) | `tests/cases/memory/`, `tests/cases/ffi/` |
| source `std` handles such as `String`, `Vec`, `Box`, collections | partial | [Standard Library](standard-library.md), [std docs](../stdlib/README.md) | `tests/cases/standard-library/` |

## Tooling And Compiler Development

| Feature | Status | Read | Tests |
| --- | --- | --- | --- |
| token, syntax, diagnostic, source-map artifact dumps | implemented seed | [Compiler Artifact Authoring](../dev/compiler-artifact-authoring.md) | `make check-compiler-artifacts` |
| current compiler developer workflow | implemented docs | [Developer Overview](../dev/README.md), [Roadmap](../dev/roadmap.md) | `make check-compiler-docs` |
| future Ari-written compiler implementation | not started and intentionally out of scope | [Compiler Readiness Inventory](../dev/compiler-readiness-inventory.md) | no implementation tree |

## How To Use This Page

When writing Ari code:

1. Start from [Getting Started](getting-started.md) and
   [Quick Reference](quick-reference.md).
2. Use this page to check whether a feature is executable or partial.
3. Open [Feature Crosswalk](feature-crosswalk.md) to find the exact example,
   test family, and small check command.
4. Open the focused feature page for rules and examples.
5. If the feature is partial or front-end only, inspect the linked test family
   before relying on it in executable code.

When developing the compiler:

1. Find the feature family in this page.
2. Read the linked language doc, [Feature Crosswalk](feature-crosswalk.md), and
   matching dev doc.
3. Add one focused `ok`, `errors`, or `artifact` fixture that proves the
   changed behavior.
4. Run the narrow target named by the docs before broad checks.
