# Compiler Module Project Authoring

This page explains how to change Ari's file-backed module and project behavior.
It is ordinary hosted-compiler development.

Compiler-sized Ari programs need predictable files, roots, module paths,
metadata, and cache behavior. The same rules should help ordinary tools,
servers, formatters, and package utilities.

## Current Contract

The current module contract is intentionally small:

- `mod name;` loads a file-backed child module.
- The importing file's directory is searched first.
- `-I path` and `--module-path path` add search roots in command-line order.
- Candidate files are `name.ari`, `name.arih`, `name/mod.ari`, and
  `name/mod.arih`.
- A file-backed alias such as `mod source as Source;` searches for `source`
  but exposes declarations under `Source`.
- A resolved source file has one module identity per compiler invocation.
  Re-importing the same file as a different module is rejected.
- Cyclic imports are rejected by module identity and by currently-loading
  source path, so nested file layouts cannot recurse by spelling the same file
  under another module name.
- Within one search root, duplicate physical layouts such as `name.ari` and
  `name/mod.ari` are rejected as ambiguous module files.
- `.arih` is declaration-like today; it is not yet an automatic pair with a
  matching `.ari` file.
- `std` is special-cased for the source standard library and should not become
  the pattern for ordinary user or compiler projects.

When this contract changes, update [Compiler Project Model](compiler-project-model.md),
module tests, and any artifact docs that mention module graphs or metadata.

## Project Layout Rule

Until Ari has a package manager, large tools should use a boring Makefile and
an explicit module root:

```text
tool/
  Makefile
  src/
    main.ari
    source/mod.ari
    report/mod.ari
    lex/mod.ari
    parse/mod.ari
  tests/
    fixtures/
    golden/
```

The root `main.ari` should stay thin: parse command-line inputs, call modules,
and return a process status. Compiler phases should live behind modules that
own one concept each.

## Module Ownership

Use module boundaries to make reviews smaller:

| Module Kind | Owns | Should Avoid |
| --- | --- | --- |
| `source` | source ids, files, spans, line tables | type checking |
| `report` | diagnostic data and rendering | lexer tokenization |
| `lex` | characters, tokens, token spans | AST semantics |
| `parse` | grammar, AST, syntax recovery | trait dispatch |
| `resolve` | module paths, imports, names | LLVM output |
| `hir` | lowered tree vocabulary | source file loading |
| `ir` | typed resolved facts | user-facing parsing |
| `emit` | LLVM/object/shared artifacts | source-level name lookup |

If two modules need each other's private helpers, extract a smaller lower-level
module instead of creating a cycle.

## Diagnostics

Module diagnostics should name the facts a user can fix:

- importing source file
- requested module name
- active package root
- ordered module search paths
- candidate files that were checked
- visibility boundary when a private item is used
- metadata or cache input that made a cache stale

Do not report only `module not found` when the compiler already knows which
paths were searched.

## Coverage Inventory

The production-ready module subset is intentionally narrow and tested:

| Area | Status | Proof |
| --- | --- | --- |
| Inline modules and nested visibility | complete | `make check-modules` inline, file-backed bridge, and private diagnostics |
| File-backed sibling modules | complete | `file-module-main.ari`, `file-module-alias-main.ari` |
| Package-style child directories | complete | `project-compiler-main.ari` module graph |
| Ordered search paths | complete | `package-module-main.ari` with `--module-path` and `-I` |
| Aliased modules and `use` aliases | complete | alias, grouped use, glob use, and duplicate alias diagnostics |
| Cross-file structs, enums, generics, traits | complete | compiler-shaped project fixture plus `trait-project-main.ari` |
| Diamond-shaped shared modules | complete | `diamond-project-main.ari` imports shared definitions once through a parent facade |
| Cycles and duplicate module identities | complete | `cyclic-import-main.ari`, `self-import-main.ari`, `duplicate-module-file-identity.ari` |
| Ambiguous module candidates | complete | `ambiguous-module-main.ari` |
| Imported-file parse/sema diagnostics | complete | `imported-parse-error-main.ari`, `imported-semantic-error-main.ari` |
| Persistent package manifests | unsupported by design | use Makefiles plus explicit `-I` roots |
| Remote dependencies or registries | unsupported by design | outside the current compiler model |

## Metadata And Cache Inputs

Metadata and cache files are safe only when their invalidation story is clear.
Record every input class that can change meaning:

- cache format version
- compiler format version
- entry file path
- package root
- module search paths
- target triple and ABI facts
- active cfg features
- resolved module files
- source hashes
- public declaration summaries
- imported type/layout summaries when reused

A stale cache diagnostic should name the first mismatched input class. Silent
cache reuse is worse than no cache.

Use `ari --target-info` before debugging target-specific module or cache
behavior. It prints the resolved triple, ABI sizes, and active
`target("...")` predicates without requiring a source file. Keep those
predicates visible in module/cache handoff notes so the target-dependent
surface is easy to search.

## Artifact Policy

Prefer reviewable artifacts before executable checks:

| Change | First Artifact |
| --- | --- |
| Search path behavior | `--emit-module-graph` |
| Public item surface | `--emit-declaration-index` |
| Metadata format | `--emit-module-metadata` |
| Cache validation | `--emit-module-cache` and `--use-module-cache` fixtures |
| Missing module diagnostic | `--emit-diagnostics` |
| Visibility diagnostic | `--emit-diagnostics` |

Use executable tests only after the module graph or diagnostic already proves
the project layout behavior.

## Focused Checks

Use the smallest command that proves the change:

```sh
build/ari tests/cases/modules/ok/file-module-main.ari --check
build/ari --target x86_64-pc-linux-gnu --target-info
build/ari tests/cases/modules/ok/file-module-main.ari --emit-module-graph build/focused/module.graph
build/ari tests/cases/modules/errors/missing-module.ari --emit-diagnostics build/focused/missing.diagnostic
make check-modules
make check-compiler-artifacts
```

Full `make check` belongs at handoff for broad changes. Sanitizer checks are
intentionally separate.

## Test Placement

Use these folders:

- `tests/cases/modules/ok/` for valid module behavior
- `tests/cases/modules/errors/` for rejected module behavior
- `tests/packages/` for alternate module roots and package fixtures
- `tests/cases/compiler-development/artifact/ok/` for module graph and
  declaration index goldens
- `tests/cases/compiler-development/artifact/errors/` for module diagnostic
  goldens
- `tests/cases/compiler-development/ok/model/` for planning/model fixtures

Name tests after behavior: `module-missing-candidates`,
`module-private-item`, `module-graph-file-module`,
`module-graph-project-compiler`, `module-inline-file-bridge`,
`module-ambiguous-candidate`, `module-cache-source-hash`, or
`module-metadata-visibility`.

## Review Checklist

Before handing off module/project work, answer:

- Does the change help ordinary Ari projects?
- Are package roots and search paths visible in diagnostics or artifacts?
- Is `.arih` behavior described honestly without implying automatic pairing?
- Does metadata record every input that can change meaning?
- Does stale cache behavior fail closed?
- Is `--emit-module-graph`, `--emit-declaration-index`, or
  `--emit-diagnostics` the first proof?
- Is the focused check documented in the handoff note?

Good module/project behavior makes large Ari programs boring to build and
debug. That is exactly what a future compiler project will need, but the work
belongs to the general language and compiler today.
