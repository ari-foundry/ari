# Standard Library Module Guides

This folder holds focused guides for individual `std` modules. Use these pages
when the compact API reference is not enough and you need the purpose, current
limits, examples, and test files for one module.

## Available Guides

- [std::option and std::result](option-result.md): ADT helpers for absence,
  failure, conversion, and lazy fallback.
- [std::string](string.md): zone-backed owned byte-string handles, growth,
  borrowed views, byte search, ASCII case search, trim views/copies, and
  whole/prefix parsing helpers.
- [std::io](io.md): low-level process IO hooks plus `Reader`/`Writer`/`Seek`,
  `stdin`/`stdout`/`stderr`, `Cursor`, caller-buffered
  `BufReader`/`BufWriter`, exact reads, whole-slice writes, and current
  `pipe` roadmap notes.
- [std::input](input.md): stdin-facing byte and line input, including
  `try_read_byte` for `Option[u8]` EOF handling.
- [std::mem](mem.md): layout queries, raw pointer helpers, byte memory
  routines, and value `replace`/`swap`.
- [std::vec](vec.md): zone-backed growable sequence handles, borrowed slices,
  safe accessors, growth, copy, and iterator entry points.
- [std::hash](hash.md): deterministic non-cryptographic `Hasher`,
  `Hash[T]`, primitive hashing, byte-slice hashing, and collection hash
  compatibility notes.
- [std::collections](collections.md): source `Set[T]`, `Deque[T]`,
  `RingBuffer[T]`, `LinkedList[T]`, `BinaryHeap[T]`, `PriorityQueue[T]`,
  hash-table `HashMap[K,V]`/`HashSet[T]`, red-black-tree
  `TreeMap[K,V]`/`TreeSet[T]`, natural lookup/update names, explicit
  hash/comparator constructors, collection iterators, copied views, and
  explicit-zone provenance.
- [std::iter](iter.md): range constructors plus the minimal iterator traits
  used by `for` loops and collection cursors.
- [std::fmt](fmt.md): `Debug` and `Display` trait surface, plus the current
  split between `format_in!` macro lowering and source formatting traits.
- [std::cmp](cmp.md): source comparison traits, generic value selection,
  inclusive clamping, and inclusive range predicates.
- [std::algo](algo.md): source slice algorithms for sorting, binary search,
  reverse/rotate, partition, min/max/clamp, swap, fill, copy, and dedup.
- [std::convert](convert.md): explicit conversion traits plus source
  `identity`, `from`, and `into` helper calls for generic code.
- [std::context](context.md): runtime-backed program argument and thread-id
  access plus source predicates and root argument aliases.
- [std::env](env.md): user-facing process argument and environment-variable
  helpers, including `try_arg`, `program_name`, `try_get`, `set`,
  `current_dir`, `set_current_dir`, and `try_executable_path`.
- [std::process](process.md): current process id, explicit exit, status helper
  functions, and the first POSIX fork/wait child-process slice.
- [std::thread](thread.md): function-pointer thread spawn/join, runtime thread
  ids, invalid-handle sentinels, and cooperative yield.
- [std::sync](sync.md): concrete `AtomicI64` load/store/swap/fetch-add/
  compare-exchange operations and current concurrency limits.
- [std::time](time.md): runtime-backed monotonic instants, wall-clock
  timestamps, non-negative durations, elapsed-time helpers, and sleep.
- [std::fs](fs.md): runtime-backed file existence, mode-string open calls,
  close, removal, `File` value handles, `Option[File]` open helpers, and
  source byte create/read/write/append/truncate/copy plus rename,
  hard/symbolic links, and single-directory create/remove helpers.
- [std::net](net.md): source-only IPv4, IPv6, generic IP, and socket-address
  value helpers plus networking runtime roadmap.
- [std::zone](zone.md): explicit allocation capability, raw typed allocation,
  `alloc_array`, placement construction, promotion, reset, and destroy rules.
- [std::ascii](ascii.md): source-only ASCII byte classification,
  printable/control predicates, case conversion, borrowed-slice
  case-insensitive comparison/search, trimming, and digit/prefix parsing
  helpers.
- [std::math](math.md): source-only `i64` arithmetic helpers for signs, sign
  predicates, parity, powers, division rounding, greatest common divisor, and
  least common multiple.
- [std::bits](bits.md): source-only `u64` bit-mask, rotation, power-of-two,
  low-mask, alignment, and zero/one-run bit-scan helpers.

## Guide Shape

Each module guide should answer:

- why the module exists
- what the current APIs do
- which behavior is intentionally not promised yet
- which tests cover the public surface
- what compiler work would be needed for the next larger slice
