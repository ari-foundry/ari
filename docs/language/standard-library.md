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
| `std` root | Common prelude surface and shared ADTs. | `Option[T]`, `Result[T, E]`, `Slice[T]` with `try_*` accessors, `Range[T]`, `RangeInclusive[T]`, `move`, `take`, assertion helpers, panic helpers, root `Box`, `String`, and `Vec` aliases. | Implemented source surface with compiler-known hooks for selected helpers. |
| `std::option` | Convenience methods for optional values. | `is_some`, `is_none`, `is_some_and`, `is_none_or`, `unwrap_or`, `unwrap_or_else`, `unwrap`, `expect`, `map`, `or`, `or_else`, `xor`, `and_then`, `filter`, `flatten`, `transpose`, `ok_or`, `ok_or_else`. | Implemented for the current generic enum model. |
| `std::result` | Error-return convenience methods. | `is_ok`, `is_err`, `is_ok_and`, `is_err_and`, `unwrap_or`, `unwrap_or_else`, `unwrap`, `expect`, `unwrap_err`, `expect_err`, `ok`, `err`, `map`, `map_err`, `and_then`, `or`, `or_else`, `transpose`. | Implemented for the current generic enum model. |
| `std::io` | Minimal process IO hooks. | `write_i64`, `write_u64`, `write_bool`, `write_byte`, `write_bytes`, `newline`, `read_byte`, `read_line`, `read_line_owned`. | Runtime-backed hooks plus source byte-slice output. |
| `std::input` | Friendly stdin helpers. | `read_byte`, `try_read_byte`, `line`, `owned_line`. | Runtime-backed hooks plus source EOF-to-Option byte handling. |
| `std::context` | Low-level runtime context access. | `argc`, `arg`, `thread_id`, `has_arg`, `user_arg_count`, `is_main_thread`. | Runtime-backed hooks plus source predicates; initialized by the generated entry wrapper. |
| `std::env` | User-facing process argument, environment-variable, and path-state helpers. | `arg_count`, `try_arg`, `program_name`, `get`, `try_get`, `set`, `remove`, `current_dir`, `try_current_dir`, `set_current_dir`, `executable_path`. | Argument wrappers over `std::context` plus runtime-backed current-process environment/path hooks. |
| `std::process` | Current-process helpers. | `id`, `exit`, `success`, `failure`, `is_success`, `is_failure`. | First runtime-backed process slice. Child process handles, spawn, wait, and fork are future work. |
| `std::mem` | Layout and raw pointer helpers. | `size_of`, `align_of`, `ptr_offset`, `ptr_add`, `ptr_load`, `ptr_store`, `replace`, `swap`. | Compiler-lowered where layout or typed pointer semantics are required. |
| `std::zone` | Explicit allocation capability. | `create`, byte `alloc`, typed `alloc[T]`, `alloc_array[T]`, `new[T]`, `promote[T]`, `reset`, `destroy`, `allocation_zone`. | Runtime-backed with ownership/provenance checks in sema plus source raw array allocation. |
| `std::boxed` | Zone-backed single-value owner handle. | `Box[T]`, `new`, `Box::new`, `get`, `set`, `replace`, `take`, `try_take`, `clear`, `put_in`, `copy_to`, `as_ref`, `as_mut`, `swap`, raw pointer access. | Implemented as an explicit-zone seed for future smart-pointer work. |
| `std::string` | Zone-backed owned byte string seed. | `String`, `RawString`, capacity constructors, copy helpers, byte get/set/search, `try_get`, `try_pop`, growth, append helpers, ASCII case compare/search, trim views, trim copies, whole and prefix parse helpers, `as_slice`, `as_ptr`. | Implemented as a byte string. Full text/Unicode policy is still future work. |
| `std::ascii` | ASCII-only byte and slice helpers for byte strings and parsers. | `ParsedInt`, `is_digit`, `is_alpha`, `is_alphanumeric`, `is_blank`, `is_whitespace`, `is_control`, `is_printable`, `is_graphic`, `is_punctuation`, `is_hex_digit`, `to_lower`, `to_upper`, `digit_value`, `hex_value`, `equals_ignore_case`, `starts_with_ignore_case`, `ends_with_ignore_case`, `index_of_ignore_case`, `contains_ignore_case`, `trim`, `parse_decimal`, `parse_decimal_prefix`, `parse_hex`, `parse_hex_prefix`. | Implemented in Ari source; not a Unicode or locale-aware text API. |
| `std::vec` | Zone-backed growable sequence seed. | `Vec[T]`, `RawVec[T]`, `Iter[T]`, constructors, metadata, checked and `Option` element access, mutation, growth, copy, slice view, raw pointer access, iterator support. | Implemented as explicit-zone source `Vec`; root bare `Vec[T]` is still the compiler-known local vector type. |
| `std::collections` | Zone-backed collection handles beyond sequences. | `Set[T]`, constructors, `from_slice_in`, metadata, insertion-order access, `try_*` access, `contains`, `index_of`, `insert`, `remove`, `take`, `pop`, `try_pop`, `reserve`, `clear`, `as_slice`, `copy_to`. | Implemented as a linear insertion-order set seed; hash maps and hash sets are future work. |
| `std::iter` | Iteration traits and range constructors. | `range`, `range_inclusive`, `Iterator[T]`, `IntoIterator[T]`, `Iterable[T]`. | Range lowering and `std::vec::Iter` are implemented; general iterator protocols are still growing. |
| `std::fmt` | Formatting traits. | `Debug`, `Display::format_in`. | Trait surface is present; formatting macros still use compiler lowering. |
| `std::cmp` | Comparison traits and helpers. | `Eq`, `PartialEq`, `Ord`, `PartialOrd`, `min`, `max`, `clamp`, `is_between`. | Implemented for source-level trait-bound static dispatch. |
| `std::convert` | Conversion trait names and helpers. | `From`, `Into`, `TryFrom`, `TryInto`, `identity`, `from`, `into`. | First source helper slice; broad conversion impls and fallible conversion methods are future work. |
| `std::math` | Source-only numeric helpers. | `abs`, `sign`, `is_positive`, `is_negative`, `is_zero`, `is_even`, `is_odd`, `pow`, `div_floor`, `div_ceil`, `mod_floor`, `gcd`, `lcm`. | Current i64-signature helper slices with natural names; overflow policy is still future work. |
| `std::bits` | Source-only bit-mask, rotation, power-of-two, low-mask, alignment, and zero/one-run bit-scan helpers. | `is_set`, `any_set`, `set`, `clear`, `toggle`, `rotate_left`, `rotate_right`, `is_power_of_two`, `bit_width`, `floor_power_of_two`, `ceil_power_of_two`, `low_mask`, `align_down`, `align_up`, `count_ones`, `count_zeros`, `leading_zeros`, `trailing_zeros`, `leading_ones`, `trailing_ones`. | Current u64-signature helper slices; generic integer policy is future work. |

## API Conventions

Allocation APIs take a `ref mut Zone` or return values tied to a zone. The
`_in` suffix means "use this explicit zone for growth or copying". For tracked
local `std::vec::Vec[T]` and `std::string::String` handles, Ari can infer the
same source zone for common methods such as `push`, `insert`, `reserve`,
`reserve_extra`, `extend_from_slice`, and `resize`. `std::collections::Set[T]`
keeps growth explicit today through `insert(ref mut zone, value)`,
`reserve(ref mut zone, capacity)`, and
`reserve_extra(ref mut zone, additional)`.

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
| Print debug or user-facing output. | `print`, `println`, `print!`, `println!`, `io::write_bytes(slice)` | Format strings must be string literals. Use `{}` for strings, integers, bools, and `f32`/`f64`; use `{:.N}` for float precision. Use `write_bytes` for raw `Slice[u8]` output. |
| Read process arguments. | `env::try_arg(index)`, `env::program_name()`, `env::arg_count()`, `env::has_arg(index)`, root `arg_count()`, `has_arg(index)` | Arguments are lowercase `string` values. Prefer `try_arg` for normal absence; out-of-range raw `arg` returns an empty string. |
| Read or edit current-process environment variables. | `env::try_get(name)`, `env::has(name)`, `env::get(name)`, `env::set(name, value)`, `env::remove(name)` | Values are borrowed host strings; copy with `std::string::from_string(ref mut zone, value)` when owned text is needed. `get` returns an empty string for missing variables, so prefer `try_get` when absence matters. |
| Inspect or terminate the current process. | `process::id()`, `process::exit(code)`, `process::success()`, `process::failure()` | `exit` terminates immediately and does not run later Ari cleanup. Spawn, wait, and fork are future process-handle work. |
| Read stdin. | `input::try_read_byte()`, `input()`, `read_line()`, `input_owned(ref mut zone)` | `try_read_byte` returns `Option[u8]` instead of the raw `-1` EOF sentinel. Borrowed line input reuses an internal buffer. Owned line input copies into `std::string::String`. |
| Represent missing values. | `Option[T]`, `Some(value)`, `None<T>()` | Use `.unwrap_or`, `.map<U>`, `.and_then<U>`, `.filter()`, `.flatten()`, `.transpose()`, `?`, or `??` when that reads better than `match`. |
| Convert missing values into failures. | `option.ok_or<E>(error)`, `option.ok_or_else<E>(op)` | Lazy form builds the error only for `None`. |
| Represent success/failure. | `Result[T, E]`, `Ok<T, E>(value)`, `Err<T, E>(error)` | Use `.map<U>`, `.and_then<U>`, `.transpose()`, `.or<F>`, `.or_else<F>`, or `?` when that reads better than `match`. |
| Convert failures back to optional values. | `result.ok()`, `result.err()` | Keeps only the selected payload branch. |
| Work with borrowed contiguous data. | `Slice[T]`, `slice(data, len)`, `.as_slice()` | Slice methods borrow the view; use `try_get` when absence is expected, and `copy_to(ref mut zone)` when an owned collection is needed. |
| Store a small local literal sequence. | Bare `Vec[T]` from `[a, b, c]` | This is compiler-known local vector storage, not `std::vec::Vec[T]`. Empty `[]` needs an expected type. |
| Store a growable source collection. | `std::vec::new<T>(ref mut zone, capacity)` | Common tracked locals can call `push`, `insert`, `reserve`, and related methods without spelling the zone again. |
| Store unique values in insertion order. | `collections::new<T>(ref mut zone, capacity)` or `Set::new<T>(ref mut zone, capacity)` | `insert(ref mut zone, value)` returns whether a value was newly added. Use `try_get`, `try_pop`, `contains`, `remove`, `take`, `reserve`, `as_slice`, and `copy_to` for the current linear set. |
| Store owned byte text. | `std::string::from_string(ref mut zone, "text")` or `std::string::new(ref mut zone, capacity)` | The handle stores bytes, not a full Unicode text abstraction yet. |
| Compare, search, trim, or parse owned ASCII byte text. | `text.equals_ignore_case(bytes)`, `text.index_of_ignore_case(bytes)`, `text.trim()`, `text.trim_to(ref mut zone)`, `text.parse_decimal()`, `text.parse_decimal_prefix()` | Case-insensitive `String` helpers fold only ASCII letters. Plain trim methods return borrowed `Slice[u8]` views; `*_to` trim methods copy into a target zone. Whole parse methods require the whole string to be valid; prefix parsers return `Option[ascii::ParsedInt]`. |
| Classify, compare, search, trim, or parse ASCII bytes. | `ascii::is_digit`, `ascii::is_printable`, `ascii::equals_ignore_case`, `ascii::index_of_ignore_case`, `ascii::to_lower`, `ascii::trim`, `ascii::parse_decimal_prefix` | Scalar helpers take `u8`; slice helpers take `Slice[u8]`. Case-insensitive comparison/search folds only ASCII letters. Whole parsers return `Option[i64]`; prefix parsers return `Option[ascii::ParsedInt]` with `value` and consumed `len`. |
| Store one zone-backed value. | `std::boxed::new<T>(ref mut zone, value)` or `Box!(T, ref mut zone, value)` | `take()` empties the handle; `try_take()` returns `Option[T]`. |
| Allocate raw memory. | `zone::alloc`, `zone::alloc<T>`, `zone::alloc_array<T>`, `zone::new<T>` | Raw allocation does not run destructors or make memory safe by itself. `alloc_array<T>` returns uninitialized contiguous storage for `count` values. |
| Inspect layout or raw memory. | `size_of<T>`, `align_of<T>`, `ptr_add`, `ptr_load`, `ptr_store` | Use only for scalar and supported Ari-layout aggregate values. |
| Compare values generically. | `cmp::min`, `cmp::max`, `cmp::clamp`, `cmp::is_between` | Requires an `Ord[T]` impl for the compared type. `is_between` is inclusive and `clamp`/`is_between` assert that `low <= high`. |
| Convert values generically. | `convert::identity`, `convert::from`, `convert::into` | `from<T, U>` uses `convert::From[T]` for destination `U`; `into<T, U>` uses `convert::Into[T]` on source `U`. |
| Iterate ranges. | `range(start, end)`, `range_inclusive(start, end)`, `start..end`, `start..=end` | Works directly in `for` loops and stores as `Range[T]`/`RangeInclusive[T]`. |
| Work with bit masks, rotations, powers of two, and bit scans. | `bits::is_set`, `bits::rotate_left`, `bits::bit_width`, `bits::low_mask`, `bits::align_up`, `bits::leading_ones` | Current helpers take `u64`. Rotate counts are non-negative and wrap modulo 64; alignment helpers assert a non-zero power-of-two alignment. Zero-run helpers return `64` for `0u64`; one-run helpers return `64` for `~0u64`. |
| Plan OS-facing code. | `std::env` and current `std::process` today; future `std::thread`, `std::sync`, `std::fs`, `std::time`, `std::os` | Args, current-process environment variables, current directory/executable path, and current process id/exit are implemented. Process spawn/fork, thread join, shared ownership/atomics, files, time, and raw syscall wrappers need runtime and ownership policy work. |
| Plan hash map/set collections. | Current `std::collections::Set`; future `HashMap` and `HashSet` | The current set is linear. Hash tables need hashing/equality traits, iterator behavior, collision policy, and explicit-zone allocation tests before landing. |
| Implement custom iteration. | `Iterator[T]::next(self: ref mut Self) -> Option[T]` | Use `for item in iterator`; use `for let pattern in iterator` for skip-on-mismatch filtering. |
| Format into owned text. | `format_in!(ref mut zone, "...", values...)` | Default-zone `format!` is intentionally not executable in the current surface. |
| Use integer helper routines. | `math::abs`, `math::is_positive`, `math::pow`, `math::div_floor`, `math::mod_floor`, `math::gcd`, `math::lcm` | Current helpers have i64 signatures and natural names so they can grow into generic APIs later. `pow` asserts that the exponent is non-negative; division rounding helpers assert a non-zero denominator; `lcm` returns `0` if either input is `0`. |
| Share code with C. | `extern "C"`, `@repr(C)`, `@export`, `--shared`, `--emit-c-header` | Do not pass Ari ownership across C directly. Use explicit pointers, borrows, or wrappers. |

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
value.as_ptr()
value.as_slice()
value.copy_to(ref mut zone)
```

The non-`try` accessors assert on bad indexes. `try_first`, `try_last`, and
`try_get` return `Option[T]` for generic collections and `Option[u8]` for
`String`; they are the better choice for normal control flow.

`std::collections::Set[T]` also includes `insert(ref mut zone, value)`,
`remove(value)`, `take(value)`, `pop()`, `try_pop()`, `reserve(ref mut zone,
capacity)`, `reserve_extra(ref mut zone, additional)`, and `clear()`. Its
accessors, `index_of`, and `as_slice` preserve insertion order.

`std::vec::Vec[T]` mutating methods include `push`, `pop`, `try_pop`, `set`,
`replace`, `swap`, `insert`, `remove`, `truncate`, `clear`, `reserve`,
`reserve_extra`, `extend_from_slice`, `resize`, and their explicit-zone `_in`
forms where applicable.

`std::string::String` mutating methods are byte-oriented and include `push`,
`pop`, `try_pop`, `set`, `replace`, `insert`, `truncate`, `clear`, `reserve`,
`reserve_extra`, `extend_from_slice`, `resize`, `append_string`,
`append_i64`, `append_u64`, `append_bool`, `append_f32`, `append_f64`, and
the explicit-zone `_in` forms.

`trim_start`, `trim_end`, and `trim` return borrowed `Slice[u8]` views. Use
`trim_start_to(ref mut zone)`, `trim_end_to(ref mut zone)`, or
`trim_to(ref mut zone)` when the trimmed bytes need to be copied into a zone
and survive after the source string's zone is reset.

`std::string::String` read-only byte helpers also include `starts_with`,
`ends_with`, `equals`, `equals_ignore_case`, `starts_with_ignore_case`,
`ends_with_ignore_case`, `index_of_ignore_case`, `contains_ignore_case`,
`trim_start`, `trim_end`, `trim`, `parse_decimal`, `parse_decimal_prefix`,
`parse_hex`, and `parse_hex_prefix`. The plain ASCII trim helpers return
borrowed byte slices, the `*_to` trim helpers return copied strings in a target
zone, whole parsers return `Option[i64]`, and prefix parsers return
`Option[std::ascii::ParsedInt]`.

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
| Collections | Most programs need growable storage, membership checks, and borrowed views. | `std::vec`, `std::boxed`, current linear `std::collections::Set`, future hash maps/sets/deques after generic aggregate monomorphization and hash traits mature. |
| Text And Formatting | Diagnostics, CLI tools, and user programs need owned text, byte helpers, and formatting. | `std::string`, `std::ascii`, `std::fmt`, formatting macros. |
| IO And Process Context | Programs need arguments, environment variables, stdin/stdout, process status, and eventually files, child processes, and threads. | `std::io`, `std::input`, `std::context`, `std::env`, `std::process`, future `std::fs`, `std::thread`, `std::sync`. |
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
