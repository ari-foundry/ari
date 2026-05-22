# Compiler Next Slices

This page lists the next practical compiler-development slices for Ari.

It is intentionally about the hosted compiler and public language quality.
These slices should make Ari easier to use, easier to debug, and easier to
maintain.

Read this after [Compiler Implementation Playbook](compiler-implementation-playbook.md)
when you want to pick the next small ticket.

## Selection Rules

Choose a slice when it satisfies all of these:

- It improves ordinary Ari compiler behavior.
- It can be proven by one focused fixture or artifact.
- It makes large Ari tools less special, not more special.
- It does not add private compiler-only syntax, hidden allocation, or runtime `std`
  source-map APIs.

## Next Slices

| Order | Slice | First Files | Test Shape | Done When |
| --- | --- | --- | --- | --- |
| 1 | Source span edge cases | `src/source_map_dump.cpp`, source loading, lexer span creation | Add one `--emit-source-map` golden and one lexer/parser diagnostic golden. | CRLF, final-newline, empty-file, and multi-file line lookup stay deterministic. |
| 2 | Diagnostic code expansion | `src/diagnostic_dump.cpp`, lexer/parser/module/sema diagnostics | Add one `artifact/errors/diagnostic-*.diagnostic` per diagnostic family. | Expected failures show stable code families before renderer wording changes. |
| 3 | Module metadata reviewability | `src/module_metadata.cpp`, `src/module_graph_dump.cpp`, module loader | Add focused metadata/module-graph fixtures. | Imports, visibility, generic impl surfaces, and stale-cache errors can be reviewed from text. |
| 4 | Compiler-shaped aggregate pressure | type/layout/trait/enum payload helpers | Add `compiler-development/ok/model/` fixtures. | Tokens, diagnostics, source maps, and pass states use normal Ari structs/enums/results without awkward sentinels. |
| 5 | HIR artifact sketch | `src/ir.hpp`, sema lowering helpers, future HIR dump code | Add a small HIR text format before a large pass rewrite. | Reviewers can inspect lowered source facts before typed IR and LLVM output. |
| 6 | IR metadata audit | `src/ir.hpp`, `src/ir_builders.cpp`, `src/llvm_codegen.cpp` | Add typed-IR or LLVM text checks for the moved fact. | Backend codegen stops rediscovering source names, visibility, or layout choices. |

## Ticket Breakdown

Each slice should be split into these steps:

1. Add or update the smallest fixture first.
2. Make the compiler emit the artifact or diagnostic needed by that fixture.
3. Update the focused docs page.
4. Run the narrow target.
5. Only then consider broader checks.

Example for source span work:

```text
Fixture:
  tests/cases/compiler-development/artifact/ok/source-map-line-edge.map

Compiler:
  source loader and source map dump only

Docs:
  compiler-source-diagnostics.md

Check:
  make check-compiler-artifacts
```

## Not In This Slice List

Do not start these until earlier artifacts are stable:

- a full second compiler implementation
- a sema port
- optimizing backend work

This page is about the compiler-development work that should happen before
large rewrites become a good idea.

## Readiness Impact

Completing these slices should move Ari beyond the current **46-47% compiler
maturity** range by improving deterministic source identity, diagnostics,
modules, generic data modeling, HIR/IR visibility, and backend mechanicalness.
