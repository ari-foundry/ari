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
- unused source `std` function bodies stay declaration-only during semantic
  checking and are lowered to IR only when a user function, generated helper, or
  another reachable std function actually references them

## Current Baseline

The current `std` package already provides:

- prelude ADTs: `Option`, `Result`, `Slice`, `Range`, `RangeInclusive`
- source `Option`/`Result` predicates, borrowed inspection, eager/lazy
  combinators, conversions, nested option filtering, flattening,
  bidirectional option-result transposition, and lazy fallback helpers,
  including consuming payload predicate helpers
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
  `env::try_arg`, `env::try_arg_os`, `env::program_name`,
  `env::program_name_os`, `env::try_get`, `env::try_get_os`, and
  current-process environment/path hooks `get`/`get_os`/`has`/`set`/`remove`/
  `current_dir`/`current_dir_os`/`current_dir_path`/`set_current_dir`/
  `executable_path`/`executable_path_os`
- the first `std::process` current-process helpers: `id`, `uid`, `gid`, `exit`,
  `abort`, and source status/root helpers, plus the first POSIX `fork`/`wait`
  child-process slice
- the first `std::thread` helpers: `Thread`, function-pointer `spawn`, `join`,
  `yield_now`, `sleep`, runtime thread ids, available parallelism, and source
  predicates/method wrappers
- the first `std::sync` helpers: concrete `AtomicI64` with sequentially
  consistent `load`, `store`, `swap`, `fetch_add`, and `compare_exchange`
  hooks plus method wrappers, and source `Mutex`/`RwLock`/`Once` helpers built
  on the atomic primitive
- the first `std::time` helpers: monotonic nanosecond reads, wall-clock Unix
  nanosecond reads, sleep, `Duration`, `Instant`, `SystemTime`, `Deadline`,
  `UtcDateTime`, strict and fallible duration and Unix timestamp constructors,
  strict and fallible month-length helpers, and source
  elapsed-time/timeout/UTC calendar helpers
- the first `std::random` helpers: OS entropy through the hosted Linux
  `getrandom` path with `/dev/urandom` fallback, deterministic
  non-cryptographic `Prng`, deterministic booleans, unbiased bounded
  integers, unit floats, direct OS byte filling, PRNG byte filling, and generic
  slice shuffle
- the first `std::fs` helpers: file existence/removal, access-permission
  checks, read/write open hooks, byte reads/writes, close, source
  `File`/`Permissions` methods, `Option[File]` open helpers,
  `create`/`try_create`, whole-file `read`/`write`/`try_write`/`append`/`try_append`,
  `truncate`, streaming `copy`/`try_copy`, `rename`, single-directory
  `create_dir`/`remove_dir`, `read_to_string`, and `Option[String]`
  `try_read`/`try_read_to_string`
- the first `std::os` helpers: a non-owning `Fd` descriptor view, owning
  `OwnedFd`, standard descriptor constructors, invalid-descriptor checks,
  descriptor predicates, identity comparison, explicit raw-descriptor close,
  fallible descriptor duplication through `try_clone`, close-on-exec flag
  accessors, nonblocking flag accessors, an owned `Pipe` read/write descriptor
  pair, and
  `std::fs::File.descriptor()`
- the first `std::path` helpers: POSIX-style separator, absolute/relative,
  borrowed `PathBytes`, borrowed component, join, and lightweight lexical
  normalization helpers
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
  comparators, sorted iterators, boundary entries, lower/upper-bound lookup,
  fallback `get_or` map lookup, and set representative `get`/`try_get`;
  collection families support target-zone copy where they own storage,
  and all use explicit-zone storage, tracked-local zone inference for common
  growable mutation calls, and provenance checks
- `Slice[T]` and `std::vec::Vec[T]` metadata, asserting element access,
  `Option`-returning element access helpers, and borrowed
  range/split/subsequence/compare/chunk/window helpers plus receiver-form
  algorithm wrappers that return views, scalar answers, or mutate existing
  storage without allocating
- `std::string::String` empty-safe byte access, byte search, byte-slice search,
  split/chunk/window borrowed views, allocator-backed join, comparison, typed
  borrowed `Utf8`/`OsStr` text-boundary views, shared `std::c::CStr`
  construction, direct literal coercion into borrowed boundary views, ASCII
  case-insensitive comparison/search, UTF-8 validation/scalar helpers,
  borrowed and owned ASCII trim helpers, and whole/prefix ASCII parsing helpers
- range/iterator traits and the `std::vec::Iter` implementation
- comparison, formatting, and conversion trait surfaces, plus source
  comparison value helpers: `min`, `max`, `clamp`, `is_between`, and
  comparator-based `*_by` forms for call-site-specific ordering; the
  first `std::fmt` source helper slice covers `FormatSpec`, unsigned
  integer binary/octal/decimal/hex formatting, width, integer precision,
  fallible width/precision builders for runtime input, left/right/center
  alignment, debug text quoting, explicit-zone string helpers, and
  `std::io::Writer` helpers
- `std::convert` source helpers: `identity`, `from`, `into`, `try_from`, and
  `try_into`
- `std::math` integer helpers implemented in Ari source with natural names:
  `abs`, `sign`, `is_positive`, `is_negative`, `is_zero`, `is_even`,
  `is_odd`, checked add/sub/mul/div/rem/neg/abs, saturating
  add/sub/mul/div/neg/abs, wrapping/overflowing add/sub/mul, `pow`,
  `div_floor`, `div_ceil`, `mod_floor`, `gcd`, and `lcm`
- `std::ascii` byte classification, printable/control predicates, case
  conversion, borrowed-slice case-insensitive comparison/search, trimming,
  whole-slice unsigned/signed digit parsing, and prefix digit parsing helpers
- `std::parse` whole-input ASCII-trimmed integer, bool, and decimal-float
  parsers with validation/fallback helpers
- `std::encoding` ASCII/UTF-8/UTF-16 validation and counting plus lowercase
  hex and standard base64 encode/decode/fallible-decode helpers
- `std::bits` `u64` mask, rotation, power-of-two, low-mask, alignment,
  checked/wrapping alignment, byte-swap, population-count, and zero/one-run
  bit-scan helpers
- `std::test` executable unit-test `Report` helpers, generic equality checks,
  scratch-zone construction, and finish/require status helpers
- `std::log` source stderr logging levels, threshold predicates, byte-slice
  and string-message logging, and convenience level helpers
- `std::error` source recoverable error categories, compact error values,
  POSIX errno mapping, fallible boundary constructors, predicate helpers, root
  `Error`/`ErrorKind` aliases, and a raw scalar bridge for `Result[T, i64]`
  while mixed `Result[T, Error]` payload storage remains compiler roadmap work
- `std::c` source C ABI boundary helpers, borrowed `CStr`, zone-backed
  NUL-terminated `CString`, POSIX `errno`/`Error` bridging, dynamic
  `Library`/`Symbol` handles over `dlopen`/`dlsym`/`dlclose`, typed function
  pointer extraction from dynamic symbols, and root type aliases for C
  boundary values

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
- Keep `Slice[T]` borrowed sequence APIs allocation-free: `slice`, `split_at`,
  `find`, `contains_slice`, `compare`, `chunks`, `windows`, delimiter `split`,
  and receiver-form algorithm wrappers should produce views, scalar answers, or
  in-place changes, while owning copies stay explicit through
  `copy_to(ref mut zone)`.
- Grow collection families in this order: slice helpers, vector methods,
  linear `Set[T]`, hash tables, red-black trees, iterators for hash/tree
  containers, deque/ring-buffer/linked-list/heap families, red-black deletion,
  then trait-driven constructors. The iterator slice is now in place for live
  hash buckets, sorted tree traversal, deques, ring buffers, and linked lists.
- Keep `Set[T]` linear and insertion-order. Hash-backed behavior belongs in
  `HashMap`/`HashSet`, so call sites stay honest and natural.
- Keep `HashMap`/`HashSet` and `TreeMap`/`TreeSet` on explicit
  hash/comparator constructors until `Hash`, `Eq`, and `Ord` trait dispatch can
  drive generic containers directly. Public names should remain natural
  (`insert`, `get`, `get_or`, `contains`, `contains_key`, `remove`) rather than
  type-suffixed.

Likely compiler work:

- generic aggregate monomorphization for richer collection layouts
- iterator protocol diagnostics beyond direct `range` and current `Vec` support
- allocation-zone diagnostics for nested and generic wrapper types
- trait-driven `Hash`/`Eq`/`Ord` constructor selection

### Phase 4: Text, Formatting, And Diagnostics

- Keep `std::string::String` as a byte string, but let it reuse
  `std::encoding` for explicit UTF-8 scalar validation and append helpers.
- Keep the boundary types distinct in source APIs: C strings are
  borrowed `std::c::CStr` or zone-backed `std::c::CString`, owned
  `std::string::String` is bytes, `Utf8` means validated UTF-8 bytes,
  `OsStr` means raw OS string bytes, and `PathBytes` means bytes under path
  policy.
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
  variables into current-directory, executable-path, OS-string, and path-byte
  helpers after owned string behavior and OS wrapper conventions are stable.
- Add thin wrappers for file, time, process, thread, synchronization, and
  syscall-adjacent APIs in small capability-oriented slices. The first target,
  time, filesystem, POSIX fork/wait process, function-pointer thread, and
  concrete atomic integer, source mutex, and source once slices are
  implemented; the first descriptor view and explicit owned close wrapper are
  implemented in `std::os`; portable child-process handles, richer thread
  statuses, generic synchronization, blocking locks, directories, metadata,
  duplicate-with-flags policy, and raw OS wrappers still need ownership policy.
- Keep OS resources explicit. File handles, process handles, and buffers should
  be visible owners or zone-backed handles.
- Prefer small modules: `std::c`, `std::env`, `std::fs`, `std::time`,
  `std::process`, `std::thread`, `std::sync`, and `std::os`.
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

- Grow the current source-level `std::test::Report` helpers into a real test
  runner when discovery, per-test isolation, and diagnostics are stable.
- Build a library test runner around existing `@test` support when stable.
- Grow the current `std::log` line helpers toward structured logging after
  formatting and capture policy are testable.
- Build owned source text, filenames, and diagnostic labels in a
  compiler/tooling package before adding stack/backtrace, optional benchmark,
  and optional fuzzing APIs.
- Keep docs and test names readable enough that a new contributor can copy a
  nearby pattern.

## Module Backlog

| Module | Next Useful Slice | Tests To Add First | Compiler Work If Needed |
| --- | --- | --- | --- |
| `std::io` | Grow from the current `Reader`/`Writer`/`Seek`, `Stdin`, `Stdout`, `Stderr`, `PipeReader`/`PipeWriter`, `Cursor`, caller-buffered `BufReader`/`BufWriter`, `read_exact`, `write_all`, and `flush` slice into file adapters, zone-owning buffered constructors, and drop-time writer flush. | current `std-io-byte-slice` raw stdout byte-slice output, `std-io-traits-cursor` trait/cursor/exact-read/write-all/flush checks, `std-io-stderr` stderr routing and stdout/stderr separation checks, current `std-io-pipe` `std::os::Pipe` ownership splitting, trait-based pipe writes/reads, EOF after writer close, and raw fd read/write hook checks, and current `std-io-buffered` caller-provided buffer fill/flush checks; future zone-owned buffered storage, drop-time flush policy, and `File` trait-adapter tests. | Current stdout/stderr/caller-buffered/pipe source slice uses runtime stream and descriptor hooks. Future zone-owning buffered wrappers need std raw-buffer provenance support; file adapters need owned file-resource policy. |
| `std::option` | Wrong-payload diagnostics and residual conversion polish after the borrowed inspect helper slice. | Predicate/filter/inspect/combinator/conversion/flatten/transpose behavior plus wrong-payload negative tests. | Generic enum method specialization diagnostics. |
| `std::result` | Error conversion helpers that use `From`/`Into` after trait impl patterns mature. | `Result` projection/conversion, transpose, eager/lazy fallback, and `?` residual tests. | Residual conversion and trait-bound selection. |
| `std::mem` | Grow from current layout/pointer/value helpers, byte `copy_bytes`/`move_bytes`/`set_bytes`, and hosted `page_size` into safer copy/fill and mapping-adjacent helpers. | current `std-mem-value-helpers` scalar/aggregate replace/swap tests, `std-mem-byte-ops` byte copy/move/set plus LLVM intrinsic checks, and `std-mem-page-size` runtime page-size checks; future owner-rejection, typed copy/fill, and owned mapping tests. | Current byte helpers use `extern "ari"` runtime wrappers around LLVM intrinsics and `page_size` uses a hosted runtime hook. Future typed copy/fill and mapping APIs need layout service, ownership-aware raw memory checks, descriptor/error policy, and owned mapping cleanup. |
| `std::zone` | Scoped allocation helpers after the raw `alloc_array<T>` buffer helper. | Reset/destroy provenance, raw array allocation, and escape diagnostics. | Zone lifetime/state merge rules. |
| `std::boxed` | Clarify final unique-owner direction. | Empty-handle, drop, same-zone, and pointer-provenance tests. | Generic drop and allocation-zone wrapper tracking. |
| `std::env` | Path normalization and platform-specific policy after the argument, environment-variable, cwd, executable-path, OS-string, and path-byte slices. | Current `try_arg`, `try_arg_os`, `program_name`, `program_name_os`, `get`, `get_os`, `has`, `try_get`, `try_get_os`, `set`, `remove`, `current_dir`, `current_dir_os`, `current_dir_path`, `try_current_dir`, `try_current_dir_os`, `try_current_dir_path`, `set_current_dir`, `executable_path`, `executable_path_os`, `try_executable_path`, `try_executable_path_os`; future canonicalization and platform differences. | Runtime string ownership, OS wrapper declarations, and platform error policy. |
| `std::target` | Grow from compiler-known target facts into explicit build-profile reporting only after the driver owns those flags. | current `std-target-basic` x86_64 Linux GNU target enum/predicate/syscall ABI checks plus LLVM target triple inspection, and `std-target-linux64` LLVM-only classification for x86_64/aarch64/riscv64 Linux; future build-profile tests. | Current hooks are compiler-owned constants emitted by LLVM codegen. Future static/dynamic/PIE/RELRO/stack-protector facts need driver options and metadata. |
| `std::c` | Grow from C string, errno, and dynamic loading helpers into richer typed dynamic symbols and explicit owned-buffer handoff policy. | current `std-c-interop` borrowed `CStr`, zone-backed `CString`, libc alias calls, POSIX errno mapping, root aliases, and `dlopen`/`dlsym`/`dlclose` checks; current `std-c-dynamic-function` function-pointer extraction and indirect dynamic calls; future null-safe/fallible constructors, data-symbol wrappers, return-type generic inference, non-glibc errno symbol paths, and negative tests for unsafe zone-backed C handoff. | Current helpers are source Ari over C ABI aliases and hosted loader symbols, plus compiler provenance recognition for `CString` storage. Function symbols use explicit raw-pointer-to-function-pointer casts in the compiler; arbitrary owned buffer handoff still needs explicit FFI escape semantics. |
| `std::process` | Grow from the current `id`/`uid`/`gid`/`exit`/`abort`/POSIX fork seed into child process handles. | current `id`, `uid`, `gid`, explicit exit status, abort hook lowering, source status/root predicates, `std-process-fork-wait` POSIX child branch and wait-status decode; future spawn/exec/kill result handling, richer status values, optional daemon helpers, and platform guards. | Current id/uid/gid/exit/abort/fork/wait use runtime hooks; portable spawn/exec/wait/kill needs runtime wrappers for POSIX/Windows split, errno/error mapping, and handle ownership. |
| `std::time` | Grow from monotonic/wall-clock reads, sleep, monotonic deadlines, and UTC calendar values into timers and interruption-aware sleep. | current `std-time-basic` duration constructor, elapsed-time, wall-clock, and sleep-hook checks; current `std-time-try-duration` fallible duration constructor checks; current `std-time-try-unix` fallible Unix timestamp checks; current `std-time-calendar-validation` strict/fallible month-length checks; current `std-time-timeout` monotonic deadline, timeout, remaining-time, and sleep-until checks; current `std-time-utc-calendar` UTC conversion, leap-year, and month-length checks; future sleep-interruption, timer-handle, pre-1970 calendar, and limited timezone-policy tests. | Current monotonic/unix/sleep hooks use LLVM runtime calls, while `Deadline` and `UtcDateTime` are pure source over `Instant`/`SystemTime`. Future timers may need handle ownership and platform-specific wrappers. |
| `std::fs` | Grow from byte-oriented files into owned resource handles, metadata, permission mutation, directory reads, links, temporary files, owned path values, and optional locking. | current `std-fs-basic` existence/remove, mode-string open/read/write/close, byte-slice write, and `Option[File]` checks; current `std-fs-append` append mode, preservation, and failed append checks; current `std-fs-open-modes` `"r"`/`"w"`/`"a"`/`"rw"`/`"r+"`/`"w+"`/`"a+"`/empty/invalid mode checks; current `std-fs-read-write` whole-file `write`/`try_write`/`append`/`try_append`, read-to-byte-string, missing-file empty read, and truncating rewrite checks; current `std-fs-try-read` `Option[String]` read behavior that distinguishes missing files from empty files; current `std-fs-create-truncate-copy` `create`/`try_create`, natural `read`, `truncate`, source streaming `copy`/`try_copy`, missing-source copy failure, and byte-count checks; current `std-fs-rename-dir` runtime-backed `rename`, single-directory `create_dir`/`remove_dir`, duplicate-create failure, and missing-source rename failure; current `std-fs-read-dir` runtime-backed `Dir` open/next/close, one-shot `try_read_dir`/`read_dir`, lightweight `DirEntry` name/path reads, dot-entry skip, missing-directory failure, invalid-handle `None`, and cleanup; current `std-fs-links` runtime-backed `hard_link`/`symbolic_link`, read-through checks, missing-source failure, and duplicate-link failure; current `std-fs-permissions` access-style read/write/execute checks, `Permissions` methods, and missing-path behavior; current `std-fs-mode` stat-backed permission mode lookup, chmod-backed mutation, structured `Permissions` constructors/conversion, invalid-mode rejection, and missing-path failure; current `std-fs-metadata` `Option[Metadata]`, byte length, `FileKind`, regular-file and directory predicates, and missing-path behavior; current `std-fs-canonicalize` `Option[String]` existing-path resolution, absolute paths, filename preservation, and missing-path behavior; current `std-os-fd` covers the non-owning `File.descriptor()` bridge; future invalid close, metadata-bearing `DirEntry` values, recursive directory helpers, temp-file, owned path, richer owner/group/ACL/timestamps/no-follow symlink metadata, richer link metadata/platform symlink policy, locking, and options-builder tests. | Current file hooks use LLVM runtime calls to `access`, `stat`, `chmod`, `realpath`, `unlink`, `rename`, `link`, `symlink`, `mkdir`, `rmdir`, `opendir`, `readdir`, `closedir`, `open`, `read`, `write`, and `close`; future richer directory/temp/locking work needs OS-resource ownership/drop policy and platform-specific wrappers. |
| `std::os` | Grow from descriptor views and explicit close into owned OS resource wrappers. | current `std-os-fd` covers `Fd`, standard descriptors, invalid descriptor, descriptor predicates, identity comparison, and `File.descriptor`; current `std-os-owned-fd` covers `OwnedFd`, raw ownership handoff, borrowed `as_fd`, disarming `take`, explicit `close`, double-close prevention, and `ari_builtin_os_close` lowering; current `std-os-owned-fd-duplicate` covers `try_clone`, invalid-owner failure, independent close, and `ari_builtin_os_dup` lowering; current `std-os-owned-fd-flags` covers `close_on_exec`, `set_close_on_exec`, invalid-owner failure, and `fcntl`-backed runtime lowering; current `std-os-owned-fd-nonblocking` covers `is_nonblocking`, `set_nonblocking`, invalid-owner failure, and `O_NONBLOCK` flag toggling; current `std-os-pipe` covers `Pipe`, `pipe`, read/write end inspection, take-apart ownership, explicit close, and `ari_builtin_os_pipe` lowering; future duplicate-with-flags policy, broader `fcntl`, `ioctl`, `poll`, `select`, Linux `epoll`, `eventfd`, `timerfd`, `signalfd`, optional `pidfd`, optional `memfd`, signals, and memory mappings. | Current `Fd`/`OwnedFd`/`Pipe` are source Ari over small runtime close/dup/fcntl/pipe hooks. Future wrappers need richer errno/error mapping, ownership/drop policy, duplicate flag APIs, and target guards. |
| `std::path` | Grow from POSIX-style lexical byte helpers and `PathBytes` views into owned and platform-aware path values. | current `std-path-basic` separator policy, absolute/relative predicates, trailing-separator trim, file name, parent, stem, extension, join, and normalization checks; current `std-path-predicates` allocation-free final-component predicate checks; current `std-path-components` borrowed iterator checks; current `std-path-bytes` typed path-byte view and OS-string conversion checks; filesystem-backed existing-path canonicalization is covered by `std-fs-canonicalize`; future Windows drive/UNC path tests, owned path-buffer tests, richer component-kind tests, and `PathBuf` integration tests. | Current helpers are pure source Ari. Future owned `PathBuf` may need stronger zone-backed enum/aggregate provenance and should wrap the existing `std::fs` canonicalization policy. |
| `std::net` | Grow from source address values into runtime-backed sockets. | current `std-net-addresses` IPv4/IPv6 constructors, generic `IpAddr` predicates, socket-address construction, port replacement, loopback, and unspecified checks; current `std-net-address-validation` strict/fallible octet and segment accessor checks; future DNS lookup, TCP listener accept loops, TCP stream connect/read/write/shutdown, UDP send/receive, Unix domain sockets, socket options, nonblocking mode, timeout behavior, and error propagation tests. | Current address slice needs no compiler work. Future sockets need runtime wrappers for `getaddrinfo`, `socket`, `bind`, `listen`, `accept`, `connect`, `send`, `recv`, `setsockopt`, `fcntl`/platform nonblocking, timeout policy, shutdown, and owned socket handle/drop semantics. |
| `std::thread` | Grow from the current plain function-pointer spawn/join handle into safer ownership-transfer and result policy. | current `std-thread-basic` main/child id, spawn/join, invalid-handle, method-wrapper, root `Thread`, and yield checks; current `std-thread-runtime-helpers` available-parallelism and sleep wrapper checks; future moved capture rejection, user-facing thread-local storage, stack-size options, richer status/result values, shared state diagnostics, and platform guards. | Current pthread trampoline uses a runtime packet and thread-local id, and hosted available parallelism uses `sysconf`; future work needs send/share trait policy, owned handle semantics, TLS destructor policy, stack attribute builders, and platform-specific wrappers. |
| `std::sync` | Grow from concrete `AtomicI64`, primitive `Mutex`/`RwLock`, and `Once` into shared ownership, value-protecting locks, and channels. | current `std-sync-atomic-i64` load/store/swap/fetch-add/compare-exchange, method-wrapper, root alias, and LLVM atomic lowering checks; current `std-sync-mutex-once` primitive mutex lock/unlock and once state checks; current `std-sync-rwlock` explicit read/write state transitions, reader counts, root alias, and method wrappers; future `Shared`/`Weak` upgrade behavior, generic atomics, memory-order parameters, `Mutex[T]`/`RwLock[T]` guard policy, `Condvar`, `OnceLock`, `LazyLock`, `Barrier`, optional `Semaphore`, MPSC channels, and futex-backed blocking implementation tests. | Current atomic hooks lower directly to LLVM atomic instructions; source `Mutex`/`RwLock`/`Once` reuse those hooks. Future work needs reference-counted handle lowering, memory-order policy, thread-safety trait checks, guard lifetime modeling, blocking wait/wake runtime hooks, and Linux futex internals hidden behind portable APIs. |
| `std::collections` | Add tree deletion and trait-driven constructors after the current queue/list/heap slice. | current set insertion/duplicate/replace/access/optional access/representative lookup/reserve/removal/iteration/copy/after-reset/same-zone tests; hash collision/tombstone and live-bucket iterator tests; tree rotation/replacement and sorted iterator tests; deque circular growth tests; ring-buffer full/overwrite tests; linked-list node reuse tests; binary-heap and priority-queue pop-order tests; future red-black deletion tests. | Current collection handles have zone provenance recognition; next compiler work is trait-driven `Hash`/`Eq`/`Ord` dispatch, richer comparator policies, and iterator lowering beyond the current cursor slices. |
| `std::algo` | Grow from the first source slice algorithms into faster, move-aware ordering and slice transformation helpers. | current `std-algo-slice-helpers` sorting/stable sorting, comparator sorting, binary search, lower/upper bounds, min/max/clamp, reverse/rotate, partition, fill, copy, dedup, and swap checks; current `std-algo-by-helpers` comparator sortedness/search/bounds/min/max/clamp and receiver-wrapper checks over values without `Ord`; current `prelude-slice-sequence` receiver wrappers over the same policies; future large-slice sort stress tests and ownership-valued algorithm rejection/move policy tests. | Current algorithms need no compiler work. Faster generic algorithms may need move-aware temporary storage, `Copy`/ownership constraints, and trait-driven `Ord` dispatch. |
| `std::hash` | Grow the first deterministic source hasher into collection defaults and aggregate hashing policy. | current `std-hash-basic` hasher construction/reset/finalization, byte-slice hashing, fixed-width integer and bool write helpers, generic `Hash[T]` dispatch for primitive values and `Slice[u8]`, ordered `pair` hashing, precomputed `combine` hashing, width-distinct byte-feed checks, and `collections::hash_i64` compatibility checks; future aggregate impls, trait-driven hash collection constructors, and collision-seed policy tests. | Current hashing is pure source Ari and needs no compiler work. Trait-driven collection constructors may need richer generic inference and default function-value selection. |
| `std::random` | Grow from OS entropy and deterministic PRNG helpers into fallible entropy, larger distribution tests, and explicit crypto/distribution policy. | current `std-random-basic` entropy hook reachability, deterministic seed behavior, deterministic boolean checks, unbiased bounded integer checks including a wide upper bound, unit float bounds, deterministic byte filling, direct OS byte filling, and generic shuffle checks; future fallible entropy-result tests, larger byte-buffer fill tests, statistical range tests, and distribution helpers if they belong in core std. | Current `entropy` and `fill` need LLVM runtime hooks around hosted Linux `getrandom` with `/dev/urandom` fallback; the PRNG, deterministic boolean helper, unbiased bounded integer path, float, deterministic byte-fill, and shuffle helpers are source Ari. Future fallible APIs need richer `Error` result payloads. |
| `std::string` | Add checked parser diagnostics and owned text-boundary values only after text, path, and numeric policies are documented. | Search, growth, append, copy, natural literal constructors, literal-oriented append/search/comparison wrappers, direct literal coercion into `Slice[u8]`/local byte storage/typed boundary views, UTF-8 scalar validation/access/append, typed `Utf8`/`OsStr` views, shared `std::c::CStr` construction, ASCII case comparison/search, ASCII trim/parse, prefix parse, signed decimal parse wrappers, owned trim copy, and after-reset tests. | Formatting/string runtime hooks plus source UTF-8 helpers over `std::encoding`. Future owned `Utf8String` and `OsString` need fallible owned conversions and clearer OS-string policy; C ABI ownership stays in `std::c::CString`. |
| `std::ascii` | Add overflow-checked parser policy and broader byte-window helpers. | Byte classification behavior, case-insensitive comparison/search, slice trimming/parsing, unsigned and signed decimal parser behavior, prefix parser consumed-length behavior, source symbol checks, and future parser edge cases. | None for current whole-slice and prefix helpers; overflow-checked parsers may need diagnostics or wider intermediate arithmetic. |
| `std::parse` | Grow from the current whole-input integer/bool/decimal-float slice into richer parse errors and overflow policy. | current `std-parse-basic` signed decimal/radix/hex/binary/octal integer, bool, float validation/conversion, fallback, and invalid whole-input cases; future overflow, exponent edge, and locale-rejection tests. | Current helpers are pure source Ari. Future `Option[f64]` or `Result[f64,E]` needs float enum payload lowering. |
| `std::encoding` | Grow from validation plus hex/base64 into variants and richer fallible decoders. | current `std-encoding-text` ASCII/UTF-8/UTF-16 validation/counting, `std-encoding-utf8-codepoints` UTF-8 scalar decode/asserting-encode/fallible-encode behavior, and `std-encoding-codec` hex/base64 length/encode/decode/fallible-decode/invalid-input guard tests; future URL-safe base64, MIME line wrapping, richer decode errors, normalization/transcoding, and optional compression tests. | Current helpers are pure source Ari over `Slice`, `String`, and `Option[String]`. Future `Result[String,E]` decoders and richer text values need stronger error payload policy. |
| `std::error` | Grow from compact shared error values into direct `Result[T, Error]`, platform-specific mappings, and richer error payloads. | current `std-error-basic` error-kind predicates, POSIX errno mapping, root aliases, string names, compact raw bridge, and Result raw-error flow; current `std-error-validation` fallible code/raw/errno constructor checks; future Windows last-error mapping, filesystem/network/process conversion tests, mixed-payload `Result[T, Error]`, and structured error fields. | Current helpers are pure source Ari with a one-word `Error`. Direct `Result[T, Error]` for scalar success values needs mixed enum payload storage; richer messages need owned strings or formatter integration. |
| `std::log` | Grow from level-prefixed stderr lines into structured logging. | current `std-log-basic` level ordering, threshold checks, byte-slice logging, string-message logging, convenience level helpers, and stderr output checks; future target/module fields, thread ids, capture policy, and backtrace integration. | Current helpers are pure source Ari over `std::io::Stderr`. Future backtraces need driver/runtime metadata and formatting policy. |
| `std::test` | Grow from report aggregation into first-class test execution and failure reports. | current `std-test-report` report counts, generic equality checks, method wrappers, scratch-zone creation, and finish status; future test discovery, per-test names/statuses, richer assert messages, log capture, backtrace/stack trace, optional benchmark, and optional fuzzing hooks. | Current helpers are pure source Ari over `Report` and `Zone`. Future backtraces and runner integration need driver/runtime metadata and panic reporting policy. |
| `std::vec` | Grow from the current safe access/growth/iterator and direct borrowed sequence wrappers toward root/source Vec unification. | Method, `try_*` access, direct `slice`/`split_at`/`find`/`compare`/`chunks`/`windows`/`split`, in-place `reverse`/rotation/sort/search/dedup/fill/copy/partition wrappers, iterator, borrow, owner-drop, same-zone, and `std::vec::collect` tests. | Iterator lowering, generic aggregate monomorphization, and explicit-zone provenance. |
| `std::iter` | Lazy adapter and eager consumer layer over the canonical `std::Iterator[T]` protocol. | `std-iter-adapters` covers `map`, `filter`, `take`, `skip`, `enumerate`, `zip`, `fold`, `reduce`, and `collect` over `Vec` cursors. | Function-pointer callback typing, declaring-module field type resolution, and general iterator protocol lowering. |
| `std::fmt` | Source trait impls for common values, full custom formatter objects, derived/debug formatting, direct writer streaming without temporary strings, allocator-returning `format!` once default-zone policy exists. | `format_in!`, `Display`, unsupported-type diagnostics, source `FormatSpec` helpers for unsigned radix/width/precision/alignment, fallible spec validation, and writer-backed formatting. | Macro-to-trait lowering cleanup and richer trait dispatch. |
| `std::cmp` | Derived comparison impl coverage for more aggregate shapes, richer partial-order policy later. | Three-way `Ordering`, primitive `Eq`/`PartialEq`/`Ord`/`PartialOrd` impl availability, `compare`, comparator-based `compare_by`/`then_compare_by`, ordering-chain helpers, generic value helpers, inclusive range predicates, `*_by` helper tests over custom values without `Ord`, and derive interaction tests. | Trait-bound static dispatch and derive expansion. |
| `std::convert` | Concrete conversion impl patterns and richer conversion coverage across stdlib value types. | Identity/from/into behavior, Option-returning try_from/try_into behavior, explicit associated calls, and residual conversions. | Trait coherence and inference diagnostics. |
| `std::math` | Grow natural helper names from i64 signatures into documented numeric policy slices. | Sign predicate behavior, integer helper behavior, signed division rounding, checked add/sub/mul/div/rem/neg/abs behavior, saturating add/sub/mul/div/neg/abs behavior, wrapping add/sub/mul behavior, and overflowing add/sub/mul tuple-result behavior. | Current checked/wrapping/overflowing/saturating helpers are pure source Ari; cross-width helpers likely need compiler intrinsics. |
| `std::bits` | Grow natural helper names from u64 signatures into generic integer helpers. | Mask behavior, checked/wrapping alignment policy, rotate count handling, power-of-two rounding, low-mask widths, alignment preconditions, byte-swap behavior, population-count aliasing, zero/one-run scan edge cases, and future generic-width diagnostics. | Optional bit-scan/byte-swap intrinsics only after the source policy is stable. |

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
