# Types

## Integer Types

Ari supports signed and unsigned integer widths:

```ari
i8
i16
i32
i64
u8
u16
u32
u64
```

Integer literals are range-checked against their expected type:

```ari
let byte: u8 = 255
let delta: i32 = -10
```

`256` is rejected for `u8`, and `-129` is rejected for `i8`.

Both executable backends preserve the declared integer width when scalar locals
are read or written. On the raw `--freestanding` backend this includes narrow
local reloads after explicit raw-pointer writes, so an `i8`, `i16`, or `i32`
slot is sign-extended when read back through the local binding. Standalone
scalar locals are placed with byte-sized offsets and their natural alignment on
the raw backend. Local tuple, struct, tuple-struct, and fixed-array storage also
uses the shared Ari byte layout for field and element addressing. Direct raw
calls can pass and return Ari-layout aggregate values through hidden pointer
slots; foreign ABI classification is still planned.

## Bool

```ari
let ready: bool = true
let failed = false
```

Conditions for `if`, `while`, `&&`, `||`, and `!` must be `bool`.

## Casts

Integer and float-width casts use explicit `as`:

```ari
let byte: u8 = 255
let signed: i64 = byte as i64
let wrapped: u8 = 300 as u8
let wide: f64 = 1.5f32 as f64
let narrow: f32 = 2.25f64 as f32
let measured: f64 = 42i64 as f64
let count: i64 = 5.75f64 as i64
```

Narrowing keeps the low bits. Signed targets sign-extend after truncation, and
unsigned targets zero-extend. Float-width casts between `f32`, `f64`, and
`f128` are explicit as well. Integer-to-float casts preserve the numeric value
when the target float can represent it; float-to-integer casts truncate toward
zero. Ari does not implicitly cast between integer or float widths.

## Floats

The lexer, parser, and typed IR know:

```ari
f32
f64
f128
let scientific = 1.0e+1
```

The default LLVM host backend lowers `f32`, `f64`, and `f128` values as LLVM
`float`, `double`, and `fp128`. The `--freestanding` backend can store and load
local `f32`/`f64` literal values as raw IEEE bit patterns, including
`ptr_load`, `ptr_store`, and `*pointer` access through `ptr f32` or `ptr f64`.
It also lowers `f32`/`f64` arithmetic and ordered comparisons with SSE scalar
instructions, including `f32`/`f64` width casts, integer/float casts, and
direct Ari calls that pass or return `f32`/`f64`. `print` and `println` can
format `f32`/`f64` values with `{}` or `{:.N}` on both the LLVM host and raw
freestanding backends. It still rejects `f128` until the remaining native
floating-point work is implemented.

## Strings

The front end recognizes `string` and string literals:

```ari
let label: string = "count"
```

The default LLVM host backend lowers `string` as an `i8*`/C string pointer,
which makes string literals useful for C FFI, context arguments, and host
`read_line` helpers. Plain host line input returns a pointer into one internal
reusable buffer, while `read_line_owned(ref mut Zone)`,
`io::read_line_owned(ref mut Zone)`, `input::owned_line(ref mut Zone)`, and
`input_owned(ref mut Zone)` copy that line into a tracked source
`std::string::String` handle. The `--freestanding` backend still supports
string literals only as compile-time format strings for `print` and `println`.

The uppercase root type `String` is now the public prelude alias for
`std::string::String`; `std::String` names the same handle. Use lowercase
`string` for today's borrowed pointer-shaped string values.

The source prelude already has the allocator-backed seed under `std::string`.
`std::string::new(ref mut zone, capacity)` creates a tracked
`std::string::String` handle with separate `len` and `capacity` metadata over
zone-backed bytes, and `std::string::from_string(ref mut zone, text)` copies a
borrowed lowercase `string` into that handle.
`std::string::from_slice_in(ref mut Zone, Slice[u8])` copies a borrowed byte
slice into a new target-zone string handle. It supports checked byte
`first`/`last`/`get`/`set`/`replace`, fixed-capacity `push`/`pop`/`insert`, same-zone
grow-on-demand `reserve`, `reserve_extra`, `push_in`, `insert_in`,
`extend_from_slice_in`, and `resize_in`, plus `append_string_in`,
`append_i64_in`, `append_u64_in`, `append_bool_in`, `append_f32_in`,
`append_f64_in`, `truncate`, `clear`,
byte search with `contains`, `index_of`, and `count`, `as_ptr`, `as_slice`,
`equals(Slice[u8])`, `starts_with(Slice[u8])`, `ends_with(Slice[u8])`,
method-style `copy_to(ref mut Zone)`, and top-level
`std::string::copy_to(ref value, ref mut Zone)`. The zone argument passed to a
grow or append method must be the same source zone that created the handle.
Metadata, checked byte `first`/`last`/`get`, byte search, exact/prefix/suffix
checks, `as_ptr`, and target-zone copy borrow the string handle instead of
copying it. `String` is still an
explicit-zone handle: `zone::reset` or `zone::destroy` releases the bytes, and
dropping the handle only ends that binding.

## Tuples

Tuple types and literals are type-checked:

```ari
let done: () = ()
let pair: (i64, bool) = (7, true)
let first = pair.0
```

Fixed-size local tuples lower as stack aggregates on both executable backends.
Tuple index access such as `pair.0` extracts the indexed field. Tuple values can
also be destructured in local `let`/`var` bindings:

```ari
let () = done
let (left, right) = pair
```

`()` is Ari's unit value and its type is the empty tuple type. It is useful for
APIs that should return an explicit no-data value while still participating in
normal expression, binding, and match syntax. Single-element tuple types and
literals remain unsupported, so `(T)` and `(value)` are grouping forms rather
than tuples.

Direct freestanding calls can pass and return tuple values through hidden
pointer slots. Tuple FFI layout and broader aggregate pattern matching remain
planned.

## Structs

Struct declarations, local struct literals, and local field access are
executable:

```ari
struct Point {
  x: i64,
  mut y: i64,
}

struct Pair(i64, mut i64)

fn main() -> i64 {
  var point = Point { x: 10, y: 2 };
  point.y = point.x + 5;

  var pair = Pair(3, 4);
  pair.1 = point.y;
  pair.0 + pair.1
}
```

Struct fields are comma-separated. A trailing comma is accepted; semicolons are
reserved for statements and are rejected inside struct field lists.

Named structs use `Type { field: value }` literals and `value.field` access.
Tuple structs use function-style construction and positional access such as
`pair.0`. Assigning through a field requires both a `var` binding and a field
declared with `mut`; immutable fields remain readable but cannot be assigned.

Inherent impl blocks can provide constructor-style associated functions. The
preferred spelling is `T::new(...)`, implemented as an ordinary static call:

```ari
impl Point {
  fn new(x: i64, y: i64) -> Point {
    Point { x: x, y: y }
  }
}

let point = Point::new(4, 5)
```

Constructors that allocate into an explicit zone are ordinary associated
functions too. They take the allocation capability as an argument and can return
a typed raw pointer:

```ari
impl Point {
  fn new(zone: ref mut Zone, x: i64, y: i64) -> ptr Point {
    zone::new<Point>(zone, Point { x: x, y: y })
  }
}

var zone = zone::create(128)
let point = Point::new(ref mut zone, 4, 5)
zone::destroy(zone)
```

The current layout is intentionally local. Generic struct declarations,
generic struct field type checking, explicit type-argument construction, and
field/argument-based type-argument inference work:

```ari
struct Box[T] {
  value: T,
}

struct Pair[Left, Right](Left, Right)

let boxed = Box<i64> { value: 20 }
let pair = Pair<i64, bool>(boxed.value, true)

let inferred_box = Box { value: 20 }
let inferred_pair = Pair(inferred_box.value, true)
```

Inference uses the actual field or tuple-constructor argument types. If a
generic parameter appears in multiple fields, all uses must infer the same type;
parameters that never appear in the constructor inputs still require explicit
type arguments.

Generic inherent impl blocks can define methods whose `self` receiver is a
generic aggregate. The method is specialized when it is called on a concrete
receiver type:

```ari
impl[T] Box[T] {
  fn get(self) -> T {
    self.value
  }
}

let number = Box { value: 20 }.get()
let flag = Box { value: true }.get()
```

Inherent impl methods may also have method-level generics. Declarations still
use `[U]`; calls may use explicit `<U>` arguments after the method name, or
infer those method arguments from ordinary call arguments:

```ari
impl[T] Box[T] {
  fn replace[U](self, value: U) -> Box[U] {
    Box { value: value }
  }
}

let explicit = Box { value: true }.replace<i64>(20)
let inferred = Box { value: true }.replace(20)
```

Method-level generic parameters can use trait bounds. The bound is checked when
the method is specialized. A method-level generic that appears only in the
return type must be written explicitly, because Ari cannot infer it from the
arguments.

Generic trait impl blocks over generic aggregates specialize the same way when
their methods are called directly or through a generic trait bound:

```ari
trait Inner[T] {
  fn inner(self) -> T
}

impl[T] Inner[T] for Box[T] {
  fn inner(self) -> T {
    self.value
  }
}
```

Generic impl parameters can have trait bounds:

```ari
impl[T: Score] Box[T] {
  fn score(self) -> i64 {
    self.value.score()
  }
}
```

The bound is enforced when the method or trait impl is specialized for a
concrete receiver type.

Generic inherent impl blocks can also define associated functions. The call
can use explicit `<T>` arguments after the associated function name, or infer
them from normal arguments:

```ari
impl[T] Box[T] {
  fn new(value: T) -> Box[T] {
    Box { value: value }
  }
}

let boxed = Box::new<i64>(20)
let inferred_boxed = Box::new(20)
```

Associated functions can also declare method-level generics:

```ari
impl Factory {
  fn make[U](value: U) -> Box[U] {
    Box { value: value }
  }
}

let explicit = Factory::make<i64>(20)
let inferred = Factory::make(20)
```

Trait impl blocks can also implement associated functions declared by a trait.
Call them through the implementing type name, with explicit or inferred generic
arguments when the impl is generic:

```ari
trait Make[T] {
  fn make(value: T) -> Self
}

impl[T] Make[T] for Box[T] {
  fn make(value: T) -> Box[T] {
    Box { value: value }
  }
}

let boxed = Box::make<i64>(20)
let inferred = Box::make(true)
```

If two traits provide the same associated function for one type, qualify the
call with the trait and provide the implementing type after the function name,
or let Ari infer it from an explicit expected result type:

```ari
let boxed = Make<i64>::make<Box[i64]>(20)
let inferred: Box[i64] = Make<i64>::make(20)
let branched: Box[i64] = if ready { Make<i64>::make(1) } else { Make<i64>::make(2) }
```

Generic trait methods with method-level bounds are supported too. Generic
inherent and trait impl associated functions are supported.

Direct freestanding calls can pass and return struct and tuple-struct values
through hidden pointer slots. Struct FFI layout and broader non-local aggregate
materialization remain planned. Local struct values can be destructured in
`let`/`var` bindings:

```ari
let Point { x, y: renamed } = point
```

## Collection Literals

Non-empty `[...]` expressions default to fixed-size array literals when no
expected type is present:

```ari
let values = [1, 2, 3]      // [i64, 3]
let first = values[0]
let size = len(values)
```

The same literal syntax can initialize a `Vec[T]` when the surrounding type
context asks for one:

```ari
let fixed: [i64, 3] = [1, 2, 3]
let growable: Vec[i64] = [1, 2, 3]
```

For typed arrays and vectors, the expected element type is also passed into each
literal element expression.

Bare `[]` has no element type or length by itself. Today it is accepted only
where an explicit `Vec[T]` type supplies the element type.

## Vectors

Vector surface syntax is known to the language:

```ari
let values: Vec[i64] = [1, 2, 3]
```

`Vec[T]` is a prelude vector type. The current executable subset lowers vector
literals stored in local bindings as stack-backed values with a known local
capacity and a runtime length:

```ari
var values: Vec[i64] = [1, 2, 3]
let second = values[1]
```

An explicit `Vec[T]` annotation can provide the element type for an empty
literal:

```ari
let empty: Vec[i64] = []
var values: Vec[i64] = []
values = [1, 2, 3]
values = []
```

Bare `[]` still has no element type on its own, so use an annotation or assign
it to an existing `Vec[T]` binding. A non-empty `[...]` expression without that
context is an array literal, not an inferred `Vec[T]`.

Reassigning a local vector from another vector literal can change the runtime
length:

```ari
var values: Vec[i64] = [1, 2]
values = [3, 4, 5]
values = [9]
```

Use `len(values)` or method syntax `values.len()` to read the current runtime
length. Use `values.is_empty()` to compare that runtime length with zero.
When a local vector is initialized or assigned from another local vector whose
current length is still compiler-known, the compiler preserves that known
length for later `len`, `is_empty`, `as_slice`, `for`, and static-index checks.
Assigning from another local vector also widens the target's fixed local
storage to the source vector's current storage capacity before the copy.
Vec-valued `if`, block, and labeled-block expressions participate in this
tracking when their nested results have a fixed local capacity. Same-length
`if` arms and same-length labeled-block final or `break label value` paths
preserve the compiler-known current length, including paths that return local
vector bindings whose current lengths are still compiler-known.
Vec-valued `match` and `if let` expressions also size their expected local
storage before branch result materialization, so same-length literal, block, or
local-binding arms preserve the known length there too; match arms may use
labeled blocks whose typed Vec breaks require more storage than the final block
value.
`len(...)`, `.len()`, `.is_empty()`, and direct indexing of such Vec-valued
control-flow expressions use the same source-known local lengths for constants
and static out-of-range diagnostics.
The same `len(view)`, `view.len()`, and `view.is_empty()` forms read the stored
length from a `Slice[T]` view. `view[index]` reads or writes through the view's
stored raw pointer with bounds checks against that stored length. Mutable local
fixed arrays and mutable local `Vec[T]` values can create a non-owning writable
view with `values.as_slice()`; local vector views store a constant length when
the compiler knows the current local vector length. Slice views can be sliced
again with `view[start..end]` or `view[start..=end]`; the result stores an
adjusted raw pointer and length after checking the requested range against the
source view. `first()`, `last()`, and `get(index)` are checked read-only
accessors. `contains(value)`, `index_of(value)`, and `count(value)` scan
comparable elements, while `equals(view)`, `starts_with(view)`, and
`ends_with(view)` compare against another `Slice[T]`. `copy_to(ref mut Zone)`
copies the current view into a target-zone `std::vec::Vec<T>` handle whose
reset/destroy provenance follows the target zone. The LLVM host backend and raw
freestanding backend both lower the local Slice view surface for materializable
element values; target-zone Slice copy is an LLVM host explicit-zone path.
Array lengths, including direct array literal lengths, are folded directly:

```ari
let before = len(values)
values = [3, 4, 5]
let after = values.len()
let empty = values.is_empty()
let literal = [10, 20, 30].len()
```

Local vectors also support fixed-capacity methods:

```ari
var values: Vec[i64] = []
values.reserve(3)
values.push(4)
values.push(5)
let first = values.first()
let current_last = values.last()
let by_index = values.get(1)
values.swap(0, 1)
values.insert(1, 7)
let has_seven = values.contains(7)
let seven_index = values.index_of(7)
let seven_count = values.count(7)
let last = values.pop()
let capacity = values.capacity()
let empty = values.is_empty()
values.truncate(1)
values.set(0, 9)
let removed = values.remove(0)
values.clear()
```

That local method list is intentionally frozen while `Vec[T]` is still a
stack-backed executable subset. Other compiler-known collection conveniences
are reserved for the future allocator-backed std library design.
The root `Vec[T]`/`std::Vec[T]` value is local-only in this phase: it can be a
local binding, local expression result, or local vector view source, but it does
not have a non-local runtime ABI. Function and extern parameters/returns,
struct fields, and impl receivers reject root `Vec[T]` until the
runtime-capacity layout is defined. Use `std::vec::Vec<T>` when a value must be
passed as an explicit-zone heap handle, or pass `Slice[T]` for a borrowed view.

For the allocator-backed path, `std::vec::alloc_buffer<T>(ref mut zone,
capacity)` now provides the raw element-buffer seed. It takes an explicit
`Zone` capability, checks that the requested capacity is non-negative, returns
`null` for zero capacity, and otherwise allocates `capacity * size_of<T>()`
bytes with `align_of<T>()`. The result is a tracked `ptr T`, so using it after
`zone::reset` or `zone::destroy` is rejected.
`std::vec::with_capacity<T>(ref mut zone, capacity)` wraps the same allocation
in a tracked `std::vec::RawVec<T>` handle:

```ari
pub struct RawVec[T] {
  data: ptr T,
  mut len: i64,
  capacity: i64,
}
```

`std::vec::new<T>(ref mut zone, capacity)` wraps that raw handle in a tracked
source `std::vec::Vec<T>` seed:

```ari
pub struct Vec[T] {
  raw: RawVec[T],
}
```

This source `std::vec::Vec<T>` handle is the allocator-backed construction
seed. It exposes checked methods over the raw handle:

```ari
var vec = std::vec::new<i64>(ref mut zone, 4)
vec.push(10)
vec.push(20)
vec.push_in(ref mut zone, 30)
vec.reserve(ref mut zone, 8)
vec.reserve_extra(ref mut zone, 2)
vec.insert(1, 15)
vec.insert_in(ref mut zone, 2, 18)
vec.resize_in(ref mut zone, 8, 0)
vec.set(0, 25)
let old = vec.replace(1, 16)
vec.swap(0, 2)
let copied = vec.copy_to(ref mut other_zone)
let raw = vec.as_ptr()
let empty = vec.is_empty()
let len = vec.len()
let cap = vec.capacity()
let middle = vec.get(1)
let view = vec.as_slice()
let same = vec.equals(view)
let has_prefix = vec.starts_with(view)
let has_suffix = vec.ends_with(view)
let copied_from_view = std::vec::from_slice_in<i64>(ref mut other_zone, view)
vec.extend_from_slice_in(ref mut zone, view)
let removed = vec.remove(1)
let popped = vec.pop()
vec.truncate(1)
vec.clear()
```

The root `Vec[T]`/`std::Vec[T]` type is still the current local vector literal
storage until runtime heap growth is ported. Source `std::vec::Vec<T>.reserve`
is grow-only: it allocates a larger buffer from the same explicit zone, copies
the current elements, preserves `len`, and leaves the old buffer to the zone's
bulk lifetime. `std::vec::Vec<T>.reserve_extra(ref mut Zone, additional)`
grows to at least `len + additional`, which lets callers reserve append room
without recomputing the current length. `std::vec::Vec<T>.push_in(ref mut Zone,
value)` uses the same explicit zone capability and grows when the current
capacity is full before appending. `std::vec::Vec<T>.insert_in(ref mut Zone,
index, value)` follows the same explicit-zone growth rule before inserting and
shifting later elements.
`std::vec::Vec<T>.extend_from_slice_in(ref mut Zone, Slice<T>)` appends each
slice element through that same growth path. `std::vec::Vec<T>.resize_in(ref
mut Zone, length, value)` shrinks by setting `len` or grows by appending copies
of `value`. `std::vec::from_slice_in<T>(ref mut Zone, Slice<T>)` builds a new
target-zone source `Vec<T>` by copying the borrowed slice elements into fresh
zone storage. `replace(index, value)` returns the previous element at `index`
after storing the new value. Passing a different zone borrow to `reserve`,
`reserve_extra`, `push_in`, `insert_in`, `extend_from_slice_in`, or
`resize_in` is rejected because the source handle remains tied to the zone that
created it. Dropping the source `Vec<T>` handle runs `drop` on each current
element, but the explicit zone still owns the backing allocation.
`std::vec::Vec<T>.copy_to(ref mut Zone)` copies the current elements into a new
handle tied to the target zone, so resetting the source zone does not
invalidate the copy. `std::vec::Vec<T>.as_ptr()` returns the stored element
pointer with the receiver's zone provenance intact. `vec.as_slice()` returns a
`Slice[T]` over the same zone-backed buffer, and that slice is rejected after
the source zone is reset or destroyed. `equals(view)`, `starts_with(view)`, and
`ends_with(view)` compare the current Vec elements with a borrowed `Slice[T]`
view. Metadata, checked read, search, Slice comparison, target-zone copy, and
raw-pointer methods borrow the source handle receiver instead of copying it.

`reserve(n)` accepts any integer capacity. A non-negative integer literal,
integer constant, static integer arithmetic/bitwise/shift expression over
constants and literals, or immutable local integer binding initialized from one
of those static expressions widens the compiler-known local storage capacity
for that binding. Other runtime integer values lower to a bounds check against the
current reserved local capacity and panic through `panic` when the requested
capacity is negative or larger than that fixed local storage. It does not
allocate heap storage yet.
`push(value)` appends a copyable element, increments the runtime length, and
auto-widens the stack-backed local capacity when the compiler can track the
binding's current length. If the length is not precise, `push` reserves one
additional static slot so the immediate append cannot hit a full local buffer.
`capacity()` returns that reserved local capacity as an `i64`. `is_empty()`
returns `true` when the current runtime length is zero and does not inspect
reserved capacity. `len(values)`, `values.len()`, and `values.is_empty()` lower
to constants when the compiler knows the current local vector length.
`clear()` sets the current runtime length to zero while keeping the reserved
local capacity.
`pop()` returns the last copyable element, decreases the current runtime length,
and panics through `panic` when the vector is empty. It does not shrink reserved
local capacity. If the compiler knows the current local vector length is zero,
the call is rejected before lowering.
`first()` and `last()` read the first or last copyable element without changing
the runtime length, and use the same bounds checks as indexing, so empty
vectors panic. If the compiler knows the current local vector length is zero,
the call is rejected before lowering.
`get(index)` reads a copyable element at a runtime index without changing the
runtime length, and uses the same bounds checks as `values[index]`. Literal,
constant, static-expression, and immutable local indexes are rejected at compile
time when negative, and when outside the compiler-known current length, for
`get`, `set`, `swap`, `remove`, and `insert`; other runtime indexes still use
the vector bounds checks. Runtime-indexed `get`, `set`, `swap`, and `remove`
calls are rejected before lowering when the compiler knows the current local
vector length is zero.
`contains(value)` returns `true` when a copyable comparable element equal to
`value` appears inside the current runtime length. `index_of(value)` returns
the first matching runtime index as `i64`, or `-1` when no element matches.
`count(value)` returns the number of matching elements as `i64` and scans the
current runtime length.
`truncate(n)` shrinks the current runtime length to `n` when `n` is smaller
than the current length, leaves it unchanged when `n` is larger, and panics for
negative runtime lengths. Literal lengths, integer constants, static integer
arithmetic/bitwise/shift expressions, and immutable local integers initialized
from those expressions are rejected at compile time when negative; otherwise
they keep the compiler's local length tracking precise for later `push` and
`insert` storage decisions. `set(index, value)` overwrites an existing element
inside the current runtime length and uses the same bounds checks as indexing.
`swap(a, b)` exchanges two elements inside the current runtime length, with the
same bounds checks on both indexes.
`remove(index)` returns the removed copyable element, shifts later elements one
slot toward the front, decreases the runtime length, and keeps reserved local
capacity unchanged. `insert(index, value)` accepts any index from zero through
the current runtime length, shifts later elements one slot toward the back,
stores the new copyable value, and increases the runtime length. Mutating
methods such as `reserve`, `push`, `insert`, `pop`, `remove`, `clear`,
`truncate`, and `set`/`swap` require a `var` binding. Read-only methods such as
`capacity`, `is_empty`, `first`, `last`, `get`, `contains`, `index_of`, and
`count` work on immutable local vectors too.

The compiler reserves enough local storage for the largest vector literal,
explicit `reserve` capacity, or tracked `push` growth seen for that binding,
but indexing checks the current runtime length, not the reserved capacity. The
storage is deliberately not heap allocation yet. Allocator-backed growth,
slicing, and non-local vector ABI are still planned. On the raw
`--freestanding` backend, stored local vector literals, local copies, scalar
indexing, `len`, `is_empty`, `capacity`, `first`, `last`, `get`, `contains`,
`index_of`, `count`, `reserve`, `push`, `insert`, `pop`, `remove`, `clear`,
`truncate`, `set`, `swap`, and stored-vector `for` loops lower through the same
fixed local layout. These methods still use local capacity traps rather than
allocator-backed growth.

Array literals support constant indexing without materializing a runtime
aggregate:

```ari
let second = [10, 20, 30][1]
```

Stored local vectors support checked dynamic indexing. Static local vector
indexes are rejected at compile time when they fall outside the compiler-known
current length, and dynamic local vector indexing is rejected before lowering
when the compiler knows the current length is zero. A direct array literal index
still requires a constant index so the compiler can read the element without
materializing a temporary aggregate:

```ari
var index = 1
let second = values[index]
let first = [10, 20, 30][0]
```

Stored local vectors can be used as `for` loop iterables on the LLVM and raw
freestanding backends:

```ari
let values: Vec[i64] = [1, 2, 3]
for value in values {
  println("{}", value)
}
```

The loop visits the current runtime length, not reserved capacity; when the
compiler knows that current length, it lowers the loop bound as a constant.

Non-empty list literals can be consumed directly by `for`:

```ari
for value in [1, 2, 3] {
  println("{}", value)
}
```

## Fixed Arrays

Fixed arrays are value aggregates with a compile-time length. They use
`[type, size]` in type position and are stored inline rather than on the heap:

```ari
let values: [i64, 3] = [1, 2, 3]
let first = values[0]
```

Array literals must have exactly the declared length. Constant indexes are
checked at compile time, and local arrays can also be indexed with integer
expressions. Dynamic indexes are checked at runtime; a negative index or an
index greater than or equal to the array length traps through the panic path.
Non-empty array literals infer `[T, N]` from their element type and arity when
there is no expected collection type.

Fixed arrays can be destructured in local bindings and `match` arms:

```ari
let [first, .., last]: [i64, 4] = [1, 2, 3, 4]

let score = match values {
  [0, _, ..] => 1,
  [head, .., 9] => head,
  [..] => 0,
}
```

Array patterns are fixed-size product patterns. `..` skips a range of elements
but does not bind a slice yet. The same `[a, .., b]` spelling also works for
local `Vec[T]` storage and `Slice[T]` views by lowering to `len(...)` checks and
indexed element reads; runtime sequence matches need a final `_` or `[..]`
fallback arm when earlier arms are length-checked.

## Ownership-Qualified Types

```ari
own Buffer
ref Buffer
ref mut Buffer
mut ref Buffer
ptr Buffer
```

Meanings:

- `own T`: unique owner, moved or dropped exactly once
- `ref T`: immutable borrow
- `ref mut T`: exclusive mutable borrow; `mut ref T` is an equivalent spelling
- `ptr T`: raw pointer surface for FFI and explicit memory code
- `T?`: nullable raw-pointer spelling for `ptr T`
- `null`: nullable raw-pointer literal; it defaults to `ptr c_void` unless a
  `ptr T` or `T?` type is expected
- `value as ptr U`: raw pointer casts, pointer/integer address casts,
  lowercase `string` to raw pointer casts, and `(ref mut value) as ptr U`
  borrow-to-raw-pointer casts use ordinary explicit casts
- `ptr_offset(value, bytes)`: explicit byte-wise raw pointer offset; the result
  has the same `ptr T` type as `value`
- `ptr_add(value, count)`: explicit typed raw pointer offset; the result moves
  by `count * sizeof(T)` for scalar and Ari-layout aggregate `ptr T`
- `size_of<T>()`: compiler-known layout size query for scalar and Ari-layout
  aggregate types, returned as `i64`
- `align_of<T>()`: compiler-known layout alignment query for scalar and
  Ari-layout aggregate types, returned as `i64`
- `ptr_load(value)`: explicit scalar, plain Ari-layout aggregate, or supported
  aggregate enum raw pointer load from `ptr T`
- `ptr_store(value, item)`: explicit scalar, plain Ari-layout aggregate, or
  supported aggregate enum raw pointer store through `ptr T`
- `*value`: scalar, plain Ari-layout aggregate, or supported aggregate enum
  dereference load/store syntax for `ptr T`
- `(*value).field`, `(*value).0`, `(*value)[index]`: scalar field or fixed-array
  element access through a raw pointer to an Ari aggregate layout
- `Zone`: explicit allocation region capability; `zone::create` returns
  `own Zone`, `zone::temp` returns a lexical automatically destroyed
  temporary zone, raw `zone::alloc` allocates bytes from `ref mut Zone`,
  `zone::alloc<T>` allocates `size_of<T>()` bytes with `align_of<T>()`
  alignment and returns `ptr T`, `zone::new<T>` stores an initial `T` value into
  zone memory, `zone::scratch<T>` creates a local scratch pointer through a
  hidden temporary zone, `zone::promote<T>` copies a pointed-to value into an
  explicit target zone, and `zone::destroy` releases a non-temporary region
- `String`: root alias for the source `std::string::String` explicit-zone
  handle. Lowercase `string` remains today's borrowed C-string pointer-shaped
  value.
- `Box[T]`: root alias for the source `std::boxed::Box<T>` explicit-zone
  handle. Construct it with `Box::new<T>(ref mut Zone, value)`,
  `std::Box::new<T>(ref mut Zone, value)`, or
  `std::boxed::new<T>(ref mut Zone, value)`; the handle's `Drop` path runs the
  stored value's destructor when one exists, but the explicit zone still owns
  and releases the backing bytes.
- `Unique[T]`, `Shared[T]`, and `Weak[T]`: reserved root smart-pointer names.
  `Unique[T]` remains reserved for policy compatibility, and `Shared[T]` /
  `Weak[T]` are reserved for future reference-counted ownership.
- `fn(T, U) -> R`: function pointer value with checked indirect call syntax
- `dyn Trait[...]`: trait object type syntax. The compiler resolves the trait
  and its generic arguments in type positions today. Explicit
  `value as dyn Trait[...]` conversions are checked against matching impls.
  Concrete copyable non-borrow values lower on the LLVM backend as
  `{data pointer, vtable pointer}` and method calls dispatch through vtable
  thunks. Generic impls can be specialized into vtables for concrete object
  types. Generic trait methods are not object-safe and stay statically
  dispatched. Dyn-to-dyn upcasts are rejected; non-copy dyn data ownership and
  raw backend lowering are still planned.

The executable subset supports scalar `own` values, scalar and aggregate `ref`
/ `ref mut` values, and can preserve borrow qualifiers inside local tuple,
fixed-array, vector, and struct fields:

```ari
struct TokenBox {
  token: own i64,
  shared: ref i64,
}

let pair: (own i64, i64) = (make_owned(1), 2);
```

An aggregate that contains an `own` field is tracked as one move-only binding,
but owned struct fields, tuple-struct fields, nested field paths, and constant
fixed-array/vector element paths can be moved and reinitialized independently.
Aggregate bindings that contain `ref` or `ref mut` fields track each borrowed
field path independently. Reassigning a local aggregate or one of its
borrow-valued fields releases the replaced field sources after the new value is
checked, and shared borrow-valued aggregate copies acquire their own source
borrow records. Fields and elements can also be borrowed directly with
`ref value.field`, `ref mut value.0`, or constant `ref value[index]`;
unrelated field paths remain available while that borrow is live. Existing
local borrow bindings can be reborrowed through those same paths, such as
`ref borrowed.field` or `ref mut borrowed[0]`, when the borrow binding's own
mutability and the selected field path allow it. Shared struct borrows also
support read-only field projection such as `borrowed.field` or `self.field`
inside `self: ref Self` methods. Local named borrow bindings release their
source after their last visible straight-line use, while dependent reborrows
keep the intermediate borrow binding live until the dependent borrow is also
dead.

`ptr T` can appear in FFI signatures and be passed around as a pointer-shaped
value. `T?` is accepted as the nullable spelling of the same raw pointer type,
so `i64?` canonicalizes to `ptr i64` and `c_void?` canonicalizes to
`ptr c_void`. It is not an `Option[T]` shorthand for ordinary values; use that
ADT explicitly when absence is part of a value model. `?`
is a postfix type suffix and cannot be combined with `own`, `ref`, or `ptr`
qualifiers. `null` can initialize or be passed to any `ptr T` / `T?` expected
type.
`ptr_offset(pointer, bytes)` performs byte-wise address arithmetic without
loading from memory. `ptr_add(pointer, count)` performs typed pointer
arithmetic, scaling by the current Ari layout size of `T` for `ptr T`; scalar
and aggregate element types are supported. `size_of<T>()` and `align_of<T>()`
return the current scalar or Ari-layout aggregate byte size/alignment as `i64`.
Aggregate layout queries use field order, natural scalar alignment, array
element stride padding, and final aggregate padding to the maximum field
alignment. They are Ari layout queries, not a C ABI promise; use `@repr(C)`
surfaces for foreign layout once that ABI is fully specified. The raw
`--freestanding` backend uses this Ari layout for local tuple, struct,
tuple-struct, fixed-array, and fixed-capacity local vector storage, whole plain
aggregate copies, and scalar field or element access through raw aggregate
pointers. Local and
pointer-backed aggregate enum storage works for the currently supported payload
slot types. Direct raw calls can pass and return aggregate enum values through
hidden pointer slots; aggregate enum external ABI classification remains
planned there.
`ptr_load(pointer)` and
`ptr_store(pointer, value)` provide explicit unchecked scalar, plain
Ari-layout aggregate, or supported aggregate enum memory access through raw
pointers. `*pointer` provides the same dereference operation, and
`(*pointer).field`, `(*pointer).0`, or `(*pointer)[index]` can read and write
scalar slots inside a raw pointer to a struct, tuple struct, tuple, or fixed
array. Whole-aggregate raw pointer copies
are intentionally rejected for aggregates that contain `own`, `ref`, or
`ref mut` fields until the zone and ownership diagnostics are broadened.

`Zone` values are explicit allocation regions on the LLVM host backend. A zone
is created with `zone::create(capacity)` for manual ownership or
`zone::temp(capacity)` for a lexical scratch region that is destroyed
automatically when its declaring scope falls through, before returns, and before
`break`, `continue`, or labeled-block exits that leave that scope. Temporary
zones must be bound to a local and cannot be moved out. Zones are borrowed
mutably for raw byte allocation with
`zone::alloc(ref mut zone, bytes, align)`, or allocated with a typed pointer
result using `zone::alloc<T>(ref mut zone)`. The typed form still does not
construct or destroy `T`; it only computes layout and returns `ptr T`.
`zone::new<T>(ref mut zone, value)` performs placement construction by
allocating `T`-sized memory and storing the provided value there. It does not
register destructors or track zone-contained live values.
`zone::scratch<T>(capacity, value)` can initialize a local pointer binding by
creating a hidden `zone::temp(capacity)` and placing `value` in it. It is for
short-lived objects whose pointer stays inside the declaring lexical scope.
`zone::promote<T>(ref mut target, source)` copies a pointed-to `T` value into
an explicit target zone and returns a pointer tied to that target zone, which is
the escape hatch for keeping a scratch value after the scratch scope ends.
Zones can be reset with `zone::reset(ref mut zone)` and released by moving the owner into
`zone::destroy(zone)` unless they are temporary zones, where destruction is
inserted by the checker. Pointers from temporary zones cannot escape into
longer-lived bindings, returns, aggregates, or escape-prone call arguments; the
diagnostic names the pointer and the temporary zone source. Associated
constructor spelling such as `T::new(...)` is supported by ordinary inherent
impl methods that call `zone::new<T>` or another explicit allocator helper.
Direct local pointers from `zone::alloc<T>` and `zone::new<T>`,
single-zone pointer-returning calls, source `std::boxed::Box<T>` handles, and
source `std::vec::RawVec<T>` or `std::vec::Vec<T>` handles returned from a
single-zone constructor are
invalidated by `zone::reset` and `zone::destroy` in the checker, so using such
a binding afterward is rejected.
A pointer-returning function with no zone borrow parameter or more than one
zone borrow parameter cannot return a tracked zone pointer because the source
zone would be ambiguous to the checker. Reset
invalidation is merged
through ordinary control-flow joins such as `if`, `match`, labeled blocks, and
loops. Zone pointers are local capabilities: aggregate storage, raw-pointer
storage, extern C/function-pointer call arguments, and multi-zone pointer
returns are rejected unless a future capability model makes that escape
explicit.
