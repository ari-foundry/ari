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

Ari supports an early executable slice of discriminant-linked union fields
inside structs. The spelling is:

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
enum case and every enum case must have exactly one arm. When the selector type
is `bool`, the arm names must be exactly `false` and `true`. Other selector
types are rejected; model richer discriminants as an enum so the compiler can
prove that every active payload arm is covered.

Enum-selector and bool-selector `union by` fields can be constructed in struct
literals with the same arm names:

```ari
let packet = TLSCiphertext {
  content_type: application_data,
  version: tls12,
  length: 7 as u16,
  security: SecurityParameters { cipher_type: stream },
  fragment: stream => GenericStreamCipher { value: 41 },
};
```

The natural constructor spelling is `arm(payload)`, which works for enum arms
and for bool arms. The call takes exactly one payload expression. If an arm
needs multiple values, declare the arm payload as a struct or tuple and pass
that one aggregate value.

```ari
let feature = Feature {
  payload: true(40),
};
```

The field payload is checked against the selected arm's payload type. The
compiler lowers the field to internal enum storage, so the aggregate can be
laid out and emitted by the existing enum backend. When the selector value is
statically visible in the same struct literal, the constructor arm must match
that value. If the selector field is written with a dynamic expression such as
a function call or local variable, the compiler cannot prove the selected
payload arm from the literal alone and rejects the constructor. Omit the
selector so it can be inferred from the arm, or write a literal enum case/bool
selector that matches the arm.

When the selected arm's payload type is a struct, the constructor can omit the
payload type name and write the arm name directly before the payload fields:

```ari
let packet = TLSCiphertext {
  security: SecurityParameters { cipher_type: stream },
  fragment: stream { value: 41 },
};
```

This shorthand is equivalent to `fragment: stream(GenericStreamCipher { value:
41 })`. It still checks the payload fields against the arm's declared payload
type. Use the explicit `arm(value)` form for scalar, tuple, enum, or
already-built payload values. The older `arm => value` form is accepted as a
compatibility spelling.

Bool selectors use literal arm names:

```ari
struct Feature {
  enabled: bool,
  payload: union by enabled {
    false => DisabledPayload,
    true => EnabledPayload,
  },
}

let feature = Feature {
  enabled: true,
  payload: true(EnabledPayload { value: 40 }),
};
```

When the selector is a direct enum field in the same struct, the selector may
be omitted and inferred from the `union by` constructor arm:

```ari
struct Packet {
  kind: PacketKind,
  fragment: union by kind {
    stream => StreamPayload,
    block => BlockPayload,
  },
}

let packet = Packet {
  fragment: stream { value: 41 },
};
```

The compiler fills `kind` with the enum case named by the arm. The same
inference works with `arm(payload)`, `arm => payload`, or the struct-payload
shorthand. Bool selectors can also be inferred from `false` and `true`
constructor arms; for example `payload: true(1)` or `payload: true { value: 1 }`
fills an omitted `enabled: bool` selector with `true`. Nested selector paths
can be inferred the same way when the omitted selector value can be built from
the constructor arm:

```ari
struct SecurityParameters {
  cipher_type: CipherType,
}

struct TLSCiphertext {
  security: SecurityParameters,
  fragment: union by security.cipher_type {
    stream => StreamPayload,
    block => BlockPayload,
  },
}

let packet = TLSCiphertext {
  fragment: stream { value: 41 },
};
```

Here the compiler synthesizes `security: SecurityParameters { cipher_type:
stream }`. If the intermediate struct has other required fields, write that
struct field explicitly and leave only the selector field for inference. If more
than one `union by` field uses the same omitted selector, all of their
constructor arms must infer the same selector case.
For bool selectors, the inferred value is the matching bool literal.

Read the active arm by matching the field value. The pattern arm names are the
same names declared in the `union by` field, and the payload binding has the
payload type declared for that arm:

```ari
fn payload_value(packet: TLSCiphertext) -> i64 {
  return match packet.fragment {
    stream(stream_payload) => stream_payload.value,
    block(block_payload) => block_payload.value,
    aead(aead_payload) => aead_payload.value,
  };
}
```

Bool selector payloads are matched with `false(payload)` and `true(payload)`.
When code matches either the `union by` field or the selector field itself, the
matching arm proves the active payload arm. Inside that arm, direct payload-slot
projection is allowed and the projected slot has the payload type declared for
that arm:

```ari
fn payload_value(packet: TLSCiphertext) -> i64 {
  return match packet.security.cipher_type {
    stream => packet.fragment.0.value,
    block => packet.fragment.0.value,
    aead => packet.fragment.0.value,
  };
}
```

The selector match can pass through wrapper structs as long as the selector
path still reaches the same stored `union by` field:

```ari
struct Envelope {
  packet: TLSCiphertext,
}

fn payload_value(envelope: Envelope) -> i64 {
  return match envelope.packet.security.cipher_type {
    stream => envelope.packet.fragment.0.value,
    block => envelope.packet.fragment.0.value,
    aead => envelope.packet.fragment.0.value,
  };
}
```

Outside a matching arm, do not read payload storage slots directly with
tuple-index syntax such as `packet.fragment.0`: that bypasses the selector
proof. The compiler rejects unproven direct payload-slot projection for
`union by` fields, and it keeps that restriction when the field is first bound
to a local alias, including a destructuring pattern alias:

```ari
let fragment = packet.fragment;
// rejected: use match fragment { stream(payload) => ... }
let raw = fragment.0;

let TLSCiphertext { fragment: destructured, .. } = packet;
// rejected for the same reason
let raw_again = destructured.0;
```

Match the field or alias and use the payload binding from the matching arm, or
project the alias inside the arm after the alias itself has been matched:

```ari
let fragment = packet.fragment;
let value = match fragment {
  stream(_) => fragment.0.value,
  block(_) => fragment.0.value,
  aead(_) => fragment.0.value,
};
```

A local alias of a `union by` field also cannot be reassigned: it is a
proof-carrying view of the original field, not an independent enum variable.
Rebuild the whole source struct when the selector and payload should change.

Selector fields and their linked payload fields are stable after a value has
been built. Direct assignment to the selector path, to an ancestor of that
path, or to the `union by` field itself is rejected because it could make the
stored payload arm disagree with the discriminant:

```ari
packet.security.cipher_type = block; // rejected
packet.security = SecurityParameters { cipher_type: block }; // rejected
packet.fragment = packet.fragment; // rejected
```

Replace the whole struct value instead, using a selector and a `union by`
constructor arm that agree:

```ari
packet = TLSCiphertext {
  content_type: application_data,
  version: tls12,
  length: 9 as u16,
  security: SecurityParameters { cipher_type: block },
  fragment: block => GenericBlockCipher { value: 99 },
};
```

This is intentionally still an early executable slice: non-enum selector
policies, selector inference through intermediate structs that need unrelated
required fields, active-arm mutation beyond direct reconstruction, active-arm
drop diagnostics, and stable ABI naming remain compiler work. The supported
constructor forms either infer the selector or check a statically visible
selector, so they do not create a value whose payload arm disagrees with the
discriminant. Use ordinary `enum` ADTs when the discriminant is not an existing
product field or when the type must be part of a stable public ABI.

The compiler capability inventory tracks this as `union-by-fields`.

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
