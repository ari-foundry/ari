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

## Explicit Moves

`move(value)` and `take(place)` are prelude helpers for APIs where ownership
consumption should be visible at the call site. They do not add a second move
system; sema lowers them through the same ownership checker used by ordinary
reads.

```ari
let token = move(make_owned(7));
let field = take(holder.token);
```

`move(value)` accepts any non-borrow expression and returns it as the same type,
so owning locals are marked moved and owning temporaries can be named. Copyable
values remain copyable. `take(place)` is stricter: its argument must be a local
binding, field, tuple index, or index expression. Use `take` when an API wants
to make it clear that a concrete storage place is being consumed.

Both helpers reject borrow-valued results, because Ari does not allow storing
temporary borrow values through these generic prelude helpers.

## Borrowing

Borrows can be passed directly to calls, or bound to a local borrow binding.
The preferred spelling is `&T`, `&mut T`, `&value`, and `&mut value`; the
older `ref T`, `ref mut T`, `ref value`, and `ref mut value` spellings remain
accepted as explicit aliases.

```ari
fn inspect(value: &u32) -> i64 {
  return 0;
}

fn touch(value: &mut i64) -> i64 {
  return 0;
}

struct Pair {
  mut left: i64,
  mut right: i64,
}

fn main() -> i64 {
  var number: u32 = 7;
  var slot: i64 = 0;
  var pair = Pair { left: 10, right: 20 };
  let pick_first = true;
  inspect(&number);
  touch(&mut slot);
  {
    let borrowed: &u32 = &number;
    let again: &u32 = &borrowed;
    inspect(borrowed);
    inspect(again);
  }
  {
    let borrowed: &mut i64 = &mut slot;
    let again: &mut i64 = &mut borrowed;
    touch(again);
  }
  {
    let borrowed: &mut Pair = &mut pair;
    let right: &mut i64 = &mut borrowed.right;
    touch(right);
  }
  {
    let chosen: &u32 = if pick_first {
      &number
    } else {
      &number
    };
    inspect(chosen);
  }
  return 0;
}
```

Rules currently checked:

- `&mut` / `ref mut` requires a `var` binding
- a mutable borrow cannot overlap with any other borrow
- a binding cannot be moved, dropped, or assigned while borrowed
- borrowed aggregate fields and elements are tracked by path, so unrelated
  fields can still be read or assigned
- borrow-valued aggregate fields retain their own source paths; replacing a
  local aggregate or one borrow-valued field releases only the replaced field
  sources after the new value has been checked
- borrow-valued aggregate copies acquire their own source borrow records
- fields behind a `ref Struct` parameter can be read through the borrow, and
  fields behind a `ref mut Struct` parameter can also be assigned through the
  borrow, subject to the struct field's own `mut` marker
- inside a method with `self: ref mut Self`, read-only methods whose receiver is
  `self: ref Self` can be called directly through `self`; this also applies to
  trait-qualified calls such as `Trait::method(self)`
- a named borrow keeps the source borrowed until its last visible use in the
  current straight-line statement scope, or until the binding's block exits
  when the checker cannot shorten it
- borrow bindings must be initialized with `&`, `&mut`, `ref`, `ref mut`, or a
  compatible borrow-valued block, `if`, `match`, or labeled-block expression
  result
- an existing local borrow binding can be reborrowed with `ref` when the source
  is `ref` or `ref mut`
- an existing local `ref mut` borrow binding can be reborrowed with `ref mut`;
  immutable borrow bindings cannot be reborrowed mutably
- fields and constant-index elements behind an existing local borrow binding can
  also be reborrowed, so `ref borrowed.field`, `ref mut borrowed.0`, and
  `ref borrowed[0]` keep path-level conflict diagnostics
- borrow-valued control-flow expression results are allowed only when every
  result path borrows the same source path with the same borrow mode
- a borrow-valued control-flow expression cannot return a borrow of a binding
  declared inside that expression's arm or block
- a function may return `ref T` or `ref mut T` when the signature has exactly
  one borrow parameter and the returned borrow source traces back to that
  parameter, or when `@borrow_return(source.path)` names the tracked source
  explicitly
- when such a function or method always returns the same field or constant
  element below that parameter, callers keep only that returned subpath borrowed
- an extern borrow-returning declaration must use `@borrow_return(...)`; Ari
  cannot infer a source from a bodyless declaration
- a borrow return cannot come from a local binding, a value parameter, a
  function pointer call, or a function with multiple borrow parameters but no
  explicit source contract
- borrow bindings cannot be reassigned
- bare borrow expression statements are rejected

The focused ownership smoke is `make check-ownership`; it proves path
reborrowing together with move/drop fixtures and the compiler-shaped
`ownership-compiler-shaped.ari` program. `make check-errors` keeps the larger
negative matrix for borrow conflicts, invalid returns, local escapes, and
control-flow mismatch cases.
Source-aware ownership goldens also lock assignment while a binding or field is
borrowed, borrow-after-move, double-move, and invalid runtime-dependent enum
payload moves.

Named borrow lifetimes are shortened for local straight-line code. After the
last visible use of a named borrow in the current statement scope, Ari releases
that binding's source so later assignments or borrows can proceed in the same
block. A reborrow keeps the borrow binding it was created from borrowed until
the reborrow's own last visible use; only then can the source borrow binding
release its original source. A borrow-valued function or method call keeps the
caller-defined returned source path borrowed for as long as the returned borrow
binding remains visibly live. Loops and stateful ownership changes still use the
more conservative loop-state rules described above.

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

Runtime-dependent aggregate enum drops check the active tag before dropping
owned payload slots, so inactive variants are not cleaned as though they were
active. The backend artifact goldens
`backend-ownership-drop-aggregate.llvm-frag` and
`backend-ownership-drop-runtime-enum.llvm-frag` lock that lowering, while
`backend-ownership-compiler-shaped.llvm-frag` covers parser-state borrows,
generic aggregate fields, vector-stored work items, and result-like enum
matching without committing the full prelude-heavy LLVM output.

## Forget

```ari
forget token;
```

`forget` consumes a live or `maybe-unavailable` owning binding without lowering
any `Drop` calls. If the binding is live at runtime, the value is intentionally
leaked; if a loop exit already moved or dropped it on that runtime path, no
cleanup is attempted. This gives code an explicit way to resolve
`maybe-unavailable` owner states after maybe-zero loops when the right cleanup
choice is to abandon any remaining owner instead of conditionally running a
destructor.

Known `moved` or `dropped` bindings cannot be forgotten, non-owning bindings
cannot be forgotten, and active borrows still block `forget`. Temporary zones
created for automatic scope cleanup also cannot be forgotten; let their
automatic cleanup run or use an explicit non-temporary zone handle.

Local aggregates can carry `own`, `ref`, and `ref mut` fields. Owned aggregates
move as one value. Borrow-valued aggregate bindings track source borrows per
field path, so local aggregate reassignment and borrow-field reassignment can
release replaced sources while keeping unrelated borrow fields live. Fields and
elements can be borrowed directly with `ref value.field`, `ref mut value.0`, or
constant `ref value[index]`, and raw-pointer-backed aggregate slots can be
borrowed with forms such as `ref mut (*raw).0`. Those borrows block only
overlapping local paths when the compiler can track the base. Non-owned
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

`*T` is the preferred raw pointer spelling. `ptr T` remains accepted as the
older explicit keyword form. `T?` is a nullable spelling for the same raw
pointer type. `null` constructs a nullable raw pointer. `T?` is deliberately
not a value-level maybe type; use `Option<T>` when absence should be
represented as data instead of an address. Raw pointer casts use ordinary
explicit casts, including `*T` to `*U`, `T?` to another raw pointer type, `*T`
to an integer address, and an integer address back to `*T`. An explicit borrow
can also be converted to a raw pointer with `(&value) as *T`, `(&mut value) as
*T`, or the nullable spelling `(&mut value) as T?`.

`ptr_offset(pointer, bytes)` performs an explicit byte-wise address offset and
returns the same raw pointer type as `pointer`. `mem::ptr_offset` is the same
compiler-known operation. It does not dereference the pointer, check bounds, or
scale by `T`. `ptr_offset<T>(pointer, bytes)` is the same operation with an
explicit `ptr T` check on the pointer argument.

`ptr_add(pointer, count)` performs an explicit typed address offset. For a
`ptr T`, it moves by `count * sizeof(T)` bytes and returns the same `ptr T`
type. The executable subset supports scalar types and Ari-layout aggregate
types such as structs, tuple structs, tuples, and fixed arrays. `ptr_add` is
still unchecked and does not prove the resulting address is in-bounds.
`ptr_add<T>(pointer, count)` forces the pointer argument to be checked as
`ptr T` before lowering.

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
null, bounds, alignment, aliasing, or lifetime. On the LLVM backend,
`ptr f32` and `ptr f64` load/store values as raw IEEE bit patterns; `f128`
pointer access still waits for native float storage policy. Values that contain
`own`, `ref`, or `ref mut` state are rejected for whole raw-pointer copies for
now. `ptr_load<T>(pointer)` and `ptr_store<T>(pointer, value)` are accepted
when the call should validate the pointee type explicitly instead of only
inferring it from the pointer value.

`mem::replace<T>(ref mut target, value)` stores a new value in a mutable place
and returns the previous value. `mem::swap<T>(ref mut left, ref mut right)`
exchanges two mutable places. The root prelude also re-exports them as
`replace` and `swap`. These helpers are intentionally limited to the same
copyable scalar and plain Ari-layout aggregate values as `ptr_load` /
`ptr_store`; move-aware replacement for owning values should use the explicit
`move(value)` / `take(place)` surface until a broader library contract exists.

The same scalar operation can be written with dereference syntax:

```ari
var value: i64 = 4;
let raw: ptr i64 = (ref mut value) as ptr i64;
*raw = *raw + 8;
```

Dereference syntax supports scalar `ptr T` loads and stores, and whole
plain-aggregate copies for Ari-layout structs, tuple structs, tuples, and fixed
arrays. It is the same unchecked raw memory access as `ptr_load` and
`ptr_store`. When a raw pointer binding or pointer-valued field is the source,
`ref *pointer` and `ref mut *pointer` create a tracked borrow of the pointee
without copying the pointee value. This is the form used by source handles such
as `std::boxed::Box<T>.as_ref()` and `as_mut()`.

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

## Smart Pointer Policy

`Box[T]` is executable today as the root alias for `std::boxed::Box<T>`, the
source explicit-zone handle over `zone::new<T>` storage. Construct it with
`Box!(T, ref mut Zone, value)`, `Box::new<T>(ref mut Zone, value)`,
`std::Box::new<T>(ref mut Zone, value)`, or
`std::boxed::new<T>(ref mut Zone, value)`. Read-only handle methods such as
`get`, `copy_to`, `as_ref`, and `as_ptr` borrow the receiver. `as_ref`
borrows the stored value itself; `as_mut` and `as_mut_ptr` use a mutable
receiver borrow for mutable value or raw-pointer access. `set(value)` overwrites
the stored value and drops the previous value. `take()` mutably borrows the handle,
returns the stored value, and leaves the handle empty so a later
`drop boxed` consumes only the handle. `try_take()` returns `Option[T]` instead
of asserting on an empty handle. `clear()` mutably borrows the handle, drops the
stored value if one is present, and leaves the handle empty.
`put_in(ref mut Zone, value)` can refill that empty handle, but the zone
argument must match the handle's tracked source zone. Dropping a non-empty
handle runs the stored value's `Drop` impl when one exists, but the explicit
zone still owns and releases the backing bytes.

The remaining root smart-pointer policy is split between implemented explicit
handles and reserved policy aliases:

- `Unique[T]` is reserved for policy compatibility, but the preferred unique
  owner spelling is `Box[T]` once that spelling grows allocator-backed heap
  ownership.
- `Rc[T]` and `Arc[T]` are current root aliases for `std::rc::Rc[T]` and
  `std::rc::Arc[T]`.
- `Weak[T]` is the current root alias for `std::rc::Weak[T]`; it upgrades
  through `Option[Rc[T]]` or `Option[Arc[T]]`.
- `Shared[T]` remains reserved as a possible future policy alias once Ari has
  a final send/share story.

Future owning or shared handles must be created through explicit allocator or
capability arguments. Ari does not provide an ambient global heap. `drop` of a
unique or shared handle will be the operation that runs the handle's destructor
and releases or decrements its allocation; raw pointers exposed from these
handles will be non-owning views and must not transfer destruction rights.

The root `String` name follows the same capability-oriented path: it aliases
the source `std::string::String` explicit-zone handle. Lowercase `string`
remains today's borrowed C-string pointer value and does not own or free its
bytes.

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
zone::allocation_zone(data: ptr u8) -> ptr c_void
zone::metadata(data: ptr u8) -> std::zone::ZoneMetadata
zone::from_zone(zone: ref mut Zone) -> std::zone::ZoneMetadata
zone::of<T: std::zone::ZoneBacked>(value: ref T) -> std::zone::ZoneMetadata
value.zone() -> std::zone::ZoneMetadata
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

On the LLVM host backend, every non-empty `zone::alloc` result has a fixed
8-byte allocation header immediately before the returned user pointer. The
header stores only the raw zone handle at `ptr - 8`; size and alignment stay
call-site layout facts rather than pointer metadata. The user pointer remains
the real payload pointer, so normal loads, stores, and casts use it directly.
`zone::allocation_zone` exposes the raw handle without requiring source code to
do pointer-adjacent arithmetic. `zone::metadata(data)` wraps that raw handle in
`ZoneMetadata`, which is the preferred public shape. `zone::from_zone(ref mut
zone)` captures the same metadata from an explicit zone capability before any
payload allocation exists. `ZoneMetadata` exposes `as_ptr()`,
`as_zone_ptr()`, `alloc(bytes, align)`, and `alloc_array<T>(count)` for runtime
helpers that need to recover the backing zone from heap metadata. Empty source
String and Vec handles establish a small backing allocation even when logical
capacity is zero so `value.zone()` remains recoverable. Raw zero-count buffer
helpers may still return a null data pointer, so raw `metadata(data)` queries
require a non-null allocation pointer.
For stdlib heap handles, prefer `zone::of(ref value)` or `value.zone()` through
`std::zone::ZoneBacked`; they expose the same typed metadata for supported
handles, including map update-entry handles that recover through their backing
map. Raw header recovery through `metadata(data)` still
requires a non-null backing allocation.

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
When a return, labeled-block break, or value-carrying `init while` continue
needs a value computed inside that temporary-zone scope, the checker first
materializes the value into a hidden local, then emits the cleanup, then exits
the scope with the saved value.
Raw pointers allocated from a temporary zone cannot escape that lexical
lifetime. Returning such a pointer, storing it into an outer binding or
aggregate, or passing it through an escape-prone call is rejected with a
diagnostic that names both the pointer binding and the temporary zone source.

`zone::reset` only rewinds the allocation cursor; it does not destroy placed
values. Non-temporary `own Zone` values must be passed to `zone::destroy`;
plain `drop zone` is rejected so the release operation stays visible in source.
Other `own` locals still cannot silently cross `return`, `break`, or
`continue`; a live owner must be moved, returned, dropped, or forgotten before
that control-flow exit.
The checker tracks direct local pointers produced by `zone::alloc<T>` and
`zone::new<T>`, plus calls to pointer-returning functions or associated
constructors that take exactly one `ref Zone` or `ref mut Zone` parameter. Using
those bindings after the source zone has been reset or destroyed is rejected.
An `own dyn Trait` value constructed from a tracked `ptr T` keeps the same
source-zone provenance. Its `drop` path calls the erased concrete-value drop
thunk, but the zone still owns and releases the allocation bytes.
Source handles such as `std::boxed::Box<T>`, `std::string::RawString`,
`std::string::String`, `std::vec::RawVec<T>`, and `std::vec::Vec<T>` carry the
same tracked source-zone provenance when they are returned by a single-zone
constructor. Raw pointers recovered from a tracked `std::boxed::Box<T>`,
`std::string::String`, or `std::vec::Vec<T>` through `as_ptr()` keep that
provenance too; `std::vec::Vec<T>.as_mut_ptr()` preserves the same zone
through a mutable receiver. A `std::boxed::Box<T>`, `std::string::String`, or
`std::vec::Vec<T>` copied with `copy_to(ref mut Zone)`,
`std::vec::from_slice_in<T>(ref mut Zone, Slice<T>)`, or a
`std::string::String` copied with
`std::string::from_slice_in(ref mut Zone, Slice[u8])` or
`String.copy_to(ref mut Zone)` /
`std::string::copy_to(ref value, ref mut Zone)`, the result is tracked against
the target zone, not the original source zone. When a
source `std::string::String` grows through an explicit zone argument, that
argument must be the same source zone that created the handle. Source
`std::vec::Vec<T>` recovers `ZoneMetadata` from its backing allocation header,
so `push(value)`, `insert(index, value)`, `reserve(capacity)`,
`reserve_extra(additional)`, `extend_from_slice(values)`, and
`resize(length, value)` grow through that recovered runtime zone. A tracked local
`std::string::String` receiver can infer the same zone for its byte growth
methods and for
`append_string`/`append_i64`/`append_u64`/`append_bool`/`append_f32`/`append_f64`.
Callers therefore do not have to thread the zone through every ordinary
capacity-growing source Vec or String operation. Read-only
`std::string::String` and `std::vec::Vec<T>` handle methods
borrow the receiver, so metadata, checked endpoint/indexed reads, source
string/Vec search, source string exact/prefix/suffix checks, target-zone copy,
read-only Vec element borrow views, and read-only raw-pointer recovery do not
copy the handle itself. Mutable Vec element and raw-pointer recovery borrow the
source handle mutably.
Dropping a tracked source `std::vec::Vec<T>`
consumes the handle and drops each current element, while overwrite and shrink
helpers drop removed elements before reducing the live length. The explicit
zone still owns the backing bytes. When a
control-flow expression selects tracked handles from the same source zone, the
selected handle keeps that provenance. Different-source selections are not
modeled as a single-source handle; keep those values local
until APIs with explicit multi-source lifetime contracts exist.
That single-zone wrapper rule is a signature-level contract: a pointer-returning
function with no Zone borrow parameters or with more than one Zone borrow
parameter cannot return a tracked zone pointer.
Reset invalidation is merged conservatively through `if`, `match`, `??`
fallbacks, labeled-block exits, and loops, so a pointer is considered invalid
after control flow if any continuing path may have reset its zone. Calling
`zone::reset` through a named `ref mut Zone` binding invalidates the same source
zone as calling it through `ref mut zone` directly.
This is a diagnostic aid, not a full lifetime proof. Zone pointers cannot be
stored into tuple, struct, enum, or vector values, assigned into aggregate or
raw-pointer storage, passed through extern C or function-pointer calls, or
returned from functions unless the function has exactly one zone borrow
parameter and returns a pointer derived from that same parameter.
The standard source allocation handles above are explicit exceptions whose
single data field is known to the checker.

Zone-backed constructor-style APIs are ordinary associated functions. A type can
define `T::new(ref mut Zone, ...) -> ptr T` and delegate to `zone::new<T>`:

```ari
impl Point {
  fn new(zone: ref mut Zone, x: i64, y: i64) -> ptr Point {
    zone::new<Point>(zone, Point { x: x, y: y })
  }
}
```

Zone allocation lowers on the LLVM host backend with compiler-emitted
`malloc`/`free` helpers and an 8-byte zone-handle header before each non-empty
allocation payload.
