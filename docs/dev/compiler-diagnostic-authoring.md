# Compiler Diagnostic Authoring

This page explains how to add or change Ari compiler diagnostics. It is for
ordinary hosted-compiler development, not bootstrap implementation.

Diagnostics are part of the language design. A good diagnostic tells users what
rule they hit, points at the smallest useful source span, and gives maintainers
a stable test surface.

## When To Add A Diagnostic

Add or refine a diagnostic when:

- unsupported syntax is currently accepted too far into the pipeline
- an error points at the wrong file, byte span, line, or column
- a parser or sema error is too generic to guide the user
- a compiler artifact needs a stable expected-failure output
- a feature moves from front-end-only to executable and needs clearer failure
  modes

Do not hide a real language rule behind a backend crash or a generic
`ari/compiler` fallback when the compiler layer already knows what failed.

## Diagnostic Shape

Each stable diagnostic should be designed as data before text:

| Field | Rule |
| --- | --- |
| Code | Use a stable family such as `L0001`, `P0001`, `M0001`, `T0001`, `O0001`, `I0001`, or `B0001` when the rule is durable. |
| Family | Emit the owning layer name such as `lexer`, `parser`, `module`, `type`, `ownership`, `ir`, or `backend`. |
| Severity | Use `error` for rejected source; reserve warnings until warning policy is explicit. |
| Message | State the rule in user language, not internal helper names. |
| Primary label | Point at the smallest source span that caused the error. |
| Secondary labels | Point at related declarations, previous moves, borrowed values, or module roots. |
| Notes | Explain extra context that does not fit in the main message. |
| Help | Suggest one concrete next action when Ari has an obvious correction. |

Rendering belongs at the edge. Lexer, parser, resolver, sema, and backend code
should build diagnostic data or a transitional equivalent, not format long
reports inline.

## Message Style

Use direct, stable wording:

- prefer `expected expression after '='` over `parse failed`
- prefer `cannot move 'value' while it is borrowed` over `ownership error`
- prefer `module 'math' was not found on the module path` over `bad module`
- prefer `trait 'Score' is not implemented for Point` over `method lookup failed`

Avoid implementation terms unless the user wrote something that directly maps
to that term. `AST`, `typed IR`, and `monomorphization` belong in developer
artifacts, not normal user-facing messages.

## Source Spans And Labels

Span policy:

- `Span.start` is inclusive and `Span.end` is exclusive
- internal offsets are byte offsets
- rendered line and column values are one-based
- a diagnostic should have one primary label unless a future policy explicitly
  allows multiple primaries
- secondary labels should explain relationships, not decorate the same span

Use labels to show causality:

| Error | Primary Label | Secondary Label |
| --- | --- | --- |
| Parser expected expression | Missing expression position | Optional previous token |
| Unknown module | `mod name;` declaration | Module search path or package root |
| Borrow conflict | Illegal move or assignment | Previous borrow source |
| Trait method missing | Method call | Candidate type or trait declaration |
| Backend artifact failure | Requested output option | Tool path or target triple |

## Notes And Help

Use notes for context:

- why a feature is front-end-only
- which module path was searched
- which trait bound was required
- which owner or borrow is still live

Use help text for one concrete next step:

- `try adding ';' after this statement`
- `mark the field as 'mut' before assigning to it`
- `pass -I path or --module-path path for file-backed modules`
- `use --emit-llvm to inspect backend lowering before linking`

Do not use help text to list every possible fix.

## Codes And Families

Start with families, then add individual codes when behavior is stable:

| Family | Owns |
| --- | --- |
| `L0001` | lexer characters, escapes, and invalid tokens |
| `P0001` | parser grammar and recovery |
| `M0001` | modules, imports, visibility, metadata, and caches |
| `T0001` | types, traits, methods, and generic constraints |
| `O0001` | ownership, borrowing, moves, drops, and zones |
| `I0001` | typed IR lowering and resolved compiler facts |
| `B0001` | LLVM, object, executable, shared library, and artifact emission |

The diagnostic artifact prints both fields, for example
`code=P0001 family=parser`. The code is the stable search key; the family tells
contributors which compiler layer should usually own the first fix.

Once a code is documented in a golden artifact, do not reuse it for a different
rule.

## Golden Tests

For expected failures, prefer the smallest observable artifact:

| Layer | Artifact |
| --- | --- |
| Lexer | `--emit-diagnostics` with an invalid character fixture |
| Parser | `--emit-diagnostics` with a malformed syntax fixture |
| Module resolver | `--emit-diagnostics` or `--emit-module-graph` |
| Sema | `--emit-diagnostics` or future typed fact artifact |
| IR/backend | `--emit-typed-ir`, `--emit-llvm`, object, or executable smoke |

Put diagnostic golden files under
`tests/cases/compiler-development/artifact/errors/`. Keep paths normalized and
messages deterministic. Run `make check-compiler-artifacts` when artifact text
changes and `make check-compiler-dev-docs` when this policy changes.

Full `make check` belongs at handoff for broad changes. Sanitizer checks are
intentionally separate.

## Review Checklist

Before handing off a diagnostic change, answer:

- Which layer detected the error first?
- Is the diagnostic code family correct?
- Does the message describe the Ari rule in user language?
- Does the primary label point at the smallest useful span?
- Do secondary labels show real causality?
- Is there a note or help only when it reduces user guesswork?
- Is the golden artifact in the closest test bucket?
- Did the user-facing docs change if the language rule became clearer?

Good diagnostics make Ari easier to use today and make future compiler work
less surprising without introducing bootstrap-only behavior.
