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
checking layer. The LLVM/glibc backend lowers calls as LLVM calls. The
freestanding backend uses registers for the first six scalar arguments and stack
slots for the rest. Narrow integer returns from freestanding functions are
normalized at the return boundary, so `u8`, `u16`, and `u32` results wrap to
their declared width before callers observe them. Freestanding direct calls can
pass and return `f32`/`f64` values as raw IEEE bit patterns in the same scalar
ABI slots. Freestanding direct and function-pointer calls can pass and return
tuple, struct, fixed-array, and currently supported aggregate enum values
through hidden pointer slots.
Aggregate parameters are copied into callee-local storage at function entry, so
mutating a parameter copy does not mutate the caller's value.

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

Tuple, fixed-array, struct, tuple-struct, wildcard, alias, and enum-case
parameter patterns lower at function entry. Refutable enum-case parameters
panic if the caller passes a non-matching case. Owning and borrow-valued
parameter patterns are still rejected until ownership behavior for parameter
destructuring is defined. Trait and extern function signatures must keep named
parameters.
