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
| `std::io` | Minimal runtime-backed process IO. | `write_i64`, `write_u64`, `write_bool`, `write_byte`, `write_bytes`, `newline`, `read_line`. |
| `std::input` | Friendly stdin helpers. | `line`, `owned_line`, `read_byte`, `try_read_byte`. |
| `std::context` | Low-level runtime context access. | `argc`, `arg`, `thread_id`, `has_arg`, `user_arg_count`, `is_main_thread`. |
| `std::env` | User-facing process argument, environment-variable, and path-state helpers. | `arg_count`, `try_arg`, `program_name`, `get`, `try_get`, `set`, `remove`, `current_dir`, `try_current_dir`, `set_current_dir`, `executable_path`. |
| `std::process` | Current-process helpers. | `id`, `exit`, `success`, `failure`, `is_success`, `is_failure`. |
| `std::mem` | Layout and raw pointer operations. | `size_of`, `align_of`, `ptr_add`, `ptr_load`, `ptr_store`, `replace`, `swap`. |
| `std::zone` | Explicit allocation capability. | `create`, `alloc`, `alloc<T>`, `alloc_array<T>`, `new<T>`, `promote<T>`, `reset`, `destroy`. |
| `std::boxed` | Zone-backed single-value owner. | `Box[T]`, `new`, `get`, `set`, `take`, `try_take`, `copy_to`. |
| `std::string` | Zone-backed owned byte string. | `String`, `new`, `from_string`, `push`, `try_get`, `try_pop`, `append_i64_in`, `equals_ignore_case`, `index_of_ignore_case`, `trim`, `trim_to`, `parse_decimal`, `parse_decimal_prefix`, `as_slice`. |
| `std::ascii` | Source-only ASCII byte and slice helpers. | `is_digit`, `is_printable`, `equals_ignore_case`, `index_of_ignore_case`, `trim`, `parse_decimal`, `parse_decimal_prefix`. |
| `std::vec` | Zone-backed growable sequence. | `Vec[T]`, `new<T>`, `push`, `push_in`, `try_get`, `as_slice`, `iter`. |
| `std::collections` | Source collection handles beyond sequences. | `Set[T]`, `new<T>`, `from_slice_in`, `insert`, `try_get`, `try_pop`, `reserve`, `contains`, `as_slice`, `copy_to`. |
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
infer the same source zone for common mutating methods. `std::collections::Set[T]`
keeps growth explicit today, so `insert(ref mut zone, value)` spells the
allocation capability at the call site.

Use `zone::destroy(zone)` when a manually created zone is no longer needed.
Pointers, strings, vectors, boxes, and slices derived from that zone become
invalid after `reset` or `destroy`, and sema rejects later use.

## Source Versus Compiler Hooks

Most helper methods are plain Ari source. Compiler hooks remain for primitives
that need backend or checker knowledge:

- `extern "ari"` IO, panic, environment, process, string allocation, and zone
  runtime hooks.
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

`std::process` starts with a small runtime-backed current-process surface:
`id` reads the host process id, `exit` terminates with an explicit status, and
the status helpers are source Ari. Spawn, wait, fork, and process handles are
intentionally still roadmap work.

`std::collections::Set[T]` is source Ari over typed zone allocation. Its
insertion-order accessors and `try_*` methods mirror the collection vocabulary
used by `Slice[T]` and `std::vec::Vec[T]`. The compiler only recognizes the
handle shape so zone reset/destroy invalidation and same-zone growth
diagnostics stay as strong as `std::vec::Vec[T]`.

`std::zone` keeps allocation visible. Raw byte allocation and lifecycle hooks
are runtime-backed, while `alloc_array<T>` is source Ari that packages the
common "count times `size_of<T>()` at `align_of<T>()`" pattern for library
authors.

`std::input` follows that pattern for stdin. `read_byte`, `line`, and
`owned_line` are runtime hooks, while `try_read_byte` is source Ari that turns
the raw `-1` EOF sentinel into `Option[u8]`.

`std::io` keeps raw process IO visible. Scalar and line operations are runtime
hooks, while `write_bytes` is source Ari over `Slice[u8]` and the single-byte
hook.

When adding new library code, first ask whether it can be written in Ari source
using existing modules. If yes, keep it in `lib/std/`. Add compiler support
only for a primitive the source language cannot express.
