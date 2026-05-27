# Compiler Roadmap

This page tracks active compiler work for the current C++ hosted compiler. It
is deliberately short: detailed implementation rules live in the focused docs
linked from [Developer Overview](README.md).

## Scope

The roadmap is for production compiler behavior:

- source identity, spans, snippets, and multi-file diagnostics
- lexer/parser reliability on malformed input
- semantic diagnostics, module flow, traits, generics, ownership, and ABI
- deterministic artifacts that make compiler regressions reviewable

This page is not a self-host implementation plan. Do not use it to create an
Ari-written compiler tree, bootstrap/stage directories, or a cargo-like build
tool. Standard-library/library maturity is tracked separately.
The few stdlib polish items that are blocked by compiler features are mapped in
[Compiler-Bound Standard Library Gaps](compiler-bound-stdlib-gaps.md).

## Near-Term Order

1. Source identity and spans: every source-backed diagnostic and artifact should
   carry stable source ids, byte spans, line/column lookup, snippets, and
   imported-file identity.
2. Diagnostics: common parser, semantic, module, ownership, trait, generic, and
   backend errors should be specific, source-aware, and stable.
3. Module/project flow: nested imports, visibility, duplicate modules, missing
   modules, cycles, same-filename directories, and module graph artifacts should
   be deterministic.
4. Frontend reliability: invalid characters, EOF edges, unterminated strings or
   comments, malformed declarations, malformed expressions, recovery, and syntax
   artifacts should stay stable.
5. Compiler-shaped data readiness: normal Ari structs, enums, matches,
   generics, and ownership should be enough to model tokens, spans, diagnostics,
   symbols, type references, and pass results.
6. Trait and generic readiness: generic functions, generic aggregates, trait
   bounds, trait method resolution, monomorphization keys, and generic
   ownership/drop should work through general compiler paths.
7. Artifact comparison: token, source-map, syntax, diagnostic, module graph,
   declaration index, typed IR, pass summary, LLVM fragment, object/shared
   symbol, and runtime-output artifacts should be deterministic.
8. ABI/backend reliability: primitive, pointer, aggregate, enum, generic
   aggregate, function, object, shared library, and C FFI cases should either
   work in the documented subset or fail with a clear diagnostic.

## Language Ideas Parked For Later

Structural capability parameters now have a narrow executable seed for ordinary
free functions, inherent `impl` methods, trait methods, and trait impl methods:

```ari
fn save(x: has serialize() -> i64) -> i64 {
    x.serialize()
}
```

The parser accepts `has method(...) -> Type` and grouped
`has { method(...) -> Type, other(...) -> Type }` in ordinary free-function,
inherent `impl` method, trait method, and trait impl method parameter type
positions, plus explicit generic bounds such as
`fn save[T: has serialize() -> i64](x: T)`. Non-generic capability aliases such
as `type Serializable = has serialize() -> i64;` and generic aliases such as
`type Mapper[Input, Output] = has map(Input) -> Output;` can be reused as
supported function and method generic bounds. Semantic analysis lowers
anonymous parameters to hidden generics, substitutes alias type arguments into
reusable capability requirements, checks concrete call-site types for every
listed static method, and monomorphizes the body through the ordinary
method-call path. Trait impl conformance now compares structural capability
method signatures exactly, so a trait method and its impl can use direct
`has ...` spelling or an equivalent capability alias but cannot silently change
the requirement. Hidden capability generics do not count as visible method type
arguments. It must continue to avoid an `interface` keyword, accidental dynamic
dispatch, or a shortcut around named trait-bound diagnostics. Remaining
roadmap work includes richer diagnostics for when a named trait is better and
any future extension beyond method requirements.

Discriminant-linked union fields are also worth exploring for protocol and
binary-format records whose payload shape is controlled by data already present
in the surrounding value or in an explicit context value. The intent is the
same modeling niche as a tagged union inside a struct, but with the tag tied to
a real field or context expression instead of inventing a second hidden enum
tag:

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

The spelling is `union by` for the discriminant link and named arms for the
alternatives. The union field's active payload type is determined by the named
discriminant value.

This is no longer syntax-only. The parser builds a `TypeRef` for `union by`
that records the selector path and each arm's payload type, so
syntax/declaration tooling can inspect the shape. Sema validates that the
selector starts from an earlier struct field, nested selector segments resolve
through known struct fields, arm names are unique, arm payload types resolve,
enum selectors use arms that exactly cover the enum cases, and bool selectors
use exactly `false` and `true` arms. For enum and bool selectors, the compiler
lowers the field to hidden enum storage and accepts struct literal construction
with natural `fragment: stream(payload)` / `payload: true(payload)` syntax or
compatibility `fragment: stream => payload`. Struct payload arms also accept the
shorthand `fragment: stream { field: value }`, which uses the arm's declared
payload type as the struct literal type. When the selector value is visible in
the same struct literal, the constructor arm must match it. Direct enum
selector fields such as `kind`, direct bool selector
fields such as `enabled`, and nested selector paths such as
`security.cipher_type` when their omitted intermediate struct value can be
synthesized from the selector alone, may also be omitted and inferred from
any supported `union by` constructor spelling; if multiple union
fields share that omitted selector, their constructor arms must agree.
The field value can also be matched directly with the same arm names, for
example `match packet.fragment { stream(stream_payload) => ... }`; pattern
resolution prefers the subject enum type before global case names so `union by`
arms can share names with the selector enum cases. After construction, direct
assignment to the selector path, an ancestor field, or the `union by` field
itself is rejected; rebuild the whole struct when the discriminant and active
payload must change together.

It should not replace ordinary `enum` ADTs, unchecked C unions, or `match`. A
future design must specify concrete-value policies for non-enum and non-bool
discriminants, public active-arm borrowing/narrowing outside direct field
matches, runtime selector consistency policy beyond direct local assignments,
selector inference through intermediate structs with unrelated required fields,
active-arm drop diagnostics, and stable ABI naming.
The compiler capability inventory tracks this as `union-by-fields`.

## What Not To Track Here

- Ari compiler rewrite tasks
- self-host stage plans
- bootstrap/stage1 directory layouts
- package manager or cargo-like tool work
- standard-library maturity
- broad kernel/freestanding roadmaps unless a narrow compiler ABI/backend fix is
  ready to implement now

## Work Loop

For each roadmap item:

1. Find the implementation path and existing focused tests.
2. Classify the gap as implementation missing, implementation buggy, or
   verification missing.
3. Fix the hosted compiler when behavior is missing or wrong.
4. Add one focused fixture or golden only after the behavior exists.
5. Run the narrow check for that layer.
6. Update the focused doc so the next compiler developer can follow the same
   path.

## Current Non-Goals

- class syntax
- hidden inheritance
- garbage collection
- C++ ABI dependency as a source-level FFI surface
- ambient global heap as a language primitive
- adding a second backend during current 0.x compiler stabilization
