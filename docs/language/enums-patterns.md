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
Case constructor names must be unique in their module. Payload constructors
check both arity and payload type, and pattern positions reject unknown enum
case names instead of treating them as open-ended variants.
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
nested aggregate-enum values, owned word payloads written as `own i64` or
`own u64`, and plain Ari-layout tuple, fixed-array, or struct values.
Fixed-capacity vector payloads use the explicit `Vec[T; N]` storage spelling
and occupy the full payload slot as a vector value. Plain aggregate payloads
occupy the full payload slot and can be bound as full values in `match` arms,
or destructured with tuple, fixed-array, or struct payload subpatterns that
contain value bindings, aliases, wildcards, and nested product subpatterns.
Owned word payloads can be constructed and value-bound in direct temporary
constructor matches, and the bound payload must be consumed or dropped before
the arm exits. Direct constructor values stored in locals or assigned to whole
locals also seed tag-aware payload ownership for the known active case: a
fieldless case such as `None` has no live payload owner, while `Some(own i64)`
must be dropped or moved before the local exits, and `value.0` can move the
active owned payload from a direct-constructor local. Runtime-dependent
aggregate enum values, including parameters and values received from
aggregate-returning calls, can be explicitly dropped as whole values; the
lowered cleanup tests the tag and drops only the active owning payload slots.
Statement `match` arms over tracked runtime-dependent local and parameter
subjects seed tag-known owner payload states, so `Some(token)` can value-bind
an owning payload and the binding must be dropped or moved before the arm exits.
Runtime-dependent payload-slot moves outside statement `match`, such as
`value.0`, are supported for uniform owner layouts and for tag-conditioned
layouts when control flow has narrowed the active case through `match`,
`if let`, or `while let`. Expression-valued `match` arms seed the same owner
payload paths. After a statement `match`, branches that move or drop the active
payload merge as an unavailable owner state, and later whole-value cleanup skips
the already-consumed payload. Fixed-capacity vector payload slots can also be
destructured with
exact array-style element patterns such as `Values([first, second])`; the match
arm checks the vector's current runtime length before extracting the inline data
slots. If one payload position mixes
payload-word values with one nested aggregate enum type, or with one plain
aggregate whose first field is an `i64`/`u64` scalar lane, the slot uses that
aggregate layout. Payload-word cases zero-initialize that storage and write the
payload word into the scalar lane, while aggregate cases store the full value.
This mixed-slot rule covers ordinary scalar, pointer-shaped, one-word enum
payloads, and compiler-shaped `Result[Token, LexError]` values where the token
record starts with an integer kind lane. It does not allow bare root `Vec[T]`,
fixed-capacity vectors without a scalar lane, or multiple different aggregate
slot shapes to share a slot.
The LLVM backend
can store and copy local
aggregate enum values, then match local values by tag with positional payload
bindings, scalar payload literal/range tests, and one-level enum-case payload
tests for compact, homogeneous nested, or mixed-lane nested aggregate enum
payloads. Pointer-backed aggregate enum copies work through `ptr_load`,
`ptr_store`, and `*pointer` when the pointer is a `ptr EnumType`; storing a
direct enum constructor through those pointer helpers is also supported,
including homogeneous or mixed nested aggregate-enum payload values.
For low-level layout code, aggregate enum payload slots can be addressed with
tuple-index syntax on local or raw-pointer-backed values. `value.0` and
`(*raw).0` mean payload slot 0; the hidden tag field is not part of the source
index. This access does not test the active case. Scalar, pointer-shaped, and
one-word enum payload slots expose the stored `u64` payload word, while nested
aggregate-enum slots expose the nested enum storage itself. Plain tuple,
fixed-array, struct, and fixed-capacity vector slots expose the stored
aggregate value.
Direct LLVM calls can pass and return aggregate enum values through
hidden pointer slots. The LLVM backend also materializes direct
aggregate enum `match` inputs through hidden stack slots, so constructors,
aggregate-returning calls, `if`/`match`/block expression results, and
raw-pointer-backed loads such as `*raw` can be matched without first binding a
named local.

## Passing And Returning

The LLVM backend can store, pass, and return lowered enum values. Compact
one-word enum values can also be compared with `==` and `!=`; aggregate enum
comparison is planned. The LLVM backend currently keeps aggregate enum
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

Fixed-capacity `Vec[T; N]` aggregate payload slots use array-style element
patterns. The element list is exact: `Values([a, b])` only matches a vector whose
current length is 2, even when its capacity is larger. Use `Values(values)` or
`Values(_)` as the fallback arm for other lengths:

```ari
enum Packet {
  Values(Vec[i64; 3]),
  Empty,
}

let total = match packet {
  Values([first, second, third]) => first + second + third,
  Values([first, second]) => first * 10 + second,
  Values(_) => 0,
  Empty => 0,
};
```

When an aggregate enum payload slot stores a one-word enum value or a nested
aggregate enum value, a nested enum-case subpattern can inspect the inner tag
and a single scalar payload on the LLVM backend:

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

Reference binding mode can borrow addressable payload slots directly:

```ari
let ref Full(payload) = maybe_wide
let ref Pair(left, right) = nested_pair
match maybe_wide {
  Full(ref payload) => { use(payload); }
  Empty => {}
}
if let Full(&payload) = maybe_wide {
  use(payload);
}

while let Full(ref payload) = next_maybe_wide() {
  use(payload);
  break;
}
```

This form takes the same panic path when the active case does not match. It is
available for aggregate-layout enum payload slots that have an addressable
payload location, including 64-bit payload-word slots and nested aggregate-enum
slots. Compact small payloads remain value-only because their payload lives
inside the tag word rather than in a separate slot. Nested shared reference
bindings are available in enum statement/expression `match` arms and enum
`if let`/`while let` arms, including enum `while let` or-pattern alternatives
that bind the same names with the same types. Mutable payload reference
bindings are available in enum statement/expression `match`, enum `if let`,
and enum `while let` arms when the matched subject is an addressable local,
field, or indexed element. Non-addressable temporaries remain value-only for
mutable payload borrows.

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

Fixed arrays, local `Vec[T]` storage, and `Slice[T]` views can be matched with
`[a, b]` positional patterns. Like tuple patterns, array patterns can contain
literal tests, ranges, `_`, immutable bindings, aliases, nested product
patterns, and one `..` rest marker. For `Vec[T]` and `Slice[T]`, `..` makes the
runtime length check use `>=`; without it, the check uses exact length equality.
Runtime sequence rest markers can also bind the skipped middle range with
`name @ ..`. That binding has type `Slice[T]` and points at the matched
sequence without copying elements. Fixed arrays still treat `..` only as a
skip marker.
Runtime sequence `match` expressions and statements must still include an
irrefutable fallback such as `_` or `[..]`.

```ari
let score = match values {
  [0, ..] => 1,
  [head, .., 9] => head,
  [..] => 0,
};

let [first, middle @ .., last] = values;
let middle_len = len(middle);

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

Local `Vec[own T]` value patterns can move exact element bindings,
known-length suffix element bindings after `..`, and direct unknown-length
suffix element bindings after `..`; selected `_` elements and skipped rest-gap
elements are dropped from the hidden Vec storage. Unknown-length rest gaps use
runtime index loops before suffix owners are moved or dropped. Known-length
`Vec[own T]` value/rest patterns can bind `rest @ ..` as a non-owning
`Slice[own T]` view; unknown-length owning rest aliases are rejected because
the view would overlap runtime-selected suffix owner moves. Hidden pattern
storage remains borrowed while a known rest view is live, then Ari cleans any
still-owned hidden slots at scope exit. The view can be destructured by
reference patterns, but value patterns, direct indexing, and indexed assignment
cannot move or replace owner elements through that non-owning Slice.
Local `let ref` can
borrow exact local `Vec[own T]` element slots, including nested owned fields
inside aggregate elements, when each selected element path is statically known,
and local `let ref` patterns with `..` can borrow ownership-carrying prefix
elements plus suffix elements when a direct local vector has a known current
length and no rest alias. Local `let ref` and `let ref mut` suffixes over
unknown-length `Vec[own T]` storage are also allowed without rest aliases: Ari
checks that all tracked owned elements are live, then records synthetic suffix
owner paths so multi-binding mutable suffixes such as
`let ref mut [first, .., prev, last]` keep distinct borrow paths.
Local `let ref` plus function-entry reference patterns can borrow supported
non-owning `Vec[T]`/`Slice[T]` elements today.
Nested reference modes inside enum `while let` support shared borrows for
enum-case patterns and same-name/same-type or-pattern alternatives.
Pattern-position macro invocation uses reserved Rust-style `ident!(...)`
syntax and preserves a balanced token tree in the AST and module summaries. The
name must resolve to a `token_stream -> token_stream` or `ast -> ast` meta
function. Empty bodies and `return input;` bodies identity-expand by parsing the
token tree as exactly one pattern; `ast -> ast` bodies can return
`pattern!(...)` output. Expanded pattern macro output then flows through the
same match, or-pattern, and reference-binding machinery as source-written
patterns, including macro-generated `ref` payload bindings:

```ari
match value {
  make_pattern!(Some(_)) => { return 1; }
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
Match pattern bindings are currently value bindings only. Reference or mutable
binding-mode spellings inside match patterns, such as `ref name`,
`ref mut name`, `&name`, `&mut name`, and `mut name`, are still reserved for
the future shared binding-mode engine and are rejected with a dedicated
diagnostic today. Borrow inside the arm, or bind a local first with
`let ref` / `let ref mut` or `let &` / `let &mut` when the source is a tracked
local place.
When the matched subject is a direct temporary constructor, or a tracked
runtime-dependent local or parameter in a statement `match`, a payload of type
`own i64` or `own u64` can be value-bound and then explicitly dropped inside
the arm.

Expression-valued `match` currently supports enum patterns and copyable
payloads. Borrow-valued arm results are rejected until the borrow checker grows
richer expression lifetime tracking. Arms that end in `panic()`, `todo()`, or
`unreachable()` are non-continuing and do not need to manufacture a dummy value;
the reachable value arms determine the match result type and merged ownership
state. Tuple-valued and aggregate-enum match arm results lower on the LLVM
backend when the matched value and arm-result storage are supported.

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
