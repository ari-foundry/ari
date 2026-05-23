# Examples And Tests

This page connects user-facing Ari examples with the test folders that protect
the same feature. Use it after [Getting Started](getting-started.md),
[Language Tour](language-tour.md), [Quick Reference](quick-reference.md), and
[Feature Status](feature-status.md).

The examples are meant for reading and running. The tests are meant for
checking exact compiler behavior, diagnostics, and backend artifacts.

## Example Programs

| Example | What It Shows | Run |
| --- | --- | --- |
| `examples/hello.ari` | Minimal executable and `println`. | `make run-example EXAMPLE=hello` |
| `examples/count.ari` | Exit-code oriented `main() -> i64`. | `make run-example EXAMPLE=count` |
| `examples/fibonacci.ari` | Functions, recursion or loop-style numeric work. | `make run-example EXAMPLE=fibonacci` |
| `examples/modules.ari` | Inline modules, visibility, and qualified names. | `make run-example EXAMPLE=modules` |
| `examples/enums-match.ari` | Enum cases and statement `match`. | `make run-example EXAMPLE=enums-match` |
| `examples/ownership.ari` | `own`, move/drop checks, and visible ownership flow. | `make run-example EXAMPLE=ownership` |
| `examples/language-tour.ari` | A compact cross-feature tour. | `make run-example EXAMPLE=language-tour` |

Build all examples with:

```sh
make examples
```

Check all examples without linking with:

```sh
make check-examples
```

## Test Families

| Need | Start Here | Narrow Target |
| --- | --- | --- |
| Function rules, `main`, returns, calls | `tests/cases/functions/` | `make check-functions` |
| Local variables, assignment, patterns | `tests/cases/variables/` | `make check-variables` |
| Operators, literals, casts, `?`, `??` | `tests/cases/operators/`, `tests/cases/literals/` | `make check-operators` |
| Control flow, ranges, loops | `tests/cases/control-flow/` | `make check-control-flow` |
| Enums and pattern matching | `tests/cases/match/`, `tests/cases/patterns/` | `make check-match` |
| Modules, imports, metadata, cache | `tests/cases/modules/`, `tests/packages/` | `make check-modules` |
| Generics and generic aggregates | `tests/cases/generics/` | `make check-generics` |
| Traits and static dispatch | `tests/cases/traits/` | `make check-traits` |
| Ownership and borrowing | `tests/cases/ownership/`, `tests/cases/borrowing/` | focused `build/ari ... --check` |
| Memory, zones, raw pointers | `tests/cases/memory/` | focused `build/ari ... --check` |
| C FFI and shared-library surfaces | `tests/cases/ffi/`, `tests/ffi/` | `make check-ffi` |
| Source standard library behavior | `tests/cases/standard-library/` | `make check-prelude`, `make check-std-api` |
| Compiler artifacts | `tests/cases/compiler-development/artifact/` | `make check-compiler-artifacts` |

## Reading A Test File

Test files should be read as executable documentation:

- `ok/` files show behavior that should compile, emit an artifact, or run.
- `errors/` files show behavior that should fail with a stable diagnostic.
- `artifact/ok/` files are committed compiler outputs that should compare
  cleanly.
- `artifact/errors/` files are expected diagnostic or mismatch-report outputs.

When a test looks more complete than the prose docs, treat that as a signal to
update the docs after changing behavior. The test protects the compiler; the
docs explain the language.

## Small Check Workflow

Use the smallest check that proves the behavior you are editing:

```sh
./build/ari tests/cases/<feature>/ok/<case>.ari --check
./build/ari tests/cases/<feature>/ok/<case>.ari --emit-llvm build/focused/<case>.ll
make check-language-docs
```

Run broader targets only when a change crosses a compiler boundary.
Full `make check` is for handoff, not every edit loop.
