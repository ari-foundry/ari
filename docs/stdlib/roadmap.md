# Standard Library Roadmap

This page summarizes the library implementation plan. The detailed compiler
roadmap remains in `docs/dev/standard-library-roadmap.md`.

## Phase 1: Stabilize The Current Seed

- Keep `lib/std.arih` as the public root.
- Keep child modules under `lib/std/`.
- Keep all public declarations in `tests/std_api_manifest.txt`.
- Keep focused tests under `tests/cases/standard-library/`.
- Keep this `docs/stdlib/` folder current with every public API.

Current source families: `option`, `result`, root `Slice[T]` access,
subslice, split, subsequence search, compare, chunk/window, copy helpers,
`algo` slice sort/search/reorder helpers, `hash` deterministic
hasher/value/byte-slice helpers, `random` OS entropy plus deterministic
non-cryptographic PRNG helpers, `mem` layout,
pointer, value, byte memory, and hosted page-size helpers, `zone` raw
allocation plus source typed array allocation,
`boxed`, `string` byte access/search/split/chunk/window/join/ASCII helpers
including case search, prefix parsers, owned trim copies, typed borrowed
`Utf8`/`OsStr` views, and `std::c::CStr` convenience construction, `ascii`
byte classification, case-insensitive comparison/search, slice helpers, and
prefix parsers, `vec` source growable sequence handles with direct borrowed
range/split/subsequence/compare/chunk/window wrappers, `iter` range/trait
support plus lazy adapters and eager consumers, `fmt`, `cmp` comparison helpers, `convert`
identity/from/into helpers, `context` runtime hooks plus the source
`has_arg` helper, `env` source argument wrappers with `try_arg` and
`program_name` plus current-process environment `get`/`has`/`try_get`/`set`/
`remove`, OS-string views for arguments/environment/path-like values, and
path-state helpers `current_dir`/`try_current_dir`/`current_dir_path`/
`try_current_dir_path`/`set_current_dir`/`executable_path`/
`try_executable_path`/`try_executable_path_os`,
`test` source executable `Report` helpers, generic equality checks, and
scratch zone construction, `log` source stderr levels, threshold predicates,
byte-slice logging, string-message logging, and convenience level functions,
`error` shared recoverable error categories, compact error values, POSIX errno
mapping, fallible boundary constructors, predicate helpers, and raw scalar
Result bridging,
`input` runtime hooks plus the source `try_read_byte` EOF helper,
`target` compiler-known target triple, architecture, OS, environment/libc,
object/debug format, errno ABI, pointer width, syscall ABI, and Linux
API-family predicates,
`io` runtime hooks plus source `Reader`/`Writer`/`Seek`, `Stdin`, `Stdout`,
`Stderr`, `Cursor`, `BufReader`, `BufWriter`, `read_exact`, `write_all`,
`flush`, stderr routing, and byte-slice output, current
`process` id/uid/gid/exit/abort/status/root helpers plus the first POSIX
fork/wait slice, `thread`
function-pointer spawn/join/sleep/yield hooks plus runtime ids, available
parallelism, and source handle helpers, `sync` concrete `AtomicI64`
sequentially consistent load/store/swap/
fetch-add/compare-exchange hooks plus source `Mutex`, `RwLock`, and `Once`
helpers,
`time`
monotonic/wall-clock/sleep hooks plus source
strict and fallible `Duration` constructors, strict and fallible Unix
timestamp constructors, strict and fallible calendar helpers,
`Instant`/`SystemTime` helpers, `fs` byte-oriented file existence,
access-permission/open/read/write/append/close/remove hooks plus source
`File`/`Permissions`/`Metadata` methods, direct path-kind predicates, and
`Option[File]` open helpers,
`create`/`try_create`, whole-file
`read`/`write`/`try_write`/`append`/`try_append`, `truncate`, streaming `copy`/`try_copy`, `rename`,
single-directory `create_dir`/`remove_dir`, `read_to_string`, and
`Option[String]` `try_read`/`try_read_to_string`, and `Option[Metadata]`
`try_metadata`, permission `mode`/`try_mode`/`set_mode`/`set_permissions`,
plus existing-path `try_canonicalize`,
`path` POSIX-style lexical separator, typed `PathBytes`, component, join, and
lightweight normalization helpers,
`collections::Set[T]` as the linear insertion-order set with `try_*`
accessors, `pop`/`try_pop`, replace-or-insert updates, explicit reserve
growth, direct iterator support, open-addressed `HashMap`/`HashSet` with
tombstones and live-bucket iterators, red-black-tree `TreeMap`/`TreeSet` with
sorted iterators, growable `Deque`, fixed `RingBuffer`, zone-backed
`LinkedList`, and `BinaryHeap`/`PriorityQueue` priority containers, and the
first `parse` whole-input integer/bool/decimal-float parsers, `encoding`
ASCII/UTF-8/UTF-16 validation plus hex/base64 codecs, and `math` sign,
checked add/sub/mul/div/rem/pow, wrapping/overflowing add/sub/mul/pow, saturating
arithmetic including powers, division-rounding and
`bits` numeric helper slices, including byte-swap, population-count, and
zero/one-run bit scans.

## Essential Library Families

These are the library families Ari needs for modern application and systems
work. Each one should land in small tested slices with natural API names.

| Family | Purpose | Current Or Planned APIs |
| --- | --- | --- |
| `std::io` | Provide byte-oriented process IO contracts that other libraries can share without hiding raw hooks. | Current `Reader`, `Writer`, `Seek`, `Stdin`, `Stdout`, `Stderr`, `Cursor`, `BufReader`, `BufWriter`, `stdin`, `stdout`, `stderr`, `cursor`, `buf_reader`, `buf_writer`, `read_exact`, `write_all`, `flush`, stderr routing, and raw scalar/byte/line hooks; future `pipe`, file adapters, zone-owning buffered constructors, and drop-time flush after owned OS handles and resource policy are settled. |
| `std::env` | Read startup and environment state without exposing raw runtime hooks. | Current `arg_count`, `arg`, `arg_os`, `has_arg`, `try_arg`, `try_arg_os`, `program_name`, `program_name_os`, `get`, `get_os`, `has`, `try_get`, `try_get_os`, `set`, `remove`, `current_dir`, `current_dir_os`, `current_dir_path`, `try_current_dir`, `try_current_dir_os`, `try_current_dir_path`, `set_current_dir`, `executable_path`, `executable_path_os`, `try_executable_path`, `try_executable_path_os`; future path normalization and platform-specific expansion. |
| `std::target` | Report compiler-known target facts without requiring users to parse triples by hand. | Current `triple`, `arch`, `arch_name`, `os`, `os_name`, `env`, `env_name`, `object_format`, `debug_format`, `errno_abi`, `pointer_bits`, `long_bits`, source predicates for Linux/glibc/musl/ELF/DWARF/TLS, Linux syscall ABI classification, and Linux API-family predicates for procfs/sysfs/vDSO/epoll/inotify/eventfd/timerfd/signalfd/memfd plus optional API families; future build-profile facts for static/dynamic/PIE/RELRO/stack-protector only after the driver owns those flags. |
| `std::c` | Keep C ABI boundary helpers readable without making every program write raw loader and errno bindings. | Current `CStr`, `CString`, `Library`, `Symbol`, borrowed C string wrappers, zone-backed NUL-terminated storage, `errno`/`error`, `dlopen`/`dlsym`/`dlclose`/`dlerror` wrappers, typed dynamic function symbols, and root type aliases; future data-symbol wrappers, fallible string constructors, target-specific errno symbols, return-type generic inference, and explicit FFI escape rules for zone-backed buffers. |
| `std::process` | Represent the current process and child processes explicitly. | Current `id`, `uid`, `gid`, `exit`, `abort`, `success`, `failure`, status/root predicates, POSIX `fork`, `wait`, and child/error predicates; future portable `spawn`, `exec`, `kill`, richer status/result values, daemon helpers as optional policy, and process handles. |
| `std::fs` | Work with files and directories through explicit handles. | Current `File`, `Dir`, `DirEntry`, `FileKind`, `Metadata`, `Permissions`, `exists`, access-style `can_read`/`can_write`/`can_execute` and `permissions`, `metadata`/`try_metadata`, `try_file_type` plus `is_file`/`is_dir`/`is_symlink`/`is_other`, POSIX permission `mode`/`try_mode`/`set_mode`/`set_permissions`, `canonicalize`/`try_canonicalize`, `remove`, `rename`, `hard_link`, `symbolic_link`, single-directory `create_dir`/`remove_dir`, directory-name reads with one-shot `try_read_dir`/`read_dir`, lightweight entry reads with `try_read_dir_entries`/`read_dir_entries`, and streaming `try_open_dir`/`Dir::next`/`Dir::close`, mode-string `open`/`try_open` with `"r"`, `"w"`, `"a"`, `"rw"`, `"r+"`, `"w+"`, and `"a+"`, `create`/`try_create`, compatibility `open_*`/`try_open_*` wrappers, byte `read_byte`/`write_byte`/`write_bytes`, whole-file `read`/`try_read`/`write`/`try_write`/`append`/`try_append`, `truncate`, source streaming `copy`/`try_copy`, `read_to_string`, `try_read_to_string`, and `close`; future owned resource policy, metadata-bearing `DirEntry` values, richer owner/group/ACL/timestamps/no-follow metadata, recursive directory helpers, owned path values, temporary files, richer link metadata/platform symlink policy, optional file locking, and an options-style open builder. |
| `std::path` | Manipulate path bytes without opening the filesystem. | Current POSIX-style `PathBytes`, `bytes`, `from_os`, method-style path wrappers, `is_separator`, `is_absolute`, `is_relative`, `trim_trailing_separators`, borrowed `components`, `file_name`, `parent`, `extension`, `stem`, `join_in`, and `normalize_in`; existing-path runtime canonicalization lives in `std::fs`; future platform-specific paths, owned `Path`/`PathBuf`, and richer component kinds. |
| `std::net` | Represent network addresses now and sockets later through explicit handles. | Current source-only `Ipv4Addr`, `Ipv6Addr`, `IpAddr`, `SocketAddr`, constructors, strict and fallible indexed accessors, family predicates, loopback/unspecified checks, and port helpers; future DNS lookup, `TcpListener`, `TcpStream`, `UdpSocket`, Unix domain sockets, socket options, nonblocking mode, `std::time::Duration` timeouts, shutdown, and owned socket handles. |
| `std::time` | Access monotonic and wall-clock time for CLIs, servers, and tests. | Current `Duration`, `Instant`, `SystemTime`, `Deadline`, `UtcDateTime`, strict `nanoseconds`/`microseconds`/`milliseconds`/`seconds`, fallible `try_nanoseconds`/`try_microseconds`/`try_milliseconds`/`try_seconds`, `now`, `system_now`, strict and fallible Unix timestamp constructors, strict and fallible month-length helpers, UTC calendar conversion, `elapsed`, `sleep`, `deadline_at`, `timeout_after`, and `timeout`; future timers, interruption-aware sleep, pre-1970 calendar policy, and intentionally limited timezone policy. |
| `std::thread` | Start and join OS threads with clear ownership transfer. | Current `Thread`, `spawn`, `join`, `yield_now`, `sleep`, `id`, `is_main`, `available_parallelism`, and `is_join_error` for plain `fn() -> i64` entries; future captured/capability entries, user-facing `ThreadLocal[T]`, custom stack-size options, richer status/result values, and `std::sync` integration. |
| `std::sync` | Share state between threads deliberately. | Current concrete `AtomicI64` with `load`, `store`, `swap`, `fetch_add`, and `compare_exchange`, plus source primitive `Mutex`, `RwLock`, and `Once`; future generic atomics, memory-order parameters, value-protecting `Mutex[T]`/`RwLock[T]`, `Condvar`, `OnceLock`, `LazyLock`, `Barrier`, optional `Semaphore`, `Shared`, `Weak`, MPSC channels, and Linux futex-backed internals after ownership rules are stable. |
| `std::collections` | Store keyed, queue-like, linked, and priority data beyond vectors. | Current linear `Set[T]`, `Deque[T]`, `RingBuffer[T]`, `LinkedList[T]`, `BinaryHeap[T]`, `PriorityQueue[T]`, open-addressed `HashMap`/`HashSet`, red-black-tree `TreeMap`/`TreeSet`, explicit hash/comparator constructors, lookup/update/removal where implemented, FIFO/linked/heap tests, live-bucket hash iterators, sorted tree iterators; future tree deletion and trait-driven constructors. |
| root `Slice[T]` | Borrow contiguous storage without owning or allocating. | Current length/indexing/access, `try_*` access, `slice`, `split_at`, `find`, `contains_slice`, `compare`, prefix/suffix/equality, `chunks`, `windows`, delimiter `split`, and copy-to-vector; future predicate split and stronger trait-bound diagnostics for generic comparison. |
| `std::vec` | Own growable contiguous storage while keeping borrowed sequence work allocation-free. | Current explicit-zone `Vec[T]`, metadata, `try_*` accessors, mutation/growth, copy, iteration, raw pointer access, and direct `slice`, `split_at`, `find`, `contains_slice`, `compare`, `chunks`, `windows`, and delimiter `split` wrappers over live storage; future root/source vector unification and stronger trait-driven collection constructors. |
| `std::iter` | Compose sequence processing without forcing every operation onto each collection type. | Current `range`, `range_inclusive`, `Iterator`, `IntoIterator`, lazy `map`, `filter`, `take`, `skip`, `enumerate`, `zip`, eager `fold`, `reduce`, and zone-backed `collect`; future captured closures, richer adapter inference, and collect targets beyond `std::vec::Vec[T]`. |
| `std::algo` | Provide familiar algorithms over borrowed slices without forcing every helper onto `Slice[T]` itself. | Current `sort`, `sort_by`, `stable_sort`, `stable_sort_by`, `binary_search`, `is_sorted`, `reverse`, `rotate_left`, `rotate_right`, `partition`, `min`, `max`, `clamp`, `swap`, `fill`, `copy`, and `dedup`; future faster sorting and move-aware algorithm contracts. |
| `std::hash` | Provide deterministic non-cryptographic hashing without tying hash policy to one collection type. | Current `Hasher`, `Hash[T]`, `new`, `reset`, `finish`, `write`, `value`, `bytes`, primitive write helpers, and `collections::hash_i64` compatibility; future aggregate/derive impls and trait-driven hash collection constructors. |
| `std::random` | Provide OS seed material and reproducible non-cryptographic random streams without making cryptography promises. | Current `entropy`, direct OS byte `fill`, deterministic `Prng`, `seed`, `from_entropy`, `seed_from_os`, `next`, `below`, `range`, `float`, PRNG byte fill, and generic `shuffle`; future fallible entropy results, unbiased bounded integers, cryptographic stream policy, and distribution helpers. |
| `std::math` | Provide arithmetic helpers whose names communicate policy better than raw operators. | Current natural `i64` bound, sign/parity helpers, checked add/sub/mul/div/rem/neg/abs/pow, wrapping/overflowing add/sub/mul/pow, saturating add/sub/mul/div/neg/abs/pow, strict `pow`, floor/ceil division, `gcd`, and `lcm`; future generic numeric traits and floating helpers. |
| `std::parse` | Parse whole byte-slice values with names that read naturally at call sites. | Current ASCII-trimmed overflow-checked `integer`, `is_integer`, `integer_or`, base-2-through-36 overflow-checked `integer_radix`/`is_integer_radix`/`integer_radix_or`, `hex_integer`, `binary_integer`, boolean parser/validator/fallback helpers, `is_float`, `float_or`, and panicking `float`; future richer parse errors and `Option[f64]`/`Result[f64,E]` after float enum payloads are supported. |
| `std::encoding` | Validate text encodings and convert bytes to portable text forms. | Current `is_ascii`, UTF-8/UTF-16 counts, UTF-8 scalar decode/encode helpers, lowercase hex encode/decode, fallible hex decode, standard base64 encode/decode, and fallible base64 decode; future URL-safe/MIME base64 variants, richer decode errors, normalization/transcoding, and optional compression policy in a separate module. |
| text/path kinds | Keep byte strings, validated text, OS strings, paths, and C strings from collapsing into one API shape. | Current borrowed `std::string::Utf8`, `OsStr`, borrowed `std::c::CStr`, owned `std::c::CString`, `std::path::PathBytes`, and direct string-literal coercion into expected borrowed boundary views; future owned `Utf8String`, `OsString`, `PathBuf`, Windows path/OS-string semantics, fallible owned conversions, and richer C string handoff policy. |
| `std::error` | Give recoverable failures a shared vocabulary instead of bools and sentinel integers. | Current `Kind`, compact `Error`, strict and fallible constructors, POSIX `from_errno`, `from_raw`, `raw`, `kind`, `code`, predicate helpers, root `Error`/`ErrorKind` aliases, and `Result[T, i64]` bridge; future direct `Result[T, Error]`, Windows error mapping, owned messages, and conversions from fs/io/net/process wrappers. |
| `std::log` | Emit simple diagnostics without making every tool invent its own stderr prefix format. | Current `Level`, `rank`, `name`, `enabled`, `write`, `message`, `trace`, `debug`, `info`, `warn`, and `error`; future structured records, global or scoped filters, test-runner capture, and backtrace integration. |
| `std::test` | Let library/application tests aggregate checks before returning one final status. | Current `Report`, `report`, `scratch`, `check`, generic `equal`/`not_equal`, pass/fail accessors, `ok`, `finish`, `require`, and method wrappers; future test discovery/runner integration, named tests, richer assertion messages, log capture, stack/backtrace reporting, optional benchmark helpers, and optional fuzz hooks. |
| `std::os` | Hold platform-specific syscall wrappers that are too sharp for portable modules. | Future Unix/Windows gated modules, raw descriptors/handles, errno mapping, `syscall`, close-on-exec/nonblocking descriptor flags, `fcntl`, `ioctl`, `poll`, `select`, Linux `epoll`/`eventfd`/`timerfd`/`signalfd`, optional `pidfd`/`memfd`, signals, and memory mapping. |

## Phase 2: Pull More Behavior Into Ari Source

- Replace compiler hooks with source code when generic aggregates, trait
  dispatch, and module cache summaries can model them safely.
- Keep compiler-known declarations as compatibility shims only when required.
- Improve diagnostics for partial custom `std` packages.

## Phase 3: Grow Collections And Text

- Stabilize `std::vec::Vec[T]` and document the distinction from bare local
  `Vec[T]`.
- Keep shared collection access vocabulary aligned across `Slice[T]` and
  `std::vec::Vec[T]`, including `try_*` methods for `Option`-based absence.
- Keep borrowed sequence operations on `Slice[T]`: range views, split views,
  subsequence search, lexicographic compare, chunks, windows, and delimiter
  split should stay allocation-free and return views.
- Keep the same borrowed sequence vocabulary available directly on
  `std::vec::Vec[T]` when the vector owns the backing storage and the operation
  can return views without allocation.
- Add collection helpers in small slices: slice methods, vector methods, the
  current linear `Set[T]` access/update/reserve/iteration surface, hash/table
  lookup and tombstones, tree insertion/lookup, hash and tree iterators,
  deque/ring-buffer/linked-list/heap families, red-black deletion, and then
  trait-driven constructors.
- Keep `HashMap`/`HashSet` and `TreeMap`/`TreeSet` on explicit hash or
  comparator constructors until generic trait-driven `Hash`, `Eq`, and `Ord`
  selection is testable.
- Grow `std::algo` in small source slices: keep the current borrowed-slice
  algorithms simple, then add faster sort implementations only after ownership
  and temporary-storage policy are clearer.
- Grow `std::hash` toward derived aggregate impls and trait-driven collection
  constructors after `Hash`/`Eq` dispatch policy is tested.
- Keep `std::random` split between OS entropy and deterministic PRNGs. Add
  fallible entropy before exposing any cryptographic stream or
  distribution-heavy API.
- Keep `std::string::String` byte-oriented while exposing explicit UTF-8 scalar
  validation/access/append helpers through `std::encoding`.
- Keep typed string/path boundary views distinct: borrowed C ABI text uses
  `string` or `std::c::CStr`, owned C-shaped storage uses `std::c::CString`,
  general owned buffers use byte `String`, validated Unicode scalar work uses
  `Utf8`, OS boundary bytes use `OsStr`, and path manipulation uses
  `PathBytes`.
- Expose small `String` conveniences only when they preserve byte-string
  semantics, such as ASCII case comparison/search, borrowed ASCII trim views,
  owned trim copies, borrowed split/chunk/window views, byte-slice search,
  allocator-backed `join_in`, Display/Debug appends, and whole/prefix ASCII
  parsers.
- Keep ASCII-only helpers in `std::ascii` so byte-oriented classification,
  comparison, search, trimming, and parsing behavior is explicit at call sites.
- Keep whole-input value parsing in `std::parse` so application code does not
  scatter ad hoc integer/bool/float parsers. Preserve natural names without
  type suffixes; the module path already carries the parsing policy.
- Keep validation, UTF-8 scalar helpers, and byte codecs in `std::encoding`,
  not in `std::hash` or `std::algo`. Hex/base64 are implemented now;
  compression remains optional future work after byte-buffer ownership and
  error values improve.

## Phase 4: Numerics

- Expand `std::math` from i64 signatures to generic numeric helpers when the
  language has the right trait vocabulary. Preserve the existing natural names
  for signs, sign predicates, parity,
  checked add/sub/mul/div/rem/neg/abs/pow, saturating add/sub/mul/div/neg/abs/pow,
  wrapping/overflowing add/sub/mul/pow, powers, division rounding, and divisor helpers.
- Expand `std::bits` from u64 signatures to generic integer mask, rotation,
  power-of-two, low-mask, byte-swap, population-count, and bit-scan helpers
  when the same trait vocabulary exists.
- Add policy-specific parser overflow handling after compiler diagnostics make
  those operations reliable across integer widths.
- Add floating helpers only where LLVM lowering and runtime behavior are
  stable.

## Phase 5: OS-Facing Libraries

- Grow `std::env` from the current argument, variable, current-directory, and
  executable-path base into path normalization and platform-specific policy
  once owned-string behavior is stable.
- Keep `std::target` as the cross-cutting target fact module. It can expose
  compile-target facts such as x86_64/aarch64/riscv64, glibc/musl, ELF/DWARF,
  TLS, syscall ABI, errno ABI, and Linux API families; it must not grow into a
  raw syscall wrapper.
- Grow `std::path` from lexical POSIX-style helpers and borrowed `PathBytes`
  into platform-aware `Path`/`PathBuf` values once owned path buffers and
  canonicalization policy are stable.
- Keep `std::context` as the low-level runtime state boundary: arguments and
  runtime thread identity are implemented now. `std::thread` extends the same
  thread-id slot for spawned Ari threads and exposes hosted available
  parallelism instead of changing the public context API shape.
- Grow `std::process` from current-process helpers and the first POSIX
  fork/wait slice into portable child-process handles. `std::time` and
  `std::fs` now have their first thin wrappers; time should grow toward timers
  and interruption-aware sleep, while filesystem work should next add stronger
  owned-resource policy, richer directory-entry metadata, richer metadata
  timestamps/no-follow symlink metadata, owner/group/ACL permission policy,
  recursive directory helpers, owned path values, temporary files, path helpers,
  richer link metadata/platform symlink policy, and optional locking.
  `std::thread` now has its first function-pointer spawn/join wrapper, sleep
  convenience, and available-parallelism hook, and `std::sync` has a first
  concrete atomic integer primitive plus source `Mutex`, `RwLock`, and `Once`
  helpers. Grow them toward explicit ownership transfer, user-facing TLS,
  stack-size options, richer statuses, generic atomics, value-protecting locks,
  blocking waits, channels, and safer shared-state policy.
- Keep syscall-facing helpers minimal and modern: process arguments and
  environment, current directory, file descriptors/handles, time, process
  spawn/fork where the platform supports it, thread creation/join, atomics or
  shared ownership handles, and error-code conversion.
- Track Linux-specific epoll, inotify, eventfd, timerfd, signalfd, pidfd,
  memfd, optional fanotify, optional io_uring, procfs, sysfs, cgroups,
  namespaces, seccomp, and capabilities under `docs/stdlib/platform/` until
  `std::os` has an owned descriptor and error policy.
- Keep handles visible and owned; do not hide OS resources behind global state.
- Track runtime ABI foundations separately in `docs/dev/runtime-support.md`:
  `_start`, `crt0`, init/fini arrays, TLS setup, stack protector hooks,
  panic/unwind personalities, `.eh_frame`, backtraces, dynamic linker
  compatibility, compiler-rt/libgcc-style helpers, atomic fallbacks, and
  memory builtins should become public library assumptions only after the
  runtime contract is documented and testable.

## Phase 6: Library Developer Experience

- Grow the current source-level `std::test::Report` helpers into a real test
  runner when discovery, per-test isolation, and diagnostics are stable.
- Add richer `@test` or `ari test` integration around library test cases.
- Grow the current `std::log` line helpers toward structured logging only
  after formatting and capture policy are documented.
- Build compiler-tooling source maps and diagnostic labels outside runtime
  `std`; add stack/backtrace, optional benchmark, and optional fuzzing APIs
  only after their runtime and driver contracts are documented.
- Keep examples short enough to serve as contributor templates.
