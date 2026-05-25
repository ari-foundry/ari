# Compiler Test Authoring

This page explains how to add tests for Ari compiler work. It is for
contributors who are changing the hosted compiler, developer docs, artifacts,
or Ari source fixtures that put pressure on compiler behavior.

The goal is to keep compiler development testable in small pieces.

## Choose The Test Bucket

Pick the bucket by behavior, not by the source file you edited.

| Bucket | Use When | Example |
| --- | --- | --- |
| `tests/cases/<feature>/ok/` | The language behavior should compile, emit LLVM, link, or run. | `tests/cases/control-flow/ok/for-vector-llvm.ari` |
| `tests/cases/<feature>/errors/` | The source should be rejected with a stable diagnostic. | `tests/cases/functions/errors/return-type.ari` |
| `tests/cases/bootstrap-readiness/ok/` | Ari source models a small compiler-shaped data or source/diagnostic workflow without starting self-host work. | `model/model-token-span.ari` |
| `tests/cases/bootstrap-readiness/errors/` | A bootstrap-readiness source fixture should be rejected with a stable diagnostic. | `model/<behavior>.ari` |
| `tests/cases/compiler-development/artifact/ok/` | A compiler stage emits deterministic text that should match a golden file. | `token-dump-basic.tokens` |
| `tests/cases/compiler-development/artifact/errors/` | A diagnostic artifact or artifact-comparison report should match. | `diagnostic-parser-expected.diagnostic` |
| `tests/packages/` | A file-backed module or package-style project needs multiple source files. | `module_cache_stale/` |

If a test could fit two buckets, choose the one closest to the first layer that
can fail. A lexer artifact should not wait for LLVM. A type error should not be
hidden behind an executable test.

The compiler can print the same bucket map:

```text
build/ari --list-test-buckets
build/ari --explain-test-bucket compiler-artifact-ok
```

Use it before adding a fixture when the right directory or first check is not
obvious. The output names the bucket path, kind, owner, first focused check, and
intended use.

## Name The File

Use behavior names:

- `model-token-span.ari`
- `errors-result-flow.ari`
- `no-class-keyword.ari`
- `diagnostic-parser-expected.diagnostic`
- `module-graph-file-module.graph`
- `typed-ir-basic.ir`

Avoid helper names that only make sense inside the current implementation. The
next contributor should know why the test exists before opening compiler code.

## Pick The Small Check

| Test Shape | First Check |
| --- | --- |
| One valid Ari source | `build/ari path/to/case.ari --check` |
| LLVM smoke | `build/ari path/to/case.ari --emit-llvm build/focused/case.ll` |
| Executable behavior | Link one case and compare its exit code or output. |
| Bootstrap-readiness fixture | `make check-bootstrap-readiness` |
| Compiler artifact or golden text | `make check-compiler-artifacts` |
| Bootstrap/readiness docs only | `make check-bootstrap-docs` |
| Language docs only | `make check-language-docs` |
| Unknown fixture bucket | `build/ari --list-test-buckets` |

Full `make check` belongs at handoff for broad changes. Sanitizer checks are
intentionally separate and are not part of this focused authoring loop.

## Expected Results

For executable source fixtures, keep the expected exit code in `tests/Makefile`
near the command that builds and runs the fixture. The source fixture should
compute that value from meaningful data, not return a magic constant.

For artifact fixtures, commit the expected text under
`tests/cases/compiler-development/artifact/ok/` or
`tests/cases/compiler-development/artifact/errors/`. Prefer normalized text over
raw paths, temp names, or pointer-shaped values.

For error fixtures, assert the diagnostic phrase or code that proves the
intended layer rejected the source. Put normal language failures under the
owning feature's `errors/` bucket; reserve compiler-development artifact errors
for stable diagnostic text emitted through `--emit-diagnostics` or related
artifact flags.

## Comments In Tests

Use one short comment at the top of bootstrap-readiness or compiler-shaped
feature fixtures:

```ari
// Covers test authoring policy as ordinary Ari data.
```

Keep comments useful:

- Say which behavior the fixture protects.
- Do not narrate obvious assignment or arithmetic.
- Put longer rationale in docs, not inside tiny test sources.

## Artifact Update Rule

When an artifact changes:

1. Confirm the compiler behavior change is intentional.
2. Regenerate only the affected artifact.
3. Review the text diff before accepting it.
4. Update the doc or roadmap entry if the artifact format changed.
5. Run the narrow artifact target.

Artifact tests should make compiler stages reviewable. They should not become a
large pile of opaque snapshots.

## Review Checklist

Before handing off a compiler test change, answer:

- Which compiler layer owns this behavior?
- Is the file in the closest `ok`, `errors`, `artifact/ok`, or
  `artifact/errors` bucket?
- Does the filename name the behavior?
- Is the smallest check documented or wired into `tests/Makefile`?
- Did docs change when a new user-facing rule became clearer?
- Is the non-goal clear when the request could drift into unrelated work?

This keeps tests useful for the hosted compiler without creating a private
compiler-only test world.
