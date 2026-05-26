# Standard Library Module Guides

This folder holds focused guides for individual `std` modules. Use these pages
when the compact API reference is not enough and you need the purpose, current
limits, examples, and test files for one module.

For a task-oriented path through the examples, see
[../examples.md](../examples.md). For exact public spellings, see
[../generated/api-index.md](../generated/api-index.md).

## Result-First Naming

Recoverable stdlib operations use the natural module or method name and return
`Result[..., std::error::Error]` unless a module documents a more specific
error type such as `std::encoding::Utf8Error`. Compatibility helpers that
discard information are named explicitly: `_optional` returns `Option`,
`_or`/`_or_default` use caller-provided or empty fallbacks, `_bool` returns only
a success flag, `_unchecked` preserves older asserting or invalid-handle
behavior, and `_raw`/`_raw_result` expose low-level host or ABI shapes. Existing
`*_result` aliases are migration spellings for older Ari code; new code should
prefer the natural Result-returning names shown in each module guide.
Some lookups model normal absence as `Option` instead of failure; for example,
`std::env::var(name)` returns `Option[string]` because a missing environment
variable is common CLI configuration state, while `std::env::get(name)` keeps
the Result-returning form for callers that need `Error(NotFound)`.

## Available Guides

- [std::option and std::result](option-result.md): ADT helpers for absence,
  failure, conversion, borrowed inspection, eager/lazy chaining, and lazy
  fallback.
- [std::string](string.md): zone-backed owned byte-string handles, natural
  literal-oriented constructors and append/search helpers, borrowed views, byte
  search, ASCII case search, UTF-8 scalar helpers, typed `Utf8`/`OsStr`
  views, direct literal boundary coercions, shared `std::c::CStr`
  construction, trim views/copies, and whole/prefix parsing helpers.
- [std::io](io.md): low-level process IO hooks plus `Reader`/`Writer`/`Seek`,
  `stdin`/`stdout`/`stderr`, pipe `Reader`/`Writer` adapters, `Cursor`,
  `std::fs::File` adapters, caller-buffered `BufReader`/`BufWriter`, exact
  reads, direct `Error` result helpers, compatibility wrappers,
  whole-stream reads, whole-stream string reads, generic stream copies, and
  natural `Writer::write`/`Writer::write_all` whole-slice writes.
- [std::input](input.md): stdin-facing byte and line input, including
  `try_read_byte` for `Option[u8]` EOF handling.
- [std::mem](mem.md): layout queries, raw pointer helpers, byte memory
  routines, hosted page-size lookup, and value `replace`/`swap`.
- [Slice[T]](slice.md): root borrowed contiguous views, indexing, subslicing,
  subsequence search, comparison, lazy chunks/windows, delimiter splitting,
  copy-to-vector behavior, and direct algorithm wrappers.
- [std::vec](vec.md): zone-backed growable sequence handles, borrowed slices,
  safe accessors, growth, in-place reordering, ordered search, copy, and
  iterator entry points.
- [std::boxed](boxed.md): zone-backed single-value owner with explicit
  allocation, move-out, refill, copy-to-zone, borrow, and raw pointer helpers.
- [std::rc](rc.md): zone-backed reference-counted `Rc`, `Arc`, and `Weak`
  handles with strong/weak counts, downgrade/upgrade, and value-drop behavior.
- [std::cell](cell.md): `Cell`, `RefCell`, `OnceCell`, and `Lazy` helpers for
  interior mutability and one-time initialization.
- [std::hash](hash.md): deterministic non-cryptographic `Hasher`,
  `Hash[T]`, fixed-width integer and bool hashing, byte-slice hashing, and
  collection hash compatibility notes.
- [std::random](random.md): OS entropy, deterministic non-cryptographic
  `Prng`, booleans, unbiased bounded integers, unit floats, byte filling, and
  slice shuffling.
- [std::collections](collections.md): source `Set[T]`, `Deque[T]`,
  `RingBuffer[T]`, `LinkedList[T]`, `BinaryHeap[T]`, `PriorityQueue[T]`,
  hash-table `HashMap[K,V]`/`HashSet[T]`, red-black-tree
  `TreeMap[K,V]`/`TreeSet[T]`, natural lookup/update names, map entry update
  handles, explicit hash/comparator constructors, collection iterators, copied
  views, key-value removal, set representative lookup, comparator-order bounds,
  and explicit-zone
  provenance.
- [std::iter](iter.md): range constructors plus the minimal iterator traits
  used by `for` loops and collection cursors.
- [std::fmt](fmt.md): `Debug` and `Display` trait dispatch, source
  `FormatSpec` helpers for radix/width/precision/alignment, explicit-zone
  scalar text helpers, writer-backed/stdout formatting, and the current split
  between `format_in!` macro lowering and source formatting APIs.
- [std::cmp](cmp.md): source comparison traits, generic value selection,
  inclusive clamping, and inclusive range predicates.
- [std::algo](algo.md): source slice algorithms behind both free functions
  and `Slice[T]` receiver wrappers for sorting, binary search, reverse/rotate,
  partition, min/max/clamp, swap, fill, copy, and dedup.
- [std::convert](convert.md): explicit conversion traits plus source
  `identity`, `from`, and `into` helper calls for generic code.
- [std::context](context.md): runtime-backed program argument, startup path,
  and thread-id access plus source predicates and root argument aliases.
- [std::test](test.md): source executable unit-test reports, generic equality
  checks, scratch zones, temp path helpers, snapshot/golden comparisons,
  minimal benchmark timers, and compiler `@test` runner notes.
- [std::log](log.md): source-only stderr logging levels, byte-slice messages,
  string messages, convenience level functions, and diagnostics roadmap notes.
- [std::error](error.md): shared recoverable error categories, compact error
  values, POSIX errno mapping, root aliases, direct `Result[T, Error]`
  conversion helpers, and raw compatibility bridges for runtime/FFI code.
- [std::c](c.md): C ABI boundary helpers, borrowed and owned
  NUL-terminated strings, POSIX errno access, dynamic loading handles, and
  libc type-mapping notes.
- [std::target](target.md): compiler-known target triple, architecture, OS,
  environment/libc, object/debug format, errno ABI, syscall ABI, and Linux
  API-family predicates.
- [std::env](env.md): user-facing process argument and environment-variable
  helpers plus OS-string/path-byte views, including `try_arg`, `try_arg_os`,
  `program_name_os`, `try_get`, `try_get_os`, Result-returning
  `current_dir_path`, `set_current_dir`, and `executable_path_os`.
- [std::process](process.md): current process id, uid/gid, explicit exit and
  abort hooks, status helper functions, POSIX fork/wait direct `Error` result
  helpers, typed `ExitCode`/`ExitStatus`/`Signal` inspection, temp path
  helpers, child stream aliases, and the `Command`/`Child`/`Output`
  child-process builder.
- [std::os](os.md): non-owning file-descriptor views, `OwnedFd` close,
  duplicate, close-on-exec, nonblocking policy, owned `Pipe` pairs, and the
  roadmap for raw OS primitives.
- [std::thread](thread.md): function-pointer thread spawn/join, `ThreadId`,
  owning `JoinHandle`, `JoinError`, raw `Thread` compatibility handles,
  `Builder`, advisory completion checks, runtime thread ids, explicit detach,
  cooperative sleep/yield, Result-returning available parallelism, and
  explicit `ThreadLocal[T]` handles.
- [std::sync](sync.md): `Ordering`, concrete atomic wrappers, source `Mutex`,
  `MutexGuard`, `RwLock`, `RwLockReadGuard`, `RwLockWriteGuard`, `Once`,
  `OnceLock`, `Condvar`, `WaitTimeoutResult`, `Barrier`, capacity-1 channels,
  current memory-order/no-poison policy, and blocking/runtime roadmap.
- [std::time](time.md): runtime-backed monotonic instants, wall-clock
  timestamps, non-negative durations, elapsed-time helpers, monotonic
  deadlines/timeouts, UTC calendar conversion, and sleep.
- [std::fs](fs.md): runtime-backed file existence, mode-string and
  `OpenOptions` open calls, direct `Error` result helpers for open, mutation,
  and byte-count operations, raw compatibility result helpers,
  close, removal, `File` value handles, `Option[File]` open helpers, and
  source byte create/read/write/append/truncate/copy plus target-following and
  no-follow metadata, path-kind predicates, rename, hard/symbolic links, and
  single-directory create/ensure/remove helpers, recursive directory creation
  and removal, directory entries with lazy metadata, and non-truncating file
  ensure, including metadata access/modification/status-change timestamps.
- [std::path](path.md): source-only POSIX-style lexical path helpers for
  separators, absolute/relative checks, typed `PathBytes` views including
  direct literal coercion, borrowed components, join, and lightweight
  normalization.
- [std::net](net.md): IPv4, IPv6, generic IP, socket-address values, hosted
  IPv4/IPv6 DNS lookup, host-port and bracketed IPv6 endpoint resolution,
  IPv4/IPv6 TCP listener/stream handles, IPv4/IPv6 UDP single-byte datagrams,
  direct `Error` results with raw compatibility variants, TCP/UDP local-address
  helpers, TCP peer-address helpers, Unix stream sockets, TCP/Unix stream buffer helpers,
  nonblocking/timeout/shutdown helpers, and networking runtime roadmap.
- [std::zone](zone.md): explicit allocation capability, raw typed allocation,
  `alloc_array`, placement construction, promotion, reset, and destroy rules.
- [std::ascii](ascii.md): source-only ASCII byte classification,
  printable/control predicates, case conversion, borrowed-slice
  case-insensitive comparison/search, trimming, and overflow-checked
  digit/prefix parsing helpers.
- [std::parse](parse.md): whole-input ASCII-trimmed decimal, radix, hex,
  binary, octal integer, bool, and decimal float parsers with natural
  validation/fallback helpers and integer parse diagnostics.
- [std::encoding](encoding.md): ASCII, UTF-8, and UTF-16 validation/counting,
  detailed UTF-8 failure diagnostics, UTF-8 byte-string decode, UTF-8 scalar
  decode/encode, lowercase hex, standard base64, and fallible owned decode
  helpers.
- [std::math](math.md): source-only `i64` arithmetic helpers for signs,
  predicates, checked add/sub/mul/div/rem/neg/abs, saturating
  add/sub/mul/div/neg/abs, wrapping/overflowing add/sub/mul, powers, division
  rounding, greatest common divisor, and least common multiple.
- [std::bits](bits.md): source-only `u64` bit-mask, rotation, power-of-two,
  low-mask, strict/checked/wrapping alignment, byte-swap, population-count,
  and zero/one-run bit-scan helpers.

## Guide Shape

Each module guide should answer:

- why the module exists
- what the current APIs do
- which behavior is intentionally not promised yet
- which tests cover the public surface
- what compiler work would be needed for the next larger slice
