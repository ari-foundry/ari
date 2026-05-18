# Standard Library

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
| `std` root | Common prelude surface and shared ADTs. | `Option[T]`, `Result[T, E]`, `Slice[T]`, `Range[T]`, `RangeInclusive[T]`, `move`, `take`, assertion helpers, panic helpers, root `Box`, `String`, and `Vec` aliases. | Implemented source surface with compiler-known hooks for selected helpers. |
| `std::option` | Convenience methods for optional values. | `is_some`, `is_none`, `unwrap_or`, `unwrap`, `expect`, `map`, `or`, `or_else`, `and_then`. | Implemented for the current generic enum model. |
| `std::result` | Error-return convenience methods. | `is_ok`, `is_err`, `unwrap_or`, `unwrap`, `expect`, `unwrap_err`, `expect_err`, `map`, `map_err`, `and_then`, `or_else`. | Implemented for the current generic enum model. |
| `std::io` | Minimal process IO hooks. | `write_i64`, `write_u64`, `write_bool`, `write_byte`, `newline`, `read_byte`, `read_line`, `read_line_owned`. | Runtime-backed through reserved `extern "ari"` builtins. |
| `std::input` | Friendly input aliases. | `read_byte`, `line`, `owned_line`. | Runtime-backed through `std::io`-style builtins. |
| `std::context` | Program argument access. | `argc`, `arg`. | Runtime-backed; initialized by the generated entry wrapper. |
| `std::mem` | Layout and raw pointer helpers. | `size_of`, `align_of`, `ptr_offset`, `ptr_add`, `ptr_load`, `ptr_store`, `replace`, `swap`. | Compiler-lowered where layout or typed pointer semantics are required. |
| `std::zone` | Explicit allocation capability. | `create`, byte `alloc`, typed `alloc[T]`, `new[T]`, `promote[T]`, `reset`, `destroy`, `allocation_zone`. | Runtime-backed with ownership/provenance checks in sema. |
| `std::boxed` | Zone-backed single-value owner handle. | `Box[T]`, `new`, `Box::new`, `get`, `set`, `replace`, `take`, `try_take`, `clear`, `put_in`, `copy_to`, `as_ref`, `as_mut`, `swap`, raw pointer access. | Implemented as an explicit-zone seed for future smart-pointer work. |
| `std::string` | Zone-backed owned byte string seed. | `String`, `RawString`, capacity constructors, copy helpers, byte get/set/search, growth, append helpers, `as_slice`, `as_ptr`. | Implemented as a byte string. Full text/Unicode policy is still future work. |
| `std::vec` | Zone-backed growable sequence seed. | `Vec[T]`, `RawVec[T]`, `Iter[T]`, constructors, metadata, checked element access, mutation, growth, copy, slice view, raw pointer access, iterator support. | Implemented as explicit-zone source `Vec`; root bare `Vec[T]` is still the compiler-known local vector type. |
| `std::iter` | Iteration traits and range constructors. | `range`, `range_inclusive`, `Iterator[T]`, `IntoIterator[T]`, `Iterable[T]`. | Range lowering and `std::vec::Iter` are implemented; general iterator protocols are still growing. |
| `std::fmt` | Formatting traits. | `Debug`, `Display::format_in`. | Trait surface is present; formatting macros still use compiler lowering. |
| `std::cmp` | Comparison traits and helpers. | `Eq`, `PartialEq`, `Ord`, `PartialOrd`, `min`, `max`, `clamp`. | Implemented for source-level trait-bound static dispatch. |
| `std::convert` | Conversion trait names. | `From`, `Into`, `TryFrom`, `TryInto`. | Trait surface only; broad conversion impls are future library work. |

## API Conventions

Allocation APIs take a `ref mut Zone` or return values tied to a zone. The
`_in` suffix means "use this explicit zone for growth or copying". For tracked
local `std::vec::Vec[T]` and `std::string::String` handles, Ari can infer the
same source zone for common methods such as `push`, `insert`, `reserve`,
`reserve_extra`, `extend_from_slice`, and `resize`.

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
  text.push(65u8);
  text.push(66u8);

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
| Collections | Most programs need growable storage and borrowed views. | `std::vec`, `std::boxed`, future maps/sets/deques after generic aggregate monomorphization matures. |
| Text And Formatting | Diagnostics, CLI tools, and user programs need owned text and formatting. | `std::string`, `std::fmt`, formatting macros. |
| IO And Process Context | Programs need arguments, stdin/stdout, and eventually files and environment access. | `std::io`, `std::input`, `std::context`, future `std::fs`, `std::env`, `std::process`. |
| Iteration | Collections and ranges need a shared loop protocol. | `std::iter`, collection iterators. |
| Numerics | Systems programs need reliable arithmetic helpers beyond operators. | future `std::math`, integer checked/wrapping helpers, bit utilities. |
| Testing And Diagnostics | Library work needs source-level tests and stable failure reporting. | future `std::test`, richer panic messages, diagnostics helpers. |
| C Interop | Ari should call C libraries without making the standard library depend on a C++ ABI. | `extern "C"` declarations, future thin C library wrappers. |

## Adding A Public API

1. Put source declarations and source implementations under `lib/std.arih` or
   `lib/std/<module>.arih`.
2. Add compiler support only when source Ari cannot express the primitive yet.
3. Add positive tests under `tests/cases/standard-library/ok/` and negative
   diagnostics under `tests/cases/standard-library/errors/`.
4. Add or update `tests/std_api_manifest.txt` for public declarations.
5. Document the user-facing behavior here or in a focused language page.
6. Update the developer roadmap or test matrix when the API changes compiler
   behavior, ownership rules, ABI, or runtime lowering.
