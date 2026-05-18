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
value.unwrap()
value.expect("message")
value.map<U>(op)
value.and_then<U>(op)
value.or(fallback)
value.or_else(op)
```

`Result[T, E]` models success or failure:

```ari
Ok<T, E>(value)
Err<T, E>(error)
value.is_ok()
value.is_err()
value.unwrap_or(fallback)
value.unwrap()
value.expect("message")
value.unwrap_err()
value.expect_err("message")
value.map<U>(op)
value.map_err<F>(op)
value.and_then<U>(op)
value.or_else<F>(op)
```

## Slice, Vec, String, And Box

`Slice[T]` is a borrowed contiguous view:

```ari
slice(pointer, len)
view.len()
view.is_empty()
view.first()
view.last()
view.get(index)
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

`std::vec::Vec[T]` is the source growable sequence:

```ari
std::vec::new<T>(ref mut zone, capacity)
Vec!(T, ref mut zone, capacity)
vec.len()
vec.capacity()
vec.is_empty()
vec.first()
vec.last()
vec.get(index)
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
text.push(byte)
text.push_in(ref mut zone, byte)
text.append_string_in(ref mut zone, "text")
text.append_i64_in(ref mut zone, value)
text.append_u64_in(ref mut zone, value)
text.append_bool_in(ref mut zone, value)
text.append_f32_in(ref mut zone, value, precision)
text.append_f64_in(ref mut zone, value, precision)
text.as_slice()
text.as_ptr()
text.copy_to(ref mut zone)
```

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

## Choosing The Right Collection

Use bare `Vec[T]` literals like `[1, 2, 3]` for small local compiler-known
sequence storage. Use `std::vec::Vec[T]` when you need a growable source
library collection tied to an explicit allocation zone.

Use `Slice[T]` when you only need a borrowed view. Use `String` when bytes must
be owned and copied into a zone. Use `Box[T]` for one zone-backed owned value.
