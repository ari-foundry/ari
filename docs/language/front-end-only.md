# Front-End Only Syntax

This page lists syntax the front end accepts so the language can grow without
redesigning the parser.

These features are not fully executable yet.

## Trait Objects

```ari
trait Score {
}

struct Holder {
  value: dyn Score,
}
```

`dyn Trait[...]` types are parsed and resolved in declarations. The compiler
checks that the trait exists, is visible, and receives the right number of type
arguments. Explicit `value as dyn Trait[...]` conversions for concrete
copyable non-borrow values lower on the LLVM backend with vtable dispatch,
including generic impl vtables specialized for concrete object types. The
compiler rejects generic trait methods as non-object-safe at dyn conversion and
dyn method-call sites. The remaining front-end-only/planned pieces are dyn
upcasts, `own`/borrow-valued dyn data pointers, and freestanding backend
lowering.

## Generic And Destructured Structs

```ari
struct Point {
  x: i64,
  y: i64,
}

struct Meter {
  mut value: i64,
}

struct Pair[T] {
  left: T,
  right: T,
}

struct TuplePoint(i64, bool)
```

Local structs now lower with named literals, field access, tuple-struct
construction, tuple-struct positional access, field-level `mut` assignment
checks, generic struct / tuple-struct constructor type-argument inference, and
generic inherent impl methods with `self` receivers. The remaining
front-end-only pieces are broader destructuring modes and non-local aggregate
ABI layout.

## Generic ADTs

```ari
enum Option[T] {
  None,
  Some(T),
}
```

Non-generic enums lower today, including LLVM aggregate layout for multi-payload
and single `i64`/`u64` payload cases. Generic enum monomorphization is planned.

`..` can ignore the rest of a struct, tuple, array, or multi-payload ADT pattern.
Slice patterns are still planned.

## Generic Functions

```ari
fn identity[T](value: T) -> T {
  return value
}
```

Simple generic function calls are monomorphized when all type parameters can be
inferred from call arguments. Trait bounds on generic functions are checked at
call sites and can select constrained static method dispatch, including generic
trait impls such as `impl[T] Trait for Box[T]`. Generic inherent impl methods
and associated functions specialize at concrete call sites, including explicit
or inferred method-level generic calls such as `value.replace<i64>(next)`,
`value.replace(next)`, and `Factory::make<T>(next)`. Generic function names can
also specialize into function pointer values when an expected `fn(...) -> ...`
type selects the concrete parameter and result types. Generic aggregate
monomorphization now covers structs, enums, and integer range values; the
remaining planned aggregate work is broader ABI support and allocation-backed
prelude ADTs.

## Attributes

See [Attributes](attributes.md) for implemented built-in attribute behavior.
The remaining front-end-only pieces are derive expansion and user-defined
attribute macros that rewrite or insert AST nodes.

```ari
@deprecated("use NewToken")
@derive(Debug)
@repr(C)
struct Token {
  kind: i64,
}
```

The compiler accepts a small built-in attribute surface:

- `@derive(...)` on structs and enums
- `@repr(C)` on structs and enums
- `@deprecated` or `@deprecated("message")` on declarations
- `@test` on functions
- `@cfg(...)` on top-level declarations

`@repr(C)` is constrained to the supported C-layout surface. Struct fields may
use value, raw pointer, `ref`, or `ref mut` types. Generic structs are accepted
when generic parameters appear only in pointer-sized slots such as `ptr T`,
`ref T`, or `ref mut T`; value-stored generic fields are rejected until generic
aggregate layout is explicit. `own` fields are rejected until the ownership ABI
policy is explicit. `@repr(C)` enums currently must be fieldless, including
generic enums; C tagged-union payload layout is not implemented yet.

`@cfg(false)` prunes a declaration before name collection and type checking.
The disabled declaration must still parse, but its names, types, and body are
not resolved. `@cfg(true)` keeps the declaration. Boolean predicate forms
`all(...)`, `any(...)`, and `not(...)` are supported, as are target predicates
such as `target("unix")`, `target("linux")`, `target("windows")`, and common
architecture names. `feature("name")` is enabled with `--cfg-feature name` or
`--feature name`.

`@deprecated` may be written without arguments or with one string message. Uses
of deprecated functions, structs, and enums emit warnings but do not stop
compilation.

`@test` functions can be run with `ari --test`, which synthesizes a `main`
that calls each test in source order. `@derive(...)` is still validated and
retained as metadata; generated derive implementations are planned.

User-defined attributes can be reserved by declaring a matching `meta fn`:

```ari
meta fn trace(input: token_stream) -> token_stream {
}

@trace(enabled)
fn value() -> i64 {
  return 1
}
```

The attribute name is checked against meta functions today. Expansion is
planned.

## Meta Functions

```ari
meta fn rewrite(input: ast) -> ast {
}

meta fn identity_tokens(input: token_stream) -> token_stream {
}

fn main() -> i64 {
  return make_value!(1 + 2)
}
```

`meta fn` signatures may use `token_stream`, `ast`, or `type`. They are
intentionally non-generic; define one concrete meta entry point for each
transform shape instead of a `[T]`-generic meta function. The compiler parses
and validates these functions as compile-time declarations, but it does not
execute them yet.

Expression and item macro invocation use Rust-style `!` syntax. The built-in
prelude assertion, stop, `print!`, `println!`, and `matches!` macros lower
today. User `meta fn` expansion, item macros, and `format!` are still planned
and rejected with specific diagnostics.

## Raw Pointers

```ari
ptr u8
```

Raw pointer types are parsed. Nullable raw pointers, explicit pointer casts,
byte-wise `ptr_offset(pointer, bytes)`, typed scalar and Ari-layout aggregate
`ptr_add(pointer, count)` arithmetic, `size_of<T>()` / `align_of<T>()` layout
queries, `ptr_load(pointer)`, `ptr_store(pointer, value)`, and `*pointer`
dereference load/store syntax are executable today for scalar values and plain
Ari-layout aggregates. Host LLVM builds also support explicit `Zone` allocation
through `zone::create`, raw `zone::alloc`, typed `zone::alloc<T>`,
placement `zone::new<T>`, local scratch `zone::scratch<T>`, explicit scratch
promotion `zone::promote<T>`, `zone::reset`, and `zone::destroy`; the
freestanding backend still rejects zones. Associated
constructor-style APIs are executable as ordinary inherent impl functions such
as `T::new(ref mut Zone, ...)`.
