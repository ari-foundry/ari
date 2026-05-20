# Compiler Artifact Testing

This page defines how Ari compiler work should produce, normalize, and compare
artifacts. It is compiler-development infrastructure, not bootstrapping work.

Artifact tests make regressions easier to locate. If a token dump changes, the
lexer changed. If the token dump is stable but the syntax dump changes, the
parser changed. If syntax is stable but typed IR changes, sema or lowering
changed. Executable behavior should be the last signal, not the first one.

Read this page with:

- [Compiler Maturity Gates](compiler-maturity-gates.md)
- [Compiler Source And Diagnostics](compiler-source-diagnostics.md)
- [Compiler Development Roadmap](compiler-development-roadmap.md)
- [Bootstrap Readiness](bootstrap-readiness.md)

## Goals

Artifact testing should:

- keep compiler regressions close to the layer that introduced them
- compare deterministic text before comparing executables
- make golden files reviewable in normal code review
- normalize paths, generated ids, temporary names, and nondeterministic symbol
  order
- keep each check small enough to run while developing
- avoid one giant "self-host" test as the first signal of correctness

Artifact testing should not:

- hide differences behind broad snapshots
- compare raw host paths in committed golden files
- require threads, sockets, dynamic loading, or freestanding startup
- start with LLVM or executable comparison before frontend artifacts are stable
- depend on sanitizer checks for routine docs/design slices

## Artifact Order

Compare artifacts in this order:

| Order | Artifact | Purpose | Producer |
| --- | --- | --- | --- |
| 1 | Token dump | Prove lexing and source spans are stable. | lexer |
| 2 | Diagnostic dump | Prove source maps and error rendering are stable. | lexer/parser/sema |
| 3 | Syntax dump | Prove parsing and recovery are stable. | parser |
| 4 | HIR dump | Prove syntax lowering and name surfaces are stable. | lowering/resolver |
| 5 | Typed IR dump | Prove type, ownership, trait, and module facts are stable. | sema |
| 6 | LLVM text | Prove backend lowering is stable enough to inspect. | LLVM backend |
| 7 | Object/shared symbols | Prove exported symbols, visibility, and relocations. | LLVM driver |
| 8 | Executable behavior | Prove final behavior only after earlier artifacts match. | linked executable |

Do not skip directly to executable comparison for compiler frontend work. A
binary exit code can say "something changed"; it cannot say which compiler
layer changed.

## Artifact Formats

Text artifacts should be line-oriented and deterministic.

Token dump example:

```text
token Identifier "main" @ source.ari:1:4
token LParen "(" @ source.ari:1:8
token RParen ")" @ source.ari:1:9
```

Syntax dump example:

```text
FunctionDecl name=main return=i64
  Block
    Return
      IntegerLiteral value=0 type=i64
```

Typed IR dump example:

```text
fn main() -> i64 symbol=_ARNv4main
  ret i64 0
```

Diagnostic dump example:

```text
error[P0001]: expected expression
  --> source.ari:4:12
   |
 4 |   let x =
   |           ^ expected expression here
```

The exact shape can change before the first artifact runner lands, but each
format must keep stable names, source locations, and indentation rules.

## Normalization Rules

Before comparing golden files, normalize:

| Input | Normalized Form |
| --- | --- |
| Repository root paths | `<repo>/...` |
| Build directory paths | `<build>/...` |
| Temporary file names | `<tmpN>` in first-seen order |
| Generated symbol suffixes | `<symN>` when suffixes are nondeterministic |
| Pointer addresses | `<ptr>` |
| Timing data | remove unless the test is explicitly about timing |
| Diagnostic path separators | `/` in golden files |

The normalizer should normalize repository-local paths before comparing golden
files. Do not normalize meaningful language facts. Type names, module paths,
spans, diagnostic codes, visibility, ABI classes, and public symbols should
remain visible.

## Golden File Policy

Golden files are committed text outputs. They should be small and local to the
feature they protect.

Recommended layout before a real compiler tool exists:

```text
tests/cases/bootstrap-readiness/ok/artifacts/
tests/cases/bootstrap-readiness/golden/
```

Recommended layout for future Ari compiler tools:

```text
bootstrap/stage1/tests/fixtures/
bootstrap/stage1/tests/lex/ok/
bootstrap/stage1/tests/lex/errors/
bootstrap/stage1/tests/parse/ok/
bootstrap/stage1/tests/parse/errors/
bootstrap/stage1/tests/hir/ok/
bootstrap/stage1/tests/hir/errors/
bootstrap/stage1/tests/ir/ok/
bootstrap/stage1/tests/ir/errors/
bootstrap/stage1/tests/golden/
```

Golden update policy:

- do not auto-update golden files in normal checks
- provide a separate future `update-golden` target
- require review of golden diffs
- keep expected output close to the fixture name
- include one behavior per fixture when possible

## Focused Make Targets

Small targets should compare one artifact family at a time:

```text
make check-compiler-dev-docs
make check-compiler-artifacts
make check-bootstrap-readiness
make -C bootstrap check-lex
make -C bootstrap check-parse
make -C bootstrap check-report
make -C bootstrap check-hir
make -C bootstrap check-ir
```

For the current C++ compiler, prefer direct focused commands while developing:

```text
build/ari tests/cases/modules/ok/module-llvm.ari --check
build/ari tests/cases/ffi/ok/library-export.ari --shared --emit-llvm build/focused/library-export.ll
build/ari tests/cases/bootstrap-readiness/ok/formatting/formatting-artifact-line.ari --check
```

Run broad `make check` only before handing off larger compiler changes.

## First Implementation Slices

Land artifact testing in slices:

| Slice | Deliverable | Focused Tests |
| --- | --- | --- |
| Text comparator | Compare expected/actual text with a useful mismatch report. | `compare-equal`, `compare-line-mismatch`, `compare-extra-line`. |
| Path normalizer | Replace repo, build, and temporary paths. | `normalize-repo-path`, `normalize-build-path`, `normalize-temp-path`. |
| Token dump format | Stable lexer output for identifiers, literals, comments, and invalid tokens. | `token-basic`, `token-string-escapes`, `token-invalid-char`. |
| Diagnostic dump format | Stable report output using source maps and labels. | `diagnostic-single-label`, `diagnostic-note-order`. |
| Syntax dump format | Stable parser tree output. | `syntax-function`, `syntax-match`, `syntax-recovery`. |
| HIR dump format | Stable lowered structure and symbol ids. | `hir-module-path`, `hir-patterns`, `hir-imports`. |
| Typed IR dump format | Stable typed facts after sema. | `ir-types`, `ir-ownership`, `ir-trait-call`. |
| LLVM normalizer | Normalize paths and harmless temporary symbol noise. | `llvm-symbols`, `llvm-aggregate-layout`. |

Each slice should have one positive fixture and at least one negative or
mismatch fixture when the behavior can fail.

## Current Seed Implementation

The current repository has a tiny artifact comparison seed plus the first real
frontend producer:

```text
tests/check_compiler_artifacts.py
tests/cases/compiler-development/artifact/ok/
tests/cases/compiler-development/artifact/errors/
tests/cases/compiler-development/artifact/ok/token-dump-basic.ari
tests/cases/compiler-development/artifact/ok/token-dump-basic.tokens
tests/cases/compiler-development/artifact/ok/syntax-dump-basic.syntax
tests/cases/compiler-development/artifact/ok/typed-ir-basic.ir
tests/cases/compiler-development/artifact/errors/diagnostic-borrow-conflict.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-missing-module.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-parser-expected.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.ari
tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-unknown-trait.diagnostic
ari --emit-tokens path
ari --emit-syntax path
ari --emit-diagnostics path
ari --emit-typed-ir path
make check-compiler-artifacts
```

It currently proves eight low-level contracts:

- equal expected/actual text passes without output
- repository paths, build paths, temporary names, and pointer addresses
  normalize to stable placeholders
- a line mismatch produces a small report naming the fixture and line
- `--emit-tokens` writes deterministic lexer output for a small Ari source file
- `--emit-syntax` writes deterministic parser output before sema and backend
  behavior are involved
- `--emit-diagnostics` writes a normalized diagnostic artifact for an expected
  compiler failure
- `--emit-diagnostics` classifies representative lexer, parser, module, type,
  and ownership failures with stable diagnostic-code families
- `--emit-typed-ir` writes deterministic sema-lowered IR for a small Ari source
  file without involving LLVM codegen

The first typed-IR golden uses `--no-implicit-std` so the fixture protects the
source file's lowered facts instead of recording every implicit prelude
declaration. Add separate std/prelude IR fixtures only when that behavior is
the thing being tested.

This is deliberately small. Future structured diagnostic, HIR, richer typed-IR,
and backend artifact producers should plug into the same shape instead of
inventing unrelated golden comparison rules.

## Current Compiler Integration

The current compiler already has useful artifact checks:

- `--check` for frontend and sema diagnostics
- `--emit-tokens` for stable lexer token text and start locations
- `--emit-syntax` for stable parser tree text before semantic analysis
- `--emit-diagnostics` for stable expected-failure text before a full
  multi-label diagnostic model exists
- `--emit-typed-ir` for stable sema output before LLVM lowering
- `--emit-llvm` for LLVM text
- `--emit-obj` for object files
- `--shared` for shared-library visibility
- executable exit-code checks where LLVM driver support is available

Keep adding narrow checks rather than relying only on broad suite runs. For
backend changes, inspect the generated LLVM or object symbol table when the
behavior is really about ABI, linkage, runtime hooks, or visibility.

## Review Checklist

When adding a compiler feature, ask:

- Does this feature have an ok test?
- Does misuse have an error test?
- Does the test name say what behavior it protects?
- Does the smallest useful artifact get checked?
- Are paths and nondeterministic names normalized?
- Can a reviewer understand the golden diff without running the compiler?
- Is executable behavior tested only after earlier artifacts are stable?

If the answer is no, add a smaller artifact test before adding a broad end-to-
end test.

## Readiness Impact

Stage comparison remains a major blocker. Ari should stay around the current
**38-42% ready** estimate until the current token, diagnostic, syntax, and
typed-IR seeds grow into broader coverage, and HIR plus LLVM text comparison
exist enough that a future stage1 and stage2 can disagree in a useful,
localized way.
