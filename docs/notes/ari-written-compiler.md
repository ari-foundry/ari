# Ari-Written Compiler

This note tracks the Ari-written compiler source root. It is the place to keep
bootstrap status, small next steps, and known language blockers without folding
that work into the current C++ hosted compiler roadmap.

## Purpose

The Ari-written compiler is the start of a future compiler implementation
written in Ari itself. It is intentionally small right now: the first goal is a
checked source skeleton that models compiler-shaped data honestly in the Ari
language that exists today.

The existing C++ compiler remains stage0. It builds `build/ari`, checks the new
Ari source, and remains the production compiler while the Ari-written compiler
grows.

## Source Layout

`compiler/` is the Ari-written compiler source root:

```text
compiler/
  main.ari
  source.ari
  token.ari
  diagnostic.ari
  lexer.ari
  ast.ari
  parser.ari
```

The directory is source-only. It should contain Ari source files, not Markdown
planning files, generated artifacts, or test output. There is no `compiler/src/`;
new bootstrap source should continue to live directly under `compiler/` unless a
future checked package layout explicitly changes this rule.

Do not create `bootstrap/`, `stage1/`, or `docs/compiler-bootstrap/` for this
work. Keep planning here.

## Docs Boundary

`docs/dev/` documents the current C++ hosted compiler: its architecture, pass
contracts, diagnostics, artifacts, tests, and implementation roadmap. Use those
docs when changing `src/*.cpp`, `src/*.hpp`, the driver, LLVM codegen, or hosted
compiler tests.

The Ari-written compiler source work lives in `compiler/`. Its planning lives in
this note. When hosted compiler behavior blocks the Ari-written compiler, record
the blocker here and then use the relevant `docs/dev/` page to fix the hosted
compiler feature in the normal focused-test workflow.

## Current Status

- `compiler/` has been started as a direct Ari source root.
- The initial files model source spans, token kinds, diagnostics, and a tiny
  one-character lexer classification path.
- `compiler/lexer.ari` now has a small `LexResult` flow for one-character scans
  that can return either a token or a shared `diagnostic::Diagnostic`
  invalid-character failure.
- `compiler/lexer.ari` has a tiny `TokenCursor` shape over one scanned token
  and an EOF advance path. It is a placeholder for phase handoff, not a real
  source-text stream.
- `compiler/lexer.ari` exposes small cursor accessors for the current token
  span, token score, and done state.
- `compiler/lexer.ari` has a minimal `TokenHandoff` shape that carries one real
  token cursor plus an explicit EOF observation.
- `compiler/lexer.ari` exposes token-kind query helpers at the cursor and
  handoff boundary so later parser work does not need to import token internals.
- `compiler/ast.ari` now models minimal span-carrying token, statement, error,
  and missing output nodes.
- `compiler/parser.ari` exists as a phase-boundary skeleton that consumes the
  lexer handoff shape, can classify the current handoff token, and returns
  either a statement-shaped `ast::Node` over the current token span or a shared
  diagnostic failure.
- `compiler/main.ari` imports the sibling modules and exercises their public
  surfaces with a minimal smoke path.
- `make check-ari-compiler-bootstrap` checks each `compiler/*.ari` module,
  checks a small `tests/cases/ari-compiler-bootstrap/` fixture with
  `-Icompiler`, and, when an LLVM driver is available, builds and runs the
  source-root smokes.
- Each module is kept small enough to check directly with the stage0 compiler.
- No Ari-written AST, semantic checker, IR, codegen, driver, or file loader
  exists yet beyond the minimal parser-output node model.

## Incremental Roadmap

1. Keep the five-file source skeleton checking with stage0.
2. Grow `lexer.ari` from a one-token cursor into a small token stream over the
   source representation Ari can support today.
3. Grow `parser.ari` only as token handoff proves more parser-shaped data.
4. Add `ast.ari` and later phase-shaped model files only when the
   previous phase has checked output worth passing forward.
5. Add source loading and real text buffers after runtime strings, slices, and
   file IO are strong enough for compiler input.
6. Add semantic model stubs before any lowering or backend work.
7. Continue to use the C++ hosted compiler as stage0 until the Ari-written
   compiler can check meaningful multi-file inputs itself.

## Phase Architecture

Do not blindly copy the current C++ hosted compiler architecture. In
particular, do not recreate a giant `sema`-style module that owns module
loading, name lookup, type inference, type checking, ownership, lowering, and
IR generation together.

Prefer compiler phase and responsibility boundaries:

- `source` / `span` / source identity
- `diagnostic`
- `token`
- `lexer`
- `parser`
- `ast`
- `module_loader`
- `name_resolve`
- `type_infer`
- `type_check`
- `ownership`
- `hir` or another checked tree
- `lower`
- `ir`
- `codegen`
- `driver`
- later package/build integration

The early files can stay flat under `compiler/` while the project is tiny.
When the file count grows, organize by Ari module structure and phase ownership,
not by recreating the C++ source layout.

Keep these phase boundaries explicit:

- `module_loader` should load and identify source units, not perform type
  checking.
- `name_resolve` should resolve names and imports, not silently become the whole
  semantic pipeline.
- `type_infer` should be separate from final `type_check` validation where the
  language can express that cleanly.
- `ownership` should become an explicit phase boundary once checked trees can
  carry enough ownership facts.
- `lower` should translate checked forms to IR so codegen does not re-resolve
  source-level names.
- `codegen` should consume lowered/typed forms instead of owning frontend
  decisions.
- `driver` should orchestrate phases, options, and artifacts, not contain core
  compiler logic.

Also avoid the opposite mistake: do not split the compiler around tiny
container or utility concepts such as `vec`, `box`, or `map`. Those are library
and data-structure details, not compiler phase boundaries.

Phase outputs should be explicit enough that future tooling, LSP support,
package-manager integration, and self-host debugging can inspect them.

## Future Package Manager Transition

The current `compiler/` layout is an early bootstrap source-root layout. It is
not pretending to be the final package-manager-era project shape.

A future Ari package manager or build tool may reorganize:

- compiler module layout and package boundaries
- build metadata and target selection
- test discovery and fixture grouping
- dependency resolution and module search paths
- artifact naming, cache locations, and generated-output policy
- integration points for LSP, formatting, and self-host debugging

Do not design or add a cargo-like tool in this bootstrap slice. Keep the current
work compatible with the existing stage0 compiler, normal file-backed modules,
and focused Make targets. When a package manager exists, migrate from the flat
`compiler/*.ari` source root deliberately instead of growing hidden package
policy in ad hoc compiler files.

## Completed Tasks

- Started the direct `compiler/` Ari source root with `main`, `source`,
  `token`, `diagnostic`, and `lexer` modules.
- Consolidated bootstrap planning in this note and linked it from the docs
  index.
- Documented phase-oriented architecture, future package-manager transition
  points, and the rule against a giant `sema` module.
- Added one-character `LexResult` scanning and switched lexer failures to the
  shared `diagnostic::Diagnostic` payload.
- Added a tiny one-token `TokenCursor` shape as a checked lexer handoff model.
- Added a minimal `parser.ari` phase-boundary skeleton with success and
  diagnostic failure paths.
- Added a focused Ari compiler bootstrap test target and
  `tests/cases/ari-compiler-bootstrap/` source-root smoke fixture.
- Added a minimal `ast.ari` node model and connected parser success results to
  it.
- Added cursor token accessors and a minimal statement-shaped parser output
  node.
- Added a minimal `TokenHandoff` carrying one real token plus EOF, and routed
  `parser::parse_one` through that handoff.
- Added token-kind query helpers for the lexer/parser boundary and a tiny parser
  handoff classification score.

## Small Task Queue

- Use the token-kind query helpers to give parser skeletons a small
  non-statement diagnostic branch for whitespace or unknown inputs.
- Keep `compiler/main.ari` as a small integration smoke, not a real driver.

## Next Recommended Task

Use the token-kind query helpers to give `parser.ari` a small diagnostic branch
for whitespace or unknown handoff inputs. Do not implement source text scanning
or expression parsing yet; keep the next step limited to one parser boundary
decision.

## Local Validation

For the initial skeleton, use focused checks only:

```sh
make
./build/ari compiler/main.ari --check
./build/ari compiler/source.ari --check
./build/ari compiler/token.ari --check
./build/ari compiler/diagnostic.ari --check
./build/ari compiler/lexer.ari --check
./build/ari compiler/ast.ari --check
./build/ari compiler/parser.ari --check
make check-ari-compiler-bootstrap
make check-compiler-docs
make check-bootstrap-readiness
```

For a later Ari source-only change under `compiler/`, prefer:

```sh
make
./build/ari compiler/<changed-file>.ari --check
./build/ari compiler/main.ari --check
make check-ari-compiler-bootstrap
make check-bootstrap-readiness
```

For docs-only changes, run:

```sh
make check-compiler-docs
```

Do not run full `make check` for ordinary bootstrap slices.

## Known Blockers

- Runtime strings and richer text/slice operations are still not enough for real
  compiler source input.
- File-backed module and project flow exists in stage0, but the Ari-written
  compiler has no own source loader or driver.
- Generic aggregate/type monomorphization and trait dispatch are still growing,
  so keep data models simple and checked.
- General iterator support beyond compiler-known `range` is not ready.
- Raw pointer operations, allocation-zone diagnostics, and explicit allocation
  policies are still hosted-compiler roadmap work.
- There is no full AST, HIR, ownership model, IR, or LLVM backend in Ari yet.

## Stage0 Host Compiler Follow-Ups

Confirmed host compiler bugs from this bootstrap slice: none. The `LexResult`,
shared diagnostic payload, one-token cursor, cursor token accessors, parser
skeleton, minimal token handoff, token-kind query helpers, minimal AST node,
statement output node, and focused Ari compiler bootstrap test target checked
without requiring a hosted compiler fix.

When Ari-written compiler work exposes behavior that looks wrong in the current
C++ hosted compiler, keep it separate from the Ari-written compiler task list.
Record the smallest Ari repro, expected behavior, actual behavior, focused
check target, and whether it belongs in parser, modules, generics, ownership,
codegen, diagnostics, or another hosted compiler area.

Desired stage0 pressure that is not yet classified as a bug:

- Better runtime strings, slices, and file IO for real source input.
- Stronger aggregate/type monomorphization for compiler-shaped models.
- Clearer cross-module type identity ergonomics for shared phase models; this
  slice keeps AST constructors scalar at module boundaries instead of passing a
  `source::Span` across nested import paths.
- Clearer ownership-phase ergonomics for checked trees and payload movement.
- More general iterator support beyond compiler-known `range`.
