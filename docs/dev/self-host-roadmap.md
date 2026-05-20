# Self-Host Roadmap

This page defines the path for writing a new Ari compiler in Ari. It is not a
plan to rewrite the current C++ compiler in place. The C++ compiler should act
as `stage0`: a trusted bootstrap compiler used to compile and test the new Ari
implementation until Ari can compile itself.

Use [Bootstrap Readiness](bootstrap-readiness.md) as the practical start gate.
That page tracks the current estimate, the minimum language and standard
library surface, the first test layout, and the "do not start yet" boundaries.

## Current Verdict

Ari is not ready for full self-hosting yet.

Current practical estimate:

- **38-43% ready** to begin full compiler bootstrapping
- **57-62% remaining** before a self-host attempt is likely to be productive

The language and standard library are mature enough to start isolated
compiler-component experiments in Ari, especially lexing, source-coordinate
values, borrowed source text lookup, cached line maps, bounded borrowed source
maps, first diagnostic values with notes, diagnostic formatting, small parser
utilities, and golden-test tools.
They are not yet mature enough to build the whole compiler, type checker,
ownership checker, module graph, and backend in Ari without repeatedly falling
back to C++ changes.

That distinction matters:

- Ready now: write small Ari libraries and tools that process source text.
- Near-term: write a new Ari lexer and parser frontend beside the current
  compiler.
- Not ready yet: replace the full compiler pipeline and self-compile the new
  compiler.

## Stage0 Policy

The self-host track should avoid modifying the existing C++ compiler just to
make the new compiler easier to write. Treat the C++ compiler as a compatibility
target and a bootstrap tool.

Allowed C++ work on this track:

- fixing a real miscompile of already-supported Ari source
- fixing diagnostics that block ordinary Ari users
- exposing a documented language or standard-library feature that is already
  on the main roadmap

Avoid C++ work on this track:

- adding one-off syntax only for the self-host compiler
- adding compiler-known helpers when the same behavior can live in Ari source
- changing the stage0 output format just to simplify stage1 tests
- porting C++ implementation files incrementally instead of building the new
  Ari compiler as a separate Ari project

If stage1 needs a feature that Ari does not support yet, prefer one of these
responses:

1. simplify the stage1 design so it uses current Ari
2. add the need to this roadmap as a blocker
3. implement the feature as ordinary Ari standard-library code
4. only then consider a normal language/compiler roadmap item

## Definition Of Self-Hosted

Ari counts as self-hosted when all of these are true:

1. `stage0` is the current C++ Ari compiler.
2. `stage0` compiles the Ari-written compiler source into `stage1`.
3. `stage1` compiles the same Ari-written compiler source into `stage2`.
4. `stage1` and `stage2` compile a shared fixture suite with matching
   diagnostics and matching backend artifacts, or with an explicitly documented
   normalized comparison format.
5. Stage1 no longer depends on generated C++ or C++ compiler internals.

Early stages may compare text artifacts such as tokens, parsed trees, HIR, or
LLVM IR before executable output is stable.

## Proposed Project Shape

Do not create this tree until the first real Ari component lands, but keep the
shape stable once it exists:

```text
bootstrap/
  README.md
  Makefile
  stage1/
    src/
      main.ari
      lex/
      parse/
      diag/
      syntax/
      hir/
      sema/
      ir/
      emit/
    tests/
      lex/
      parse/
      diag/
      fixtures/
```

The initial `Makefile` should only call `build/ari`, run focused fixtures, and
compare checked outputs. A package manager can replace this later, but the
bootstrap path should not wait for one.

## Readiness Matrix

| Area | Current Readiness | Needed Before Full Self-Host |
| --- | --- | --- |
| Source IO | Good enough for small tools through `std::fs`, `std::path`, `std::env`, and `std::io`. | Directory walking, richer metadata, canonical paths, stable owned path buffers, and better file errors. |
| Text | Good enough for byte strings, UTF-8 validation, C strings, OS string views, formatting, and encoding helpers. | Owned UTF-8/OS/path string policies, richer parse errors, and less special-case formatting lowering. |
| Collections | Good first pass: `Vec`, `Slice`, hash/tree maps and sets, queues, lists, heaps, and iterators exist. | Trait-driven `Hash`/`Eq`/`Ord`, deletion/comparator policy completion, stress tests, and cleaner nested generic aggregate behavior. |
| Diagnostics | Basic logging, panic, formatting, test helpers, `std::diag` diagnostic values with one borrowed note, and `std::source` source-coordinate values plus borrowed `SourceFile`, cached `LineMap`, and bounded borrowed `SourceMap` lookup exist. | Owned filename/source-text maps, multi-label diagnostic builders, multiple notes, fix-it text, and stable golden output. |
| Parser support | Ari can express basic parser code with structs, enums, loops, functions, modules, and collections. | More ergonomic strings, file-backed modules, better generic aggregate diagnostics, and parser-specific tests. |
| Semantic model | Current compiler supports many front-end surfaces, but not all are comfortable to reimplement in Ari. | Stable generic aggregates, trait dispatch, associated-type ergonomics, richer `Result[T, Error]`, and clearer ownership errors. |
| Memory model | Explicit zones, provenance checks, source `String`/`Vec`, and low-level memory helpers exist. | Stronger long-lived arena policy, escape rules for compiler-owned graphs, drop policy for nested structures, and allocation profiling hooks. |
| Backend | Current compiler emits LLVM IR and can link through LLVM drivers. | A stage1 backend strategy: initially emit comparable HIR/IR text, then LLVM IR text, then object/executable output. |
| Build/test | `make` and focused stdlib/compiler checks exist. | Bootstrap-specific fixture runner, stage comparison, golden updater policy, and a small self-compile smoke path. |

## Milestones

### Milestone 0: Freeze The Contract

- Keep the current C++ compiler as stage0.
- Document exactly which Ari language surface stage1 may rely on.
- Add bootstrap fixtures before adding stage1 code.
- Keep all bootstrap checks focused and runnable from a `Makefile`.

Exit criteria:

- A contributor can tell whether a failure is a stage0 bug, a stage1 bug, or an
  unsupported feature request.

### Milestone 1: Ari Lexer

- Implement a standalone Ari lexer in Ari.
- Represent tokens, spans, and lexical diagnostics in Ari data structures.
- Compare stage1 token output against golden fixtures.
- Cover string literals, char literals, comments, operators, identifiers,
  keywords, numbers, and error recovery.

Exit criteria:

- `stage0` compiles the Ari lexer tool.
- The lexer tool runs on the language fixture set.
- Golden token and diagnostic outputs are stable.

### Milestone 2: Ari Parser Frontend

- Build a parser over the Ari lexer.
- Start with declarations, functions, modules, statements, expressions, and
  types.
- Emit a stable syntax tree dump, not backend code.
- Keep recovery behavior deterministic enough for golden diagnostics.

Exit criteria:

- Parser fixtures cover accepted and rejected syntax.
- The parser can parse the current standard-library source files enough to
  produce syntax tree dumps.

### Milestone 3: HIR And Name Resolution

- Lower parsed syntax into a simpler HIR.
- Resolve modules, imports, item visibility, and qualified paths.
- Add a symbol table and interned names.
- Keep error recovery good enough to report multiple diagnostics.

Exit criteria:

- Stage1 can build a module graph for selected stdlib files.
- Name-resolution golden tests match the documented behavior.

### Milestone 4: Type And Ownership Subset

- Type-check scalar values, functions, structs, enums, generics, and common
  `std` ADTs.
- Add ownership and borrow checks needed by the stage1 compiler code itself.
- Start with the subset used by stage1 instead of the entire language.

Exit criteria:

- Stage1 type-checks its own lexer/parser/HIR modules.
- Stage1 rejects focused ownership and type errors with stable diagnostics.

### Milestone 5: Stage1 IR

- Emit a stage1 IR or normalized textual form that can be compared in tests.
- Do not require executable output yet.
- Keep the IR simple enough to debug without the C++ compiler internals.

Exit criteria:

- Stage1 emits comparable IR for a fixture suite.
- Stage0 and stage1 frontend behavior can be compared without manual reading.

### Milestone 6: LLVM Text Backend

- Emit LLVM IR text from Ari source for a narrow executable subset.
- Use the existing system LLVM driver, such as `clang-21`, for link/run tests.
- Reuse the documented Ari ABI and symbol mangling rules.

Exit criteria:

- Stage1 compiles small programs to LLVM IR.
- Linked executables match expected exit codes or stdout.

### Milestone 7: First Self-Compile

- Use stage0 to build stage1.
- Use stage1 to build stage2 from the same source.
- Compare stage1 and stage2 normalized outputs.

Exit criteria:

- The Ari-written compiler can rebuild itself for the supported subset.
- Differences are either empty or explained by a documented normalization step.

## Minimum Standard Library Surface For Stage1

The first Ari compiler should rely only on a conservative subset:

- `Option`, `Result`, and small error values
- `Slice`, `Vec`, `String`, and explicit `Zone`
- `HashMap` or `TreeMap` with explicit hash/comparator functions
- `std::fmt` and `format_in!` for diagnostics
- `std::fs` read/write helpers and `std::path` lexical helpers
- `std::env` args/current-directory helpers
- `std::encoding` and `std::ascii` for source text validation
- `std::test` only for stage1's own Ari tests

Avoid depending on threads, networking, dynamic loading, raw OS wrappers,
shared ownership, or freestanding/kernel APIs until the single-threaded hosted
compiler path is stable.

## What Still Blocks Full Self-Host

- first-class source span and diagnostic APIs
- stable owned compiler data structures over zones
- stronger generic aggregate and enum payload behavior
- trait-driven collection defaults and formatting dispatch
- file-backed modules and multi-file build ergonomics
- clear `Result[T, Error]` payload policy
- bootstrap fixture runner and stage comparison tooling
- backend emission strategy for stage1

These blockers are real, but they are not reasons to wait. They define the
order: lexer first, parser second, then HIR/name resolution, then type checking,
then backend.
