# Memory And Ownership

## Direction

Ari aims for explicit memory management that is stricter than C and C++ without
using a hidden garbage collector.

The language should make ownership transfer, borrowing, allocation, and
destruction visible in source code, then reject common lifetime mistakes at
compile time.

## Core Rules

- Values live in one owner at a time unless their type is explicitly cheap to
  copy.
- Resource-owning values move by default.
- After a move, the old binding cannot be read, assigned, borrowed, or dropped.
- Owned values must be consumed, returned, moved, or dropped exactly once.
- Assigning into a live owner is rejected.
- An owned expression result cannot be silently discarded.
- A return path cannot leave live owners behind.
- Branches must merge to compatible ownership states.
- Loops currently cannot change the ownership state of outer bindings.
- Borrowing is temporary and checked.
- Any number of immutable borrows may exist, or one mutable borrow may exist,
  but not both.

## Owning Values

```ari
fn consume(value: own i64) -> i64 {
  drop value;
  return 0;
}

fn main() -> i64 {
  let token: own i64 = 42;
  consume(token);
  return 0;
}
```

The call to `consume` moves `token`. Reading `token` after that call is rejected.

## Borrowing

Borrows can be passed directly to calls, or bound to a local borrow binding:

```ari
fn inspect(value: ref u32) -> i64 {
  return 0;
}

fn touch(value: ref mut i16) -> i64 {
  return 0;
}

fn main() -> i64 {
  var number: u32 = 7;
  var slot: i16 = 0;
  inspect(ref number);
  touch(ref mut slot);
  {
    let borrowed: ref u32 = ref number;
    inspect(borrowed);
  }
  return 0;
}
```

Rules currently checked:

- `ref mut` requires a `var` binding
- a mutable borrow cannot overlap with any other borrow
- a binding cannot be moved, dropped, or assigned while borrowed
- borrowed aggregate fields and elements are tracked by path, so unrelated
  fields can still be read or assigned
- a named borrow keeps the source borrowed until the binding's block exits
- borrow bindings must be initialized directly with `ref` or `ref mut`
- borrow bindings cannot be reassigned
- borrow values cannot be returned
- bare borrow expression statements are rejected

Named borrow lifetimes are lexical today. Future borrow-checker refinement may
shorten a named borrow to its last use and support reborrowing from existing
borrow values.

## Drop

```ari
drop token;
```

`drop` consumes an owning binding. If the value type implements the prelude
`Drop` trait, the compiler lowers `drop value;` to that implementation's
`fn drop(self) -> void` method before marking the binding as dropped. Without a
`Drop` impl, dropping the currently supported primitive owners is a no-op in
codegen but still participates in ownership checking. Aggregates with owned
fields are tracked and dropped as whole bindings, and `drop aggregate;` lowers
destructor calls for owned tuple, fixed-array, vector, and struct fields that
provide a matching `Drop` impl.

Local aggregates can carry `own`, `ref`, and `ref mut` fields. Owned aggregates
move as one value. Borrow-valued aggregate bindings keep their sources borrowed
until the aggregate binding goes out of scope. Fields and elements can be
borrowed directly with `ref value.field`, `ref mut value.0`, or constant
`ref value[index]`, and those borrows block only overlapping paths. Non-owned
fields of owning structs and tuple structs can be read or assigned without
moving the whole aggregate. Owned struct and tuple-struct fields, nested owned
field paths, and constant fixed-array/vector indexes can be moved independently
and later reinitialized through mutable field or element assignment; while any
owned field or element has been moved out, moving the whole aggregate is
rejected. Moving an owning array or vector element through a dynamic index is
rejected; use a constant index or move the whole aggregate. Owned fields and
elements can be moved only from named local aggregates; moving them directly
out of temporary aggregate expressions is rejected, so bind the aggregate first.

## Allocation Direction

Allocation should be capability based. Code should receive an allocator or
region value explicitly instead of calling a global heap primitive.

Possible future shape:

```ari
trait Allocator {
  fn alloc(self: ref mut Allocator, bytes: i64, align: i64) -> ptr u8;
  fn free(self: ref mut Allocator, data: ptr u8, bytes: i64, align: i64);
}
```

Regions and zones are the preferred model for allocation-heavy code. A zone is
a large allocation area: values can be placed inside it freely, and destroying
the zone releases the whole contained area at once. Ari's memory model is not
intended to prove full memory safety; it is meant to make common C/C++-style
memory work more explicit and easier to audit.

Borrowing and ownership checks are useful diagnostics, not a promise that raw
memory operations cannot go wrong. The language should still allow explicit
escape hatches such as pointer casts, manual zone management, and explicit
pointer loads/stores.

`ptr T` can be stored and passed as an FFI pointer-shaped value, and `T?` is a
nullable spelling for the same raw pointer type. `null` constructs a nullable
raw pointer. `T?` is deliberately not a value-level maybe type; use
`Option[T]` or `Maybe[T]` when absence should be represented as data instead
of an address. Raw pointer casts use ordinary explicit casts, including
`ptr T` to `ptr U`, `T?` to another raw pointer type, `ptr T` to an integer
address, and an integer address back to `ptr T`. An explicit borrow can also
be converted to a raw pointer with `(ref value) as ptr T`, `(ref mut value) as
ptr T`, or the nullable spelling `(ref mut value) as T?`.

`ptr_offset(pointer, bytes)` performs an explicit byte-wise address offset and
returns the same raw pointer type as `pointer`. `mem::ptr_offset` is the same
compiler-known operation. It does not dereference the pointer, check bounds, or
scale by `T`.

`ptr_add(pointer, count)` performs an explicit typed address offset. For a
`ptr T`, it moves by `count * sizeof(T)` bytes and returns the same `ptr T`
type. The executable subset supports scalar types and Ari-layout aggregate
types such as structs, tuple structs, tuples, and fixed arrays. `ptr_add` is
still unchecked and does not prove the resulting address is in-bounds.

`size_of<T>()` and `align_of<T>()` are compiler-known layout queries that
return `i64` byte counts. They support scalar types, raw/borrow pointer-shaped
types, and Ari-layout aggregates. Aggregate results describe Ari's current
executable local layout, where aggregate fields and fixed-array elements occupy
8-byte slots; they are not a `repr(C)` ABI promise.

`ptr_load(pointer)` reads one scalar, plain Ari-layout aggregate, or supported
aggregate enum value from a `ptr T` and returns `T`. `ptr_store(pointer, value)`
writes one scalar, plain Ari-layout aggregate, or supported aggregate enum
value through a `ptr T` and returns `void`. The
`mem::ptr_load` and `mem::ptr_store` spellings are the same compiler-known
operations. These operations are deliberately unchecked: they do not test for
null, bounds, alignment, aliasing, or lifetime. On the freestanding backend,
`ptr f32` and `ptr f64` load/store values as raw IEEE bit patterns; `f128`
pointer access still waits for native float storage policy. Aggregates that
contain `own`, `ref`, or `ref mut` fields are rejected for whole raw-pointer
copies for now.

The same scalar operation can be written with dereference syntax:

```ari
var value: i64 = 4;
let raw: ptr i64 = (ref mut value) as ptr i64;
*raw = *raw + 8;
```

Dereference syntax supports scalar `ptr T` loads and stores, and whole
plain-aggregate copies for Ari-layout structs, tuple structs, tuples, and fixed
arrays. It is the same unchecked raw memory access as `ptr_load` and
`ptr_store`.

Raw pointers to local aggregate layouts can also address scalar fields and
elements without materializing the whole aggregate:

```ari
struct Point {
  x: i64,
  mut y: i64,
}

var point = Point { x: 4, y: 5 };
let point_ptr: ptr Point = (ref mut point) as ptr Point;
(*point_ptr).y = (*point_ptr).x + 8;

var numbers: [i64, 3] = [1, 2, 3];
let numbers_ptr: ptr [i64, 3] = (ref mut numbers) as ptr [i64, 3];
(*numbers_ptr)[1] = (*point_ptr).y;
```

Plain aggregate values can also be copied through raw pointers when the
aggregate does not contain ownership or borrow fields:

```ari
let copy = *point_ptr;
*point_ptr = Point { x: copy.x + 1, y: copy.y + 2 };
ptr_store(point_ptr, ptr_load(point_ptr));
```

## Zone Allocation

`Zone` is the first executable allocation capability. `zone::create(capacity)`
creates an owned zone backed by one host allocation; values can then be placed
inside it with explicit byte size and alignment. Destroying the zone releases
the whole contained area at once.

```ari
fn main() -> i64 {
  var zone = zone::create(64);
  let item = zone::alloc<i64>(ref mut zone);
  *item = 42;
  let result = *item;
  zone::destroy(zone);
  return result;
}
```

Available functions:

```ari
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

`zone::alloc` is unchecked raw memory allocation. It does not construct a `T`,
run destructors, track which values live in the zone, or prove that later raw
pointer loads/stores are valid. The generic `zone::alloc<T>` spelling computes
`size_of<T>()` and `align_of<T>()` at compile time and returns a typed raw
pointer; it is still allocation only. `zone::new<T>` allocates enough space for
`T`, stores the provided value into that memory, and returns `ptr T`. It is
placement construction only: it does not register a destructor or track that the
zone contains a live `T`.

`zone::scratch<T>(capacity, value)` is local-binding sugar for the common
temporary-object case. It can only appear as the initializer of a local `let` or
`var` binding. The checker creates a hidden lexical temporary zone, places
`value` into that zone with `zone::new<T>`, and binds the visible name to the
resulting `ptr T`:

```ari
fn score() -> i64 {
  let item = zone::scratch<i64>(64, 42);
  return *item;
}
```

The hidden zone is destroyed when the declaring scope exits, so pointers from
`zone::scratch<T>` follow the same escape rules as pointers from
`zone::temp`: they cannot be returned, stored into longer-lived bindings or
aggregates, or passed through escape-prone calls.

To keep a scratch value longer, copy it into an explicit target zone:

```ari
fn keep(zone: ref mut Zone) -> ptr i64 {
  let scratch = zone::scratch<i64>(64, 42);
  return zone::promote<i64>(zone, scratch);
}
```

`zone::promote<T>(target, source)` loads the `T` value through `source`, places
that copy into `target`, and returns a pointer whose lifetime is tied to
`target`. Like `zone::new<T>`, it does not run or register destructors and does
not support ownership- or borrow-valued `T` yet.

`zone::temp(capacity)` creates a lexical temporary zone. It must initialize a
local binding, usually a `var` so allocation can borrow it as `ref mut`. The
owner cannot be moved, returned, or passed as an owning argument; the compiler
inserts `zone::destroy` when the declaring scope falls through, before returns,
and before `break`, `continue`, or labeled-block exits that leave the
declaring scope. This is a raw memory reclaim policy for short-lived objects:
destructors for values placed inside the zone are not run.
Raw pointers allocated from a temporary zone cannot escape that lexical
lifetime. Returning such a pointer, storing it into an outer binding or
aggregate, or passing it through an escape-prone call is rejected with a
diagnostic that names both the pointer binding and the temporary zone source.

`zone::reset` only rewinds the allocation cursor; it does not destroy placed
values. Non-temporary `own Zone` values must be passed to `zone::destroy`;
plain `drop zone` is rejected so the release operation stays visible in source.
The checker tracks direct local pointers produced by `zone::alloc<T>` and
`zone::new<T>`, plus calls to pointer-returning functions or associated
constructors that take exactly one `ref Zone` or `ref mut Zone` parameter. Using
those bindings after the source zone has been reset or destroyed is rejected.
Reset invalidation is merged conservatively through `if`, `match`, `??`
fallbacks, labeled-block exits, and loops, so a pointer is considered invalid
after control flow if any continuing path may have reset its zone.
This is a diagnostic aid, not a full lifetime proof. Zone pointers cannot be
stored into tuple, struct, enum, or vector values, assigned into aggregate or
raw-pointer storage, passed through extern C or function-pointer calls, or
returned from functions unless the function has exactly one zone borrow
parameter and returns a pointer derived from that same parameter.

Zone-backed constructor-style APIs are ordinary associated functions. A type can
define `T::new(ref mut Zone, ...) -> ptr T` and delegate to `zone::new<T>`:

```ari
impl Point {
  fn new(zone: ref mut Zone, x: i64, y: i64) -> ptr Point {
    zone::new<Point>(zone, Point { x: x, y: y })
  }
}
```

Zone allocation lowers on the LLVM host backend with `malloc`/`free`.
The freestanding backend deliberately rejects zone allocation until Ari has a
raw-backend runtime allocation policy.
