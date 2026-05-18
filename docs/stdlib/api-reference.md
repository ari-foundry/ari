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
newline()
read_byte()
read_line()
read_line_owned(ref mut zone)
input()
input_owned(ref mut zone)
arg_count()
arg(index)
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
create(bytes)
alloc(ref mut zone, bytes, align)
new<T>(ref mut zone, value)
promote<T>(ref mut target, value)
reset(ref mut zone)
destroy(zone)
range(start, end)
range_inclusive(start, end)
```

## Option And Result

`Option[T]` models a present or missing value:

```ari
Some(value)
None<T>()
value.is_some()
value.is_none()
value.unwrap_or(fallback)
value.unwrap_or_else(op)
value.unwrap()
value.expect("message")
value.map<U>(op)
value.and_then<U>(op)
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
value.or_else<F>(op)
```

Use the lazy `*_else` forms when the fallback is expensive or should only run
on the missing/error branch. Use `ok_or` and `ok_or_else` when an optional
value needs to enter a `Result`-returning flow. Use `ok` and `err` when a
`Result` should be projected back into an `Option`.

## Slice, Vec, String, And Box

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
they return `Option[T]`.

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

`std::string::String` is an owned byte string:

```ari
std::string::new(ref mut zone, capacity)
std::string::from_string(ref mut zone, "text")
std::string::from_slice_in(ref mut zone, bytes)
text.len()
text.capacity()
text.is_empty()
text.first()
text.last()
text.get(index)
text.set(index, byte)
text.replace(index, byte)
text.push(byte)
text.push_in(ref mut zone, byte)
text.pop()
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
text.append_string_in(ref mut zone, "text")
text.append_i64_in(ref mut zone, value)
text.append_u64_in(ref mut zone, value)
text.append_bool_in(ref mut zone, value)
text.append_f32_in(ref mut zone, value, precision)
text.append_f64_in(ref mut zone, value, precision)
text.trim_start()
text.trim_end()
text.trim()
text.parse_decimal()
text.parse_hex()
text.as_slice()
text.as_ptr()
text.copy_to(ref mut zone)
```

`String` stores bytes, so `trim_start`, `trim_end`, `trim`,
`parse_decimal`, and `parse_hex` intentionally reuse `std::ascii` behavior.
The trim methods return borrowed `Slice[u8]` views. The parse methods require
the whole string to be valid and return `Option[i64]`; trim first when leading
or trailing ASCII whitespace should be ignored.

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

## Math

`std::math` currently contains conservative source-only helpers with `i64`
signatures. The names intentionally avoid type suffixes so they can grow into
generic numeric APIs later without changing call sites:

```ari
math::abs(value)
math::sign(value)
math::is_even(value)
math::is_odd(value)
math::pow(base, exponent)
math::gcd(left, right)
```

`pow` requires a non-negative exponent and asserts that precondition at
runtime. These helpers intentionally do not define overflow semantics yet.

## Bits

`std::bits` contains source-only `u64` helpers for bit masks, power-of-two
alignment, and bit scans:

```ari
bits::is_set(value, mask)
bits::any_set(value, mask)
bits::set(value, mask)
bits::clear(value, mask)
bits::toggle(value, mask)
bits::is_power_of_two(value)
bits::align_down(value, alignment)
bits::align_up(value, alignment)
bits::count_ones(value)
bits::count_zeros(value)
bits::leading_zeros(value)
bits::trailing_zeros(value)
```

`is_set` requires all bits from `mask`; `any_set` requires at least one
overlap. `align_down` and `align_up` assert that `alignment` is a non-zero
power of two. These helpers currently have `u64` signatures and intentionally
avoid type suffixes so future generic integer APIs can keep the same names.
The zero value has 64 leading zeros and 64 trailing zeros.

## ASCII

`std::ascii` contains byte-oriented helpers for ASCII-only text and parser
code. All classification and case helpers take `u8` values and use natural
names because the module path already says the policy:

```ari
ascii::is_digit(byte)
ascii::is_lower(byte)
ascii::is_upper(byte)
ascii::is_alpha(byte)
ascii::is_alphanumeric(byte)
ascii::is_whitespace(byte)
ascii::is_hex_digit(byte)
ascii::to_lower(byte)
ascii::to_upper(byte)
ascii::digit_value(byte)
ascii::hex_value(byte)
ascii::skip_whitespace(bytes)
ascii::trim_start(bytes)
ascii::trim_end(bytes)
ascii::trim(bytes)
ascii::parse_decimal(bytes)
ascii::parse_hex(bytes)
```

`digit_value` and `hex_value` return `Option[i64]`. Non-digit input returns
`None<i64>()` where appropriate.

`skip_whitespace`, `trim_start`, `trim_end`, and `trim` operate on
`Slice[u8]` and return either a byte offset or a borrowed sub-slice.
`parse_decimal` and `parse_hex` parse the entire slice and return
`Option[i64]`; empty input or invalid bytes return `None<i64>()`. These parser
helpers do not define overflow behavior yet.

## Choosing The Right Collection

Use bare `Vec[T]` literals like `[1, 2, 3]` for small local compiler-known
sequence storage. Use `std::vec::Vec[T]` when you need a growable source
library collection tied to an explicit allocation zone.

Use `Slice[T]` when you only need a borrowed view. Use `String` when bytes must
be owned and copied into a zone. Use `Box[T]` for one zone-backed owned value.
