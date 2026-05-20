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
| `std` | Prelude root, shared ADTs, root aliases. | `Option`, `Result`, `Slice`, `char`, `try_get`, `move`, `take`, `assert`, `panic`, `Error`, `ErrorKind`, `CStr`, `CString`, `Library`, `Symbol`, `AtomicI64`, `Mutex`, `RwLock`, `Once`. Root `char` is an ASCII `u8` alias; root `Slice[T]` includes access, subslicing, subsequence search, compare, chunks, windows, split, and copy helpers. |
| `std::option` | Convenience methods for optional values. | `is_some`, `is_none`, `is_some_and`, `is_none_or`, `unwrap_or_else`, `map`, `and_then`, `filter`, `flatten`, `transpose`, `ok_or`. |
| `std::result` | Convenience methods for success/failure values. | `is_ok`, `is_err`, `is_ok_and`, `is_err_and`, `unwrap_or_else`, `ok`, `err`, `map_err`, `or`, `transpose`. |
| `std::io` | Byte-oriented process IO contracts and hooks. | `Reader`, `Writer`, `Seek`, `Stdin`, `Stdout`, `Stderr`, `Cursor`, `BufReader`, `BufWriter`, `stdin`, `stdout`, `stderr`, `cursor`, `buf_reader`, `buf_writer`, `read_exact`, `write_all`, `flush`, `write_bytes`, `read_line`. |
| `std::input` | Friendly stdin helpers. | `line`, `owned_line`, `read_byte`, `try_read_byte`. |
| `std::context` | Low-level runtime context access. | `argc`, `arg`, `thread_id`, startup `cwd`, startup `executable_path`, `has_arg`, `user_arg_count`, `is_main_thread`. |
| `std::test` | Executable unit-test helpers. | `Report`, `report`, `scratch`, `check`, `equal`, `not_equal`, `passed`, `failed`, `ok`, `finish`, `require`. |
| `std::log` | Level-prefixed stderr diagnostics. | `Level`, `rank`, `name`, `enabled`, `write`, `message`, `trace`, `debug`, `info`, `warn`, `error`. |
| `std::error` | Shared recoverable error values. | `Kind`, `Error`, `new`, `with_code`, `from_errno`, `from_raw`, `kind`, `code`, `raw`, `is_kind`, `is_not_found`, `is_interrupted`, `is_retryable`, `name`, `message`. |
| `std::c` | C ABI boundary helpers. | `CStr`, `CString`, `Library`, `Symbol`, `from_string`, `from_ptr`, `from_slice_in`, `from_cstr_in`, `is_null`, `errno`, `error`, `open`, `main_program`, `symbol`, `function`, `close`, `last_error`, `lazy`, `now`, `local`, `global`. |
| `std::target` | Compiler-known target and platform facts. | `triple`, `arch`, `os`, `env`, `pointer_bits`, `uses_elf`, `uses_dwarf`, `syscall_abi`, Linux API-family predicates. |
| `std::env` | User-facing process argument, environment-variable, OS-string, and path-state helpers. | `arg_count`, `try_arg`, `try_arg_os`, `program_name`, `program_name_os`, `get`, `get_os`, `try_get`, `try_get_os`, `set`, `remove`, `current_dir`, `current_dir_os`, `current_dir_path`, `try_current_dir_path`, `set_current_dir`, `executable_path`, `executable_path_os`. |
| `std::process` | Current-process helpers and POSIX child-process control. | `id`, `uid`, `gid`, `exit`, `abort`, `success`, `failure`, `is_success`, `is_failure`, `is_root`, `fork`, `wait`, `is_child`, `is_parent`, `is_fork_error`, `is_wait_error`. |
| `std::thread` | Function-pointer thread spawn/join, runtime ids, sleep/yield hints, and hosted parallelism. | `Thread`, `spawn`, `join`, `yield_now`, `sleep`, `id`, `is_main`, `available_parallelism`, `is_join_error`. |
| `std::sync` | Small explicit synchronization primitives. | `AtomicI64`, `Mutex`, `RwLock`, `Once`, atomic `load`/`store`/`swap`/`fetch_add`/`compare_exchange`, mutex helpers, rwlock helpers, `call_once`. |
| `std::time` | Monotonic time, wall-clock time, sleep, deadlines, and UTC calendar values. | `Duration`, `Instant`, `SystemTime`, `Deadline`, `UtcDateTime`, `nanoseconds`, `milliseconds`, `seconds`, `now`, `system_now`, `system_from_unix`, `utc_from_unix`, `elapsed`, `sleep`, `timeout`, `timeout_after`, `deadline_at`. |
| `std::fs` | Byte-oriented filesystem handles. | `File`, `FileKind`, `Metadata`, `Permissions`, `exists`, `can_read`, `can_write`, `can_execute`, `permissions`, `metadata`, `try_metadata`, `mode`, `try_mode`, `set_mode`, `set_permissions`, `canonicalize`, `try_canonicalize`, `remove`, `rename`, `hard_link`, `symbolic_link`, `create_dir`, `remove_dir`, `open`, `try_open`, `create`, `try_create`, compatibility `open_read`/`open_write`/`open_append`, `read_byte`, `write_byte`, `write_bytes`, whole-file `read`, `try_read`, `write`, `try_write`, `append`, `try_append`, `truncate`, `copy`, `try_copy`, `read_to_string`, `try_read_to_string`, `close`. |
| `std::path` | Source lexical path manipulation. | `PathBytes`, `bytes`, `from_os`, method-style path-byte helpers, `is_separator`, `is_absolute`, `is_relative`, `trim_trailing_separators`, `components`, `file_name`, `parent`, `extension`, `stem`, `join_in`, `normalize_in`. |
| `std::net` | Source network address values. | `Ipv4Addr`, `Ipv6Addr`, `IpAddr`, `SocketAddr`, `ipv4`, `ipv6`, `socket_addr`, `localhost`, family/loopback/unspecified predicates, port helpers. |
| `std::mem` | Layout, raw pointer, byte memory, and hosted page-size operations. | `size_of`, `align_of`, `ptr_offset`, `ptr_add`, `ptr_load`, `ptr_store`, `copy_bytes`, `move_bytes`, `set_bytes`, `page_size`, `replace`, `swap`. |
| `std::zone` | Explicit allocation capability. | `create`, `alloc`, `alloc<T>`, `alloc_array<T>`, `new<T>`, `promote<T>`, `reset`, `destroy`. |
| `std::boxed` | Zone-backed single-value owner. | `Box[T]`, `new`, `get`, `set`, `take`, `try_take`, `copy_to`. |
| `std::string` | Zone-backed owned byte string and typed borrowed text-boundary views. | `String`, `Utf8`, `OsStr`, direct string-literal coercion to `Slice[u8]` / `Vec[u8]` / `[u8, N]` / `Utf8` / `OsStr` / `PathBytes` / `CStr`, `utf8`, `os_str`, `c_str`, `c_len`, `c_bytes`, `bytes`, `new`, `empty`, `from`, `copy`, `from_string`, `from_slice_in`, `join_in`, `push`, `try_get`, `try_pop`, `append`, `append_byte`, `append_bytes`, `find_text`, `contains_text`, `split`, `chunks`, `windows`, `push_codepoint_in`, `try_utf8`, `is_utf8`, `codepoint_count`, `codepoint_at`, `equals_text`, `equals_text_ignore_case`, `trim`, `trimmed`, `parse_decimal`, `parse_decimal_prefix`, `as_slice`. `c_str` returns the shared `std::c::CStr` type. |
| `std::ascii` | Source-only ASCII byte and slice helpers. | `is_digit`, `is_printable`, `equals_ignore_case`, `index_of_ignore_case`, `trim`, `parse_decimal`, `parse_decimal_prefix`. |
| `std::parse` | Whole-input value parsers over byte slices. | `integer`, `boolean`, `is_float`, `float_or`, `float`. |
| `std::encoding` | Text validation, UTF-8 scalar helpers, and byte codecs. | `is_ascii`, `is_unicode_scalar`, `utf8_count`, `is_utf8`, `utf8_at`, `utf8_next_index`, `encode_utf8_in`, `try_encode_utf8_in`, `utf16_count`, `is_utf16`, `encode_hex_in`, `decode_hex_in`, `try_decode_hex_in`, `encode_base64_in`, `decode_base64_in`, `try_decode_base64_in`. |
| `std::vec` | Zone-backed growable sequence. | `Vec[T]`, `new<T>`, `push`, `push_in`, `try_get`, `slice`, `split_at`, `find`, `contains_slice`, `compare`, `chunks`, `windows`, `split`, `as_slice`, `iter`. |
| `std::hash` | Deterministic non-cryptographic hashing. | `Hasher`, `Hash[T]`, `new`, `reset`, `finish`, `write`, `value`, `bytes`, primitive write helpers. |
| `std::random` | OS entropy and deterministic non-cryptographic PRNG helpers. | `Prng`, `entropy`, `fill`, `seed`, `from_entropy`, `seed_from_os`, `next`, `below`, `range`, `float`, `fill_from`, `shuffle`. |
| `std::collections` | Source collection handles beyond sequences. | Linear `Set[T]`, `Deque[T]`, `RingBuffer[T]`, `LinkedList[T]`, `BinaryHeap[T]`, `PriorityQueue[T]`, hash-table `HashMap[K,V]`/`HashSet[T]`, red-black-tree `TreeMap[K,V]`/`TreeSet[T]`, explicit hash/comparator constructors, lookup, insertion, replacement, removal, reserve, clear, FIFO/linked/heap iteration where applicable, live-bucket hash iteration, and sorted tree iteration. |
| `std::iter` | Range, iterator traits, lazy adapters, and eager consumers. | `range`, `range_inclusive`, `Iterator`, `IntoIterator`, `map`, `filter`, `take`, `skip`, `enumerate`, `zip`, `fold`, `reduce`, `collect`. |
| `std::fmt` | Formatting traits plus explicit-zone, writer, and stdout formatting helpers. Root `Display`/`Debug` re-export these traits. | `Display::format_in`, `Debug::debug_in`, `FormatSpec`, `decimal`, `hex`, `binary`, `octal`, `with_width`, `with_precision`, `left`, `right`, `center`, `uppercase`, `alternate`, `unsigned_in`, `integer_in`, `boolean_in`, `float_in`, `text_in`, `debug_text_in`, `debug_value`, `write_unsigned`, `write_integer`, `write_boolean`, `write_text`, `write_value`, `write_debug`, `print_value`, `println_value`, `print_debug`, `println_debug`. |
| `std::cmp` | Comparison traits and helpers. | `Ord`, `min`, `max`, `clamp`, `is_between`. |
| `std::algo` | Source algorithms over borrowed slices. | `sort`, `sort_by`, `stable_sort`, `stable_sort_by`, `binary_search`, `is_sorted`, `reverse`, `rotate_left`, `rotate_right`, `partition`, `min`, `max`, `clamp`, `swap`, `fill`, `copy`, `dedup`. |
| `std::convert` | Explicit conversion trait names and helpers. | `From`, `Into`, `TryFrom`, `TryInto`, `identity`, `from`, `into`. |
| `std::math` | Source-only numeric helpers. | `abs`, `sign`, sign/parity predicates, checked add/sub/mul/div/rem/neg/abs, wrapping/overflowing add/sub/mul, saturating add/sub/mul/div/neg/abs, `pow`, floor/ceil division, `gcd`, `lcm`. |
| `std::bits` | Source-only bit-mask, rotation, power-of-two, low-mask, alignment, byte-swap, population-count, and zero/one-run bit-scan helpers. | `is_set`, `set`, `rotate_left`, `bit_width`, `low_mask`, `align_up`, `byte_swap`, `population_count`, `leading_ones`. |

## Allocation Rules

Anything that allocates takes a `ref mut Zone` or returns a handle tied to a
zone. Methods with an `_in` suffix take an explicit zone for growth or copying;
methods with a `_to` suffix copy a derived value into a target zone.
For tracked local `std::vec::Vec[T]` and `std::string::String` handles, Ari can
infer the same source zone for common mutating methods. `std::collections`
keeps growth explicit today: `Set`, `Deque`, `LinkedList`, `BinaryHeap`,
`PriorityQueue`, `HashMap`, `HashSet`, `TreeMap`, and `TreeSet` spell
`ref mut zone` on methods that may allocate. Fixed-capacity `RingBuffer`
allocates only at construction. Their handles keep the same zone provenance as
their backing storage, and collection iterator cursors preserve that
provenance.

Use `zone::destroy(zone)` when a manually created zone is no longer needed.
Pointers, strings, vectors, boxes, and slices derived from that zone become
invalid after `reset` or `destroy`, and sema rejects later use.

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
`std::parse` keeps whole-input integer, bool, and decimal float parsing out of
individual call sites. `std::encoding` validates ASCII/UTF-8/UTF-16 and
encodes or decodes hex/base64 into caller-provided zones. Fallible hex/base64
decoders now return `Option[String]`; richer `Result[String, E]` error
payloads and `Option[f64]` still wait on the broader error/float payload
roadmap.

`std::random` has OS-backed hooks for `entropy()` and `fill(values)`, because
seed material must come from the host and byte slices should be filled without
round-tripping through one-word entropy calls. The deterministic `Prng`,
bounded integer helpers, unit float helper, byte filling from a seeded PRNG,
and generic shuffle are source Ari. Cryptographic streams and fallible entropy
errors remain future work.

`std::test`, `std::log`, and `std::error` are also source-first.
`std::test::Report` aggregates checks, generic `equal`/`not_equal` stay
naturally named, and `scratch` simply creates an explicit `Zone` for tests.
`std::log` writes level-prefixed diagnostic lines to `stderr` through
`std::io::Stderr`. `std::error` defines compact recoverable error values,
stable error categories, POSIX errno mapping, and a raw scalar bridge for
today's `Result[T, i64]` storage limits. Rich test discovery, source
locations, structured logging, stack traces, backtraces, and direct
`Result[T, Error]` mixed-payload storage remain runtime, driver, and compiler
roadmap work.

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
`Option`. `Slice[T]` and `std::vec::Vec[T]` also share borrowed-view
operations such as `slice`, `split_at`, subsequence `find`, `contains_slice`,
lexicographic `compare`, lazy `chunks`, lazy `windows`, and delimiter `split`;
`Vec[T]` returns views over its live element storage, and `String` exposes the
same byte-view shape plus allocator-backed `join_in`.

`std::bits` follows the same rule for current `u64` mask, rotation,
power-of-two, low-mask, alignment, byte-swap, population-count, and source-loop
zero/one-run bit-scan helpers. Future intrinsic-backed implementations may
need compiler support, but the public edge-case behavior should stay
source-defined.

`std::cmp` is also source-first. Its current helpers build on the minimal
`Ord[T]::lt` method so generic code can select, clamp, or range-check values
without compiler-known comparison intrinsics.

`std::convert` follows the same source-first path for generic conversion
helpers. `identity`, `from`, and `into` are plain Ari functions over the
module's trait surface.

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

`std::process` starts with a small runtime-backed process surface: `id` reads
the host process id, `uid`/`gid` read current user and group identity, `exit`
terminates with an explicit status, `abort` terminates abnormally, and the
status/root helpers are source Ari. The first POSIX child-process slice adds
runtime-backed `fork`/`wait` plus source branch and error predicates. Portable
spawn, richer status values, and process handles are intentionally still
roadmap work.

`std::thread` is the first thread slice. `spawn`, `join`, `yield_now`, and
`available_parallelism` are runtime-backed because they call the host threading
or process APIs and install Ari's per-thread runtime id before source code
runs. `sleep` delegates to `std::time`, while `id`, `is_main`,
`is_join_error`, and the `Thread` methods are source helpers. Capturing
closures, user-facing thread-local storage, custom stack sizes, shared
ownership, locks, and richer status values remain future `std::sync` and
richer thread-policy work.

`std::sync` now starts with `AtomicI64`, plus source `Mutex`, `RwLock`, and
`Once` helpers built on it. Atomic method names are the names developers expect:
`load`, `store`, `swap`, `fetch_add`, and `compare_exchange`. The runtime
hooks lower directly to LLVM atomic operations with sequentially consistent
ordering. `Mutex` is a primitive spin/yield lock without a protected payload
or guard type, `RwLock` is a primitive explicit reader/writer lock without
guards, and `Once` runs plain `fn() -> void` entries at most once. `Shared`,
`Weak`, `Condvar`, `OnceLock`, `LazyLock`, channels, barriers, semaphores,
futex-backed blocking locks, and explicit memory-order arguments remain future
work.

`std::time` follows the same OS-facing pattern. `monotonic_nanos`,
`unix_nanos`, and `sleep_nanos` are runtime-backed because they call the host
clock and sleep APIs, while `Duration`, `Instant`, `SystemTime`, `Deadline`,
`UtcDateTime`, and the constructor/elapsed/timeout/calendar helpers are
ordinary Ari source. Use `Instant` and `Deadline` for elapsed time and timeout
policy; use `SystemTime` only for wall-clock timestamps and convert it to UTC
with `to_utc()` when a calendar value is needed.

`std::fs` is the first filesystem slice. `exists`, access-style permission
checks, `remove`, mode-string `open`, close, and single-byte read/write are
runtime-backed because they call host file-descriptor APIs. `permissions`,
`try_open`, compatibility `try_open_*` wrappers, `rename`, `create_dir`,
`remove_dir`, `create`/`try_create`, `write_bytes`, whole-file `read`, `write`,
`try_read`, `try_write`, `append`, `try_append`, `truncate`, `copy`,
byte-counting `try_copy`, byte-string `read_to_string`, fallible
`try_read_to_string`, `metadata`/`try_metadata`,
permission `mode`/`try_mode`/`set_mode`/`set_permissions`,
`canonicalize`/`try_canonicalize`, and the `File`, `Metadata`, and
`Permissions` methods are ordinary Ari source or thin runtime hooks over the
raw OS boundary. The handle is a visible value today and should become a
stronger owned resource when OS resource ownership is modeled by the language.

`std::path` is source-only and deliberately lexical. It works over borrowed
`Slice[u8]` values, typed `PathBytes` views, borrowed component iterators, and
zone-backed `String` outputs, so tools can split, join, iterate, and lightly
normalize paths without touching the filesystem. The current policy is
POSIX-style `/` separators only; platform-specific path forms remain future
work.

`std::net` starts with deterministic value types: IPv4, IPv6, generic IP, and
socket addresses. DNS lookup and TCP/UDP/Unix sockets remain runtime-backed
roadmap work because they need owned OS handles, errors, nonblocking behavior,
timeouts, and shutdown policy.

`std::collections` is source Ari over typed zone allocation. `Set[T]` remains a
small, insertion-order, linear set with iterator support. `Deque[T]` and
`RingBuffer[T]` cover growable and bounded queue shapes, `LinkedList[T]` uses
zone-backed reusable node slots, and `BinaryHeap[T]`/`PriorityQueue[T]` cover
highest-priority removal. `HashMap[K,V]` and `HashSet[T]` are real
open-addressed hash tables with tombstones and live-bucket iterators.
`TreeMap[K,V]` and `TreeSet[T]` are red-black trees with sorted successor
iterators for keys, values, and set values. Hash, tree, and heap constructors
take explicit policy functions until trait dispatch is strong enough for fully
trait-driven containers. The compiler only recognizes the handle shapes so
zone reset/destroy invalidation and same-zone growth diagnostics stay as strong
as `std::vec::Vec[T]`.

`std::zone` keeps allocation visible. Raw byte allocation and lifecycle hooks
are runtime-backed, while `alloc_array<T>` is source Ari that packages the
common "count times `size_of<T>()` at `align_of<T>()`" pattern for library
authors.

`std::input` follows that pattern for stdin. `read_byte`, `line`, and
`owned_line` are runtime hooks, while `try_read_byte` is source Ari that turns
the raw `-1` EOF sentinel into `Option[u8]`.

`std::io` keeps raw process IO visible and adds a small source trait layer.
Scalar and line operations are runtime hooks. `write_bytes`, `read_exact`,
`write_all`, `flush`, `Stdin`, `Stdout`, `Stderr`, `Cursor`, `BufReader`, and
`BufWriter` are source Ari over `Slice[u8]`, raw pointers, explicit
caller-provided buffers, and the process stream hooks. `pipe`, file adapters,
and zone-owning buffered constructors stay on the roadmap until owned OS
handles and resource flush/drop rules are explicit.

When adding new library code, first ask whether it can be written in Ari source
using existing modules. If yes, keep it in `lib/std/`. Add compiler support
only for a primitive the source language cannot express.
