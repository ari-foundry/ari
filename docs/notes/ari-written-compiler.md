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

The early compatibility target is behavior parity with the C++ stage0 compiler
for the currently supported language surface. Ari-written compiler phases should
match stage0 success/failure decisions, result codes, and diagnostic identity
unless a stage0 behavior is isolated and recorded as a hosted compiler bug.

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
  driver.ari
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

The Ari-written compiler source work lives in `compiler/`. Its bootstrap
planning starts in this note and continues in the split-out notes linked below.
When hosted compiler behavior blocks the Ari-written compiler, record the
blocker in the follow-up note and then use the relevant `docs/dev/` page to fix
the hosted compiler feature in the normal focused-test workflow.

## Working Rules

- Do not make bootstrap design or bug judgments from memory. Inspect the actual
  repository structure, Ari source, tests, stdlib APIs, and stage0 behavior
  before recording a conclusion or choosing an implementation direction.
- Assume `lib/std` is available to Ari-written compiler code and use it first.
  When a capability that belongs in a normal stdlib is missing, awkward, or
  behaves like a bug, record the exact friction or smallest repro separately
  and fix that stdlib or hosted-compiler issue deliberately instead of silently
  working around it.
- Record implementation friction while coding. The point is to know when lexer
  progress is exposing host compiler or stdlib work that should be fixed before
  the Ari-written compiler grows larger.

## Temporary Split

The detailed bootstrap handoff has been split into smaller temporary notes under
`docs/notes/`. This is intentional even when repeatable bootstrap prompts say to
use this Ari-written compiler note as the handoff document: start here, then
follow the links below for the authoritative details.

These split-out bootstrap notes are temporary. Once the Ari-written compiler is
complete enough to stop needing this bootstrap ledger, remove or fold these notes
into the permanent documentation set.

## Detailed Bootstrap Notes

- [Status](ari-written-compiler-status.md): current implemented surface and
  source-root state.
- [Roadmap](ari-written-compiler-roadmap.md): incremental roadmap, phase
  architecture, and package-manager transition notes.
- [Tasks](ari-written-compiler-tasks.md): completed task log, small task queue,
  and next recommended task.
- [Validation And Follow-Ups](ari-written-compiler-validation.md): local checks,
  known blockers, confirmed host compiler bugs, and desired stage0 pressure.

## Current Status

See [Ari-Written Compiler Status](ari-written-compiler-status.md#current-status).

## Incremental Roadmap

See [Ari-Written Compiler Roadmap](ari-written-compiler-roadmap.md#incremental-roadmap).

## Phase Architecture

See [Ari-Written Compiler Roadmap](ari-written-compiler-roadmap.md#phase-architecture).
Do not blindly copy the current C++ hosted compiler architecture.

## Future Package Manager Transition

See [Ari-Written Compiler Roadmap](ari-written-compiler-roadmap.md#future-package-manager-transition).

## Completed Tasks

See [Ari-Written Compiler Tasks](ari-written-compiler-tasks.md#completed-tasks).

## Small Task Queue

See [Ari-Written Compiler Tasks](ari-written-compiler-tasks.md#small-task-queue).

## Next Recommended Task

See [Ari-Written Compiler Tasks](ari-written-compiler-tasks.md#next-recommended-task).

## Local Validation

See [Ari-Written Compiler Validation And Follow-Ups](ari-written-compiler-validation.md#local-validation).
The focused bootstrap target remains `make check-ari-compiler-bootstrap`, and
fixtures remain under `tests/cases/ari-compiler-bootstrap/`.

## Known Blockers

See [Ari-Written Compiler Validation And Follow-Ups](ari-written-compiler-validation.md#known-blockers).

## Stage0 Host Compiler Follow-Ups

See [Ari-Written Compiler Validation And Follow-Ups](ari-written-compiler-validation.md#stage0-host-compiler-follow-ups).
Confirmed host compiler bugs from this bootstrap slice are tracked there; the
current status remains none unless that page records a concrete repro.
