# Generic Aggregates

Generic aggregates are Ari structs, tuple structs, enums, and type aliases that
take type parameters. They are ordinary language features, not stdlib-specific
hooks: `Box[T]`, `Maybe[T]`, `Result[T, E]`, `Vec[T]`, and a compiler-shaped
`AstSlot[T]` all go through the same type application, substitution, layout,
and monomorphization rules.

## Syntax

Declarations use square brackets:

```ari
struct Box[T] {
  value: T,
}

struct Pair[A, B] {
  left: A,
  right: B,
}

enum Maybe[T] {
  Nothing,
  Just(T),
}

type TokenBox = Box[Token];
type PassAlias[T, E] = PassResult[T, E];
```

Type positions use `Name[Arg]`. Constructors and generic calls use angle
brackets when you need explicit type arguments:

```ari
let boxed: Box[Token] = Box<Token> { value: token };
let none: Maybe[Token] = Nothing<Token>();
let some = Just<Token>(token);
```

When the expected type is known, enum constructors can use it:

```ari
let value: Maybe[Token] = Nothing();
```

When a payload mentions every generic parameter, the constructor can infer the
arguments:

```ari
let value = Just(token);
```

## Discriminant-Linked Union Fields

Ari does not currently support discriminant-linked union fields inside
structs. The reserved roadmap spelling is:

```ari
struct TLSCiphertext {
  content_type: ContentType,
  version: ProtocolVersion,
  length: u16,
  security: SecurityParameters,
  fragment: union by security.cipher_type {
    stream => GenericStreamCipher,
    block => GenericBlockCipher,
    aead => GenericAEADCipher,
  },
}
```

The parser reads `union by` as a real type reference and preserves the selector
path and arm type list in the AST. Syntax dumps and declaration metadata can
show the field shape. During semantic validation, the selector must start from
an earlier field in the same struct, each nested selector segment must resolve
through a known struct field, arm names must be unique, and every arm payload
type must resolve. When the selector type is an enum, every arm name must be an
enum case and every enum case must have exactly one arm. After those checks
pass, executable lowering still rejects the field with a targeted type
diagnostic before layout or code generation.
Model this with an ordinary enum payload today, and keep any external
discriminant relationship explicit in constructor and validation code.

The compiler capability inventory tracks the reserved syntax as
`union-by-fields`. Enum selector arm checking is implemented as a semantic
diagnostic layer. Construction, active-arm drop, narrowing, layout, ABI, and
positive execution support remain future compiler work.

## Substitution

Ari substitutes concrete type arguments through all aggregate positions:

```ari
struct ParserBox[T] {
  cursor: i64,
  value: T,
}

enum PassResult[T, E] {
  Failed(E),
  Passed(T),
}

type Nested[T, E] = PassResult[Box[T], Maybe[E]];
```

For `ParserBox[Token]`, the field `value: T` becomes `value: Token`.
For `PassResult[Token, LexError]`, `Passed(T)` carries `Token` and
`Failed(E)` carries `LexError`. For
`Nested[Token, Diagnostic]`, the concrete type is
`PassResult[Box[Token], Maybe[Diagnostic]]`.

Substitution applies in:

- struct and tuple-struct fields
- enum payloads and constructors
- function parameters and returns
- method receivers and method results
- match payload bindings
- field access
- nested generic arguments
- generic type aliases

## Identity

Generic aggregate identity is nominal plus the concrete type argument list.
Different declaration names are different types even if their fields have the
same shape, and different concrete arguments are different types:

```ari
Box[Token] != Box[Span]
Result[Token, LexError] != Result[Span, LexError]
```

Type aliases do not create new runtime types. They expand to the target type
before type equality, layout, and code generation:

```ari
type TokenBox = Box[Token];

let one: TokenBox = Box<Token> { value: token };
```

Diagnostics should name the concrete type that failed. For example, passing
`Box[Box[Span]]` where `Box[Box[Token]]` is expected is a type mismatch.

## Layout And Codegen

Concrete generic aggregate instances are monomorphized. The compiler lowers a
separate concrete shape for each used type application, including nested
applications such as:

```ari
PassResult[Box[Token], Maybe[Diagnostic]]
AggregateEnvelope[Token, Diagnostic]
std::vec::Vec[Result[Token, LexError]]
```

Generic enum payload layout is computed from the concrete payload types. When
payloads have different aggregate shapes, Ari uses an aggregate enum payload
slot large enough to hold the active case payload, then casts back to the
selected case payload after the tag is known.

The root `Vec[T]` spelling is the local vector/storage type. The source stdlib
handle is `std::vec::Vec[T]` or the exported `std::Vec[T]` alias where that
handle is expected. Zone-backed stdlib handles carry allocation provenance, so
their field-storage rules are stricter than plain user-defined aggregates.

## Ownership

Ownership qualifiers inside generic arguments are preserved:

```ari
struct Box[T] {
  value: T,
}

fn consume(value: Box[own i64]) -> i64 {
  let moved = value.value;
  drop moved;
  drop value;
  return 1;
}
```

After `value.value` is moved, reading that field again is a use-after-move
error. Whole-value drops skip fields already moved or dropped, and enum drops
drop only the active payload.

## Current Limits

- Recursive aggregate values still need an indirection such as a pointer or a
  zone-backed handle; directly infinite value layout is rejected with
  `recursive aggregate value type 'Name[...]' requires indirection`.
- Generic aggregate values are reliable for local executable code and the LLVM
  backend paths covered by tests. External ABI exposure is narrower and still
  follows the aggregate ABI docs.
- Zone-backed stdlib handles are not a loophole around zone provenance checks.
  Prefer plain user-defined generic fixtures when testing compiler generic
  behavior, and use stdlib containers as stress tests.

## Tests To Read

- `tests/cases/generics/ok/generic-aggregate-monomorphization.ari`
- `tests/cases/generics/ok/generic-aggregate-stdlib-stress.ari`
- `tests/cases/generics/ok/generic-aggregate-recursive-pointer.ari`
- `tests/cases/generics/errors/generic-aggregate-*.ari`

Run focused checks with:

```sh
make check-generics
make check-compiler-artifacts
```
