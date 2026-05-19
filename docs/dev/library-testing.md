# Library Testing

This page explains how standard library tests are organized. It is meant for
contributors adding APIs under `lib/std.arih` or `lib/std/`.

## Test Layers

| Layer | Purpose | Where |
| --- | --- | --- |
| API manifest | Prevent public `std` APIs from changing accidentally. | `tests/std_api_manifest.txt`, checked by `make check-std-api`. |
| Positive source behavior | Compile and run valid library use. | `tests/cases/standard-library/ok/<feature>/std-<module>-<case>.ari`, `tests/cases/standard-library/ok/prelude/prelude-<case>.ari`. |
| Negative diagnostics | Lock down misuse and unsupported surfaces. | `tests/cases/standard-library/errors/<feature>/std-<module>-<case>.ari`, `tests/cases/standard-library/errors/prelude/prelude-<case>.ari`. |
| Backend checks | Inspect LLVM, symbols, runtime hooks, and executable output. | `tests/Makefile` under the relevant check target. |
| Cross-library smoke | Exercise several library families together. | `tests/cases/standard-library/ok/smoke/std-library-smoke.ari`. |
| Docs coverage | Explain user-visible behavior and remaining limits. | `docs/language/standard-library.md`, focused language docs, `docs/dev/test-matrix.md`. |

## Naming Scheme

Use file names that identify both the library family and the behavior under
test. The containing folder identifies the feature family, and the basename
identifies the exact case:

- `std-vec-try-pop.ari`: source `std::vec` method behavior.
- `std-vec-try-access.ari`: source `std::vec` Option-returning access
  behavior.
- `prelude-slice-metadata.ari`: root `Slice[T]` borrowed metadata helper
  behavior.
- `prelude-slice-option-access.ari`: root `Slice[T]` Option-returning access
  behavior.
- `std-string-append-u64.ari`: source `std::string` formatting helper.
- `std-string-ascii-helpers.ari`: source `std::string` borrowed ASCII trim
  views and whole-string parsing behavior.
- `std-string-ascii-case-helpers.ari`: source `std::string` ASCII-only
  case-insensitive comparison and first-match search helpers.
- `std-string-prefix-parsers.ari`: source `std::string` prefix parser
  conveniences over the `std::ascii` parser result shape.
- `std-string-trim-copy.ari`: source `std::string` owned ASCII trim copies
  into a target zone, including source-zone reset behavior.
- `std-string-trim-to-after-target-reset.ari`: negative target-zone
  provenance diagnostic for copied trim results.
- `std-string-try-byte-access.ari`: source `std::string` Option-returning
  byte access and empty pop behavior.
- `std-context-args.ari`: runtime-backed `std::context` argument and thread-id
  access, source context predicates, and root alias behavior.
- `std-env-args.ari`: source `std::env` argument helpers, `Option`-returning
  `try_arg`, and `program_name` behavior over the runtime context.
- `std-env-vars.ari`: runtime-backed `std::env` environment variable
  `get`/`has`/`try_get`/`set`/`remove` behavior.
- `std-env-paths.ari`: runtime-backed `std::env` current directory,
  `set_current_dir`, executable path, and `Option` wrapper behavior.
- `std-process-basic.ari`: runtime-backed `std::process::id` plus source
  success/failure status helper behavior.
- `std-process-exit.ari`: runtime-backed explicit process exit status
  behavior.
- `std-process-fork-wait.ari`: runtime-backed POSIX `fork`/`wait` behavior,
  child/parent predicates, explicit child exit, and wait failure sentinel.
- `std-thread-basic.ari`: runtime-backed `std::thread` function-pointer
  spawn/join behavior, child runtime ids, invalid-handle sentinels, root
  `Thread`, method wrappers, and scheduler yield.
- `std-sync-atomic-i64.ari`: runtime-backed `std::sync::AtomicI64`
  load/store/swap/fetch-add/compare-exchange behavior, method wrappers, root
  alias, and LLVM atomic lowering.
- `std-time-basic.ari`: runtime-backed monotonic and wall-clock nanosecond
  reads, source duration/instant wrappers, elapsed-time helpers, and sleep.
- `std-fs-basic.ari`: runtime-backed file existence, mode-string
  creation/truncating write, byte reads/writes, close, removal, and
  `Option[File]` open helpers.
- `std-fs-append.ari`: runtime-backed `"a"` append mode, preservation of
  existing bytes, appended byte-slice writes, and failed append opens as
  `Option`.
- `std-fs-open-modes.ari`: mode-string `open`/`try_open` behavior for `"r"`,
  `"w"`, `"a"`, `"rw"`, `"r+"`, `"w+"`, `"a+"`, empty modes, and invalid
  mode strings.
- `std-fs-read-write.ari`: source whole-file `write`, `append`,
  `read_to_string`, missing-file empty reads, and truncating rewrite behavior.
- `std-fs-create-truncate-copy.ari`: source `create`, `try_create`, natural
  `read`, `truncate`, streaming `copy`, and missing-source copy failure.
- `std-fs-rename-dir.ari`: runtime-backed `rename`, single-directory
  `create_dir`/`remove_dir`, duplicate-create failure, and missing-source
  rename failure.
- `std-net-addresses.ari`: source `std::net` IPv4/IPv6 constructors, generic
  IP predicates, socket-address construction, port replacement, loopback, and
  unspecified checks.
- `std-collections-set.ari`: source `std::collections::Set[T]` constructor,
  insertion, duplicate rejection, membership, removal, borrowed view, copy, and
  target/source zone behavior.
- `std-collections-set-access.ari`: source set insertion-order accessors,
  optional accessors, explicit reserve growth, `pop`, and `try_pop` behavior.
- `std-collections-set-replace.ari`: source set replace-or-insert behavior,
  returned previous values, missing-value insertion, and growth behavior.
- `std-collections-set-iter.ari`: source set cursor iteration, direct
  `for value in set.iter()`, and `IntoIterator` lowering for `for value in set`.
- `std-collections-set-after-reset.ari`: negative source-zone provenance
  diagnostic for a tracked set after `zone::reset`.
- `std-collections-set-iter-after-reset.ari`: negative source-zone provenance
  diagnostic for a set iterator after its source zone is reset.
- `std-collections-set-insert-different-zone.ari`: negative same-zone growth
  diagnostic for set insertion with the wrong allocation zone.
- `std-collections-set-replace-different-zone.ari`: negative same-zone growth
  diagnostic for replace-or-insert with the wrong allocation zone.
- `std-collections-set-reserve-different-zone.ari`: negative same-zone growth
  diagnostic for absolute set reserve with the wrong allocation zone.
- `std-collections-set-reserve-extra-different-zone.ari`: negative same-zone
  growth diagnostic for spare-capacity set reserve with the wrong allocation
  zone.
- `std-collections-hash.ari`: open-addressed `HashMap`/`HashSet` collision,
  replacement, tombstone, removal, and set-taking behavior.
- `std-collections-hash-iter.ari`: live-bucket `HashMap.keys`,
  `HashMap.values`, `HashSet.iter`, and direct `HashSet` `IntoIterator`
  behavior after tombstone reuse.
- `std-collections-hash-map-keys-after-reset.ari`: negative source-zone
  provenance diagnostic for hash map key cursors after reset.
- `std-collections-hash-map-values-after-reset.ari`: negative source-zone
  provenance diagnostic for hash map value cursors after reset.
- `std-collections-hash-set-iter-after-reset.ari`: negative source-zone
  provenance diagnostic for hash set iterators after reset.
- `std-collections-tree.ari`: red-black `TreeMap`/`TreeSet` insertion,
  replacement, lookup, and rotation-path behavior.
- `std-collections-tree-iter.ari`: sorted `TreeMap.keys`, `TreeMap.values`,
  `TreeSet.iter`, and direct `TreeSet` `IntoIterator` successor traversal.
- `deque/std-collections-deque.ari`: `Deque[T]` front/back pushes and pops,
  circular growth, optional helpers, root aliasing, and direct `IntoIterator`.
- `ring-buffer/std-collections-ring-buffer.ari`: `RingBuffer[T]` full-state
  push rejection, overwrite-oldest behavior, FIFO pop order, wraparound reads,
  optional helpers, and iteration.
- `linked-list/std-collections-linked-list.ari`: `LinkedList[T]` front/back
  operations, indexed removal, freed-slot reuse, optional helpers, root
  aliasing, and direct iteration.
- `heap/std-collections-heap.ari`: `BinaryHeap[T]` and `PriorityQueue[T]`
  comparator-driven max-priority ordering, growth, optional helpers, and root
  aliases.
- `std-collections-deque-iter-after-reset.ari`: negative source-zone
  provenance diagnostic for deque cursors after reset.
- `std-collections-ring-buffer-after-reset.ari`: negative source-zone
  provenance diagnostic for ring buffer handles after reset.
- `std-collections-linked-list-iter-after-reset.ari`: negative source-zone
  provenance diagnostic for linked-list cursors after reset.
- `std-collections-deque-push-different-zone.ari`: negative same-zone growth
  diagnostic for deque push with the wrong allocation zone.
- `std-collections-linked-list-push-different-zone.ari`: negative same-zone
  growth diagnostic for linked-list push with the wrong allocation zone.
- `std-collections-binary-heap-push-different-zone.ari`: negative same-zone
  growth diagnostic for binary-heap push with the wrong allocation zone.
- `std-collections-priority-queue-push-different-zone.ari`: negative
  same-zone growth diagnostic for priority-queue push with the wrong
  allocation zone.
- `std-collections-tree-map-keys-after-reset.ari`: negative source-zone
  provenance diagnostic for tree map key cursors after reset.
- `std-collections-tree-map-values-after-reset.ari`: negative source-zone
  provenance diagnostic for tree map value cursors after reset.
- `std-collections-tree-set-iter-after-reset.ari`: negative source-zone
  provenance diagnostic for tree set iterators after reset.
- `std-io-byte-slice.ari`: source `std::io` byte-slice output over the raw
  write-byte backend hook.
- `std-io-traits-cursor.ari`: source `std::io` `Reader`/`Writer`/`Seek`,
  `Cursor`, `stdin`, `stdout`, `read_exact`, `write_all`, and `flush`
  behavior plus generated helper symbol checks.
- `std-io-stderr.ari`: source `std::io` `Stderr` writer behavior, stderr
  routing, explicit flush success, stdout/stderr separation, and generated
  helper symbol checks.
- `std-io-buffered.ari`: source `std::io` `BufReader`/`BufWriter` behavior
  with caller-provided buffers, exact reads, explicit flush, stdout output,
  and generated helper symbol checks.
- `std-mem-byte-ops.ari`: `std::mem` byte `copy_bytes`, `move_bytes`, and
  `set_bytes` behavior plus LLVM `memcpy`/`memmove`/`memset` intrinsic
  lowering checks.
- `std-input-byte-option.ari`: source `std::input` EOF-to-Option byte helper
  behavior over the raw stdin hook.
- `std-cmp-value-helpers.ari`: source `std::cmp` trait-bound value selection,
  clamping, inclusive range predicates, and root re-export behavior.
- `std-convert-value-helpers.ari`: source `std::convert` identity and
  trait-bound conversion helper behavior.
- `std-math-integer-helpers.ari`: source `std::math` i64 sign predicate and
  helper behavior.
- `std-math-division-rounding.ari`: source `std::math` signed division
  rounding and paired floor remainder behavior.
- `std-ascii-class-helpers.ari`: source `std::ascii` extended byte
  classification behavior.
- `std-ascii-case-compare.ari`: source `std::ascii` borrowed-slice
  case-insensitive comparison behavior.
- `std-ascii-case-search.ari`: source `std::ascii` borrowed-slice
  case-insensitive first-match search behavior.
- `std-ascii-prefix-parsers.ari`: source `std::ascii` prefix parser result
  shape and consumed-byte behavior.
- `std-ascii-slice-helpers.ari`: source `std::ascii` borrowed-slice trimming
  and integer parsing behavior.
- `std-bits-mask-helpers.ari`: source `std::bits` u64 mask and alignment
  helper behavior.
- `std-bits-rotate-helpers.ari`: source `std::bits` u64 rotate behavior and
  modulo-64 count handling.
- `std-bits-scan-helpers.ari`: source `std::bits` u64 bit-scan and zero edge
  case behavior.
- `std-bits-one-run-helpers.ari`: source `std::bits` u64 leading/trailing
  one-run bit-scan behavior.
- `std-bits-width-helpers.ari`: source `std::bits` u64 bit-width,
  power-of-two rounding, and low-mask behavior.
- `std-zone-alloc-array.ari`: source `std::zone` typed raw array allocation,
  root re-export, zero-count null return, and raw pointer load/store behavior.
- `std-boxed-as-ptr-after-reset.ari`: negative zone provenance diagnostic.
- `prelude-option-result-methods.ari`: root prelude ADT method behavior.
- `prelude-option-result-predicates.ari`: consuming Option/Result payload
  predicate helpers.
- `prelude-option-filter.ari`: borrowed-predicate Option filtering behavior.
- `prelude-option-flatten.ari`: nested Option flattening behavior.
- `prelude-option-transpose.ari`: Option/Result transposition behavior.
- `prelude-result-transpose.ari`: Result/Option transposition behavior.
- `prelude-option-result-combinators.ari`: Option/Result map, and_then,
  eager fallback, and lazy fallback combinator behavior.
- `prelude-option-result-conversions.ari`: Option/Result conversion, xor, and
  lazy fallback behavior.
- `prelude-macro-format-no-default-zone.ari`: macro or implicit prelude
  diagnostic.
- `zone-promote-return.ari`: compiler-known zone lifetime behavior.
- `std-library-smoke.ari`: cross-module integration smoke.

Prefer one focused behavior per file. If a test intentionally covers a family
of tiny methods, make that clear in the name, such as `metadata-methods`,
`fixed-ops`, or `slice-compare`.

## Check Targets

Run these depending on the change:

- `make check-std-api`: public API manifest only.
- `make check-prelude`: source `std`, prelude aliases, formatting, zones,
  `Box`, `String`, and source `Vec` coverage.
- `make check-ffi`: C FFI, pointer, layout, and explicit zone backend coverage.
- `make check`: full default suite after compiler or public API changes.
- `make check-sanitize`: preferred for parser, semantic checker, ownership,
  zone provenance, or codegen changes.

## Adding A Library Test

1. Put valid standard-library programs under `tests/cases/standard-library/ok/<feature>/`; put diagnostics under `tests/cases/standard-library/errors/<feature>/`.
2. Use a file name that starts with the library category.
3. Add the test to the narrowest `tests/Makefile` target.
4. For runtime behavior, compile to an executable and check the exit code or
   stdout.
5. For lowering behavior, emit LLVM and grep for stable function names,
   builtin calls, visibility, or layout operations.
6. If the test introduces a public API, update `tests/std_api_manifest.txt`
   with a coverage note.
7. Update user-facing docs when behavior is visible to Ari programmers.

## What To Test For Libraries

Every public library API should eventually have:

- a simple positive call
- an executable result check
- a source path check through `std::module::name`
- a prelude/root alias check when the API is re-exported
- negative diagnostics for wrong types, wrong arity, invalid ownership, or
  invalid zone use
- LLVM checks when the API depends on runtime hooks, layout, ABI, or symbol
  names
- ownership and borrowing interaction tests when the API moves, drops, borrows,
  or returns references

Do not add broad tests that only prove "the whole library works". Add one
cross-library smoke test for integration confidence, then keep detailed tests
small and local.
