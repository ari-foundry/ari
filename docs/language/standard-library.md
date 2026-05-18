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
| `std` root | Common prelude surface and shared ADTs. | `Option[T]`, `Result[T, E]`, `Slice[T]`, `Range[T]`, `RangeInclusive[T]`, `move`, `take`, assertion helpers, panic helpers, root `Box`, `String`, and `Vec` aliases. | Implemented source surface with compiler-known hooks for selected helpers. |
| `std::option` | Convenience methods for optional values. | `is_some`, `is_none`, `unwrap_or`, `unwrap_or_else`, `unwrap`, `expect`, `map`, `or`, `or_else`, `xor`, `and_then`, `ok_or`, `ok_or_else`. | Implemented for the current generic enum model. |
| `std::result` | Error-return convenience methods. | `is_ok`, `is_err`, `unwrap_or`, `unwrap_or_else`, `unwrap`, `expect`, `unwrap_err`, `expect_err`, `ok`, `err`, `map`, `map_err`, `and_then`, `or_else`. | Implemented for the current generic enum model. |
| `std::io` | Minimal process IO hooks. | `write_i64`, `write_u64`, `write_bool`, `write_byte`, `newline`, `read_byte`, `read_line`, `read_line_owned`. | Runtime-backed through reserved `extern "ari"` builtins. |
| `std::input` | Friendly input aliases. | `read_byte`, `line`, `owned_line`. | Runtime-backed through `std::io`-style builtins. |
| `std::context` | Program argument access. | `argc`, `arg`. | Runtime-backed; initialized by the generated entry wrapper. |
| `std::mem` | Layout and raw pointer helpers. | `size_of`, `align_of`, `ptr_offset`, `ptr_add`, `ptr_load`, `ptr_store`, `replace`, `swap`. | Compiler-lowered where layout or typed pointer semantics are required. |
| `std::zone` | Explicit allocation capability. | `create`, byte `alloc`, typed `alloc[T]`, `new[T]`, `promote[T]`, `reset`, `destroy`, `allocation_zone`. | Runtime-backed with ownership/provenance checks in sema. |
| `std::boxed` | Zone-backed single-value owner handle. | `Box[T]`, `new`, `Box::new`, `get`, `set`, `replace`, `take`, `try_take`, `clear`, `put_in`, `copy_to`, `as_ref`, `as_mut`, `swap`, raw pointer access. | Implemented as an explicit-zone seed for future smart-pointer work. |
| `std::string` | Zone-backed owned byte string seed. | `String`, `RawString`, capacity constructors, copy helpers, byte get/set/search, growth, append helpers, `as_slice`, `as_ptr`. | Implemented as a byte string. Full text/Unicode policy is still future work. |
| `std::ascii` | ASCII-only byte helpers for byte strings and parsers. | `is_digit`, `is_alpha`, `is_alphanumeric`, `is_whitespace`, `is_hex_digit`, `to_lower`, `to_upper`, `digit_value`, `hex_value`. | Implemented in Ari source; not a Unicode or locale-aware text API. |
| `std::vec` | Zone-backed growable sequence seed. | `Vec[T]`, `RawVec[T]`, `Iter[T]`, constructors, metadata, checked element access, mutation, growth, copy, slice view, raw pointer access, iterator support. | Implemented as explicit-zone source `Vec`; root bare `Vec[T]` is still the compiler-known local vector type. |
| `std::iter` | Iteration traits and range constructors. | `range`, `range_inclusive`, `Iterator[T]`, `IntoIterator[T]`, `Iterable[T]`. | Range lowering and `std::vec::Iter` are implemented; general iterator protocols are still growing. |
| `std::fmt` | Formatting traits. | `Debug`, `Display::format_in`. | Trait surface is present; formatting macros still use compiler lowering. |
| `std::cmp` | Comparison traits and helpers. | `Eq`, `PartialEq`, `Ord`, `PartialOrd`, `min`, `max`, `clamp`. | Implemented for source-level trait-bound static dispatch. |
| `std::convert` | Conversion trait names. | `From`, `Into`, `TryFrom`, `TryInto`. | Trait surface only; broad conversion impls are future library work. |
| `std::math` | Source-only numeric helpers. | `abs`, `sign`, `is_even`, `is_odd`, `pow`, `gcd`. | First i64-signature helper slice; overflow policy is still future work. |
| `std::bits` | Source-only bit-mask and power-of-two alignment helpers. | `is_set`, `any_set`, `set`, `clear`, `toggle`, `is_power_of_two`, `align_down`, `align_up`. | First u64-signature helper slice; generic integer policy is future work. |

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

## Task-Oriented API Guide

Use this table when writing code from docs alone:

| Task | Preferred API | Notes |
| --- | --- | --- |
| Print debug or user-facing output. | `print`, `println`, `print!`, `println!` | Format strings must be string literals. Use `{}` for strings, integers, bools, and `f32`/`f64`; use `{:.N}` for float precision. |
| Read process arguments. | `arg_count()`, `arg(index)`, `context::argc()`, `context::arg(index)` | Arguments are lowercase `string` values. Out-of-range `arg` returns an empty string. |
| Read stdin. | `input()`, `read_line()`, `input_owned(ref mut zone)` | Borrowed line input reuses an internal buffer. Owned line input copies into `std::string::String`. |
| Represent missing values. | `Option[T]`, `Some(value)`, `None<T>()` | Use `.unwrap_or`, `.map<U>`, `.and_then<U>`, `?`, or `??` when that reads better than `match`. |
| Convert missing values into failures. | `option.ok_or<E>(error)`, `option.ok_or_else<E>(op)` | Lazy form builds the error only for `None`. |
| Represent success/failure. | `Result[T, E]`, `Ok<T, E>(value)`, `Err<T, E>(error)` | `?` propagates residual cases and runs hidden iterator cleanup when needed. |
| Convert failures back to optional values. | `result.ok()`, `result.err()` | Keeps only the selected payload branch. |
| Work with borrowed contiguous data. | `Slice[T]`, `slice(data, len)`, `.as_slice()` | Slice methods borrow the view; `copy_to(ref mut zone)` makes a new owned collection when available. |
| Store a small local literal sequence. | Bare `Vec[T]` from `[a, b, c]` | This is compiler-known local vector storage, not `std::vec::Vec[T]`. Empty `[]` needs an expected type. |
| Store a growable source collection. | `std::vec::new<T>(ref mut zone, capacity)` | Common tracked locals can call `push`, `insert`, `reserve`, and related methods without spelling the zone again. |
| Store owned byte text. | `std::string::from_string(ref mut zone, "text")` or `std::string::new(ref mut zone, capacity)` | The handle stores bytes, not a full Unicode text abstraction yet. |
| Classify or convert ASCII bytes. | `ascii::is_digit`, `ascii::to_lower`, `ascii::hex_value` | Takes `u8`. Digit parsers return `Option[i64]`; non-ASCII text policy is future work. |
| Store one zone-backed value. | `std::boxed::new<T>(ref mut zone, value)` or `Box!(T, ref mut zone, value)` | `take()` empties the handle; `try_take()` returns `Option[T]`. |
| Allocate raw memory. | `zone::alloc`, `zone::alloc<T>`, `zone::new<T>` | Raw allocation does not run destructors or make memory safe by itself. |
| Inspect layout or raw memory. | `size_of<T>`, `align_of<T>`, `ptr_add`, `ptr_load`, `ptr_store` | Use only for scalar and supported Ari-layout aggregate values. |
| Compare values generically. | `cmp::min`, `cmp::max`, `cmp::clamp` | Requires an `Ord[T]` impl for the compared type. |
| Iterate ranges. | `range(start, end)`, `range_inclusive(start, end)`, `start..end`, `start..=end` | Works directly in `for` loops and stores as `Range[T]`/`RangeInclusive[T]`. |
| Work with bit masks. | `bits::is_set`, `bits::set`, `bits::clear`, `bits::align_up` | Current helpers take `u64`. Alignment helpers assert a non-zero power-of-two alignment. |
| Implement custom iteration. | `Iterator[T]::next(self: ref mut Self) -> Option[T]` | Use `for item in iterator`; use `for let pattern in iterator` for skip-on-mismatch filtering. |
| Format into owned text. | `format_in!(ref mut zone, "...", values...)` | Default-zone `format!` is intentionally not executable in the current surface. |
| Use integer helper routines. | `math::abs`, `math::pow`, `math::gcd` | Current helpers have i64 signatures and are source implemented. `pow` asserts that the exponent is non-negative. |
| Share code with C. | `extern "C"`, `@repr(C)`, `@export`, `--shared`, `--emit-c-header` | Do not pass Ari ownership across C directly. Use explicit pointers, borrows, or wrappers. |

## Common Method Groups

`Option[T]`:

```ari
value.is_some()
value.is_none()
value.unwrap_or(fallback)
value.unwrap_or_else(fn_name)
value.unwrap()
value.expect()
value.map<U>(fn_name)
value.and_then<U>(fn_name)
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
value.or_else<F>(fn_name)
```

`Slice[T]`, `std::vec::Vec[T]`, and `std::string::String` share a small
collection vocabulary where the operation makes sense:

```ari
value.len()
value.is_empty()
value.first()
value.last()
value.get(index)
value.contains(item)
value.index_of(item)
value.count(item)
value.as_ptr()
value.as_slice()
value.copy_to(ref mut zone)
```

`std::vec::Vec[T]` mutating methods include `push`, `pop`, `try_pop`, `set`,
`replace`, `swap`, `insert`, `remove`, `truncate`, `clear`, `reserve`,
`reserve_extra`, `extend_from_slice`, `resize`, and their explicit-zone `_in`
forms where applicable.

`std::string::String` mutating methods are byte-oriented and include `push`,
`pop`, `set`, `replace`, `insert`, `truncate`, `clear`, `reserve`,
`reserve_extra`, `extend_from_slice`, `resize`, `append_string`,
`append_i64`, `append_u64`, `append_bool`, `append_f32`, `append_f64`, and
the explicit-zone `_in` forms.

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
| Text And Formatting | Diagnostics, CLI tools, and user programs need owned text, byte helpers, and formatting. | `std::string`, `std::ascii`, `std::fmt`, formatting macros. |
| IO And Process Context | Programs need arguments, stdin/stdout, and eventually files and environment access. | `std::io`, `std::input`, `std::context`, future `std::fs`, `std::env`, `std::process`. |
| Iteration | Collections and ranges need a shared loop protocol. | `std::iter`, collection iterators. |
| Numerics | Systems programs need reliable arithmetic and bit helpers beyond operators. | `std::math`, `std::bits`, future integer checked/wrapping helpers. |
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
