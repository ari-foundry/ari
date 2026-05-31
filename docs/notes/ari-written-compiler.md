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
- `compiler/main.ari` imports the sibling modules and exercises their public
  surfaces with a minimal smoke path.
- Each module is kept small enough to check directly with the stage0 compiler.
- No Ari-written parser, semantic checker, IR, codegen, driver, or file loader
  exists yet.

## Incremental Roadmap

1. Keep the five-file source skeleton checking with stage0.
2. Grow `lexer.ari` from one-character classification into a small result-based
   scanner over the source representation Ari can support today.
3. Add parser-shaped enums and structs only after token flow is stable.
4. Add source loading and real text buffers after runtime strings, slices, and
   file IO are strong enough for compiler input.
5. Add semantic model stubs before any lowering or backend work.
6. Continue to use the C++ hosted compiler as stage0 until the Ari-written
   compiler can check meaningful multi-file inputs itself.

## Small Task Queue

- Add a `LexResult` enum in `lexer.ari` for one-character scans that can return
  either a token or a diagnostic-shaped failure.
- Decide the first shared diagnostic payload that can be used without making
  standalone module checks fragile.
- Add a tiny token cursor once Ari has a source text representation suitable for
  compiler input.
- Keep `compiler/main.ari` as a small integration smoke, not a real driver.

## Next Recommended Task

Add a `LexResult` enum to `compiler/lexer.ari` for one-character scans, with an
invalid-character path carrying a diagnostic code and byte offset. Keep the
change small and verify `compiler/lexer.ari` and `compiler/main.ari` with
`--check`.

## Local Validation

For the initial skeleton, use focused checks only:

```sh
make
./build/ari compiler/main.ari --check
./build/ari compiler/source.ari --check
./build/ari compiler/token.ari --check
./build/ari compiler/diagnostic.ari --check
./build/ari compiler/lexer.ari --check
make check-compiler-docs
make check-bootstrap-readiness
```

For a later Ari source-only change under `compiler/`, prefer:

```sh
make
./build/ari compiler/<changed-file>.ari --check
./build/ari compiler/main.ari --check
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
- There is no parser, AST, HIR, ownership model, IR, or LLVM backend in Ari yet.
