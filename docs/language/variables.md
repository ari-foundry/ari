# Variables

## Immutable Bindings

```ari
let answer = 42
let explicit: i64 = 42
```

`let` creates an immutable binding. The type is inferred from the initializer
when no annotation is present.

## Mutable Bindings

```ari
var counter = 0
counter = counter + 1
```

`var` creates a mutable binding. Assignment is allowed only for mutable
bindings.

## Type Inference

```ari
let flag = true
let count = 10
```

Local inference is initializer-based. Integer literals default to `i64` when
there is no expected type from a variable annotation, parameter, or return.

## Constants

Top-level constants are declared with an explicit type:

```ari
struct Point {
  x: i64,
  y: i64,
}

enum MaybeI64 {
  Some(i64),
  None,
}

const LIMIT: i64 = 10;
const ENABLED: bool = true;
const ORIGIN: Point = Point { x: 0, y: 0 };
const PAIR: (i64, bool) = (3, true);
const VALUES: [i64, 3] = [1, 2, 3];
const STATE: MaybeI64 = Some(5);
```

Constants are module items, so `pub const`, `use`, `A::B`, `self::`, and
`super::` follow the same visibility and path rules as other module items.
Current constant initializers support integer literals, bool literals, other
constants, integer arithmetic (`+`, `-`, `*`, `/`, `%`), bit operations
(`&`, `|`, `^`, `~`, `<<`, `>>`), unary `-`, bool `!`, `&&`, `||`, scalar
comparisons, explicit integer casts with `as`, tuple literals, fixed-array
literals, named struct literals, tuple-struct constructors, and enum case
constructors. Constant initializers can also read fields, tuple indexes, and
fixed-array indexes from materialized aggregate constants.
Integer literal suffixes such as `7u8` are checked before the value is assigned
to the constant's declared type; unsuffixed integer literals are checked
directly against that declared type.
Generic enum constants are specialized from the declared constant type, so
`const VALUE: Option[i32] = Some(7i32);` and
`const EMPTY: Option[i32] = None;` are accepted.
Aggregate constants cannot contain ownership or borrow-qualified fields yet.
Constants are lowered inline when used in expressions; scalar integer and bool
constants can also be used as match patterns.
If constants form a dependency cycle, the diagnostic reports the full cycle
path, such as `A -> B -> C -> A`.

## No Local Shadowing

The executable subset rejects local shadowing:

```ari
let value = 1
let value = 2
```

This keeps every lowered local mapped to one stable stack slot.

`let` and `var` can also destructure tuple, fixed array, and struct values. `_`
ignores a value or field, and `var` makes every introduced binding mutable:

```ari
let (left, right) = (4, 5)
var (total, flag): (i64, bool) = (left + right, true)
total += 1

let (_, kept) = (99, total)
let ((nested_left, _), nested_right) = ((2, false), 3)
let (first, .., last) = (1, 40, 50, 2)
let [head, .., tail]: [i64, 4] = [1, 20, 30, 2]
```

Struct binding patterns use the struct type name and can either bind shorthand
fields or rename a field with `field: pattern`. `..` ignores any remaining
tuple elements or struct fields:

```ari
struct Point {
  x: i64,
  y: i64,
}

let Point { x, y: renamed } = point
var Point { x: total, y: other, .. } = Point { x: 4, y: 5 }
total += other
```

Tuple and fixed array pattern arity must match the value exactly unless the
pattern uses one `..` rest. Struct pattern fields must exist on the struct.
Aggregate declaration patterns can also be refutable when they contain literal,
range, alias, or or-pattern tests. A mismatch takes the normal panic path:

```ari
let ((left, 0) | (0, left)) = pair
var ([head, 0] | [0, head]): [i64, 2] = values
head += left

let (Point { x: axis, y: 0 } | Point { x: 0, y: axis }) = point
```

Declaration patterns bind by value. `let mut pattern = value` is accepted as
declaration-level mutability for every binding introduced by the pattern, so it
matches `var pattern = value` while keeping the familiar `let mut` spelling.
`let ref pattern = value` and `let ref mut pattern = value` bind references
instead of copying the selected values. `let &pattern = value` and
`let &mut pattern = value` are equivalent shorthand forms. The initializer must
be a tracked local place, such as a local name, field access, tuple index, or
constant array/vector index. Tuple, fixed-array, and struct destructuring borrow
each introduced binding's source path independently:

```ari
let ref shared = value
let &alias = value
let ref mut unique = cell
let &mut alias_unique = cell
let ref (left, right) = pair
let &(copy_left, copy_right) = pair
let ref [head, tail]: [i64, 2] = values
let ref [head, rest @ ..] = vec_values
let ref mut [first, middle @ .., _] = vec_values
let ref Point { x, y: renamed } = point
let ref Full(payload) = maybe_wide
```

`ref mut` requires a mutable source binding and mutable struct field when the
selected path ends at a field. The introduced bindings themselves are ordinary
immutable borrow bindings, matching `let unique = ref mut cell`. Reference
patterns over direct local `Vec[T]` storage and `Slice[T]` view bindings use
the same runtime length guard as value sequence patterns. Prefix elements
before `..` and plain named suffix elements after `..` can be borrowed by
reference, and `name @ ..` binds the skipped range as a `Slice[T]` view.
Runtime-sequence element borrows keep distinct element paths, including nested
tuple, fixed-array, and struct subpatterns such as
`let ref mut [(left, right), .., (tail_left, tail_right)] = view`. Destructuring
of ownership-carrying aggregates remains planned. Nested shared reference
binding modes are supported in enum `match`, enum `if let`, and enum
`while let` patterns, including same-name/same-type enum `while let`
or-pattern alternatives. Mutable enum payload reference bindings are supported
in enum statement/expression `match`, enum `if let`, and enum `while let` when
the matched subject is an addressable local, field, or indexed element. Tuple,
fixed-array, and struct control-flow patterns also support mutable field
reference bindings in statement/expression `match`, aggregate `if let`, and
aggregate `while let` when the matched subject is addressable. Runtime-sequence
`Slice[T]`/`Vec[T]` control-flow patterns support mutable element reference
bindings in statement/expression `match`, `if let`, and `while let` when the
matched subject is addressable. Ownership-carrying aggregate destructuring
through these binding modes remains planned.
Function parameter patterns support
`ref PATTERN: T`, `ref mut PATTERN: T`, `&PATTERN: T`, and `&mut PATTERN: T`
for the same name, wildcard, tuple, fixed-array, struct, and `Slice[T]`
runtime-sequence shapes. Enum-case reference patterns work when the
matched enum stores the payload in an addressable aggregate slot, such as an
`i64`/`u64` payload-word slot or a nested aggregate-enum payload slot. Compact
small payloads and narrow non-addressable aggregate payload words remain
value-only, and the checker reports the payload type when such a payload is
used in a reference pattern.

The `[a, b]` pattern spelling works for fixed arrays and for runtime sequence
subjects such as local `Vec[T]` storage and `Slice[T]` views. On `Vec[T]` and
`Slice[T]`, Ari lowers the pattern to a length check plus indexed element
bindings. A pattern without `..` requires `len(value) == element_count`; a
pattern with `..` requires `len(value) >= non_rest_element_count`. A mismatch in
`let` or `var` destructuring takes the normal panic path.

Single-payload enum cases can also be used in local bindings:

```ari
let Some(value) = item
var Some(total) = next_item
let (Left(value) | Right(value)) = choice
let picked @ (Left(1) | Right(2)) = choice
```

These enum bindings are refutable. If the value is not the requested case, Ari
takes the normal panic path. Use `if let` or `match` when the mismatch needs an
explicit else arm. The payload binding follows the declaration mutability:
`let Some(x)` makes `x` immutable, while `var Some(x)` makes `x` mutable.
Or-pattern alternatives must bind the same names with the same types, just like
match arms.

## Assignment

```ari
var value: i32 = 4
value = value + 1
```

The assigned expression must match the variable type exactly. Use an explicit
cast when converting between integer widths:

```ari
var wide: i64 = 10
wide = (wide + 1) as i64
```

## Drop

```ari
let temporary = 1;
drop temporary;
```

`drop` ends a binding explicitly. If the value type implements the prelude
`Drop` trait, `drop` lowers to `fn drop(self) -> void`. Otherwise, dropping the
currently supported primitive owners is a codegen no-op while still ending the
binding for ownership checking. Aggregate bindings that contain owned fields
can also be dropped as a whole; the compiler lowers `Drop::drop` for owned
tuple, fixed-array, vector, and struct fields that provide a matching impl.

After a binding is dropped, later reads are rejected.

## Owning Bindings

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

`own T` values are move-only in the current checker. After `token` is passed to
`consume`, it cannot be read, assigned, borrowed, or dropped again.

Reassigning a live owning binding is rejected. Move or drop the old owner first.

Tuples, fixed arrays, vectors, and structs can store ownership-qualified fields.
An aggregate containing any `own` field is move-only as a whole:

```ari
let pair: (own i64, i64) = (make_owned(1), 2);
consume_pair(pair);
```

Borrow-valued aggregate bindings keep each borrow field's source borrowed while
that field is live. Reassigning the whole local aggregate or a borrow-valued
field releases the old field source after the new value is checked, so a
replaced source can be assigned again while the aggregate continues to live.
Named borrow bindings are also shortened in straight-line statement scopes:
after the borrow's last visible use, its source can be assigned or borrowed
again before the block ends. Reborrow chains keep the intermediate borrow
binding live until the dependent borrow is also dead.
Local aggregate fields and elements can also be borrowed directly with
`ref aggregate.field`, `ref mut aggregate.0`, or `ref aggregate[index]` when
the index is a constant fixed-array or local vector index. The checker tracks
those paths independently, so unrelated fields or elements can still be read or
assigned while the borrow is live.

For structs and tuple structs, non-owned fields can still be read or assigned
while the owning aggregate remains live; moving or overwriting the owned field
itself is also tracked. Nested field paths and constant fixed-array or local
vector indexes use the same tracking:

```ari
var holder = Holder { token: make_owned(1), count: 0 };
let token = holder.token;
holder.count = 3;
holder.token = make_owned(2);
drop token;
drop holder;

var values: [own i64, 2] = [make_owned(1), make_owned(2)];
let first = values[0];
values[0] = make_owned(3);
drop first;
drop values;
```

After an owned field is moved, reading it again is rejected until a mutable
field or indexed-element assignment reinitializes it. Moving the whole
aggregate after one of its owned fields has moved is rejected. Moving an owning
array or vector element through a dynamic index is rejected because the checker
cannot name a single tracked element path statically; use a constant index or
move the whole aggregate. Owned fields and elements can be moved only from
named local aggregates; moving them directly out of temporary aggregate
expressions is rejected, so bind the aggregate first.
