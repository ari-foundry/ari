# Feature Crosswalk

This page connects Ari features to the docs, examples, test families, and small
checks that prove them. Use it after [Feature Status](feature-status.md) when
you know the feature name but need the next practical place to read or test.

The goal is docs-only usability: a first-time Ari user should be able to find a
working pattern, and a first-time compiler contributor should be able to find
the test bucket that protects the behavior.

## How To Read A Row

Each row names four things:

- **Read**: the smallest language page that explains the current spelling.
- **Example**: a runnable example or nearby test to copy from.
- **Tests**: the focused family that protects the behavior.
- **Small check**: the narrow command to run while iterating.

If a row is marked partial, read the linked tests before relying on the feature
in executable code. Parsed syntax is not always lowered all the way to LLVM.

## Executable Program Surface

| Feature | Read | Example | Tests | Small check |
| --- | --- | --- | --- | --- |
| Core language readiness | [Feature Status](feature-status.md), [Functions](functions.md), [Variables](variables.md), [Operators](operators.md), [Control Flow](control-flow.md) | `tests/cases/core-language/ok/core-language-readiness.ari` | `tests/cases/core-language/` | `make check-core-language` |
| Program entry and return code | [Getting Started](getting-started.md), [Functions](functions.md) | `examples/hello.ari`, `examples/count.ari` | `tests/cases/functions/ok/` | `build/ari path/to/file.ari --check` |
| Local values and mutation | [Variables](variables.md) | [Language Tour](language-tour.md) | `tests/cases/variables/` | `make check-variables` |
| Integer, bool, char, and string literals | [Literals](literals.md), [Types](types.md) | [Quick Reference](quick-reference.md) | `tests/cases/literals/`, `tests/cases/operators/` | `build/ari path/to/file.ari --check` |
| Operators and explicit casts | [Operators](operators.md) | [Cookbook](cookbook.md) | `tests/cases/operators/` | `make check-operators` |
| `if`, `while`, `for`, `break`, `continue` | [Control Flow](control-flow.md) | `examples/language-tour.ari` | `tests/cases/control-flow/` | `make check-control-flow` |
| Functions and generic calls | [Functions](functions.md), [Types](types.md) | `fn identity[T](value: T) -> T` in [Language Tour](language-tour.md) | `tests/cases/functions/`, `tests/cases/generics/` | `make check-functions` or `make check-generics` |

## Data And Names

| Feature | Read | Example | Tests | Small check |
| --- | --- | --- | --- | --- |
| Structs and field access | [Types](types.md) | `struct Point` in [Language Tour](language-tour.md) | `tests/cases/structs/` | `make check-structs` |
| Enums and statement `match` | [Enums And Pattern Matching](enums-patterns.md) | `enum ScoreResult` in [Language Tour](language-tour.md) | `tests/cases/match/` | `make check-match` |
| Generic structs and enums | [Types](types.md), [Feature Status](feature-status.md) | generic aggregate fixtures | `tests/cases/generics/` | `make check-generics` |
| Inline modules | [Modules](modules.md) | `mod Math { ... }` in [Language Tour](language-tour.md) | `tests/cases/modules/` | `make check-modules` |
| File-backed modules | [Modules](modules.md), [Getting Started](getting-started.md) | `mod math;` in [Getting Started](getting-started.md) | `tests/cases/modules/`, `tests/packages/` | `make check-modules` |
| Visibility and qualified names | [Modules](modules.md) | `Math::double(base)` in [Language Tour](language-tour.md) | `tests/cases/modules/` | `make check-modules` |

## Traits, Memory, And Boundaries

| Feature | Read | Example | Tests | Small check |
| --- | --- | --- | --- | --- |
| Trait declarations and concrete impls | [Traits](traits.md) | `trait Score` in [Language Tour](language-tour.md) | `tests/cases/traits/` | `make check-traits` |
| Minimum static trait subset | [Traits](traits.md), [Minimum Trait Readiness](../dev/trait-minimum-readiness.md) | compiler-shaped Eq/Hash/Debug/Ord fixtures | `tests/cases/traits/ok/trait-minimum-*.ari` | `make check-traits` |
| Static trait dispatch | [Traits](traits.md), [Feature Status](feature-status.md) | trait fixtures | `tests/cases/traits/`, `tests/cases/generics/` | `make check-traits` |
| Ownership, move, borrow, and `drop` | [Memory And Ownership](memory.md) | [Cookbook](cookbook.md) | `tests/cases/ownership/`, `tests/cases/borrowing/` | `make check-ownership` |
| Explicit zones and local vectors | [Memory And Ownership](memory.md), [Cookbook](cookbook.md) | `std::vec::new<i64>` in [Language Tour](language-tour.md) | `tests/cases/memory/`, `tests/cases/vectors/` | `build/ari path/to/file.ari --check` |
| C FFI and shared libraries | [C FFI And Libraries](ffi.md) | FFI tests | `tests/cases/ffi/` | `make check-ffi` |
| Prelude formatting | [Prelude And Formatting](prelude.md) | `println("value={}", value)` | `tests/cases/standard-library/ok/format/` | `make check-prelude` |

## Compiler Development Surface

| Feature | Read | Example | Tests | Small check |
| --- | --- | --- | --- | --- |
| Source identity and spans | [Compiler Source Identity](../dev/compiler-source-identity.md) | `source-map-utf8.map` | `tests/cases/compiler-development/artifact/ok/` | `make check-compiler-artifacts` |
| Diagnostic codes and labels | [Compiler Diagnostic Authoring](../dev/compiler-diagnostic-authoring.md) | `diagnostic-parser-expected.diagnostic` | `tests/cases/compiler-development/artifact/errors/` | `make check-compiler-artifacts` |
| Artifact producers and goldens | [Compiler Artifact Authoring](../dev/compiler-artifact-authoring.md) | `token-dump-basic.tokens` | `tests/cases/compiler-development/artifact/ok/` | `make check-compiler-artifacts` |
| Module/project artifacts | [Compiler Module Project Authoring](../dev/compiler-module-project-authoring.md) | `module-graph-file-module.graph` | `tests/cases/compiler-development/artifact/ok/` | `make check-compiler-artifacts` |
| Compiler readiness inventory | [Compiler Readiness Inventory](../dev/compiler-readiness-inventory.md) | readiness gates | `tests/cases/bootstrap-readiness/` | `make check-compiler-docs` |

## When Docs And Behavior Differ

Treat a mismatch as a compiler-development bug report, not as permission to add
a private shortcut:

1. Update the closest `ok`, `errors`, or `artifact` fixture.
2. Update [Feature Status](feature-status.md) if the implemented state changed.
3. Update the focused language page if the rule is user-visible.
4. Update the dev roadmap or readiness inventory if the change moves a
   compiler start gate.
5. Run the smallest check named by the row before broad handoff checks.

Do not add bootstrap-only syntax, hidden global allocation, or special backend
behavior that ordinary Ari programs cannot use.
