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
| `fn main() -> i64`, calls, recursion, returns | implemented | [Functions](functions.md) | `tests/cases/functions/` |
| `let`, `var`, assignment, local inference | implemented | [Variables](variables.md) | `tests/cases/variables/` |
| integers, bool, `char`, strings, floats | partial | [Literals](literals.md), [Types](types.md) | `tests/cases/literals/`, `tests/cases/operators/` |
| arithmetic, bit, comparison, logical operators | implemented | [Operators](operators.md) | `tests/cases/operators/` |
| explicit casts with `as` | implemented | [Operators](operators.md), [Types](types.md) | `tests/cases/operators/` |
| blocks, `if`, `while`, `break`, `continue` | implemented | [Control Flow](control-flow.md) | `tests/cases/control-flow/` |
| ranges and `for` loops | implemented | [Control Flow](control-flow.md), [Cookbook](cookbook.md) | `tests/cases/control-flow/` |
| `Option`, `Result`, `?`, `??` | implemented | [Operators](operators.md), [Cookbook](cookbook.md) | `tests/cases/operators/`, `tests/cases/generics/` |

## Data Modeling

| Feature | Status | Read | Tests |
| --- | --- | --- | --- |
| structs, tuple structs, field access | implemented for local/runtime-supported shapes | [Types](types.md), [Front-End Only Syntax](front-end-only.md) | `tests/cases/structs/` |
| field `mut` assignment checks | implemented | [Types](types.md), [Variables](variables.md) | `tests/cases/structs/`, `tests/cases/variables/` |
| non-generic enums and `match` | implemented | [Enums And Pattern Matching](enums-patterns.md) | `tests/cases/match/` |
| generic structs and enums | partial | [Types](types.md), [Generics sections in Quick Reference](quick-reference.md) | `tests/cases/generics/` |
| tuple, fixed array, local `Vec[T]`, `Slice[T]` | partial | [Types](types.md), [Variables](variables.md) | `tests/cases/variables/`, `tests/cases/vectors/` |
| aggregate destructuring | partial | [Variables](variables.md), [Front-End Only Syntax](front-end-only.md) | `tests/cases/patterns/` |

## Names And Modules

| Feature | Status | Read | Tests |
| --- | --- | --- | --- |
| inline `mod`, `pub`, `use`, aliases | implemented | [Modules](modules.md) | `tests/cases/modules/` |
| file-backed modules with `mod name;` | implemented, still hardening project ergonomics | [Modules](modules.md), [Getting Started](getting-started.md) | `tests/cases/modules/`, `tests/packages/` |
| module metadata and cache | partial | [Compiler Project Model](../dev/compiler-project-model.md) | `tests/cases/modules/`, `tests/packages/` |
| package manager | planned | [Compiler Next Slices](../dev/compiler-next-slices.md) | not yet |

## Traits And Methods

| Feature | Status | Read | Tests |
| --- | --- | --- | --- |
| trait declarations and concrete impls | implemented | [Traits](traits.md) | `tests/cases/traits/` |
| constrained static dispatch | implemented for supported bounds | [Traits](traits.md) | `tests/cases/traits/`, `tests/cases/generics/` |
| generic impls and generic trait methods | partial | [Traits](traits.md), [Front-End Only Syntax](front-end-only.md) | `tests/cases/traits/`, `tests/cases/generics/` |
| `dyn Trait` | partial/front-end only depending on shape | [Traits](traits.md), [Front-End Only Syntax](front-end-only.md) | `tests/cases/traits/` |
| associated types and broader trait resolution | partial/planned | [Traits](traits.md) | `tests/cases/traits/` |

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
| token, syntax, diagnostic, source-map artifact dumps | implemented seed | [Compiler Artifact Testing](../dev/compiler-artifact-testing.md) | `make check-compiler-artifacts` |
| compiler-shaped model fixtures | implemented seed | [Compiler Readiness Inventory](../dev/compiler-readiness-inventory.md) | `tests/cases/compiler-development/ok/model/`, `make check-compiler-development` |
| implementation playbook and next slices | implemented docs | [Compiler Implementation Playbook](../dev/compiler-implementation-playbook.md), [Compiler Next Slices](../dev/compiler-next-slices.md) | `make check-compiler-dev-docs` |
| bootstrap implementation | not started | [Bootstrap Readiness](../dev/bootstrap-readiness.md) | readiness fixtures only |

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
3. Add one `ok`, `errors`, `artifact`, or `compiler-development/ok/model`
   fixture that proves the changed behavior.
4. Run the narrow target named by the docs before broad checks.
