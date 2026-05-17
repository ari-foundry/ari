# Functions

## Basic Form

```ari
fn add(left: i64, right: i64) -> i64 {
  left + right
}
```

A function has:

- a name
- zero or more named parameters
- one return type after `->`
- a block body

## Main

Executable programs need:

```ari
fn main() -> i64 {
  0
}
```

`main` must return `i64` in the current compiler. The generated ELF exits with
that value.

## Parameters

Parameters are always typed:

```ari
fn clamp(value: i64, low: i64, high: i64) -> i64 {
  if value < low {
    return low;
  }
  if value > high {
    return high;
  }
  value
}
```

Functions and calls are limited to 65,535 parameters/arguments at the language
checking layer. The LLVM/glibc backend lowers calls as LLVM calls. Narrow
integer returns are normalized at the return boundary, so `u8`, `u16`, and
`u32` results wrap to their declared width before callers observe them. Direct
and function-pointer calls can pass `f32`/`f64` values, and the currently
supported tuple, struct, fixed-array, and aggregate enum values lower through
the LLVM ABI paths.
Aggregate parameters are copied into callee-local storage at function entry, so
mutating a parameter copy does not mutate the caller's value.
Aggregate enum call results can also be used directly as LLVM `match`
inputs; the LLVM backend materializes the result into hidden stack storage before
reading tag and payload slots.

## Return

Every lowered function must return a value matching its declared return type:

```ari
fn ok() -> bool {
  return true;
}
```

Non-void functions must return on every statically visible path. A missing
return is rejected instead of silently falling through.

The final expression in a function body is treated as an implicit return when
it omits `;`:

```ari
fn answer() -> i64 {
  let base = 40;
  base + 2
}
```

Non-final expression statements still need `;`. Adding `;` to the final value
turns it into a discarded expression statement, so non-void functions still
need another return path.

Functions with no `->` return type are treated as `void`:

```ari
fn log_done() {
  println("done");
  return;
}
```

Returning a value from a void function is rejected.

Use `-> ()` when a function should return Ari's explicit unit value instead of
host-style `void`:

```ari
fn mark_done() -> () {
  ()
}
```

The final `()` is a normal implicit return value. A bare `return;` remains only
for `void` functions.

Returning an owned value moves it out of the function. A return path cannot
leave another live owner behind.

## Recursion

Direct recursion is supported in the current executable subset:

```ari
fn fact(value: i64) -> i64 {
  if value <= 1 {
    return 1;
  }
  value * fact(value - 1)
}
```

## Calls

```ari
fn twice(value: i64) -> i64 {
  value + value
}

fn main() -> i64 {
  twice(21)
}
```

Calls are type-checked against the callee signature. Ari does not implicitly cast
between integer widths, so callers must write `as` when they want a conversion.

## Overload Policy

Ari does not choose between functions by argument count or argument type. Two
concrete functions cannot share the same fully qualified source path, even when
their parameter lists differ.

Use distinct names, module paths, inherent methods, or traits when a concept has
multiple typed entry points. This keeps calls and ABI symbols path-based instead
of choosing overloads from argument types.

A generic function may share a path with a concrete function when calls are
syntactically disambiguated with type arguments, such as `alloc<T>(...)`. Calls
without type arguments choose the concrete function first.

## Function Pointers

Function pointer types use `fn(param_types...) -> result_type`:

```ari
fn add_four(value: i64) -> i64 {
  value + 4
}

fn apply(op: fn(i64) -> i64, value: i64) -> i64 {
  op(value)
}

fn main() -> i64 {
  let op: fn(i64) -> i64 = add_four;
  apply(op, 3)
}
```

Function names can be used as function pointer values. Local function pointer
bindings can be called with normal call syntax, and calls are checked against
the function pointer's parameter and result types.

Generic function names can also become function pointer values when an expected
function pointer type selects a concrete specialization:

```ari
fn identity[T](value: T) -> T {
  value
}

fn apply(op: fn(i64) -> i64, value: i64) -> i64 {
  op(value)
}

fn main() -> i64 {
  let op: fn(i64) -> i64 = identity;
  apply(identity, 3) + op(4)
}
```

The expected `fn(i64) -> i64` type specializes `identity[T]` as
`identity[i64]`. A generic parameter that does not appear in the expected
function pointer signature cannot be inferred.

## Generics

Generic syntax is parsed:

```ari
fn identity[T](value: T) -> T {
  value
}
```

Generic functions are monomorphized when called with concrete executable types:

```ari
fn identity[T](value: T) -> T {
  value
}

fn main() -> i64 {
  identity(42)
}
```

Generic declarations keep the declaration-side `[T]` list. Call sites can spell
their type arguments explicitly with `<T>` so `[]` stays available for indexing:

```ari
fn make[T]() -> T {
  return 7
}

fn main() -> i64 {
  make<i64>()
}
```

Explicit type arguments are checked against the same parameter constraints as
inferred calls, so `identity<i64>(true)` is rejected. Generic type parameters
that cannot be inferred and are not written explicitly are rejected during
lowering, and a single type parameter must resolve to one concrete type for the
whole call.

This generic syntax is for Ari declarations. `extern "C"` declarations remain
concrete C symbols and cannot have `[T]` parameters; expose one concrete C
wrapper per foreign specialization and call those wrappers from Ari code.
Reserved `extern "ari"` builtin declarations are also concrete and non-generic.

## Parameter Patterns

Function parameters can be named bindings:

```ari
fn add(left: i64, right: i64) -> i64 {
  left + right
}
```

They can also destructure value parameters with the same pattern surface used
by `let`/`var` bindings:

```ari
struct Point {
  x: i64,
  y: i64,
}

enum OptionI64 {
  Some(i64),
  None,
}

fn sum_pair((left, right): (i64, i64)) -> i64 {
  left + right
}

fn sum_point(Point { x, y }: Point) -> i64 {
  x + y
}

fn sum_array([first, .., last]: [i64, 4]) -> i64 {
  first + last
}

fn sum_slice([first, rest @ ..]: Slice[i64]) -> i64 {
  first + len(rest)
}

fn sum_alias(point @ Point { x, y }: Point) -> i64 {
  point.x + x + y
}

fn take_some(Some(value): OptionI64) -> i64 {
  value
}

fn ignore_first(_: i64, value: i64) -> i64 {
  value
}
```

Tuple, fixed-array, `Slice[T]` runtime-sequence, struct, tuple-struct,
wildcard, alias, and enum-case parameter patterns lower at function entry.
`Slice[T]` runtime-sequence parameters support `name @ ..` rest bindings, which
bind the skipped range as another `Slice[T]` view. Refutable enum-case and
`Slice[T]` sequence parameters panic if the caller passes a non-matching case
or length. Owning and borrow-valued parameter patterns are still rejected until
ownership behavior for parameter destructuring is defined. Root `Vec[T]`
function parameters are allowed in
ordinary direct calls and in function pointer parameter positions such as
`fn(Vec[T]) -> R`: the compiler lowers those parameter slots to a borrowed
`Slice[T]`-shaped ABI, so one function body works for local Vec values with
different caller capacities. Calls create the ABI view from named local Vec or
array bindings, and from temporary Vec literals or Vec-valued control-flow
expressions such as `sum([1, 2, 3])`. Generic functions whose source parameter is
`Vec[T]` use the same view ABI and specialize by element type; generic by-value
`T` parameters still carry concrete Vec capacity when `T` itself resolves to
local Vec storage. Trait and impl method parameters use the same view ABI for
ordinary parameter slots, while root `Vec[T]` function returns and trait method
return types still wait for the runtime-capacity Vec ABI.
Parameter patterns can also use reference binding modes:

```ari
fn sum_pair(ref (left, right): (i64, i64)) -> i64 {
  return read(left) + read(right);
}

fn sum_pair_short(&(left, right): (i64, i64)) -> i64 {
  return read(left) + read(right);
}

fn adjust(ref mut Point { x, y }: Point) -> i64 {
  bump(x);
  bump(y);
  return read_mut(x) + read_mut(y);
}

fn adjust_short(&mut Point { x, y }: Point) -> i64 {
  bump(x);
  bump(y);
  return read_mut(x) + read_mut(y);
}
```

The ABI parameter is still the declared value type. Ari creates hidden function
entry storage for the parameter, then introduces `ref` or `ref mut` bindings to
that storage before the body runs. For by-value parameters, `ref mut` mutates
the function's local parameter copy, not the caller's original value. The
current 0.x slice supports name, wildcard, tuple, fixed-array, and struct
reference parameter patterns over by-value parameter storage, matching local
`let ref` / `let ref mut` / `let &` / `let &mut` patterns. Owning or
borrow-valued parameter patterns, enum-payload reference bindings,
runtime-sequence reference rest bindings, nested reference binding modes inside
subpatterns, and standalone `mut` binding-mode patterns remain rejected. Trait
and extern function signatures must keep named parameters.

## Borrow Returns

Functions can return a borrow when the compiler can track one source
conservatively. Without an explicit contract, the function signature must have
exactly one borrow parameter, and every returned `ref` or `ref mut` value must
trace back to that parameter. Local-source escapes remain rejected.

```ari
fn identity(value: ref i64) -> ref i64 {
  return value;
}

struct Pair {
  left: i64,
  right: i64,
}

fn left(pair: ref Pair) -> ref i64 {
  return ref pair.left;
}

fn bad(value: ref i64) -> ref i64 {
  var local: i64 = 1;
  return ref local; // rejected
}
```

Use `@borrow_return(source)` when a borrow-returning function has more than one
borrow parameter, or when an extern declaration has no body for Ari to inspect.
The source path can include struct fields or constant tuple/array/vector
indexes:

```ari
@borrow_return(left)
fn pick_left(left: ref i64, right: ref i64) -> ref i64 {
  return ref left;
}

@borrow_return(pair.left)
extern "C" fn pair_left(pair: ref Pair) -> ref i64;
```

For Ari functions with bodies, the compiler checks the contract against every
return path.

When a caller binds the result, the original argument source remains borrowed
until that result binding's last visible straight-line use, or until the
binding leaves scope when the checker cannot shorten it. If every return path
borrows the same field or constant element below that parameter, only that
subpath stays borrowed at the call site, so a returned `ref pair.left` does not
block an unrelated borrow of `pair.right`. The same rule applies to method calls whose
`self` parameter is the single borrow source. Function pointer calls and
borrow-valued aggregate returns are still rejected until Ari has explicit
source/lifetime contracts for those shapes.
