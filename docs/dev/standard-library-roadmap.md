# Standard Library Roadmap

This roadmap tracks source standard library work. General compiler work remains
in [Roadmap](roadmap.md), and coverage details remain in
[Feature Test Matrix](test-matrix.md).

## Goal

Ari's libraries should be ordinary Ari source whenever possible. The compiler
should only know about primitives that the current language cannot express:
layout queries, raw pointer operations, zone runtime hooks, formatting macros,
temporary-zone lowering, and backend-specific entry/runtime glue.

The library contract is explicit and capability-oriented:

- no hidden global heap
- no source-level C++ ABI dependency
- allocations flow through visible capabilities such as `Zone`
- ownership, borrowing, and zone provenance are checked before LLVM lowering
- public APIs are tracked in `tests/std_api_manifest.txt`
- every public API has focused positive, negative, and backend coverage where
  applicable

## Current Baseline

The current `std` package already provides:

- prelude ADTs: `Option`, `Result`, `Slice`, `Range`, `RangeInclusive`
- source `Option`/`Result` predicates, combinators, conversions, nested option
  filtering, flattening, bidirectional option-result transposition, and lazy
  fallback helpers, including consuming payload predicate helpers
- assertion, panic, `move`, and `take` helpers
- IO/input/context/env declarations and source helpers such as
  `io::Reader`, `io::Writer`, `io::Seek`, `io::cursor`,
  `io::BufReader`, `io::BufWriter`, `io::stderr`, `io::read_exact`,
  `io::write_all`, `io::flush`, `io::write_bytes`,
  `input::try_read_byte`, `context::has_arg`,
  `context::user_arg_count`, `context::is_main_thread`,
  `target::triple`, `target::arch`, `target::os`, `target::env`,
  `target::pointer_bits`, `target::long_bits`, `target::syscall_abi`,
  target predicates for Linux/glibc/musl/ELF/DWARF/TLS and Linux API families,
  `env::try_arg`, `env::program_name`, `env::try_get`, and current-process
  environment/path hooks `get`/`has`/`set`/`remove`/`current_dir`/
  `set_current_dir`/`executable_path`
- the first `std::process` current-process helpers: `id`, `uid`, `gid`, `exit`,
  `abort`, and source status/root helpers, plus the first POSIX `fork`/`wait`
  child-process slice
- the first `std::thread` helpers: `Thread`, function-pointer `spawn`, `join`,
  `yield_now`, runtime thread ids, and source predicates/method wrappers
- the first `std::sync` helper: concrete `AtomicI64` with sequentially
  consistent `load`, `store`, `swap`, `fetch_add`, and `compare_exchange`
  hooks plus method wrappers
- the first `std::time` helpers: monotonic nanosecond reads, wall-clock Unix
  nanosecond reads, sleep, `Duration`, `Instant`, `SystemTime`, and source
  elapsed-time helpers
- the first `std::fs` helpers: file existence/removal, access-permission
  checks, read/write open hooks, byte reads/writes, close, source
  `File`/`Permissions` methods, `Option[File]` open helpers,
  `create`/`try_create`, whole-file `read`/`write`/`append`,
  `truncate`, streaming `copy`, `rename`, single-directory
  `create_dir`/`remove_dir`, and `read_to_string`
- the first `std::path` helpers: POSIX-style separator, absolute/relative,
  borrowed component, join, and lightweight lexical normalization helpers
- layout, pointer, value, and byte memory helpers in `std::mem`, including
  `copy_bytes`, `move_bytes`, `set_bytes` lowering through LLVM memory
  intrinsics, and hosted `page_size`
- explicit-zone allocation in `std::zone`, including the source
  `alloc_array<T>` raw buffer helper
- source handles for `Box`, `String`, and `Vec`
- source `std::collections` handles: linear insertion-order `Set[T]` with
  iterator support, `Deque[T]`, `RingBuffer[T]`, `LinkedList[T]`,
  `BinaryHeap[T]`, `PriorityQueue[T]`, open-addressed
  `HashMap[K,V]`/`HashSet[T]` with explicit hash functions and live-bucket
  iterators, and red-black-tree `TreeMap[K,V]`/`TreeSet[T]` with explicit
  comparators and sorted iterators; all use explicit-zone storage and
  provenance checks
- `Slice[T]` and `std::vec::Vec[T]` metadata, asserting element access, and
  `Option`-returning element access helpers
- `std::string::String` empty-safe byte access, byte search, comparison,
  ASCII case-insensitive comparison/search, borrowed and owned ASCII trim
  helpers, and whole/prefix ASCII parsing helpers
- range/iterator traits and the `std::vec::Iter` implementation
- comparison, formatting, and conversion trait surfaces, plus source
  comparison value helpers: `min`, `max`, `clamp`, and `is_between`
- `std::convert` source helpers: `identity`, `from`, and `into`
- `std::math` integer helpers implemented in Ari source with natural names:
  `abs`, `sign`, `is_positive`, `is_negative`, `is_zero`, `is_even`,
  `is_odd`, `pow`, `div_floor`, `div_ceil`, `mod_floor`, `gcd`, and `lcm`
- `std::ascii` byte classification, printable/control predicates, case
  conversion, borrowed-slice case-insensitive comparison/search, trimming,
  whole-slice digit parsing, and prefix digit parsing helpers
- `std::parse` whole-input ASCII-trimmed integer, bool, and decimal-float
  parsers with validation/fallback helpers
- `std::encoding` ASCII/UTF-8/UTF-16 validation and counting plus lowercase
  hex and standard base64 encode/decode helpers
- `std::bits` `u64` mask, rotation, power-of-two, low-mask, alignment, and
  zero/one-run bit-scan helpers

This baseline is useful, but it is still a seed. Some APIs are compiler hooks
with source declarations, and some names exist mainly so user code can start
depending on stable module paths.

## Phases

### Phase 1: Stabilize The Current Source `std`

- Keep `lib/std.arih` as the single public root.
- Keep child modules file-backed under `lib/std/`.
- Maintain `tests/std_api_manifest.txt` for every public declaration.
- Keep the user-facing API guide in
  [Standard Library](../language/standard-library.md).
- Split library tests by purpose using the naming scheme in
  [Library Testing](library-testing.md).
- Prefer source implementations for helper methods and combinators.

Exit criteria:

- `make check-std-api` passes.
- `make check-prelude` covers every public API family.
- New contributors can find module purpose, API shape, and tests from docs.

### Phase 2: Pull More Behavior Into Source Ari

- Move helper logic out of compiler hooks when structs, generic aggregates, and
  trait dispatch can express it safely.
- Keep compiler-known declarations as compatibility shims only when source
  lowering cannot model the primitive yet.
- Do not make LLVM codegen re-resolve source names; sema should lower the IR
  metadata needed by the backend.

Likely compiler work:

- generic aggregate/type monomorphization cleanup
- stronger trait-bound dispatch for reusable helper impls
- richer module-cache summaries for source library bodies
- clearer diagnostics when a partial custom `std` omits required helpers

### Phase 3: Collections And Allocation

- Stabilize `std::vec::Vec[T]` as the source growable vector handle.
- Keep root bare `Vec[T]` and source `std::Vec[T]` distinctions documented
  until the compiler-known local vector model is unified or retired.
- Add collection APIs only when ownership, borrowing, and zone provenance can
  be tested in focused slices.
- Grow collection families in this order: slice helpers, vector methods,
  linear `Set[T]`, hash tables, red-black trees, iterators for hash/tree
  containers, deque/ring-buffer/linked-list/heap families, red-black deletion,
  then trait-driven constructors. The iterator slice is now in place for live
  hash buckets, sorted tree traversal, deques, ring buffers, and linked lists.
- Keep `Set[T]` linear and insertion-order. Hash-backed behavior belongs in
  `HashMap`/`HashSet`, so call sites stay honest and natural.
- Keep `HashMap`/`HashSet` and `TreeMap`/`TreeSet` on explicit hash/comparator
  constructors until `Hash`, `Eq`, and `Ord` trait dispatch can drive generic
  containers directly. Public names should remain natural (`insert`, `get`,
  `contains`, `remove`) rather than type-suffixed.

Likely compiler work:

- generic aggregate monomorphization for richer collection layouts
- iterator protocol diagnostics beyond direct `range` and current `Vec` support
- allocation-zone diagnostics for nested and generic wrapper types
- trait-driven `Hash`/`Eq`/`Ord` constructor selection

### Phase 4: Text, Formatting, And Diagnostics

- Keep `std::string::String` as a byte string until a deliberate Unicode/text
  policy is designed.
- Add formatting APIs through `std::fmt` only when `Display` and `Debug`
  behavior can be expressed or cleanly lowered.
- Prefer `format_in!(ref mut zone, ...)` for owned formatted strings.
- Add richer panic/assert messages only after string lifetime and allocation
  behavior stays predictable.

Likely compiler work:

- reduce formatting macro special cases as trait dispatch matures
- owned runtime string diagnostics for invalid zone or type usage
- optional float/text runtime helpers behind Ari builtin declarations

### Phase 5: OS-Facing Libraries

- Grow `std::env` from process arguments and current-process environment
  variables into current-directory and executable-path helpers after owned
  string behavior and OS wrapper conventions are stable.
- Add thin wrappers for file, time, process, thread, synchronization, and
  syscall-adjacent APIs in small capability-oriented slices. The first target,
  time, filesystem, POSIX fork/wait process, function-pointer thread, and
  concrete atomic integer slices are implemented; portable child-process
  handles, richer thread statuses, generic synchronization, directories,
  metadata, and raw OS wrappers still need ownership policy.
- Keep OS resources explicit. File handles, process handles, and buffers should
  be visible owners or zone-backed handles.
- Prefer small modules: `std::env`, `std::fs`, `std::time`, `std::process`,
  `std::thread`, `std::sync`, and `std::os`.
- Treat modern must-have system APIs as explicit capabilities: args/env,
  current directory, file descriptors or handles, process spawn and platform
  fork, thread spawn/join, shared/atomic coordination, time, and error-code
  conversion. Avoid a raw "everything syscall" module until safe wrappers
  define ownership and lifetime rules.
- Keep target/platform facts in `std::target` and
  `docs/stdlib/platform/`. x86_64/aarch64/riscv64 target ABI, glibc/musl,
  static/dynamic/PIE/RELRO/stack-protector policy, TLS, vDSO, syscall ABI,
  errno ABI, ELF, DWARF, procfs/sysfs, cgroups, namespaces, seccomp,
  capabilities, io_uring, epoll, inotify, fanotify, eventfd, timerfd,
  signalfd, pidfd, and memfd should be tracked there before any raw `std::os`
  wrapper lands.

Likely compiler work:

- no new syntax should be needed
- runtime or C wrapper declarations may be needed for platform-specific calls
- shared-library and object-output behavior must stay on the LLVM path

### Phase 6: Library Developer Experience

- Add source-level test helpers when the language can express them.
- Build a library test runner around existing `@test` support when stable.
- Keep docs and test names readable enough that a new contributor can copy a
  nearby pattern.

## Module Backlog

| Module | Next Useful Slice | Tests To Add First | Compiler Work If Needed |
| --- | --- | --- | --- |
| `std::io` | Grow from the current `Reader`/`Writer`/`Seek`, `Stdin`, `Stdout`, `Stderr`, `Cursor`, caller-buffered `BufReader`/`BufWriter`, `read_exact`, `write_all`, and `flush` slice into `pipe`, file adapters, zone-owning buffered constructors, and drop-time writer flush. | current `std-io-byte-slice` raw stdout byte-slice output, `std-io-traits-cursor` trait/cursor/exact-read/write-all/flush checks, `std-io-stderr` stderr routing and stdout/stderr separation checks, and `std-io-buffered` caller-provided buffer fill/flush checks; future pipe EOF/close behavior, zone-owned buffered storage, drop-time flush policy, and `File` trait-adapter tests. | Current stdout/stderr/caller-buffered source slice uses runtime stream hooks. Future zone-owning buffered wrappers need std raw-buffer provenance support, while `pipe` needs runtime hooks and owned OS handle policy. |
| `std::option` | Inspect-style helpers after borrowed function-pointer ergonomics are clear. | Predicate/filter/combinator/conversion/flatten/transpose behavior plus wrong-payload negative tests. | Generic enum method specialization diagnostics. |
| `std::result` | Error conversion helpers that use `From`/`Into` after trait impl patterns mature. | `Result` projection/conversion, transpose, eager/lazy fallback, and `?` residual tests. | Residual conversion and trait-bound selection. |
| `std::mem` | Grow from current layout/pointer/value helpers, byte `copy_bytes`/`move_bytes`/`set_bytes`, and hosted `page_size` into safer copy/fill and mapping-adjacent helpers. | current `std-mem-value-helpers` scalar/aggregate replace/swap tests, `std-mem-byte-ops` byte copy/move/set plus LLVM intrinsic checks, and `std-mem-page-size` runtime page-size checks; future owner-rejection, typed copy/fill, and owned mapping tests. | Current byte helpers use `extern "ari"` runtime wrappers around LLVM intrinsics and `page_size` uses a hosted runtime hook. Future typed copy/fill and mapping APIs need layout service, ownership-aware raw memory checks, descriptor/error policy, and owned mapping cleanup. |
| `std::zone` | Scoped allocation helpers after the raw `alloc_array<T>` buffer helper. | Reset/destroy provenance, raw array allocation, and escape diagnostics. | Zone lifetime/state merge rules. |
| `std::boxed` | Clarify final unique-owner direction. | Empty-handle, drop, same-zone, and pointer-provenance tests. | Generic drop and allocation-zone wrapper tracking. |
| `std::env` | Path normalization and platform-specific policy after the argument, environment-variable, cwd, and executable-path slices. | Current `try_arg`, `program_name`, `get`, `has`, `try_get`, `set`, `remove`, `current_dir`, `try_current_dir`, `set_current_dir`, `executable_path`, `try_executable_path`; future canonicalization and platform differences. | Runtime string ownership, OS wrapper declarations, and platform error policy. |
| `std::target` | Grow from compiler-known target facts into explicit build-profile reporting only after the driver owns those flags. | current `std-target-basic` x86_64 Linux GNU target enum/predicate/syscall ABI checks plus LLVM target triple inspection, and `std-target-linux64` LLVM-only classification for x86_64/aarch64/riscv64 Linux; future build-profile tests. | Current hooks are compiler-owned constants emitted by LLVM codegen. Future static/dynamic/PIE/RELRO/stack-protector facts need driver options and metadata. |
| `std::process` | Grow from the current `id`/`uid`/`gid`/`exit`/`abort`/POSIX fork seed into child process handles. | current `id`, `uid`, `gid`, explicit exit status, abort hook lowering, source status/root predicates, `std-process-fork-wait` POSIX child branch and wait-status decode; future spawn/exec/kill result handling, richer status values, optional daemon helpers, and platform guards. | Current id/uid/gid/exit/abort/fork/wait use runtime hooks; portable spawn/exec/wait/kill needs runtime wrappers for POSIX/Windows split, errno/error mapping, and handle ownership. |
| `std::time` | Grow from monotonic/wall-clock reads and sleep into timers and interruption-aware sleep. | current `std-time-basic` duration constructor, elapsed-time, wall-clock, and sleep-hook checks; future sleep-interruption and timer-handle tests. | Current monotonic/unix/sleep hooks use LLVM runtime calls; future timers may need handle ownership and platform-specific wrappers. |
| `std::fs` | Grow from byte-oriented files into owned resource handles, metadata, permission mutation, directory iteration, links, temporary files, owned path values, and optional locking. | current `std-fs-basic` existence/remove, mode-string open/read/write/close, byte-slice write, and `Option[File]` checks; current `std-fs-append` append mode, preservation, and failed append checks; current `std-fs-open-modes` `"r"`/`"w"`/`"a"`/`"rw"`/`"r+"`/`"w+"`/`"a+"`/empty/invalid mode checks; current `std-fs-read-write` whole-file `write`/`append`, read-to-byte-string, missing-file empty read, and truncating rewrite checks; current `std-fs-create-truncate-copy` `create`/`try_create`, natural `read`, `truncate`, source streaming `copy`, and missing-source copy failure; current `std-fs-rename-dir` runtime-backed `rename`, single-directory `create_dir`/`remove_dir`, duplicate-create failure, and missing-source rename failure; current `std-fs-links` runtime-backed `hard_link`/`symbolic_link`, read-through checks, missing-source failure, and duplicate-link failure; current `std-fs-permissions` access-style read/write/execute checks, `Permissions` methods, and missing-path behavior; future invalid close, metadata, permission mutation, read directory, recursive directory helpers, canonicalize, temp-file, owned path, richer link metadata/platform symlink policy, locking, and options-builder tests. | Current file hooks use LLVM runtime calls to `access`, `unlink`, `rename`, `link`, `symlink`, `mkdir`, `rmdir`, `open`, `read`, `write`, and `close`; future metadata/directory-iteration/canonicalize/temp/locking work needs OS-resource ownership/drop policy and platform-specific wrappers. |
| `std::path` | Grow from POSIX-style lexical byte helpers into owned and platform-aware path values. | current `std-path-basic` separator policy, absolute/relative predicates, trailing-separator trim, file name, parent, stem, extension, join, and normalization checks; current `std-path-components` borrowed iterator checks; future Windows drive/UNC path tests, owned path-buffer tests, richer component-kind tests, and canonicalization integration tests. | Current helpers are pure source Ari. Future owned `PathBuf` may need stronger zone-backed enum/aggregate provenance and canonicalization needs runtime filesystem wrappers. |
| `std::net` | Grow from source address values into runtime-backed sockets. | current `std-net-addresses` IPv4/IPv6 constructors, generic `IpAddr` predicates, socket-address construction, port replacement, loopback, and unspecified checks; future DNS lookup, TCP listener accept loops, TCP stream connect/read/write/shutdown, UDP send/receive, Unix domain sockets, socket options, nonblocking mode, timeout behavior, and error propagation tests. | Current address slice needs no compiler work. Future sockets need runtime wrappers for `getaddrinfo`, `socket`, `bind`, `listen`, `accept`, `connect`, `send`, `recv`, `setsockopt`, `fcntl`/platform nonblocking, timeout policy, shutdown, and owned socket handle/drop semantics. |
| `std::thread` | Grow from the current plain function-pointer spawn/join handle into safer ownership-transfer and result policy. | current `std-thread-basic` main/child id, spawn/join, invalid-handle, method-wrapper, root `Thread`, and yield checks; future moved capture rejection, richer status/result values, shared state diagnostics, and platform guards. | Current pthread trampoline uses a runtime packet and thread-local id; future work needs send/share trait policy, owned handle semantics, and platform-specific wrappers. |
| `std::sync` | Grow from concrete `AtomicI64` into shared ownership before locks/channels. | current `std-sync-atomic-i64` load/store/swap/fetch-add/compare-exchange, method-wrapper, root alias, and LLVM atomic lowering checks; future `Shared`/`Weak` upgrade behavior, generic atomics, and mutex poisoning or no-poison policy. | Current atomic hooks lower directly to LLVM atomic instructions; future work needs reference-counted handle lowering, memory-order policy, and thread-safety trait checks. |
| `std::collections` | Add tree deletion and trait-driven constructors after the current queue/list/heap slice. | current set insertion/duplicate/replace/access/optional access/reserve/removal/iteration/copy/after-reset/same-zone tests; hash collision/tombstone and live-bucket iterator tests; tree rotation/replacement and sorted iterator tests; deque circular growth tests; ring-buffer full/overwrite tests; linked-list node reuse tests; binary-heap and priority-queue pop-order tests; future red-black deletion tests. | Current collection handles have zone provenance recognition; next compiler work is trait-driven `Hash`/`Eq`/`Ord` dispatch, richer comparator policies, and iterator lowering beyond the current cursor slices. |
| `std::algo` | Grow from the first source slice algorithms into faster, move-aware ordering and slice transformation helpers. | current `std-algo-slice-helpers` sorting/stable sorting, comparator sorting, binary search, min/max/clamp, reverse/rotate, partition, fill, copy, dedup, and swap checks; future large-slice sort stress tests and ownership-valued algorithm rejection/move policy tests. | Current algorithms need no compiler work. Faster generic algorithms may need move-aware temporary storage, `Copy`/ownership constraints, and trait-driven `Ord` dispatch. |
| `std::hash` | Grow the first deterministic source hasher into collection defaults and aggregate hashing policy. | current `std-hash-basic` hasher construction/reset/finalization, byte-slice hashing, primitive write helpers, generic `Hash[T]` dispatch for primitive values, and `collections::hash_i64` compatibility checks; future aggregate impls, trait-driven hash collection constructors, and collision-seed policy tests. | Current hashing is pure source Ari and needs no compiler work. Trait-driven collection constructors may need richer generic inference and default function-value selection. |
| `std::string` | Add signed/checked parsers only after text and numeric policies are documented. | Search, growth, append, copy, ASCII case comparison/search, ASCII trim/parse, prefix parse, owned trim copy, and after-reset tests. | Formatting/string runtime hooks. |
| `std::ascii` | Add signed parsers only after numeric sign and overflow policy is documented. | Byte classification behavior, case-insensitive comparison/search, slice trimming/parsing, prefix parser consumed-length behavior, source symbol checks, and future parser edge cases. | None for current whole-slice and prefix helpers; signed/checked parsers may need overflow diagnostics. |
| `std::parse` | Grow from the current whole-input integer/bool/decimal-float slice into richer parse errors and overflow policy. | current `std-parse-basic` signed integer, bool, float validation/conversion, fallback, and invalid whole-input cases; future overflow, exponent edge, and locale-rejection tests. | Current helpers are pure source Ari. Future `Option[f64]` or `Result[f64,E]` needs float enum payload lowering. |
| `std::encoding` | Grow from validation plus hex/base64 into variants and fallible owned decoders. | current `std-encoding-text` ASCII/UTF-8/UTF-16 validation/counting and `std-encoding-codec` hex/base64 length/encode/decode/invalid-input guard tests; future URL-safe base64, MIME line wrapping, richer decode errors, and optional compression tests. | Current helpers are pure source Ari over `Slice` and `String`. Future `Option[String]`/`Result[String,E]` decoders need zone-backed enum payload support. |
| `std::vec` | Iterator collection support and root/source Vec unification plan after safe accessors. | Method, `try_*` access, iterator, borrow, owner-drop, same-zone, and `std::vec::collect` tests. | Iterator lowering, generic aggregate monomorphization, and explicit-zone provenance. |
| `std::iter` | Lazy adapter and eager consumer layer over the canonical `std::Iterator[T]` protocol. | `std-iter-adapters` covers `map`, `filter`, `take`, `skip`, `enumerate`, `zip`, `fold`, `reduce`, and `collect` over `Vec` cursors. | Function-pointer callback typing, declaring-module field type resolution, and general iterator protocol lowering. |
| `std::fmt` | Source trait impls for common values. | `format_in!`, `Display`, unsupported-type diagnostics. | Macro-to-trait lowering cleanup. |
| `std::cmp` | Derived comparison impl coverage for more aggregate shapes. | Generic helper, inclusive range predicate, and derive interaction tests. | Trait-bound static dispatch and derive expansion. |
| `std::convert` | Concrete `From`/`Into` impl patterns and fallible conversion policy. | Identity/from/into behavior, explicit associated calls, and residual conversions. | Trait coherence and inference diagnostics. |
| `std::math` | Grow natural helper names from i64 signatures into documented numeric policy slices. | Sign predicate behavior, integer helper behavior, signed division rounding, overflow-policy diagnostics, and future checked/wrapping helpers. | Overflow intrinsics or diagnostics only after the source policy is designed. |
| `std::bits` | Grow natural helper names from u64 signatures into generic integer helpers. | Mask behavior, rotate count handling, power-of-two rounding, low-mask widths, alignment preconditions, zero/one-run scan edge cases, and future overflow-policy diagnostics. | Optional bit-scan intrinsics only after the source policy is stable. |

## API Landing Checklist

Before landing a public library API:

- Add or update source in `lib/std.arih` or `lib/std/<module>.arih`.
- Add focused positive tests in `tests/cases/standard-library/ok/<feature>/`.
- Add negative diagnostics in `tests/cases/standard-library/errors/<feature>/`
  for misuse.
- Add IR or executable checks in `tests/Makefile` when behavior reaches the
  backend.
- Update `tests/std_api_manifest.txt`.
- Update [Standard Library](../language/standard-library.md) or a focused
  language page.
- Update [Feature Test Matrix](test-matrix.md) when compiler semantics,
  backend lowering, ownership, borrowing, or ABI behavior changes.
- Run `make check-std-api` and the narrowest affected check target; prefer
  `make check-sanitize` for parser, sema, ownership, or codegen changes.
