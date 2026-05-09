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
current aggregate surface can use C layout. Generic aggregates are rejected, and
`@repr(C)` struct fields may use value, raw pointer, `ref`, or `ref mut` types;
`own` fields are rejected until the ownership ABI policy is explicit.
`@repr(C)` enums currently must be fieldless.

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

Exported symbols must be C identifiers. In executable builds, exporting the
symbol `main` is rejected because Ari generates the C entry point itself.

`@derive(...)` is validated on structs and enums and retained as metadata.
Actual derive expansion is planned as part of compile-time meta expansion.

## User Attributes

User-defined attributes can be reserved by declaring a matching `meta fn`:

```ari
meta fn trace(input: token_stream) -> token_stream {
}

@trace(enabled)
fn value() -> i64 {
  return 1;
}
```

User attributes are currently parsed, validated by name, and retained as
front-end metadata. The reserving `meta fn` must be concrete and non-generic;
generic meta functions are rejected so future expansion can bind each
attribute/macro entry point to one explicit `token_stream`, `ast`, or `type`
transform. Attribute macros that rewrite or insert AST nodes are planned.
