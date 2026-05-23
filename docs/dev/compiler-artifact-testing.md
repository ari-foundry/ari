# Compiler Artifact Testing

This page defines how Ari compiler work should produce, normalize, and compare
artifacts for the compiler that exists today.

Artifact tests make regressions easier to locate. If a token dump changes, the
lexer changed. If the token dump is stable but the syntax dump changes, the
parser changed. If syntax is stable but typed IR changes, sema or lowering
changed. Executable behavior should be the last signal, not the first one.

Read this page with:

- [Compiler Maturity Gates](compiler-maturity-gates.md)
- [Compiler Artifact Authoring](compiler-artifact-authoring.md)
- [Compiler Source And Diagnostics](compiler-source-diagnostics.md)
- [Compiler Development Roadmap](compiler-development-roadmap.md)

## Goals

Artifact testing should:

- keep compiler regressions close to the layer that introduced them
- compare deterministic text before comparing executables
- make golden files reviewable in normal code review
- normalize paths, generated ids, temporary names, and nondeterministic symbol
  order
- keep each check small enough to run while developing
- avoid one giant executable test as the first signal of correctness

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
| 2 | Capability inventory | Prove implemented, partial, planned, and rejected compiler features are explicit. | driver |
| 3 | Source map dump | Prove file text, byte offsets, line tables, and newline policy are stable. | source loader |
| 4 | Token dump | Prove lexing and source spans are stable. | lexer |
| 5 | Diagnostic dump | Prove source maps and error rendering are stable. | lexer/parser/sema |
| 6 | Diagnostic catalog | Prove diagnostic code ownership and fallback policy are stable. | diagnostics |
| 7 | Syntax dump | Prove parsing and recovery are stable. | parser |
| 8 | Module graph dump | Prove file-backed module loading, imports, and public item surfaces are stable. | module loader |
| 9 | Declaration index dump | Prove parsed declaration signatures, visibility, and source locations are stable. | parser/module loader |
| 10 | HIR dump | Prove syntax lowering and name surfaces are stable. | lowering/resolver |
| 11 | Typed IR dump | Prove type, ownership, trait, and module facts are stable. | sema |
| 12 | Pass summary | Prove stage counts and module/sema boundaries are stable. | driver |
| 13 | LLVM text | Prove backend lowering is stable enough to inspect. | LLVM backend |
| 14 | C header text | Prove public ABI wrapper spelling, C-compatible aggregates, and enum payload slots are stable. | C header emitter |
| 15 | Object/shared symbols | Prove exported symbols, visibility, and relocations. | LLVM driver |
| 16 | Executable behavior | Prove final behavior only after earlier artifacts match. | linked executable |

Do not skip directly to executable comparison for compiler frontend work. A
binary exit code can say "something changed"; it cannot say which compiler
layer changed.

## Artifact Formats

Text artifacts should be line-oriented and deterministic.

Source map dump example:

```text
SourceMap source=src/main.ari files=1
  File module=<root> source_id=0 kind=file root=true path=src/main.ari display="src/main.ari" bytes=20 eof_offset=20 line_starts=2 lines=1 trailing_newline=true
    Line number=1 byte_start=0 byte_len=19 newline=lf text="fn main() -> i64 {}"
```

Capability inventory example:

```text
CompilerCapabilityInventory version=1 target=x86_64-pc-linux-gnu implicit_std=false entries=2
  capability=functions status=implemented owner=parser/sema/backend first_check="make check-functions" proves="function declarations, calls, returns, and main entry points"
  capability=hir-artifact status=planned owner=lowering/resolver first_check="future check-compiler-artifacts" proves="lowered syntax and resolver-facing node shapes before typed IR"
```

Pass catalog example:

```text
CompilerPassCatalog version=1 entries=11
  pass=sema layer=middle owner=sema input="AST, modules, declarations, and cfg features" output="typed IR plus warnings" artifact="--emit-typed-ir" first_check="make check-compiler-artifacts" purpose="prove names, types, traits, ownership, and lowering facts"
  Rule one_pass_owner=true earliest_artifact_first=true executable_last=true
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
diagnostic error code=P0001 family=parser message="expected expression" sources=1 labels=1 notes=1 helps=1
  Source source_id=0 kind=file path="source.ari" display="source.ari" bytes=42
  Label index=0 role=primary source_id=0 source="source.ari" line=4 column=12 end_line=4 end_column=12 byte_start=31 byte_end=31 message=""
  Snippet label=0 text="  --> source.ari:4:12\n   |\n 4 |   let x =\n   |           ^"
  Note index=0 location=none message="Ari requires a value after '='"
  Help index=0 location=none message="add an expression after '='"
```

Diagnostic artifacts are field-oriented. The header owns severity, code,
family, message, and counts. `Source` rows describe every source used by labels
or located notes/help. `Label` rows carry `byte_start`, `byte_end`, one-based
start `line`/`column`, and one-based exclusive `end_line`/`end_column`.
`Note` and `Help` rows use `location=none` when they are message-only, or
`location=source` plus the same span fields when they point at source text.

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

Current compiler-development layout:

```text
tests/cases/compiler-development/artifact/ok/
tests/cases/compiler-development/artifact/errors/
```

`artifact/ok` stores fixtures and committed outputs that should compare
cleanly. `artifact/errors` stores expected compiler diagnostic artifacts and
seed mismatch reports for the text comparator.

Golden update policy:

- do not auto-update golden files in normal checks
- use `python3 tests/check_compiler_artifacts.py --update expected actual`
  only after reviewing the actual output
- require review of golden diffs
- keep expected output close to the fixture name
- include one behavior per fixture when possible

## Focused Make Targets

Small targets should compare one artifact family at a time:

```text
make check-compiler-dev-docs
make check-compiler-artifacts
```

For the current C++ compiler, prefer direct focused commands while developing:

```text
python3 tests/check_compiler_artifact_cli.py
python3 tests/check_compiler_capability_cli.py
python3 tests/check_compiler_pass_cli.py
python3 tests/check_compiler_test_bucket_cli.py
python3 tests/check_compiler_work_item_cli.py
python3 tests/check_compiler_artifacts.py --list-fixtures ok
build/ari --list-artifacts
build/ari --list-passes
build/ari --explain-pass sema
build/ari --list-test-buckets
build/ari --explain-test-bucket compiler-artifact-ok
build/ari --list-work-items
build/ari --explain-work-item generic-aggregate-stress
build/ari --list-capabilities
build/ari --explain-capability trait-resolution
build/ari tests/cases/modules/ok/module-llvm.ari --check
build/ari tests/cases/ffi/ok/library-export.ari --shared --emit-llvm build/focused/library-export.ll
python3 tests/extract_symbol_names.py build/focused/library-export.o _ARNv3add _ARNv4main
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
| Diagnostic dump format | Stable field-oriented output for code, family, sources, labels, spans, snippets, notes, help, and message. | `diagnostic-single-label`, `diagnostic-note-order`. |
| Syntax dump format | Stable parser tree output. | `syntax-function`, `syntax-match`, `syntax-recovery`. |
| Module graph dump format | Stable file-backed sources, imports, and item surfaces. | `module-graph-file`, `module-graph-search-path`, `module-graph-cfg`. |
| Declaration index dump format | Stable declaration names, signatures, visibility, and locations. | `declaration-index-basic`, `declaration-index-module`, `declaration-index-generic`. |
| HIR dump format | Stable lowered structure and symbol ids. | `hir-module-path`, `hir-patterns`, `hir-imports`. |
| Typed IR dump format | Stable typed facts after sema. | `ir-types`, `ir-ownership`, `ir-trait-call`. |
| LLVM normalizer | Normalize paths and harmless temporary symbol noise. | `llvm-symbols`, `llvm-aggregate-layout`. |

Each slice should have one positive fixture and at least one negative or
mismatch fixture when the behavior can fail.

## Current Seed Implementation

The current repository has a tiny artifact comparison seed plus frontend,
middle-end, backend-fragment, and runtime-output producers:

```text
tests/check_compiler_artifact_cli.py
tests/check_compiler_artifacts.py
tests/extract_symbol_names.py
tests/materialize_crlf_fixture.py
tests/source_map_unit.cpp
tests/cases/compiler-development/artifact/ok/
tests/cases/compiler-development/artifact/errors/
tests/cases/compiler-development/artifact/ok/artifact-fixtures.inventory
tests/cases/compiler-development/artifact/ok/capability-inventory.inventory
tests/cases/compiler-development/artifact/ok/backend-aggregate-match-model.llvm-frag
tests/cases/compiler-development/artifact/ok/backend-core.llvm-frag
tests/cases/compiler-development/artifact/ok/backend-generic-function.llvm-frag
tests/cases/compiler-development/artifact/ok/backend-generic-aggregate.llvm-frag
tests/cases/compiler-development/artifact/ok/backend-layout-aggregate.llvm-frag
tests/cases/compiler-development/artifact/ok/backend-ownership-drop-aggregate.llvm-frag
tests/cases/compiler-development/artifact/ok/backend-ownership-drop-runtime-enum.llvm-frag
tests/cases/compiler-development/artifact/ok/backend-ownership-compiler-shaped.llvm-frag
tests/cases/compiler-development/artifact/ok/backend-trait-dispatch.llvm-frag
tests/cases/compiler-development/artifact/ok/c-header-repr-struct.h
tests/cases/compiler-development/artifact/ok/c-header-repr-payload-enum.h
tests/cases/compiler-development/artifact/ok/c-header-generated-aggregates.h
tests/cases/compiler-development/artifact/ok/compiler-shaped-aggregates.ir
tests/cases/compiler-development/artifact/ok/declaration-index-basic.ari
tests/cases/compiler-development/artifact/ok/declaration-index-basic.decls
tests/cases/compiler-development/artifact/ok/declaration-index-generic-aggregate.decls
tests/cases/compiler-development/artifact/ok/declaration-index-project-compiler.decls
tests/cases/compiler-development/artifact/ok/diagnostic-catalog.catalog
tests/cases/compiler-development/artifact/errors/diagnostic-generic-explicit-arity.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-generic-explicit-mismatch.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-generic-duplicate-param.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-generic-inference-conflict.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-generic-uninferred.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-generic-return-context-uninferred.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-generic-signature-unknown-param.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-generic-specialization-name-conflict.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-generic-function-pointer-uninferred.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-generic-function-pointer-mismatch.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-generic-nongeneric-type-args.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-generic-private-function.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-assignment-while-borrowed.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-borrow-after-move.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-double-move.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-enum-payload-invalid-move.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-compact-enum-payload-ref.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-ownership-aggregate-enum-payload.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-borrow-aggregate-enum-payload.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-match-nonexhaustive.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-match-struct-pattern-duplicate-field.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-match-struct-pattern-tuple-struct.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-match-struct-pattern-unknown-field.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-struct-field-unknown.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-ffi-nonrepr-aggregate-import.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-field-assignment-while-borrowed.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-return-live-owner.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-loop-break-live-owner.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-loop-continue-live-owner.diagnostic
tests/cases/compiler-development/artifact/ok/object-aggregate-extern-link.symbols
tests/cases/compiler-development/artifact/ok/object-library-export.symbols
tests/cases/compiler-development/artifact/ok/ownership-aggregate-field-move.ir
tests/cases/compiler-development/artifact/ok/generic-function-compiler-shaped.ir
tests/cases/compiler-development/artifact/ok/project-compiler.ir
tests/cases/compiler-development/artifact/ok/runtime-output-basic.stdout
tests/cases/compiler-development/artifact/ok/runtime-output-trait.stdout
tests/cases/compiler-development/artifact/ok/shared-visibility.symbols
tests/cases/compiler-development/artifact/ok/source-map-crlf.map
tests/cases/compiler-development/artifact/ok/source-map-empty.map
tests/cases/compiler-development/artifact/ok/source-map-file-module.map
tests/cases/compiler-development/artifact/ok/source-map-utf8.map
tests/cases/compiler-development/artifact/ok/token-dump-basic.ari
tests/cases/compiler-development/artifact/ok/token-dump-basic.tokens
tests/cases/compiler-development/artifact/ok/token-dump-crlf.tokens
tests/cases/compiler-development/artifact/ok/token-dump-lexical-surface.tokens
tests/cases/compiler-development/artifact/ok/module-graph-file-module.graph
tests/cases/compiler-development/artifact/ok/pass-summary-basic.summary
tests/cases/compiler-development/artifact/ok/stage-plan-basic.plan
tests/cases/compiler-development/artifact/ok/syntax-control-flow.syntax
tests/cases/compiler-development/artifact/ok/syntax-declarations.syntax
tests/cases/compiler-development/artifact/ok/syntax-dump-basic.syntax
tests/cases/compiler-development/artifact/ok/typed-ir-basic.ir
tests/cases/compiler-development/artifact/errors/diagnostic-ambiguous-module.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-borrow-conflict.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-cyclic-module.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-move-borrowed-owner.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-missing-module.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-ownership-partial-move.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-ownership-vector-dynamic-move.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-ownership-temporary-element-move.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-parser-expected.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-type-assignment.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.ari
tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-unknown-trait.diagnostic
tests/cases/compiler-development/artifact/errors/diagnostic-use-after-move.diagnostic
ari --emit-tokens path
ari --emit-capability-inventory path
ari --list-passes
ari --explain-pass name
ari --list-test-buckets
ari --explain-test-bucket name
ari --list-work-items
ari --explain-work-item name
ari --emit-source-map path
ari --emit-syntax path
ari --emit-diagnostics path
ari --emit-diagnostic-catalog path
ari --emit-module-graph path
ari --emit-declaration-index path
ari --emit-stage-plan path
ari --emit-pass-summary path
ari --emit-typed-ir path
python3 tests/check_compiler_artifacts.py --list-fixtures [all|ok|errors]
make check-compiler-artifacts
```

It currently proves more than two dozen low-level contracts:

- equal expected/actual text passes without output
- repository paths, build paths, temporary names, and pointer addresses
  normalize to stable placeholders
- `--list-fixtures` writes a deterministic inventory of registered ok/error
  artifact fixtures, their groups, and their artifact kinds
- a line mismatch produces a small report naming the fixture, line, expected
  and actual text, and expected/actual line counts
- `--emit-stage-plan` writes deterministic artifact order, owner, first-check,
  and development-gate text directly from the compiler driver
- `--emit-capability-inventory` writes the compiler's implemented, partial,
  planned, and rejected public feature surface with owners and first checks
- `--list-passes` prints compiler pass owners, inputs, outputs, first
  artifacts, and first focused checks without needing an input file
- `--explain-pass sema` prints the sema pass contract used for triage
- `--list-test-buckets` prints compiler fixture buckets and first focused
  checks without needing an input file
- `--explain-test-bucket compiler-artifact-ok` prints when to use the artifact
  golden bucket
- `--list-work-items` prints compiler implementation slices with first files,
  artifacts, and focused checks
- `--explain-work-item generic-aggregate-stress` prints the next compiler work
  contract for a concrete roadmap item
- `--list-capabilities` prints the same compiler feature surface without
  needing an input file
- `--explain-capability trait-resolution` prints the owner, status, first
  focused check, and proof purpose for one compiler area
- `--list-artifacts` prints the available artifact producers without needing a
  source file
- `--explain-artifact --emit-tokens` prints the owner, first focused check,
  and proof purpose for one producer
- artifact CLI misuse names the exact conflicting artifact options, such as
  `--emit-tokens, --emit-syntax`
- `--emit-source-map` writes deterministic source file identity, kind,
  canonical path, display name, EOF offset, line table size, byte offset, line,
  and newline-policy text for root, file-backed modules, empty files, CRLF
  files, and UTF-8 fixtures
- CRLF input bytes are materialized into `build/` from an LF-normalized fixture
  template, `source-map-crlf-template.ari`, so committed sources keep passing
  whitespace checks while source artifacts still prove real CRLF behavior
- `--emit-tokens` writes deterministic lexer output for small and lexical
  surface Ari source files, including comments, strings, literal suffixes,
  compound operators, EOF, CRLF line starts, and UTF-8 byte spans
- `--emit-syntax` writes deterministic parser output with AST `source_id=` and
  byte spans before sema and backend behavior are involved, including a
  declaration-surface fixture with modules, generics, traits, impls, and match,
  plus a control-flow fixture with `init while`, `continue`, `break`, and
  match expression shape
- `--emit-diagnostics` writes a normalized diagnostic artifact for an expected
  compiler failure
- `--emit-diagnostics` classifies representative lexer, parser, module, type,
  and ownership failures with stable diagnostic codes and `family=...` layer names
  for borrow conflicts, use-after-move, moving borrowed owners, partial moves,
  and unsupported dynamic owner-element moves
- `--emit-diagnostics` also writes parseable `Source`, `Label`, `Snippet`,
  `Note`, and `Help` rows. Located rows include `source_id=`, `source=`,
  `line=`, `column=`, `end_line=`, `end_column=`, `byte_start=`, and
  `byte_end=` fields for location-aware tooling.
- Message-only notes and help use `location=none`; located notes and help use
  `location=source` plus the normal span fields.
- `--emit-diagnostic-catalog` writes the current diagnostic code table, owning
  compiler source file, family, and fallback policy
- `--emit-module-graph` writes deterministic file-backed source, import, and
  item-surface facts without running sema or LLVM codegen
- `--emit-declaration-index` writes deterministic declaration signatures,
  visibility, module names, and source locations before sema or LLVM codegen,
  including a compiler-shaped file-backed project and a generic aggregate
  surface with aliases, generic impls, and nested payload declarations
- `--emit-typed-ir` writes deterministic sema-lowered IR for small Ari, core
  scalar flow, trait dispatch, generic function specialization, generic
  aggregate, compiler-shaped struct/enum/match, file-backed module, and
  ownership/drop fixtures without involving LLVM codegen
- `--emit-pass-summary` writes deterministic stage counts for lexing, syntax,
  module loading, and sema
- `--emit-llvm` is checked through review-sized function fragments for core
  control flow, generic function specialization, generic aggregate lowering,
  ownership/drop lowering, and static trait dispatch instead of committing the
  whole runtime-heavy LLVM file
- `tests/extract_symbol_names.py` turns object and linked shared-library symbol
  tables into a deterministic allow-list artifact, so exported Ari symbols,
  hidden helpers, absent `main`, and hidden builtins are reviewed as text
- executable stdout is checked through small runtime-output goldens, including
  one trait-bound dispatch program, after the earlier source, syntax, typed IR,
  and LLVM checks pass

The first typed-IR golden uses `--no-implicit-std` so the fixture protects the
source file's lowered facts instead of recording every implicit prelude
declaration. Add separate std/prelude IR fixtures only when that behavior is
the thing being tested.

This is deliberately small. Future HIR, object-symbol, shared-library, richer
typed-IR, and broader backend artifact producers should plug into the same
shape instead of inventing unrelated golden comparison rules.

## Current Compiler Integration

The current compiler already has useful artifact checks:

- `--check` for frontend and sema diagnostics
- `--emit-source-map` for stable source ids, canonical/display paths, EOF
  offsets, byte offsets, line tables, and snippet text
- `--emit-source-map` for stable byte offset review remains the smallest proof
  before parser or sema behavior is involved
- `--emit-stage-plan` for stable stage-order and first-check routing from the
  compiler binary
- `--emit-capability-inventory` for stable public compiler feature status,
  owners, and next-check routing
- `--emit-diagnostic-catalog` for stable diagnostic code ownership
- `--emit-tokens` for stable lexer token text, source ids, ownership, and byte
  spans
- `--emit-syntax` for stable parser tree text before semantic analysis
- `--emit-diagnostics` for stable expected-failure text with structured
  sources, labels, snippets, notes, and help
- `--emit-module-graph` for stable file-backed source, import, and item-surface
  text before sema or backend behavior are involved
- `--emit-declaration-index` for stable declaration signatures, visibility,
  module names, and source locations before semantic lowering
- `--emit-typed-ir` for stable sema output before LLVM lowering
- `--emit-pass-summary` for quick stage-boundary counts in compiler-development
  tests
- `--emit-llvm` for LLVM text and extracted review-sized function fragments
- `--emit-obj` plus `tests/extract_symbol_names.py` for object symbol goldens
- `--shared` plus `tests/extract_symbol_names.py --dynamic` for linked
  shared-library symbol goldens
- executable exit-code and stdout golden checks where LLVM driver support is
  available

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

Stage comparison remains a compiler maturity blocker, but the artifact bucket
now covers frontend, source identity, diagnostics, module graphs, declarations
for single-file, file-backed, and generic surfaces, typed IR for
modules, ownership, generic functions, generic aggregates, traits, and
compiler-shaped aggregate programs, LLVM fragments, object/shared symbol
goldens, and stdout goldens. The remaining larger gaps are HIR, richer
object/header/relocation inventories, and broader backend/runtime fixture
breadth.
