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
value, fixed-size array, raw pointer, `ref`, or `ref mut` types.
Generic structs may store generic fields by value; each concrete instantiation
resolves those fields to concrete layout slots before IR emission. Generic
`ptr T`, `ref T`, and `ref mut T` fields remain pointer-sized slots. C header
emission renders `ref T` as `const T*`; `ptr T` and `ref mut T` remain mutable
pointer spellings, with `[T, N]` fields and pointer-to-array slots using C array
declarators. `own` fields are rejected because the C ABI cannot carry Ari
ownership; expose ownership-sensitive values through an explicit `ptr` or `ref`
ABI instead.
Fieldless `@repr(C)` enums use Ari's fixed 64-bit tag ABI. Public non-generic
payload-bearing `@repr(C)` enums use an explicit C struct layout with an
`int32_t tag` field followed by payload storage slots. Scalar and
pointer-shaped payloads use raw `uint64_t payloadN` slots; non-scalar plain
aggregate payloads use generated wrapper typedefs. Generic `@repr(C)` enums are
accepted only when their type parameters do not appear in payload storage.
Payload slots that contain `own` are rejected; C-facing enums should expose
ownership-sensitive data through explicit `ptr` or `ref` wrapper APIs.

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
ari --test tests/cases/attributes/ok/attribute-test-runner.ari -o build/test-runner
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
declarations whose fields are scalar, fixed-size arrays, raw pointer, `ref`, or
`ref mut` slots, public non-generic `@repr(C)` structs passed or returned by
value, public fieldless `@repr(C)` enums, and public non-generic
payload-bearing `@repr(C)` enums. Immutable `ref` fields and
exported parameters are written as `const` C pointers in the header, including
`ref [T, N]` parameters and fields rendered as `const T (*)[N]`. `ptr [T, N]`
and `ref mut [T, N]` are rendered as mutable C pointer-to-array declarators.
Fieldless generic enum type
parameters do not affect layout, so the C header emits one erased tag typedef
for the source enum name. Enum headers use Ari's current fixed tag ABI by
emitting `typedef int64_t Name;` plus prefixed integer constants such as
`Name_Case = 0`. Payload-bearing enum headers emit `typedef struct Name Name;`,
define `struct Name` with `tag`/`payloadN` fields, and emit the same prefixed
case constants. The current header emitter skips private helpers, private
structs, private enums, and implicit source `std` helper functions. Generic
`@repr(C)` structs are emitted as opaque typedefs so pointer-only C APIs can
name them; exported by-value instantiations also get concrete
typedefs/definitions such as `GenericHandle_i64`. Direct by-value fixed-size
array parameters and returns are exposed through generated wrapper typedefs
such as `AriArray_i64_2`, because C function parameters cannot carry arrays by
value directly. By-value aggregate parameters and returns are checked by the
shared non-local aggregate ABI classifier and are emitted only for direct
aggregate ABI values on 64-bit Unix targets, currently up to 16 bytes with at
most 8-byte alignment. Larger or target-specific cases should use an explicit
pointer ABI. The classifier also covers tuples, fixed-capacity vector storage
values, and aggregate-layout enums, but header generation still rejects those
Ari-only value spellings until their C wrapper surface is explicit. Header
generation rejects values such as `string`, owned values, and aggregate values
whose C ABI policy is not explicit.
LLVM object and shared-library output record explicit export/no-mangle names in
their symbol tables. Imported `extern "C"` calls are resolved through the LLVM
driver and the normal host linker path.

`@derive(Debug)`, `@derive(Copy)`, `@derive(Clone)`, `@derive(Eq)`,
`@derive(PartialEq)`, `@derive(Ord)`, and `@derive(PartialOrd)` are supported
on structs and enums. `@derive(Default)` is
supported on named structs, tuple structs, and enums with an explicit default
case marker such as `@derive(Default(Ready))`. Each derive preserves the
generic parameters of generic declarations.
`Debug` and `Copy` expand to empty trait impls so derived values satisfy their
trait bounds. `Clone` expands on structs, fieldless enums, and payload-bearing
enums to an impl with a value-self `clone` method that returns `self`, matching
Ari's current `Clone` trait contract. Struct
`Default` derives expand to a `fn default() -> Self` method that constructs the
struct by calling `Default::default<FieldType>()` for each field; field types
must therefore have a visible `Default` impl. Enum `Default(CaseName)` derives
return the named case from `default`; payload values for that case are built
with `Default::default<PayloadType>()`. Struct `Eq` and `PartialEq` derives
expand to `fn eq(self, other: Self) -> bool` by comparing every field with the
matching `Eq[FieldType]::eq` or `PartialEq[FieldType]::eq` trait-qualified
call. Enum `Eq` and `PartialEq` derive methods compare cases first, then compare
matching payload slots with the matching payload `eq` trait method. Struct
`Ord` and `PartialOrd` derives expand to lexicographic
`fn lt(self, other: Self) -> bool` methods that call the matching field `lt`
trait method, returning on the first unequal field and `false` when all fields
compare equal. Enum `Ord` and `PartialOrd` derive methods order cases by source
declaration order, then compare matching payload slots lexicographically with
the matching payload `lt` trait method. Generic `Default`, `Eq`, `PartialEq`,
`Ord`, and `PartialOrd` derives add the matching std trait bounds for generic
parameters that appear in generated field or payload construction when those
parameters do not already have another constraint. `Copy` derive is a marker
trait impl only; it does not change Ari's structural copyability rules for
values. Other derive names are rejected until their trait method surfaces and
expansion contracts are defined.

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
reserve attributes; at those type sites, `type -> type` bodies may return
type AST output with `type!(...)`. Attribute macros over top-level functions,
structs, enums, traits, and impls can rewrite the annotated declaration when
their reserving `meta fn` returns declaration output through `decl!(...)` or
token output through `tokens!(...)`. The meta input is the annotated
declaration token tree without its attributes, and the original declaration is
omitted unless the output explicitly splices `input` back in:

```ari
meta fn add_generated(input: ast) -> ast {
  return decl!(
    input

    fn generated() -> i64 {
      return original() + 1;
    }
  );
}

@add_generated
fn original() -> i64 {
  return 41;
}
```

`token_stream -> token_stream` attribute macros can inspect their own balanced
argument token tree. `attribute_args_present(input)` distinguishes `@name(...)`
from a bare `@name`, `attribute_args_empty(input)` checks whether the argument
token tree is empty, and `attribute_args_count(input)` returns its token count.
`attribute_args_match(input, "...", "$name", "$tail...", ...)` uses the same
token-pattern language as `input.matches(...)`; captures from a successful
branch can be spliced into `tokens!(...)` output with `~name`. Direct token
returns can also use `attribute_args_capture(input, "name", ...)` or
`attribute_args_slice(input, start, end)`:

```ari
meta fn add_named(input: token_stream) -> token_stream {
  return if attribute_args_present(input) &&
            attribute_args_count(input) == 1 &&
            attribute_args_match(input, "$Name") {
    tokens!(
      input

      fn ~Name() -> i64 {
        return 41;
      }
    )
  } else {
    input
  };
}

@add_named(generated)
fn original() -> i64 {
  return 1;
}
```

Non-identity `ast -> ast` bodies are also executable at expression macro sites,
where a body can return an expression AST and substitute the meta input
parameter, at item macro sites, where a body can return declaration AST output
with `decl!(...)`, and at pattern macro sites, where a body can return pattern
AST output with `pattern!(...)`. Declaration-returning `ast -> ast` bodies can
branch on `decl_kind(input)` / `input.kind()`, `decl_name(input)` /
`input.name()`, `decl_count(input)` / `input.count()`, `decl_is_public(input)` /
`input.is_public()`, `decl_is(input, "kind")` / `input.is("kind")`, and
declaration shape counters such as `decl_field_count(input)` /
`input.field_count()`, `decl_param_count(input)` / `input.param_count()`,
`decl_case_count(input)` / `input.case_count()`, `decl_method_count(input)` /
`input.method_count()`, `decl_generic_count(input)` / `input.generic_count()`,
and `decl_associated_type_count(input)` / `input.associated_type_count()`.
Declaration members can also be inspected by name with predicates such as
`decl_has_field(input, "field")` / `input.has_field("field")`,
`decl_has_case(input, "Case")` / `input.has_case("Case")`,
`decl_has_method(input, "method")` / `input.has_method("method")`, and
`decl_has_associated_type(input, "Item")` /
`input.has_associated_type("Item")`. Type-summary helpers include
`decl_param_type`, `decl_field_type`, `decl_case_payload_type`,
`decl_method_param_type`, `decl_method_return_type`, `decl_return_type`,
`decl_trait_type`, and `decl_associated_type_type`, with matching
`input.*(...)` method forms. Member integer summaries include
`decl_case_payload_count`, `decl_method_generic_count`, and
`decl_method_param_count`, again with matching `input.*(...)` forms. Missing
members evaluate as false, zero, or an empty string depending on the helper
before choosing a `decl!(...)` output.
When a macro output declaration carries a
rewriting attribute, Ari expands that generated attribute before later
declaration collection; accidental self-generating chains stop at a recursion
limit.
