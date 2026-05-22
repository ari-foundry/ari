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
direct algorithm wrappers, `algo` slice sort/search/reorder helpers, `hash` deterministic
hasher/value/byte-slice helpers, `random` OS entropy plus deterministic
non-cryptographic PRNG helpers, `mem` layout,
pointer, value, byte memory, and hosted page-size helpers, `zone` raw
allocation plus source typed array allocation and `ZoneMetadata`/`ZoneBacked`
handle zone metadata access,
`boxed`, `string` byte access/search/split/chunk/window/join/ASCII helpers
including case search, prefix parsers, owned trim copies, typed borrowed
`Utf8`/`OsStr` views, and `std::c::CStr` convenience construction, `ascii`
byte classification, case-insensitive comparison/search, slice helpers, and
prefix parsers, `vec` source growable sequence handles with direct borrowed
range/split/subsequence/compare/chunk/window wrappers, `iter` range/trait
support plus lazy adapters and eager consumers, `fmt` formatting traits plus
strict and fallible `FormatSpec` builders, `cmp` three-way ordering and value
comparison helpers, `convert` identity/from/into helpers, `context` runtime hooks plus the source
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
`Stderr`, `Cursor`, `BufReader`, `BufWriter`, `read_exact`, `read_all`,
`read_to_string`, `try_copy`, `copy`, `write_all`, `flush`, stderr routing,
byte-slice output, and `std::fs::File` Reader/Writer/Seek adapters, current
`process` id/uid/gid/exit/abort/status/root helpers plus the first POSIX
fork/wait slice, `thread`
function-pointer spawn/join/sleep/yield hooks plus runtime ids, advisory
completion, available parallelism, `Builder`, and source handle helpers,
`sync` concrete atomic wrappers, explicit `Ordering` vocabulary, source
`Mutex`, `RwLock`, `Once`, `OnceLock`, `Condvar`, `Barrier`, and single-slot
MPSC channel helpers,
`time`
monotonic/wall-clock/sleep hooks plus source
strict and fallible `Duration` constructors, strict and fallible Unix
timestamp constructors, strict and fallible calendar helpers,
`Instant`/`SystemTime` helpers, `fs` byte-oriented file existence,
access-permission/open/read/write/append/close/remove hooks plus source
`File`/`Permissions`/`Metadata` methods, direct path-kind predicates, and
`Option[File]` open helpers,
`create`/`try_create`, non-truncating `ensure_file`, whole-file
`read`/`write`/`try_write`/`append`/`try_append`, file cursor `position`/`seek`,
`truncate`, streaming `copy`/`try_copy`, `rename`,
single-directory `create_dir`/`ensure_dir`/`remove_dir`, recursive
`create_dir_all`/`ensure_dir_all`, `read_to_string`, and
`Option[String]` `try_read`/`try_read_to_string`, and `Option[Metadata]`
`try_metadata`, permission `mode`/`try_mode`/`set_mode`/`set_permissions`,
plus existing-path `try_canonicalize`,
`path` POSIX-style lexical separator, typed `PathBytes`, component,
component-aware prefix/suffix, join, and lightweight normalization helpers,
`net` source IP/socket-address values plus hosted IPv4 DNS lookup and
host-port endpoint resolution, IPv4 TCP
listener/stream handles, IPv4 UDP single-byte datagrams, Unix stream sockets,
descriptor views, nonblocking flags, `std::time::Duration` socket timeouts
with raw millisecond compatibility helpers, stream shutdown, and TCP/Unix
`std::io` adapters,
`collections::Set[T]` as the linear insertion-order set with `try_*`
accessors, `pop`/`try_pop`, replace-or-insert updates, set-relationship
predicates, explicit reserve growth plus tracked-local zone inference across
growable collection mutation calls, direct iterator support, target-zone
collection copy across collection families, natural `contains_key`,
`contains_value`, `get_or`, entry update, key-value removal, map entry
`iter()`/`iter_mut()`/direct iteration, mutable value cursors, and drain helpers,
open-addressed `HashMap`/`HashSet` with hash-set relationship predicates,
tombstones, and live-bucket iterators, red-black-tree `TreeMap`/`TreeSet` with
tree-set relationship predicates, map entry iteration, ordered boundary entries,
direct red-black deletion, sorted iterators, and sorted drains, growable `Deque`, fixed `RingBuffer`, zone-backed
`LinkedList`, and `BinaryHeap`/`PriorityQueue` priority containers, and the
first `parse` whole-input integer/bool/decimal-float parsers, `encoding`
ASCII/UTF-8/UTF-16 validation plus hex/base64 codecs, and `math` sign,
checked add/sub/mul/div/rem/pow, wrapping/overflowing add/sub/mul/pow, saturating
arithmetic including powers, division-rounding and
`bits` numeric helper slices, including checked/wrapping alignment,
byte-swap, population-count, and zero/one-run bit scans.

## Essential Library Families

These are the library families Ari needs for modern application and systems
work. Each one should land in small tested slices with natural API names.

| Family | Purpose | Current Or Planned APIs |
| --- | --- | --- |
| `std::io` | Provide byte-oriented process IO contracts that other libraries can share without hiding raw hooks. | Current `Reader`, `Writer`, `Seek`, `Stdin`, `Stdout`, `Stderr`, `Pipe`, `PipeReader`, `PipeWriter`, `Cursor`, `BufReader`, `BufWriter`, `stdin`, `stdout`, `stderr`, `pipe`, `cursor`, `buf_reader`, `buf_writer`, direct `Error` helpers `read_exact_result`/`copy_result`/`write_all_result`/`flush_result`, compatibility `read_exact`, zone-backed `read_all`, zone-backed `read_to_string`, generic `try_copy`/`copy`, `write_all`, `flush`, stderr routing, pipe adapters over `std::os::Pipe`, `std::fs::File` Reader/Writer/Seek adapters, and raw scalar/byte/line hooks; future zone-owning buffered constructors and drop-time flush after owned resource policy is settled. |
| `std::env` | Read startup and environment state without exposing raw runtime hooks. | Current `arg_count`, `arg`, `arg_os`, `has_arg`, `try_arg`, `try_arg_os`, `program_name`, `program_name_os`, `get`, `get_os`, `has`, `try_get`, `try_get_os`, `set`, `remove`, `current_dir`, `current_dir_os`, `current_dir_path`, `try_current_dir`, `try_current_dir_os`, `try_current_dir_path`, `set_current_dir`, `executable_path`, `executable_path_os`, `try_executable_path`, `try_executable_path_os`; future path normalization and platform-specific expansion. |
| `std::target` | Report compiler-known target facts without requiring users to parse triples by hand. | Current `triple`, `arch`, `arch_name`, `os`, `os_name`, `env`, `env_name`, `object_format`, `debug_format`, `errno_abi`, `pointer_bits`, `long_bits`, source predicates for Linux/glibc/musl/ELF/DWARF/TLS, Linux syscall ABI classification, and Linux API-family predicates for procfs/sysfs/vDSO/epoll/inotify/eventfd/timerfd/signalfd/memfd plus optional API families; future build-profile facts for static/dynamic/PIE/RELRO/stack-protector only after the driver owns those flags. |
| `std::c` | Keep C ABI boundary helpers readable without making every program write raw loader and errno bindings. | Current `CStr`, `CString`, `Library`, `Symbol`, borrowed C string wrappers, zone-backed NUL-terminated storage, `errno`/`error`, `dlopen`/`dlsym`/`dlclose`/`dlerror` wrappers, typed dynamic function symbols, and root type aliases; future data-symbol wrappers, fallible string constructors, target-specific errno symbols, return-type generic inference, and explicit FFI escape rules for zone-backed buffers. |
| `std::process` | Represent the current process and child processes explicitly. | Current `id`, `uid`, `gid`, `exit`, `abort`, `success`, `failure`, typed `ExitCode`, status/root predicates, POSIX `fork_result`/`wait_status_result`/`wait_result` direct `Error` helpers, raw compatibility `fork`/`wait`, child/error predicates, typed `Signal` helpers, `Arg`/`EnvVar` wrappers, `Command` builder with `arg`/`args`/`env`/`env_var`/`current_dir`, module-level and method `spawn`/`status`/`exit_status`/`output`/`output_in`/`exec`, `Child` handle, `ChildStdin`/`ChildStdout`/`ChildStderr` aliases, typed `ExitStatus`, `Output` capture handle, `kill`, `kill_signal`, `terminate`, current/executable path wrappers, temp file/temp dir constructors, child environment setup, child working-directory setup, normal/signal status inspection, and small stdout/stderr capture; future large-output readiness draining, stdin redirection, portable Windows mapping, richer platform status fields, daemon helpers as optional policy, and environment inheritance/clearing policy. |
| `std::fs` | Work with files and directories through explicit handles. | Current `File`, `OpenOptions`, `Dir`, `DirEntry`, `FileKind`, `Metadata`, `Permissions`, existence/access checks, target-following and no-follow metadata, metadata access/modification/status-change timestamps, file-kind predicates, POSIX permission mode lookup/mutation, canonicalization, links, non-truncating file creation, single and recursive directory helpers, directory-name and entry reads, lazy `DirEntry` metadata, streaming directory handles, mode-string and named `OpenOptions` opens, `open_result`/`create_result`/`OpenOptions::open_result` direct `Result[File, Error]` forms, `metadata_result`/`symlink_metadata_result`/`file_type_result`/`mode_result` direct query results, `canonicalize_result`/`read_link_result` and directory `open_dir_result`/`read_dir_result`/`read_dir_entries_result`, `read_result`/`read_to_string_result` direct `Result[String, Error]` forms, `remove_result`/`rename_result`/`create_dir_result`/`remove_dir_result` direct `Result[(), Error]` forms, `write_result`/`append_result`/`copy_result` direct `Result[i64, Error]` forms, raw `*_raw_result` compatibility variants, byte/file-cursor/whole-file helpers, `std::io` adapters, descriptor views, and explicit close; future owned resource policy, richer per-entry errors, richer owner/group/ACL and portable creation/birth-time policy, owned path values, temporary files, richer link metadata/platform symlink policy, optional file locking, and structured filesystem error fields. |
| `std::path` | Manipulate path bytes without opening the filesystem. | Current POSIX-style `PathBytes`, `bytes`, `from_os`, method-style path wrappers, `is_separator`, `is_absolute`, `is_relative`, `trim_trailing_separators`, borrowed `components`, `file_name`, `parent`, `extension`, `stem`/`file_stem`, allocation-free `has_*` predicates, component-aware `starts_with`/`ends_with` and borrowed `strip_prefix`/`strip_suffix`, zone-backed `with_file_name_in`/`with_extension_in`, `join_in`, and `normalize_in`; existing-path runtime canonicalization lives in `std::fs`; future platform-specific paths, owned `Path`/`PathBuf`, and richer component kinds. |
| `std::net` | Represent network addresses, DNS results, and explicit socket handles. | Current `Ipv4Addr`, `Ipv6Addr`, `IpAddr`, `SocketAddr`, constructors, strict and fallible indexed accessors, family predicates, loopback/unspecified checks, port helpers, hosted IPv4 `lookup_v4` plus direct `Error` and raw compatibility lookup results, `"host:port"` `resolve`/`resolve_result`/`resolve_raw_result`, `to_socket_addrs`, `ToSocketAddrs`, host-port `connect_host`/`tcp_connect_host`, module-level `listen`/`connect`/`tcp_listen`/`tcp_connect`/`udp_bind`/`unix_listen`/`unix_connect`, hosted IPv4 `TcpListener`/`TcpStream` bind/connect/accept/local-port/local-address/peer-address helpers with direct `Error` and raw compatibility result forms, hosted IPv4 `UdpSocket` bind/local-address/send-byte/receive-byte helpers with direct `Error` and raw compatibility bind results, hosted Unix stream listener/connect/accept helpers with direct `Error` and raw compatibility result forms, descriptor views, explicit close, nonblocking flags, TCP listener/UDP reuse-address helpers, TCP nodelay helpers, `std::time::Duration` read/write/accept timeouts plus raw millisecond compatibility helpers, `Shutdown`, TCP/Unix `std::io::Reader`/`Writer` byte adapters, TCP/Unix `read_exact`/`write_all` stream buffer helpers, and restricted-host `PermissionDenied` error bridge tests; future IPv6 socket handles, multi-address DNS iterators, service-name resolution, buffer-oriented UDP datagrams, UDP source address helpers, remaining socket options, timeout-specific error results, and richer owned socket policy. |
| `std::time` | Access monotonic and wall-clock time for CLIs, servers, and tests. | Current `Duration`, `Instant`, `SystemTime`, `Deadline`, `UtcDateTime`, strict `nanoseconds`/`microseconds`/`milliseconds`/`seconds`, fallible `try_nanoseconds`/`try_microseconds`/`try_milliseconds`/`try_seconds`, `now`, `system_now`, strict and fallible Unix timestamp constructors, strict and fallible month-length helpers, UTC calendar conversion, `elapsed`, `sleep`, `deadline_at`, `timeout_after`, and `timeout`; future timers, interruption-aware sleep, pre-1970 calendar policy, and intentionally limited timezone policy. |
| `std::thread` | Start and join OS threads with clear ownership transfer. | Current `Thread`, `Builder`, explicit `ThreadLocal[T]`, `spawn`, `spawn_configured`, `join`, advisory `is_finished`, `yield_now`, `sleep`, `id`, `is_main`, `available_parallelism`, and `is_join_error` for plain `fn() -> i64` entries; future captured/capability entries, compiler-level `thread_local` declaration sugar, richer status/result values, and `std::sync` integration. |
| `std::sync` | Share state between threads deliberately. | Current `AtomicI64`, `AtomicBool`, `AtomicUsize`, `AtomicPtr[T]`, explicit `Ordering`, source primitive `Mutex`, `RwLock`, `Once`, `OnceLock`, `Condvar`, `Barrier`, and single-slot MPSC channels; future target-native generic atomics, value-protecting `Mutex[T]`/`RwLock[T]`, optional `Semaphore`, blocking wait/wake, timeout waits, and Linux futex-backed internals after ownership rules are stable. |
| `std::cell` | Mutate small internal state without changing the outer API shape. | Current `Cell`, runtime-checked `RefCell`, zone-backed `OnceCell`, and `Lazy`; future closure captures, initializer result policy, richer move-aware value contracts, and optional sync-facing `LazyLock` naming if the thread-sharing policy wants it. |
| `std::rc` | Share ownership explicitly without a magical global heap. | Current zone-backed `Rc`, atomic-control-block `Arc`, and `Weak` with downgrade/upgrade/count helpers; future send/share trait enforcement, mutable unique access, copy-on-write helpers, cycle diagnostics, and possible root `Shared` policy alias after the ownership model is stable. |
| `std::collections` | Store keyed, queue-like, linked, and priority data beyond vectors. | Current linear `Set[T]`, `Deque[T]`, `RingBuffer[T]`, `LinkedList[T]`, `BinaryHeap[T]`, `PriorityQueue[T]`, open-addressed `HashMap`/`HashSet`, red-black-tree `TreeMap`/`TreeSet`, tracked-local zone inference for growable mutation calls, target-zone copy across collection families, natural `contains_key`/`contains_value` predicates and `get_or` fallback lookup for maps, direct `get_mut`/`try_get_mut` mutable map value access, named map `replace`, `HashMapEntry`/`TreeMapEntry` update handles with `or_insert`/`or_insert_with`/`or_default`/`and_modify` plus direct `insert`/`insert_entry`/`remove`/`key`/`value`/`value_mut`, `remove_entry` key-value removal, set representative `get`/`try_get`, `MapEntry[K,V]` map entry iterators and tree boundary entries with `key()`/`value()` accessors, map `iter()` aliases, direct map `IntoIterator`, mutable map value cursors, map `iter_mut()` over copied-key mutable-value handles, in-place `retain` for `Set`/`HashMap`/`HashSet`, collection drains for maps and sets, ordered tree boundary and lower/upper-bound access, direct red-black tree deletion with compacting slot movement, explicit hash/comparator constructors, lookup/update/removal where implemented, FIFO/linked/heap tests, live-bucket hash iterators, sorted tree iterators; future borrowed hash lookup, lazy set algebra iterators, tree retain/range mutation, split/append operations, first-class reference-valued iterator item support for tighter mutable cursor ergonomics, and trait-driven constructors. |
| root `Slice[T]` | Borrow contiguous storage without owning or allocating. | Current length/indexing/access, mutable element borrows, `try_*` access, `iter`/`iter_mut` with mutable value handles, `slice`, `split_at`, endpoint splitting, `find`, `contains_slice`, predicate search/count helpers, `compare`, prefix/suffix/equality, prefix/suffix stripping, `chunks`, `windows`, delimiter `split`, copy-to-vector, receiver-form algorithm wrappers for reverse/rotate/fill/copy_from/partition/stable_partition/dedup/dedup_by/dedup_by_key/introsort-backed sort/merge-sort-backed stable_sort plus explicit-zone and `Result` stable-sort forms/is_sorted/binary_search/lower_bound/upper_bound/equal_range/partition_point/min/max, and half-open `copy_within`/`fill_range`/`reverse_range`/`rotate_range`; future predicate split and stronger trait-bound diagnostics for generic comparison. |
| `std::vec` | Own growable contiguous storage while keeping borrowed sequence work allocation-free. | Current explicit-zone `Vec[T]`, metadata, `try_*` accessors including optional indexed removal, mutation/growth, capacity convenience (`try_reserve`, `shrink_to_fit`, associated `with_capacity`), slice and iterator extension, `append`, `resize_with`, `swap_remove`, `split_off`, whole-vector and range draining, range insert/remove/splice helpers, stable in-place `retain`, owned `dedup`/`dedup_by`/`dedup_by_key`, live-prefix `fill`/`copy_from`/`partition`/`stable_partition`, half-open `copy_within`/`fill_range`/`reverse_range`/`rotate_range`, copy, `iter`/`iter_mut`/`IntoIterator`, raw pointer access, direct `slice`, `split_at`, `find`, `contains_slice`, `compare`, `chunks`, `windows`, and delimiter `split` wrappers over live storage, plus in-place `reverse`, rotation, introsort-backed sorting, merge-sort-backed stable sorting with explicit-zone and `Result` forms, sortedness, binary search, lower/upper/equal-range bounds, partition point, and min/max wrappers. Current value movement contracts are documented for copyable/plain elements, generator-based growth, vector shrink/drop paths, split-off tails, unordered removal, and drain cursors; future root/source vector unification, clone-aware growth traits, move-aware place transfers, and stronger trait-driven collection constructors remain. |
| `std::iter` | Compose sequence processing without forcing every operation onto each collection type. | Current `range`, `range_inclusive`, finite `empty`/`once` sources, generator-backed `repeat_with`, `Iterator`, `IntoIterator`, Slice/Vec iterator sources, mutable value cursors, lazy `map`, `filter`, `take`, `skip`, `enumerate`, `zip`, eager `count`, `count_if`, `nth`, `last`, `find_if`, `position`, `any`, `all`, `fold`, `reduce`, and zone-backed `collect`; future captured closures, richer adapter inference, first-class reference-valued iterator items, and collect targets beyond `std::vec::Vec[T]`. |
| `std::algo` | Provide familiar algorithms over borrowed slices and back the natural receiver methods. | Current `sort`/`sort_by` introsort with 3-way partitioning, insertion-sort cutoff, median/ninther pivoting, and heapsort fallback; `stable_sort`/`stable_sort_by` merge sort with explicit-zone `stable_sort_in`/`stable_sort_by_in` and `Result` wrappers; `binary_search`, `lower_bound`, `upper_bound`, `equal_range`, `partition_point`, `is_sorted`, `reverse`, half-open `reverse_range`, `rotate_left`, `rotate_right`, half-open `rotate_range`, `partition`, `stable_partition`, `min`, `max`, `clamp`, `swap`, `fill`, half-open `fill_range`, `copy`, overlap-safe half-open `copy_within`, `dedup`, `dedup_by`, and `dedup_by_key`, with comparator `*_by` variants where ordering matters. Current copy/drop/overlap limits are documented in `docs/stdlib/value-contracts.md`; future by-reference comparator forms, clone/generator APIs, true fallible allocation, and move-aware place transfers remain. |
| `std::hash` | Provide deterministic non-cryptographic hashing without tying hash policy to one collection type. | Current `Hasher`, `Hash[T]`, `new`, `reset`, `finish`, `write`, `value`, `bytes`, fixed-width integer and bool write helpers, and `collections::hash_i64` compatibility; future aggregate/derive impls and trait-driven hash collection constructors. |
| `std::random` | Provide OS seed material and reproducible non-cryptographic random streams without making cryptography promises. | Current strict `entropy`, direct OS byte `fill`, recoverable `entropy_result`/`fill_result` and raw compatibility result forms, deterministic `Prng`, `seed`, `from_entropy`, `from_entropy_result`, `seed_from_os`, `seed_from_os_result`, `next`, `boolean`, unbiased `below`/`range` plus fallible bound checks, `float`, PRNG byte fill, and generic `shuffle`; future larger distribution tests, cryptographic stream policy, and distribution helpers. |
| `std::math` | Provide arithmetic helpers whose names communicate policy better than raw operators. | Current natural `i64` bound, sign/parity helpers, checked add/sub/mul/div/rem/neg/abs/pow, wrapping/overflowing add/sub/mul/pow, saturating add/sub/mul/div/neg/abs/pow, strict `pow`, floor/ceil division, `gcd`, and `lcm`; future generic numeric traits and floating helpers. |
| `std::parse` | Parse whole byte-slice values with names that read naturally at call sites. | Current ASCII-trimmed overflow-checked signed `integer`, `is_integer`, `integer_or`, base-2-through-36 signed `integer_radix`/`is_integer_radix`/`integer_radix_or`, unsigned `unsigned`/`is_unsigned`/`unsigned_or` and `unsigned_radix` helpers, `hex_integer`, `binary_integer`, `octal_integer`, boolean parser/validator/fallback helpers, `is_float`, `float_or`, panicking `float`, and trait-backed `Parse` with `parse<T>`/`parse_or<T>`/`is_parse<T>` for `i64`, `u64`, `bool`, and `f64`; future richer parse errors and `Option[f64]`/`Result[f64,E]` after float enum payloads are supported. |
| `std::encoding` | Validate text encodings and convert bytes to portable text forms. | Current `is_ascii`, UTF-8/UTF-16 counts, detailed UTF-8 validation errors, UTF-8 scalar decode/encode helpers, lowercase hex encode/decode, fallible hex decode, standard base64 encode/decode, and fallible base64 decode; future URL-safe/MIME base64 variants, richer codec decode errors, normalization/transcoding, and optional compression policy in a separate module. |
| text/path kinds | Keep byte strings, validated text, OS strings, paths, and C strings from collapsing into one API shape. | Current byte-oriented `std::string::String` with strict and `try_` byte access/removal plus stable byte `retain`, borrowed `std::string::Utf8`, `OsStr`, borrowed `std::c::CStr`, owned `std::c::CString`, `std::path::PathBytes`, and direct string-literal coercion into expected borrowed boundary views; future owned `Utf8String`, `OsString`, `PathBuf`, Windows path/OS-string semantics, fallible owned conversions, and richer C string handoff policy. |
| `std::error` | Give recoverable failures a shared vocabulary instead of bools and sentinel integers. | Current `Kind` categories including filesystem, IO, and network cases such as `ConnectionRefused`, compact `Error`, strict and fallible constructors, POSIX `from_errno`/`from_os_code`, errno-result and raw-result conversion helpers, `to_raw_result`, `raw`, `kind`, `code`, predicate helpers, root and fs/io/net/os/process `Error`/`ErrorKind` aliases, Display/Debug formatting, direct `Result[T, Error]` conversion helpers, and first direct fs/io/net/process result surfaces; future Windows error mapping, owned messages, structured fields, and remaining wrapper conversions. |
| `std::log` | Emit simple diagnostics without making every tool invent its own stderr prefix format. | Current `Level`, `rank`, `name`, `enabled`, `write`, `message`, `trace`, `debug`, `info`, `warn`, and `error`; future structured records, global or scoped filters, test-runner capture, and backtrace integration. |
| `std::test` | Let library/application tests aggregate checks before returning one final status. | Current `Report`, `report`, `scratch`, temp file/dir forwarding helpers, snapshot/golden byte comparisons, `Bench` elapsed-time handles, `check`, generic `equal`/`not_equal`, pass/fail accessors, `ok`, `finish`, `require`, method wrappers, compiler `@test` runner generation, `ari test` subcommand, substring filtering, non-zero `i64` test-status propagation, and stderr progress/failure markers; future per-test panic isolation/log capture, named result summaries, richer assertion messages, doctest extraction, stack/backtrace reporting, statistical benchmark harnesses, and optional fuzz hooks. |
| `std::os` | Hold platform-specific values and syscall wrappers that are too sharp for portable modules. | Current non-owning `Fd` view, owning `OwnedFd`, owned `Pipe`, `fd`, `invalid`, `stdin`, `stdout`, `stderr`, `pipe`, descriptor predicates, equality, `OwnedFd::from_raw`, `as_fd`, `take`, `try_clone`, `close_on_exec`, `set_close_on_exec`, `is_nonblocking`, `set_nonblocking`, `close`, pipe end inspection/take/close helpers, `std::fs::File.descriptor`, and `std::fs::File` as `std::io::Reader`/`Writer`/`Seek`; future errno mapping, `syscall`, duplicate-with-flags policy, broader `fcntl`, `ioctl`, `poll`, `select`, Linux `epoll`/`eventfd`/`timerfd`/`signalfd`, optional `pidfd`/`memfd`, signals, and memory mapping. |

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
  split should stay allocation-free and return views. Receiver-form algorithm
  wrappers should mutate the borrowed storage in place or return scalar/Option
  answers without allocating.
- Keep the same borrowed sequence vocabulary available directly on
  `std::vec::Vec[T]` when the vector owns the backing storage and the operation
  can return views without allocation.
- Add collection helpers in small slices: slice methods, vector methods, the
  current linear `Set[T]` access/update/reserve/iteration surface, hash/table
  lookup and tombstones, tree insertion/lookup, hash and tree iterators,
  deque/ring-buffer/linked-list/heap families, direct red-black deletion, and
  then trait-driven constructors.
- Keep `HashMap`/`HashSet` and `TreeMap`/`TreeSet` on explicit hash or
  comparator constructors until generic trait-driven `Hash`, `Eq`, and `Ord`
  selection is testable.
- When trait-driven collection constructors land, make the common constructor
  names use trait defaults: `HashMap::new`/`HashSet::new` from `Hash + Eq`,
  and `TreeMap::new`/`TreeSet::new`/heap constructors from `Ord`. Keep custom
  policy in explicit names such as `with_hash` or `with_less` so unusual
  ordering remains obvious at call sites.
- Grow `std::algo` from the current production-shaped sort engines toward
  by-reference comparators, true fallible temporary allocation, and move-aware
  place transfers for resource-owning elements.
- Grow `std::hash` toward derived aggregate impls and trait-driven collection
  constructors after `Hash`/`Eq` dispatch policy is tested.
- Keep `std::random` split between OS entropy and deterministic PRNGs. Grow
  failure injection and distribution tests before exposing any cryptographic
  stream or distribution-heavy API.
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
  power-of-two, low-mask, checked/wrapping alignment, byte-swap,
  population-count, and bit-scan helpers when the same trait vocabulary exists.
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
- Grow `std::process` from current-process helpers, POSIX fork/wait,
  `ExitCode`, typed `ExitStatus`/`Signal`, temp path constructors, and the
  current `Command`/`Child`/`Output` builder into fully portable child-process
  handles. The next process slices should add readiness-based large output
  draining, explicit stdin redirection, richer platform status fields, and
  Windows mapping.
  `std::time` and `std::fs` now have their first thin wrappers; time
  should grow toward timers
  and interruption-aware sleep, while filesystem work should next add stronger
  owned-resource policy, richer per-entry directory errors, portable metadata
  creation/birth-time policy, owner/group/ACL permission policy,
  owned path values, temporary files, path helpers,
  richer link metadata/platform symlink policy, and optional locking.
  `std::net` now has IPv4 DNS lookup, host-port endpoint resolution, TCP, UDP, and Unix stream handles on top
  of `std::os::OwnedFd`, plus method-style TCP/Unix stream buffer helpers and
  IPv4 local-address reporting and direct `Error` result helpers; next network
  work should add IPv6 handles, buffer-oriented UDP datagrams, UDP source
  address reporting, remaining socket options, and
  timeout-specific error results.
  `std::thread` now has its first function-pointer spawn/join wrapper, sleep
  convenience, available-parallelism hook, advisory completion check,
  configured builder spawn, and explicit thread-local handle. `std::sync` has
  concrete atomic wrappers, source `Mutex`,
  `RwLock`, `Once`, `OnceLock`, `Condvar`, `Barrier`, and a single-slot MPSC
  channel. Grow them toward explicit ownership transfer, user-facing TLS,
  richer statuses, target-native atomics, value-protecting locks, blocking
  waits, and safer shared-state policy.
- Keep syscall-facing helpers minimal and modern: process arguments and
  environment, current directory, file descriptors/handles, time, process
  spawn/fork where the platform supports it, thread creation/join, atomics or
  shared ownership handles, and error-code conversion.
- Track Linux-specific epoll, inotify, eventfd, timerfd, signalfd, pidfd,
  memfd, optional fanotify, optional io_uring, procfs, sysfs, cgroups,
  namespaces, seccomp, and capabilities under `docs/stdlib/platform/` until
  `std::os` has duplicate-with-flags and a richer error policy.
- Keep handles visible and owned; do not hide OS resources behind global state.
- Track runtime ABI foundations separately in `docs/dev/runtime-support.md`:
  `_start`, `crt0`, init/fini arrays, TLS setup, stack protector hooks,
  panic/unwind personalities, `.eh_frame`, backtraces, dynamic linker
  compatibility, compiler-rt/libgcc-style helpers, atomic fallbacks, and
  memory builtins should become public library assumptions only after the
  runtime contract is documented and testable.

## Phase 6: Library Developer Experience

- Keep growing the current `ari test`/`@test` runner from generated executable
  calls and stderr progress markers toward per-test process isolation, true
  panic capture, named result summaries, and stable diagnostics.
- Add doctest extraction once source-span rendering and example compilation
  policy are stable enough for docs to become executable fixtures.
- Grow the current `std::log` line helpers toward structured logging only
  after formatting and capture policy are documented.
- Build compiler-tooling source maps and diagnostic labels outside runtime
  `std`; add stack/backtrace, optional benchmark, and optional fuzzing APIs
  only after their runtime and driver contracts are documented.
- Keep examples short enough to serve as contributor templates.
