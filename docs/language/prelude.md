# Prelude And Formatting

## What The Prelude Is

Ari auto-loads the source `std` surface into ordinary executable modules and
adds implicit aliases for its public prelude names. You can call those names
without a `use`.

The source declaration root lives at `lib/std.arih`, with child modules under
`lib/std/`. The compiler auto-loads that package as the `std` module when the
root is present, so explicit `std::...` names work without a `mod std;`
declaration:

```ari
use std::io::{write_i64, newline};

fn main() -> i64 {
  write_i64(42);
  newline();
  return std::mem::size_of<i64>()
}
```

The package exposes declaration-shaped IO, input, context, assertion, trait,
range, pointer/memory, and zone surfaces. Ari-owned runtime hooks are declared
with `extern "ari"` and explicit `ari_builtin_*` symbols, not `extern "C"`;
they are compiler/runtime builtins, not libc bindings. The semantic checker
uses those source declarations for ordinary function and trait signatures.
Formatting `print`/`println`, layout queries, range constructors, typed memory
helpers, and lexical temporary-zone helpers still lower through compiler hooks
after the matching source `std` surface is visible.

Like Rust, public standard names are imported by the implicit prelude when
`std` is auto-loaded. The source header and its file-backed child modules decide
this surface: public root items, root `pub use` re-exports, and public `std`
child modules become visible in ordinary code. Write `Vec[T]`, `Option[T]`,
`Range[T]`, `Iterator[T]`, `range(...)`, `size_of<T>()`,
`write_i64(...)`, `create(...)`, `new<T>(...)`, and pointer helpers directly.
Nested forms such as `fmt::Display`, `iter::Iterator[T]`, `mem::size_of<T>()`,
`input::read_byte()`, and `zone::new<T>(...)` resolve through implicit aliases
to the source `std` module, even when a root re-export such as `input()` shares
the prefix.
The explicit paths still exist as `std::Vec`, `std::iter::range`,
`std::mem::size_of`, and `std::zone::new`. `std::vec::Vec` names the
source-backed allocator seed handle while the root `Vec`/`std::Vec` spelling is
still the current compiler-known local vector type. `std::boxed::Box` names the
source zone-backed box seed; the root `Box[T]`, `Unique[T]`, `Shared[T]`, and
`Weak[T]` smart-pointer spellings remain reserved. Local declarations and
explicit `use` aliases win over these implicit prelude names. If you want a
separate namespace handle, alias the module explicitly:

```ari
use std as core

fn main() -> i64 {
  return core::mem::size_of<i64>()
}
```

The `std::vec` module currently exposes allocator-seeding helpers for the
future growable vector runtime. Its source lives in `lib/std/vec.arih`.
`std::vec::alloc_buffer<T>(ref mut zone, capacity)` allocates
`capacity * size_of<T>()` bytes from an explicit `Zone`
capability and returns a tracked `ptr T`; `std::vec::with_capacity<T>(ref mut
zone, capacity)` wraps that pointer in a tracked `RawVec<T>` handle with
`data`, mutable `len`, and `capacity` fields. `std::vec::new<T>(ref mut zone,
capacity)` wraps that raw handle in the public source `std::vec::Vec<T>` seed.
The source handle currently exposes element methods: `len`, `capacity`,
`is_empty`, `first`, `last`, `get`, `set`, `replace`, `swap`, `push`,
`push_in(ref mut zone, value)`, grow-only same-zone `reserve`,
`reserve_extra(ref mut zone, additional)`, `pop`, `insert`,
`insert_in(ref mut zone, index, value)`, `remove`, `clear`, `truncate`,
`contains`, `index_of`, `count`, `extend_from_slice_in(ref mut zone, values)`,
`resize_in(ref mut zone, length, value)`, `copy_to(ref mut zone)`, and
`as_ptr()` and `as_slice`. `reserve`, `reserve_extra`, `push_in`, `insert_in`,
`extend_from_slice_in`, and `resize_in` use the same explicit zone capability
to grow the buffer. `copy_to(ref mut zone)` copies the current elements into a
new handle tied to the target zone. `as_ptr()` returns the stored element
pointer with the source zone provenance preserved. This is not the final root
`Vec[T]` method API.

The `std::boxed` module exposes `std::boxed::new<T>(ref mut zone, value)` for a
tracked source `std::boxed::Box<T>` handle over one value placed in a zone. Its
source lives in `lib/std/boxed.arih`. The handle has `get()`, `set(value)`,
`replace(value)`, `copy_to(ref mut zone)`, `swap(ref mut other)`, and
`as_ptr()` methods for copyable, zone-placeable values. `replace(value)` stores
a new value and returns the previous one.
`copy_to(ref mut zone)` copies the current value into another explicit zone and
returns a new tracked `std::boxed::Box<T>` handle for that target zone.
`swap(ref mut other)` exchanges the values stored by two boxes without changing
which zone each handle belongs to. `as_ptr()` returns the stored `ptr T` with
the same zone provenance as the handle, so using that pointer after the source
zone is reset or destroyed is also rejected by the checker. This is not yet the
final owning root `Box[T]` smart pointer surface.

Pass `--no-implicit-std` when testing the source header as ordinary module
code only. In that mode `use std::...` does not load anything by itself; import
the header through the normal module system instead. Compiler-known helper
lowering such as unqualified `size_of<T>()`, `range(...)`, `ptr_load<T>()`,
and `zone::new<T>(...)` is also disabled until the `std` module is present.
Raw pointer helpers can infer `T` from their pointer argument or take an
explicit `<T>` argument.
Source function signatures such as `write_i64(...)`, `io::write_i64(...)`,
`arg_count()`, and `zone::create(...)` follow the same rule; without implicit
`std` or an explicit `mod std;`, they are ordinary unknown calls. Prelude trait
names such as `Debug`, `Drop`, and `Iterable[T]` also come from source `std`,
so they are unavailable in that mode unless the source `std` module is loaded.
For helper surfaces that the source `std` package can describe as generic
function declarations, the declaration must actually exist; a partial custom
`std` header does not silently inherit those compiler-known names:

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
  println("pi={:.2}", 3.14159f64)
  print("escaped braces: {{}}")
  newline()
  return 0
}
```

Formatting rules:

- the first argument must be a string literal
- `{}` consumes one value argument
- `{:.N}` consumes one `f32` or `f64` value and prints it with `N` digits
  after the decimal point; `N` must be between `0` and `64`
- `{{` writes a literal `{`
- `}}` writes a literal `}`
- the placeholder count is checked at compile time
- formatted values currently support integers, bool, `f32`, and `f64`
- `println` appends one newline
- `print` does not append a newline

`bool` currently prints as `1` or `0`.
On the raw `--freestanding` backend, formatted float output is reserved until
the freestanding runtime grows decimal float formatting; integer and bool
formatting still work there.

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
allocator-backed `String` values. The root `String` type name is reserved for
that future owned buffer surface; use lowercase `string` for today's borrowed
pointer-shaped text values. The `--freestanding` backend rejects line input
until that backend has runtime string storage.

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
`matches!` uses the unqualified parser-special spelling and expands to a
bool-valued pattern test using the same pattern engine as `match`, so enum,
scalar, tuple, array, struct, tuple-struct, alias, and or-pattern forms follow
the same rules. `format!` is reserved until Ari has owned runtime strings.
Other prelude expression macros are recognized as unqualified names or paths
that resolve to the root `std` macro name, such as `std::print!` or an alias of
`std::format!`; arbitrary module paths whose basename is `format`, `print`, or
another prelude macro name remain user macro paths.

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

`Iterator[T]` has one required step method:

```ari
fn next(self: ref mut Self) -> Option[T]
```

Direct `for` lowering works for copyable non-borrow iterator values by storing
the iterator once, mutably borrowing that hidden iterator binding, and calling
`next` until it returns `None`. Value-self `next(self)` impls remain accepted as
a compatibility subset for copyable snapshot-style iterators.

`IntoIterator[T]` has one current conversion method:

```ari
fn into_iter(self: ref mut Self) -> Self
```

`Self` is the default return shape in the source header. Trait impls may return
a distinct iterator type while the associated-iterator contract is still
compiler-known; impl validation requires the actual `into_iter` result to
implement `Iterator[T]`, including generic result types such as `BagIter[T]`,
and `for` lowering rechecks the specialized result. Value-self
`into_iter(self)` impls remain accepted as a compatibility subset for copyable
snapshot-style containers.

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
`std::vec::alloc_buffer<T>(ref mut Zone, capacity)` is the raw
vector-allocation seed. `std::vec::with_capacity<T>(ref mut Zone, capacity)`
builds a source `RawVec<T>` handle around that allocation, and
`std::vec::new<T>(ref mut Zone, capacity)` wraps it in source
`std::vec::Vec<T>`. The source handle has methods for metadata, checked
read/write/replace, push/pop, grow-on-demand
`push_in(ref mut Zone, value)`, grow-only explicit
`reserve(ref mut Zone, capacity)`, `reserve_extra(ref mut Zone, additional)`,
grow-on-demand `insert_in(ref mut Zone, index, value)`,
`resize_in(ref mut Zone, length, value)`, insert/remove, truncate/clear, swap,
simple linear search, and
`extend_from_slice_in(ref mut Zone, Slice<T>)`, and `vec.as_slice()` creates a
mutable `Slice[T]` view over the same zone-backed buffer. `copy_to(ref mut
Zone)` copies the current elements into a new handle tied to the target zone.
`as_ptr()` returns the stored element pointer with the receiver's zone
provenance, so it is rejected after that zone is reset or destroyed.
`reserve`, `reserve_extra`, `push_in`, `insert_in`, `extend_from_slice_in`,
and `resize_in` must receive the same zone that created the handle; they copy
existing elements into a larger zone allocation when growth is needed and keep
old storage under the zone's bulk lifetime. Callers can still use
`vec.raw.data` with `ptr_store`, `ptr_load`, and `ptr_add` directly for
lower-level experiments.

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
`Vec[T]` annotation or assignment target supplies that expected type. That
element type is propagated into each literal element before calls and coercions
are checked. `Vec[T]` local storage keeps a runtime length plus a local stack
capacity chosen from the largest vector literal assigned to the binding.
Literal `reserve(n)`,
integer constants, static integer arithmetic/bitwise/shift expressions over
constants and literals, and immutable local integer capacities initialized from
those static expressions can raise that local capacity, while runtime
`reserve(n)` checks that the requested capacity already fits the fixed local
storage. The same static integer forms keep `truncate(n)` length tracking
precise for later local `push` and `insert` storage decisions. A typed `Vec[T]`
context supplies the
element type for empty `[]`. `len(values)` and `values.len()` read
vector runtime length, fixed array length, or `Slice[T].len`. `view[index]`
loads or stores a `Slice[T]` element through its raw `data` pointer after
checking the stored length. The temporary compiler-known local `Vec[T]` method
set is intentionally frozen; methods outside the documented local subset are
reserved for allocator-backed std collection APIs. Growable heap vectors remain
an explicit-allocator feature for later. The lower-level
`std::vec::alloc_buffer<T>(ref mut Zone, capacity)` and
`std::vec::with_capacity<T>(ref mut Zone, capacity)` helpers already exercise
the explicit allocator path for future Vec storage, and
`std::vec::new<T>(ref mut Zone, capacity)` exposes that seed as source
`std::vec::Vec<T>`. The source handle supports metadata, read/write/replace,
push/pop, same-zone `push_in` growth, same-zone grow-only `reserve`,
insert/remove, swap,
same-zone `reserve_extra`, same-zone `insert_in` growth, same-zone
`extend_from_slice_in` growth, same-zone `resize_in` growth, truncate/clear,
simple search, target-zone `copy_to`, and `as_slice` calls over the stored raw
handle. The resulting `Slice[T]` keeps the same zone provenance, so using it
after `zone::reset` or `zone::destroy` is rejected. `as_ptr()` raw pointers
and copied Vec handles track their source or target zone respectively. The root
`Vec[T]` type and its current local method set remain fixed-local until runtime
growth is ported. Root `Vec[T]` therefore cannot cross non-local ABI or storage
boundaries yet: function and extern parameters/returns, struct fields, and impl
receivers reject it with a dedicated diagnostic. Use `std::vec::Vec<T>` for an
explicit-zone heap handle or `Slice[T]` for a borrowed view.

`std::boxed::Box<T>` is the source `std` allocation seed for a single
zone-backed value:

```ari
var zone = zone::create(64)
var other_zone = zone::create(64)
var boxed = std::boxed::new<i64>(ref mut zone, 21)
let before = boxed.get()
boxed.set(9)
let after = boxed.get()
let replaced = boxed.replace(12)
var copied = boxed.copy_to(ref mut other_zone)
boxed.swap(ref mut copied)
let raw = boxed.as_ptr()
```

The handle stores a raw pointer returned by `zone::new<T>` and keeps the same
zone provenance, so reset/destroy invalidation applies to the handle and to raw
pointers recovered through `as_ptr()`. It does not run destructors or free the
value independently; memory is released with the zone.

Root smart-pointer names are reserved now so the lint and library surfaces do
not drift. `Box[T]` is the future unique owning smart pointer spelling.
`Unique[T]` is reserved as policy/design space, but the root unique owner should
be spelled `Box[T]` once implemented. `Shared[T]` is reserved for future
reference-counted shared ownership, and `Weak[T]` is reserved for non-owning
handles that can be upgraded back to `Option[Shared[T]]`. These future handles
must be constructed through explicit allocator or capability arguments; Ari
does not add a magical global heap for them.

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
existing element. The raw freestanding backend uses the same pointer/length
metadata for local Slice indexing, indexed assignment, and range slicing. Slice
patterns are still planned after the binding policy is nailed down.
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

They are available without `std::` through the implicit prelude. Ari keeps the
standard absence type to one spelling, `Option[T]`; there is no `Maybe[T]`
alias. Use explicit paths such as `std::Option[i64]`, `std::Some(1)`, or
`std::Ok<i64, i32>(1)` when you want to spell the source module. Postfix `?`
and `??` recognize the same Option/Result-style enum shapes on the LLVM
backend path; the freestanding backend still needs the broader aggregate enum
return/value ABI work.

Additional Rust-like root standard surfaces are reserved with clear diagnostics:

```ari
String
Box[T]
Unique[T]
Shared[T]
Weak[T]
```

`Hash` and hash-map containers are intentionally not part of the prelude. They
belong in an explicit collection module later.
