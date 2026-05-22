# Compiler Project Model

This page defines the project and file-backed module model Ari needs for large
compiler work. It is compiler-development infrastructure for ordinary Ari
tools.

Use this page with [Compiler Development Roadmap](compiler-development-roadmap.md),
[Compiler Maturity Gates](compiler-maturity-gates.md),
[Compiler Module Project Authoring](compiler-module-project-authoring.md),
[Compiler Source And Diagnostics](compiler-source-diagnostics.md),
[Compiler Artifact Testing](compiler-artifact-testing.md), and
[Production Compiler Design](production-compiler-design.md).

## Goals

The project model should make Ari multi-file tools boring to build, inspect,
and debug:

- file-backed modules resolve the same way every time
- package roots are explicit enough for tools and Makefiles
- module search paths are ordered and visible in diagnostics
- visibility and `pub use` rules keep module boundaries reviewable
- metadata and module cache files record exactly what made them valid
- build targets remain small until a future package manager exists

This is not a special dialect for a compiler. It is the same shape a formatter,
linter, package tool, or server program should be able to use.

## Current Module Contract

The current compiler already has a useful file module surface:

- `mod name;` loads a file-backed child module.
- Lookup first checks the importing file's directory.
- Then lookup checks paths from `-I` and `--module-path` in command-line order.
- Candidate files are tried as `name.ari`, `name.arih`, `name/mod.ari`, and
  `name/mod.arih`.
- `std` is special-cased for the source standard library and should not be the
  model for ordinary compiler projects.
- `.arih` is a header-like convention today, not an automatic pair with a
  matching `.ari` implementation file.
- File-backed aliases search by the local file name but expose the alias as
  the module identity.
- A resolved source file is allowed only one module identity during a compiler
  invocation.
- Cyclic import detection checks both module names and currently-loading source
  paths.

Missing-module diagnostics show the module name, importing source file, ordered
module search paths, and candidate paths that failed. Module graph artifacts
show resolved source paths, import edges, public/private item surfaces, target,
cfg features, and search paths. That makes project layout bugs fixable without
reading `src/module_loader.cpp`.

## Recommended Project Layout

Before Ari has a package manager, a serious Ari tool should use a Makefile and
an explicit `src/` module root:

```text
tool/
  Makefile
  src/
    main.ari
    source/
      mod.ari
    report/
      mod.ari
    lex/
      mod.ari
    syntax/
      mod.ari
    parse/
      mod.ari
    hir/
      mod.ari
    resolve/
      mod.ari
  tests/
    fixtures/
    golden/
```

The root file should stay thin. It should import modules, wire the command-line
entry point, and call into smaller packages. Large compiler phases should live
behind module boundaries so tests can exercise them without running the whole
program.

## Module Ownership Rules

Use these rules for compiler-sized Ari projects:

- A module owns one concept: source files, diagnostics, lexing, parsing, names,
  types, lowering, or artifact emission.
- A module may expose a small public facade with `pub` declarations and keep
  helper data private.
- `pub use` should be used as a deliberate facade, not as a way to flatten the
  whole project.
- Cycles should be treated as design pressure. Shared definitions belong in a
  smaller lower-level module.
- Test-only helper modules should be named as test support and not imported by
  production modules.
- Generated artifacts should be written under `build/` or a test output folder,
  never beside source files.

This keeps compiler code readable when the project grows from a lexer to syntax
trees, HIR, type checking, and artifact emission.

## Header And Source Policy

Today, `.arih` means "source surface intended to be declaration-like". It does
not automatically pair with a `.ari` file, and lookup chooses one resolved file.
That is good enough for the current compiler and standard library, but large
projects need a clearer policy before they depend on it.

Future `.arih` / `.ari` pairing should:

- keep public declarations in `.arih` and implementation bodies in `.ari`
- validate that declarations and implementations agree
- record both files in metadata and module cache inputs
- reject duplicate public items across the pair with clear diagnostics
- allow projects to use only `.ari` when a header split is unnecessary

Until that exists, prefer ordinary `.ari` modules for compiler project code and
reserve `.arih` for stable external-facing surfaces.

## Metadata And Cache Policy

Module metadata and module cache files are only useful if their invalidation
rules are easy to explain. The existing flags `--emit-module-metadata`,
`--check-module-metadata`, `--emit-module-cache`, and `--use-module-cache`
should converge on one contract.

`--emit-module-graph` is the review-friendly companion artifact. It does not
replace metadata or cache files; it writes the resolved source files, imports,
and visible item surfaces in deterministic text so a project-layout change can
be reviewed before sema or LLVM output are involved.

Each metadata or cache artifact should include:

- cache format version
- Ari compiler version or compatible format id
- package root
- importing file path
- resolved module file path
- module search paths from `-I` and `--module-path`
- active cfg features and target facts
- source hashes for all imported module files
- import graph edges
- public declaration summary
- type and layout summaries needed by downstream modules
- lowered AST or IR summaries only when the cache mode actually reuses them

When a cache is stale, diagnostics should name the first changed input class:
source hash, search path, cfg feature, target fact, compiler format version, or
missing dependency. A stale cache should never silently change program meaning.

Use `ari --target-info` before debugging target-specific cache or module
behavior. It prints the resolved triple, architecture, pointer and C `long`
widths, signed `char` policy, and active `target("...")` predicates without
requiring a source file.

## Build Flow

Makefiles are the right bridge until Ari has a package manager. A focused tool
Makefile should keep targets small and explicit:

```makefile
ARI ?= ../../build/ari
SRC := src/main.ari
OUT := build/tool

check:
	$(ARI) $(SRC) --check -I src

llvm:
	$(ARI) $(SRC) --emit-llvm build/tool.ll -I src

build:
	$(ARI) $(SRC) -o $(OUT) -I src

check-fixtures: build
	$(OUT) tests/fixtures/sample.ari > build/sample.out
	diff -u tests/golden/sample.out build/sample.out
```

Do not hide module paths in environment variables. The command line should show
which roots are part of the build so failures can be reproduced from logs.

## Implementation Slices

These slices define the supported production subset:

1. Project roots: the entry file directory plus explicit `-I` and
   `--module-path` roots define lookup. Manifest inference is unsupported.
2. Module path diagnostics: missing modules report checked candidates and
   ordered search paths.
3. Visibility diagnostics: private functions, constants, enums, cases, structs,
   and nested modules are rejected with source locations.
4. Cache validation: metadata/cache checks fail closed for stale source hash,
   cfg, target, search path, format, missing import, and item changes.
5. Header/source policy: `.ari` and `.arih` are alternative candidates today;
   automatic pairing remains unsupported by design.
6. Multi-file tool smoke: `project-compiler-main.ari` builds a small
   source/diagnostic/symbol/parser project through file-backed child modules.
7. Module graph dump: deterministic `--emit-module-graph` artifacts capture
   source files, import edges, traversal-independent item order, visibility,
   search paths, cfg features, and target.

The module graph dump is especially useful because it gives reviewers a compact
view of imports, resolved files, visibility edges, and eventually cache hits.

## Test Layout

Project/module tests should be grouped by behavior:

```text
tests/cases/modules/ok/
tests/cases/modules/errors/
tests/cases/modules/cache/
tests/packages/
tests/compiler-projects/
```

Future large-tool fixtures can use this shape:

```text
tests/compiler-projects/simple-tool/
  Makefile
  src/main.ari
  src/source/mod.ari
  src/report/mod.ari
  tests/fixtures/
  tests/golden/
```

Name tests after the behavior they prove:

- `module-missing-candidates.ari`
- `module-private-item.ari`
- `module-cache-source-hash.ari`
- `module-cache-search-path.ari`
- `module-graph-dump.ari`
- `module-graph-project-compiler.ari`
- `module-cyclic-import.ari`

## Review Checklist

Before accepting project/module changes, check:

- Does the diagnostic name the importing file and module path?
- Are package roots and module search paths visible in failing output?
- Does cache metadata record every input that can change meaning?
- Can the feature be tested with `build/ari path/to/file.ari --check` or a
  small Make target?
- Does the change help ordinary Ari projects, not only a future compiler tree?
- Are `.arih` rules documented honestly instead of implying automatic pairing?

## Readiness Impact

This work removes hidden assumptions around files, roots, imports, and cache
reuse, and it feeds the compiler-development maturity estimate by turning
multi-file projects from a planned surface into a tested compiler capability.

The current model is ready for ordinary multi-file Ari tools that use explicit
roots and Makefile-style build commands. Remaining project-management features
such as manifests, remote dependency fetching, registry support, and automatic
`.arih`/`.ari` pairing are intentionally outside this compiler subset.
