# Core Language Readiness

This page records the readiness contract for Ari's current executable core
language. It is about stability of existing behavior, not new syntax. Keep this
surface boring and explicit: functions, locals, scalar operators, casts, blocks,
branches, loops, `break`, `continue`, and `return`.

Run the focused gate with:

```sh
make check-core-language
```

Do not use this page to justify new parser, sema, or backend design. New syntax
belongs in its feature page first.

## Coverage Audit

| Area | Coverage | Classification |
| --- | --- | --- |
| Simple function calls | `tests/cases/functions/ok/function-recursion.ari`, `tests/cases/core-language/ok/core-language-readiness.ari` | already supported, now locked by core runtime smoke |
| Multiple parameters | function arity fixtures and `combine(first, second, third)` in the core smoke | already supported |
| Recursion | `function-recursion.ari`, `fact(4)` in the core smoke | already supported |
| Return checking | function error fixtures for missing and mismatched returns | already supported |
| `let`, `var`, assignment | variable fixtures plus `locals_and_assignment()` in the core smoke | already supported |
| Basic local type inference | `variables-basic.ari` plus inferred integer and bool locals in the core smoke | already supported |
| Arithmetic and modulo | operator fixtures plus `arithmetic()` in the core smoke | already supported |
| Comparisons | operator/control-flow fixtures plus `comparisons_and_boolean()` | already supported |
| Boolean operators | `logical.ari` and the core smoke short boolean chain | already supported |
| Bitwise operators and shifts | `bitwise.ari`, `operators-bitwise-not.ari`, and the core smoke | already supported |
| Unary `-` and `!` | numeric width/operator fixtures plus `unary_and_casts()` and `comparisons_and_boolean()` | already supported |
| Explicit casts | operator cast fixtures plus `byte as i64` in the core smoke | already supported |
| Blocks and block expressions | `block-expression.ari` plus `blocks_and_nested_if()` | already supported |
| `if` / `else` and nested `if` | control-flow fixtures plus the core smoke | already supported |
| `while` | `while-basic.ari` plus `while_loop()` | already supported |
| `for` / range | range fixtures plus `for value in 0..6` in the core smoke | already supported |
| `break` / `continue` | control-flow fixtures plus `while_loop()` and `for_range_loop()` | already supported |
| Return from nested branch/block | `nested_branch_return()` in the core smoke | now explicitly covered |
| Wrong function arity/type | existing function errors, rechecked by `make check-core-language` | stable negative |
| Missing/wrong return | existing function errors, rechecked by `make check-core-language` | stable negative |
| Immutable/type-mismatched assignment | existing variable errors, rechecked by `make check-core-language` | stable negative |
| Use before declaration | `tests/cases/core-language/errors/core-use-before-declaration.ari` | newly covered negative |
| Invalid binary operator types | `tests/cases/core-language/errors/core-invalid-binary-operator.ari` | newly covered negative |
| Invalid unary operator type | `tests/cases/core-language/errors/core-invalid-unary-minus.ari` plus existing `not-non-bool.ari` | newly covered negative |
| Invalid casts | existing operator error, rechecked by `make check-core-language` | stable negative |
| Break/continue outside loop | existing control-flow errors, rechecked by `make check-core-language` | stable negative |
| Branch expression type mismatch | existing control-flow error, rechecked by `make check-core-language` | stable negative |
| Typed IR artifact | `tests/cases/compiler-development/artifact/ok/core-language-scalar-flow.ir` | stable artifact |
| Diagnostic artifact | `tests/cases/compiler-development/artifact/errors/diagnostic-core-use-before-declaration.diagnostic` | stable artifact |

## Runtime Contract

`tests/cases/core-language/ok/core-language-readiness.ari` exits with `115`.
That single program intentionally touches every core surface in the scope:

- direct calls, multi-argument calls, recursion, and nested branch returns
- immutable and mutable locals, assignment, and inferred scalar locals
- arithmetic, modulo, comparison, boolean, bitwise, shift, unary, and cast
  expressions
- block expressions, `if` / `else if` / `else`, `while`, `for` over a range,
  `break`, `continue`, and explicit `return`

The LLVM check in `make check-core-language` also looks for core backend shapes:
function symbols, recursive calls, `while.cond`, `for.cond`, `srem`, `shl`, and
integer comparisons.

## Intentional Limits

- Ari has no implicit numeric casts. Use `as`.
- Logical operators require `bool`.
- Bitwise and shift operators require integer values.
- Local names must be declared before use, and ordinary local shadowing remains
  rejected by the variables suite.
- Non-void functions must return on every statically visible path.
- Branch-valued expressions must agree on one result type unless a no-return arm
  makes the type unambiguous.
- `break` and `continue` are loop control statements. Value-carrying `break`
  belongs to labeled block/loop tests, not the minimal core smoke.
- `range(start, end)` is a prelude helper. The core smoke uses `0..end` so the
  syntax remains visibly a range loop; no new range semantics are introduced
  here.

## Adding Tests

When changing one of these core behaviors:

1. Add the smallest focused fixture in the owning feature directory when
   possible: `functions`, `variables`, `operators`, or `control-flow`.
2. Extend `core-language-readiness.ari` only when the cross-feature contract
   changes.
3. Add a stable negative diagnostic under `tests/cases/core-language/errors/`
   only when no narrower feature directory already owns that diagnostic.
4. Update the typed IR or diagnostic artifact only when the user-visible core
   shape changes.
5. Run `make check-core-language`; then run the narrow owning target such as
   `make check-functions` or `make check-operators`.
