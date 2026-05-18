# Standard Library Testing

Standard library tests live in `tests/cases/standard-library/` and are wired
through `tests/Makefile`.

## Test Layers

| Layer | Purpose | Command |
| --- | --- | --- |
| API manifest | Detect accidental public API drift. | `make check-std-api` |
| Focused source behavior | Compile and usually run one valid behavior. | `make check-prelude` |
| Negative diagnostics | Lock down bad uses and planned gaps. | `make check-prelude` |
| Backend checks | Inspect LLVM symbols, runtime hooks, and layout. | `make check-prelude` or `make check` |
| Cross-library smoke | Exercise multiple modules together once. | `std-library-smoke.ari` |

## Naming

Use these prefixes consistently:

- `std-<module>-<feature>.ari` for source module APIs.
- `prelude-<feature>.ari` for implicit root aliases and macros.
- `format-<feature>.ari` for formatting behavior.
- `ari-builtin-<feature>.ari` for `extern "ari"` validation.

Examples:

```text
std-vec-try-pop.ari
std-vec-try-access.ari
prelude-slice-option-access.ari
std-string-append-u64.ari
std-string-ascii-helpers.ari
std-math-integer-helpers.ari
std-ascii-class-helpers.ari
std-ascii-slice-helpers.ari
std-bits-rotate-helpers.ari
std-bits-scan-helpers.ari
std-bits-width-helpers.ari
prelude-option-result-methods.ari
prelude-option-result-predicates.ari
```

## What Counts As Covered

For a public API, aim for:

- one simple positive call
- one executable result check when behavior is runtime-visible
- one LLVM symbol or runtime-hook check when lowering matters
- one module-path call through `std::<module>::name`
- one prelude/root alias check when the API is re-exported
- negative diagnostics for wrong types, ownership misuse, or zone misuse

Runtime assertions do not need negative compile tests unless the checker can
reject the misuse statically.
