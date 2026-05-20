# Compiler Bootstrap Fixture Plan

This page defines the pre-bootstrap fixtures Ari should add before a real
`bootstrap/` compiler tree exists.

The fixtures are not a hidden stage1 implementation. They are normal Ari programs
and expected-output files that prove the public language can express
compiler-shaped code. When a fixture exposes awkward syntax, brittle
diagnostics, or a missing compiler feature, fix the normal language/compiler
surface instead of adding a private bootstrap shortcut.

Read this with [Production Compiler Design](production-compiler-design.md),
[Bootstrap Readiness](bootstrap-readiness.md), and
[Self-Host Roadmap](self-host-roadmap.md).

## Goal

The first useful milestone is not self-compilation. The first useful milestone
is confidence that Ari can build and test small compiler tools in the same way
ordinary Ari applications are built and tested.

Pre-bootstrap fixtures should answer these questions:

- Can Ari model source files, byte spans, tokens, syntax nodes, diagnostics,
  symbols, and typed facts with public structs, enums, generics, traits, and
  zones?
- Can compiler code use readable domain types such as `SourceId`, `Span`,
  `TokenKind`, `Result[T, E]`, and `char` without numeric encoding tricks?
- Can focused tests compare deterministic text artifacts without running a
  whole self-hosting pipeline?
- Can a new contributor understand each fixture from its folder, filename, and
  expected artifact type?

## Placement

Do not create the real `bootstrap/` tree until the lexer pilot starts. Before
that, keep readiness fixtures under the normal test tree:

```text
tests/cases/bootstrap-readiness/
  ok/
    modules/
    source/
    model/
    generics/
    traits/
    errors/
    zones/
    formatting/
    artifacts/
  errors/
    modules/
    source/
    model/
    generics/
    traits/
    errors/
    zones/
    formatting/
    artifacts/
  golden/
```

The `ok/` fixtures are Ari programs that should compile with today's compiler.
The `errors/` fixtures are Ari programs that should fail with stable compiler
diagnostics. The `golden/` files are expected text artifacts for future
compiler-shaped tools.

## Fixture Groups

| Group | What It Proves | Example Files |
| --- | --- | --- |
| `modules` | Multi-file package shape, import paths, visibility, and stale module-cache diagnostics. | `modules-package-root.ari`, `modules-private-item.ari` |
| `source` | UTF-8 source ownership, byte span math, line endings, char literals, and source text slices. | `source-span-byte-range.ari`, `source-line-column.ari` |
| `model` | Token, syntax, diagnostic, HIR, and symbol records using structs, enums, tuples, and type aliases. | `model-token-kind.ari`, `model-syntax-node.ari` |
| `generics` | Nested compiler data using vectors, maps, sets, options, results, and generic enum payloads. | `generics-ast-node.ari`, `generics-symbol-map.ari` |
| `traits` | Deterministic selection for debug formatting, equality, ordering, hashing, and drop hooks. | `traits-debug-node.ari`, `traits-hash-key.ari` |
| `errors` | Expected failure flow through `Result[T, E]`, small error structs, and recovery values without panic. | `errors-lex-result.ari`, `errors-parse-recovery.ari` |
| `zones` | Explicit arena policy for source text, tokens, trees, diagnostics, and temporary test data. | `zones-source-map.ari`, `zones-syntax-arena.ari` |
| `formatting` | Named captures, debug formatting, width/base formatting, and buffer-backed artifact output. | `formatting-named-capture.ari`, `formatting-token-dump.ari` |
| `artifacts` | Normalized token, syntax, HIR, typed IR, and LLVM text comparison rules. | `artifacts-token-dump.ari`, `artifacts-normalized-ir.ari` |

## Naming Rules

Use names that explain the behavior, not the implementation detail:

```text
<group>-<feature>.ari
<group>-<feature>.tokens
<group>-<feature>.diag
<group>-<feature>.syntax
<group>-<feature>.hir
<group>-<feature>.ir
```

Examples:

- `source-line-column.ari`
- `model-token-kind.ari`
- `generics-symbol-map.ari`
- `traits-debug-node.ari`
- `artifacts-token-dump.tokens`

Each fixture should exercise one pressure point. If a fixture needs many
unrelated helpers, split it.

## Focused Targets

Keep targets small. Do not make early readiness checks depend on a full
`make check` run.

Current and recommended targets:

```text
make check-bootstrap-docs
make check-bootstrap-readiness
make check-bootstrap-readiness-model
make check-bootstrap-readiness-modules
make check-bootstrap-readiness-artifacts
```

`check-bootstrap-readiness` is the first concrete target. It should stay tiny:
compile a few normal Ari programs that model compiler-shaped data, including
aggregate `Result[Token, LexError]` failure values, then grow only when a new
readiness fixture proves a specific pressure point.

Once the lexer pilot starts and the real `bootstrap/` tree exists, move from
readiness fixtures to stage tool checks:

```text
make -C bootstrap check-lex
make -C bootstrap check-parse
make -C bootstrap check-report
make -C bootstrap check-stage-artifacts
```

## Artifact Policy

Text comparison should start earlier than executable comparison.

Comparison order:

1. token dumps
2. syntax dumps
3. diagnostic reports
4. HIR dumps
5. typed IR dumps
6. LLVM text with normalized paths, temporary names, and symbol ordering
7. executable behavior

Executable comparison is intentionally last. A frontend regression should be
visible in a small textual artifact before it becomes a backend mystery.

## Start Gate

The real `bootstrap/` tree may start when readiness fixtures prove these
properties:

- source text and span values are easy to represent
- compiler diagnostics can be rendered as stable text and kept not in runtime `std`
- compiler-shaped structs, enums, and generic containers compile without
  awkward encoding tricks
- trait selection is deterministic enough for maps, sets, sorting, formatting,
  and cleanup
- focused Make targets can run one artifact family at a time

Until then, keep the work in normal docs, normal tests, and normal compiler
feature improvements.
