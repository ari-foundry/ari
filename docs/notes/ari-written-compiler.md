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

The Ari-written compiler source work lives in `compiler/`. Its planning lives in
this note. When hosted compiler behavior blocks the Ari-written compiler, record
the blocker here and then use the relevant `docs/dev/` page to fix the hosted
compiler feature in the normal focused-test workflow.

## Current Status

- `compiler/` has been started as a direct Ari source root.
- The initial files model source spans, token kinds, diagnostics, and a tiny
  one-character lexer classification path.
- `compiler/token.ari` models simple delimiter punctuation tokens for
  parentheses, braces, comma, colon, and semicolon.
- `compiler/token.ari` models simple one-character operator tokens for
  assignment and arithmetic operators.
- `compiler/source.ari` exposes small span query helpers for downstream phase
  payload smokes.
- `compiler/lexer.ari` now has a small `LexResult` flow for one-character scans
  that can return either a token or a shared `diagnostic::Diagnostic`
  invalid-character failure.
- `compiler/lexer.ari` has a tiny `TokenCursor` shape over one scanned token
  and an EOF advance path. It is a placeholder for phase handoff, not a real
  source-text stream.
- `compiler/lexer.ari` exposes small cursor accessors for the current token
  span, token score, and done state.
- `compiler/lexer.ari` has a minimal `TokenHandoff` shape that carries one real
  token cursor plus an explicit EOF observation.
- `compiler/lexer.ari` has a fixed two-token `TokenStream` shape that carries
  two scanned token cursors plus EOF as the first step beyond one-token
  handoff smokes.
- `compiler/lexer.ari` exposes a tiny malformed handoff helper for checking the
  parser's missing-EOF diagnostic path without making malformed handoffs normal
  driver input.
- `compiler/lexer.ari` exposes token-kind query helpers at the cursor and
  handoff boundary so later parser work does not need to import token internals.
- `compiler/lexer.ari` exposes an explicit unknown-token query at the cursor
  and handoff boundary.
- `compiler/lexer.ari` classifies simple delimiter punctuation separately from
  unknown characters and exposes punctuation queries at the cursor and handoff
  boundary.
- `compiler/lexer.ari` classifies `.` as punctuation so field-access tokens no
  longer fall through the unknown-token path.
- `compiler/lexer.ari` classifies the stage0 dot-run punctuation family
  `..`, `..=`, and `...` with source-text longest-match behavior.
- `compiler/lexer.ari` classifies `[` and `]` as punctuation so generic
  argument and future indexing tokens no longer fall through the unknown-token
  path.
- `compiler/token.ari` centralizes token-kind class queries so lexer cursor
  helpers do not need to repeat a full token-kind match for every public query.
- `compiler/lexer.ari` classifies `"` as punctuation so string-literal
  delimiter tokenization can start without falling through the unknown-token
  path.
- `compiler/lexer.ari` classifies `@` as punctuation so attribute and alias
  marker tokenization no longer falls through the unknown-token path.
- `compiler/lexer.ari` classifies simple one-character operators separately
  from unknown characters and exposes operator queries at the cursor and
  handoff boundary.
- `compiler/lexer.ari` classifies `~` as a one-character operator so
  bitwise-not tokenization no longer falls through the unknown-token path.
- `compiler/lexer.ari` classifies simple compound-assignment operators `+=`,
  `-=`, `*=`, `/=`, and `%=` as fixed-width two-character operators.
- `compiler/lexer.ari` classifies bitwise compound-assignment operators `&=`,
  `|=`, and `^=` while preserving `&&` and `||` logical-operator priority.
- `compiler/lexer.ari` classifies shift compound-assignment operators `<<=`
  and `>>=` with source-text longest-match behavior while preserving `<<`,
  `>>`, `<=`, and `>=`.
- `compiler/lexer.ari` classifies exact source-text `fn`, `let`, `var`, `own`,
  `ref`, `mut`, `ptr`, and `return` as keywords while preserving longer
  identifier runs such as `fn1`, `letter`, `variant`, `owner`, `reference`,
  `mutable`, `ptrace`, and `returning` as identifiers.
- `compiler/lexer.ari` classifies `?` and `??` as operators so
  result-propagation and null-coalescing tokens match the stage0 spellings.
- `compiler/lexer.ari` exposes one `scan_two`/`cursor_from_two` path for all
  current two-character scans instead of one public helper per token spelling.
- The two-character lexer path covers fixed-width two-character spellings:
  `::`, `??`, `==`, `=>`, `!=`, `<=`, `>=`, `&&`, `||`, `<<`, `>>`, `->`,
  `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, and `^=`
  with one-character fallback behavior.
- `compiler/lexer.ari` can scan a real `Slice[u8]` source text from an offset,
  including multi-byte identifier, number, and whitespace runs, two-character
  tokens, one-character fallback tokens, and EOF at the source end.
- `compiler/lexer.ari` exposes text-backed cursor advance and handoff helpers,
  including significant-token advance that skips leading and trailing
  whitespace around the current single-statement parser handoff.
- `compiler/lexer.ari` classifies one-character comparison operators `!`, `<`,
  and `>` and uses them as fallbacks for two-character comparison operators.
- `compiler/lexer.ari` classifies one-character bitwise operators `&`, `|`,
  and `^` and uses `&` and `|` as fallbacks for logical `&&` and `||`.
- `compiler/ast.ari` now models minimal span-carrying token, statement, error,
  and missing output nodes.
- `compiler/ast.ari` exposes a statement-kind query helper so parser payload
  shape can be checked without relying on aggregate field access or score
  arithmetic.
- `compiler/ast.ari` exposes a node span-length query helper so parser payload
  spans can be checked without relying on `ast::node_score` arithmetic.
- `compiler/ast.ari` exposes a node value query helper so parser payload values
  can be checked without relying on `ast::node_score` arithmetic.
- `compiler/ast.ari` exposes a node start-offset query helper so parser payload
  offsets can be checked without relying on `ast::node_score` arithmetic.
- `compiler/ast.ari` exposes a node end-offset query helper so parser payload
  offsets can be checked without relying on `ast::node_score` arithmetic.
- `compiler/ast.ari` exposes a node source-id query helper so parser payload
  source identities can be checked without relying on `ast::node_score`
  arithmetic.
- `compiler/parser.ari` exists as a phase-boundary skeleton that consumes the
  lexer handoff shape, can classify the current handoff token, and returns
  either a statement-shaped `ast::Node` over the current token span or a shared
  diagnostic failure.
- `compiler/parser.ari` only treats identifier and number handoff tokens as
  statement skeletons; EOF, whitespace, and unknown tokens now stay on
  diagnostic paths.
- `compiler/parser.ari` exposes a `parse_is_success` helper so downstream
  phases can distinguish `Parsed` from diagnostic `Failed` without using
  smoke-test score values.
- `compiler/parser.ari` exposes a parser statement-node query helper so
  downstream smokes can inspect successful payload shape through the parser
  phase boundary.
- `compiler/parser.ari` exposes a parser statement span-length helper so
  downstream smokes can inspect successful payload spans through the parser
  phase boundary.
- `compiler/parser.ari` exposes a parser statement value helper so downstream
  smokes can inspect successful payload values through the parser phase
  boundary.
- `compiler/parser.ari` exposes a parser statement start-offset helper so
  downstream smokes can inspect successful payload offsets through the parser
  phase boundary.
- `compiler/parser.ari` exposes a parser statement end-offset helper so
  downstream smokes can inspect successful payload offsets through the parser
  phase boundary.
- `compiler/parser.ari` exposes a parser statement source-id helper so
  downstream smokes can inspect successful payload source identities through
  the parser phase boundary.
- `compiler/diagnostic.ari` exposes a diagnostic-code accessor, and
  `compiler/parser.ari` exposes a parser failure-code helper for phase
  boundaries that need diagnostic identity without rendering diagnostics.
- `compiler/diagnostic.ari` exposes a diagnostic start-offset accessor, and
  `compiler/parser.ari` exposes a parser failure start-offset helper for phase
  boundaries that need diagnostic location metadata without rendering
  diagnostics.
- `compiler/diagnostic.ari` exposes a diagnostic end-offset accessor, and
  `compiler/parser.ari` exposes a parser failure end-offset helper for phase
  boundaries that need diagnostic location metadata without rendering
  diagnostics.
- `compiler/diagnostic.ari` exposes a diagnostic severity-score accessor, and
  `compiler/parser.ari` exposes a parser failure severity-score helper for
  phase boundaries that need diagnostic severity metadata without rendering
  diagnostics.
- `compiler/parser.ari` exposes a tiny `parse_one_eof` helper so EOF-cursor
  diagnostics can be tested without exporting or passing nested lexer cursor
  types across module paths.
- `compiler/parser.ari` exposes a tiny `parse_one_without_eof` helper so
  malformed handoff diagnostics can be tested without exposing handoff internals
  to the bootstrap fixture.
- The bootstrap source-root smoke checks the parser empty-input diagnostic code
  through `parser::parse_failure_code` instead of relying only on diagnostic
  smoke-score arithmetic.
- The bootstrap source-root smoke checks the parser EOF-cursor diagnostic code
  through `parser::parse_failure_code` instead of relying only on diagnostic
  smoke-score arithmetic.
- The bootstrap source-root smoke checks the parser unknown-token diagnostic
  code through `parser::parse_failure_code` instead of relying only on driver
  indirection or diagnostic smoke-score arithmetic.
- The bootstrap source-root smoke checks the parser missing-EOF handoff
  diagnostic code through `parser::parse_failure_code` instead of relying only
  on parser smoke-score arithmetic.
- The bootstrap source-root smoke checks the parser whitespace diagnostic start
  offset through `parser::parse_failure_start_offset` instead of relying only
  on diagnostic smoke-score arithmetic.
- The bootstrap source-root smoke checks the parser unknown-token diagnostic
  start offset through `parser::parse_failure_start_offset` instead of relying
  only on diagnostic smoke-score arithmetic.
- The bootstrap source-root smoke checks the parser whitespace diagnostic end
  offset through `parser::parse_failure_end_offset` instead of relying only on
  diagnostic smoke-score arithmetic.
- The bootstrap source-root smoke checks the parser unknown-token diagnostic
  end offset through `parser::parse_failure_end_offset` instead of relying only
  on diagnostic smoke-score arithmetic.
- The bootstrap source-root smoke checks the parser whitespace diagnostic
  severity through `parser::parse_failure_severity_score` instead of relying
  only on diagnostic smoke-score arithmetic.
- The bootstrap source-root smoke checks the parser number statement success
  path through `parser::parse_is_success` instead of relying only on parser
  smoke-score arithmetic.
- The bootstrap source-root smoke checks successful parser payloads are
  statement nodes without relying on `ast::node_score` arithmetic.
- The bootstrap source-root smoke checks number parser payloads are statement
  nodes without relying on parser score arithmetic.
- The bootstrap source-root smoke checks successful parser payload span length
  without relying on `ast::node_score` arithmetic.
- The bootstrap source-root smoke checks number parser payload span length
  without relying on parser score arithmetic.
- The bootstrap source-root smoke checks successful parser payload values
  without relying on `ast::node_score` arithmetic.
- The bootstrap source-root smoke checks number parser payload values without
  relying on parser score arithmetic.
- The bootstrap source-root smoke checks successful parser payload start offsets
  without relying on `ast::node_score` arithmetic.
- The bootstrap source-root smoke checks number parser payload start offsets
  without relying on parser score arithmetic.
- The bootstrap source-root smoke checks successful parser payload end offsets
  without relying on `ast::node_score` arithmetic.
- The bootstrap source-root smoke checks number parser payload end offsets
  without relying on parser score arithmetic.
- The bootstrap source-root smoke checks successful parser payload source ids
  without relying on `ast::node_score` arithmetic.
- The bootstrap source-root smoke checks number parser payload source ids
  without relying on parser score arithmetic.
- `compiler/driver.ari` owns the current bootstrap entry flow and returns a
  standard-library `std::Result[i64, i64]` instead of embedding smoke arithmetic
  in `main`.
- `compiler/driver.ari` can read a source file path through `std::fs` and can
  use `std::context` argv when the executable is invoked with a file argument.
- `compiler/source.ari` has a minimal `LoadedSourceSummary` for source id,
  byte length, first byte, and first-byte offset. It is metadata for the
  legacy scalar driver smoke, not a source table or owned text buffer.
- `compiler/parser.ari` can parse a source-text slice through the text-backed
  lexer handoff path.
- `compiler/driver.ari` routes file and text input through `std::string::String`
  / `Slice[u8]` source text and preserves parser diagnostic codes from that
  path.
- `compiler/driver.ari` now uses the parser success helper before returning
  success, so diagnostic parse results no longer count as successful driver
  runs only because they have positive smoke scores.
- `compiler/driver.ari` now preserves parser failure diagnostic codes for the
  current one-token handoff path instead of collapsing them to generic driver
  error `1003`.
- `compiler/driver.ari` exposes a `result_code` helper for bootstrap tests and
  later internal plumbing that need to inspect either `Ok` or `Err` payloads
  without using CLI exit-code mapping.
- `compiler/driver.ari` exposes a scalar `DriverInput` constructor helper so
  bootstrap callers do not need to construct the public input shape with
  module-qualified aggregate literals.
- The bootstrap source-root smoke checks the default driver success path with
  both CLI-style `exit_code` mapping and raw `result_code` payload inspection.
- The bootstrap source-root smoke checks the file-input driver success path
  with both CLI-style `exit_code` mapping and raw `result_code` payload
  inspection.
- The bootstrap source-root smoke checks the loaded-source driver success path
  with both CLI-style `exit_code` mapping and raw `result_code` payload
  inspection.
- The bootstrap source-root smoke checks the raw `DriverInput` success path
  with raw `result_code` payload inspection.
- The bootstrap source-root smoke checks the source-text driver success path
  with raw `result_code` payload inspection.
- The bootstrap source-root smoke covers the current `DriverInput` offset guard
  errors for both invalid start offsets and invalid one-byte end bounds through
  the scalar constructor helper.
- The bootstrap source-root smoke covers the current driver missing-file path
  and checks that file-read failures preserve driver error code `1102`.
- The bootstrap source-root smoke covers the current empty source-text driver
  path and checks that text-input validation preserves driver error code
  `1101`.
- File-input smoke paths use explicit `zone(65536)` allocation blocks because
  the source-root fixture is now large enough to exceed the default zone
  capacity when read into an owned string.
- The bootstrap source-root smoke now covers both valid loaded-source handoff
  and an out-of-range first-byte offset error from the driver path.
- `compiler/main.ari` is now a thin entrypoint that delegates to the driver and
  maps the driver's result to an exit code.
- `make check-ari-compiler-bootstrap` checks each `compiler/*.ari` module,
  checks a small `tests/cases/ari-compiler-bootstrap/` fixture with
  `-Icompiler`, and, when an LLVM driver is available, builds and runs the
  source-root smokes. The compiled `compiler/main.ari` file-input smoke reads
  `tests/cases/ari-compiler-bootstrap/input/one-token.ari` because the
  Ari-written parser is not yet a full Ari source-file parser.
- Each module is kept small enough to check directly with the stage0 compiler.
- No full Ari-written parse tree, semantic checker, IR, codegen, source table,
  or real source loader exists yet beyond the minimal parser-output node model
  and file-input driver smoke.

## Incremental Roadmap

1. Keep the five-file source skeleton checking with stage0.
2. Grow `lexer.ari` from a one-token cursor into a small token stream over the
   source representation Ari can support today.
3. Grow `parser.ari` only as token handoff proves more parser-shaped data.
4. Add `ast.ari` and later phase-shaped model files only when the
   previous phase has checked output worth passing forward.
5. Grow source loading from the current `std::string::String` / `Slice[u8]`
   text path into a source table with source ids, file paths, and diagnostic
   locations.
6. Add semantic model stubs before any lowering or backend work.
7. Continue to use the C++ hosted compiler as stage0 until the Ari-written
   compiler can check meaningful multi-file inputs itself.

## Phase Architecture

Do not blindly copy the current C++ hosted compiler architecture. In
particular, do not recreate a giant `sema`-style module that owns module
loading, name lookup, type inference, type checking, ownership, lowering, and
IR generation together.

Prefer compiler phase and responsibility boundaries:

- `source` / `span` / source identity
- `diagnostic`
- `token`
- `lexer`
- `parser`
- `ast`
- `module_loader`
- `name_resolve`
- `type_infer`
- `type_check`
- `ownership`
- `hir` or another checked tree
- `lower`
- `ir`
- `codegen`
- `driver`
- later package/build integration

The early files can stay flat under `compiler/` while the project is tiny.
When the file count grows, organize by Ari module structure and phase ownership,
not by recreating the C++ source layout.

Keep these phase boundaries explicit:

- `module_loader` should load and identify source units, not perform type
  checking.
- `name_resolve` should resolve names and imports, not silently become the whole
  semantic pipeline.
- `type_infer` should be separate from final `type_check` validation where the
  language can express that cleanly.
- `ownership` should become an explicit phase boundary once checked trees can
  carry enough ownership facts.
- `lower` should translate checked forms to IR so codegen does not re-resolve
  source-level names.
- `codegen` should consume lowered/typed forms instead of owning frontend
  decisions.
- `driver` should orchestrate phases, options, and artifacts, not contain core
  compiler logic.

Also avoid the opposite mistake: do not split the compiler around tiny
container or utility concepts such as `vec`, `box`, or `map`. Those are library
and data-structure details, not compiler phase boundaries.

Phase outputs should be explicit enough that future tooling, LSP support,
package-manager integration, and self-host debugging can inspect them.

## Future Package Manager Transition

The current `compiler/` layout is an early bootstrap source-root layout. It is
not pretending to be the final package-manager-era project shape.

A future Ari package manager or build tool may reorganize:

- compiler module layout and package boundaries
- build metadata and target selection
- test discovery and fixture grouping
- dependency resolution and module search paths
- artifact naming, cache locations, and generated-output policy
- integration points for LSP, formatting, and self-host debugging

Do not design or add a cargo-like tool in this bootstrap slice. Keep the current
work compatible with the existing stage0 compiler, normal file-backed modules,
and focused Make targets. When a package manager exists, migrate from the flat
`compiler/*.ari` source root deliberately instead of growing hidden package
policy in ad hoc compiler files.

## Completed Tasks

- Started the direct `compiler/` Ari source root with `main`, `source`,
  `token`, `diagnostic`, and `lexer` modules.
- Consolidated bootstrap planning in this note and linked it from the docs
  index.
- Documented phase-oriented architecture, future package-manager transition
  points, and the rule against a giant `sema` module.
- Added one-character `LexResult` scanning and switched lexer failures to the
  shared `diagnostic::Diagnostic` payload.
- Added a tiny one-token `TokenCursor` shape as a checked lexer handoff model.
- Added a minimal `parser.ari` phase-boundary skeleton with success and
  diagnostic failure paths.
- Added a focused Ari compiler bootstrap test target and
  `tests/cases/ari-compiler-bootstrap/` source-root smoke fixture.
- Added a minimal `ast.ari` node model and connected parser success results to
  it.
- Added cursor token accessors and a minimal statement-shaped parser output
  node.
- Added a minimal `TokenHandoff` carrying one real token plus EOF, and routed
  `parser::parse_one` through that handoff.
- Added a fixed two-token lexer stream shape with first, second, length, and
  EOF accessors, plus source-root smoke coverage for second-token offsets and
  EOF placement.
- Added lexer classification for simple delimiter punctuation tokens and
  source-root smoke coverage that they are scanned, scored, and exposed through
  punctuation queries instead of unknown-token paths.
- Added a focused `.` punctuation token and source-root smoke coverage that it
  is punctuation, not an operator or unknown token.
- Added the stage0 dot-run punctuation family (`..`, `..=`, and `...`) with
  source-root smoke coverage for source-text longest-match behavior, cursor
  advance, and the existing `..` two-character helper path.
- Added focused `[` and `]` punctuation tokens and source-root smoke coverage
  that they are punctuation, not operators or unknown tokens.
- Centralized token-kind class queries in `compiler/token.ari` and routed
  lexer cursor classification helpers through those shared predicates instead
  of repeating full token-kind matches in `compiler/lexer.ari`.
- Added a focused `"` delimiter token and source-root smoke coverage that it is
  punctuation, not an operator or unknown token.
- Added a focused `@` punctuation token and source-root smoke coverage that it
  is punctuation, not an operator or unknown token, while unknown-token smokes
  now use `$` as a still-unknown character.
- Added a focused `::` path separator token and source-root smoke coverage that
  it falls back to the one-character colon token when the second character does
  not match.
- Added lexer classification for simple one-character operator tokens and
  source-root smoke coverage that they are scanned, scored, and exposed through
  operator queries instead of unknown-token paths.
- Added focused `?` and `??` operator tokens and source-root smoke coverage
  that `??` is a fixed-width two-character spelling while `?` remains the
  one-character fallback.
- Added focused text-backed identifier, number, and whitespace span smokes so
  variable-width token runs are checked through `Slice[u8]` source input rather
  than through the fixed-width two-character spelling helper.
- Added a focused `==` equality operator helper and source-root smoke coverage
  that distinguishes it from one-character assignment.
- Added a focused `=>` fat-arrow operator token and source-root smoke coverage
  that it falls back to the one-character assignment token when the second
  character does not match.
- Added one-character comparison operator tokens for `!`, `<`, and `>`, with
  source-root smoke coverage that they are scanned and exposed as operators.
- Added a focused `~` bitwise-not operator token and source-root smoke coverage
  that it is an operator, not punctuation or unknown.
- Added focused `!=`, `<=`, and `>=` comparison operator tokens and source-root
  smoke coverage that they fall back to one-character comparison tokens when
  the second character is not `=`.
- Added one-character bitwise operator tokens for `&`, `|`, and `^`, with
  source-root smoke coverage that they are scanned and exposed as operators.
- Added focused `&&` and `||` logical operator tokens and source-root smoke
  coverage that they fall back to one-character bitwise tokens when the second
  character does not match.
- Added focused `<<` and `>>` shift operator tokens and source-root smoke
  coverage that they fall back to one-character comparison tokens when the
  second character does not match.
- Added a focused `->` arrow operator token and source-root smoke coverage that
  it falls back to the one-character minus token when the second character does
  not match.
- Added focused `+=`, `-=`, `*=`, `/=`, and `%=` compound-assignment operator
  tokens and source-root smoke coverage that `+=` falls back to the
  one-character plus token when the second character does not match.
- Added focused `&=`, `|=`, and `^=` bitwise compound-assignment operator
  tokens and source-root smoke coverage that `&&` and `||` still take logical
  operator priority while `&` falls back to the one-character bitwise token.
- Added focused `<<=` and `>>=` shift compound-assignment operator tokens and
  source-root smoke coverage for source-text longest-match behavior while
  preserving `<<` and `<=` fallbacks.
- Added the first text-backed keyword token, `fn`, with source-root smoke
  coverage that exact `fn` is a keyword and longer `fn1` remains an identifier.
- Added the second text-backed keyword token, `let`, with source-root smoke
  coverage that exact `let` is a keyword and longer `letter` remains an
  identifier.
- Added the third text-backed keyword token, `var`, with source-root smoke
  coverage that exact `var` is a keyword and longer `variant` remains an
  identifier.
- Added the fourth text-backed keyword token, `own`, with source-root smoke
  coverage that exact `own` is a keyword and longer `owner` remains an
  identifier.
- Added the fifth text-backed keyword token, `ref`, with source-root smoke
  coverage that exact `ref` is a keyword and longer `reference` remains an
  identifier.
- Added the sixth text-backed keyword token, `mut`, with source-root smoke
  coverage that exact `mut` is a keyword and longer `mutable` remains an
  identifier.
- Added the seventh text-backed keyword token, `ptr`, with source-root smoke
  coverage that exact `ptr` is a keyword and longer `ptrace` remains an
  identifier.
- Added the eighth text-backed keyword token, `return`, with source-root smoke
  coverage that exact `return` is a keyword and longer `returning` remains an
  identifier.
- Consolidated the repeated keyword source-root smoke checks behind one helper
  so each keyword case adds data instead of another full copy of the same
  cursor checks.
- Consolidated the public two-character lexer helpers into one
  `scan_two`/`cursor_from_two` path while preserving the existing source-root
  smokes for spans, operators, punctuation, and fallback behavior.
- Added token-kind query helpers for the lexer/parser boundary and a tiny parser
  handoff classification score.
- Moved the test-like entry arithmetic out of `compiler/main.ari` into a
  `driver.ari` bootstrap entry flow that uses `std::Result`.
- Added a file-input driver path using `std::fs::read_to_string` and
  `std::context` argv, and wired the bootstrap target to execute `main` with a
  source fixture path.
- Added parser non-statement diagnostic branches for whitespace and unknown
  handoff tokens, with bootstrap smoke coverage for both paths.
- Added a minimal loaded-source summary shape for file input and routed the
  driver text/file path through it before creating the current parser handoff.
- Added an invalid loaded-source summary smoke that checks the driver's
  out-of-range first-byte offset error payload.
- Added an explicit parser success helper and routed the driver through it, with
  source-root smoke coverage for a parse failure path.
- Added a diagnostic-code accessor and parser failure-code helper, with
  source-root smoke coverage for both the raw diagnostic accessor and a parser
  whitespace failure code.
- Added a focused parser empty-input failure-code smoke that checks diagnostic
  code `2002` through `parser::parse_failure_code(parser::parse_empty())`.
- Added a tiny parser EOF helper and a focused parser EOF-cursor failure-code
  smoke that checks diagnostic code `2001` through `parser::parse_failure_code`.
- Added a focused parser unknown-token failure-code smoke that checks
  diagnostic code `2005` through `parser::parse_failure_code(parser::parse_one(...))`.
- Added a tiny malformed handoff helper and a focused parser missing-EOF
  failure-code smoke that checks diagnostic code `2003` through
  `parser::parse_failure_code`.
- Added a focused parser number-success smoke that checks
  `parser::parse_is_success(parser::parse_one('9', ...))` without parser score
  arithmetic.
- Added an AST statement-kind query helper and a parser payload-shape smoke that
  checks successful parser output is a statement node without `ast::node_score`
  arithmetic.
- Added an AST node span-length query helper and a parser payload-span smoke
  that checks successful parser output spans without `ast::node_score`
  arithmetic.
- Added an AST node value query helper and a parser payload-value smoke that
  checks successful parser output values without `ast::node_score` arithmetic.
- Added source span-start, AST node start-offset, and parser payload-start
  helpers with a smoke that checks successful parser output start offsets
  without `ast::node_score` arithmetic.
- Added source span-end, AST node end-offset, and parser payload-end helpers
  with a smoke that checks successful parser output end offsets without
  `ast::node_score` arithmetic.
- Added source span-source, AST node source-id, and parser payload-source
  helpers with a smoke that checks successful parser output source ids without
  `ast::node_score` arithmetic.
- Added a focused parser number payload-value smoke using the existing parser
  statement value helper without parser score arithmetic.
- Added a focused parser number payload span-length smoke using the existing
  parser statement span-length helper without parser score arithmetic.
- Added a focused parser number payload start-offset smoke using the existing
  parser statement start-offset helper without parser score arithmetic.
- Added a focused parser number payload end-offset smoke using the existing
  parser statement end-offset helper without parser score arithmetic.
- Added a focused parser number payload source-id smoke using the existing
  parser statement source-id helper without parser score arithmetic.
- Added a focused parser number payload statement-node smoke using the existing
  parser statement-node helper without parser score arithmetic.
- Added a diagnostic start-offset accessor and a parser failure start-offset
  helper, with source-root smoke coverage for the whitespace diagnostic
  location.
- Added a diagnostic end-offset accessor and a parser failure end-offset helper,
  with source-root smoke coverage for the whitespace diagnostic location.
- Added a diagnostic severity-score accessor and a parser failure
  severity-score helper, with source-root smoke coverage for the whitespace
  diagnostic severity.
- Added a focused parser unknown-token failure start-offset smoke using the
  existing parser failure start-offset helper without diagnostic rendering.
- Added a focused parser unknown-token failure end-offset smoke using the
  existing parser failure end-offset helper without diagnostic rendering.
- Routed driver parse failures through the parser failure-code helper, with
  source-root smoke coverage for whitespace and unknown-token diagnostic codes.
- Added a driver result-code helper and simplified bootstrap smokes that inspect
  internal driver error payloads.
- Added focused driver input-bound smokes for the existing `1001` and `1002`
  offset validation errors.
- Added a scalar `DriverInput` constructor helper and routed the current
  input-bound smokes through it instead of public aggregate literals.
- Added a focused missing-file driver smoke for the existing `1102` file-read
  error payload.
- Added a focused empty source-text driver smoke for the existing `1101`
  text-input validation error payload.
- Added a focused default-driver success smoke that checks the internal
  `Ok(0)` payload through `result_code(driver::run())`.
- Added a focused file-driver success smoke that checks the internal file-input
  `Ok(0)` payload through `result_code(driver::run_file(...))`.
- Added a focused loaded-source driver success smoke that checks the internal
  `Ok(0)` payload through `result_code(driver::run_loaded_source_summary(...))`.
- Added a focused raw `DriverInput` success smoke that checks the internal
  `Ok(0)` payload through `result_code(driver::run_input(...))`.
- Added a focused source-text driver success smoke that checks the internal
  `Ok(0)` payload through `result_code(driver::run_source_text(...))`.
- Added a text-backed lexer cursor over `Slice[u8]`, source-root smoke coverage
  for multi-byte identifier and number spans, text cursor advance, whitespace
  skipping, and EOF placement.
- Added `parser::parse_text` and routed `driver::run_source_text` through the
  text-backed lexer handoff instead of first-byte summary input.
- Added a focused source-text extra-token driver smoke that checks `p+`
  preserves parser missing-EOF diagnostic code `2003`, proving text input no
  longer ignores bytes after the first token.
- Added a one-token file-input fixture for the compiled `compiler/main.ari`
  bootstrap run, so file input uses real loaded text without pretending the
  larger source-root smoke fixture is already parseable as a full Ari program.
- Raised file-input smoke allocation blocks to explicit `zone(65536)` after
  the growing source-root fixture exceeded the previous explicit zone capacity
  at runtime.

## Small Task Queue

- Keep `compiler/main.ari` thin; grow real entry behavior in `driver.ari` only
  when the underlying phases have checked handoff data.
- Add text-backed keyword classification for stage0 `if`, preserving longer
  identifiers such as `iffy` as identifiers.

## Next Recommended Task

Add text-backed keyword classification for stage0 `if`, preserving longer
identifiers such as `iffy` as identifiers.

## Local Validation

For the initial skeleton, use focused checks only:

```sh
make
./build/ari compiler/main.ari --check
./build/ari compiler/source.ari --check
./build/ari compiler/token.ari --check
./build/ari compiler/diagnostic.ari --check
./build/ari compiler/lexer.ari --check
./build/ari compiler/ast.ari --check
./build/ari compiler/parser.ari --check
./build/ari compiler/driver.ari --check
make check-ari-compiler-bootstrap
make check-compiler-docs
make check-bootstrap-readiness
```

For a later Ari source-only change under `compiler/`, prefer:

```sh
make
./build/ari compiler/<changed-file>.ari --check
./build/ari compiler/main.ari --check
make check-ari-compiler-bootstrap
make check-bootstrap-readiness
```

For docs-only changes, run:

```sh
make check-compiler-docs
```

Do not run full `make check` for ordinary bootstrap slices.

## Known Blockers

- Runtime strings, slices, and file IO are usable for current source-text
  lexing. The remaining gap is a real source table with source ids, file paths,
  lifetime policy, and diagnostic location mapping over loaded text.
- Default `zone { ... }` capacity is small for self-host-style file smokes;
  current file-input paths use explicit `zone(capacity)` until source loading
  owns allocation policy deliberately.
- File-backed module and project flow exists in stage0, but the Ari-written
  compiler only has a minimal file-reading driver path and loaded-source
  summary, not a real source loader, source table, or diagnostics over loaded
  text.
- Generic aggregate/type monomorphization and trait dispatch are still growing,
  so keep data models simple and checked.
- General iterator support beyond compiler-known `range` is not ready.
- Raw pointer operations, allocation-zone diagnostics, and explicit allocation
  policies are still hosted-compiler roadmap work.
- There is no full AST, HIR, ownership model, IR, or LLVM backend in Ari yet.

## Stage0 Host Compiler Follow-Ups

Confirmed host compiler bugs from this bootstrap slice: none. The `LexResult`,
shared diagnostic payload, one-token cursor, cursor token accessors, parser
skeleton, minimal token handoff, token-kind query helpers, unknown-token query
helpers, minimal AST node, statement output node, parser non-statement
diagnostic paths, parser success helper, diagnostic-code accessor, parser
failure-code helper, loaded-source summary, `std::Result`-based driver entry
flow, driver result-code helper, scalar `DriverInput` constructor helper, and
focused Ari compiler bootstrap test target checked without requiring a hosted
compiler fix. The file-input smoke path also checked with `std::fs` and
`std::context` argv without requiring a hosted compiler fix. The invalid
loaded-source summary smoke and parse-failure driver smokes also checked
`std::Result` payload inspection without requiring a hosted compiler fix. The
driver input-bound smokes checked negative integer offsets through the scalar
constructor helper without requiring a hosted compiler fix. The missing-file
driver smoke checked `std::fs::read_to_string` error propagation through
`result_code` without requiring a hosted compiler fix. The empty source-text
smoke checked `std::string::empty()` construction and text-input validation
through `result_code` without requiring a hosted compiler fix. The default
driver success smoke checked the `Ok(0)` payload through `result_code` without
requiring a hosted compiler fix. The file-driver success smoke checked the
file-input `Ok(0)` payload through `std::fs::read_to_string` and `result_code`
without requiring a hosted compiler fix. The loaded-source success smoke
checked the loaded-source `Ok(0)` payload through `result_code` without
requiring a hosted compiler fix. The raw `DriverInput` success smoke checked
the input-path `Ok(0)` payload through `result_code` without requiring a hosted
compiler fix. The source-text success smoke checked `std::string::from()`
construction and the text-input `Ok(0)` payload through `result_code` without
requiring a hosted compiler fix. The parser empty-input failure-code smoke
checked diagnostic code `2002` through `parser::parse_failure_code` without
requiring a hosted compiler fix. The parser EOF-cursor failure-code smoke
checked diagnostic code `2001` through `parser::parse_failure_code` without
requiring a hosted compiler fix. The parser unknown-token failure-code smoke
checked diagnostic code `2005` through `parser::parse_failure_code` without
requiring a hosted compiler fix. The parser missing-EOF handoff failure-code
smoke checked diagnostic code `2003` through `parser::parse_failure_code`
without requiring a hosted compiler fix. The parser number-success smoke checked
the number statement path through `parser::parse_is_success` without requiring
a hosted compiler fix. The AST statement-kind query and parser payload-shape
smoke checked successful statement output without requiring a hosted compiler
fix. The AST node span-length query and parser payload-span smoke checked
successful statement spans without requiring a hosted compiler fix. The AST
node value query and parser payload-value smoke checked successful statement
values without requiring a hosted compiler fix. The source span-start, AST
node start-offset, and parser payload-start helpers checked successful
statement start offsets without requiring a hosted compiler fix. The source
span-end, AST node end-offset, and parser payload-end helpers checked
successful statement end offsets without requiring a hosted compiler fix. The
source span-source, AST node source-id, and parser payload-source helpers
checked successful statement source ids without requiring a hosted compiler
fix. The parser number payload-value smoke checked number statement values
without requiring a hosted compiler fix. The parser number payload span-length
smoke checked number statement spans without requiring a hosted compiler fix.
The parser number payload start-offset smoke checked number statement start
offsets without requiring a hosted compiler fix. The parser number payload
end-offset smoke checked number statement end offsets without requiring a
hosted compiler fix. The parser number payload source-id smoke checked number
statement source ids without requiring a hosted compiler fix. The parser number
payload statement-node smoke checked number statement shape without requiring a
hosted compiler fix. The parser failure start-offset smoke checked whitespace
diagnostic start metadata without requiring a hosted compiler fix. The parser
failure end-offset smoke checked whitespace diagnostic end metadata without
requiring a hosted compiler fix. The parser failure severity smoke checked
whitespace diagnostic severity metadata without requiring a hosted compiler
fix. The parser unknown-token failure start-offset smoke checked unknown-token
diagnostic start metadata without requiring a hosted compiler fix. The parser
unknown-token failure end-offset smoke checked unknown-token diagnostic end
metadata without requiring a hosted compiler fix. The two-token lexer stream
smoke checked fixed stream cursors and EOF placement without requiring a hosted
compiler fix. The lexer punctuation smoke checked delimiter token
classification without requiring a hosted compiler fix. The lexer operator
smoke checked one-character operator token classification without requiring a
hosted compiler fix. The lexer dot punctuation smoke checked `.` tokenization
as punctuation without requiring a hosted compiler fix. The lexer square
bracket punctuation smoke checked `[` and `]` tokenization as punctuation
without requiring a hosted compiler fix. The lexer two-character
path separator smoke checked `::`
tokenization plus the one-character colon fallback path without requiring a
hosted compiler fix. The lexer identifier, number, and whitespace span smokes
checked variable-width `Slice[u8]` text runs without requiring a hosted
compiler fix. The lexer equality operator smoke checked `==` tokenization and
assignment fallback without requiring a hosted compiler fix. The lexer two-character
fat-arrow smoke checked `=>` tokenization plus the one-character assignment
fallback path without requiring a hosted compiler fix. The lexer one-character
comparison operator smoke checked `!`, `<`, and `>` tokenization without
requiring a hosted compiler fix. The lexer two-character comparison operator
smoke checked `!=`, `<=`, and `>=` tokenization plus one-character fallback
paths without requiring a hosted compiler fix. The lexer one-character bitwise
operator smoke checked `&`, `|`, and `^` tokenization without requiring a
hosted compiler fix. The lexer two-character logical operator smoke checked
`&&` and `||` tokenization plus one-character bitwise fallback paths without
requiring a hosted compiler fix. The lexer two-character shift operator smoke
checked `<<` and `>>` tokenization plus one-character comparison fallback paths
without requiring a hosted compiler fix. The lexer two-character arrow operator
smoke checked `->` tokenization plus the one-character minus fallback path
without requiring a hosted compiler fix. The consolidated `scan_two` smoke
path checked the same two-character token spans and fallback cases without
requiring a hosted compiler fix.
The lexer question-operator smoke checked `?` and `??` tokenization plus the
one-character `?` fallback path without requiring a hosted compiler fix.
The lexer at-punctuation smoke checked `@` tokenization as punctuation and the
unknown-token smokes moved to `$` without requiring a hosted compiler fix.
The lexer dot-run punctuation smoke checked `..`, `..=`, and `...` source-text
longest-match behavior without requiring a hosted compiler fix.
The lexer simple compound-assignment smoke checked `+=`, `-=`, `*=`, `/=`, and
`%=` tokenization plus the one-character `+` fallback path without requiring a
hosted compiler fix.
The lexer bitwise compound-assignment smoke checked `&=`, `|=`, and `^=`
tokenization, preserved `&&` / `||` priority, and checked the one-character
`&` fallback path without requiring a hosted compiler fix.
The lexer shift compound-assignment smoke checked `<<=` and `>>=` source-text
longest-match behavior while preserving `<<` and `<=` paths without requiring
a hosted compiler fix.
The lexer tilde-operator smoke checked `~` tokenization as an operator without
requiring a hosted compiler fix.
The lexer keyword smoke checked exact `fn` source-text classification and
preserved `fn1` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `let` source-text classification and
preserved `letter` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `var` source-text classification and
preserved `variant` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `own` source-text classification and
preserved `owner` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `ref` source-text classification and
preserved `reference` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `mut` source-text classification and
preserved `mutable` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `ptr` source-text classification and
preserved `ptrace` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `return` source-text classification and
preserved `returning` as an identifier without requiring a hosted compiler fix.
The token-kind class helper refactor checked through the bootstrap source root
without requiring a hosted compiler fix. The lexer double-quote delimiter smoke
checked `"` tokenization as punctuation without requiring a hosted compiler
fix.
The text-backed lexer cursor, `parser::parse_text`, and source-text extra-token
driver smoke checked `Slice[u8]` / `std::string::String` input without
requiring a hosted compiler fix.
The one-token file-input fixture checked the compiled `compiler/main.ari`
file path against real loaded text without requiring a hosted compiler fix.
The growing source-root fixture did expose allocation-capacity runtime traps
while reading the file smoke; this is fixed locally with explicit `zone(65536)`
allocation blocks and is recorded as allocation-policy pressure rather than a
confirmed hosted compiler bug.

This slice also showed that reusing the same payload binding name across
`std::Result` match arms is rejected as local shadowing/redeclaration. The
helper uses distinct binding names; this is recorded as match-arm scoping
ergonomics pressure, not a confirmed hosted compiler bug.
The text-backed lexer scan also had to use distinct branch-local names such as
`identifier_end`, `number_end`, and `whitespace_end`; reusing `end` across
branches hit the same local redeclaration rule and remains ergonomics pressure,
not a confirmed hosted compiler bug.

This slice also reconfirmed the existing cross-module type identity pressure:
a value constructed as root `source::LoadedSourceSummary` is not the same type
as `driver::source::LoadedSourceSummary`, and a root `lexer::TokenCursor` is
not the same type as `parser::lexer::TokenCursor`. That is not classified as a
hosted compiler bug in this slice; public driver helpers use scalar fields
while the nested summary remains an internal driver handoff, and the parser EOF
smoke uses a parser-local helper so it does not pass nested lexer cursor values
across module paths.

When Ari-written compiler work exposes behavior that looks wrong in the current
C++ hosted compiler, keep it separate from the Ari-written compiler task list.
Record the smallest Ari repro, expected behavior, actual behavior, focused
check target, and whether it belongs in parser, modules, generics, ownership,
codegen, diagnostics, or another hosted compiler area.

Desired stage0 pressure that is not yet classified as a bug:

- More polished source-table ergonomics over the usable runtime string, slice,
  and file-IO primitives.
- Stronger aggregate/type monomorphization for compiler-shaped models.
- Clearer cross-module type identity ergonomics for shared phase models; these
  slices keep AST constructors and public driver helpers scalar at module
  boundaries, and parser helpers construct parser-local lexer cursors instead
  of passing root `source`, `LoadedSourceSummary`, or `lexer::TokenCursor`
  values across nested import paths.
- Clearer ownership-phase ergonomics for checked trees and payload movement.
- More general iterator support beyond compiler-known `range`.
- Clearer allocation-policy ergonomics for self-host file reads; default
  `zone { ... }` capacity can be too small for growing compiler fixtures, so
  explicit `zone(capacity)` is currently required.
- Clearer match-arm binding scoping ergonomics; today a helper that matches
  both `std::Ok(code)` and `std::Err(code)` must use distinct payload names.
