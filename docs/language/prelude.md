# Prelude And Formatting

## What The Prelude Is

Ari auto-loads the source `std` surface into ordinary executable modules and
adds implicit aliases for its public prelude names. You can call those names
without a `use`.

The source declaration header lives at `lib/std.arih`. The compiler auto-loads
it as the `std` module when the file is present, so explicit `std::...` names
work without a `mod std;` declaration:

```ari
use std::io::{write_i64, newline};

fn main() -> i64 {
  write_i64(42);
  newline();
  return std::mem::size_of<i64>()
}
```

The header exposes declaration-shaped IO, input, context, assertion, trait,
range, pointer/memory, and zone surfaces. Ari-owned runtime hooks are declared
with `extern "ari"` and explicit `ari_builtin_*` symbols, not `extern "C"`;
they are compiler/runtime builtins, not libc bindings. The semantic checker
uses those source declarations for ordinary function and trait signatures.
Formatting `print`/`println`, layout queries, range constructors, typed memory
helpers, and lexical temporary-zone helpers still lower through compiler hooks
after the matching source `std` surface is visible.

Like Rust, public standard names are imported by the implicit prelude when
`std` is auto-loaded. The source header decides this surface: public root items,
root `pub use` re-exports, and public `std` child modules from `lib/std.arih`
become visible in ordinary code. Write `Vec[T]`, `Option[T]`, `Maybe[T]`,
`Optional[T]`, `Range[T]`, `Iterator[T]`, `range(...)`, `size_of<T>()`,
`write_i64(...)`, `create(...)`, `new<T>(...)`, and pointer helpers directly.
Nested forms such as `fmt::Display`, `iter::Iterator[T]`, `mem::size_of<T>()`,
`input::read_byte()`, and `zone::new<T>(...)` resolve through implicit aliases
to the source `std` module, even when a root re-export such as `input()` shares
the prefix.
The explicit paths still exist as `std::Vec`, `std::iter::range`,
`std::mem::size_of`, and `std::zone::new`; `std::vec::Vec` is not the normal
spelling. Local declarations and explicit `use` aliases win over these
implicit prelude names. If you want a separate namespace handle, alias the
module explicitly:

```ari
use std as core

fn main() -> i64 {
  return core::mem::size_of<i64>()
}
```

Pass `--no-implicit-std` when testing the source header as ordinary module
code only. In that mode `use std::...` does not load anything by itself; import
the header through the normal module system instead. Compiler-known helper
lowering such as unqualified `size_of<T>()`, `range(...)`, `ptr_load<T>()`,
and `zone::new<T>(...)` is also disabled until the `std` module is present.
Source function signatures such as `write_i64(...)`, `io::write_i64(...)`,
`arg_count()`, and `zone::create(...)` follow the same rule; without implicit
`std` or an explicit `mod std;`, they are ordinary unknown calls. Prelude trait
names such as `Debug`, `Drop`, and `Iterable[T]` also come from source `std`,
so they are unavailable in that mode unless the source `std` module is loaded.
For helper surfaces that `lib/std.arih` can describe as generic function
declarations, the declaration must actually exist; a partial custom `std`
header does not silently inherit those compiler-known names:

```ari
mod std;
use std::io::write_i64;
```

Compile it with a module search path that can find `lib/std.arih`, for example
`ari app.ari --no-implicit-std --module-path lib`.

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
std::print("x={}", 1)
std::println("x={}", 1)
std::io::print("x={}", 1)
std::io::println("x={}", 1)
```

If `std` is imported under an alias, the same builtin formatting lowering is
available through that alias, such as `use std as core; core::println("x={}", 1)`.

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

## Current Source Signatures

These signatures are declared by `lib/std.arih` and exposed through implicit
prelude aliases when source `std` is loaded:

```ari
print(format: string, ...) -> i64
println(format: string, ...) -> i64
io::print(format: string, ...) -> i64
io::println(format: string, ...) -> i64
std::print(format: string, ...) -> i64
std::println(format: string, ...) -> i64
std::io::print(format: string, ...) -> i64
std::io::println(format: string, ...) -> i64
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
slice<T>(data: ptr T, len: i64) -> Slice[T]
std::slice<T>(data: ptr T, len: i64) -> std::Slice[T]
```

On the default LLVM/glibc host backend, IO builtins lower to C stdio calls. On
`--freestanding`, byte-oriented IO builtins lower to Linux `read`/`write`
syscalls.

## Prelude Traits

Prelude traits are ordinary public trait declarations in `lib/std.arih`. The
implicit prelude exposes the root forms and the child-module forms:

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
bounds. Concrete impl method calls, trait-bound generic calls, and the current
copyable LLVM trait-object subset lower through the same source trait table.

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

The source header `lib/std.arih` exposes the declaration-shaped zone API:
`std::zone::create`, raw and typed `alloc`, `new`, `promote`, `reset`, and
`destroy`. `zone::temp` and `zone::scratch` remain compiler-known lexical
helpers until source declarations can express their hidden lifetime cleanup.

## Aggregate Surfaces

The prelude also exposes language-known aggregate surfaces:

```ari
let maybe: Option[i64] = Some(7)
let missing: Option[i64] = None<i64>()
let result: Result[i64, i32] = Ok<i64, i32>(9)
let pair: (i64, bool) = (7, true)
let values: Vec[i64] = [1, 2, 3]
let empty: Vec[i64] = []
let fixed = [1, 2, 3]
let span: Range[i64] = 1..4
let byte_span: Range[u8] = range(1, 4)
var first = 1
let view: Slice[i64] = slice((ref mut first) as ptr i64, 1)
var data = [1, 2, 3]
let data_view: Slice[i64] = data.as_slice()
```

These names are available without `std::` when implicit `std` loading is on.
Use explicit paths only when you want to show the source module, for example
`std::Vec[i64]` or `std::iter::range(1, 4)`.

Tuples, fixed arrays, local vector values, and prelude ranges lower as local
stack aggregate values. Non-empty `[...]` defaults to a fixed array when no
expected type is present, and the same syntax initializes `Vec[T]` when a
`Vec[T]` annotation or assignment target supplies that expected type. `Vec[T]`
local storage keeps a runtime length plus a local stack capacity chosen from
the largest vector literal assigned to the binding. A typed `Vec[T]` context
supplies the element type for empty `[]`. `len(values)` and `values.len()` read
vector runtime length, fixed array length, or `Slice[T].len`. `view[index]`
loads or stores a `Slice[T]` element through its raw `data` pointer after
checking the stored length. The temporary compiler-known local `Vec[T]` method
set is intentionally frozen; methods outside the documented local subset are
reserved for allocator-backed std collection APIs. Growable heap vectors remain
an explicit-allocator feature for later.

`Slice[T]` is a source `std` view struct:

```ari
struct Slice[T] {
  data: ptr T,
  len: i64,
}
```

It is non-owning and carries no hidden borrow lifetime yet. The helper
`slice(data, len)` constructs the same view and is available as `std::slice` or
through a `std` alias. Passing its `data` is the same explicit raw-pointer
promise as other `ptr T` uses. `view[index]` traps on negative or out-of-range
indexes; assigning to `view[index]` writes through the stored raw pointer and
does not mutate the `Slice[T]` metadata. Mutable local fixed arrays and mutable
local `Vec[T]` storage can build the same non-owning writable view with
`values.as_slice()`. The helper captures the array size or current vector
runtime length, so later vector length changes do not update an existing view.
`view[start..end]` and `view[start..=end]` produce another `Slice[T]` with
bounds checks against the source view; exclusive ranges allow empty slices when
`start == end`, while inclusive ranges require the end index to name an
existing element. Slice patterns are still planned after the binding policy is
nailed down.
`len(view)`, `view.len()`, and `view.is_empty()` read the stored length.

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

`Option[T]` and `Result[T, E]` are ordinary source `std` generic enums:

```ari
enum Option[T] {
  None,
  Some(T),
}

enum Result[T, E] {
  Err(E),
  Ok(T),
}
```

They are available without `std::` through the implicit prelude. `Maybe[T]`
and `Optional[T]` are public aliases for `Option[T]`, so `Maybe[i64]`,
`Optional[i64]`, `std::Optional[i64]`, and a module alias such as
`core::Optional[i64]` all name the same enum. Use explicit paths such as
`std::Option[i64]`, `std::Maybe[i64]`, `std::Optional[i64]`, `std::Some(1)`, or
`std::Ok<i64, i32>(1)` when you want to spell the source module. Postfix `?`
and `??` recognize the same Maybe/Result-style enum shapes on the LLVM backend
path; the freestanding backend still needs the broader aggregate enum
return/value ABI work.

Additional Rust-like standard surfaces are reserved with clear diagnostics:

```ari
Box[T]
```

`Hash` and hash-map containers are intentionally not part of the prelude. They
belong in an explicit collection module later.
