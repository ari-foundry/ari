# Attributes

Attributes attach metadata or compiler behavior to declarations.

```ari
@deprecated("use next_value")
fn old_value() -> i64 {
  return 1;
}

@repr(C)
struct Point {
  x: i64,
  y: i64,
}
```

## Built-In Attributes

`@repr(C)` may be used on structs and enums. The compiler validates that the
current aggregate surface can use C layout. `@repr(C)` struct fields may use
value, raw pointer, `ref`, or `ref mut` types.
Generic structs may store generic fields by value; each concrete instantiation
resolves those fields to concrete layout slots before IR emission. Generic
`ptr T`, `ref T`, and `ref mut T` fields remain pointer-sized slots. C header
emission renders `ref T` as `const T*`; `ptr T` and `ref mut T` remain mutable
pointer spellings. `own` fields are rejected because the C ABI cannot carry
Ari ownership; expose ownership-sensitive values through an explicit `ptr` or
`ref` ABI instead.
`@repr(C)` enums currently must be fieldless, including generic enums; generic
parameters are accepted only when they do not appear in payload storage.

`@cfg(...)` prunes top-level declarations before name collection and type
checking. Disabled declarations must still parse, but their names, types, and
bodies are not resolved. Supported predicates are `true`, `false`, `all(...)`,
`any(...)`, `not(...)`, `target("name")`, and `feature("name")`.

```ari
@cfg(feature("fast"))
fn value() -> i64 {
  return 13;
}

@cfg(not(feature("fast")))
fn value() -> i64 {
  return 1;
}
```

Enable feature predicates from the command line with `--cfg-feature name` or
`--feature name`.

`@deprecated` may be written without arguments or with one string message. Uses
of deprecated functions, structs, and enums emit warnings but do not stop
compilation.

`@borrow_return(source)` may be used on borrow-returning functions and extern
declarations to name the parameter that the returned `ref` or `ref mut` comes
from. Add `.field` or `.0`-style components when the returned borrow is a
known subpath:

```ari
@borrow_return(left)
fn pick_left(left: ref i64, right: ref i64) -> ref i64 {
  return ref left;
}

@borrow_return(value)
extern "C" fn identity_ref(value: ref i64) -> ref i64;
```

For Ari functions with bodies, the checker verifies that every return follows
the declared source path. Extern declarations must use this attribute before
they may return tracked Ari borrow values.

`@test` marks a function for the compiler test runner. Run tests with:

```sh
ari --test tests/ok/attribute-test-runner.ari -o build/test-runner
```

In test mode, Ari synthesizes a `main` that calls all `@test` functions in
source order. Test functions cannot take parameters, be generic, be extern,
be meta functions, or be named `main`. They may return `void` or `i64`; the
runner ignores returned values and exits `0` after all tests return. Assertion
helpers such as `assert` and `assert_eq_i64` can stop the process with a
non-zero status.

`@export`, `@export("symbol")`, and `@no_mangle` may be used on non-generic
Ari functions to choose the emitted C symbol. `@export` and `@no_mangle`
without arguments use the final source path segment:

```ari
@export("ari_public_add")
pub fn add(left: i64, right: i64) -> i64 {
  return left + right;
}

@no_mangle
pub fn increment(value: i64) -> i64 {
  return value + 1;
}
```

Exported symbols must be C identifiers and cannot collide with another emitted
Ari function symbol. In executable builds, exporting the symbol `main` is
rejected because Ari generates the C entry point itself.
When building with `--shared`, `pub` functions and explicit export/no-mangle
functions are ABI-visible; private Ari helpers are emitted with hidden LLVM
visibility, as are Ari-owned runtime helpers.
Use `--emit-c-header path` with the LLVM/shared path to write a small C header
for exported scalar/raw-pointer functions, public non-generic `@repr(C)` struct
declarations whose fields are scalar, raw pointer, `ref`, or `ref mut` slots,
public non-generic `@repr(C)` structs passed or returned by value, and public
fieldless `@repr(C)` enums. Immutable `ref` fields and exported parameters are
written as `const` C pointers in the header. Fieldless generic enum type
parameters do not affect layout, so the C header emits one erased tag typedef
for the source enum name. Enum headers use Ari's current fixed tag ABI by
emitting `typedef int64_t Name;` plus prefixed integer constants such as
`Name_Case = 0`. The current header emitter skips private helpers, private
structs, and private enums. Generic `@repr(C)` structs are emitted as opaque
typedefs so pointer-only C APIs can name them; exported by-value instantiations
also get concrete typedefs/definitions such as `GenericHandle_i64`. By-value
struct parameters and returns are emitted only for direct aggregate ABI values
on 64-bit Unix targets, currently up to 16 bytes with at most 8-byte alignment;
larger or target-specific cases should use an explicit pointer ABI. Header
generation rejects Ari-only values such as `string`, owned values, and
aggregate values whose C ABI policy is not explicit.
Raw `--freestanding` ELF output records explicit export/no-mangle names in the
static symbol table too. Imported `extern "C"` calls still require the LLVM host
backend until the raw backend grows a native C link path.

`@derive(Debug)`, `@derive(Copy)`, `@derive(Clone)`, `@derive(Eq)`, and
`@derive(PartialEq)` are supported on structs and enums. `@derive(Default)` is
supported on named and tuple structs. Each derive preserves the generic
parameters of generic declarations. `Debug` and `Copy` expand to empty trait
impls so derived values satisfy their trait bounds. `Clone` expands to an impl
with a value-self `clone` method that returns `self`, matching Ari's current
`Clone` trait contract. Struct `Default` derives expand to a `fn default() ->
Self` method that constructs the struct by calling
`Default::default<FieldType>()` for each field; field types must therefore have
a visible `Default` impl. Struct `Eq` and `PartialEq` derives expand to
`fn eq(self, other: Self) -> bool` by comparing every field with the matching
`Eq[FieldType]::eq` or `PartialEq[FieldType]::eq` trait-qualified call.
Fieldless enum `Eq` and `PartialEq` derives compare the enum tags directly;
payload-bearing enum equality derives are reserved until payload comparison
policy is defined. Generic `Default`, `Eq`, and `PartialEq` derives add the
matching std trait bounds for generic parameters that appear in fields when
those parameters do not already have another constraint. `Default` for enums is
reserved until Ari has an explicit default-case marker. `Copy` derive is a
marker-trait impl only; it does not change Ari's structural copyability rules
for values. Other derive names are rejected until their trait method surfaces
and expansion contracts are defined.

## User Attributes

User-defined attributes can be reserved by declaring a matching `meta fn`:

```ari
meta fn trace(input: token_stream) -> token_stream {
}

@trace({ enabled([1, 2], (3)) })
fn value() -> i64 {
  return 1;
}
```

User attributes are currently parsed, validated by name, and retained as
front-end metadata. Attribute arguments are a balanced token tree inside the
outer parentheses, so nested `(...)`, `{...}`, and `[...]` groups are accepted
and mismatched delimiters are rejected during parsing. The full token payload is
preserved in module AST summaries, including numeric values, float bits, and
literal suffixes. The reserving `meta fn` must be concrete, non-generic, and
written as either `token_stream -> token_stream` or `ast -> ast`; `type -> type`
meta functions are accepted only at type-position macro sites and cannot
reserve attributes. Meta bodies must stay empty until Ari has a compile-time
evaluator. Attribute macros that rewrite or insert AST nodes are planned.
