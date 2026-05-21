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
- `std-vec-sequence.ari`: source `std::vec::Vec[T]` direct borrowed
  `slice`, `split_at`, subsequence search, lexicographic compare, chunks,
  windows, delimiter splitting, reverse, rotation, sorting, binary search,
  lower/upper/equal-range bounds, partition point, owned consecutive
  deduplication, live-prefix fill/copy, borrowed-predicate partition, and
  min/max wrappers over live storage.
- `std-vec-convenience-api.ari`: source `std::vec::Vec[T]` practical
  convenience surface for associated `with_capacity`, `try_reserve`,
  `shrink_to_fit`, `extend`, `append`, `insert_many`, `remove_range`,
  `splice`, `drain`, `truncate`, `resize`, and `clear`.
- `prelude-slice-metadata.ari`: root `Slice[T]` borrowed metadata helper
  behavior.
- `prelude-slice-option-access.ari`: root `Slice[T]` Option-returning access
  behavior.
- `prelude-slice-convenience.ari`: root `Slice[T]` affix stripping, endpoint
  splitting, mutable element borrows, and borrowed-predicate search/count
  helpers.
- `prelude-slice-sequence.ari`: root `Slice[T]` range views, split views,
  subsequence search, lexicographic compare, chunks, windows, delimiter
  splitting, and receiver-form algorithm wrappers including sorted/equal-range
  bounds and partition point.
- `std-iter-adapters.ari`: source `std::iter` lazy `map`, `filter`, `take`,
  `skip`, `enumerate`, `zip`, eager `fold`, `reduce`, and zone-backed
  `collect` behavior over `std::vec::Iter[T]`.
- `std-iter-exact-size.ari`: root `ExactSizeIterator[T]` supertrait behavior,
  including child-bound access to parent `Iterator[T]::next` and exact
  remaining-length checks for vector and slice cursors.
- `std-iter-double-ended.ari`: root `DoubleEndedIterator[T]` supertrait
  behavior, including mixed front/back consumption and parent `Iterator[T]`
  method access for vector, slice, set, deque, and ring-buffer cursors.
- `std-string-append-u64.ari`: source `std::string` formatting helper.
- `std-string-append-debug.ari`: source `std::string` generic Debug append
  helper, implicit same-zone lowering, and quoted built-in debug text.
- `std-string-split-join.ari`: source `std::string` byte-slice search,
  borrowed split/chunk/window views, and allocator-backed `join_in`.
- `std-string-natural-api.ari`: source `std::string` literal-oriented
  constructors, append helpers, text search/comparison wrappers, owned trim
  aliases, and validated UTF-8 view access.
- `std-string-ascii-helpers.ari`: source `std::string` borrowed ASCII trim
  views and whole-string parsing behavior.
- `std-string-ascii-case-helpers.ari`: source `std::string` ASCII-only
  case-insensitive comparison and first-match search helpers.
- `std-string-unicode-helpers.ari`: source `std::string` UTF-8 validation,
  code-point access, and scalar append convenience methods.
- `std-string-text-kinds.ari`: source `std::string` typed borrowed
  `Utf8`/`OsStr` views, shared `std::c::CStr` construction, C string byte
  slicing, and OS-to-UTF-8 validation behavior.
- `std-string-literal-boundaries.ari`: direct string-literal coercion into
  expected `Utf8`, `OsStr`, `PathBytes`, and `CStr` boundary views.
- `std-string-byte-literals.ari`: byte character literals, literal byte arrays,
  and `std::string::bytes` views over lowercase `string` values.
- `std-string-prefix-parsers.ari`: source `std::string` prefix parser
  conveniences over the `std::ascii` parser result shape.
- `std-string-signed-parsers.ari`: source `std::string` signed decimal parser
  conveniences over the `std::ascii` optional-sign parser policy.
- `std-string-trim-copy.ari`: source `std::string` owned ASCII trim copies
  into a target zone, including source-zone reset behavior.
- `std-string-trim-to-after-target-reset.ari`: negative target-zone
  provenance diagnostic for copied trim results.
- `std-string-append-debug-different-zone.ari`: negative same-zone diagnostic
  for explicit Debug string appends.
- `std-string-try-byte-access.ari`: source `std::string` Option-returning
  byte access and empty pop behavior.
- `std-fmt-format-spec.ari`: source `std::fmt::FormatSpec` helper behavior,
  unsigned radix/width/precision/alignment formatting, debug text quoting,
  explicit-zone strings, and `io::Writer` output.
- `std-fmt-debug-values.ari`: source `std::fmt::Debug` trait dispatch,
  built-in and user debug impls, Writer-backed debug output, and stdout debug
  helpers.
- `std-fmt-char-values.ari`: source `char` Display/Debug formatting,
  `format_in!` byte-character append lowering, and debug escape behavior.
- `format-named-capture.ari`: compiler formatting named local capture for
  direct stdout formatting and explicit-zone `format_in!`, including
  field/tuple-index capture and mixed named/positional placeholders.
- `prelude-format-in-debug.ari`: `format_in!` `{:?}` placeholder lowering,
  custom `Debug::debug_in` dispatch, built-in debug values, and one-evaluation
  string assembly behavior.
- `std-context-args.ari`: runtime-backed `std::context` argument, startup cwd,
  startup executable path, and thread-id access, source context predicates, and
  root alias behavior.
- `std-test-report.ari`: source `std::test::Report` aggregation, generic
  equality helpers, method wrappers, explicit finish status, and scratch zone
  creation.
- `std-error-basic.ari`: source `std::error::Kind` and compact `Error`
  behavior, POSIX errno mapping, root aliases, predicate helpers, string names,
  direct `Result[T, Error]` conversion, and raw scalar compatibility bridges.
- `std-c-interop.ari`: source `std::c` C ABI string views, zone-backed
  NUL-terminated `CString`, libc alias calls, POSIX errno mapping, dynamic
  loader handles, and root aliases.
- `std-c-dynamic-function.ari`: source `std::c::Symbol` to typed function
  pointer extraction and indirect dynamic calls.
- `std-target-basic.ari`: compiler-known `std::target` triple,
  architecture, OS, environment/libc, object/debug format, errno ABI, syscall
  ABI, pointer-width, and Linux API-family predicate behavior.
- `std-target-linux64.ari`: LLVM-only target classification coverage for
  x86_64, aarch64, and riscv64 Linux triples without cross-linking.
- `std-env-args.ari`: source `std::env` argument helpers, `Option`-returning
  `try_arg`, and `program_name` behavior over the runtime context.
- `std-env-vars.ari`: runtime-backed `std::env` environment variable
  `get`/`has`/`try_get`/`set`/`remove` behavior.
- `std-env-paths.ari`: runtime-backed `std::env` current directory,
  `set_current_dir`, executable path, and `Option` wrapper behavior.
- `std-env-os-path-views.ari`: `std::env` `OsStr` argument/environment/path
  views plus `PathBytes` current-directory conversion.
- `std-process-basic.ari`: runtime-backed `std::process::id` plus source
  success/failure status helper behavior.
- `std-process-identity.ari`: runtime-backed `std::process::uid`/`gid` plus
  source `is_root` behavior.
- `std-process-exit.ari`: runtime-backed explicit process exit status
  behavior.
- `std-process-abort.ari`: abort hook lowering while the executable follows a
  non-aborting path.
- `std-process-fork-wait.ari`: runtime-backed POSIX `fork`/`wait` behavior,
  child/parent predicates, explicit child exit, and wait failure sentinel.
- `std-process-result.ari`: direct `Result[i64, Error]` process fork/wait
  helpers, normal child exit propagation, waitpid errno payloads, and raw
  compatibility wait sentinel behavior after the child has been consumed.
- `std-process-exit-status.ari`: typed `ExitStatus` process results, including
  `wait_status_result`, `Command::exit_status`, `Child::wait_status`, normal
  exit-code access, signal termination access, and compatibility `status`.
- `std-process-command.ari`: first `Command`/`Child` builder slice, including
  argv construction through `process::arg`, child environment setup,
  working-directory setup, method and module-level `status`/`spawn`,
  module-level `exit_status`, `Child::wait`, and non-destructive `kill(0)`
  probes.
- `std-process-output.ari`: `Command::output_in` small stdout/stderr capture,
  module-level `process::output_in`, `Output` accessors, missing-command `127`
  status, `pipe`/`dup2` lowering, and compiler zone provenance for the
  zone-backed `Output` handle.
- `std-thread-basic.ari`: runtime-backed `std::thread` function-pointer
  spawn/join behavior, child runtime ids, invalid-handle sentinels, root
  `Thread`, method wrappers, and scheduler yield.
- `std-thread-runtime-helpers.ari`: runtime-backed `std::thread`
  `available_parallelism`, source `sleep` wrapper over `std::time`, child
  thread use of both helpers, and `sysconf`/sleep hook lowering checks.
- `std-sync-atomic-i64.ari`: runtime-backed `std::sync::AtomicI64`
  load/store/swap/fetch-add/compare-exchange behavior, method wrappers, root
  alias, and LLVM atomic lowering.
- `std-sync-mutex-once.ari`: source `std::sync::Mutex` try/lock/unlock
  behavior, source `std::sync::Once` call-once state transitions, method
  wrappers, root aliases, and compare-exchange-backed lowering.
- `std-sync-rwlock.ari`: source `std::sync::RwLock` read/write lock
  transitions, reader-count diagnostics, method wrappers, root alias, and
  compare-exchange/fetch-add-backed lowering.
- `std-time-basic.ari`: runtime-backed monotonic and wall-clock nanosecond
  reads, source duration/instant wrappers, elapsed-time helpers, and sleep.
- `std-time-timeout.ari`: source monotonic `Deadline`/timeout helpers,
  remaining-time checks, and zero-duration sleep-until behavior.
- `std-time-utc-calendar.ari`: UTC-only Unix timestamp conversion,
  leap-year/month-length policy, and `SystemTime::to_utc` accessors.
- `std-fs-basic.ari`: runtime-backed file existence, mode-string
  creation/truncating write, byte reads/writes, close, removal, and
  `Option[File]` open helpers.
- `std-fs-append.ari`: runtime-backed `"a"` append mode, preservation of
  existing bytes, appended byte-slice writes, and failed append opens as
  `Option`.
- `std-fs-open-modes.ari`: mode-string `open`/`try_open` behavior for `"r"`,
  `"w"`, `"a"`, `"rw"`, `"r+"`, `"w+"`, `"a+"`, empty modes, and invalid
  mode strings.
- `std-fs-open-options.ari`: `OpenOptions` value builder behavior for
  exclusive creation, non-truncating read/write, append-with-read, and invalid
  option combinations.
- `std-fs-open-result.ari`: `open_result`, `create_result`, and
  `OpenOptions::open_result` direct `Error` results, plus raw compatibility
  bridges for invalid input, missing files, exclusive-create failures, and
  successful handles.
- `std-fs-read-write.ari`: source whole-file `write`/`try_write`,
  `append`/`try_append`, `read_to_string`, missing-file empty reads,
  byte-count checks, and truncating rewrite behavior.
- `std-fs-read-result.ari`: `read_result` and `read_to_string_result` direct
  `Error` whole-file reads, including successful owned byte-string reads,
  missing-file `NotFound`, and compatibility `try_read` absence.
- `std-fs-query-result.ari`: direct `Error` query helpers for metadata,
  no-follow metadata, path kind, POSIX mode, canonicalization, symbolic-link
  target reads, directory open, and one-shot directory listings.
- `std-fs-byte-result.ari`: `write_result`, `append_result`, and
  `copy_result` direct `Error` byte-count results plus raw compatibility open
  failures.
- `std-fs-try-read.ari`: `Option[String]` whole-file reads that distinguish
  missing files from empty files.
- `std-fs-create-truncate-copy.ari`: source `create`, `try_create`, natural
  `read`, `truncate`, streaming `copy`/`try_copy`, byte-count checks, and
  missing-source copy failure.
- `std-fs-io-traits.ari`: `File` as generic `std::io::Reader`/`Writer`,
  `read_to_string`, EOF through `read_exact`, file-to-file `try_copy`,
  whole-slice `write_all`, direct-descriptor `flush`, and invalid handles.
- `std-fs-seek.ari`: runtime-backed `File` cursor positioning, direct
  `fs::position`/`fs::seek` hooks, method syntax through `std::io::Seek`,
  generic `S: io::Seek` dispatch, and negative seek rejection.
- `std-fs-rename-dir.ari`: runtime-backed `rename`, single-directory
  `create_dir`/`remove_dir`, duplicate-create failure, and missing-source
  rename failure.
- `std-fs-mutation-result.ari`: `remove_result`, `rename_result`,
  `create_dir_result`, and `remove_dir_result` unit-success direct `Error`
  results plus raw compatibility bridges.
- `std-fs-create-dir-all.ari`: runtime-backed recursive `create_dir_all` and
  source `ensure_dir_all`, existing-directory idempotence, file-path
  rejection, blocked child creation, nested writes, and cleanup.
- `std-fs-remove-dir-all.ari`: source recursive `remove_dir_all`, missing-path
  and non-directory-root rejection, empty-directory removal, file and symlink
  unlinking inside the tree, and no-follow behavior for directory symlinks.
- `std-fs-ensure-file.ari`: source `ensure_file` idempotence, existing-file
  preservation, missing-file creation, directory rejection, missing-parent
  failure, and cleanup.
- `std-fs-read-dir.ari`: runtime-backed `Dir` open/next/close, one-shot
  `try_read_dir`/`read_dir`, lightweight `DirEntry` name/path helpers,
  dot-entry skip, missing-directory failure, invalid-handle `None`, and
  cleanup.
- `std-fs-dir-entry-metadata.ari`: lazy metadata helpers on collected
  `DirEntry` values, including target-following metadata, no-follow symlink
  metadata, path-kind predicates, byte length, permission snapshots, and reuse
  of stored zone-backed entry paths.
- `errors/fs/std-fs-dir-entry-different-zone.ari`: rejects manually constructed
  `DirEntry` values whose zone-backed name and path come from different zones.
- `std-fs-links.ari`: runtime-backed `hard_link` and `symbolic_link`,
  read-through behavior, missing-source failure, and duplicate-link failure.
- `std-fs-read-link.ari`: runtime-backed symbolic-link target reads,
  `Option[String]` failure for regular and missing paths, and asserting
  `read_link` behavior.
- `std-fs-symlink-metadata.ari`: runtime-backed no-follow `lstat` metadata,
  target-following `metadata` behavior over links, direct `is_symlink`
  behavior, and missing-path `None`.
- `std-fs-permissions.ari`: runtime-backed access permission checks,
  `Permissions` method wrappers, directory execute/search checks, and
  missing-path all-false behavior.
- `std-fs-mode.ari`: runtime-backed permission mode lookup, chmod mutation call,
  structured `Permissions` constructors/conversion, invalid-mode rejection,
  missing-path failure, and host-filesystem-independent cleanup.
- `std-fs-metadata.ari`: runtime-backed target-following `stat` metadata
  checks, missing-path `None`, regular-file length/kind, directory kind, and
  `Metadata` methods.
- `std-fs-metadata-times.ari`: access, modification, and POSIX status-change
  timestamp methods on target-following metadata, no-follow symlink metadata,
  and stored-path `DirEntry` metadata.
- `std-fs-canonicalize.ari`: runtime-backed `realpath` canonicalization,
  absolute owned paths, filename preservation, and missing-path `None`.
- `std-net-addresses.ari`: source `std::net` IPv4/IPv6 constructors, generic
  IP predicates, socket-address construction, port replacement, loopback, and
  unspecified checks.
- `std-net-address-validation.ari`: strict and fallible IPv4 octet and IPv6
  segment accessors for known-good indexes and parsed-index validation.
- `std-net-tcp-loopback.ari`: hosted IPv4 TCP listener/stream bind, local-port
  and local-address lookup, connect, accept, stream local/peer-address lookup,
  `std::io::Reader`/`Writer` byte transfer, method-style stream
  `read_exact`/`write_all`, explicit close,
  timeout/nonblocking helpers, stream shutdown, direct `Error` and raw
  compatibility result helpers, IPv6 unsupported errors, and
  restricted-host `PermissionDenied` behavior when the test environment
  forbids socket creation.
- `std-net-udp-socket.ari`: hosted IPv4 UDP bind, local-port/local-address lookup,
  timeout/nonblocking helpers, single-byte datagram send/receive, direct
  `Error` and raw compatibility result helpers, unsupported IPv6 bind errors,
  restricted-host fallback, and explicit close.
- `std-net-unix-socket.ari`: hosted Unix stream listener bind, stream connect,
  accept, direct `Error` and raw compatibility result helpers,
  timeout/nonblocking helpers, bidirectional byte and buffer IO, stream
  shutdown, close, and socket-file cleanup.
- `std-net-dns-lookup.ari`: hosted IPv4 numeric lookup through both `Option`
  and direct `Error`/raw compatibility `Result` APIs, unsupported IPv6 text
  input, and edge IPv4 addresses.
- `std-algo-slice-helpers.ari`: source `std::algo` sort/stable sort,
  comparator sort, binary search, lower/upper/equal-range bounds, partition
  point, reverse/rotate, partition, min/max/clamp, swap, fill, copy, and dedup
  behavior over borrowed slices, including custom `Ord` values through natural
  ordering operators.
- `std-algo-dedup-partition.ari`: source `std::algo` `dedup_by`,
  `dedup_by_key`, stable partition, and the matching natural `Slice`/`Vec`
  receiver wrappers, including owned `Vec` truncation after dedup.
- `std-algo-by-helpers.ari`: source `std::algo` comparator sortedness,
  search/bounds/equal-range, partition point, min/max/clamp, stable comparator
  ordering, and natural `Slice`/`Vec` receiver wrappers over a custom value
  with no `Ord` impl.
- `std-algo-copy-own.ari`: negative value-movement diagnostic showing that
  current borrowed-slice algorithm copying does not accept ownership-carrying
  aggregate elements as copyable array/vector views.
- `std-hash-basic.ari`: source `std::hash` deterministic `Hasher`
  construction/reset/finalization, byte-slice hashing, generic `Hash[T]`
  dispatch for primitive values, primitive write helpers, and the
  `collections::hash_i64` compatibility wrapper.
- `std-hash-integer-widths.ari`: fixed-width signed and unsigned integer
  writer helpers, generic `Hash[T]` dispatch, and width-distinct byte feeds.
- `std-hash-combine-helpers.ari`: ordered two-value hashing, precomputed
  `u64` hash composition, and generic `Slice[u8]` hashing.
- `std-random-basic.ari`: runtime-backed OS entropy hook reachability, direct
  runtime OS byte filling, deterministic non-cryptographic `Prng` behavior,
  deterministic booleans, unbiased bounded integers including a wide upper
  bound, unit floats, PRNG byte filling, and generic slice shuffle.
- `std-random-result.ari`: `Result`-returning OS entropy and byte-fill helpers,
  raw compatibility result bridges, OS-seeded `Prng` result constructors, and
  recoverable invalid-slice error mapping.
- `std-parse-basic.ari`: source `std::parse` ASCII-trimmed signed
  decimal/radix/hex/binary/octal integer, bool, decimal float
  validation/conversion, fallback, and invalid whole-input behavior.
- `std-encoding-text.ari`: source `std::encoding` ASCII, UTF-8, and UTF-16
  validation/counting behavior.
- `std-encoding-utf8-codepoints.ari`: source `std::encoding` UTF-8 lead-byte
  width, Unicode scalar validation, byte-offset decoding, next-index, and
  scalar encoding behavior.
- `std-encoding-codec.ari`: source `std::encoding` hex/base64 length,
  encode, decode, and invalid-input guard behavior.
- `std-path-basic.ari`: source `std::path` POSIX-style separator policy,
  absolute/relative checks, borrowed component views, join, and lightweight
  normalization behavior.
- `std-path-predicates.ari`: source `std::path` allocation-free
  final-component predicate helpers and `PathBytes` receiver wrappers.
- `std-path-affixes.ari`: source `std::path` component-aware prefix/suffix
  predicates, borrowed affix stripping, root-prefix behavior, and `PathBytes`
  receiver wrappers.
- `std-path-components.ari`: source `std::path::components` lazy borrowed
  iterator behavior for absolute paths, repeated separators, trailing
  separators, and root-only paths.
- `std-path-bytes.ari`: source `std::path::PathBytes` typed path-byte view,
  conversion from OS string bytes, and method-style path helper wrappers.
- `std-collections-set.ari`: source `std::collections::Set[T]` constructor,
  insertion, duplicate rejection, membership, removal, borrowed view, copy, and
  target/source zone behavior.
- `std-collections-set-access.ari`: source set insertion-order accessors,
  optional accessors, explicit reserve growth, `pop`, and `try_pop` behavior.
- `std-collections-set-replace.ari`: source set replace-or-insert behavior,
  returned previous values, missing-value insertion, and growth behavior.
- `std-collections-set-implicit-zone.ari`: tracked local set calls infer the
  source zone for insert, replace, reserve, and reserve-extra growth methods.
- `std-collections-implicit-zone.ari`: tracked local hash, tree, deque,
  linked-list, heap, and priority-queue calls infer the source zone for
  mutating growth methods, including spare-capacity `reserve_extra` where the
  collection can grow after construction.
- `std-collections-set-iter.ari`: source set cursor iteration, direct
  `for value in set.iter()`, and `IntoIterator` lowering for `for value in set`.
- `std-collections-copy-to.ari`: target-zone copies for hash and tree maps/sets,
  including tombstone skipping and post-source-destroy reads from copied
  storage.
- `std-collections-structure-copy-to.ari`: target-zone copies for deque, ring
  buffer, linked list, binary heap, and priority queue after source-zone
  destroy, covering logical order and priority pop order.
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
- `std-collections-set-implicit-zone-untracked.ari`: negative diagnostic for
  omitting the zone on a set handle with no tracked allocation provenance.
- `std-collections-implicit-zone-untracked.ari`: negative diagnostic for
  omitting the zone on a non-set collection handle with no tracked allocation
  provenance.
- `std-collections-hash.ari`: open-addressed `HashMap`/`HashSet` collision,
  replacement, tombstone, removal, and set-taking behavior.
- `std-collections-hash-iter.ari`: live-bucket `HashMap.keys`,
  `HashMap.values`, `HashSet.iter`, and direct `HashSet` `IntoIterator`
  behavior after tombstone reuse.
- `std-collections-set-representatives.ari`: `HashSet.get`/`try_get` and
  `TreeSet.get`/`try_get` representative lookup before and after
  replacement/removal paths.
- `std-collections-map-natural-api.ari`: natural map lookup spellings,
  including compatibility `contains`, preferred `contains_key`, and fallback
  `get_or` for both hash and tree maps.
- `std-collections-map-mut-access.ari`: asserting `get_mut`, optional
  `try_get_mut` value handles, named `replace`, and removal for both hash and
  tree maps.
- `std-collections-map-entry-api.ari`: Rust-style hash/tree map update
  handles, including `entry(key)`, `or_insert`, `or_insert_with`,
  `and_modify`, direct `+=` through returned `ref mut V`, and
  `remove_entry`.
- `std-collections-map-entry-accessors.ari`: live `HashMapEntry` and
  `TreeMapEntry` key/value access, mutable value borrowing, direct replacement,
  direct entry removal, and copied `MapEntry.key()`/`value()` accessors.
- `std-collections-view-api.ari`: `HashMap`/`TreeMap` `values_mut` cursors,
  map `iter()` aliases, `MapEntryMut`-based `iter_mut()` cursors, direct map
  `IntoIterator`, and draining cursors for linear, hash, and tree maps/sets.
- `std-collections-hash-map-keys-after-reset.ari`: negative source-zone
  provenance diagnostic for hash map key cursors after reset.
- `std-collections-hash-map-values-after-reset.ari`: negative source-zone
  provenance diagnostic for hash map value cursors after reset.
- `std-collections-hash-set-iter-after-reset.ari`: negative source-zone
  provenance diagnostic for hash set iterators after reset.
- `std-collections-tree.ari`: red-black `TreeMap`/`TreeSet` insertion,
  replacement, lookup, and rotation-path behavior.
- `std-collections-tree-entry-boundaries.ari`: `TreeMap.first_entry`,
  `TreeMap.last_entry`, and optional boundary-entry helpers before and after
  direct red-black deletion.
- `std-collections-tree-bounds.ari`: `TreeMap.lower_bound`,
  `TreeMap.upper_bound`, `TreeSet.lower_bound`, and `TreeSet.upper_bound`
  nearest-value lookup over comparator order.
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
- `std-io-read-all.ari`: source `std::io` `read_all` collection over generic
  `Reader` values, byte order, EOF after collection, and generated generic
  helper symbol checks.
- `std-io-read-to-string.ari`: source `std::io` `read_to_string` collection
  over generic `Reader` values, owned byte-string contents, EOF after
  collection, and generated generic helper symbol checks.
- `std-io-copy.ari`: source `std::io` `try_copy` byte counts, `copy` bool
  wrapper behavior, writer flush, writer failure handling, and generated
  generic helper symbol checks.
- `std-io-result.ari`: source `std::io` direct `Result[..., Error]` helpers
  for exact reads, whole-slice writes, explicit flushes, and generic stream
  copies, including `UnexpectedEof`, `BrokenPipe`, and generic flush-error
  classification plus compatibility wrapper delegation.
- `std-io-stderr.ari`: source `std::io` `Stderr` writer behavior, stderr
  routing, explicit flush success, stdout/stderr separation, and generated
  helper symbol checks.
- `std-io-buffered.ari`: source `std::io` `BufReader`/`BufWriter` behavior
  with caller-provided buffers, exact reads, explicit flush, stdout output,
  and generated helper symbol checks.
- `std-log-basic.ari`: source `std::log` level ordering, threshold checks,
  byte-slice logging, string-message logging, convenience level helpers, and
  stderr output format.
- `std-mem-byte-ops.ari`: `std::mem` byte `copy_bytes`, `move_bytes`, and
  `set_bytes` behavior plus LLVM `memcpy`/`memmove`/`memset` intrinsic
  lowering checks.
- `std-mem-pointer-ops.ari`: natural raw pointer `+`, `-`, dereference load,
  and dereference store operators alongside explicit `ptr_*` helpers, with
  typed and byte GEP lowering checks.
- `std-mem-page-size.ari`: `std::mem::page_size` hosted runtime hook lowering
  and basic page-size invariants.
- `std-input-byte-option.ari`: source `std::input` EOF-to-Option byte helper
  behavior over the raw stdin hook.
- `std-cmp-value-helpers.ari`: source `std::cmp` trait-bound value selection,
  clamping, inclusive range predicates, root re-export behavior, and generic
  helper dispatch through natural ordering operators.
- `std-cmp-by-helpers.ari`: source `std::cmp` comparator-based three-way
  comparison, ordering chaining, min/max/clamp/range helpers, method wrapper,
  and root re-export behavior over a custom type with no `Ord` impl.
- `std-cmp-primitive-impls.ari`: standard
  `Eq`/`PartialEq`/`Ord`/`PartialOrd` impl availability for common primitive
  scalar types without per-test or per-program boilerplate.
- `std-cmp-equality-operator.ari`: trait-backed `==` and `!=` lowering through
  `cmp::Eq[T]::eq` for concrete and generic values.
- `std-cmp-order-operators.ari`: trait-backed `<`, `<=`, `>`, and `>=`
  lowering through `cmp::Ord[T]::lt` for concrete and generic values.
- `std-convert-value-helpers.ari`: source `std::convert` identity and
  trait-bound conversion helper behavior.
- `std-convert-try-helpers.ari`: source `std::convert` fallible
  `TryFrom`/`TryInto` helper dispatch with success and `None` cases.
- `std-math-integer-helpers.ari`: source `std::math` i64 sign predicate and
  helper behavior.
- `std-math-division-rounding.ari`: source `std::math` signed division
  rounding and paired floor remainder behavior.
- `std-math-checked-saturating.ari`: source `std::math` checked
  add/sub/mul/div/rem/neg/abs and saturating add/sub/mul/div/neg/abs overflow
  policy behavior.
- `std-math-wrapping-overflowing.ari`: source `std::math` wrapping
  add/sub/mul and overflowing add/sub/mul tuple-result behavior.
- `std-ascii-class-helpers.ari`: source `std::ascii` extended byte
  classification behavior.
- `std-ascii-case-compare.ari`: source `std::ascii` borrowed-slice
  case-insensitive comparison behavior.
- `std-ascii-case-search.ari`: source `std::ascii` borrowed-slice
  case-insensitive first-match search behavior.
- `std-ascii-prefix-parsers.ari`: source `std::ascii` prefix parser result
  shape and consumed-byte behavior.
- `std-ascii-signed-parsers.ari`: source `std::ascii` optional-sign decimal
  parser behavior for whole-slice and prefix forms.
- `std-ascii-overflow-parsers.ari`: source `std::ascii` decimal, signed
  decimal, and hexadecimal parser overflow rejection at `i64` boundaries.
- `std-ascii-slice-helpers.ari`: source `std::ascii` borrowed-slice trimming
  and integer parsing behavior.
- `std-bits-mask-helpers.ari`: source `std::bits` u64 mask and alignment
  helper behavior.
- `std-bits-alignment-policy.ari`: source `std::bits` checked and wrapping
  alignment policy, including invalid alignment and overflow cases.
- `std-bits-rotate-helpers.ari`: source `std::bits` u64 rotate behavior and
  modulo-64 count handling.
- `std-bits-scan-helpers.ari`: source `std::bits` u64 bit-scan and zero edge
  case behavior.
- `std-bits-one-run-helpers.ari`: source `std::bits` u64 leading/trailing
  one-run bit-scan behavior.
- `std-bits-width-helpers.ari`: source `std::bits` u64 bit-width,
  power-of-two rounding, and low-mask behavior.
- `std-bits-byte-population.ari`: source `std::bits` u64 byte-swap behavior
  and the `population_count` alias.
- `std-zone-alloc-array.ari`: source `std::zone` typed raw array allocation,
  root re-export, zero-count null return, and raw pointer load/store behavior.
- `std-zone-backed.ari`: source `std::zone::ZoneMetadata`,
  `std::zone::ZoneBacked`, `zone::metadata(data)`, `zone::of(ref value)`, and
  `value.zone()` agree with allocation-header metadata for boxed, string,
  vector, set, map, sequence, linked list, heap, and priority queue handles.
- `std-boxed-as-ptr-after-reset.ari`: negative zone provenance diagnostic.
- `prelude-option-result-methods.ari`: root prelude ADT method behavior.
- `prelude-option-result-predicates.ari`: consuming Option/Result payload
  predicates and exact value-membership helpers.
- `prelude-option-filter.ari`: borrowed-predicate Option filtering behavior.
- `prelude-option-flatten.ari`: nested Option flattening behavior.
- `prelude-option-transpose.ari`: Option/Result transposition behavior.
- `prelude-result-transpose.ari`: Result/Option transposition behavior.
- `prelude-option-result-combinators.ari`: Option/Result map,
  eager/lazy `map_or`, borrowed `inspect`/`inspect_err`, eager `and`, lazy
  `and_then`, eager fallback, lazy fallback, and error-preserving `Result.and`
  combinator behavior.
- `prelude-option-result-conversions.ari`: Option/Result conversion, xor, and
  lazy fallback behavior.
- `prelude-macro-format-no-default-zone.ari`: macro or implicit prelude
  diagnostic.
- `prelude-format-in-no-debug.ari`: negative `format_in!` `{:?}` diagnostic
  when a value has no `Debug` impl.
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
