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
prelude-slice-metadata.ari
prelude-slice-option-access.ari
std-string-append-u64.ari
std-string-ascii-helpers.ari
std-string-ascii-case-helpers.ari
std-string-prefix-parsers.ari
std-string-trim-copy.ari
std-string-trim-to-after-target-reset.ari
std-string-try-byte-access.ari
std-context-args.ari
std-target-basic.ari
std-target-linux64.ari
std-env-paths.ari
std-env-vars.ari
std-process-basic.ari
std-process-exit.ari
std-process-fork-wait.ari
std-thread-basic.ari
std-sync-atomic-i64.ari
std-time-basic.ari
std-fs-basic.ari
std-fs-append.ari
std-fs-open-modes.ari
std-fs-read-write.ari
std-fs-create-truncate-copy.ari
std-fs-rename-dir.ari
std-fs-links.ari
std-fs-permissions.ari
std-algo-slice-helpers.ari
std-hash-basic.ari
std-parse-basic.ari
std-encoding-text.ari
std-encoding-codec.ari
std-path-basic.ari
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
std-mem-byte-ops.ari
std-input-byte-option.ari
std-cmp-value-helpers.ari
std-convert-value-helpers.ari
std-math-integer-helpers.ari
std-math-division-rounding.ari
std-ascii-case-compare.ari
std-ascii-case-search.ari
std-ascii-class-helpers.ari
std-ascii-prefix-parsers.ari
std-ascii-slice-helpers.ari
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
