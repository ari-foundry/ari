# Standard Library Testing

Standard library tests live in `tests/cases/standard-library/` and are wired
through `tests/Makefile`.

## Test Layers

| Layer | Purpose | Command |
| --- | --- | --- |
| API manifest | Detect accidental public API drift. | `make check-std-api` |
| Docs readiness | Keep production readiness, module tier, failure, non-goal, and per-module guide policy documented. | `make check-stdlib-docs` |
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
prelude-slice-metadata.ari
prelude-slice-option-access.ari
std-string-append-u64.ari
std-string-ascii-helpers.ari
std-string-ascii-case-helpers.ari
std-string-unicode-helpers.ari
std-string-text-kinds.ari
std-string-byte-literals.ari
std-string-prefix-parsers.ari
std-string-trim-copy.ari
std-string-trim-to-after-target-reset.ari
std-string-try-byte-access.ari
std-fmt-format-spec.ari
std-context-args.ari
std-test-report.ari
std-error-basic.ari
std-c-interop.ari
std-c-dynamic-function.ari
std-target-basic.ari
std-target-linux64.ari
std-env-paths.ari
std-env-os-path-views.ari
std-env-vars.ari
std-process-basic.ari
std-process-identity.ari
std-process-exit.ari
std-process-abort.ari
std-process-fork-wait.ari
std-thread-basic.ari
std-thread-runtime-helpers.ari
std-sync-atomic-i64.ari
std-sync-mutex-once.ari
std-sync-rwlock.ari
std-time-basic.ari
std-time-timeout.ari
std-time-utc-calendar.ari
std-fs-basic.ari
std-fs-append.ari
std-fs-open-modes.ari
std-fs-read-write.ari
std-fs-try-read.ari
std-fs-create-truncate-copy.ari
std-fs-rename-dir.ari
std-fs-read-dir.ari
std-fs-links.ari
std-fs-permissions.ari
std-fs-mode.ari
std-algo-slice-helpers.ari
std-hash-basic.ari
std-random-basic.ari
std-parse-basic.ari
std-encoding-text.ari
std-encoding-utf8-codepoints.ari
std-encoding-codec.ari
std-path-basic.ari
std-path-components.ari
std-path-bytes.ari
std-collections-set.ari
std-collections-set-access.ari
std-collections-set-replace.ari
std-collections-set-iter.ari
std-collections-hash.ari
std-collections-hash-iter.ari
std-collections-tree.ari
std-collections-tree-iter.ari
std-collections-deque.ari
std-collections-ring-buffer.ari
std-collections-linked-list.ari
std-collections-heap.ari
std-collections-deque-iter-after-reset.ari
std-collections-ring-buffer-after-reset.ari
std-collections-binary-heap-push-different-zone.ari
std-iter-adapters.ari
std-collections-set-after-reset.ari
std-collections-set-iter-after-reset.ari
std-collections-set-insert-different-zone.ari
std-collections-set-replace-different-zone.ari
std-collections-set-reserve-different-zone.ari
std-collections-set-reserve-extra-different-zone.ari
std-io-byte-slice.ari
std-io-traits-cursor.ari
std-io-stderr.ari
std-io-buffered.ari
std-log-basic.ari
std-mem-byte-ops.ari
std-mem-page-size.ari
std-input-byte-option.ari
std-cmp-value-helpers.ari
std-convert-value-helpers.ari
std-math-integer-helpers.ari
std-math-division-rounding.ari
std-math-checked-saturating.ari
std-math-wrapping-overflowing.ari
std-ascii-case-compare.ari
std-ascii-case-search.ari
std-ascii-class-helpers.ari
std-ascii-prefix-parsers.ari
std-ascii-slice-helpers.ari
std-bits-byte-population.ari
std-bits-rotate-helpers.ari
std-bits-scan-helpers.ari
std-bits-one-run-helpers.ari
std-bits-width-helpers.ari
std-zone-alloc-array.ari
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
