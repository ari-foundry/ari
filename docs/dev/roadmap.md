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

Structural capability parameters are worth exploring after the current trait
and diagnostic path is solid:

```ari
fn save(x: has serialize() -> String) {
    file.write(x.serialize())
}
```

This is only a roadmap idea. It must not add an `interface` keyword, dynamic
dispatch by accident, or a shortcut around normal trait-bound diagnostics.

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
