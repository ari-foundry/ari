# Standard Library

The dedicated standard library docs live in [docs/stdlib](../stdlib/README.md).
This language page is kept as a broad user-facing map; use the dedicated
folder for the API reference, library development guide, testing guide, and
implementation roadmap.

This page is the map for Ari's source standard library. It explains what the
current libraries are for, which APIs are already usable, and which library
families should come next.

The standard library root lives at `lib/std.arih`. Child modules live under
`lib/std/`. Ordinary Ari programs auto-load this package as `std`, then the
prelude exposes common root names and module aliases. Pass `--no-implicit-std`
when you want to test the source package as an ordinary module dependency.

The library is intentionally small and capability-oriented. APIs that allocate
take an explicit `Zone`; Ari does not provide a magical global heap. Some
operations, such as layout queries, pointer operations, formatting macros, and
zone runtime calls, are declared in source but still lower through compiler
hooks because the current language cannot express those primitives directly.

## Library Map

| Library | Purpose | Current API Highlights | Status |
| --- | --- | --- | --- |
| `std` root | Common prelude surface and shared ADTs. | `Option[T]`, `Result[T, E]`, `Slice[T]` with `try_*`, `slice`, `split_at`, `find`, `compare`, `chunks`, `windows`, and `split`, `Range[T]`, `RangeInclusive[T]`, `move`, `take`, assertion helpers, panic helpers, root `Box`, `String`, `Vec`, `Error`, `ErrorKind`, `CStr`, `CString`, `Library`, `Symbol`, `AtomicI64`, `Mutex`, `RwLock`, and `Once` aliases. | Implemented source surface with compiler-known hooks for selected helpers. |
| `std::option` | Convenience methods for optional values. | `is_some`, `is_none`, `is_some_and`, `is_none_or`, `unwrap_or`, `unwrap_or_else`, `unwrap`, `expect`, `map`, `or`, `or_else`, `xor`, `and_then`, `filter`, `flatten`, `transpose`, `ok_or`, `ok_or_else`. | Implemented for the current generic enum model. |
| `std::result` | Error-return convenience methods. | `is_ok`, `is_err`, `is_ok_and`, `is_err_and`, `unwrap_or`, `unwrap_or_else`, `unwrap`, `expect`, `unwrap_err`, `expect_err`, `ok`, `err`, `map`, `map_err`, `and_then`, `or`, `or_else`, `transpose`. | Implemented for the current generic enum model. |
| `std::io` | Byte-oriented process IO hooks and contracts. | `Reader`, `Writer`, `Seek`, `Stdin`, `Stdout`, `Stderr`, `Cursor`, `BufReader`, `BufWriter`, `stdin`, `stdout`, `stderr`, `cursor`, `buf_reader`, `buf_writer`, `read_exact`, `write_all`, `flush`, raw byte/scalar/line hooks. | Runtime-backed hooks plus source trait/cursor/buffered/exact-read/write-all helpers. `pipe`, file adapters, and zone-owning buffered constructors are roadmap items. |
| `std::input` | Friendly stdin helpers. | `read_byte`, `try_read_byte`, `line`, `owned_line`. | Runtime-backed hooks plus source EOF-to-Option byte handling. |
| `std::context` | Low-level runtime context access. | `argc`, `arg`, `thread_id`, startup `cwd`, startup `executable_path`, `has_arg`, `user_arg_count`, `is_main_thread`. | Runtime-backed startup hooks plus source predicates and path views; initialized by the generated entry wrapper. |
| `std::test` | Executable unit-test helpers. | `Report`, `report`, `scratch`, `check`, generic `equal`/`not_equal`, pass/fail accessors, `ok`, `finish`, `require`, and method wrappers. | Source helpers for aggregated checks and temporary test zones. Test discovery, log capture, source locations, stack/backtrace, benchmark, and fuzzing hooks are roadmap work. |
| `std::log` | Level-prefixed stderr diagnostics. | `Level`, `rank`, `name`, `enabled`, `write`, `message`, `trace`, `debug`, `info`, `warn`, `error`. | Source-only helper over `std::io::Stderr`. Structured records, source locations, stack/backtrace, and capture policy are future work. |
| `std::error` | Shared recoverable error values. | `Kind`, `Error`, `new`, `with_code`, `from_errno`, `from_raw`, `kind`, `code`, `raw`, `is_kind`, `is_not_found`, `is_interrupted`, `is_retryable`, `name`, `message`. | Source compact error value for OS/runtime/library failures. Use `raw()` with `Result[T, i64]` until mixed `Result[T, Error]` payload storage lands. |
| `std::c` | C ABI boundary helpers. | `CStr`, `CString`, `Library`, `Symbol`, C string constructors, POSIX `errno`/`error`, null checks, `dlopen`/`dlsym` dynamic loading wrappers, and typed dynamic function extraction. | Source helpers over compiler-known C ABI aliases and hosted loader symbols. Data-symbol wrappers and explicit owned-buffer FFI escape policy are future work. |
| `std::target` | Compiler-known target and platform facts. | `triple`, `arch`, `arch_name`, `os`, `env`, `pointer_bits`, `long_bits`, `uses_glibc`, `uses_musl`, `uses_elf`, `uses_dwarf`, `syscall_abi`, Linux API-family predicates. | Runtime-backed compiler constants plus source predicates. Build-mode hardening flags and raw OS wrappers remain roadmap work. |
| `std::env` | User-facing process argument, environment-variable, and path-state helpers. | `arg_count`, `try_arg`, `program_name`, `get`, `try_get`, `set`, `remove`, `current_dir`, `try_current_dir`, `set_current_dir`, `executable_path`. | Argument wrappers over `std::context` plus runtime-backed current-process environment/path hooks. |
| `std::process` | Current-process helpers and the first POSIX child-process slice. | `id`, `uid`, `gid`, `exit`, `abort`, `success`, `failure`, `is_success`, `is_failure`, `is_root`, `fork`, `wait`, `is_child`, `is_parent`, `is_fork_error`, `is_wait_error`. | Runtime-backed id/uid/gid/exit/abort/fork/wait hooks plus source predicates. Portable spawn, exec, kill, richer statuses, and process handles are future work. |
| `std::thread` | Function-pointer thread spawn/join, runtime ids, sleep/yield hints, and hosted parallelism. | `Thread`, `spawn`, `join`, `yield_now`, `sleep`, `id`, `is_main`, `available_parallelism`, `is_join_error`. | First pthread-backed LLVM/Linux slice. Capturing entries, user-facing thread-local storage, custom stack sizes, shared ownership, locks, and richer statuses are future work. |
| `std::sync` | Small explicit synchronization primitives. | `AtomicI64`, `Mutex`, `RwLock`, `Once`, atomic `load`/`store`/`swap`/`fetch_add`/`compare_exchange`, mutex helpers, rwlock helpers, `call_once`. | LLVM atomic slice plus source spin/yield `Mutex`, explicit `RwLock`, and `Once`. Generic atomics, value-protecting locks, shared ownership, `Condvar`, `OnceLock`, `LazyLock`, channels, and futex-backed blocking are future work. |
| `std::time` | Monotonic instants, wall-clock timestamps, durations, sleep, deadlines, and UTC calendar values. | `Duration`, `Instant`, `SystemTime`, `Deadline`, `UtcDateTime`, `nanoseconds`, `microseconds`, `milliseconds`, `seconds`, `now`, `system_now`, `system_from_unix`, `utc_from_unix`, `elapsed`, `sleep`, `timeout`, `timeout_after`, `deadline_at`. | Runtime-backed clock/sleep hooks plus source value wrappers. UTC calendar conversion is built in; timezone data remains external-package or later platform work. |
| `std::fs` | Byte-oriented filesystem handles. | `File`, `Permissions`, `exists`, `can_read`, `can_write`, `can_execute`, `permissions`, `remove`, `rename`, `hard_link`, `symbolic_link`, `create_dir`, `remove_dir`, `open`, `try_open`, `create`, `try_create`, compatibility `open_*`/`try_open_*`, `read_byte`, `write_byte`, `write_bytes`, whole-file `read`, `try_read`, `write`, `try_write`, `append`, `try_append`, `truncate`, `copy`, `try_copy`, `read_to_string`, `try_read_to_string`, `close`. | First runtime-backed file slice over host file descriptor hooks plus source `Option[File]`, `Option[String]`, `Option[i64]` byte-count helpers, `Permissions`, and whole-file byte helpers. Metadata, permission mutation, directory iteration, recursive directory helpers, canonical paths, temp files, and optional locking remain roadmap work. |
| `std::path` | Source-only lexical path manipulation. | `PathBytes`, `bytes`, `from_os`, method-style path-byte helpers, `is_separator`, `is_absolute`, `is_relative`, `trim_trailing_separators`, `components`, `file_name`, `parent`, `extension`, `stem`, `join_in`, `normalize_in`. | POSIX-style `/` separator policy implemented in Ari source. Platform-specific paths, owned path buffers, and runtime canonicalization are future work. |
| `std::net` | Network address values. | `Ipv4Addr`, `Ipv6Addr`, `IpAddr`, `SocketAddr`, `ipv4`, `ipv6`, `socket_addr`, `localhost`, family, loopback, unspecified, and port helpers. | Source-only address slice. DNS, TCP, UDP, Unix sockets, socket options, nonblocking mode, timeouts, and shutdown are runtime roadmap work. |
| `std::mem` | Layout, raw pointer, byte memory, and hosted page-size helpers. | `size_of`, `align_of`, `ptr_offset`, `ptr_add`, `ptr_load`, `ptr_store`, `copy_bytes`, `move_bytes`, `set_bytes`, `page_size`, `replace`, `swap`. | Compiler-lowered where layout, typed pointer, memory intrinsic, or hosted page-size semantics are required. |
| `std::zone` | Explicit allocation capability. | `create`, byte `alloc`, typed `alloc[T]`, `alloc_array[T]`, `new[T]`, `promote[T]`, `reset`, `destroy`, `allocation_zone`. | Runtime-backed with ownership/provenance checks in sema plus source raw array allocation. |
| `std::boxed` | Zone-backed single-value owner handle. | `Box[T]`, `new`, `Box::new`, `get`, `set`, `replace`, `take`, `try_take`, `clear`, `put_in`, `copy_to`, `as_ref`, `as_mut`, `swap`, raw pointer access. | Implemented as an explicit-zone seed for future smart-pointer work. |
| `std::string` | Zone-backed owned byte string seed plus typed borrowed text-boundary views. | `String`, `RawString`, `Utf8`, `OsStr`, `utf8`, `os_str`, `c_str`, `c_len`, `c_bytes`, `bytes`, capacity constructors, `join_in`, copy helpers, byte get/set/search, byte-slice `find`, borrowed split/chunk/window views, `try_get`, `try_pop`, growth, append helpers, UTF-8 validation/scalar helpers, ASCII case compare/search, trim views, trim copies, whole and prefix parse helpers, `as_slice`, `as_ptr`. | Implemented as a byte string with UTF-8 scalar helpers and boundary views. `c_str` returns the shared `std::c::CStr` type. Owned `Utf8String`/`OsString`, normalization, grapheme clusters, and locale policy remain future work. C ABI owned strings live in `std::c::CString`. |
| `std::ascii` | ASCII-only byte and slice helpers for byte strings and parsers. | `ParsedInt`, `is_digit`, `is_alpha`, `is_alphanumeric`, `is_blank`, `is_whitespace`, `is_control`, `is_printable`, `is_graphic`, `is_punctuation`, `is_hex_digit`, `to_lower`, `to_upper`, `digit_value`, `hex_value`, `equals_ignore_case`, `starts_with_ignore_case`, `ends_with_ignore_case`, `index_of_ignore_case`, `contains_ignore_case`, `trim`, `parse_decimal`, `parse_decimal_prefix`, `parse_hex`, `parse_hex_prefix`. | Implemented in Ari source; not a Unicode or locale-aware text API. |
| `std::parse` | Whole-input parsers over ASCII-trimmed byte slices. | `integer`, `boolean`, `is_float`, `float_or`, `float`. | Implemented in Ari source. `float`/`float_or` are used instead of `Option[f64]` until float enum payloads are supported. |
| `std::encoding` | Text validation, UTF-8 scalar helpers, and byte codecs. | `Utf8Char`, `is_ascii`, `is_unicode_scalar`, `utf8_count`, `is_utf8`, `utf8_width`, `utf8_encoded_len`, `utf8_at`, `utf8_next_index`, `encode_utf8_in`, `try_encode_utf8_in`, `utf16_count`, `is_utf16`, `hex_encoded_len`, `encode_hex_in`, `hex_decoded_len`, `can_decode_hex`, `decode_hex_in`, `try_decode_hex_in`, `base64_encoded_len`, `encode_base64_in`, `base64_decoded_len`, `can_decode_base64`, `decode_base64_in`, `try_decode_base64_in`. | Implemented in Ari source. Use `try_*` helpers for untrusted input and asserting helpers only when invalid input is a programmer error. |
| `std::vec` | Zone-backed growable sequence seed. | `Vec[T]`, `RawVec[T]`, `Iter[T]`, constructors, metadata, checked and `Option` element access, mutation, growth, copy, direct borrowed `slice`/`split_at`/`find`/`contains_slice`/`compare`/`chunks`/`windows`/`split`, raw pointer access, iterator support. | Implemented as explicit-zone source `Vec`; root bare `Vec[T]` is still the compiler-known local vector type. |
| `std::hash` | Deterministic non-cryptographic hashing. | `Hasher`, `Hash[T]`, `new`, `reset`, `finish`, `write`, `value`, `bytes`, primitive write helpers. | Source-only first slice for values and byte slices. Hash-table constructors still take explicit hash functions until trait-driven collection constructors land. |
| `std::random` | OS entropy and deterministic non-cryptographic random streams. | `Prng`, `entropy`, `fill`, `seed`, `from_entropy`, `seed_from_os`, `next`, `below`, `range`, `float`, `fill_from`, `shuffle`. | `entropy` is runtime-backed on Linux through `getrandom` with `/dev/urandom` fallback. `Prng` and shuffle are source Ari and are not cryptographic. |
| `std::collections` | Zone-backed collection handles beyond sequences. | Linear `Set[T]`/`Iter[T]`, `Deque[T]`, `RingBuffer[T]`, `LinkedList[T]`, `BinaryHeap[T]`, `PriorityQueue[T]`, hash-table `HashMap[K,V]`/`HashSet[T]`, red-black-tree `TreeMap[K,V]`/`TreeSet[T]`, explicit hash/comparator constructors, queue/list/heap operations, lookup, insertion, replacement, removal, reserve, clear, hash bucket iterators, and sorted tree iterators. | Implemented in source Ari with compiler provenance recognition for reset/destroy and same-zone growth checks. |
| `std::iter` | Iteration traits, range constructors, lazy adapters, and eager consumers. | `range`, `range_inclusive`, `Iterator[T]`, `IntoIterator[T]`, `Iterable[T]`, `map`, `filter`, `take`, `skip`, `enumerate`, `zip`, `fold`, `reduce`, `collect`. | Adapter callbacks are plain function pointers today; closures and richer inference remain future work. |
| `std::fmt` | Formatting traits and explicit formatting helpers. Root `Display`/`Debug` are aliases for these traits. | `Debug::debug_in`, `Display::format_in`, `FormatSpec`, `decimal`, `hex`, `binary`, `octal`, width/precision/alignment modifiers, allocator-backed `*_in` helpers including `float_in`, `debug_value`, `write_value`, `write_debug`, `print_value`, `println_value`, `print_debug`, `println_debug`, and type-specific `write_*` helpers for `io::Writer`. | Formatting macros still use compiler lowering. `{}` maps to Display, `{:?}` maps to Debug where an explicit zone is available, and `FormatSpec` covers source hex/binary/octal, width, precision, and alignment for unsigned integers. |
| `std::cmp` | Comparison traits and helpers. | `Eq`, `PartialEq`, `Ord`, `PartialOrd`, `min`, `max`, `clamp`, `is_between`. | Implemented for source-level trait-bound static dispatch. |
| `std::algo` | Slice algorithms. | `sort`, `sort_by`, `stable_sort`, `stable_sort_by`, `binary_search`, `is_sorted`, `reverse`, `rotate_left`, `rotate_right`, `partition`, `min`, `max`, `clamp`, `swap`, `fill`, `copy`, `dedup`. | Source-only first slice over borrowed `Slice[T]` views. Faster sorts, move-aware contracts, hex/base64 encoding, and compression policy remain roadmap work. |
| `std::convert` | Conversion trait names and helpers. | `From`, `Into`, `TryFrom`, `TryInto`, `identity`, `from`, `into`. | First source helper slice; broad conversion impls and fallible conversion methods are future work. |
| `std::math` | Source-only numeric helpers. | `abs`, `sign`, sign/parity predicates, checked add/sub/mul/div/rem/neg/abs, wrapping/overflowing add/sub/mul, saturating add/sub/mul/div/neg/abs, `pow`, floor/ceil division, `gcd`, `lcm`. | Current i64-signature helper slices with natural names; generic numeric traits remain future work. |
| `std::bits` | Source-only bit-mask, rotation, power-of-two, low-mask, alignment, byte-swap, population-count, and zero/one-run bit-scan helpers. | `is_set`, `any_set`, `set`, `clear`, `toggle`, `rotate_left`, `rotate_right`, `is_power_of_two`, `bit_width`, `floor_power_of_two`, `ceil_power_of_two`, `low_mask`, `align_down`, `align_up`, `count_ones`, `population_count`, `count_zeros`, `byte_swap`, `leading_zeros`, `trailing_zeros`, `leading_ones`, `trailing_ones`. | Current u64-signature helper slices; generic integer policy is future work. |

## API Conventions

Allocation APIs take a `ref mut Zone` or return values tied to a zone. The
`_in` suffix means "use this explicit zone for growth or copying". For tracked
local `std::vec::Vec[T]` and `std::string::String` handles, Ari can infer the
same source zone for common methods such as `push`, `insert`, `reserve`,
`reserve_extra`, `extend_from_slice`, and `resize`. `std::collections` handles
keep growth explicit today: `Set`, `HashMap`, `HashSet`, `TreeMap`, and
`TreeSet`, plus `Deque`, `LinkedList`, `BinaryHeap`, and `PriorityQueue`,
spell `ref mut zone` on methods that may allocate, such as `insert`,
`replace`, `push`, `push_front`, `push_back`, `reserve`, and `reserve_extra`
where that method exists. `RingBuffer` is fixed-capacity after construction.

Generic APIs should keep natural names. Prefer `insert`, `get`, `contains`,
and `copy_to` over type-suffixed names such as `insert_i64`; the type belongs
in `Set[T]`, `Vec[T]`, or an explicit generic call like `collections::new<i64>`.
Current Ari function declarations still spell result types because generics
describe type parameters, not the full public signature.

Read-only methods usually take `self: ref Self`; mutating methods take
`self: ref mut Self`. Raw pointer accessors preserve the allocation provenance
known by the checker, so using a pointer after its zone is reset or destroyed
is rejected.

APIs that can fail without a diagnostic should prefer `Option` or `Result`.
APIs that would indicate a programmer error, such as out-of-bounds checked
access in today's collection methods, use `std::assert` and trap.

Root aliases are convenience names, not separate implementations. `std::Box`
is `std::boxed::Box`, `std::String` is `std::string::String`, and `std::Vec`
is `std::vec::Vec`. Bare root `Vec[T]` without the `std::` source alias is
still the compiler-known local vector surface.

## Task-Oriented API Guide

Use this table when writing code from docs alone:

| Task | Preferred API | Notes |
| --- | --- | --- |
| Print debug or user-facing output. | `print`, `println`, `print!`, `println!`, `log::debug("...")`, `log::info("...")`, `log::write(log::Info, bytes)`, `var out = io::stdout(); io::write_all(ref mut out, slice)`, `var err = io::stderr(); io::write_all(ref mut err, slice)`, `io::write_bytes(slice)` | Format strings must be string literals. Use `{}` or `{name}` for strings, byte `char` values, integers, bools, and `f32`/`f64`; named captures can read fields like `{point.x}` and tuple slots like `{pair.0}`. Use `{:.N}` or `{name:.N}` for float precision; use `{:?}` or `{name:?}` for debug text. Use `std::log` for level-prefixed stderr diagnostics and `write_all`/`write_bytes` for raw `Slice[u8]` output. |
| Write a small executable unit test. | `var report = test::report(); report.equal(left, right); report.check(condition); var zone = test::scratch(n); report.finish()` | `std::test` aggregates pass/fail counts and returns a final status. Destroy scratch zones explicitly. Use root `assert`, `assert_equal`, `assert_not_equal`, `panic`, `todo`, or `unreachable` when the first failure should stop immediately. Source locations, logging, stack/backtrace, benchmark, and fuzzing integration are future work. |
| Read process arguments. | `env::try_arg(index)`, `env::try_arg_os(index)`, `env::program_name()`, `env::program_name_os()`, `env::arg_count()`, `env::has_arg(index)`, root `arg_count()`, `has_arg(index)` | Use `try_arg` for lowercase Ari `string` values and `try_arg_os` when the argument should stay as `std::string::OsStr` until validated or treated as bytes. |
| Read or edit current-process environment variables. | `env::try_get(name)`, `env::try_get_os(name)`, `env::has(name)`, `env::get(name)`, `env::get_os(name)`, `env::set(name, value)`, `env::remove(name)` | Values are borrowed host strings or OS-string views; copy with `std::string::from_string(ref mut zone, value)` when owned text is needed. `get` returns an empty string for missing variables, so prefer `try_get`/`try_get_os` when absence matters. |
| Inspect current process paths. | `env::try_current_dir()`, `env::try_current_dir_os()`, `env::try_current_dir_path()`, `env::set_current_dir(path)`, `env::try_executable_path()`, `env::try_executable_path_os()`, `std::path::from_os(value)` | Use `PathBytes` for lexical path work and `OsStr` when the path is still OS-boundary data. The current executable path can be converted with `std::path::from_os`. |
| Inspect or terminate the current process. | `process::id()`, `process::uid()`, `process::gid()`, `process::is_root()`, `process::exit(code)`, `process::abort()`, `process::success()`, `process::failure()` | `exit` and `abort` terminate immediately and do not run later Ari cleanup. `is_root` is a convenience check, not a permission guarantee. |
| Fork and wait for a child on POSIX. | `process::fork()`, `process::is_child(pid)`, `process::is_parent(pid)`, `process::wait(pid)`, `process::is_wait_error(status)` | This is the first Linux/LLVM runtime path slice. `wait` returns a normal child exit code or `-1`. Portable spawn, process handles, and richer status values are future work. |
| Start and join a thread. | `thread::spawn(worker)`, `handle.join()`, `thread::join(handle)`, `thread::id()`, `thread::yield_now()`, `thread::sleep(duration)`, `thread::available_parallelism()` | Thread entries are plain `fn() -> i64` today. The main thread id is `0`; spawned thread ids are positive. Join returns the entry result or `-1` for the current failure sentinel. Available parallelism is at least `1`; stack-size configuration and user TLS remain roadmap work. |
| Coordinate simple atomic state. | `AtomicI64::new(value)`, `atomic.load()`, `atomic.store(replacement)`, `atomic.fetch_add(amount)`, `atomic.compare_exchange(expected, replacement)` | Operations are sequentially consistent. Thread entries cannot capture shared atomic references yet, so this is a primitive building block, not the whole shared-state model. |
| Use a small explicit lock or one-time initializer. | `Mutex::new()`, `mutex.try_lock()`, `mutex.lock()`, `mutex.unlock()`, `RwLock::new()`, `rwlock.read_lock()`, `rwlock.write_lock()`, `Once::new()`, `once.call_once(init)` | Current `Mutex` and `RwLock` are source primitive locks without protected payloads or guard types. Current `Once` runs a plain `fn() -> void` at most once. `Mutex[T]`, `RwLock[T]`, `Condvar`, `OnceLock`, `LazyLock`, channels, and futex-backed blocking are future work. |
| Measure elapsed time, sleep, or build a timeout. | `time::now()`, `start.elapsed()`, `time::elapsed(start)`, `time::milliseconds(n)`, `time::sleep(duration)`, `time::timeout(duration)`, `deadline.remaining()`, `deadline.has_expired()` | Use `Instant` for elapsed time because it is monotonic. `Deadline` is also monotonic and should be preferred for timeout policy. `sleep` is a thin current-thread sleep wrapper and does not report interruption yet. |
| Read wall-clock Unix time. | `time::system_now()`, `system_time.as_unix_nanos()` | Use `SystemTime` for timestamps, not duration measurement; host wall clocks can move. |
| Convert Unix time to UTC calendar fields. | `time::utc_from_unix(seconds, nanos)`, `time::SystemTime::from_unix(seconds, nanos)`, `system_time.to_utc()`, `utc.year()`, `utc.month()`, `utc.day()` | The current calendar API is UTC-only, non-negative Unix timestamp only, and does not include timezone databases or local time conversion. |
| Work with small byte files. | `fs::try_write(path, bytes)`, `fs::try_append(path, bytes)`, `fs::write(path, bytes)`, `fs::append(path, bytes)`, `fs::try_read(ref mut zone, path)`, `fs::read(ref mut zone, path)`, `fs::try_copy(source, target)`, `fs::copy(source, target)`, `fs::rename(source, target)`, `fs::truncate(path)`, `fs::try_create(path)`, `fs::try_open(path, "r")`, `file.read_byte()`, `file.write_bytes(slice)`, `file.close()`, `fs::exists(path)`, `fs::permissions(path)`, `fs::remove(path)` | Prefer `try_read` when absence must be different from an empty file; it returns `None` for missing/unopenable paths and `Some(empty)` for empty files. `try_write`, `try_append`, and `try_copy` return successful byte counts. `read`/`read_to_string`, `write`, `append`, and `copy` keep compatibility fallback shapes. `"w"` creates or truncates, `"a"` creates or appends, and `"rw"` opens an existing file for reading/writing. `permissions` is a preflight access snapshot, not a replacement for handling later file errors. The current `File` is a value handle, so close successful handles once and do not reuse copies after closing. |
| Split or join lexical paths. | `path::bytes(bytes)`, `path::from_os(os)`, `path::components(bytes)`, `path::file_name(bytes)`, `path::parent(bytes)`, `path::stem(bytes)`, `path::extension(bytes)`, `path::join_in(ref mut zone, base, child)`, `path::normalize_in(ref mut zone, bytes)` | Prefer `PathBytes` when a value should be treated as a path rather than a generic byte string. These helpers do not query the filesystem. Borrowed component helpers return views into the original bytes. `components` is a lazy iterator over non-empty path parts. Current separator policy is POSIX-style `/` only. |
| Create or remove one directory. | `fs::create_dir(path)`, `fs::remove_dir(path)`, `fs::exists(path)` | These are single-directory helpers. They do not create parents, remove non-empty directories, or iterate entries yet. |
| Represent network addresses. | `net::Ipv4Addr::localhost()`, `net::Ipv6Addr::any()`, `addr.as_ip()`, `net::socket_addr(ip, port)`, `net::SocketAddr::localhost(port)` | This is value-only and does not open sockets. DNS lookup, TCP/UDP/Unix sockets, socket options, nonblocking mode, timeouts, and shutdown are future runtime slices. |
| Inspect the target/platform contract. | `target::triple()`, `target::arch()`, `target::is_linux()`, `target::uses_glibc()`, `target::uses_elf()`, `target::syscall_abi()`, `target::has_epoll()` | These are compile-target facts, not live kernel probes. Static/dynamic linking, PIE, RELRO, stack protector, and raw descriptor APIs are platform roadmap items. |
| Read stdin. | `input::try_read_byte()`, `input()`, `read_line()`, `input_owned(ref mut zone)` | `try_read_byte` returns `Option[u8]` instead of the raw `-1` EOF sentinel. Borrowed line input reuses an internal buffer. Owned line input copies into `std::string::String`. |
| Write generic byte-output helpers. | `io::Writer`, `io::BufWriter`, `io::write_all[W: io::Writer](writer: ref mut W, bytes)`, `io::flush[W: io::Writer](writer: ref mut W)` | `io::Stdout` and `io::Stderr` implement `Writer` now. `BufWriter` uses a caller-provided `Slice[u8]` buffer and flushes explicitly. File writer adapters are planned after owned handle behavior is specified. |
| Test parser or binary read logic without host stdin. | `io::cursor(bytes)`, `io::BufReader`, `io::Reader`, `io::Seek`, `io::read_exact[R: io::Reader](reader: ref mut R, output, len)` | `Cursor` reads from a borrowed `Slice[u8]`, supports `position()` and `seek(position)`, and returns `-1` at EOF through `Reader.read_byte()`. `BufReader` uses a caller-provided `Slice[u8]` buffer. |
| Represent missing values. | `Option[T]`, `Some(value)`, `None<T>()` | Use `.unwrap_or`, `.map<U>`, `.and_then<U>`, `.filter()`, `.flatten()`, `.transpose()`, `?`, or `??` when that reads better than `match`. |
| Convert missing values into failures. | `option.ok_or<E>(error)`, `option.ok_or_else<E>(op)` | Lazy form builds the error only for `None`. |
| Represent success/failure. | `Result[T, E]`, `Ok<T, E>(value)`, `Err<T, E>(error)` | Use `.map<U>`, `.and_then<U>`, `.transpose()`, `.or<F>`, `.or_else<F>`, or `?` when that reads better than `match`. |
| Convert failures back to optional values. | `result.ok()`, `result.err()` | Keeps only the selected payload branch. |
| Work with borrowed contiguous data. | `Slice[T]`, `slice(data, len)`, `.as_slice()`, `.slice(start, end)`, `.split_at(index)`, `.find(needle)`, `.chunks(size)`, `.windows(size)`, `.split(delimiter)` | Slice methods borrow the view; use `try_get` when absence is expected, `find`/`contains_slice` for subslice search, lazy chunks/windows/split for allocation-free views, and `copy_to(ref mut zone)` when an owned collection is needed. |
| Store a small local literal sequence. | Bare `Vec[T]` from `[a, b, c]` | This is compiler-known local vector storage, not `std::vec::Vec[T]`. Empty `[]` needs an expected type. |
| Store a growable source collection. | `std::vec::new<T>(ref mut zone, capacity)` | Common tracked locals can call `push`, `insert`, `reserve`, and related methods without spelling the zone again. Use `vec.slice`, `vec.split_at`, `vec.find`, `vec.contains_slice`, `vec.compare`, `vec.chunks`, `vec.windows`, and `vec.split` for allocation-free borrowed sequence views. |
| Store unique values in insertion order. | `collections::new<T>(ref mut zone, capacity)` or `Set::new<T>(ref mut zone, capacity)` | `insert(ref mut zone, value)` returns whether a value was newly added. `replace(ref mut zone, value)` returns the previous equal value or inserts the missing value. Use `try_get`, `try_pop`, `contains`, `remove`, `take`, `reserve`, `iter`, `as_slice`, and `copy_to` for the linear set. |
| Use both ends of a queue. | `Deque::new<T>(ref mut zone, capacity)` or `collections::deque<T>(...)` | `push_front`/`push_back` may grow with the same zone. `pop_front`/`pop_back`, `front`/`back`, `try_*`, `get`, and iteration use logical front-to-back order. |
| Keep a bounded FIFO buffer. | `RingBuffer::new<T>(ref mut zone, capacity)` or `collections::ring_buffer<T>(...)` | `push(value)` returns `false` when full. `push_overwrite(value)` keeps the newest value and returns the overwritten oldest value as `Option[T]`. |
| Use linked front/back nodes. | `LinkedList::new<T>(ref mut zone, capacity)` or `collections::linked_list<T>(...)` | Uses zone-backed reusable node slots. `push_front`/`push_back`, `pop_front`/`pop_back`, `remove_at`, `try_remove_at`, and `iter` are available; indexed access is O(n). |
| Pop highest-priority values. | `BinaryHeap::new<T>(ref mut zone, capacity, less)` or `PriorityQueue::new<T>(ref mut zone, capacity, less)` | `less(a, b)` means `a` has lower priority than `b`. With `collections::less_i64`, larger integers pop first. Use `push`, `peek`, `try_peek`, `pop`, and `try_pop`. |
| Store values by hash lookup. | `HashMap::new<K,V>(ref mut zone, capacity, hash)` or `collections::hash_map<K,V>(...)` | Hash functions have shape `fn(K) -> u64`. `collections::hash_i64` is available for i64 keys and delegates to `hash::value<i64>`. `insert` returns the replaced `Option[V]`, `try_get` handles absence, `remove` leaves a tombstone for later probing, and `keys()`/`values()` iterate live buckets. |
| Store hash-based membership. | `HashSet::new<T>(ref mut zone, capacity, hash)` or `collections::hash_set<T>(...)` | `insert` returns whether the value was new. Use `replace`, `take`, `remove`, `contains`, `reserve`, `clear`, and `iter`. Direct `for value in set` uses the same live-bucket cursor. |
| Store values by ordered lookup. | `TreeMap::new<K,V>(ref mut zone, capacity, less)` or `collections::tree_map<K,V>(...)` | The comparator has shape `fn(K, K) -> bool` and must be a strict less-than relation. `collections::less_i64` is available for i64 keys. `keys()` and `values()` iterate in ascending key order. |
| Store ordered membership. | `TreeSet::new<T>(ref mut zone, capacity, less)` or `collections::tree_set<T>(...)` | Uses a red-black tree. `insert` rejects equal values, `replace` returns the previous equal value, and `iter`/direct `for value in set` walk ascending comparator order. |
| Generate random values or shuffle a slice. | `random::entropy()`, `random::seed(123u64)`, `random::from_entropy()`, `rng.range(start, end)`, `rng.float()`, `rng.shuffle<T>(values)` | Use `entropy`/`fill` for OS-backed seed material. `Prng` is deterministic and non-cryptographic, so it is for simulations, tests, randomized algorithms, and repeatable shuffles. |
| Store owned byte text. | `std::string::from(ref mut zone, "text")`, `std::string::copy(ref mut zone, bytes)`, `std::string::empty(ref mut zone)`, `std::string::join_in(ref mut zone, parts, separator)` | The handle stores bytes, not a full Unicode text abstraction yet. Use `append`, `append_byte`, `append_bytes`, `contains_text`, `starts_with_text`, and `equals_text` for ordinary string-literal workflows. String literals coerce to `Slice[u8]`, local `Vec[u8]`, or fixed `[u8, N]` when those byte types are expected; use `std::string::bytes("literal")` when the boundary should be explicit. |
| Distinguish text and path boundary values. | `std::string::utf8(bytes)`, `std::string::os_str(bytes)`, `std::string::c_str(text)`, `std::path::bytes(bytes)`, `std::path::from_os(os)` | `String` is owned bytes, `Utf8` is validated borrowed UTF-8, `OsStr` is raw OS bytes, `std::c::CStr` wraps NUL-terminated C ABI text, and `PathBytes` means path policy. String literals coerce directly to expected `Utf8`, `OsStr`, `PathBytes`, and `CStr` boundary views. |
| Compare, search, trim, or parse owned ASCII byte text. | `text.equals_text_ignore_case("text")`, `text.index_of_text_ignore_case("text")`, `text.trim()`, `text.trimmed(ref mut zone)`, `text.parse_decimal()`, `text.parse_decimal_prefix()` | Case-insensitive `String` helpers fold only ASCII letters. Plain trim methods return borrowed `Slice[u8]` views; `trimmed*`/`*_to` methods copy into a target zone. Whole parse methods require the whole string to be valid; prefix parsers return `Option[ascii::ParsedInt]`. |
| Classify, compare, search, trim, or parse ASCII bytes. | `ascii::is_digit`, `ascii::is_printable`, `ascii::equals_ignore_case`, `ascii::index_of_ignore_case`, `ascii::to_lower`, `ascii::trim`, `ascii::parse_decimal_prefix` | Scalar helpers take `char`, the standard alias for an ASCII `u8`; slice helpers take `Slice[u8]` and accept string literals directly. Case-insensitive comparison/search folds only ASCII letters. Whole parsers return `Option[i64]`; prefix parsers return `Option[ascii::ParsedInt]` with `value` and consumed `len`. |
| Parse whole byte-slice values. | `parse::integer(bytes)`, `parse::boolean(bytes)`, `parse::is_float(bytes)`, `parse::float_or(bytes, fallback)`, `parse::float(bytes)` | These helpers trim ASCII whitespace and reject trailing garbage. `boolean` accepts lowercase `true`/`false`. `float` panics on invalid input; use `is_float` or `float_or` when invalid input is ordinary. |
| Validate text or encode bytes. | `encoding::is_ascii(bytes)`, `encoding::is_utf8(bytes)`, `encoding::utf8_count(bytes)`, `encoding::is_utf16(words)`, `encoding::encode_hex_in(ref mut zone, bytes)`, `encoding::try_decode_hex_in(ref mut zone, text)`, `encoding::encode_base64_in(ref mut zone, bytes)`, `encoding::try_decode_base64_in(ref mut zone, text)` | Hex emits lowercase. Base64 uses the standard `+`/`/` alphabet with `=` padding. Use `try_decode_*_in` for untrusted input; `decode_*_in` is the asserting form. |
| Store one zone-backed value. | `std::boxed::new<T>(ref mut zone, value)` or `Box!(T, ref mut zone, value)` | `take()` empties the handle; `try_take()` returns `Option[T]`. |
| Allocate raw memory. | `zone::alloc`, `zone::alloc<T>`, `zone::alloc_array<T>`, `zone::new<T>` | Raw allocation does not run destructors or make memory safe by itself. `alloc_array<T>` returns uninitialized contiguous storage for `count` values. |
| Inspect layout or raw memory. | `size_of<T>`, `align_of<T>`, `ptr_add`, `ptr_load`, `ptr_store`, `mem::copy_bytes`, `mem::move_bytes`, `mem::set_bytes`, `mem::page_size()` | Use typed helpers only for scalar and supported Ari-layout aggregate values. Use byte helpers for raw `ptr u8` regions where you own initialization, overlap, and length invariants. `page_size` reports the hosted runtime page size for alignment and future mapping work. |
| Compare values generically. | `cmp::min`, `cmp::max`, `cmp::clamp`, `cmp::is_between` | Requires an `Ord[T]` impl for the compared type. `is_between` is inclusive and `clamp`/`is_between` assert that `low <= high`. |
| Convert values generically. | `convert::identity`, `convert::from`, `convert::into` | `from<T, U>` uses `convert::From[T]` for destination `U`; `into<T, U>` uses `convert::Into[T]` on source `U`. |
| Iterate ranges. | `range(start, end)`, `range_inclusive(start, end)`, `start..end`, `start..=end` | Works directly in `for` loops and stores as `Range[T]`/`RangeInclusive[T]`. |
| Transform iterators lazily. | `iter::map`, `iter::filter`, `iter::take`, `iter::skip`, `iter::enumerate`, `iter::zip` | Store the adapter in a mutable local and iterate with `for value in ref mut adapter` when it tracks progress. `skip` is the drop-count adapter name because `drop` is a language operation. |
| Consume iterators eagerly. | `iter::fold`, `iter::reduce`, `iter::collect` | `reduce` returns `Option[T]`; `collect` needs `ref mut Zone` and returns `std::vec::Vec[T]`. |
| Work with bit masks, rotations, powers of two, byte order, and bit scans. | `bits::is_set`, `bits::rotate_left`, `bits::bit_width`, `bits::low_mask`, `bits::align_up`, `bits::byte_swap`, `bits::population_count`, `bits::leading_ones` | Current helpers take `u64`. Rotate counts are non-negative and wrap modulo 64; alignment helpers assert a non-zero power-of-two alignment. `population_count` aliases `count_ones`; `byte_swap` reverses the eight bytes. Zero-run helpers return `64` for `0u64`; one-run helpers return `64` for `~0u64`. |
| Plan OS-facing code. | `std::target`, `std::env`, `std::process`, `std::thread`, `std::sync`, `std::time`, `std::fs`, `std::mem`, `std::random`, and source `std::net` values today; future `std::os` | Args, current-process environment variables, current directory/executable path, process id/uid/gid/exit/abort, POSIX fork/wait, function-pointer thread spawn/join/sleep/yield, available parallelism, concrete atomic i64 operations, source `Mutex`/`RwLock`/`Once`, monotonic/wall-clock reads, sleep, byte-oriented file handles, access permissions, path rename, single-directory create/remove, runtime page size, OS entropy through `getrandom` with `/dev/urandom` fallback, network address values, and target facts are implemented. Portable process spawn/exec/kill, richer thread statuses, user thread-local storage, custom thread stack sizes, shared ownership, generic synchronization, Condvar/OnceLock/LazyLock/channels/futex-backed blocking, filesystem metadata and permission mutation, directory iteration, recursive directories, canonical paths, temporary files, network sockets/DNS/options, optional locking, raw syscall wrappers, fd abstractions, fcntl/ioctl/poll/select, epoll/inotify/eventfd/timerfd/signalfd/pidfd/memfd/io_uring, signal mask/sigaction/alt-stack, mmap/munmap/mprotect/msync/mlock/madvise, fallible entropy, and build hardening profiles need runtime and ownership policy work. |
| Choose between collection families. | `Set`, `Deque`, `RingBuffer`, `LinkedList`, `BinaryHeap`/`PriorityQueue`, `HashMap`/`HashSet`, `TreeMap`/`TreeSet` | Use `Set` for small insertion-order unique lists, `Deque` for both-end queues, `RingBuffer` for bounded FIFO storage, `LinkedList` for linked front/back operations, heaps for priority removal, hash containers for average-case lookup, and tree containers for ordered lookup. Hash/tree/heap containers currently take explicit hash/comparator functions until trait-driven constructors land. |
| Implement custom iteration. | `Iterator[T]::next(self: ref mut Self) -> Option[T]` | Use `for item in iterator`; use `for let pattern in iterator` for skip-on-mismatch filtering. Implementors should return `Some(value)` until exhausted and then `None<T>()`. |
| Format into owned text. | `format_in!(ref mut zone, "...", values...)`, `fmt::unsigned_in(ref mut zone, value, spec)`, `fmt::integer_in(ref mut zone, value)`, `fmt::debug_value(ref mut zone, value)` | Default-zone `format!` is intentionally not executable in the current surface. `{}` calls `Display`, `{:?}` calls `Debug`, and `FormatSpec` covers source binary/octal/hex, width, precision, and alignment control. |
| Format into a writer. | `fmt::write_unsigned<W>(ref mut writer, ref mut zone, value, spec)` | `write_*` helpers take a temporary zone explicitly and work with any `std::io::Writer`. |
| Use integer helper routines. | `math::abs`, `math::is_positive`, `math::checked_add`, `math::checked_mul`, `math::checked_div`, `math::checked_rem`, `math::wrapping_add`, `math::wrapping_mul`, `math::overflowing_add`, `math::overflowing_mul`, `math::saturating_add`, `math::saturating_mul`, `math::saturating_div`, `math::pow`, `math::div_floor`, `math::mod_floor`, `math::gcd`, `math::lcm` | Current helpers have i64 signatures and natural names so they can grow into generic APIs later. Checked helpers return `Option[i64]`; overflowing helpers return the tuple `(value, overflowed)`; saturating helpers clamp to i64 bounds where meaningful. `checked_mul` guards before multiplying, checked division/remainder return `None` for zero divisors and the `i64_min / -1` edge, `pow` asserts that the exponent is non-negative, division rounding helpers assert a non-zero representable quotient, and `lcm` returns `0` if either input is `0`. |
| Validate and inspect UTF-8 byte strings. | `text.is_utf8()`, `text.codepoint_count()`, `text.codepoint_at(byte_index)`, `text.push_codepoint_in(ref mut zone, scalar)`, `encoding::utf8_at(bytes, byte_index)`, `encoding::try_encode_utf8_in(ref mut zone, scalar)` | Ari `String` is still byte-oriented. These helpers validate and count Unicode scalar values at byte offsets; they do not implement grapheme clusters, normalization, or locale-sensitive case conversion. Use `try_encode_utf8_in` when scalar validity comes from input. |
| Share code with C. | `extern "C"`, C ABI aliases such as `c_int`/`c_char`/`c_void`, `std::c::CStr`, `std::c::CString`, `std::c::Library`, `std::c::Symbol`, `@repr(C)`, `@export`, `--shared`, `--emit-c-header` | Use `c::from_string("name").as_ptr()` for borrowed C strings, `c::from_slice_in(ref mut zone, bytes)` for owned NUL-terminated storage, `c::errno()`/`c::error()` for sentinel-plus-errno C APIs, and `symbol.function<fn(...) -> ...>()` for loaded dynamic functions. Do not pass Ari ownership across C directly. Zone-backed `CString` direct handoff to arbitrary externs is still behind an explicit FFI escape policy. |

## Common Method Groups

`Option[T]`:

```ari
value.is_some()
value.is_none()
value.is_some_and(fn_name)
value.is_none_or(fn_name)
value.unwrap_or(fallback)
value.unwrap_or_else(fn_name)
value.unwrap()
value.expect()
value.map<U>(fn_name)
value.and_then<U>(fn_name)
value.filter(fn_name)
value.flatten()
value.transpose()
value.or(other)
value.or_else(fn_name)
value.xor(other)
value.ok_or<E>(error)
value.ok_or_else<E>(fn_name)
```

`Result[T, E]`:

```ari
value.is_ok()
value.is_err()
value.is_ok_and(fn_name)
value.is_err_and(fn_name)
value.unwrap_or(fallback)
value.unwrap_or_else(fn_name)
value.unwrap()
value.expect()
value.unwrap_err()
value.expect_err()
value.ok()
value.err()
value.map<U>(fn_name)
value.map_err<F>(fn_name)
value.and_then<U>(fn_name)
value.or<F>(fallback)
value.or_else<F>(fn_name)
value.transpose()
```

The plain case predicates borrow the enum value. The `*_and` and `*_or`
predicate helpers consume the enum and pass its payload to the given function.

`Slice[T]`, `std::vec::Vec[T]`, `std::collections::Set[T]`, and
`std::string::String` share a small
collection vocabulary where the operation makes sense:

```ari
value.len()
value.is_empty()
value.first()
value.try_first()
value.last()
value.try_last()
value.get(index)
value.try_get(index)
value.contains(item)
value.index_of(item)
value.count(item)
value.find(needle)
value.contains_slice(needle)
value.compare(other)
value.slice(start, end)
value.split_at(index)
value.chunks(size)
value.windows(size)
value.split(delimiter)
value.as_ptr()
value.as_slice()
value.copy_to(ref mut zone)
```

The non-`try` accessors assert on bad indexes. `try_first`, `try_last`, and
`try_get` return `Option[T]` for generic collections and `Option[u8]` for
`String`; they are the better choice for normal control flow. `find`,
`contains_slice`, `compare`, `slice`, `split_at`, `chunks`, `windows`, and
delimiter `split` are available on `Slice[T]` and `std::vec::Vec[T]`; on
`String` the same names operate on byte views.

`std::collections::Set[T]` also includes `insert(ref mut zone, value)`,
`replace(ref mut zone, value)`, `remove(value)`, `take(value)`, `pop()`,
`try_pop()`, `reserve(ref mut zone, capacity)`,
`reserve_extra(ref mut zone, additional)`, and `clear()`. Its accessors,
`index_of`, `as_slice`, and `iter()` preserve insertion order, and the handle
implements `IntoIterator[T]` for direct `for value in set` loops.

`std::collections::HashMap[K,V]` and `TreeMap[K,V]` share `len`, `capacity`,
`is_empty`, `contains`, `get`, `try_get`, `insert(ref mut zone, key, value)`,
`reserve(ref mut zone, capacity)`, `clear`, `keys()`, and `values()`.
`HashMap` also has `remove(key)`. Hash map iterators walk live buckets;
tree map iterators walk ascending key order.

`std::collections::HashSet[T]` and `TreeSet[T]` share `len`, `capacity`,
`is_empty`, `contains`, `insert(ref mut zone, value)`,
`replace(ref mut zone, value)`, `reserve(ref mut zone, capacity)`, and
`clear`. Both have `iter()` and direct `for value in set` support. `HashSet`
also has `take(value)` and `remove(value)`. Hash set iteration walks live
buckets; tree set iteration walks ascending comparator order.

`std::fs::File` methods include `invalid`, `is_open`, `close`, `read_byte`,
`write_byte`, and `write_bytes`. Use `fs::try_open(path, mode)` to obtain an
`Option[File]` before calling them, or `fs::try_create(path)` for the common
create/truncate case. Supported modes are `"r"`, `"w"`, `"a"`, `"rw"`,
`"r+"`, `"w+"`, and `"a+"`. The current handle is copyable source data, so
closing is a caller convention until the language has a stronger OS-resource
ownership model.

`std::fs::Permissions` is a small access snapshot. Use
`fs::can_read(path)`, `fs::can_write(path)`, and `fs::can_execute(path)` for
direct checks, or `fs::permissions(path)` when you want to pass the three
booleans around as one value. These checks can race with filesystem changes, so
they are for diagnostics and preflight behavior; real open/read/write calls
still need normal failure handling.

For small whole-file byte work, `fs::try_write(path, bytes)` truncates or
creates and returns the written byte count, `fs::try_append(path, bytes)`
appends and returns the appended byte count, `fs::truncate(path)` creates or
empties a file, `fs::try_copy(source, target)` streams bytes into a truncating
target and returns the copied byte count, and `fs::try_read(ref mut zone, path)`
reads into Ari's zone-backed byte-oriented `String`. The shorter
`write`/`append`/`copy` helpers are bool wrappers, and `read`/`read_to_string`
keep the compatibility empty-string fallback. Use `try_read` when empty-file and
missing-file cases must be separated.

For path-level changes, `fs::rename(source, target)` moves or renames one path
using the host runtime policy. For directories, `fs::create_dir(path)` creates
one directory and `fs::remove_dir(path)` removes one empty directory. Parent
creation, recursive removal, and directory iteration are still roadmap work.

`std::net` address values are ordinary source structs. Use
`net::Ipv4Addr::localhost()`, `net::Ipv6Addr::any()`, `addr.as_ip()`, and
`net::socket_addr(ip, port)` to build addresses without opening sockets. The
first slice intentionally stops before DNS and socket handles because those
need owned OS-resource and error policy.

`std::vec::Vec[T]` mutating methods include `push`, `pop`, `try_pop`, `set`,
`replace`, `swap`, `insert`, `remove`, `truncate`, `clear`, `reserve`,
`reserve_extra`, `extend_from_slice`, `resize`, and their explicit-zone `_in`
forms where applicable.

`std::string::String` mutating methods are byte-oriented and include `push`,
`pop`, `try_pop`, `set`, `replace`, `insert`, `truncate`, `clear`, `reserve`,
`reserve_extra`, `append`, `append_byte`, `append_bytes`, `append_string_in`,
`append_i64_in`, `append_u64_in`, `append_bool_in`, `append_f32_in`,
`append_f64_in`, `append_value_in[T: fmt::Display]`, and the explicit-zone
`_in` forms. Tracked local strings can use `append_value(value)` to avoid
type-specific append names for user-defined display values. Single-byte text-like
parameters are spelled `char`, Ari's alias for ASCII `u8`, so call sites can
prefer `'/'`, `'0'`, and `'!'` over decimal byte literals.

`trim_start`, `trim_end`, and `trim` return borrowed `Slice[u8]` views. Use
`trimmed_start(ref mut zone)`, `trimmed_end(ref mut zone)`,
`trimmed(ref mut zone)`, or the older `*_to` names when the trimmed bytes need
to be copied into a zone and survive after the source string's zone is reset.

`std::string::String` read-only byte helpers also include `find_text`,
`contains_text`, `starts_with`, `starts_with_text`, `ends_with`,
`ends_with_text`, `equals`, `equals_text`, `equals_ignore_case`,
`equals_text_ignore_case`, `starts_with_ignore_case`,
`starts_with_text_ignore_case`, `ends_with_ignore_case`,
`ends_with_text_ignore_case`, `index_of_ignore_case`,
`index_of_text_ignore_case`, `contains_ignore_case`, `contains_text_ignore_case`,
`try_utf8`, `trim_start`, `trim_end`, `trim`, `parse_decimal`,
`parse_decimal_prefix`, `parse_hex`, and `parse_hex_prefix`. The plain ASCII
trim helpers return borrowed byte slices, the `*_to` trim helpers return
copied strings in a target zone, whole parsers return `Option[i64]`, and prefix
parsers return `Option[std::ascii::ParsedInt]`.

`std::boxed::Box[T]` methods include `get`, `set`, `replace`, `take`,
`try_take`, `clear`, `put_in`, `copy_to`, `as_ref`, `as_mut`, `as_ptr`,
`as_mut_ptr`, `swap`, and `is_empty`.

## Example

```ari
impl cmp::Ord[i64] for i64 {
  fn lt(self, other: i64) -> bool {
    return self < other;
  }
}

fn main() -> i64 {
  var zone = zone::create(256);

  var numbers = std::vec::new<i64>(ref mut zone, 2);
  numbers.push(3);
  numbers.push(5);

  var text = std::string::new(ref mut zone, 4);
  text.push('A');
  text.push('B');

  var boxed = std::boxed::new<i64>(ref mut zone, 7);
  let score =
    numbers.len() +
    numbers.get(0) +
    text.len() +
    (text.first() as i64) +
    boxed.get() +
    cmp::max<i64>(4, 9);

  zone::destroy(zone);
  return score;
}
```

## Needed Library Families

These are the broad libraries Ari should grow toward. Each step should land as
small source APIs with focused tests before becoming a larger design promise.

| Family | Why Ari Needs It | Likely First Modules |
| --- | --- | --- |
| Foundation | Programs need stable ADTs, traits, assertions, and low-level helpers before higher-level APIs can be trusted. | `std`, `std::option`, `std::result`, `std::cmp`, `std::convert`, `std::mem`. |
| Allocation | Ari's memory model is explicit, so allocation must be visible and capability-based. | `std::zone`, future allocator traits, future scoped scratch helpers. |
| Collections | Most programs need growable storage, membership checks, ordered lookup, queues, priority queues, and borrowed views. | `std::vec`, `std::boxed`, `std::collections::Set`, `Deque`, `RingBuffer`, `LinkedList`, `BinaryHeap`, `PriorityQueue`, `HashMap`, `HashSet`, `TreeMap`, `TreeSet`, future tree deletion and trait-driven collection constructors. |
| Text And Formatting | Diagnostics, CLI tools, and user programs need owned text, byte helpers, and formatting. | `std::string`, `std::ascii`, `std::fmt`, formatting macros. |
| IO And Process Context | Programs need arguments, environment variables, stdin/stdout/stderr, process status, files, child processes, threads, and synchronization. | `std::io`, `std::input`, `std::context`, `std::env`, `std::process`, `std::thread`, `std::sync`, `std::fs`. |
| Iteration | Collections and ranges need a shared loop protocol. | `std::iter`, collection iterators. |
| Numerics | Systems programs need reliable arithmetic and bit helpers beyond operators. | `std::math`, `std::bits`, current checked add/sub/mul/div/rem/neg/abs, wrapping/overflowing add/sub/mul, saturating add/sub/mul/div/neg/abs, byte-swap, population count, and future generic numeric traits. |
| Randomness | Tests, simulations, randomized algorithms, and shuffling need repeatable streams, while seeds and tokens need host entropy. | `std::random`, current OS entropy, deterministic non-crypto `Prng`, bounded integers, unit floats, byte filling, and shuffling. |
| Testing And Diagnostics | Library work needs source-level tests and stable failure reporting. | current `std::test::Report` and `std::log` line helpers; future runner discovery, richer panic/assert messages, source locations, structured logging, stack/backtrace, optional benchmarks, and optional fuzz hooks. |
| C Interop | Ari should call C libraries without making the standard library depend on a C++ ABI. | `extern "C"` declarations, `std::c` C string/errno/dynamic-loader helpers, typed dynamic function symbols, and future platform-specific low-level OS modules. |

## Adding A Public API

1. Put source declarations and source implementations under `lib/std.arih` or
   `lib/std/<module>.arih`.
2. Add compiler support only when source Ari cannot express the primitive yet.
3. Add positive tests under `tests/cases/standard-library/ok/<feature>/` and
   negative diagnostics under
   `tests/cases/standard-library/errors/<feature>/`.
4. Add or update `tests/std_api_manifest.txt` for public declarations.
5. Document the user-facing behavior here or in a focused language page.
6. Update the developer roadmap or test matrix when the API changes compiler
   behavior, ownership rules, ABI, or runtime lowering.
