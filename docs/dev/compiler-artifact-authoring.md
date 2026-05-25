# Compiler Artifact Authoring

This page explains how to add or change compiler artifacts in Ari.

Artifacts are the compiler's review trail. They let contributors see whether a
change belongs to source loading, lexing, parsing, diagnostics, module loading,
sema, typed IR, LLVM, object emission, or final executable behavior.

## Artifact Design Rule

Design every artifact around one question:

```text
What is the earliest compiler layer that can prove this behavior?
```

If a source-map artifact can prove the change, do not wait for tokens. If a
token dump can prove it, do not wait for syntax. If typed IR can prove it, do
not rely only on a linked executable.

## Artifact Order

Use this order when choosing a new artifact or golden:

| Order | Artifact | First Use |
| --- | --- | --- |
| 1 | `--emit-stage-plan` | Artifact ladder, owners, and first checks. |
| 2 | `--emit-capability-inventory` | Implemented, partial, planned, and rejected compiler capabilities. |
| 3 | `--emit-source-map` | Source files, bytes, line starts, and paths. |
| 4 | `--emit-tokens` | Lexer output and token spans. |
| 5 | `--emit-diagnostics` | Expected failures, labels, notes, and path normalization. |
| 6 | `--emit-diagnostic-catalog` | Diagnostic codes, families, owners, and current fallback policy. |
| 7 | `--emit-syntax` | Parser tree and recovery shape. |
| 8 | `--emit-module-graph` | File-backed modules, imports, and item surfaces. |
| 9 | `--emit-declaration-index` | Declaration names, visibility, signatures, source locations, imports, and uses. |
| 10 | `--emit-resolved-index` | Resolver-facing functions, locals, calls, enum cases, pattern bindings, imports, declarations, signatures, visibility, and source locations. |
| 11 | `--emit-typed-ir` | Type, ownership, trait, and lowering facts. |
| 12 | `--emit-pass-summary` | Stage counts and pass boundaries. |
| 13 | `--emit-c-header` | C-compatible public ABI wrapper spelling. |
| 14 | `--emit-llvm` | Backend lowering once earlier layers are stable. |
| 15 | `--emit-llvm-fragment` | Requested LLVM function fragments after full LLVM text is emitted. |
| 16 | `--emit-obj` | Object ABI, exported names, and relocation surfaces. |
| 17 | `--emit-symbols` | Requested object or shared-library symbol inventory after backend output. |
| 18 | `--shared` | Shared-library export and dynamic symbol surfaces. |
| 19 | `-o` | Final stdout/stderr runtime behavior after earlier artifacts match. |

The order is not bureaucracy. It keeps failures near the layer that changed.

## Format Rules

Text artifacts should be:

- line-oriented
- deterministic
- small enough to review
- grouped by source or module when multiple files are involved
- explicit about source path, module path, span, visibility, and symbol facts
- free of host-specific absolute paths unless normalized

Avoid wide snapshots that include unrelated data. A good artifact names the
facts a reviewer must inspect and leaves unrelated layers out.

## Normalization Policy

Normalize nondeterministic data before comparison:

| Data | Normalized Form |
| --- | --- |
| Repository root | `<repo>` |
| Build directory | `<build>` |
| Temporary names | `<tmpN>` in first-seen order |
| Pointer addresses | `<ptr>` |
| Generated suffixes | `<symN>` when the suffix is nondeterministic |
| Host path separators | `/` |
| Timing | omitted unless timing is the behavior |

Do not normalize meaningful language facts: type names, module names, public
symbols, ABI classes, diagnostic codes, source spans, or visibility.

## Golden Update Rule

Golden files are committed expected outputs. Update them only when behavior
changed intentionally:

1. Run the producer into `build/`.
2. Compare expected and actual output.
3. Read the diff before accepting it.
4. Update only the affected golden with
   `python3 tests/check_compiler_artifacts.py --update expected actual`.
5. Update docs if the artifact format or compiler rule changed.
6. Run the narrow artifact check.

Do not auto-update goldens as part of normal checks. The comparator's
`--update` mode is explicit and reviewable; a future make target may wrap it,
but should never run during normal checks.

## Test Placement

Use these locations:

- `tests/cases/compiler-development/artifact/ok/` for expected matching
  artifacts
- `tests/cases/compiler-development/artifact/errors/` for diagnostic goldens
  and comparison mismatch reports
- `tests/cases/bootstrap-readiness/ok/` only for Ari source fixtures that
  exercise compiler-shaped artifact data
- `tests/cases/<feature>/ok/` when the behavior is better proven by a feature
  fixture plus one producer command

Keep fixture names behavior-based:

- `source-map-file-module.map`
- `artifact-fixtures.inventory`
- `source-map-utf8.map`
- `source-map-empty.map`
- `source-map-crlf.map`
- `token-dump-basic.tokens`
- `token-dump-lexical-surface.tokens`
- `token-dump-crlf.tokens`
- `syntax-dump-basic.syntax`
- `syntax-declarations.syntax`
- `syntax-control-flow.syntax`
- `module-graph-file-module.graph`
- `declaration-index-basic.decls`
- `declaration-index-project-compiler.decls`
- `declaration-index-generic-aggregate.decls`
- `capability-inventory.inventory`
- `diagnostic-catalog.catalog`
- `stage-plan-basic.plan`
- `typed-ir-basic.ir`
- `project-compiler.ir`
- `ownership-aggregate-field-move.ir`
- `compiler-shaped-aggregates.ir`
- `generic-function-compiler-shaped.ir`
- `backend-core.llvm-frag`
- `backend-generic-function.llvm-frag`
- `backend-generic-aggregate.llvm-frag`
- `backend-aggregate-match-model.llvm-frag`
- `backend-layout-aggregate.llvm-frag`
- `backend-ownership-drop-aggregate.llvm-frag`
- `backend-ownership-drop-runtime-enum.llvm-frag`
- `backend-ownership-compiler-shaped.llvm-frag`
- `backend-trait-dispatch.llvm-frag`
- `c-header-repr-struct.h`
- `c-header-repr-payload-enum.h`
- `c-header-generated-aggregates.h`
- `object-library-export.symbols`
- `object-aggregate-extern-link.symbols`
- `shared-visibility.symbols`
- `runtime-output-basic.stdout`
- `runtime-output-trait.stdout`
- `diagnostic-assignment-while-borrowed.diagnostic`
- `diagnostic-field-assignment-while-borrowed.diagnostic`
- `diagnostic-return-live-owner.diagnostic`
- `diagnostic-loop-break-live-owner.diagnostic`
- `diagnostic-loop-continue-live-owner.diagnostic`
- `diagnostic-borrow-after-move.diagnostic`
- `diagnostic-double-move.diagnostic`
- `diagnostic-enum-payload-invalid-move.diagnostic`
- `diagnostic-match-branch-state-mismatch.diagnostic`
- `diagnostic-stored-owned-enum-payload-undropped.diagnostic`
- `diagnostic-match-runtime-owned-enum-payload-undropped.diagnostic`
- `diagnostic-runtime-owned-enum-conditional-payload-slot-move.diagnostic`
- `diagnostic-compact-enum-payload-ref.diagnostic`
- `diagnostic-c-header-large-aggregate-by-value.diagnostic`
- `diagnostic-c-header-large-aggregate-enum-by-value.diagnostic`
- `diagnostic-c-header-large-array-by-value.diagnostic`
- `diagnostic-c-header-large-tuple-by-value.diagnostic`
- `diagnostic-c-header-target-aggregate-by-value.diagnostic`
- `diagnostic-c-header-target-array-by-value.diagnostic`
- `diagnostic-c-header-target-tuple-by-value.diagnostic`
- `diagnostic-ownership-aggregate-enum-payload.diagnostic`
- `diagnostic-borrow-aggregate-enum-payload.diagnostic`
- `diagnostic-generic-explicit-arity.diagnostic`
- `diagnostic-generic-explicit-mismatch.diagnostic`
- `diagnostic-generic-duplicate-param.diagnostic`
- `diagnostic-generic-inference-conflict.diagnostic`
- `diagnostic-generic-uninferred.diagnostic`
- `diagnostic-generic-return-context-uninferred.diagnostic`
- `diagnostic-generic-signature-unknown-param.diagnostic`
- `diagnostic-generic-specialization-name-conflict.diagnostic`
- `diagnostic-generic-function-pointer-uninferred.diagnostic`
- `diagnostic-generic-function-pointer-mismatch.diagnostic`
- `diagnostic-generic-function-unknown-type-arg.diagnostic`
- `diagnostic-generic-return-substitution-mismatch.diagnostic`
- `diagnostic-generic-nongeneric-type-args.diagnostic`
- `diagnostic-generic-private-function.diagnostic`
- `diagnostic-enum-constructor-payload-type.diagnostic`
- `diagnostic-enum-duplicate-case.diagnostic`
- `diagnostic-enum-payload-count.diagnostic`
- `diagnostic-match-duplicate-arm.diagnostic`
- `diagnostic-match-empty.diagnostic`
- `diagnostic-match-expression-type-mismatch.diagnostic`
- `diagnostic-match-no-payload.diagnostic`
- `diagnostic-match-non-enum.diagnostic`
- `diagnostic-match-nonexhaustive.diagnostic`
- `diagnostic-match-payload-required.diagnostic`
- `diagnostic-match-struct-pattern-duplicate-field.diagnostic`
- `diagnostic-match-struct-pattern-missing-field.diagnostic`
- `diagnostic-match-struct-pattern-tuple-struct.diagnostic`
- `diagnostic-match-struct-pattern-unknown-field.diagnostic`
- `diagnostic-match-unknown-enum-case.diagnostic`
- `diagnostic-match-unreachable-after-wildcard.diagnostic`
- `diagnostic-match-wrong-enum-case.diagnostic`
- `diagnostic-struct-field-access-non-struct.diagnostic`
- `diagnostic-struct-field-unknown.diagnostic`
- `diagnostic-struct-literal-duplicate-field.diagnostic`
- `diagnostic-struct-literal-extra-field.diagnostic`
- `diagnostic-struct-literal-field-type.diagnostic`
- `diagnostic-struct-literal-missing-field.diagnostic`
- `diagnostic-ffi-extern-abi.diagnostic`
- `diagnostic-ffi-extern-body.diagnostic`
- `diagnostic-ffi-extern-generic.diagnostic`
- `diagnostic-ffi-extern-invalid-link-name.diagnostic`
- `diagnostic-ffi-extern-varargs-aggregate.diagnostic`
- `diagnostic-ffi-extern-varargs-empty.diagnostic`
- `diagnostic-ffi-extern-varargs-function-pointer.diagnostic`
- `diagnostic-ffi-extern-varargs-nonextern.diagnostic`
- `diagnostic-ffi-extern-void-param.diagnostic`
- `diagnostic-ffi-large-aggregate-import.diagnostic`
- `diagnostic-ffi-nonrepr-aggregate-import.diagnostic`
- `diagnostic-ffi-target-aggregate-import.diagnostic`
- `diagnostic-use-after-move.diagnostic`
- `diagnostic-move-borrowed-owner.diagnostic`
- `diagnostic-ownership-partial-move.diagnostic`
- `diagnostic-ownership-vector-dynamic-move.diagnostic`
- `diagnostic-ownership-temporary-element-move.diagnostic`
- `diagnostic-parser-expected.diagnostic`

## Focused Checks

Use the smallest command that proves the artifact:

```sh
python3 tests/check_compiler_artifact_cli.py
build/ari --list-artifacts
build/ari --explain-artifact --emit-tokens
build/ari --list-passes
build/ari --explain-pass sema
build/ari --list-test-buckets
build/ari --explain-test-bucket compiler-artifact-ok
build/ari --list-work-items
build/ari --explain-work-item generic-aggregate-stress
build/ari --list-capabilities
build/ari --explain-capability trait-resolution
build/ari tests/cases/compiler-development/artifact/ok/token-dump-basic.ari --emit-tokens build/focused/token.tokens
python3 tests/check_compiler_artifacts.py --list-fixtures ok
python3 tests/materialize_crlf_fixture.py tests/cases/compiler-development/artifact/ok/source-map-crlf-template.ari build/focused/source-map-crlf.ari
build/ari tests/cases/ffi/ok/library-export.ari --emit-obj build/focused/library-export.o --emit-symbols build/focused/library-export.symbols --symbol _ARNv3add --symbol _ARNv4main
python3 tests/check_compiler_artifacts.py expected actual
python3 tests/check_compiler_artifacts.py --update expected actual
make check-compiler-artifacts
```

Use `--explain-artifact` before adding a broader check. It records the artifact
owner, the first focused check to run, and the exact behavior the artifact is
supposed to prove.
Use `--list-passes` or `--explain-pass` when the question is about a compiler
boundary: pass owner, pass input, pass output, first artifact, or first focused
check.
Use `--list-test-buckets` or `--explain-test-bucket` when the question is about
where the fixture or golden file belongs.
Use `--list-work-items` or `--explain-work-item` when choosing an implementation
slice and its first files, artifact, and focused check.
Use `--list-capabilities` or `--explain-capability` when the change is really a
compiler feature-surface change rather than a new artifact format.

For docs or artifact policy changes, use `make check-bootstrap-docs`.
Full `make check` belongs at handoff for broad changes. Sanitizer checks are
intentionally separate.

## Review Checklist

Before handing off an artifact change, answer:

- Is this the earliest artifact that can prove the behavior?
- Is the output deterministic and normalized?
- Is the artifact small enough to review?
- Does the filename name the behavior?
- Does CLI misuse name the conflicting `--emit-*` flags?
- Did the golden diff avoid unrelated churn?
- Did docs change when the artifact format changed?
- Is executable behavior used only after earlier artifacts are stable?
- Is this useful for ordinary compiler development, not a private tool path?

Good artifacts make Ari compiler work understandable from small diffs. That is
the path toward a maintainable compiler.
