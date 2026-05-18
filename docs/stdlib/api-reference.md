# Standard Library API Reference

This is a compact guide to the current public `std` surface. The source of
truth is still `lib/std.arih`, `lib/std/*.arih`, and
`tests/std_api_manifest.txt`.

## Prelude Root

Common programs can use these names through implicit `std` loading:

```ari
Option[T]
Result[T, E]
Slice[T]
Range[T]
RangeInclusive[T]
Box[T]
String
Set[T]
std::Vec[T]
move(value)
take(place)
assert(condition)
debug_assert(condition)
panic()
todo()
unreachable()
```

Root aliases include IO, context, memory, comparison, zone, and range helpers:

```ari
write_i64(value)
write_u64(value)
write_bool(value)
write_byte(value)
write_bytes(values)
newline()
read_byte()
read_line()
read_line_owned(ref mut zone)
input()
input_owned(ref mut zone)
arg_count()
arg(index)
has_arg(index)
size_of<T>()
align_of<T>()
ptr_add(pointer, count)
ptr_load(pointer)
ptr_store(pointer, value)
replace(ref mut place, value)
swap(ref mut left, ref mut right)
min<T>(left, right)
max<T>(left, right)
clamp<T>(value, low, high)
is_between<T>(value, low, high)
create(capacity)
alloc(ref mut zone, bytes, align)
alloc_array<T>(ref mut zone, count)
new<T>(ref mut zone, value)
promote<T>(ref mut target, source)
reset(ref mut zone)
destroy(zone)
range(start, end)
range_inclusive(start, end)
```

## Process Context And Environment

Runtime-backed context access lives in `std::context`:

```ari
context::argc()
context::arg(index)
context::thread_id()
context::has_args()
context::has_arg(index)
context::user_arg_count()
context::has_user_args()
context::is_main_thread()
arg_count()
arg(index)
has_arg(index)
```

`has_arg(index)` is true only for `0 <= index < context::argc()`. It is the
preferred low-level guard before reading optional host arguments. `arg(index)`
returns a lowercase `string`; out-of-range access returns an empty string.
`user_arg_count()` excludes `argv[0]`, `has_user_args()` is its boolean form,
and `thread_id()` returns the Ari runtime thread id. The main thread is `0`, so
`is_main_thread()` is true for current executable builds.

Application code should usually use the user-facing `std::env` wrappers:

```ari
env::arg_count()
env::arg(index)
env::has_arg(index)
env::try_arg(index)
env::program_name()
env::get(name)
env::has(name)
env::try_get(name)
env::set(name, value)
env::remove(name)
env::current_dir()
env::try_current_dir()
env::set_current_dir(path)
env::executable_path()
env::try_executable_path()
```

`env::try_arg(index)` returns `Option[string]`, and `env::program_name()` is
the optional `argv[0]` value.

`env::try_get(name)` returns `Option[string]` for environment variables.
`env::get(name)` returns an empty string when the variable is missing, so prefer
`try_get` or `has` when absence matters. `env::set(name, value)` overwrites a
current-process variable and `env::remove(name)` unsets it; both return whether
the host accepted the request. `env::current_dir()` and
`env::executable_path()` return borrowed runtime strings, with `try_*` wrappers
for ordinary failure; `env::set_current_dir(path)` mutates the current process
working directory. Files, time, child processes, and thread APIs are still
roadmap items.

Current-process helpers live in `std::process`:

```ari
process::id()
process::exit(code)
process::success()
process::failure()
process::is_success(code)
process::is_failure(code)
```

`id()` returns the host process id as `i64`. `exit(code)` terminates the
process and does not return. The status helpers are source functions for the
common `0` success and `1` failure convention. Child process handles, spawn,
wait, and fork are still future work.

## IO And Input

`std::io` exposes low-level process IO hooks, while `std::input` keeps
stdin-oriented helper names:

```ari
io::write_i64(value)
io::write_u64(value)
io::write_bool(value)
io::write_byte(value)
io::write_bytes(values)
io::newline()
io::read_byte()
io::read_line()
io::read_line_owned(ref mut zone)

input::read_byte()
input::try_read_byte()
input::line()
input::owned_line(ref mut zone)
```

`read_byte` returns an `i64` byte value or `-1` at EOF.
`input::try_read_byte()` wraps that shape as `Option[u8]`. `write_bytes`
writes every byte in a `Slice[u8]` and returns the byte count attempted.
Borrowed line input uses a reusable runtime buffer; use the owned forms when
the line must survive later input reads.

## Memory And Zones

`std::mem` exposes layout and raw pointer helpers:

```ari
mem::size_of<T>()
mem::align_of<T>()
mem::ptr_offset<T>(pointer, bytes)
mem::ptr_add<T>(pointer, count)
mem::ptr_load<T>(pointer)
mem::ptr_store<T>(pointer, value)
mem::replace<T>(ref mut place, value)
mem::swap<T>(ref mut left, ref mut right)
```

`std::zone` exposes the explicit allocation capability:

```ari
zone::create(capacity)
zone::alloc(ref mut zone, bytes, align)
zone::alloc<T>(ref mut zone)
zone::alloc_array<T>(ref mut zone, count)
zone::new<T>(ref mut zone, value)
zone::promote<T>(ref mut target, source)
zone::allocation_zone(data)
zone::reset(ref mut zone)
zone::destroy(zone)
```

`alloc_array<T>` returns uninitialized storage for `count` consecutive `T`
values. It returns null for `0`, asserts for negative counts, and does not run
destructors for the slots; initialize before reading and prefer higher-level
handles when ownership matters.

## Option And Result

`Option[T]` models a present or missing value:

```ari
Some(value)
None<T>()
value.is_some()
value.is_none()
value.is_some_and(op)
value.is_none_or(op)
value.unwrap_or(fallback)
value.unwrap_or_else(op)
value.unwrap()
value.expect("message")
value.map<U>(op)
value.and_then<U>(op)
value.filter(op)
value.flatten()
value.transpose()
value.or(fallback)
value.or_else(op)
value.xor(other)
value.ok_or<E>(error)
value.ok_or_else<E>(op)
```

`Result[T, E]` models success or failure:

```ari
Ok<T, E>(value)
Err<T, E>(error)
value.is_ok()
value.is_err()
value.is_ok_and(op)
value.is_err_and(op)
value.unwrap_or(fallback)
value.unwrap_or_else(op)
value.unwrap()
value.expect("message")
value.unwrap_err()
value.expect_err("message")
value.ok()
value.err()
value.map<U>(op)
value.map_err<F>(op)
value.and_then<U>(op)
value.or<F>(fallback)
value.or_else<F>(op)
value.transpose()
```

Use `is_some_and`, `is_none_or`, `is_ok_and`, and `is_err_and` when a branch
depends on both the enum case and a payload predicate. These helpers consume
the enum value and pass the payload to the predicate. `Option::flatten` is
available on `Option[Option[T]]` and removes one optional layer. `filter`
keeps a `Some(T)` only when a borrowed `fn(ref T) -> bool` predicate accepts
the payload.
`Option::transpose` is available on `Option[Result[T, E]]` and turns optional
fallible work into fallible optional work. Use the lazy `*_else` forms when the
fallback is expensive or should only run on the missing/error branch. Use
`ok_or` and `ok_or_else` when an optional value needs to enter a
`Result`-returning flow. Use `ok` and `err` when a `Result` should be projected
back into an `Option`. Use `Result::or` when a fallback `Result` is already
available; use `Result::or_else` when the fallback should be built from the
error only on the `Err` branch. `Result::transpose` is available on
`Result[Option[T], E]` and turns fallible optional work back into optional
fallible work.

## Slice, Vec, Set, String, And Box

`Slice[T]` is a borrowed contiguous view:

```ari
slice(pointer, len)
view.len()
view.is_empty()
view.first()
view.try_first()
view.last()
view.try_last()
view.get(index)
view.try_get(index)
view[index]
view.as_ptr()
view.contains(value)
view.index_of(value)
view.count(value)
view.starts_with(other)
view.ends_with(other)
view.equals(other)
view.copy_to(ref mut zone)
```

`first`, `last`, and `get` assert when the requested element does not exist.
Use `try_first`, `try_last`, and `try_get` when absence is an ordinary branch;
they return `Option[T]`. `is_empty` is a source method that borrows the view
and checks whether the stored length is zero.

`std::vec::Vec[T]` is the source growable sequence:

```ari
std::vec::new<T>(ref mut zone, capacity)
Vec!(T, ref mut zone, capacity)
vec.len()
vec.capacity()
vec.is_empty()
vec.first()
vec.try_first()
vec.last()
vec.try_last()
vec.get(index)
vec.try_get(index)
vec.get_ref(index)
vec.get_mut(index)
vec.push(value)
vec.push_in(ref mut zone, value)
vec.pop()
vec.try_pop()
vec.set(index, value)
vec.replace(index, value)
vec.swap(left, right)
vec.insert(index, value)
vec.insert_in(ref mut zone, index, value)
vec.remove(index)
vec.truncate(length)
vec.clear()
vec.reserve(ref mut zone, capacity)
vec.reserve_extra(ref mut zone, additional)
vec.extend_from_slice_in(ref mut zone, values)
vec.resize_in(ref mut zone, length, value)
vec.as_slice()
vec.as_ptr()
vec.as_mut_ptr()
vec.copy_to(ref mut zone)
vec.iter()
```

The `try_*` accessors return `Option[T]` for empty or out-of-range reads.
Use the non-`try` forms when absence is a programmer error and an assertion is
the desired behavior.

`std::collections::Set[T]` is a zone-backed linear set:

```ari
collections::new<T>(ref mut zone, capacity)
Set::new<T>(ref mut zone, capacity)
collections::from_slice_in<T>(ref mut zone, values)
set.len()
set.capacity()
set.is_empty()
set.index_of(value)
set.contains(value)
set.insert(ref mut zone, value)
set.remove(value)
set.take(value)
set.clear()
set.as_slice()
set.copy_to(ref mut zone)
```

`insert` returns `true` only for newly inserted values. `remove` drops the
removed value and reports whether it was present; `take` returns
`Option[T]`. The set preserves insertion order in `index_of`, `as_slice`, and
`copy_to`. It is linear, not hash-backed, so future `HashMap`/`HashSet` APIs
can still choose a deliberate hashing and equality policy.

`std::string::String` is an owned byte string:

```ari
std::string::new(ref mut zone, capacity)
std::string::from_string(ref mut zone, "text")
std::string::from_slice_in(ref mut zone, bytes)
text.len()
text.capacity()
text.is_empty()
text.first()
text.try_first()
text.last()
text.try_last()
text.get(index)
text.try_get(index)
text.set(index, byte)
text.replace(index, byte)
text.push(byte)
text.push_in(ref mut zone, byte)
text.pop()
text.try_pop()
text.insert(index, byte)
text.insert_in(ref mut zone, index, byte)
text.clear()
text.truncate(length)
text.reserve(ref mut zone, capacity)
text.reserve_extra(ref mut zone, additional)
text.extend_from_slice_in(ref mut zone, bytes)
text.resize_in(ref mut zone, length, byte)
text.index_of(byte)
text.contains(byte)
text.count(byte)
text.starts_with(bytes)
text.ends_with(bytes)
text.equals(bytes)
text.equals_ignore_case(bytes)
text.starts_with_ignore_case(bytes)
text.ends_with_ignore_case(bytes)
text.index_of_ignore_case(bytes)
text.contains_ignore_case(bytes)
text.append_string_in(ref mut zone, "text")
text.append_i64_in(ref mut zone, value)
text.append_u64_in(ref mut zone, value)
text.append_bool_in(ref mut zone, value)
text.append_f32_in(ref mut zone, value, precision)
text.append_f64_in(ref mut zone, value, precision)
text.trim_start()
text.trim_start_to(ref mut zone)
text.trim_end()
text.trim_end_to(ref mut zone)
text.trim()
text.trim_to(ref mut zone)
text.parse_decimal()
text.parse_decimal_prefix()
text.parse_hex()
text.parse_hex_prefix()
text.as_slice()
text.as_ptr()
text.copy_to(ref mut zone)
```

`String` stores bytes, so `equals_ignore_case`, `starts_with_ignore_case`,
`ends_with_ignore_case`, `index_of_ignore_case`, `contains_ignore_case`,
`trim_start`, `trim_end`, `trim`, `parse_decimal`, `parse_decimal_prefix`,
`parse_hex`, and `parse_hex_prefix` intentionally reuse `std::ascii` behavior.
The `try_*` byte accessors return `Option[u8]` for empty or out-of-range
access. The plain trim methods return borrowed `Slice[u8]` views, while
`trim_start_to`, `trim_end_to`, and `trim_to` copy the trimmed bytes into a
target zone and return owned `String` handles. The whole parse methods require
the whole string to be valid and return `Option[i64]`; prefix parsers return
`Option[std::ascii::ParsedInt]` and stop before the first invalid byte. Trim
first when leading or trailing ASCII whitespace should be ignored.

`std::boxed::Box[T]` is a zone-backed single-value owner:

```ari
std::boxed::new<T>(ref mut zone, value)
Box!(T, ref mut zone, value)
box.get()
box.set(value)
box.replace(value)
box.take()
box.try_take()
box.clear()
box.put_in(ref mut zone, value)
box.copy_to(ref mut zone)
box.as_ref()
box.as_mut()
box.as_ptr()
box.as_mut_ptr()
box.swap(ref mut other)
box.is_empty()
```

## Comparison

`std::cmp` contains source comparison traits and generic value helpers:

```ari
cmp::Eq[T]
cmp::PartialEq[T]
cmp::Ord[T]
cmp::PartialOrd[T]
cmp::min<T>(left, right)
cmp::max<T>(left, right)
cmp::clamp<T>(value, low, high)
cmp::is_between<T>(value, low, high)
```

`Ord[T]` currently requires `lt(self, other: T) -> bool`. `min`, `max`,
`clamp`, and `is_between` use that trait bound, so custom ordered values need
an `impl cmp::Ord[T] for T`. `clamp` and `is_between` assert that
`low <= high`; `is_between` is inclusive at both ends.

The root prelude re-exports the value helpers as `min<T>`, `max<T>`,
`clamp<T>`, and `is_between<T>`.

## Conversion

`std::convert` contains explicit conversion trait names and source helper
functions:

```ari
convert::From[T]
convert::Into[T]
convert::TryFrom[T]
convert::TryInto[T]
convert::identity<T>(value)
convert::from<T, U>(value)
convert::into<T, U>(value)
```

`identity` returns its input unchanged. `from<T, U>` builds destination `U`
through `convert::From[T]`, and `into<T, U>` turns source `U` into destination
`T` through `convert::Into[T]`. `TryFrom` and `TryInto` are reserved trait
names only for now; fallible conversion methods are future library work.

## Math

`std::math` currently contains conservative source-only helpers with `i64`
signatures. The names intentionally avoid type suffixes so they can grow into
generic numeric APIs later without changing call sites:

```ari
math::abs(value)
math::sign(value)
math::is_positive(value)
math::is_negative(value)
math::is_zero(value)
math::is_even(value)
math::is_odd(value)
math::pow(base, exponent)
math::div_floor(numerator, denominator)
math::div_ceil(numerator, denominator)
math::mod_floor(numerator, denominator)
math::gcd(left, right)
math::lcm(left, right)
```

`is_positive`, `is_negative`, and `is_zero` are predicate forms for the same
sign policy as `sign`. `pow` requires a non-negative exponent and asserts that
precondition at runtime. `div_floor` rounds signed division toward negative
infinity, `div_ceil` rounds toward positive infinity, and `mod_floor` returns
the matching floor remainder. The division helpers assert that
`denominator != 0`. `gcd` and `lcm` normalize negative inputs through absolute
values. `lcm` returns `0` when either input is `0`. These helpers
intentionally do not define overflow semantics yet.

## Bits

`std::bits` contains source-only `u64` helpers for bit masks, rotations,
power-of-two rounding, low-bit masks, alignment, and zero/one-run bit scans:

```ari
bits::is_set(value, mask)
bits::any_set(value, mask)
bits::set(value, mask)
bits::clear(value, mask)
bits::toggle(value, mask)
bits::rotate_left(value, count)
bits::rotate_right(value, count)
bits::is_power_of_two(value)
bits::bit_width(value)
bits::floor_power_of_two(value)
bits::ceil_power_of_two(value)
bits::low_mask(width)
bits::align_down(value, alignment)
bits::align_up(value, alignment)
bits::count_ones(value)
bits::count_zeros(value)
bits::leading_zeros(value)
bits::trailing_zeros(value)
bits::leading_ones(value)
bits::trailing_ones(value)
```

`is_set` requires all bits from `mask`; `any_set` requires at least one
overlap. `rotate_left` and `rotate_right` assert that `count` is non-negative
and then rotate modulo 64. `align_down` and `align_up` assert that `alignment`
is a non-zero power of two. These helpers currently have `u64` signatures and
intentionally avoid type suffixes so future generic integer APIs can keep the
same names. The zero value has 64 leading zeros, 64 trailing zeros, zero
leading ones, and zero trailing ones; `~0u64` has 64 leading ones and 64
trailing ones.
`bit_width` returns the number of bits needed to represent a value,
`floor_power_of_two` and `ceil_power_of_two` round to nearby powers of two, and
`low_mask(width)` returns a mask with the lowest `width` bits set. `low_mask`
accepts widths from 0 through 64.

## ASCII

`std::ascii` contains byte-oriented helpers for ASCII-only text and parser
code. Public names stay natural because the module path already says the
policy:

```ari
ascii::ParsedInt
ascii::is_digit(byte)
ascii::is_lower(byte)
ascii::is_upper(byte)
ascii::is_alpha(byte)
ascii::is_alphanumeric(byte)
ascii::is_blank(byte)
ascii::is_whitespace(byte)
ascii::is_control(byte)
ascii::is_printable(byte)
ascii::is_graphic(byte)
ascii::is_punctuation(byte)
ascii::is_hex_digit(byte)
ascii::to_lower(byte)
ascii::to_upper(byte)
ascii::digit_value(byte)
ascii::hex_value(byte)
ascii::equals_ignore_case(left, right)
ascii::starts_with_ignore_case(bytes, prefix)
ascii::ends_with_ignore_case(bytes, suffix)
ascii::index_of_ignore_case(bytes, needle)
ascii::contains_ignore_case(bytes, needle)
ascii::skip_whitespace(bytes)
ascii::trim_start(bytes)
ascii::trim_end(bytes)
ascii::trim(bytes)
ascii::parse_decimal(bytes)
ascii::parse_decimal_prefix(bytes)
ascii::parse_hex(bytes)
ascii::parse_hex_prefix(bytes)
```

`is_blank` covers space and tab. `is_whitespace` covers space, tab, line feed,
and carriage return. `is_printable` includes space; `is_graphic` excludes
space. `is_punctuation` is true for graphic ASCII bytes that are not letters or
digits.

`digit_value` and `hex_value` return `Option[i64]`. Non-digit input returns
`None<i64>()` where appropriate.

`equals_ignore_case`, `starts_with_ignore_case`, `ends_with_ignore_case`,
`index_of_ignore_case`, and `contains_ignore_case` operate on `Slice[u8]` and
fold only ASCII letter case. Empty prefixes, suffixes, and search needles
match. `index_of_ignore_case` returns the first matching byte offset or `-1`;
`contains_ignore_case` returns the same search as a bool. `skip_whitespace`,
`trim_start`, `trim_end`, and `trim` also operate on `Slice[u8]` and return
either a byte offset or a borrowed sub-slice. `parse_decimal` and `parse_hex`
parse the entire slice and return `Option[i64]`; empty input or invalid bytes
return `None<i64>()`. These parser helpers do not define overflow behavior
yet.

`ParsedInt` carries `value: i64` and `len: i64` for prefix parser results.
`parse_decimal_prefix` and `parse_hex_prefix` parse only the leading digit run,
stop before the first invalid byte, and return `None<ParsedInt>()` when the
first byte is empty or invalid. They do not trim, parse signs, or recognize
hexadecimal prefixes such as `0x`.

## Choosing The Right Collection

Use bare `Vec[T]` literals like `[1, 2, 3]` for small local compiler-known
sequence storage. Use `std::vec::Vec[T]` when you need a growable source
library collection tied to an explicit allocation zone.

Use `Slice[T]` when you only need a borrowed view. Use `String` when bytes must
be owned and copied into a zone. Use `Box[T]` for one zone-backed owned value.
