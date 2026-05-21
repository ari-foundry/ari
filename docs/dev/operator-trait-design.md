# Trait-Backed Operators

This page records the operator plan for compiler and standard-library work.
It separates what is implemented today from the planned user-defined operator
surface, so contributors do not have to guess which layer owns a change.

## Implemented Today

`==`, `!=`, `<`, `<=`, `>`, and `>=` first use Ari's builtin comparable or
ordered operands:

- integers, including compatible `char`/`u8` boundary cases
- floats
- bool
- value enums in the supported tagged-union subset

When builtin comparison is not available, sema rewrites the operator through
method dispatch:

```ari
left == right  // left.eq(right)
left != right  // !left.eq(right)
left < right   // left.lt(right)
left > right   // right.lt(left)
left <= right  // !right.lt(left)
left >= right  // !left.lt(right)
```

The selected `eq` or `lt` method must return `bool`. The standard-library
spellings are `cmp::Eq[T]` and `cmp::Ord[T]`, but the lowering intentionally
reuses normal method dispatch so generic bounds and existing impl selection
rules stay consistent:

```ari
fn same[T: cmp::Eq[T]](left: T, right: T) -> bool {
  return left == right;
}

fn before[T: cmp::Ord[T]](left: T, right: T) -> bool {
  return left < right;
}
```

This is a narrow bridge for builtin comparison glyphs. Custom glyphs such as
`++` are not parsed as Ari operators yet.

Source `std` code should use this bridge too. Library helpers such as
`cmp::compare`, `Slice.ordering`, `algo::sort`, and `algo::binary_search`
should read as ordinary operator code while keeping `cmp::Eq[T]` and
`cmp::Ord[T]` as the public extension points.

## Planned Surface

The planned source shape is an operator declaration that binds a glyph to a
normal function or trait method:

```ari
trait DoubleAdd[T] {
  fn doubleadd(self, other: T) -> T;
}

op infix(60, left) `++` = doubleadd;
```

The exact marker-trait names are still open, but the intended vocabulary is:

```ari
trait PrefixOper[Symbol, Self]
trait InfixOper[Symbol, Left, Right]
trait PostfixOper[Symbol, Self]
```

`PrefixOper` and `PostfixOper` are unary. `InfixOper` is binary. The compiler
should reject a declaration whose target has the wrong arity before lowering
any call site.

## Builtin Collision Rule

User code must not redefine builtin or reserved operator spellings:

```text
+ is available only if no builtin token or reserved syntax uses it.
==, !=, +, -, *, /, %, &&, ||, ?, ??, .., ..=, ->, =>, :: are reserved.
Assignment forms such as += and <<= are also reserved.
```

This keeps existing Ari code stable. Builtin operators can become trait-backed
one by one, as `==` and `!=` now do, but that is a compiler-owned mapping, not
a user-level redefinition.

## Parser Model

The lexer should continue to tokenize known builtin operators as fixed tokens.
Unknown operator runs can become a single custom-operator token only when the
custom operator feature is enabled. Operator declarations should be collected
into a small table before expression parsing needs precedence.

The Pratt parser can then keep its current shape:

1. Parse prefix/primary/postfix normally.
2. For an infix token, look up precedence and associativity in a constant-time
   table.
3. Parse the right-hand side with the normal precedence rule.

This remains linear in the number of tokens. The parser never searches all
operator declarations while parsing an expression.

## Lowering Contract

Sema should lower a custom operator to the same call machinery used by ordinary
functions and trait methods:

- prefix/postfix targets take one operand
- infix targets take two operands
- target visibility and module rules are normal Ari rules
- generic functions and trait bounds use existing monomorphization
- diagnostics name both the operator glyph and the resolved target

IR and LLVM should not know whether a call came from operator syntax. Sema
should lower enough metadata that codegen emits an ordinary call or builtin
operation.

## First Useful Test Slices

- Positive: `cmp::Eq[T]`-backed `==` and `!=` for a concrete struct.
- Positive: `cmp::Ord[T]`-backed `<`, `<=`, `>`, and `>=` for a concrete
  struct.
- Positive: the same operators inside `fn same[T: cmp::Eq[T]]` and
  `fn before[T: cmp::Ord[T]]`.
- Negative: `a == b` on a struct with no `eq` method reports the missing
  method.
- Negative: `a < b` on a struct with no `lt` method reports the missing
  method.
- Future positive: ``op infix(60, left) `++` = doubleadd;`` parses and lowers
  to a two-argument call.
- Future negative: ``op infix(60, left) `==` = equal;`` is rejected because
  the builtin equality token is reserved.
