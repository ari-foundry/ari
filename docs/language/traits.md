# Traits

## Trait Declarations

```ari
trait Comparable[T] {
  fn eq(self, other: T) -> bool
}
```

`trait` is Ari's single behavior-declaration model.

Inside trait and impl method signatures, a bare `self` parameter is treated as
`Self`, the type named by the `impl ... for SelfType` target.

`interface` is intentionally not a keyword, and Ari has no `class` declaration.

## Impl Surface

```ari
impl Comparable[i64] for i64 {
  fn eq(self, other: i64) -> bool {
    return self == other
  }
}
```

The compiler now validates trait names, generic arity, visibility, and duplicate
trait impls. It also checks that trait impls provide exactly the declared
methods with matching parameter and return types.

Concrete impl methods can be called with method syntax:

```ari
impl Eq[i64] for i64 {
  fn eq(self, other: i64) -> bool {
    self == other
  }
}

fn main() -> i64 {
  let value = 7;
  if value.eq(7) {
    0
  } else {
    1
  }
}
```

This lowers to static dispatch through the matching impl method. If more than
one visible impl provides the same method name for the same receiver type, the
call is rejected as ambiguous until Ari has explicit disambiguation syntax.

## Trait Bounds

Generic function declarations can constrain type parameters with one trait
application:

```ari
trait Score {
  fn score(self) -> i64
}

fn use_score[T: Score](value: T) -> i64 {
  value.score()
}
```

At each call site, Ari checks that the concrete type implements the required
trait application. Generic trait arguments can refer to the same generic
parameters:

```ari
fn same[T: Eq[T]](left: T, right: T) -> bool {
  left.eq(right)
}
```

Inside a constrained generic function, a bound also selects the matching impl
method for static dispatch. This means `value.score()` can be accepted even if
the concrete type has multiple visible `score` methods from different traits,
as long as the generic parameter has a single bound that declares `score`.
Generic trait methods are still planned.

Generic impl blocks can also constrain their own parameters:

```ari
impl[T: Score] Box[T] {
  fn score(self) -> i64 {
    self.value.score()
  }
}
```

The bound is checked when the impl method is specialized for a concrete
receiver. Calling `Box[i64].score()` requires `i64` to implement `Score`.

Generic inherent impl blocks can define methods over a generic `self` receiver:

```ari
struct Box[T] {
  value: T,
}

impl[T] Box[T] {
  fn get(self) -> T {
    self.value
  }
}
```

`Box { value: 1 }.get()` and `Box { value: true }.get()` specialize `get` for
their concrete receiver types.

Generic trait impl blocks can also specialize at method-call and trait-bound
call sites:

```ari
trait Inner[T] {
  fn inner(self) -> T
}

impl[T] Inner[T] for Box[T] {
  fn inner(self) -> T {
    self.value
  }
}

fn read[Wrapped: Inner[i64]](value: Wrapped) -> i64 {
  value.inner()
}
```

`Box { value: 1 }.inner()` and `Box { value: true }.inner()` each pick the
matching generic trait impl specialization. Generic trait impls can use the
same `impl[T: Trait]` bounds, and those bounds participate in trait-bound
static dispatch.

Trait impls must be coherent. Ari rejects concrete and generic impls, or two
generic impls, when they can describe the same trait/type pair:

```ari
impl[T] Inner[T] for Box[T] { ... }
impl Inner[i64] for Box[i64] { ... } // overlaps the generic impl
```

The current coherence check is conservative for generic impls: if two generic
patterns can unify, Ari reports the overlap before dispatch becomes ambiguous.

Inherent methods can have method-level generics. Declarations use `[U]`, while
calls can either infer method type arguments from ordinary arguments or spell
them explicitly with `<U>`:

```ari
impl[T] Box[T] {
  fn replace[U](self, value: U) -> Box[U] {
    Box { value: value }
  }
}

let explicit = Box { value: true }.replace<i64>(20)
let inferred = Box { value: true }.replace(20)
```

Trait methods and trait impl methods can have method-level generics too. The
impl may use different generic parameter names; Ari compares them by position.
Calls can pass method type arguments explicitly or infer them from ordinary
arguments:

```ari
trait Convert {
  fn keep[Value: Score](self, value: Value) -> i64
}

impl Convert for Box {
  fn keep[Out: Score](self, value: Out) -> i64 {
    value.score()
  }
}

let explicit = Box { value: 1 }.keep<i64>(20)
let inferred = Box { value: 2 }.keep(20)
```

Method-level trait bounds on generic trait methods are checked when a concrete
call specializes the method. The impl may choose a different method generic
name, but the bound must match by position. If a method-level generic appears
only in the return type, pass it explicitly because it cannot be inferred from
call arguments.

Inherent impl blocks can also define associated functions without a `self`
receiver. Call them with `Type::name(...)`:

```ari
impl Point {
  pub fn new(x: i64, y: i64) -> Point {
    Point { x: x, y: y }
  }
}

let point = Point::new(4, 5)
```

Generic inherent impl blocks can define associated functions too. Use explicit
`<T>` type arguments on the associated call, or let Ari infer them from normal
call arguments when possible:

```ari
impl[T] Box[T] {
  fn new(value: T) -> Box[T] {
    Box { value: value }
  }
}

let boxed = Box::new<i64>(20)
let inferred = Box::new(20)
```

Associated functions can have method-level generics as well:

```ari
impl Factory {
  fn make[U](value: U) -> Box[U] {
    Box { value: value }
  }
}

let explicit = Factory::make<i64>(20)
let inferred = Factory::make(20)
```

Trait impl blocks can satisfy associated functions declared by a trait. They
are called with the implementing type name, and Ari rejects the call if more
than one visible trait impl provides the same associated function for that
type:

```ari
trait Make[Item] {
  fn make(value: Item) -> Self
}

impl[T] Make[T] for Box[T] {
  fn make(value: T) -> Box[T] {
    Box { value: value }
  }
}

let explicit = Box::make<i64>(20)
let inferred = Box::make(true)
```

Inside modules, methods are private by default. Mark the method `pub`, or mark
the whole `impl` block `pub`, to make its methods callable from outside the
module.

## Prelude Traits

Ari reserves compiler-known Rust-like trait names in the prelude:

```ari
Debug
Display
Default
Clone
Copy
Drop
Eq[T]
PartialEq[T]
Ord[T]
PartialOrd[T]
From[T]
Into[T]
TryFrom[T]
TryInto[T]
Iterable[T]
Iterator[T]
IntoIterator[T]
iter::Iterable[T]
iter::Iterator[T]
iter::IntoIterator[T]
ToString
ToOwned
```

`Iterator[T]` requires `fn next(self) -> Option[T]`. Direct `for` lowering works
for copyable non-borrow iterator values.

`IntoIterator[T]` currently requires `fn into_iter(self) -> Self`. A `for` loop
can use this subset when the returned `Self` value also implements
`Iterator[T]`. Returning a distinct iterator type and stateful iterator receiver
policy remain planned.

`Drop` is the prelude trait connected to explicit ownership destruction:

```ari
impl Drop for i64 {
  fn drop(self) -> void {
    return;
  }
}
```

When a binding is consumed by `drop value;`, Ari calls the lowered `Drop::drop`
implementation for that value type if one exists.

These can be referenced without importing a module:

```ari
impl Eq[i64] for i64 {
  fn eq(self, other: i64) -> bool {
    return self == other;
  }
}
```

Today this lets libraries agree on the same names while the compiler grows
associated types, `Result`-based conversion traits, iterator lowering, and
source-level standard modules. Concrete impl method calls and trait-bound
generic calls already lower through static dispatch. `Hash` is intentionally not reserved in the
prelude; it should live in an explicit collection/hash module later.

## Intended Model

Traits should describe behavior without inheritance:

- no class hierarchy
- no hidden object layout
- no implicit virtual dispatch by default
- static dispatch first
- explicit vtable-backed dynamic dispatch only through a clear trait-object
  representation

## Trait Object Types

Trait object type syntax uses `dyn`:

```ari
trait Score {
}

trait Compare[T] {
}

struct Holder {
  score: dyn Score,
  compare: dyn Compare[i64],
}
```

The compiler resolves the trait name, generic trait arguments, and module
visibility in type positions today.

Trait-object values must be requested explicitly with `as dyn Trait[...]`:

```ari
trait Score {
  fn score(self) -> i64
}

impl Score for i64 {
  fn score(self) -> i64 {
    self
  }
}

let value: dyn Score = 7 as dyn Score;
```

The semantic checker accepts this only when the concrete source type has a
matching impl. Implicit assignment from a concrete value to `dyn Trait` is still
rejected.

On the LLVM backend, concrete copyable non-borrow values are materialized as a
pair of data pointer and vtable pointer. The vtable stores erased receiver
thunks, so `value.score()` dispatches through the vtable slot while the thunk
loads the concrete receiver and calls the original impl method. Generic impls
such as `impl[T] Score[T] for Box[T]` can also be specialized into vtables for
concrete object types such as `Box[i64] as dyn Score[i64]`. Generic trait
methods are not object-safe: they remain available through static dispatch and
are rejected at `as dyn` conversion or dyn method-call sites. Trait-object
upcasts are rejected: Ari does not reinterpret one dyn object as another dyn
trait object, so create the target dyn value from a concrete source with
`as dyn Trait[...]`. `own`/borrow-valued dyn data pointers and raw
`--freestanding` lowering are still planned.

## Current Status

Trait declarations, impl conformance, bare `self` receiver inference,
concrete `value.method(...)` static dispatch, trait-bound generic static
dispatch, generic inherent `self` receiver methods, generic inherent
method-level generics, generic trait impl methods with `self` receivers,
generic impl bounds, trait impl coherence checks, inherent associated functions
such as `T::new(...)` and `Box::new<i64>(...)`, and trait impl associated
functions such as `Box::make<i64>(...)` are executable. Generic trait methods
with method-level bounds are executable. `dyn Trait[...]` type syntax resolves,
and explicit concrete-to-`dyn` conversions plus LLVM vtable dispatch are
executable for concrete copyable source values, including vtables built from
generic impl specializations. Generic trait methods are deliberately static-only
for dyn objects. Associated types, non-copy dyn data ownership, and raw backend
dyn dispatch are still planned. Dyn-to-dyn upcasts are explicitly rejected.
