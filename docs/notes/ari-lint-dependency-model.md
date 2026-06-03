# ari-lint Dependency Model

## Purpose

This document records the preferred dependency model for future `ari-lint`
repository split planning.

It does not move files, create repositories, or change build behavior. It is a
decision note for the split plan, not a change to the current toolchain.

## Current Status

- `ari-lint` is currently bundled in the `ari` repository.
- `ari-lint` currently delegates compiler-backed checking to `ari --check`.
- `ari-lint` currently shares diagnostic expectations with the Ari compiler and
  `ari-lsp`.
- This document is a split-planning note, not an implementation change.

## Candidate Models

### Model A: invoke `ari --check`

In this model, `ari-lint` treats the Ari compiler as an external executable.
The compiler remains responsible for parser, module-loader, semantic,
ownership, and lowering diagnostics.

Current source evidence matches this model: `tools/lint/checker.cpp` builds a
compiler command that includes the configured compiler path, any `-I` module
paths, the source file, and `--check`, then invokes it through
`tools/ari_tooling/process.cpp`.

The compiler path is supplied by `--ari` or `ARI_COMPILER` in the current
implementation. PATH/discovery behavior is not documented as current
`ari-lint` behavior and needs follow-up if a future standalone tool wants it.

Module include paths are forwarded through `-I` in the current implementation.
Standalone include path behavior outside the `ari` repository needs follow-up.

Compiler diagnostics remain owned by the `ari` compiler. `ari-lint` parses and
normalizes those diagnostics, then runs lint-only source rules after compiler
checking.

### Model B: link compiler libraries directly

In this model, `ari-lint` links compiler, parser, semantic checker, or checker
code directly instead of invoking a compiler executable.

This may reduce process overhead and could avoid parsing textual compiler
diagnostics, but it creates a tighter ABI/source dependency between a future
`ari-lint` repository and compiler internals. This is risky before the compiler
internal API is stable and intentionally supported.

Local source evidence does not show `ari-lint` linking `src/*.cpp` compiler
implementation files today. The current boundary inventory records a process
boundary plus shared tooling helpers, not a direct compiler library boundary.

### Model C: shared checker or diagnostics library

In this model, `ari`, `ari-lint`, and `ari-lsp` share a separately versioned
checker or diagnostic component.

This may be desirable later because `ari-lsp` already shares the lint checker
library and diagnostic shape with `ari-lint`, while `ari-lint` also normalizes
compiler diagnostics. A future shared component could make that boundary more
explicit.

This requires a stable library boundary, versioning policy, and ownership model.
It should not be introduced as part of the first split.

## Preferred Model

Preferred near-term model: Model A, invoke `ari --check`.

Reasons:

- It preserves compiler ownership of parser, module-loader, semantic,
  ownership, and lowering diagnostics.
- In short, Model A preserves compiler ownership of parser/module/sema
  diagnostics.
- It reduces coupling between a future `ari-lint` repository and compiler
  internals.
- It matches current documentation that `ari-lint` delegates compiler-backed
  checking to `ari --check`.
- It allows a future split without first stabilizing compiler internal APIs.

Model B is deferred until compiler internals expose a stable supported API.
Model C is deferred until shared diagnostics/checker boundaries are
intentionally designed.

Any future change away from Model A must update the CLI/diagnostic contract and
compatibility matrix.

## Required Boundary For Model A

Model A requires a clear executable boundary:

- `ari-lint` must accept or discover an Ari compiler executable.
- `--ari` compiler path behavior must be documented.
- `ARI_COMPILER` fallback behavior must be documented if supported.
- `-I` include path forwarding must be documented if supported.
- Compiler failure behavior must be distinguished from lint-only diagnostics.
- JSON diagnostic compatibility must be preserved or versioned.
- Tests must cover missing compiler binary, compiler failure, lint-only
  diagnostics, and mixed diagnostics.

Current evidence:

- `--ari` is implemented and documented, but dedicated precedence tests need
  follow-up.
- `ARI_COMPILER` is implemented and documented, but fallback and precedence
  tests need follow-up.
- `-I` forwarding is implemented and documented, but standalone use outside the
  `ari` repository needs follow-up.
- Compiler-backed JSON diagnostics and lint-only JSON diagnostics are covered
  through `tests/Makefile`, but exact schema and mixed-diagnostic fixtures need
  follow-up.
- Missing compiler binary behavior comes from the shared process helper and
  needs follow-up before it becomes a public compatibility promise.

## Compatibility Implications

Future `ari-lint` releases must declare compatible Ari compiler versions. The
dependency is not just an executable name; it includes compiler diagnostic text,
diagnostic codes, module lookup behavior, `--check` behavior, and any JSON shape
that downstream tools consume.

The Ari Foundry portal compatibility matrix should include `ari-lint` only
after a real repository and release exist. Future repo links must not be
invented before repositories exist.

Compatibility cannot rely on undocumented local monorepo paths. A future split
must define how CI installs or locates a compatible Ari compiler binary.

## Split Risks

- External compiler binary discovery may be fragile.
- Compiler diagnostic JSON may change.
- Module lookup through `-I` may differ outside the monorepo.
- Config discovery may depend on source file layout.
- `ari-lsp` may rely on shared lint behavior.
- CI must provision a compatible Ari compiler binary.

## Follow-up Checklist

- [ ] Confirm `--ari` behavior from source and tests
- [ ] Confirm `ARI_COMPILER` fallback behavior from source and tests
- [ ] Confirm `-I` forwarding behavior from source and tests
- [ ] Document missing compiler binary behavior
- [ ] Document compiler failure vs lint-only failure behavior
- [ ] Define compatible Ari compiler version policy
- [ ] Add standalone fixtures for external compiler invocation
- [ ] Decide when Model B or Model C should be reconsidered

## Explicit Non-Goals

- Do not move `tools/lint` in this step.
- Do not move `docs/lint` in this step.
- Do not create `ari-foundry/ari-lint` in this step.
- Do not modify `Makefile` or `tests/Makefile` in this step.
- Do not modify compiler source in this step.
- Do not change `ari-lint` behavior in this step.
- Do not introduce a shared checker library in this step.
- Do not invent future repository links.
- Do not modify `ari-foundry.github.io` in this step.
