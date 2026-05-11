# Control Flow

## Return

```ari
fn main() -> i64 {
  return 0;
}
```

`return` exits the current function with a value.

## If

```ari
if value < 10 {
  return 1;
} else if value < 20 {
  return 2;
} else {
  return 3;
}
```

Conditions must be `bool` or an integer value. Integer conditions are treated
as `value != 0`; ownership-qualified values are not implicitly converted.
For statement `if`, literal `true` and `false` conditions are checked as normal,
but flow analysis follows the selected branch. This lets ownership and
non-returning path checks ignore statement paths that a literal condition cannot
reach.

`if` can also be used as an expression when both arms produce a value:

```ari
let score = if passed { 10 } else { 0 };
let adjusted = if score > 0 { score } else if retry { 1 } else { 0 };
```

Expression arms are scoped blocks that may contain local statements before a
final value expression. Both branches must produce compatible types, and the
checker merges ownership state across the branches just like statement `if`.

The final value can come after several local statements:

```ari
let score = if passed {
  let base = 8;
  var bonus = 1;
  bonus += 1;
  base + bonus
} else {
  0
};
```

Ownership state is merged across branches. If one branch moves or drops an
owner and the other branch leaves it live, the checker rejects the program until
both paths make the same ownership state explicit.

Enum payload and aggregate tests can use `if let`:

```ari
if let Some(value) = maybe_value {
  return value as i64;
} else {
  return 0;
}

if let picked @ (Left(value) | Right(value)) = choice {
  return weight(picked) + (value as i64);
}

if let (x, true) = pair {
  return x;
} else {
  return 0;
}

if let Point { x: px, flag: true, .. } = point {
  return px;
}
```

Enum `if let` supports same-name/same-type or-pattern alternatives and
alias-wrapped alternatives. `if let` can also produce a value when it has an
`else` arm. The pattern arm can bind a payload or aggregate fields, and each
arm follows the same final-expression rule as plain `if` expressions:

```ari
let score = if let Some(value) = maybe_value {
  let bonus = 2;
  (value as i64) + bonus
} else {
  0
};

let side_score = if let (Left(value) | Right(value)) = choice {
  value as i64
} else {
  -1
};

let aggregate_score = if let (Point { y: py, .. }, Rgb(red, false)) = (point, color) {
  py + red
} else {
  0
};
```

Aggregate `if let` supports the same tuple, fixed array, named struct, and
tuple-struct patterns as `match`. Or-pattern alternatives may bind the same
names with the same types, so `(left, 0) | (0, left)` binds `left` from the
alternative that actually matched. Irrefutable aggregate patterns such as
`(x, y)` do not need `if let`; Ari rejects them when an `else` arm is present.

## Block Expressions

Plain `{ ... }` blocks can produce values in expression positions. The block
opens a local scope, may contain local statements, and must end with a final
value expression:

```ari
let value = {
  identity(1);
  let base = 2;
  var total = base + 3;
  total += 4;
  total
};
```

Inside value-producing blocks and `if` expression arms, a non-final expression
statement must end with `;`. The final value expression omits `;`; adding one
makes it a discarded statement, so the block no longer has a result value.
Ordinary statement blocks use the same `;` rule for expression statements.

The result type is the type of the final expression. Borrow-valued block
results are not supported yet.

The freestanding backend materializes aggregate-valued `if`, `match`, and
block expression results directly into their target storage, or into hidden
temporaries when the expression result is discarded.

## While

```ari
var value = 0;
while value < 10 {
  value = value + 1;
}
```

`while let` repeats while an enum-case or aggregate pattern matches:

```ari
while let Some(value) = next(index) {
  index = index + 1;
}

while let chosen @ (Add(value) | Sub(value)) = next(index) {
  total = total + weight(chosen) + (value as i64);
  index = index + 1;
}

while let ((left, 0) | (0, left)) = tuple_at(index) {
  total = total + left;
  index = index + 1;
}
```

Enum `while let` supports same-name/same-type or-pattern alternatives and
alias-wrapped alternatives. The loop body is checked once with the shared
bindings, then each matching alternative fills those bindings before entering
the body. Aggregate `while let` supports tuple, fixed array, named struct, and
tuple-struct or-pattern alternatives. It re-evaluates the aggregate expression
each iteration, executes the first matching alternative, and exits the loop
when no alternative matches.

Loops currently cannot change the ownership state of an outer binding. That
rule is conservative until the checker can track loop invariants.

## For

The first working `for` loop is a range iterator:

```ari
var total = 0;
for value in range(0, 5) {
  if value == 3 {
    continue;
  }
  total = total + value;
}
```

The `0..5` spelling is equivalent:

```ari
for value in 0..5 {
  total = total + value;
}
```

This iterates `0, 1, 2, 3, 4`. The loop binding is immutable inside the body.
Inclusive `..=` ranges include the end value:

```ari
for value in 0..=5 {
  total = total + value;
}
```

The intended long-term model is iterator `next` plus enum pattern matching. The
current executable subset recognizes `range(start, end)`, `iter::range(start,
end)`, `range_inclusive(start, end)`, `iter::range_inclusive(start, end)`,
`start..end`, and `start..=end` as concrete iterator forms and lowers them like
checked while loops.

Range expressions also produce local range values. `Range[T]` and
`RangeInclusive[T]` expose `start`/`end` fields and tuple indices `.0`/`.1`
for integer bound types, so a range can be named before it is iterated:

```ari
let span: Range[i64] = 2..5;
for value in span {
  total = total + value;
}

let bytes: Range[u8] = range(1, 4);
for byte in bytes {
  total = total + byte as i64;
}
```

Non-empty list literals and stored local vectors can also be iterated directly:

```ari
var total = 0;
for value in [1, 2, 3] {
  total = total + value;
}

let values: Vec[i64] = [4, 5, 6];
for value in values {
  total = total + value;
}
```

Stored vector loops use the vector's current runtime length, not its reserved
local capacity; when the compiler knows that length, the loop bound is lowered
as a constant. Vec-valued `if`/`match`/block results whose branches copy local
vectors with the same compiler-known current length use that known length as
the stored-vector loop bound too.

`for _ in [1, 2, 3]` runs once per element while ignoring the element value.
Range, list-literal, and stored-vector loops also support alias patterns when
the wrapped pattern is irrefutable, so the loop body can use both the whole
item and the inner binding:

```ari
for item @ value in range(1, 4) {
  total = total + item + value;
}
```

Bare empty `[]` cannot be iterated directly because it has no element type by
itself. Give the value a typed local binding first, such as
`let values: Vec[i64] = []`, when an empty vector should participate in normal
stored-vector control flow. Stored local vector loops lower on the LLVM and raw
freestanding backends.

List-literal and stored-vector loops can additionally destructure irrefutable
aggregate element patterns:

```ari
for (x, (y, z)) in [(1, (2, 3)), (4, (5, 6))] {
  total = total + x + y + z;
}

for Point { x: px, y, .. } in [p1, p2] {
  total = total + px + y;
}

for Rgb(red, _) in [Rgb(7, true), Rgb(8, false)] {
  total = total + red;
}

for whole @ (left, right) in [(1, 2), (3, 4)] {
  total = total + whole.0 + left + right;
}
```

These loop heads are destructuring bindings, not filters. Literal, range,
or-pattern, and enum-case loop heads are still rejected for range/list/vector
loops because those loops bind every element instead of calling
`Iterator[T].next()`.

Direct copyable non-borrow `Iterator[T]` values lower through the standard
`next(self: ref mut Self) -> Option[T]` step operation. Owning iterator values
whose `next` receiver is `ref mut Self` are consumed into the hidden iterator
storage, advanced there, and dropped at loop exit, so the original binding is
moved by `for item in cursor`. Explicit `ref mut` iterator values are also
accepted, so `for item in ref mut cursor` advances the original `cursor` and
releases the hidden borrow after the loop. The iterator expression is evaluated
once into a hidden mutable iterator binding, then Ari repeats `iterator.next()`
while it returns `std::Some(item)`. Existing value-self iterator impls remain
accepted for copyable snapshot-style iterators, but stateful iterators should
prefer `self: ref mut Self`.
If a `return` exits the function from inside an owning iterator loop, Ari
drops the hidden iterator owner before returning. A `break` from the iterator
loop itself uses the normal loop-exit cleanup; a break to an outer label also
drops any hidden owning iterators for nested loops that it skips. Postfix `?`
residual returns use the same cleanup path before returning from the function.
Copyable non-borrow `IntoIterator[T]` values also lower when
`into_iter(self: ref mut Self)` returns either `Self` or another copyable
non-borrow value that implements `Iterator[T]`. Generic impls may return a
distinct generic iterator type such as `BagIter[T]`. Existing value-self
`into_iter(self)` impls remain accepted for copyable snapshot-style containers.

Iterator item patterns may be more specific than a plain binding:

```ari
for Ready in cursor {
  total = total + 1;
}

for 11 in numbers {
  total = total + 1;
}

for 11 | 12 in numbers {
  total = total + 1;
}

for 10..=20 in scores {
  total = total + 1;
}

for Just(value) in maybe_values {
  total = total + (value as i64);
}

for Just(5 | 6) in maybe_values {
  total = total + 1;
}

for Pair(left @ 4, true) in pair_values {
  total = total + left;
}

for let Pair(value @ (4 | 12), ok @ (true | false)) in pair_values {
  if ok {
    total = total + value;
  }
}
```

These patterns use `while let Some(pattern) = iterator.next()` semantics. If an
item does not match, the loop ends; it is not skipped. This keeps the lowering
predictable for plain `for` loops.

Use `for let pattern in iterator` when non-matching iterator items should be
skipped instead:

```ari
for let 10..=20 in scores {
  total = total + 1;
}
```

`for let` filters currently require an `Iterator[T]` or `IntoIterator[T]`
value. Range, list-literal, and stored-vector loops still use irrefutable
binding/destructuring loop heads. Or-pattern alternatives must bind the same
names. Enum-case item patterns support fieldless cases, compact enum payload
cases, and nested aggregate-enum item cases whose `Option[T]` payload slot
stores one nested aggregate enum type. Nested aggregate enum cases may bind or
test positional multi-payload slots with literal/range/or/alias subpatterns.

The current source trait still uses an Ari-specific return contract for
`into_iter` instead of a first-class associated iterator type. Impl validation
requires the concrete result to implement `Iterator[T]`, including generic
impl headers such as `impl[T] IntoIterator[T] for Bag[T]`, and `for` lowering
rechecks the specialized result. Mutable `Iterator.next` and
`IntoIterator.into_iter` receivers now work for copyable non-borrow values; the
broader policy for owner/borrow iterator values and iterator lifetime rules
remains planned.

## Break

```ari
while true {
  break;
}
```

`break` exits the nearest loop.

When a `break` exits a loop, Ari checks that any owning bindings visible after
the loop have the same state as they would have if the loop ran zero times. This
keeps paths such as `drop owner; break;` from leaving the owner ambiguously live
after the loop.

For a literal `while true` loop, there is no zero-iteration exit. In that case,
plain `break` paths define the ownership state after the loop. If every body
path returns, the `while true` statement itself is treated as returning. If the
body has no reachable `break` and can only continue into the next iteration,
Ari treats the loop as non-fallthrough, so a later function return is not
required for that path.

Because `break` leaves the current nested scopes, any owning binding declared in
those scopes must be moved or dropped before the jump.

Loops can be labeled, and `break label` exits the matching active loop:

```ari
outer: while true {
  while true {
    break outer;
  }
}
```

Blocks can also be labeled as statement scopes. A labeled block can be exited
early with `break label`:

```ari
block: {
  cleanup_step();
  break block;
  unreachable_step();
}
```

In expression position, a labeled block can produce a value. `break label value`
stores the block result and exits early:

```ari
let result = done: {
  let base = 4;
  if base > 3 {
    break done base + 8;
  }
  1
};
```

The break value and the final block value must have compatible types. Borrow
break values are supported when every result path borrows the same source path
with the same borrow mode. Owning break values can move an `own` value out of
the labeled block, and the moved value becomes the block result. Tuple and
struct results can be bound from labeled blocks:

```ari
struct Point {
  x: i64,
  y: i64,
}

let point = done: {
  break done Point { x: 4, y: 5 };
  Point { x: 0, y: 0 }
};
```

If a labeled block expression has both a final-value path and one or more
`break label value` paths, every path must leave outer owning bindings in the
same state. For example, one path cannot drop an `own` binding while another
typed-break path leaves it alive. Moving an outer `own` binding through a
typed break is valid only when the reachable block-result state records that
same move.

When labeled block results are local `Vec[T]` values on the LLVM backend, typed
`break label value` paths participate in the same fixed local storage sizing as
the final block value. If every Vec result path has the same compiler-known
length, later `len`, `is_empty`, static indexing, `as_slice`, and stored-vector
`for` lowering can reuse that known length.

## Continue

```ari
while value < 10 {
  value = value + 1;
  continue;
}
```

`continue` jumps to the next loop iteration.

Because `continue` leaves the current iteration scopes, any owning binding
declared in those scopes must be moved or dropped before the jump. Ari also
checks that owning bindings visible at the loop boundary have the same state on
`continue` paths as they had at loop entry, because the next condition check may
exit the loop.

`continue` with values is only valid in `init while` loops. Plain `while` and
`for` loops reject value continues because they do not have positional loop
state to update.

## Init-While Sugar

```ari
init a = 1, b = 5 while a < b {
  if a == 3 {
    continue a + 1, b
  }
} next a + 1, b
```

The `init` bindings are created before the loop. The expression after `while`
is the loop condition. The `next` values are assigned positionally after a
normal iteration.

For:

```ari
init a = 1, b = 5 while a < b {
  ...
} next 3, 4
```

the normal end-of-iteration update behaves like:

```ari
a = 3
b = 4
```

Inside the loop:

```ari
continue 2, 5
```

behaves like:

```ari
a = 2
b = 5
continue
```

The number of values in `continue` and `next` must match the number of `init`
bindings.

Owning `init` bindings are allowed when every executed `next` or value
`continue` reinitializes that binding only after the old owner has been moved
or dropped. For example, this loop drops the current owner before replacing it:

```ari
init token = make_token(), i = 0 while i < 3 {
  drop token;
} next make_token(), i + 1
```

The update is still positional and parallel: all update expressions are
evaluated before the loop bindings are written. In loops with owning `init`
bindings, `continue` must provide explicit update values so the checker can
validate the owner state at that jump.

`let ... while ... next ...` is accepted as the preferred spelling for the same
loop-state form:

```ari
let a = 1, b = 5 while a < b {
  continue a + 1, b
} next a + 1, b
```

The older `init ... while ... next ...` form remains accepted for now.

## Limits And Diagnostics

- `if`, `while`, `while let`, `init while`, and `let while` conditions must be
  `bool` or integer-convertible.
- Plain `break` and `continue` must appear inside a loop.
- `break label` must name an active loop or block label.
- `for` currently accepts integer `Range[T]` and `RangeInclusive[T]` values,
  `range(start, end)`, `iter::range(start, end)`,
  `range_inclusive(start, end)`, `iter::range_inclusive(start, end)`,
  `start..end`, `start..=end`, non-empty list literals, or stored local vector
  values.
- Range `for` patterns currently support a binding name or `_`. Vector loops
  also support irrefutable alias, tuple, array, named struct, and tuple-struct
  product patterns. Enum-case and other refutable loop-head patterns are
  reserved for future iterator lowering.
- Loops currently cannot change the ownership state of an outer binding.
