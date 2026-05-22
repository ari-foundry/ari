# Standard Library Stability Policy

This page explains how to decide whether a standard-library API is ready for
normal Ari programs. It is written for new contributors as well as library
users: if you can answer the questions on this page, you should know whether an
API is safe to depend on, which platforms it works on, and how breaking changes
are handled.

## Stability Labels

Use one of these labels in module guides, roadmap notes, and generated API
summaries.

| Label | Meaning | Promise |
| --- | --- | --- |
| stable | The API has docs, examples, manifest coverage, focused tests, ownership rules, failure rules, and platform notes where needed. | Do not break without a migration period and deprecation note. |
| usable | The main behavior is implemented and tested, but edge cases, ergonomics, or portability may still be hardened. | Prefer additive changes. Breaking changes need a clear reason and docs update. |
| platform-backed | The API works through the hosted runtime, libc, OS handles, clocks, sockets, threads, or environment state. | Document the supported target family before calling it portable. |
| platform-specific | The API exposes ABI, target, loader, descriptor, errno, syscall, or OS-family facts. | Keep names honest and avoid pretending all targets support it. |
| experimental | The shape is still being designed or needs compiler/runtime support before users should rely on it. | May change or disappear. Do not teach it as the normal API. |

No API should be called stable only because it compiles. Stability means the
behavior is understandable, tested, and maintainable.

## What Makes An API Stable

An API can move to `stable` when all of these are true:

- Purpose: the docs say why the API exists and which module owns it.
- Signature: allocation, borrowing, ownership, error, and platform requirements
  are visible from the name, type, or module.
- Example: the module guide shows a small realistic call pattern.
- Manifest: `tests/std_api_manifest.txt` has a focused coverage note.
- Tests: there is at least one positive behavior test and one relevant failure,
  edge-case, or diagnostic test when the feature can fail.
- Value contract: copy, move, clone, drop, drain, reset, and iterator
  invalidation behavior are documented when the API moves values.
- Error contract: ordinary failure uses `Option`, `Result`, or
  `std::error::Error`; raw integers stay compatibility-only.
- Platform contract: hosted or platform-specific APIs say which targets are
  supported today and which targets are roadmap-only.

## Generated API Docs

The exhaustive public API index is generated:

```sh
python3 tools/generate_std_api_docs.py
python3 tools/generate_std_api_docs.py --check
```

Generated docs live at [generated/api-index.md](generated/api-index.md). They
are produced from `tests/std_api_manifest.txt`, so each public API appears next
to the test or documentation note that justifies it.

Hand-written docs still matter. The generated index answers "what spelling
exists?" Module guides answer "why does this exist?", "how do I use it?", "what
does it allocate?", and "what happens on failure?"

## Stable Versus Experimental

Teach the stable or usable path first. Keep experimental work in roadmap notes
until it has source, tests, and docs.

Good:

```text
std::fs::read_result(path) -> Result[String, Error]
```

This makes failure explicit and uses the shared error vocabulary.

Compatibility-only:

```text
std::fs::read_raw(path) -> Result[String, i64]
```

Raw error bridges can remain for low-level runtime or FFI work, but the guide
should tell users which typed API replaces them.

Experimental:

```text
future async socket polling
```

This belongs in roadmap docs until runtime ownership, cancellation, and error
contracts are clear.

## Deprecation Policy

Deprecation should be boring and predictable:

1. Add the replacement API, tests, and docs first.
2. Update examples so new users see the replacement path.
3. Mark the old API as compatibility-only or deprecated in the module guide.
4. Keep the old API long enough for existing tests and examples to migrate.
5. Remove it only after the replacement has coverage and the migration note
   names the old and new spellings.

When the language supports a source attribute such as `@deprecated("use ...")`,
use it for compiler-visible warnings. Until then, keep the deprecation note in
the module guide and generated manifest coverage note.

## Self-Review Rule

When adding or editing standard-library docs, read the new text once as a
first-time Ari user and once as the maintainer who must debug it later. Fix the
text if any of these are unclear:

- Which module owns the API?
- Is this stable, usable, platform-backed, platform-specific, or experimental?
- Does the example compile under the current language surface?
- What happens on absence, OS failure, invalid input, allocation failure, or
  zone reset?
- Does the doc accidentally promise unsupported platforms or future features?
- Does the generated API index still agree with the manifest?

This review pass is part of the work, not cleanup after the work.
