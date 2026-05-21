# Compiler Artifact Testing

This page defines how Ari compiler work should produce, normalize, and compare
artifacts. It is compiler-development infrastructure, not bootstrapping work.

Artifact tests make regressions easier to locate. If a token dump changes, the
lexer changed. If the token dump is stable but the syntax dump changes, the
parser changed. If syntax is stable but typed IR changes, sema or lowering
changed. Executable behavior should be the last signal, not the first one.

Read this page with:

- [Compiler Maturity Gates](compiler-maturity-gates.md)
- [Compiler Artifact Authoring](compiler-artifact-authoring.md)
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
| 1 | Stage plan | Prove artifact order, owners, and first checks are visible from the compiler. | driver |
| 2 | Source map dump | Prove file text, byte offsets, line tables, and newline policy are stable. | source loader |
| 3 | Token dump | Prove lexing and source spans are stable. | lexer |
| 4 | Diagnostic dump | Prove source maps and error rendering are stable. | lexer/parser/sema |
| 5 | Diagnostic catalog | Prove diagnostic code ownership and fallback policy are stable. | diagnostics |
| 6 | Syntax dump | Prove parsing and recovery are stable. | parser |
| 7 | Module graph dump | Prove file-backed module loading, imports, and public item surfaces are stable. | module loader |
| 8 | Declaration index dump | Prove parsed declaration signatures, visibility, and source locations are stable. | parser/module loader |
| 9 | HIR dump | Prove syntax lowering and name surfaces are stable. | lowering/resolver |
| 10 | Typed IR dump | Prove type, ownership, trait, and module facts are stable. | sema |
| 11 | Pass summary | Prove stage counts and module/sema boundaries are stable. | driver |
| 12 | LLVM text | Prove backend lowering is stable enough to inspect. | LLVM backend |
| 13 | Object/shared symbols | Prove exported symbols, visibility, and relocations. | LLVM driver |
| 14 | Executable behavior | Prove final behavior only after earlier artifacts match. | linked executable |

Do not skip directly to executable comparison for compiler frontend work. A
binary exit code can say "something changed"; it cannot say which compiler
layer changed.

## Artifact Formats

Text artifacts should be line-oriented and deterministic.

Source map dump example:

```text
SourceMap source=src/main.ari files=1
  File module=<root> root=true path=src/main.ari bytes=20 lines=1 trailing_newline=true
    Line number=1 byte_start=0 byte_len=19 newline=lf text="fn main() -> i64 {}"
```

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

Module graph dump example:

```text
ModuleGraph source=src/main.ari target=x86_64-pc-linux-gnu implicit_std=false
  Sources count=2
    Source module=<root> root=true path=src/main.ari
    Source module=math root=false path=src/math.ari
  Imports count=1
    Import owner=<root> module=math local=math visibility=private source=src/math.ari
```

Declaration index dump example:

```text
DeclarationIndex source=src/main.ari modules=1 declarations=2
  Decl module=<root> kind=struct name=Span visibility=pub loc=src/main.ari:1:12 generics=[] tuple=false fields=[start:i64, end:i64]
  Decl module=<root> kind=fn name=main visibility=private loc=src/main.ari:6:4 generics=[] params=[] return=i64 body=true
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

Current compiler-development layout before a real Ari compiler tool exists:

```text
tests/cases/compiler-development/artifact/ok/
tests/cases/compiler-development/artifact/errors/
```

`artifact/ok` stores fixtures and committed outputs that should compare
cleanly. `artifact/errors` stores expected compiler diagnostic artifacts and
seed mismatch reports for the text comparator.

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
| Source map dump format | Stable byte offsets, line lengths, newline policy, and source snippets. | `source-map-file`, `source-map-crlf`, `source-map-empty`. |
| Token dump format | Stable lexer output for identifiers, literals, comments, and invalid tokens. | `token-basic`, `token-string-escapes`, `token-invalid-char`. |
| Diagnostic dump format | Stable field-oriented output for code, family, source, line, column, and message. | `diagnostic-single-label`, `diagnostic-note-order`. |
| Syntax dump format | Stable parser tree output. | `syntax-function`, `syntax-match`, `syntax-recovery`. |
| Module graph dump format | Stable file-backed sources, imports, and item surfaces. | `module-graph-file`, `module-graph-search-path`, `module-graph-cfg`. |
| Declaration index dump format | Stable declaration names, signatures, visibility, and locations. | `declaration-index-basic`, `declaration-index-module`. |
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
tests/cases/compiler-development/artifact/ok/declaration-index-basic.ari
tests/cases/compiler-development/artifact/ok/declaration-index-basic.decls
tests/cases/compiler-development/artifact/ok/diagnostic-catalog.catalog
tests/cases/compiler-development/artifact/ok/source-map-file-module.map
tests/cases/compiler-development/artifact/ok/token-dump-basic.ari
tests/cases/compiler-development/artifact/ok/token-dump-basic.tokens
tests/cases/compiler-development/artifact/ok/module-graph-file-module.graph
tests/cases/compiler-development/artifact/ok/pass-summary-basic.summary
tests/cases/compiler-development/artifact/ok/stage-plan-basic.plan
tests/cases/compiler-development/artifact/ok/syntax-dump-basic.syntax
tests/cases/compiler-development/artifact/ok/typed-ir-basic.ir
tests/cases/compiler-development/artifact/errors/diagnostic-borrow-conflict.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-missing-module.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-parser-expected.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.ari
tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-unknown-trait.diagnostic
ari --emit-tokens path
ari --emit-source-map path
ari --emit-syntax path
ari --emit-diagnostics path
ari --emit-diagnostic-catalog path
ari --emit-module-graph path
ari --emit-declaration-index path
ari --emit-stage-plan path
ari --emit-pass-summary path
ari --emit-typed-ir path
make check-compiler-artifacts
```

It currently proves fourteen low-level contracts:

- equal expected/actual text passes without output
- repository paths, build paths, temporary names, and pointer addresses
  normalize to stable placeholders
- a line mismatch produces a small report naming the fixture and line
- `--emit-stage-plan` writes deterministic artifact order, owner, first-check,
  and development-gate text directly from the compiler driver
- `--emit-source-map` writes deterministic source file, byte offset, line, and
  newline-policy text for root and file-backed modules
- `--emit-tokens` writes deterministic lexer output for a small Ari source file
- `--emit-syntax` writes deterministic parser output before sema and backend
  behavior are involved
- `--emit-diagnostics` writes a normalized diagnostic artifact for an expected
  compiler failure
- `--emit-diagnostics` classifies representative lexer, parser, module, type,
  and ownership failures with stable diagnostic codes and `family=...` layer names
- `--emit-diagnostics` also writes parseable `source=`, `line=`, and `column=` fields
  for location-aware tooling
- `--emit-diagnostic-catalog` writes the current diagnostic code table, owning
  compiler source file, family, and fallback policy
- `--emit-module-graph` writes deterministic file-backed source, import, and
  item-surface facts without running sema or LLVM codegen
- `--emit-declaration-index` writes deterministic declaration signatures,
  visibility, module names, and source locations before sema or LLVM codegen
- `--emit-typed-ir` writes deterministic sema-lowered IR for a small Ari source
  file without involving LLVM codegen
- `--emit-pass-summary` writes deterministic stage counts for lexing, syntax,
  module loading, and sema

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
- `--emit-source-map` for stable byte offset, line table, and snippet text
- `--emit-stage-plan` for stable stage-order and first-check routing from the
  compiler binary
- `--emit-diagnostic-catalog` for stable diagnostic code ownership
- `--emit-tokens` for stable lexer token text and start locations
- `--emit-syntax` for stable parser tree text before semantic analysis
- `--emit-diagnostics` for stable expected-failure text before a full
  multi-label diagnostic model exists
- `--emit-module-graph` for stable file-backed source, import, and item-surface
  text before sema or backend behavior are involved
- `--emit-declaration-index` for stable declaration signatures, visibility,
  module names, and source locations before semantic lowering
- `--emit-typed-ir` for stable sema output before LLVM lowering
- `--emit-pass-summary` for quick stage-boundary counts in compiler-development
  tests
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
