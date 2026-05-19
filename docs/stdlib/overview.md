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

## Module Map

| Module | Purpose | First Things To Use |
| --- | --- | --- |
| `std` | Prelude root, shared ADTs, root aliases. | `Option`, `Result`, `Slice`, `try_get`, `move`, `take`, `assert`, `panic`. |
| `std::option` | Convenience methods for optional values. | `is_some`, `is_none`, `is_some_and`, `is_none_or`, `unwrap_or_else`, `map`, `and_then`, `filter`, `flatten`, `transpose`, `ok_or`. |
| `std::result` | Convenience methods for success/failure values. | `is_ok`, `is_err`, `is_ok_and`, `is_err_and`, `unwrap_or_else`, `ok`, `err`, `map_err`, `or`, `transpose`. |
| `std::io` | Byte-oriented process IO contracts and hooks. | `Reader`, `Writer`, `Seek`, `Stdin`, `Stdout`, `Cursor`, `BufReader`, `BufWriter`, `stdin`, `stdout`, `cursor`, `buf_reader`, `buf_writer`, `read_exact`, `write_all`, `flush`, `write_bytes`, `read_line`. |
| `std::input` | Friendly stdin helpers. | `line`, `owned_line`, `read_byte`, `try_read_byte`. |
| `std::context` | Low-level runtime context access. | `argc`, `arg`, `thread_id`, `has_arg`, `user_arg_count`, `is_main_thread`. |
| `std::env` | User-facing process argument, environment-variable, and path-state helpers. | `arg_count`, `try_arg`, `program_name`, `get`, `try_get`, `set`, `remove`, `current_dir`, `try_current_dir`, `set_current_dir`, `executable_path`. |
| `std::process` | Current-process helpers and POSIX child-process control. | `id`, `exit`, `success`, `failure`, `is_success`, `is_failure`, `fork`, `wait`, `is_child`, `is_parent`, `is_fork_error`, `is_wait_error`. |
| `std::thread` | Function-pointer thread spawn/join and runtime ids. | `Thread`, `spawn`, `join`, `yield_now`, `id`, `is_main`, `is_join_error`. |
| `std::sync` | Small explicit synchronization primitives. | `AtomicI64`, `load`, `store`, `swap`, `fetch_add`, `compare_exchange`. |
| `std::time` | Monotonic time, wall-clock time, and sleep. | `Duration`, `Instant`, `SystemTime`, `nanoseconds`, `milliseconds`, `seconds`, `now`, `system_now`, `elapsed`, `sleep`. |
| `std::fs` | Byte-oriented filesystem handles. | `File`, `exists`, `remove`, `open`, `try_open`, compatibility `open_read`/`open_write`/`open_append`, `read_byte`, `write_byte`, `write_bytes`, whole-file `write`, `append`, `read_to_string`, `close`. |
| `std::mem` | Layout and raw pointer operations. | `size_of`, `align_of`, `ptr_add`, `ptr_load`, `ptr_store`, `replace`, `swap`. |
| `std::zone` | Explicit allocation capability. | `create`, `alloc`, `alloc<T>`, `alloc_array<T>`, `new<T>`, `promote<T>`, `reset`, `destroy`. |
| `std::boxed` | Zone-backed single-value owner. | `Box[T]`, `new`, `get`, `set`, `take`, `try_take`, `copy_to`. |
| `std::string` | Zone-backed owned byte string. | `String`, `new`, `from_string`, `push`, `try_get`, `try_pop`, `append_i64_in`, `equals_ignore_case`, `index_of_ignore_case`, `trim`, `trim_to`, `parse_decimal`, `parse_decimal_prefix`, `as_slice`. |
| `std::ascii` | Source-only ASCII byte and slice helpers. | `is_digit`, `is_printable`, `equals_ignore_case`, `index_of_ignore_case`, `trim`, `parse_decimal`, `parse_decimal_prefix`. |
| `std::vec` | Zone-backed growable sequence. | `Vec[T]`, `new<T>`, `push`, `push_in`, `try_get`, `as_slice`, `iter`. |
| `std::collections` | Source collection handles beyond sequences. | Linear `Set[T]`, `Deque[T]`, `RingBuffer[T]`, `LinkedList[T]`, `BinaryHeap[T]`, `PriorityQueue[T]`, hash-table `HashMap[K,V]`/`HashSet[T]`, red-black-tree `TreeMap[K,V]`/`TreeSet[T]`, explicit hash/comparator constructors, lookup, insertion, replacement, removal, reserve, clear, FIFO/linked/heap iteration where applicable, live-bucket hash iteration, and sorted tree iteration. |
| `std::iter` | Range and iterator traits. | `range`, `range_inclusive`, `Iterator`, `IntoIterator`. |
| `std::fmt` | Formatting trait surface. | `Display::format_in`, `Debug`. |
| `std::cmp` | Comparison traits and helpers. | `Ord`, `min`, `max`, `clamp`, `is_between`. |
| `std::convert` | Explicit conversion trait names and helpers. | `From`, `Into`, `TryFrom`, `TryInto`, `identity`, `from`, `into`. |
| `std::math` | Source-only numeric helpers. | `abs`, `sign`, `is_positive`, `is_negative`, `is_zero`, `is_even`, `is_odd`, `pow`, `div_floor`, `div_ceil`, `mod_floor`, `gcd`, `lcm`. |
| `std::bits` | Source-only bit-mask, rotation, power-of-two, low-mask, alignment, and zero/one-run bit-scan helpers. | `is_set`, `set`, `rotate_left`, `bit_width`, `low_mask`, `align_up`, `leading_ones`. |

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

- `extern "ari"` IO, panic, environment, process, thread, sync, string
  allocation, and zone runtime hooks.
- layout queries and typed pointer operations in `std::mem`.
- formatting macros, because they inspect literal format strings.
- provenance checks for zone-backed handles and raw pointers.

`std::ascii` is an example of the preferred path: it is ordinary Ari source
because byte classification, printable/control predicates, case conversion,
borrowed-slice case-insensitive comparison/search, trimming, whole-slice
integer parsing, and prefix integer parsing need no compiler knowledge.

`std::string::String` follows the same direction where it can. Its allocation
constructors and runtime copy hooks still depend on compiler-known zone/string
primitives, but byte access, empty-safe `try_*` accessors, byte search,
comparison, ASCII case-insensitive comparison/search, ASCII trim views, owned
trim copies, and whole/prefix ASCII parsing are plain source methods.

`Slice[T]`, `std::vec::Vec[T]`, and byte-oriented `std::string::String` share
the preferred collection vocabulary where it fits: `is_empty` for length
metadata, asserting `first`/`last`/`get` for programmer errors, and
`try_first`/`try_last`/`try_get` for ordinary absence handled through
`Option`.

`std::bits` follows the same rule for current `u64` mask, rotation,
power-of-two, low-mask, alignment, and source-loop zero/one-run bit-scan
helpers. Future intrinsic-backed implementations may need compiler support,
but the public edge-case behavior should stay source-defined.

`std::cmp` is also source-first. Its current helpers build on the minimal
`Ord[T]::lt` method so generic code can select, clamp, or range-check values
without compiler-known comparison intrinsics.

`std::convert` follows the same source-first path for generic conversion
helpers. `identity`, `from`, and `into` are plain Ari functions over the
module's trait surface.

`std::context` keeps the same split: `argc`, `arg`, and `thread_id` are runtime
hooks because they read the host runtime context, while `has_arg`,
`user_arg_count`, `has_user_args`, and `is_main_thread` are ordinary source
helpers that document Ari's valid-index and main-thread policies in one
reusable place.

`std::env` wraps the context hooks with the names application code should use
and adds `Option`-based argument access through `try_arg` and `program_name`.
Environment variables and process-local path state use small runtime-backed
hooks for `get`, `has`, `set`, `remove`, `current_dir`, `set_current_dir`, and
`executable_path`, with source `try_get`, `try_current_dir`, and
`try_executable_path` helpers keeping ordinary absence in `Option[string]`.

`std::process` starts with a small runtime-backed process surface: `id` reads
the host process id, `exit` terminates with an explicit status, and the status
helpers are source Ari. The first POSIX child-process slice adds runtime-backed
`fork`/`wait` plus source branch and error predicates. Portable spawn, richer
status values, and process handles are intentionally still roadmap work.

`std::thread` is the first thread slice. `spawn`, `join`, and `yield_now` are
runtime-backed because they call the host threading API and install Ari's
per-thread runtime id before source code runs. `id`, `is_main`,
`is_join_error`, and the `Thread` methods are source helpers. Capturing
closures, shared ownership, locks, and richer status values remain future
`std::sync` and richer thread-policy work.

`std::sync` now starts with `AtomicI64`. The public method names are the same
names developers expect from atomic values: `load`, `store`, `swap`,
`fetch_add`, and `compare_exchange`. The runtime hooks lower directly to LLVM
atomic operations with sequentially consistent ordering. `Shared`, `Weak`,
`Mutex`, channels, and explicit memory-order arguments remain future work.

`std::time` follows the same OS-facing pattern. `monotonic_nanos`,
`unix_nanos`, and `sleep_nanos` are runtime-backed because they call the host
clock and sleep APIs, while `Duration`, `Instant`, `SystemTime`, and the
constructor/elapsed helpers are ordinary Ari source.

`std::fs` is the first filesystem slice. `exists`, `remove`, mode-string
`open`, close, and single-byte read/write are runtime-backed because they call
host file-descriptor APIs. `try_open`, compatibility `try_open_*` wrappers,
`write_bytes`, whole-file `write`, `append`, byte-string `read_to_string`, and
the `File` methods are ordinary Ari source over the raw hooks. The handle is a
visible value today and should become a stronger owned resource when OS
resource ownership is modeled by the language.

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
`write_all`, `flush`, `Stdin`, `Stdout`, `Cursor`, `BufReader`, and
`BufWriter` are source Ari over `Slice[u8]`, raw pointers, explicit
caller-provided buffers, and the single-byte hooks. `stderr`, `pipe`, file
adapters, and zone-owning buffered constructors stay on the roadmap until
owned OS handles and resource flush/drop rules are explicit.

When adding new library code, first ask whether it can be written in Ari source
using existing modules. If yes, keep it in `lib/std/`. Add compiler support
only for a primitive the source language cannot express.
