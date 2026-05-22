# Standard Library Overview

Ari's standard library is intentionally small, explicit, and source-first. It
exists to provide the common building blocks every Ari program needs without
hiding allocation, ownership, or backend behavior.

## Design Goals

- Keep public APIs readable from source in `lib/std.arih` and `lib/std/`.
- Prefer source Ari implementations over compiler hooks.
- Keep allocation explicit through `Zone`; there is no invisible global heap.
- Make ownership and borrowing behavior visible in API signatures.
- Track every public declaration in `tests/std_api_manifest.txt`.
- Keep tests focused enough that a new contributor can copy a nearby pattern.

## Return Shape Policy

Use enums for real alternatives. `Option[T]` means a value may be absent, such
as `try_get`, `binary_search`, parser failure, or checked arithmetic overflow.
`Result[T, E]` means success and failure are different control-flow outcomes.

Use tuples for small product values that are always present and conventional at
the call site. `math::overflowing_add(left, right)` and
`math::overflowing_sub(left, right)` return `(value, overflowed)` because the
wrapped result and the flag are both produced every time. `iter::enumerate` and
`iter::zip` likewise yield tuple items. Prefer a
named struct when fields need domain names, invariants, methods, or longer-term
API evolution.

## Module Map

| Module | Purpose | First Things To Use |
| --- | --- | --- |
| `std` | Prelude root, shared ADTs, root aliases. | `Option`, `Result`, `Slice`, `char`, `try_get`, `move`, `take`, `assert`, `panic`, `Error`, `ErrorKind`, `CStr`, `CString`, `Library`, `Symbol`, `AtomicI64`, `Mutex`, `RwLock`, `Once`. Root `char` is an ASCII `u8` alias; root `Slice[T]` includes `len`, access, subslicing, subsequence search, compare, chunks, windows, split, copy helpers, stable/unstable partitioning, dedup variants, and direct algorithm wrappers. |
| `std::option` | Convenience methods for optional values. | `is_some`, `is_none`, `is_some_and`, `is_none_or`, `unwrap_or_else`, `map`, `and_then`, `filter`, `flatten`, `transpose`, `ok_or`. |
| `std::result` | Convenience methods for success/failure values. | `is_ok`, `is_err`, `is_ok_and`, `is_err_and`, `unwrap_or_else`, `ok`, `err`, `map_err`, `or`, `transpose`. |
| `std::io` | Byte-oriented process IO contracts and hooks. | `Reader`, `Writer`, `Seek`, `Stdin`, `Stdout`, `Stderr`, `Pipe`, `PipeReader`, `PipeWriter`, `Cursor`, `BufReader`, `BufWriter`, `stdin`, `stdout`, `stderr`, `pipe`, `cursor`, `buf_reader`, `buf_writer`, direct `Error` helpers `read_exact_result`, `copy_result`, `write_all_result`, `flush_result`, compatibility `read_exact`, `read_all`, `read_to_string`, `try_copy`, `copy`, `write_all`, `flush`, `write_bytes`, `read_line`. |
| `std::input` | Friendly stdin helpers. | `line`, `owned_line`, `read_byte`, `try_read_byte`. |
| `std::context` | Low-level runtime context access. | `argc`, `arg`, `thread_id`, startup `cwd`, startup `executable_path`, `has_arg`, `user_arg_count`, `is_main_thread`. |
| `std::test` | Executable unit-test helpers. | `Report`, `Bench`, `report`, `scratch`, `temp_file`, `temp_dir`, `bench`, `benchmark`, `check`, `equal`, `not_equal`, `matches_snapshot`, `golden_matches`, `check_snapshot`, pass/fail accessors, `ok`, `finish`, `require`. |
| `std::log` | Level-prefixed stderr diagnostics. | `Level`, `rank`, `name`, `enabled`, `write`, `message`, `trace`, `debug`, `info`, `warn`, `error`. |
| `std::error` | Shared recoverable error values. | `Kind`, `Error`, strict and fallible constructors, `from_errno`, `from_raw`, `kind`, `code`, `raw`, `is_kind`, `is_not_found`, `is_interrupted`, `is_retryable`, `name`, `message`. |
| `std::c` | C ABI boundary helpers. | `CStr`, `CString`, `Library`, `Symbol`, `from_string`, `from_ptr`, `from_slice_in`, `from_cstr_in`, `is_null`, `errno`, `error`, `open`, `main_program`, `symbol`, `function`, `close`, `last_error`, `lazy`, `now`, `local`, `global`. |
| `std::target` | Compiler-known target and platform facts. | `triple`, `arch`, `os`, `env`, `pointer_bits`, `uses_elf`, `uses_dwarf`, `syscall_abi`, Linux API-family predicates. |
| `std::env` | User-facing process argument, environment-variable, OS-string, and path-state helpers. | `arg_count`, `try_arg`, `try_arg_os`, `program_name`, `program_name_os`, `get`, `get_os`, `try_get`, `try_get_os`, `set`, `remove`, `current_dir`, `current_dir_os`, `current_dir_path`, `try_current_dir_path`, `set_current_dir`, `executable_path`, `executable_path_os`. |
| `std::process` | Current-process helpers and POSIX child-process control. | `id`, `uid`, `gid`, `exit`, `abort`, `success`, `failure`, `ExitCode`, typed `Signal`, direct `Error` helpers `fork_result`, `wait_status_result`, `wait_result`, raw compatibility `fork`, `wait`, `Arg`, `EnvVar`, `Command`, `Child`, `ChildStdin`/`ChildStdout`/`ChildStderr`, `ExitStatus`, `Output`, `TempFile`, `TempDir`, `arg`, `env_var`, `command`, `command_with_args`, `kill`, `kill_signal`, `terminate`, command `arg`/`args`/`env`/`env_var`/`current_dir`/`spawn`/`status`/`exit_status`/`output`/`output_in`/`exec`, current/executable path wrappers, temp file/dir constructors, status/output/child accessors. |
| `std::thread` | Function-pointer thread spawn/join, runtime ids, sleep/yield hints, and hosted parallelism. | `Thread`, `Builder`, `spawn`, `join`, `is_finished`, `yield_now`, `sleep`, `id`, `is_main`, `available_parallelism`, `is_join_error`. |
| `std::sync` | Small explicit synchronization primitives. | `Ordering`, `AtomicI64`, `AtomicBool`, `AtomicUsize`, `AtomicPtr`, `Mutex`, `RwLock`, `Once`, `OnceLock`, `Condvar`, `Barrier`, `Channel`, `Sender`, `Receiver`, atomic helpers, lock helpers, `call_once`, `channel`, `mpsc_channel`. |
| `std::cell` | Interior mutability and one-time initialization. | `Cell`, `RefCell`, `Ref`, `RefMut`, `OnceCell`, `Lazy`. |
| `std::rc` | Reference-counted shared ownership. | `Rc`, `Arc`, `Weak`, strong/weak counts, downgrade, upgrade, pointer equality. |
| `std::time` | Monotonic time, wall-clock time, sleep, deadlines, and UTC calendar values. | `Duration`, `Instant`, `SystemTime`, `Deadline`, `UtcDateTime`, strict and fallible duration constructors, strict and fallible Unix timestamp constructors, strict and fallible calendar helpers, `now`, `system_now`, `elapsed`, `sleep`, `timeout`, `timeout_after`, `deadline_at`. |
| `std::fs` | Byte-oriented filesystem handles. | `File`, `Dir`, `DirEntry`, `FileKind`, `Metadata`, `Permissions`, `exists`, `can_read`, `can_write`, `can_execute`, `permissions`, `metadata`, `try_metadata`, `symlink_metadata`, `try_symlink_metadata`, `try_file_type`, `is_file`, `is_dir`, `is_symlink`, `is_other`, `mode`, `try_mode`, `set_mode`, `set_permissions`, `canonicalize`, `try_canonicalize`, `remove`, `rename`, `hard_link`, `symbolic_link`, `read_link`, `try_read_link`, `ensure_file`, `create_dir`, `ensure_dir`, `create_dir_all`, `ensure_dir_all`, `remove_dir`, `remove_dir_all`, `try_read_dir`, `read_dir`, `try_read_dir_entries`, `read_dir_entries`, `try_open_dir`, `Dir::next`, `Dir::close`, `DirEntry::metadata`, `DirEntry::try_metadata`, `DirEntry::symlink_metadata`, `DirEntry::try_symlink_metadata`, `DirEntry::try_file_type`, `DirEntry::is_file`, `DirEntry::is_dir`, `DirEntry::is_symlink`, `DirEntry::is_other`, `open`, `try_open`, `create`, `try_create`, compatibility `open_read`/`open_write`/`open_append`, `read_byte`, `try_read_byte`, `write_byte`, `write_bytes`, `position`, `seek`, whole-file `read`, `try_read`, `write`, `try_write`, `append`, `try_append`, `truncate`, `copy`, `try_copy`, `read_to_string`, `try_read_to_string`, `close`. |
| `std::path` | Source lexical path manipulation. | `PathBytes`, `bytes`, `from_os`, method-style path-byte helpers, `is_separator`, `is_absolute`, `is_relative`, `trim_trailing_separators`, `components`, `file_name`, `parent`, `extension`, `stem`, `file_stem`, `has_file_name`, `has_extension`, `has_stem`, `has_file_stem`, `starts_with`, `strip_prefix`, `ends_with`, `strip_suffix`, `with_file_name_in`, `with_extension_in`, `join_in`, `normalize_in`. |
| `std::net` | Network address values, DNS lookup, and explicit socket handles. | `Ipv4Addr`, `Ipv6Addr`, `IpAddr`, `SocketAddr`, `TcpListener`, `TcpStream`, `UdpSocket`, `UnixListener`, `UnixStream`, `Shutdown`, `ToSocketAddrs`, address constructors/accessors, family/loopback/unspecified predicates, IPv4 DNS lookup plus `"host:port"` endpoint resolution with direct `Error` and raw compatibility results, host-port TCP connect helpers, IPv4 TCP bind/connect/accept/local-port/local-address/peer-address helpers, IPv4 UDP bind/local-address/send-byte/receive-byte helpers, Unix stream bind/connect/accept helpers, direct `Error` and raw compatibility socket construction/accept/connect results, descriptor views, nonblocking flags, TCP listener/UDP reuse-address helpers, TCP nodelay helpers, `std::time::Duration` timeouts with raw millisecond compatibility helpers, stream shutdown, explicit close, TCP/Unix `std::io` byte adapters, and TCP/Unix `read_exact`/`write_all` stream buffer helpers. |
| `std::mem` | Layout, raw pointer, byte memory, and hosted page-size operations. | `size_of`, `align_of`, `ptr_offset`, `ptr_add`, `ptr_load`, `ptr_store`, `copy_bytes`, `move_bytes`, `set_bytes`, `page_size`, `replace`, `swap`. |
| `std::zone` | Explicit allocation capability. | `create`, `alloc`, `alloc<T>`, `alloc_array<T>`, `new<T>`, `promote<T>`, `allocation_zone`, `metadata`, `from_zone`, `ZoneMetadata`, metadata handle allocation, `ZoneBacked`, `of`, `reset`, `destroy`. |
| `std::boxed` | Zone-backed single-value owner. | `Box[T]`, `new`, `get`, `try_get`, `set`, `take`, `try_take`, `copy_to`. |
| `std::string` | Zone-backed owned byte string and typed borrowed text-boundary views. | `String`, `Utf8`, `OsStr`, direct string-literal coercion to `Slice[u8]` / `Vec[u8]` / `[u8, N]` / `Utf8` / `OsStr` / `PathBytes` / `CStr`, `utf8`, `os_str`, `c_str`, `c_len`, `c_bytes`, `bytes`, `new`, `empty`, `from`, `copy`, `from_string`, `from_slice_in`, `join_in`, `push`, `try_get`, `try_pop`, `remove`, `try_remove`, `retain`, `append`, `append_byte`, `append_bytes`, `find_text`, `contains_text`, `split`, `chunks`, `windows`, `push_codepoint_in`, `try_utf8`, `is_utf8`, `codepoint_count`, `codepoint_at`, `equals_text`, `equals_text_ignore_case`, `trim`, `trimmed`, `parse_decimal`, `parse_signed_decimal`, `parse_decimal_prefix`, `parse_signed_decimal_prefix`, `as_slice`. `c_str` returns the shared `std::c::CStr` type. |
| `std::ascii` | Source-only ASCII byte and slice helpers. | `is_digit`, `is_printable`, `equals_ignore_case`, `index_of_ignore_case`, `trim`, `parse_decimal`, `parse_decimal_prefix`, signed decimal parsers, and overflow-checked `i64` parser policy. |
| `std::parse` | Whole-input value parsers over byte slices. | `Parse`, `parse<T>`, `parse_or<T>`, `is_parse<T>`, signed `integer`/`integer_radix`, unsigned `unsigned`/`unsigned_radix`, radix wrappers for hex/binary/octal signed integers, `boolean`, `is_float`, `float_or`, `float`. |
| `std::encoding` | Text validation, UTF-8 scalar helpers, and byte codecs. | `is_ascii`, `is_unicode_scalar`, `utf8_count`, `is_utf8`, `utf8_at`, `utf8_next_index`, `encode_utf8_in`, `try_encode_utf8_in`, `utf16_count`, `is_utf16`, `encode_hex_in`, `decode_hex_in`, `try_decode_hex_in`, `encode_base64_in`, `decode_base64_in`, `try_decode_base64_in`. |
| `std::vec` | Zone-backed growable sequence. | `Vec[T]`, `new<T>`, owning-zone `push`, `insert`, `reserve`, `reserve_extra`, `try_reserve`, `extend`, `extend_from_slice`, `extend_iter`, `append`, `resize`, `resize_with`, explicit `_in` compatibility methods, `try_get`, `try_remove`, `swap_remove`, `retain`, `dedup`, `dedup_by`, `dedup_by_key`, `fill`, `fill_range`, `copy_from`, `copy_within`, `partition`, `stable_partition`, `drain`, `drain_range`, `split_off`, `splice`, `slice`, `split_at`, `find`, `contains_slice`, `compare`, `chunks`, `windows`, `split`, `reverse`, `reverse_range`, `rotate_left`, `rotate_right`, `rotate_range`, introsort-backed `sort`, merge-sort-backed `stable_sort`, explicit-zone stable sort, `try_stable_sort`, `is_sorted`, `binary_search`, `lower_bound`, `upper_bound`, `equal_range`, `partition_point`, `min`, `max`, `as_slice`, `iter`, `iter_mut`. |
| `std::hash` | Deterministic non-cryptographic hashing. | `Hasher`, `Hash[T]`, `new`, `reset`, `finish`, `write`, `value`, `pair`, `combine`, `bytes`, fixed-width integer and bool write helpers, and `Slice[u8]` hashing. |
| `std::random` | OS entropy and deterministic non-cryptographic PRNG helpers. | `Prng`, `entropy`, `entropy_result`, `fill`, `fill_result`, `seed`, `from_entropy`, `from_entropy_result`, `seed_from_os`, `seed_from_os_result`, `next`, `boolean`, unbiased `below`/`try_below`, unbiased `range`/`try_range`, `float`, `fill_from`, `shuffle`. |
| `std::collections` | Source collection handles beyond sequences. | Linear `Set[T]` with insertion-order access and set-relationship predicates, `Deque[T]`, `RingBuffer[T]`, `LinkedList[T]`, `BinaryHeap[T]`, `PriorityQueue[T]`, target-zone copy across collection families, hash-table `HashMap[K,V]`/`HashSet[T]` with natural `contains_key`/`contains_value`/`get_or` map lookup, entry update handles, set representative lookup, entry iteration, and live-bucket set relationships, red-black-tree `TreeMap[K,V]`/`TreeSet[T]` with natural `contains_key`/`contains_value`/`get_or` map lookup, entry update handles, set representative lookup, sorted entry iteration, ordered key/value/entry boundary access, lower/upper bound lookup, key-value removal, direct red-black deletion, and ordered-set relationships, explicit hash/comparator constructors, lookup, insertion, replacement, removal, reserve, clear, retain filtering for linear/hash collections, FIFO/linked/heap iteration where applicable, live-bucket hash iteration, and sorted tree iteration. |
| `std::iter` | Range, iterator traits, lazy adapters, and eager consumers. | `range`, `range_inclusive`, finite `empty`/`once` sources, generator-backed `repeat_with`, `Iterator`, `IntoIterator`, `map`, `filter`, `take`, `skip`, `enumerate`, `zip`, `count`, `count_if`, `nth`, `last`, `find_if`, `position`, `any`, `all`, `fold`, `reduce`, `collect`, plus `SliceIter[T]`/`SliceIterMut[T]` and `std::vec::Iter[T]` sources from Slice/Vec. |
| `std::fmt` | Formatting traits plus explicit-zone, writer, and stdout formatting helpers. Root `Display`/`Debug` re-export these traits. | `Display::format_in`, `Debug::debug_in`, `FormatSpec`, `decimal`, `hex`, `binary`, `octal`, strict `with_width`/`with_precision`, fallible `try_with_width`/`try_with_precision`, `left`, `right`, `center`, `uppercase`, `alternate`, `unsigned_in`, `integer_in`, `boolean_in`, `float_in`, `text_in`, `debug_text_in`, `debug_value`, `write_unsigned`, `write_integer`, `write_boolean`, `write_text`, `write_value`, `write_debug`, `print_value`, `println_value`, `print_debug`, `println_debug`. |
| `std::cmp` | Comparison traits and helpers. | `Eq`, `PartialEq`, `Ord`, `PartialOrd`, primitive scalar impls, `Ordering`, `compare`, `compare_by`, `then_compare`, `then_compare_by`, `min`, `max`, `clamp`, `is_between`, comparator-based `*_by` value helpers. |
| `std::algo` | Source algorithms over borrowed slices. | Introsort-backed `sort`/`sort_by`, merge-sort-backed `stable_sort`/`stable_sort_by`, `stable_sort_in`, `stable_sort_by_in`, `try_stable_sort`, `try_stable_sort_by`, `binary_search`, `binary_search_by`, `lower_bound`, `lower_bound_by`, `upper_bound`, `upper_bound_by`, `equal_range`, `equal_range_by`, `partition_point`, `is_sorted`, `is_sorted_by`, `reverse`, `reverse_range`, `rotate_left`, `rotate_right`, `rotate_range`, `partition`, `stable_partition`, `min`, `min_by`, `max`, `max_by`, `clamp`, `clamp_by`, `swap`, `fill`, `fill_range`, `copy`, `copy_within`, `dedup`, `dedup_by`, `dedup_by_key`, plus receiver-form `Slice[T]`/`Vec[T]` wrappers for common call sites. |
| `std::convert` | Explicit conversion trait names and helpers. | `From`, `Into`, `TryFrom`, `TryInto`, `identity`, `from`, `into`, `try_from`, `try_into`. |
| `std::math` | Source-only numeric helpers. | `abs`, `sign`, sign/parity predicates, checked add/sub/mul/div/rem/neg/abs, wrapping/overflowing add/sub/mul, saturating add/sub/mul/div/neg/abs, `pow`, floor/ceil division, `gcd`, `lcm`. |
| `std::bits` | Source-only bit-mask, rotation, power-of-two, low-mask, checked/wrapping alignment, byte-swap, population-count, and zero/one-run bit-scan helpers. | `is_set`, `set`, `rotate_left`, `bit_width`, `low_mask`, `align_up`, `checked_align_up`, `wrapping_align_up`, `byte_swap`, `population_count`, `leading_ones`. |

## Allocation Rules

Anything that allocates takes a `ref mut Zone` or returns a handle tied to a
zone. Methods with an `_in` suffix take an explicit zone for growth or copying;
methods with a `_to` suffix copy a derived value into a target zone.
Heap-backed handles recover `ZoneMetadata` from their own backing allocation
headers, so natural growth methods such as `vec.push(value)`,
`vec.reserve(capacity)`, and `String` byte appends allocate through the
recovered runtime zone after construction. For tracked local
`std::collections` handles, Ari can infer the same source zone for common
mutating methods on tracked locals.
`map.insert(key, value)`,
`set.insert(value)`, `deque.push_back(value)`, `list.push_front(value)`, and
`heap.push(value)` are natural forms over handles whose constructor established
zone provenance. Fixed-capacity `RingBuffer` allocates only at construction.
Collection handles keep the same zone provenance as their backing storage, and
collection iterator cursors preserve that provenance.

Use `zone::destroy(zone)` when a manually created zone is no longer needed.
Pointers, strings, vectors, boxes, and slices derived from that zone become
invalid after `reset` or `destroy`, and sema rejects later use.

## Value Movement Rules

Sequence helpers make their value contract explicit. Current `std::algo` and
`std::vec` copy/reorder helpers are for copyable scalar values and plain
Ari-layout aggregates. Ownership- or borrow-valued elements are not silently
copied through generic algorithms. Removed live elements in owning containers
are dropped through normal `Drop` lowering, while borrowed `Slice[T]` compaction
helpers return a logical prefix length because they do not own the suffix.

Use [Value Movement Contracts](value-contracts.md) when adding or reviewing
helpers such as `copy`, `fill`, `resize`, `sort`, `dedup`, `partition`, and
`drain`.

## Source Versus Compiler Hooks

Most helper methods are plain Ari source. Compiler hooks remain for primitives
that need backend or checker knowledge:

- `extern "ari"` IO, panic, environment, process, thread, sync, random
  entropy, string allocation, and zone runtime hooks.
- layout queries, typed pointer operations, and byte memory intrinsics in
  `std::mem`.
- formatting macros, because they inspect literal format strings.
- provenance checks for zone-backed handles and raw pointers.

`std::ascii` is an example of the preferred path: it is ordinary Ari source
because byte classification, printable/control predicates, case conversion,
borrowed-slice case-insensitive comparison/search, trimming, whole-slice
integer parsing, and prefix integer parsing need no compiler knowledge.

`std::parse` and `std::encoding` continue that source-first pattern.
`std::parse` keeps whole-input signed/unsigned integer, bool, typed
`Parse` dispatch, and decimal float parsing out of individual call sites.
`std::encoding` validates ASCII/UTF-8/UTF-16 and
encodes or decodes hex/base64 into caller-provided zones. Fallible hex/base64
decoders now return `Option[String]`; richer `Result[String, E]` error
payloads and `Option[f64]` still wait on the broader error/float payload
roadmap, while `parse[f64]`/`parse_or[f64]` provide strict and fallback
typed float parsing today.

`std::random` has OS-backed hooks for `entropy()` and `fill(values)`, because
seed material must come from the host and byte slices should be filled without
round-tripping through one-word entropy calls. Result-returning entropy helpers
map hosted failures into `std::error::Error` while keeping strict hard-fail
helpers available. The deterministic `Prng`, boolean helper, bounded integer
helpers, unit float helper, byte filling from a seeded PRNG, and generic shuffle
are source Ari. Cryptographic streams remain future work.

`std::test`, `std::log`, and `std::error` are also source-first.
`std::test::Report` aggregates checks, generic `equal`/`not_equal` stay
naturally named, `scratch` simply creates an explicit `Zone` for tests, and
small temp-path, snapshot/golden comparison, and benchmark timing helpers cover
common test fixtures. The compiler-generated runner also writes progress and
failure markers to `stderr` so aborting tests leave the running test name in
the output.
`std::log` writes level-prefixed diagnostic lines to `stderr` through
`std::io::Stderr`. `std::error`
defines compact recoverable error values, stable error categories, POSIX errno
mapping, fallible boundary validation, direct `Result[T, Error]` bridges, and
raw scalar compatibility conversion for runtime/FFI boundaries.
Per-test panic/log capture, compiler-owned source text and filename storage,
doctests, structured logging, stack traces, backtraces, owned error messages,
and richer structured error fields remain runtime, driver, and compiler roadmap
work.

`std::c` is source Ari around compiler-known C ABI primitives. The compiler
owns `c_int`, `c_char`, `c_void`, `size_t`, and target-sensitive C width
mapping, while `std::c` owns readable boundary helpers such as `CStr`,
zone-backed `CString`, POSIX `errno`, and hosted dynamic loading with
`Library` and `Symbol`, including explicit function-pointer extraction for
loaded dynamic function symbols. It stays narrow on purpose: raw descriptor,
syscall, signal, mmap, and socket APIs should grow in `std::os`, `std::fs`,
`std::process`, or `std::net` instead of turning `std::c` into a full libc
binding.

`std::string::String` follows the same direction where it can. Its allocation
constructors and runtime copy hooks still depend on compiler-known zone/string
primitives, but byte access, empty-safe `try_*` accessors, byte search,
comparison, typed `Utf8`/`OsStr` borrowed views, `std::c::CStr` convenience
construction, ASCII
case-insensitive comparison/search, UTF-8 validation and scalar helpers, ASCII
trim views, owned trim copies, and whole/prefix ASCII parsing are plain source
methods.

`Slice[T]`, `std::vec::Vec[T]`, and byte-oriented `std::string::String` share
the preferred collection vocabulary where it fits: `is_empty` for length
metadata, asserting `first`/`last`/`get` for programmer errors, and
`try_first`/`try_last`/`try_get` for ordinary absence handled through
`Option`. `Slice[T]` adds mutable element borrows, iterator and mutable cursor
entry points, endpoint splitting, predicate search/count helpers, and
prefix/suffix stripping. `Slice[T]` and
`std::vec::Vec[T]` also share borrowed-view operations such as `slice`,
`split_at`, subsequence `find`, `contains_slice`, lexicographic `compare`,
lazy `chunks`, lazy `windows`, delimiter `split`, in-place reordering,
copying/filling, half-open range mutation, stable/unstable partitioning, dedup variants, sorting,
ordered search, equal-range lookup, partition-point lookup, and min/max
wrappers. `Vec[T]` returns views over its
live element storage, and `String` exposes the same byte-view shape plus
allocator-backed `join_in`.

`std::bits` follows the same rule for current `u64` mask, rotation,
power-of-two, low-mask, strict/checked/wrapping alignment, byte-swap,
population-count, and source-loop zero/one-run bit-scan helpers. Future
intrinsic-backed implementations may need compiler support, but the public
edge-case behavior should stay source-defined.

`std::cmp` is also source-first. Its current helpers build on primitive
`Eq`/`PartialEq`/`Ord`/`PartialOrd` impls and the minimal `Ord[T]::lt` method
so generic code can compare, chain lexicographic ordering, select, clamp, or
range-check values without compiler-known comparison intrinsics. Comparator
forms such as `compare_by`, `min_by`, and `is_between_by` cover one-off
ordering policies without forcing a type-wide `Ord` impl.

`std::convert` follows the same source-first path for generic conversion
helpers. `identity`, `from`, `into`, `try_from`, and `try_into` are plain Ari
functions over the module's trait surface; fallible conversion returns
`Option` when the caller only needs success or failure.

`std::context` keeps the same split: `argc`, `arg`, `thread_id`, startup `cwd`,
and startup `executable_path` are runtime hooks because they read the host
runtime context, while `has_arg`, `user_arg_count`, `has_user_args`,
`is_main_thread`, and path view adapters are ordinary source helpers that
document Ari's valid-index, main-thread, and startup-path policies in one
reusable place.

`std::target` is compiler-backed because the selected target triple,
architecture, libc/environment, object format, debug format, pointer width, and
ABI families are known to the compiler and LLVM driver, not to portable source
code. The readable predicates around those hooks are source Ari. Linux
API-family helpers describe target availability; live kernel probing and
fallible descriptor creation belong in future `std::os` wrappers.

`std::os` now owns the first raw OS value boundary with a non-owning `Fd`
descriptor view, an owning `OwnedFd` wrapper, and an owning `Pipe` pair.
`std::os::stdin()`,
`stdout()`, `stderr()`, `invalid()`, and `fd(raw)` make descriptor identity
explicit, while `std::fs::File.descriptor()` bridges file handles into that
view without transferring ownership. `OwnedFd::from_raw(raw)` is the explicit
ownership handoff for exactly-one-close responsibility, and
`OwnedFd::try_clone()` creates an independent owner around a duplicated
descriptor. `OwnedFd::close_on_exec()` and `set_close_on_exec(enabled)` cover
the first descriptor inheritance flag without making callers pass raw `fcntl`
constants. `OwnedFd::is_nonblocking()` and `set_nonblocking(enabled)` cover
blocking behavior with the same owned-descriptor shape. `std::os::pipe()`
returns `Option[Pipe]`, where each successful pair owns its read and write
ends until they are taken or closed. Raw syscalls, readiness APIs, signals, and
memory mapping still wait for duplicate-with-flags policy and richer error
results.

`std::net` builds on that owned-descriptor work. Address values and socket
address helpers stay source Ari, while hosted runtime hooks provide IPv4 TCP,
IPv4 UDP, Unix stream sockets, one-address IPv4 lookup, host-port endpoint
resolution, direct `Error` result
helpers with raw compatibility variants, millisecond socket timeouts, and stream shutdown. TCP and Unix streams implement the shared
`std::io::Reader`/`Writer` traits so higher-level byte helpers do not need
socket-specific overloads, and TCP/UDP handles can report their local IPv4
socket address. TCP listeners and UDP sockets expose reuse-address policy, and
TCP streams can report the connected peer IPv4 address and nodelay policy.
The current network layer still keeps IPv6 socket handles,
buffer-oriented datagrams, UDP source address queries, remaining socket options, and
timeout-specific error results on the roadmap.

`std::env` wraps the context hooks with the names application code should use
and adds `Option`-based argument access through `try_arg` and `program_name`.
`arg_os`, `try_arg_os`, and `program_name_os` expose the same startup data as
`std::string::OsStr` views. Environment variables and process-local path state
use small runtime-backed hooks for `get`, `has`, `set`, `remove`,
`current_dir`, `set_current_dir`, and `executable_path`, with source
`try_get`, `try_current_dir`, and `try_executable_path` helpers keeping
ordinary absence in `Option[string]`. Use `get_os`, `try_get_os`,
`current_dir_os`, `current_dir_path`, `try_current_dir_path`, and
`executable_path_os` when OS strings or lexical path bytes should stay distinct
from ordinary text.

`std::process` starts with a runtime-backed process surface: `id` reads the
host process id, `uid`/`gid` read current user and group identity, `exit`
terminates with an explicit status, `abort` terminates abnormally, and the
status/root helpers are source Ari. POSIX child-process slices now include
`fork_result`/`wait_status_result`/`wait_result` direct `Error` helpers, raw
compatibility `fork`/`wait`, source branch/error predicates, and a `Command`
builder for argument passing, environment setup, working-directory setup,
`arg`, `env_var`, `spawn`, `status`, `exit_status`, `output`, `output_in`,
`exec`, `Child` handles, typed `ExitStatus`/`ExitCode` values, typed
`Signal`, `Output` handles, stdout/stderr capture for small outputs,
child-stream endpoint aliases, current/executable path wrappers, temp file/dir
constructors, and `kill`/`kill_signal`. Large-stream readiness, stdin
redirection, richer platform status fields, and Windows process mapping remain
roadmap work.

`std::thread` is the first thread slice. `spawn`, `join`, `is_finished`,
`yield_now`, and `available_parallelism` are runtime-backed because they call
the host threading or process APIs and install Ari's per-thread runtime id
before source code runs. `sleep` delegates to `std::time`, while `id`,
`is_main`, `is_join_error`, the `Builder` accessors, and most `Thread` methods
are source helpers. `Builder` records a requested name and stack size, but
runtime application of those options remains platform work. `ThreadLocal[T]`
is an explicit zone-backed handle for current-thread values; compiler-level
`thread_local` statics, capturing closures, richer result/status values, and
send/share typing remain richer thread-policy work.

`std::sync` now starts with `AtomicI64`, `AtomicBool`, `AtomicUsize`,
`AtomicPtr[T]`, source `Mutex`, `RwLock`, `Once`, `OnceLock`, `Condvar`,
`Barrier`, and a single-slot MPSC channel shape. `std::cell` adds local
interior mutability through `Cell`, runtime-checked `RefCell`, and zone-backed
`OnceCell`/`Lazy` one-time initialization. `std::rc` adds explicit `Rc`,
`Arc`, and `Weak` shared ownership handles. Atomic method names are the names
developers expect:
`load`, `store`, `swap`, `fetch_add`, and `compare_exchange`. The runtime
hooks lower directly to LLVM atomic operations. Default methods are
sequentially consistent, while explicit-order methods lower Ari `Ordering`
values to the matching LLVM ordering for load/store/RMW/compare-exchange.
`Mutex` and `RwLock` are primitive spin/yield locks without protected payloads
or guards, `Condvar` and `Barrier` are source coordination primitives, and
channels carry only a shared state pointer rather than redundant zone handles.
`Arc` uses an atomic control block, but send/share trait policy,
value-protecting locks, semaphores, futex-backed blocking locks, timeout waits,
and non-LLVM target atomic policy remain future work.

`std::time` follows the same OS-facing pattern. `monotonic_nanos`,
`unix_nanos`, and `sleep_nanos` are runtime-backed because they call the host
clock and sleep APIs, while `Duration`, `Instant`, `SystemTime`, `Deadline`,
`UtcDateTime`, and the constructor/elapsed/timeout/calendar helpers are
ordinary Ari source. Use strict `Duration` constructors for constants and
`try_*` constructors for user-provided values; the Unix timestamp constructors
and month-length helpers follow the same strict/fallible split. Use `Instant`
and `Deadline` for elapsed time and timeout policy; use `SystemTime` only for
wall-clock timestamps and convert it to UTC with `to_utc()` when a calendar
value is needed.

`std::fs` is the first filesystem slice. `exists`, access-style permission
checks, `remove`, mode-string and `OpenOptions` open helpers, direct
`Result[..., Error]` open/mutation/byte-count helpers, raw compatibility
bridges, close, single-byte read/write, and file
cursor `position`/`seek` are runtime-backed because they call host
file-descriptor APIs. `permissions`,
`try_open`, `open_result`, `open_options`, compatibility `try_open_*` wrappers, `rename`, `rename_result`, `create_dir`,
`ensure_dir`, recursive `create_dir_all`/`ensure_dir_all`, `remove_dir`, `remove_dir_all`,
non-truncating `ensure_file`, `create`/`try_create`,
`write_bytes`, whole-file `read`, `write`, `write_result`,
`try_read`, `try_write`, `append`, `append_result`, `try_append`, `truncate`, `copy`,
`copy_result`, byte-counting `try_copy`, byte-string `read_to_string`, fallible
`try_read_to_string`, `try_read_dir`/`read_dir`,
`try_read_dir_entries`/`read_dir_entries`,
`try_open_dir`/`Dir::next`/`Dir::close`,
target-following `metadata`/`try_metadata`, no-follow
`symlink_metadata`/`try_symlink_metadata`, `try_file_type`, direct path-kind predicates,
permission `mode`/`try_mode`/`set_mode`/`set_permissions`,
`canonicalize`/`try_canonicalize`, `read_link`/`try_read_link`, and the `File`,
`Dir`, `DirEntry`, `Metadata`, and `Permissions` methods are ordinary Ari
source or thin runtime hooks over the raw OS boundary. Directory entry values
now expose names, joined child paths, and lazy metadata/file-kind predicates.
Metadata values expose access, modification, and POSIX status-change times as
`SystemTime`; richer per-entry errors remain future work. Each handle is a
visible value
today and should become a stronger owned resource when OS resource ownership is
modeled by the language.

`std::path` is source-only and deliberately lexical. It works over borrowed
`Slice[u8]` values, typed `PathBytes` views, borrowed component iterators, and
zone-backed `String` outputs, so tools can split, join, iterate, check
component-aware prefixes/suffixes, strip path affixes, and lightly normalize
paths without touching the filesystem. The current policy is POSIX-style `/`
separators only; platform-specific path forms remain future work.

`std::net` starts with deterministic value types: IPv4, IPv6, generic IP, and
socket addresses. Address components support strict access for known-good
indexes and fallible access for parsed indexes. The hosted socket layer now
adds one-address IPv4 lookup, IPv4 `TcpListener`/`TcpStream` handles, IPv4
`UdpSocket` single-byte datagrams, Unix stream sockets, ephemeral local-port
and local-address lookup where it applies, descriptor views, nonblocking
flags, `std::time::Duration` timeout setters with raw millisecond
compatibility helpers, stream shutdown, explicit close, and TCP/Unix
`std::io::Reader`/`Writer` byte adapters plus method-style
`read_exact`/`write_all` stream buffer helpers. IPv6 socket handles,
buffer-oriented UDP datagrams, broad options, UDP source address helpers, and
timeout-specific error results remain roadmap work.

`std::collections` is source Ari over typed zone allocation. `Set[T]` remains a
small, insertion-order, linear set with iterator support and stable in-place
`retain` filtering. `Deque[T]` and
`RingBuffer[T]` cover growable and bounded queue shapes, `LinkedList[T]` uses
zone-backed reusable node slots, and `BinaryHeap[T]`/`PriorityQueue[T]` cover
highest-priority removal. `HashMap[K,V]` and `HashSet[T]` are real
open-addressed hash tables with tombstones, live-bucket iterators, and
in-place `retain` that drops rejected entries while preserving probe
correctness.
`TreeMap[K,V]` and `TreeSet[T]` are red-black trees with sorted successor
iterators for keys, values, entries, and set values, plus first/last and
lower/upper-bound access in comparator order. Hash and tree sets expose
`get`/`try_get` so callers can recover the stored representative for an equal
value without scanning. Map entry iterators, boundary entry helpers, and map
bound helpers yield `MapEntry[K,V]` values so callers do not need tuple
convention knowledge to read `.key`/`.value` or `key()`/`value()`.
Map types expose `contains_key` as the preferred key-membership spelling while
keeping `contains` compatible, `contains_value` for value membership scans, and
`get_or` for ordinary fallback lookups. `HashMap.entry(key)` and
`TreeMap.entry(key)` return update handles with `or_insert`,
`or_insert_with`, `or_default`, `and_modify`, direct `insert`, `insert_entry`,
direct `remove`, `key`, `value`, and `value_mut`, and `remove_entry(key)`
returns copied `MapEntry[K,V]` key-value pairs for removal cases that need both
fields. Update-entry handles keep only the backing map pointer and key; growth
recovers the zone from allocation metadata.
Tree removal compacts live nodes and rebuilds links in place, so the public
API does not need a zone argument. Hash, tree, and heap constructors take
explicit policy functions until trait dispatch is strong enough for fully
trait-driven containers. The compiler only recognizes the handle shapes so
zone reset/destroy invalidation and same-zone growth diagnostics stay as
strong as `std::vec::Vec[T]`.

`std::zone` keeps allocation visible. Raw byte allocation and lifecycle hooks
are runtime-backed, while `alloc_array<T>` is source Ari that packages the
common "count times `size_of<T>()` at `align_of<T>()`" pattern for library
authors. `allocation_zone(data)` is the raw allocation-header reader, while
`metadata(data)` wraps that handle in `ZoneMetadata`. `from_zone(ref mut zone)`
captures metadata from an explicit zone capability, and `ZoneMetadata` can
allocate directly through that recovered runtime handle. `ZoneBacked` plus
`zone::of(ref value)`/`value.zone()` give higher-level handles a standard way
to expose the same typed metadata from a real backing allocation, including
zone-backed collection and map-entry handles.

`std::input` follows that pattern for stdin. `read_byte`, `line`, and
`owned_line` are runtime hooks, while `try_read_byte` is source Ari that turns
the raw `-1` EOF sentinel into `Option[u8]`.

`std::io` keeps raw process IO visible and adds a small source trait layer.
Scalar and line operations are runtime hooks. `write_bytes`,
`read_exact_result`, `copy_result`, `write_all_result`, `flush_result`,
`read_exact`, `read_all`, `read_to_string`, `try_copy`, `copy`, `write_all`, `flush`,
`Stdin`, `Stdout`, `Stderr`, `Cursor`, `BufReader`, and `BufWriter` are source Ari over
`Slice[u8]`, raw pointers, explicit caller-provided buffers, and the process
stream hooks. `PipeReader` and
`PipeWriter` adapt `std::os::Pipe` into the same traits. `std::fs::File`
also adapts to `Reader`, `Writer`, and `Seek` so filesystem handles can use
the same generic helpers. The next IO roadmap items are zone-owning buffered
constructors and resource flush/drop rules.

When adding new library code, first ask whether it can be written in Ari source
using existing modules. If yes, keep it in `lib/std/`. Add compiler support
only for a primitive the source language cannot express.
