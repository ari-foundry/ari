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

## Supertraits

Traits can require other traits with `:`:

```ari
trait Base {
  fn base(self) -> i64
}

trait Child: Base {
  fn child(self) -> i64
}
```

Any `impl Child for T` must also have a matching `impl Base for T`. Generic
bounds inherit supertrait methods for static dispatch, so a `T: Child` function
can call both `child()` and `base()` when the concrete type provides both impls.

When multiple traits provide the same method name for a type, use a
trait-qualified call to pick the impl explicitly:

```ari
let left = LeftScore::score(value)
let right = RightScore::score(value)
```

Generic trait applications use expression-call brackets:

```ari
let value = AddAs<i64>::add_as(20, 2)
```

A child trait can name a unique inherited supertrait method, such as
`Child::base(value)`. If more than one supertrait exposes the same method name,
name the specific supertrait instead.

This is behavior composition, not struct inheritance. Ari keeps structs as data
layout declarations; use named fields, tuple structs, or explicit embedding for
data reuse. There is no hidden base-object layout or implicit field/method
inheritance between structs.

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

When more than one trait impl provides the same associated function for the
same type, qualify the call with the trait and pass the implementing type as
the first associated-call type argument:

```ari
let first = First::pick<Box>(1)
let second = Second::pick<Box>(1)
```

Generic trait arguments stay on the trait path, while the implementing type is
written after the associated function name:

```ari
let boxed = Make<i64>::make<Box[i64]>(20)
```

When the destination has an explicit value type, Ari can use that expected
result type as the implementing type:

```ari
let boxed: Box[i64] = Make<i64>::make(20)
let branched: Box[i64] = if ready {
  Make<i64>::make(1)
} else {
  Make<i64>::make(2)
}
let pair: (Box[i64], Box[i64]) = (Make<i64>::make(3), Make<i64>::make(4))
```

Expected enum payload types are forwarded too, so a trait-qualified associated
call inside `Some(...)` or `Ok(...)` can use the payload type to select `Self`.
This includes primitive implementing types such as `i64`.

If the associated function has method-level generics, put them after the
implementing type or let Ari infer them from the value arguments. If the
implementing type is inferred from the expected result type, method-level
generics must be inferred from the value arguments.

## Associated Types

Ari reserves associated type declarations inside traits and associated type
projections for generic trait applications in type positions:

```ari
trait Iterator[T] {
  type Item
}

impl Iterator[i64] for Counter {
  type Item = i64
}

let item: Iterator[i64]::Item = 1
```

The spelling is Ari-style `Trait[T]::Item`, not Rust's
`<T as Trait>::Item`. The parser preserves declarations, impl witnesses, and
projections in the AST and module cache so lint and language-server tooling can
treat them as stable. During semantic lowering, a projection resolves to the
impl witness type when exactly one visible impl witness matches the trait
application. If no witness matches, or more than one impl witness matches, Ari
rejects the projection with a focused diagnostic instead of guessing a type.

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

`Display` and `fmt::Display` use the explicit-zone formatting hook:

```ari
fn format_in(self: ref Self, zone: ref mut Zone) -> std::string::String
```

`format_in!(ref mut zone, "{}", value)` calls this hook for user-defined value
types that are not one of the built-in string, integer, bool, or float cases.
The macro evaluates the value once, passes a shared borrow to the hook, then
appends the returned source `String` into the final output. Struct display impls
can read fields through the shared receiver, for example `self.x`, without
loading the receiver through a raw pointer.

`Iterator[T]` requires `fn next(self: ref mut Self) -> Option[T]`. Direct `for`
lowering works for copyable non-borrow iterator values by storing the iterator
once and mutably borrowing that hidden iterator binding for each `next` call.
The compiler still accepts value-self `next(self)` impls for copyable
snapshot-style iterators while older surface tests migrate.

`IntoIterator[T]` currently declares `fn into_iter(self: ref mut Self) -> Self`
as the default source shape. Impl methods may return a distinct iterator type;
impl validation requires the concrete `into_iter` result to implement
`Iterator[T]`, including generic impl headers whose `T` is still a placeholder
and distinct generic result types such as `BagIter[T]`, and `for` lowering
rechecks the specialized result. The compiler still accepts value-self
`into_iter(self)` impls for copyable snapshot-style containers. The first
stateful `Iterator.next` and `IntoIterator.into_iter` receiver policies are
implemented for copyable values. A first-class associated iterator result
spelling can replace this Ari-specific contract later; owner/borrow iterator
values and explicit iterator lifetime rules remain planned.

The `std::cmp` child module exposes its own comparison traits and value helpers:

```ari
min<T>(left, right)
max<T>(left, right)
clamp<T>(value, low, high)
cmp::min<T>(left, right)
cmp::max<T>(left, right)
cmp::clamp<T>(value, low, high)
```

These helpers require an impl of `cmp::Ord[T]` for `T` and are also re-exported
at the source `std` root. `clamp` asserts that the lower bound is not greater
than the upper bound.

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

Traits should describe behavior without class-style inheritance:

- no class hierarchy
- no struct inheritance
- no hidden object layout
- no implicit virtual dispatch by default
- static dispatch first
- explicit trait composition through supertraits
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
are rejected at `as dyn` conversion or dyn method-call sites. A `dyn Child`
value includes object-safe methods declared by `Child` and its supertraits, so
`value.base()` can dispatch through a `dyn Child` vtable when `Child: Base`.
If more than one supertrait exposes the same dyn method name, the method call is
ambiguous; use static trait-qualified dispatch on the concrete value before
erasing it. A dyn object can also be upcast to the same trait or one of its
supertraits with `as dyn Base`; the data pointer is preserved and the vtable
pointer is adjusted to the inherited supertrait method slots. Unrelated
dyn-to-dyn casts remain rejected. `own`/borrow-valued dyn data pointers and raw
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
generic impl specializations and inherited object-safe supertrait methods.
Generic trait methods are deliberately static-only for dyn objects. Associated
types, non-copy dyn data ownership, and raw backend dyn dispatch are still
planned. Dyn-to-dyn upcasts are executable when the target is the same trait or
an inherited supertrait; unrelated dyn casts are rejected.
