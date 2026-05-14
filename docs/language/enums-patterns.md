# Enums And Pattern Matching

## ADT Enums

```ari
enum OptionI32 {
  None,
  Some(i32),
}
```

Enum cases can have no payload, one payload written with `Case(T)`, or a
positional payload list such as `Case(T, U, V)`.
Cases are comma-separated. A trailing comma is accepted; semicolons are
reserved for statements and are rejected inside enum case lists.
Generic enum declarations use square brackets:

```ari
enum Option[T] {
  None,
  Some(T),
}
```

Concrete generic enum types also use square brackets in type positions:

```ari
let value: Option[i64] = Some(20)
```

Generic enum values can be constructed once the type argument is explicit, can
be inferred from payload values, or comes from an expected enum type such as a
local annotation, assignment target, or return type. Option/Result-style
generic enums support `?` and `??`; generic enum pattern matching and
refutable payload bindings use the concrete type arguments of the matched
value. Constant generic enum values can be initialized when the constant
annotation supplies the concrete enum type.

## Constructors

Zero-payload cases can be used by name:

```ari
let empty = None
```

Payload cases use call syntax:

```ari
let full = Some(42)
```

Generic payload constructors can infer type arguments:

```ari
let full: Option[i64] = Some(42)
```

The expected enum type can also specialize zero-payload cases and is passed
into payload expressions before they are checked:

```ari
let empty: Option[i64] = None()
let full: Option[i64] = Some(42)
```

Use angle brackets on the call when neither the payload nor an expected enum
type determines the type, or when spelling the specialization explicitly:

```ari
let empty = None<i64>()
let full = Some<i64>(42)
```

Constants use their declared type to specialize generic enum cases:

```ari
const EMPTY: Option[i32] = None;
const FULL: Option[i32] = Some(7i32);
const EXPLICIT: Option[i32] = Some<i32>(9i32);
```

## Current Runtime Layout

The executable subset lowers compact non-generic enums as one-word tagged
unions:

- low 32 bits: case tag
- high 32 bits: one payload slot

The payload slot currently accepts:

```ari
bool
i8
i16
i32
u8
u16
u32
```

When a case carries multiple payloads, or a single `i64`/`u64` payload, the LLVM
backend switches that enum to an aggregate layout:

```text
{ i32 tag, i64 payload0, i64 payload1, ... }
```

Aggregate enum payload slots currently accept integer, bool, pointer-shaped
values such as `string`, `ptr T`, and `fn(...) -> ...`, one-word enum values,
and nested aggregate-enum values when every case that uses the same payload
slot stores the same nested enum type. Tuples, structs, vectors, owned values,
and mixed scalar/nested-enum payload slots remain planned. The freestanding
backend can store and copy local aggregate enum values, then match local values
by tag with positional payload bindings, scalar payload literal/range tests,
and one-level enum-case payload tests for compact or homogeneous nested
aggregate enum payloads. Pointer-backed aggregate enum copies work through
`ptr_load`, `ptr_store`, and `*pointer` when the pointer is a `ptr EnumType`;
storing a direct enum constructor through those pointer helpers is also
supported, including homogeneous nested aggregate-enum payload values.
Direct freestanding calls can pass and return aggregate enum values through
hidden pointer slots.

## Passing And Returning

The LLVM backend can store, pass, and return lowered enum values. Compact
one-word enum values can also be compared with `==` and `!=`; aggregate enum
comparison is planned. The freestanding backend currently keeps aggregate enum
support local-stack-only.

```ari
fn choose(flag: bool) -> OptionI32 {
  if flag {
    return Some(7)
  }
  return None
}
```

## Try Propagation

Two-case enums can participate in postfix `?` propagation when they use the
same surface shape as `Option` or `Result`. Success cases are named `Some`,
`Ok`, or `Success` and must carry one payload. Residual cases are named `None`,
`Err`, `Error`, or `Failure`.

```ari
fn bump(value: OptionI32) -> OptionI32 {
  let inner = value?
  return Some(inner + 1i32)
}
```

On a success case, `?` evaluates to the payload. On a residual case, the
current function returns the original enum value immediately. The current
function must return that same enum type.

Generic Option/Result-style enums use the concrete type arguments of the enum
value:

```ari
enum Option[T] {
  None,
  Some(T),
}

fn bump(value: Option[i64]) -> Option[i64] {
  let inner = value?
  return Some(inner + 1)
}

fn main() -> i64 {
  return bump(Some(41)) ?? 0
}
```

## Match

`match` is available as both a statement and an expression in the executable
subset. Statement arms use `=>` followed by a block body.

```ari
fn score(value: OptionI32) -> i64 {
  match value {
    Some(inner) => {
      return inner as i64;
    }
    None => {
      return 0;
    }
  }
}
```

Expression-valued `match` arms use `=>` followed by an expression. The arms
must produce one compatible result type, and the whole expression can be bound,
returned, or used inside a larger expression.

```ari
fn score(value: OptionI32) -> i64 {
  return match value {
    Some(inner) => inner as i64,
    None => 0,
  };
}
```

Single-payload enum cases can match a binding, `_`, a scalar literal, a range,
an alias around those forms, or an or-pattern of literal/range alternatives
inside the payload slot. This works for compact one-word enum payloads; it also
works for aggregate enum layouts such as `i64`/`u64` payload storage on the
LLVM backend. A bool payload case is exhaustive when both `true` and `false`
payload literals are covered:

```ari
let score = match value {
  Some(1) => 10,
  Some(other) => other as i64,
  None => 0,
};

let flag_score = match maybe_flag {
  Flag(true) => 1,
  Flag(false) => 0,
  Empty => -1,
};
```

When an aggregate enum payload slot stores a one-word enum value or a nested
aggregate enum value, a nested enum-case subpattern can inspect the inner tag
and a single scalar payload on the LLVM and freestanding backends:

```ari
enum Inner {
  Empty,
  Small(i32),
}

enum Outer {
  Wrap(Inner),
  Other(i64),
}

let score = match value {
  Wrap(Small(n @ 2..=4)) => n as i64,
  Wrap(Empty) => 10,
  Wrap(_) => 0,
  Other(_) => -1,
};
```

The same forms work for concrete generic enum values:

```ari
fn score(value: Option[i32]) -> i64 {
  return match value {
    Some(inner) => inner as i64,
    None => 0,
  };
}

let Some(inner) = Some<i32>(7i32)
```

Multi-payload enum cases use positional payload patterns. `..` can ignore the
middle or remaining payload fields:

```ari
enum Color {
  Rgb(u8, u8, u8),
  Gray(i64),
  Off,
}

let score = match color {
  Rgb(red, green, blue) => (red as i64) + (green as i64) + (blue as i64),
  Gray(level) => level,
  Off => 0,
};

let edge = match color {
  Rgb(first, ..) => first as i64,
  Gray(..) => 7,
  Off => 0,
};
```

The same single-payload enum-case binding form can be used in local `let` and
`var` declarations:

```ari
let Some(value) = maybe_value
var Some(total) = next_value
let (Left(value) | Right(value)) = choice
let picked @ (Left(1) | Right(2)) = choice
```

Multi-payload enum binding patterns use the same positional form:

```ari
let Rgb(red, green, blue) = color
let Gray(..) = shade
```

These declaration patterns are refutable. If the value is a different enum
case, Ari takes the panic path. Use `if let` or `match` when the failure path
needs its own program logic. The payload binding follows the declaration
mutability: `let Some(x)` creates an immutable `x`, while `var Some(x)` creates
a mutable `x`. Or-pattern alternatives in declarations must bind the same names
with the same types. Alias-wrapped or-pattern declarations bind the whole enum
value on each successful alternative.

Parentheses can group a pattern without changing what it matches. This is
mainly useful for readability and for lower-precedence `|` alternatives:

```ari
let score = match value {
  (Some(1)) => 10,
  Some((other)) => other as i64,
  (None) => 0,
};
```

Use `|` to share one arm body across alternatives. Or-patterns are expanded in
source order, so overlapping alternatives keep the normal first-match behavior.
Alternatives may bind names when every expanded alternative introduces the same
names with the same types:

```ari
let score = match value {
  1 | 2 => 10,
  3..=5 | 8 => 20,
  _ => 0,
};

match event {
  Ping | Error(_) => {
    return 0;
  }
  Data(code) => {
    return code as i64;
  }
}
```

```ari
let score = match event {
  Add(amount) | Sub(amount) => amount as i64,
  End => 0,
};

let mirrored = match pair {
  (value, 0) | (0, value) => value,
  _ => 0,
};

let array_mirrored = match [0, 8] {
  [item, 0] | [0, item] => item,
  _ => 0,
};

let point_mirrored = match point {
  Point { x: axis, y: 0 } | Point { x: 0, y: axis } => axis,
  _ => 0,
};
```

Aliases can wrap an or-pattern in match arms. Ari expands the alternatives and
binds the alias in each expanded arm:

```ari
let score = match event {
  whole @ (Add(1) | Sub(2)) => classify(whole),
  rest @ (Add(amount) | Sub(amount)) => (amount as i64) + classify(rest),
  End => 0,
};

let tuple_score = match pair {
  matched_pair @ ((1, true) | (2, false)) => matched_pair.0,
  _ => 0,
};
```

An or-pattern such as `Some(value) | None` is rejected because `None` does not
bind `value`. `Small(value) | Big(value)` is also rejected if the two payload
types differ.

Integer and bool values can also be matched with literal patterns. Integer
matches can use literal or range patterns. They need `_` unless literal/range
arms cover the whole finite integer type. Bool matches may either cover both
`true` and `false` or include `_`.

```ari
let score = match value {
  1 => 10,
  2..=5 => 20,
  6..8 => 30,
  _ => 0,
};

let byte_score = match byte {
  0..=127 => 1,
  128..=255 => 2,
};

let flag_score = match flag {
  true => 1,
  false => 0,
};
```

Path patterns can also name scalar constants:

```ari
const LOW: i64 = 1;

mod Limits {
  pub const HIGH: i64 = 10;
  pub const YES: bool = true;
}

let score = match value {
  LOW => 1,
  Limits::HIGH => 2,
  _ => 0,
};

let flag_score = match flag {
  Limits::YES => 1,
  false => 0,
};
```

Constant patterns also work inside single scalar enum payload slots and nested
tuple or struct field patterns. In binding positions, a bare identifier still
introduces a binding; use a qualified path such as `Limits::HIGH` when that
position would otherwise be ambiguous.

Integer literal and range arms are checked in order. If more than one arm could
match the same value, the first matching arm wins:

```ari
let score = match value {
  0..=10 => 1,
  10..=20 => 2,
  _ => 0,
};
```

For `value == 10`, this produces `1`.
If a later scalar arm is completely covered by earlier scalar arms, Ari reports
a warning and keeps compiling. This catches dead cases without changing the
ordered first-match behavior.

Tuple values can be matched with tuple patterns. Tuple match arms may use
literal tests, ranges, `_`, immutable bindings, aliases, nested tuple patterns,
and one `..` rest marker to skip middle fields. Product matches are exhaustive
when they either contain an irrefutable arm such as `_`, `(..)`, or `(x, y)`,
or when Ari can prove every combination is covered. Small bool/integer products
use finite coverage. Larger integer products use symbolic rectangle coverage for
literal, range, wildcard, binding, alias, or-pattern, nested tuple, fixed array,
named struct, tuple-struct, and `..` rest patterns. Unsupported product shapes
should still use an irrefutable fallback arm.
When Ari rejects a non-exhaustive product match, it includes a missing case hint
when the gap can be represented as a literal, range, wildcard, or nested product
pattern.
If a later product arm is fully covered by earlier product arms, Ari reports a
warning and keeps compiling.

The empty tuple pattern `()` matches Ari's unit value:

```ari
let unit_score = match () {
  () => 1,
};

let score = match pair {
  (0, _) => 1,
  (1..=10, true) => 2,
  (x, _) => x,
};

let nested = match ((4, false), 8) {
  ((n, false), tail @ 8) => n + tail,
  _ => 0,
};

let rest = match (5, 6, true) {
  (0, ..) => 99,
  (head, .., true) => head,
  _ => 0,
};

let finite = match (left, right) {
  (true, true) => 4,
  (true, false) => 3,
  (false, true) => 2,
  (false, false) => 1,
};

match (2, false) {
  (1, _) => {
    return 1;
  },
  (x, false) => {
    return x;
  },
  _ => {
    return 0;
  },
}
```

Fixed arrays can be matched with `[a, b]` positional patterns. Like tuple
patterns, array patterns can contain literal tests, ranges, `_`, immutable
bindings, aliases, nested product patterns, and one `..` rest marker. The rest
marker skips elements; it does not bind a runtime slice yet.

```ari
let score = match values {
  [0, ..] => 1,
  [head, .., 9] => head,
  [..] => 0,
};

let nested = match ([1, 2], false) {
  ([1, tail], false) => tail,
  _ => 0,
};
```

Struct values can be matched with named-field patterns, and tuple structs can
be matched with positional constructor patterns. Named struct patterns must
mention every field unless they use `..`.

```ari
struct Point {
  x: i64,
  y: i64,
  flag: bool,
}

struct Rgb(i64, bool)

let named = match point {
  Point { x: 0, .. } => 99,
  Point { x: px, flag: true, .. } => px,
  _ => 0,
};

let tupled = match color {
  Rgb(1, _) => 99,
  Rgb(red, false) => red,
  _ => 0,
};

let nested_struct = match (point, color) {
  (Point { y: py, .. }, Rgb(red, true)) => py + red,
  _ => 0,
};
```

Slice patterns remain planned for the shared richer pattern engine.
Pattern-position macro invocation uses reserved Rust-style `ident!(...)`
syntax and preserves a balanced token tree, but active pattern expansion is
still planned:

```ari
match value {
  make_pattern!({ Some(_) }) => { return 1; }
  _ => { return 0; }
}
```

Alias patterns bind the matched value while still testing an inner pattern:

```ari
let score = match value {
  whole @ Some(inner) => {
    let bonus = match whole {
      Some(_) => 1,
      None => 0,
    };
    (inner as i64) + bonus
  },
  None => 0,
};
```

The alias is immutable and scoped to its arm. Alias patterns may wrap literal,
range, enum-case, tuple, array, struct, or or-pattern forms in match arms.
Payload aliases are also supported for the current single-payload enum surface:

```ari
let score = match value {
  Some(inner @ 5) => inner as i64,
  _ => 0,
};
```

## Exhaustiveness

`match` must cover every enum case unless a wildcard arm exists:

```ari
match value {
  Some(_) => {
    return 1;
  }
  _ => {
    return 0;
  }
}
```

Payload cases must write a payload pattern. Use `_` to ignore a payload.

Payload bindings are immutable branch-local values.
Pattern bindings are currently value bindings only. Reference or mutable
binding-mode spellings such as `ref name`, `ref mut name`, `&name`, `&mut name`,
and `mut name` are reserved for the future shared binding-mode engine and are
rejected with a dedicated diagnostic today. Borrow inside the arm or use a
named `ref` / `ref mut` parameter instead.

Expression-valued `match` currently supports enum patterns and copyable
payloads. Borrow-valued arm results are rejected until the borrow checker grows
richer expression lifetime tracking. Tuple-valued and aggregate-enum match arm
results lower on the LLVM backend and on the freestanding backend when the raw
backend already supports the matched value and arm-result storage. Freestanding
arm results work for local aggregate enum matches that use tag checks and
payload bindings, scalar payload literal/range checks, or one-level compact
enum-case payload checks.

## Match Diagnostics

- A `match` must have at least one arm.
- Enum matches must be exhaustive unless a wildcard arm is present.
- Duplicate case arms are rejected.
- Arms after a wildcard are unreachable and rejected.
- Payload cases must use `Case(name)` or `Case(_)`.
- No-payload cases must not use a payload pattern.
- Cases from a different enum cannot be matched against the current enum value.
- Branches that continue after the `match` must leave ownership state
  compatible with each other.
- Expression arms must produce compatible types and ownership states.
