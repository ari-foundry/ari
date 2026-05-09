# Prelude And Formatting

## What The Prelude Is

Ari injects a small compiler-known prelude into every executable module. You can
call these names without a `use`.

The current prelude is implemented inside the compiler, not as Ari source files.

## Formatting

```ari
fn main() -> i64 {
  println("value={} ok={}", 42, true)
  print("escaped braces: {{}}")
  newline()
  return 0
}
```

Formatting rules:

- the first argument must be a string literal
- `{}` consumes one value argument
- `{{` writes a literal `{`
- `}}` writes a literal `}`
- the placeholder count is checked at compile time
- formatted values currently support integers and bool
- `println` appends one newline
- `print` does not append a newline

`bool` currently prints as `1` or `0`.

## Qualified Formatting Names

These forms are accepted:

```ari
print("x={}", 1)
println("x={}", 1)
io::print("x={}", 1)
io::println("x={}", 1)
```

## Lower-Level IO

```ari
io::write_i64(42)
io::write_bool(true)
io::write_byte(65 as u8)
io::newline()
let byte = io::read_byte()
```

Unqualified aliases are also available:

```ari
write_i64(42)
write_bool(true)
write_byte(65 as u8)
newline()
let byte = read_byte()
```

`input::read_byte()` is also available. `read_byte` returns the next byte as an
`i64`, or `-1` at end of input.

Line-oriented input is available on the LLVM/glibc host backend:

```ari
let first = read_line()
let second = io::read_line()
let third = input()
let fourth = input::line()
```

These helpers return a `string` with one trailing `\n` or `\r` removed. End of
input returns an empty string. The current implementation uses one internal
line buffer, so a later line read can overwrite the bytes seen through an
earlier returned `string`. Copying into owned runtime strings is planned with
allocator-backed strings. The `--freestanding` backend rejects line input until
that backend has runtime string storage.

## Assertions And Stops

The current executable prelude has function forms for small assertion and stop
helpers:

```ari
assert(condition: bool) -> i64
debug_assert(condition: bool) -> i64
assert_eq_i64(left: i64, right: i64) -> i64
assert_ne_i64(left: i64, right: i64) -> i64
assert_eq_bool(left: bool, right: bool) -> i64
assert_ne_bool(left: bool, right: bool) -> i64
panic() -> void
todo() -> void
unreachable() -> void
```

Failed assertions and stop helpers terminate the process with status `1`.

Rust-style macro forms are available for the executable assertion, stop, and
printing helpers:

```ari
assert!(ready)
debug_assert!(count > 0)
assert_eq!(count, 3)
assert_ne!(enabled, false)
print!("count={}", count)
println!(" ok={}", enabled)
panic!()
todo!()
unreachable!()
matches!(value, Some(_))
```

`assert_eq!` and `assert_ne!` dispatch to integer or bool assertion builtins.
`matches!` expands to a bool-valued pattern test using the same pattern engine
as `match`, so enum, scalar, tuple, array, struct, tuple-struct, alias, and
or-pattern forms follow the same rules. `format!` is reserved until Ari has
owned runtime strings.

## Current Prelude Signatures

```ari
print(format: string, ...) -> i64
println(format: string, ...) -> i64
io::print(format: string, ...) -> i64
io::println(format: string, ...) -> i64
io::write_i64(value: i64) -> i64
io::write_bool(value: bool) -> i64
io::write_byte(value: u8) -> i64
io::newline() -> i64
io::read_byte() -> i64
io::read_line() -> string
write_i64(value: i64) -> i64
write_bool(value: bool) -> i64
write_byte(value: u8) -> i64
newline() -> i64
read_byte() -> i64
read_line() -> string
input() -> string
input::read_byte() -> i64
input::line() -> string
assert(condition: bool) -> i64
debug_assert(condition: bool) -> i64
assert_eq_i64(left: i64, right: i64) -> i64
assert_ne_i64(left: i64, right: i64) -> i64
assert_eq_bool(left: bool, right: bool) -> i64
assert_ne_bool(left: bool, right: bool) -> i64
panic() -> void
todo() -> void
unreachable() -> void
context::argc() -> i64
context::arg(index: i64) -> string
arg_count() -> i64
arg(index: i64) -> string
```

On the default LLVM/glibc host backend, IO builtins lower to C stdio calls. On
`--freestanding`, byte-oriented IO builtins lower to Linux `read`/`write`
syscalls.

## Prelude Traits

The prelude reserves iterator trait names so libraries can use one shared ABI
surface before the full source standard library exists:

```ari
Debug
fmt::Debug
Display
fmt::Display
Default
Clone
Copy
Drop
Eq[T]
PartialEq[T]
Ord[T]
PartialOrd[T]
cmp::Eq[T]
cmp::PartialEq[T]
cmp::Ord[T]
cmp::PartialOrd[T]
From[T]
Into[T]
TryFrom[T]
TryInto[T]
convert::From[T]
convert::Into[T]
convert::TryFrom[T]
convert::TryInto[T]
Iterable[T]
Iterator[T]
IntoIterator[T]
iter::Iterable[T]
iter::Iterator[T]
iter::IntoIterator[T]
ToString
ToOwned
```

`Drop` has one required method:

```ari
fn drop(self) -> void
```

An explicit `drop value;` statement calls this method when an impl exists for
the value type. Types without a `Drop` impl still use `drop` as an ownership
state operation.

The compiler validates trait existence, type-argument count, visibility,
duplicate impls, declared method conformance, and generic function trait
bounds. Concrete impl method calls and trait-bound generic calls lower through
static dispatch; vtable-backed trait objects are still planned.

## Context

The LLVM host backend initializes a small runtime context inside `@ari_entry`
before the `@"ari::main"` bridge calls source `main`.
It stores `argc`, `argv`, and a thread-id slot for later thread/runtime work.

Available context builtins:

```ari
context::argc() -> i64
context::arg(index: i64) -> string
arg_count() -> i64
arg(index: i64) -> string
```

Out-of-range `arg(index)` currently returns an empty string.

## Layout Queries

Compiler-known layout helpers are available for explicit memory code:

```ari
size_of<T>() -> i64
align_of<T>() -> i64
mem::size_of<T>() -> i64
mem::align_of<T>() -> i64
```

They support scalar types, pointer-shaped types, and Ari-layout aggregates such
as structs, tuples, tuple structs, and fixed arrays. Aggregate results describe
the current executable Ari layout, not a C ABI promise.

## Zone Allocation

The prelude exposes the host-backed allocation zone capability:

```ari
Zone
zone::create(capacity: i64) -> own Zone
zone::temp(capacity: i64) -> own Zone
zone::alloc(zone: ref mut Zone, bytes: i64, align: i64) -> ptr u8
zone::alloc<T>(zone: ref mut Zone) -> ptr T
zone::new<T>(zone: ref mut Zone, value: T) -> ptr T
zone::scratch<T>(capacity: i64, value: T) -> ptr T
zone::promote<T>(target: ref mut Zone, source: ptr T) -> ptr T
zone::reset(zone: ref mut Zone) -> void
zone::destroy(zone: own Zone) -> void
```

Zones are explicit allocation regions. Allocation returns raw `ptr u8` memory,
so callers cast or offset pointers deliberately and use `ptr_load`,
`ptr_store`, or `*pointer` for unchecked memory access. The generic
`zone::alloc<T>` form computes `T`'s layout and returns `ptr T` directly.
`zone::new<T>` additionally stores a provided value into that memory. It is
placement construction only; it does not register destructors.
`zone::scratch<T>(capacity, value)` can initialize a local pointer binding by
creating a hidden lexical temporary zone and placing `value` into it. The
resulting pointer cannot escape that local scratch lifetime.
`zone::promote<T>(target, source)` copies a pointed-to scratch value into an
explicit target zone and returns a pointer tied to the target zone.
`zone::temp(capacity)` creates a lexical scratch zone that is destroyed
automatically when its declaring scope falls through, before returns, and before
`break`, `continue`, or labeled-block exits that leave that scope. Other
`own Zone` values must be released with `zone::destroy`; `drop zone` is
rejected to avoid hiding the bulk free. Zone allocation is host-backed today;
the freestanding backend rejects it until a raw-backend allocation runtime
exists.
Pointers allocated from a temporary zone are lexical too: returning them,
storing them into longer-lived bindings or aggregates, or sending them through
escape-prone calls is rejected with a diagnostic that names the pointer and the
temporary zone.

## Reserved Aggregate Surfaces

The prelude also reserves language-known aggregate surfaces:

```ari
let pair: (i64, bool) = (7, true)
let values: Vec[i64] = [1, 2, 3]
let empty: Vec[i64] = []
let fixed = [1, 2, 3]
let span: Range[i64] = 1..4
let byte_span: Range[u8] = range(1, 4)
```

Tuples, fixed arrays, local vector values, and prelude ranges lower as local
stack aggregate values. Non-empty `[...]` defaults to a fixed array when no
expected type is present, and the same syntax initializes `Vec[T]` when a
`Vec[T]` annotation or assignment target supplies that expected type. `Vec[T]`
local storage keeps a runtime length plus a local stack capacity chosen from
the largest vector literal assigned to the binding. A typed `Vec[T]` context
supplies the element type for empty `[]`. `len(values)` and `values.len()` read
vector runtime length or fixed array length. Growable heap vectors remain an
explicit-allocator feature for later.

Ranges are compiler-known two-field values:

```ari
let half_open: Range[i64] = range(1, 4)
let inclusive: RangeInclusive[i64] = 1..=4
let bytes: Range[u8] = range(1, 4)

let start = half_open.start
let end = half_open.1

for value in half_open {
  println(value)
}
```

The executable subset supports integer range bounds. Half-open ranges iterate
`start` through `end - 1`; inclusive ranges include `end`.

Additional Rust-like standard surfaces are reserved with clear diagnostics:

```ari
Option[T]
Maybe[T]
Result[T, E]
Box[T]
Slice[T]
```

`Hash` and hash-map containers are intentionally not part of the prelude. They
belong in an explicit collection module later.
