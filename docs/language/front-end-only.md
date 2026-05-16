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
The remaining front-end-only pieces are additional method-generating built-in
derives and broader structured AST declaration inspection.

```ari
@deprecated("use NewToken")
@derive(Debug, Clone, Default, Eq, PartialEq, Ord, PartialOrd)
@repr(C)
struct Token {
  kind: i64,
}
```

The compiler accepts a small built-in attribute surface:

- `@derive(Debug)` / `@derive(Copy)` / `@derive(Clone)` / `@derive(Eq)` / `@derive(PartialEq)` on structs and enums
- `@derive(Default)` on structs and `@derive(Default(CaseName))` on enums
- `@derive(Ord)` / `@derive(PartialOrd)` on structs
- `@repr(C)` on structs and enums
- `@deprecated` or `@deprecated("message")` on declarations
- `@test` on functions
- `@cfg(...)` on top-level declarations

`@repr(C)` is constrained to the supported C-layout surface. Struct fields may
use value, raw pointer, `ref`, or `ref mut` types. Generic structs may store
generic fields by value, and concrete instantiations resolve those fields to
concrete layout slots before IR emission. Generic `ptr T`, `ref T`, and
`ref mut T` fields remain pointer-sized slots. `own` fields are rejected
because C cannot model Ari ownership; use explicit pointer or borrow slots at
the boundary. `@repr(C)` enums currently must be fieldless, including generic
enums; C tagged-union payload layout is not implemented yet.

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
that calls each test in source order. `@derive(Debug)`, `@derive(Copy)`,
`@derive(Clone)`, `@derive(Eq)`, and `@derive(PartialEq)` expand for structs
and enums, including generic declarations; `@derive(Default)`, `@derive(Ord)`,
and `@derive(PartialOrd)` expand for structs; `@derive(Default(CaseName))`,
`@derive(Ord)`, and `@derive(PartialOrd)` expand for enums. `Debug` and `Copy`
generate empty trait impls, `Clone` generates `fn clone(self) -> Self { return
self; }`, struct `Default` generates a `default` associated function that
defaults each field, enum `Default(CaseName)` returns the named case with
defaulted payloads, `Eq`/`PartialEq` generate value-self `eq` methods for
structs and enums, and `Ord`/`PartialOrd` generate value-self `lt` methods for
structs and enums. Enum ordering uses source case order before payload
lexicographic comparison. `Copy` derive is only a marker-trait impl and does
not change structural copyability. Other derive
names remain reserved until their trait method surfaces are implemented.

User-defined attributes can be reserved by declaring a matching `meta fn`:

```ari
meta fn trace(input: token_stream) -> token_stream {
}

@trace({ enabled([1, 2], (3)) })
fn value() -> i64 {
  return 1
}
```

The attribute name is checked against meta functions today. Attribute arguments
are parsed as a balanced token tree inside the outer parentheses, preserving
nested `(...)`, `{...}`, and `[...]` groups for the future evaluator. Module
AST summaries keep the full token payload, including literal suffixes.
User-defined attribute macros over top-level functions, structs, enums, traits,
or impls can rewrite that declaration by returning top-level declarations.

## Meta Functions

```ari
meta fn rewrite(input: ast) -> ast {
}

meta fn identity_tokens(input: token_stream) -> token_stream {
}

meta fn choose_type(input: type) -> type {
}

fn main() -> i64 {
  return make_value!(1 + 2)
}
```

`meta fn` is the compile-time-only transform entry surface. Its current
stable declaration shape is one parameter and a matching return domain:
`token_stream -> token_stream`, `ast -> ast`, or `type -> type`. Meta
functions are intentionally non-generic; define one concrete entry point for
each transform shape instead of a `[T]`-generic meta function. Bodies may be
empty or contain a single `return input;` identity return, where `input` is the
meta function parameter. `token_stream -> token_stream` bodies may also return
raw output tokens with `tokens!(...)`; bare `input` tokens inside that
constructor are replaced with the invocation token payload before the output is
parsed at the invocation site:

```ari
meta fn double_expr(input: token_stream) -> token_stream {
  return tokens!((input) * 2);
}

fn main() -> i64 {
  return double_expr!(3 + 4)
}
```

The first token evaluator step supports expression-only branches over whether
the invocation token payload is empty, how many tokens it contains, or whether
it starts or ends with a specific token-text sequence, or whether a fixed token
position has a specific text spelling, or whether the whole payload is wrapped
by one delimiter pair. Token-stream returns can also extract an end-exclusive
slice with `tokens_slice(input, start, end)` or `input.slice(start, end)`.
Slice bounds use the same integer expressions as conditions, including
`tokens_count(input)`, `input.len()`, and `+` / `-` arithmetic over
non-negative values. The condition can use bool literals, `!`, `&&`, `||`,
`tokens_empty(input)`, `input.is_empty()`, integer comparisons over
`tokens_count(input)` or `input.len()` with that arithmetic, boundary checks with
`tokens_starts_with(input, "...", ...)`, `input.starts_with("...", ...)`,
`tokens_ends_with(input, "...", ...)`, or `input.ends_with("...", ...)`, and
delimiter-aware wrapper checks with `tokens_wrapped_by(input, "(", ")")` or
`input.wrapped_by("(", ")")`, and indexed checks with
`tokens_nth_is(input, index, "...")` or `input.nth_is(index, "...")`. Token
patterns use `tokens_match(input, "...", "_", ...)` or
`input.matches("...", "_", ...)`, where `"_"` matches one token. Pattern
strings beginning with `$`, such as `"$value"`, bind a named single-token
capture; a trailing ellipsis, such as `"$value..."`, binds a zero-or-more-token
span and backtracks against later pattern pieces. Branches can return that
capture or span with
`tokens_capture(input, "value", "...", "$value", ...)` or
`input.capture("value", "...", "$value", ...)`.
When a branch condition matches named captures, `tokens!(...)` can splice
those captures with `~name`; this lets an item macro match a declaration and
emit related declarations next to it.
Each branch must return token output, the input stream, a capture, or a slice:

```ari
meta fn default_expr(input: token_stream) -> token_stream {
  return if tokens_empty(input) {
    tokens!(0)
  } else {
    tokens!(input)
  };
}

meta fn count_expr(input: token_stream) -> token_stream {
  return if tokens_count(input) == 1 {
    tokens!((input) * 3)
  } else {
    tokens!((input) + 10)
  };
}

meta fn keyword_expr(input: token_stream) -> token_stream {
  return if tokens_starts_with(input, "fast", "+") {
    tokens!(40)
  } else {
    tokens!(input)
  };
}

meta fn operator_expr(input: token_stream) -> token_stream {
  return if tokens_nth_is(input, 1, "+") {
    tokens!(20)
  } else {
    tokens!(input)
  };
}

meta fn suffix_expr(input: token_stream) -> token_stream {
  return if input.ends_with("+", "fast") {
    tokens!(30)
  } else {
    tokens!(input)
  };
}

meta fn grouped_expr(input: token_stream) -> token_stream {
  return if tokens_wrapped_by(input, "(", ")") {
    tokens!(40)
  } else {
    tokens!(input)
  };
}

meta fn ungroup_expr(input: token_stream) -> token_stream {
  return if input.wrapped_by("(", ")") {
    input.slice(1, input.len() - 1)
  } else {
    input
  };
}

meta fn function_like(input: token_stream) -> token_stream {
  return if input.matches("fn", "_", "(", ")") {
    tokens!(1)
  } else {
    tokens!(0)
  };
}

meta fn wrapped_value(input: token_stream) -> token_stream {
  return if input.matches("wrap", "(", "$value", ")") {
    input.capture("value", "wrap", "(", "$value", ")")
  } else {
    tokens!(0)
  };
}

meta fn wrapped_expr(input: token_stream) -> token_stream {
  return if input.matches("wrap", "(", "$expr...", ")") {
    input.capture("expr", "wrap", "(", "$expr...", ")")
  } else {
    tokens!(0)
  };
}

meta fn CreateNewFn(input: token_stream) -> token_stream {
  return if input.matches("struct", "$Name", "{", "value", ":", "i64", ",", "}") {
    tokens!(
      input

      impl ~Name {
        fn new(value: i64) -> ~Name {
          ~Name { value: value }
        }
      }
    )
  } else {
    input
  };
}

CreateNewFn!(struct GeneratedBox {
  value: i64,
});
```

Expression-position `ast -> ast` macros may also return an expression AST
directly and use the meta input parameter as a substitution point. Literal,
struct literal, tuple, vector, field/index/tuple
access, borrow, postfix try, null-coalescing, function call, method call,
unary, binary, and cast expression trees are accepted. `if`, block, and
`match` expression returns are accepted too; block expressions and `if`/`if let`
arms may contain statement bodies:

```ari
meta fn add_one(input: ast) -> ast {
  return input + 1;
}

meta fn score(input: ast) -> ast {
  return add(input.left, input.values[1]);
}

meta fn make_pair(input: ast) -> ast {
  return Pair { left: input, right: 2 };
}

meta fn borrow_shared(input: ast) -> ast {
  return ref input;
}

meta fn or_zero(input: ast) -> ast {
  return input ?? 0;
}

meta fn select_positive(input: ast) -> ast {
  return if input > 0 { input } else { 0 };
}

meta fn staged(input: ast) -> ast {
  return {
    let base = input + 1;
    var total = base * 2;
    total += 3;
    total
  };
}

meta fn bumped(input: ast) -> ast {
  return input.bump(2);
}

fn main() -> i64 {
  return add_one!(40 + 1)
}
```

The invocation input is still parsed as one expression to validate the macro
argument domain. When the returned AST contains the meta parameter name as a
bare expression, Ari substitutes that node with the parsed invocation
expression. This first non-identity AST step has no quote/eval syntax, and
names other than the meta input parameter are rejected in returned expression
ASTs unless they are generated local bindings in the returned statement body or
pattern-bound names inside the current `if let` or `match` arm. Generated local
and pattern bindings are renamed internally so repeated macro calls do not
collide with Ari's local-name rules. Item-position `ast -> ast` macros can return
declaration AST output with the meta-body-only `decl!(...)` constructor:

```ari
meta fn make_answer(input: ast) -> ast {
  return decl!(fn generated_answer() -> i64 {
    return 42;
  });
}

make_answer!();
```

`decl!(...)` parses its payload as top-level declarations and can currently be
used only as the return value of an `ast -> ast` meta function that is invoked
from an item macro site or from a top-level declaration attribute macro. A bare
token that matches the meta parameter name inside `decl!(...)` is replaced with
the item macro invocation's declaration input tokens or the annotated
declaration tokens:

```ari
meta fn append_generated(input: ast) -> ast {
  return decl!(input const GENERATED: i64 = 1;);
}

append_generated!(fn original() -> i64 {
  return 41;
});
```

Declaration-returning `ast -> ast` meta functions can branch on structured
declaration input before choosing a `decl!(...)` output. The supported
inspection helpers are `decl_kind(input)` / `input.kind()`, `decl_name(input)` /
`input.name()`, `decl_count(input)` / `input.count()`, `decl_is_public(input)` /
`input.is_public()`, and `decl_is(input, "kind")` / `input.is("kind")`.
Shape counters are also available as
`decl_generic_count(input)` / `input.generic_count()`,
`decl_param_count(input)` / `input.param_count()`,
`decl_field_count(input)` / `input.field_count()`,
`decl_case_count(input)` / `input.case_count()`,
`decl_method_count(input)` / `input.method_count()`, and
`decl_associated_type_count(input)` / `input.associated_type_count()`.
Member summaries can be tested with `decl_has_generic(input, "T")` /
`input.has_generic("T")`, `decl_has_param(input, "name")` /
`input.has_param("name")`, `decl_has_field(input, "name")` /
`input.has_field("name")`, `decl_has_case(input, "Case")` /
`input.has_case("Case")`, `decl_has_method(input, "name")` /
`input.has_method("name")`, and
`decl_has_associated_type(input, "Item")` /
`input.has_associated_type("Item")`. Type summaries are string-valued and can
be compared in the same branch conditions: `decl_return_type(input)` /
`input.return_type()`, `decl_trait_type(input)` / `input.trait_type()`,
`decl_param_type(input, "name")` / `input.param_type("name")`,
`decl_field_type(input, "name")` / `input.field_type("name")`,
`decl_case_payload_type(input, "Case", 0)` /
`input.case_payload_type("Case", 0)`, `decl_method_return_type(input, "name")`
/ `input.method_return_type("name")`, `decl_method_param_type(input, "method",
"param")` / `input.method_param_type("method", "param")`, and
`decl_associated_type_type(input, "Item")` /
`input.associated_type_type("Item")`. Member integer summaries are available as
`decl_case_payload_count(input, "Case")` /
`input.case_payload_count("Case")`,
`decl_method_generic_count(input, "name")` /
`input.method_generic_count("name")`, and
`decl_method_param_count(input, "name")` /
`input.method_param_count("name")`. Missing members evaluate as false, zero, or
an empty string depending on the helper:

```ari
meta fn add_for_struct(input: ast) -> ast {
  return if input.is("struct") && input.name() == "Widget" {
    decl!(input fn generated() -> i64 { return 41; })
  } else {
    decl!(input fn generated() -> i64 { return 0; })
  };
}

add_for_struct!(struct Widget {
  value: i64,
});
```

`token_stream -> token_stream` meta functions can use `tokens!(...)` and the
empty-input branch form at item and pattern macro sites too. The chosen output
is parsed in the site domain after raw token substitution. Count-based
conditions, token matching, named captures, and `~name` capture splices are
available there as well, so an item macro can preserve an input declaration and
emit related generated declarations in one expansion.

User-defined attribute macros can use the declaration output path too. The meta
input is the annotated declaration token tree without its attributes, and the
original declaration is replaced by the macro output. Splice `input` into the
output when the generated declarations should keep the annotated declaration.
For `token_stream -> token_stream` attribute macros, the attribute's own
balanced argument tokens are available through `attribute_args_present(input)`,
`attribute_args_empty(input)`, `attribute_args_count(input)`,
`attribute_args_match(input, ...)`, `attribute_args_capture(input, ...)`, and
`attribute_args_slice(input, ...)`; successful pattern captures can be spliced
into `tokens!(...)` output with `~name`. Rewriting attributes generated by
macro output are expanded through the same declaration rewrite path:

```ari
meta fn second(input: token_stream) -> token_stream {
  return tokens!(
    input

    fn generated() -> i64 {
      return 41;
    }
  );
}

meta fn first(input: token_stream) -> token_stream {
  return tokens!(@second input);
}

@first
fn original() -> i64 {
  return 1;
}
```

Pattern-position `ast -> ast` macros can return pattern AST output with the
meta-body-only `pattern!(...)` constructor. A bare token that matches the meta
parameter name inside `pattern!(...)` is replaced with the invocation's pattern
input tokens:

```ari
meta fn wrap_some(input: ast) -> ast {
  return pattern!(Some(input));
}

match value {
  wrap_some!(inner) => {
    return inner;
  }
}
```

Type-position `type -> type` macros can return type output with the
meta-body-only `type!(...)` constructor. A bare token that matches the meta
parameter name inside `type!(...)` is replaced with the invocation's type
input tokens:

```ari
meta fn pointer_to(input: type) -> type {
  return type!(ptr input);
}

let raw: pointer_to!(i64) = (ref mut value) as pointer_to!(i64);
```

Expression, item, type, and pattern-position macro invocation use Rust-style
`ident!(...)` syntax. The built-in prelude assertion, stop, `print!`,
`println!`, and unqualified `matches!` macros lower today. Prelude expression
macros other than the parser-special `matches!` spelling are matched only
through unqualified names or paths that resolve to the root `std` macro
spelling; other qualified paths are left for user meta functions even when
their basename matches a prelude macro. User expression macros must resolve to
`token_stream -> token_stream` or `ast -> ast` meta functions. Empty and
`return input;` bodies are identity transforms: the token tree inside
`ident!(...)` is parsed as one expression and then lowered as that expression.
Expression returns from `ast -> ast` bodies replace that parsed input at
expression macro sites, with bare meta-parameter references substituted by the
parsed input expression. Function names and field names in the returned AST are
ordinary source names resolved after expansion; only bare expression names other
than the meta parameter are rejected at meta body validation time. User
syntax-rewriting attributes and active
item-position macros must also resolve to
`token_stream -> token_stream` or `ast -> ast` meta functions. Function,
constant, struct, enum, trait, impl, inline module, and use item macro
expansion can parse the token tree as top-level
function/constant/struct/enum/trait/impl/module/use declarations and splice
those generated items into normal semantic checking. Non-identity
item-position `token_stream -> token_stream` bodies can generate those
declaration kinds through `tokens!(...)`; captures from a successful
`input.matches(...)` branch are spliced into that output with `~name`.
Non-identity item-position `ast -> ast` bodies can also generate those
declaration kinds by returning `decl!(...)`, and they may splice the input
declaration token tree back into that constructor with the meta parameter name.
Declaration-returning AST bodies may choose among `decl!(...)` outputs with
expression-only `if` branches over the declaration inspection helpers listed
above.
Generated file-backed `mod name;` imports are not supported in item macro
output because module loading runs before semantic expansion; keep file-backed
imports as source-level `mod` declarations. Type-position
macro invocations must resolve to
`type -> type` meta functions. Empty and `return input;` bodies
identity-expand by parsing their token tree as one type before semantic type
lowering; `type -> type` bodies that return `type!(...)` replace that input
with the constructed type AST. General syntax-rewriting meta evaluation and
additional derives are still planned. Default-zone `format!` is intentionally
not executable in the 0.x language surface because Ari has no implicit
allocation zone; use executable `format_in!` for explicit-zone owned string
formatting.
Pattern-position `ident!(...)` uses the same reserved spelling and balanced
token-tree parser. It is preserved in the AST and module summaries, checked
against `token_stream -> token_stream` or `ast -> ast` meta functions. Empty
and `return input;` bodies identity-expand by parsing the token tree as one
pattern before ordinary pattern lowering; `ast -> ast` bodies that return
`pattern!(...)` replace that input with the constructed pattern AST.

Macro invocation is the only parser-level token-tree expression form. A macro
call is always an ordinary named call such as `make_tokens!(...)`; there is no
separate anonymous macro grammar. The parser captures the balanced token tree
inside the outer parentheses and preserves nested `(...)`, `{...}`, and `[...]`
groups. The selected `meta fn` parameter domain determines whether the future
evaluator receives `token_stream`, `ast`, or `type` input:

```ari
meta fn make_tokens(input: token_stream) -> token_stream {
}

let value = make_tokens!(1 + 2 * 3);

meta fn make_item(input: token_stream) -> token_stream {
}

make_item!(fn generated_value() -> i64 {
  return 21;
});

make_item!(const GENERATED: i64 = 21;);

make_item!(struct Generated {
  value: i64,
});

make_item!(enum GeneratedState {
  Ready,
  Done,
});

make_item!(trait Score {
  fn score(self) -> i64
});

make_item!(impl Score for i64 {
  fn score(self) -> i64 {
    return self + 1;
  }
});

make_item!(
  mod GeneratedApi {
    pub fn value() -> i64 {
      return 21;
    }
  }

  use GeneratedApi::value as generated_value;
);

meta fn make_type(input: type) -> type {
}

let value: make_type!(i64) = 0;

meta fn pointer_to(input: type) -> type {
  return type!(ptr input);
}

let raw: pointer_to!(i64) = null;
```

Active user macro expressions parse their input as one expression immediately;
malformed expression input, such as extra comma-separated tokens, is rejected
before semantic expression lowering. Item-position macro invocations parse
function, constant, struct, enum, trait, impl, inline module, and use
declaration output immediately; malformed supported items or generated
unsupported item kinds are rejected before normal declaration collection.
Pattern-position macro invocations parse their input as one pattern
immediately; malformed pattern input, such as extra comma-separated tokens, is
rejected before pattern lowering. Type-position macro invocations parse their
input as a type immediately; malformed type input, such as extra
comma-separated tokens, is rejected before semantic type lowering. The
`ident!(...)` token-tree surface is fixed for linting and disabled
`@cfg(false)` declarations still parse.

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
