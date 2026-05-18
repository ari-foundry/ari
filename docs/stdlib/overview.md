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
| `std` | Prelude root, shared ADTs, root aliases. | `Option`, `Result`, `Slice`, `move`, `take`, `assert`, `panic`. |
| `std::option` | Convenience methods for optional values. | `is_some`, `is_none`, `unwrap_or`, `unwrap`, `map`, `and_then`. |
| `std::result` | Convenience methods for success/failure values. | `is_ok`, `is_err`, `unwrap_or`, `map`, `map_err`, `and_then`. |
| `std::io` | Minimal runtime-backed process IO. | `write_i64`, `write_u64`, `write_bool`, `write_byte`, `newline`, `read_line`. |
| `std::input` | Friendly input aliases. | `line`, `owned_line`, `read_byte`. |
| `std::context` | Process argument access. | `argc`, `arg`. |
| `std::mem` | Layout and raw pointer operations. | `size_of`, `align_of`, `ptr_add`, `ptr_load`, `ptr_store`, `replace`, `swap`. |
| `std::zone` | Explicit allocation capability. | `create`, `alloc`, `alloc<T>`, `new<T>`, `promote<T>`, `reset`, `destroy`. |
| `std::boxed` | Zone-backed single-value owner. | `Box[T]`, `new`, `get`, `set`, `take`, `try_take`, `copy_to`. |
| `std::string` | Zone-backed owned byte string. | `String`, `new`, `from_string`, `push`, `append_i64_in`, `as_slice`. |
| `std::vec` | Zone-backed growable sequence. | `Vec[T]`, `new<T>`, `push`, `push_in`, `get`, `as_slice`, `iter`. |
| `std::iter` | Range and iterator traits. | `range`, `range_inclusive`, `Iterator`, `IntoIterator`. |
| `std::fmt` | Formatting trait surface. | `Display::format_in`, `Debug`. |
| `std::cmp` | Comparison traits and helpers. | `Ord`, `min`, `max`, `clamp`. |
| `std::convert` | Conversion trait names. | `From`, `Into`, `TryFrom`, `TryInto`. |
| `std::math` | Source-only numeric helpers. | `abs_i64`, `sign_i64`, `is_even_i64`, `is_odd_i64`, `pow_i64`, `gcd_i64`. |

## Allocation Rules

Anything that allocates takes a `ref mut Zone` or returns a handle tied to a
zone. Methods with an `_in` suffix take an explicit zone for growth or copying.
For tracked local `std::vec::Vec[T]` and `std::string::String` handles, Ari can
infer the same source zone for common mutating methods.

Use `zone::destroy(zone)` when a manually created zone is no longer needed.
Pointers, strings, vectors, boxes, and slices derived from that zone become
invalid after `reset` or `destroy`, and sema rejects later use.

## Source Versus Compiler Hooks

Most helper methods are plain Ari source. Compiler hooks remain for primitives
that need backend or checker knowledge:

- `extern "ari"` IO, panic, string allocation, and zone runtime hooks.
- layout queries and typed pointer operations in `std::mem`.
- formatting macros, because they inspect literal format strings.
- provenance checks for zone-backed handles and raw pointers.

When adding new library code, first ask whether it can be written in Ari source
using existing modules. If yes, keep it in `lib/std/`. Add compiler support
only for a primitive the source language cannot express.
