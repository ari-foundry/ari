# Ari-Written Compiler Roadmap

This is a temporary split-out note for the Ari-written compiler bootstrap. It
keeps the large handoff content out of the main index while the Ari-written
compiler is incomplete. When the Ari-written compiler is complete, these
temporary bootstrap notes can be removed or folded into permanent docs.

Back to [Ari-Written Compiler](ari-written-compiler.md).

## Incremental Roadmap

1. Keep the five-file source skeleton checking with stage0.
2. Grow `lexer.ari` from a one-token cursor into a small token stream over the
   source representation Ari can support today.
3. Grow `parser.ari` only as token handoff proves more parser-shaped data.
4. Add `ast.ari` and later phase-shaped model files only when the
   previous phase has checked output worth passing forward.
5. Grow source loading from the current `std::string::String` / `Slice[u8]`
   text path into a source table with source ids, file paths, and diagnostic
   locations.
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
