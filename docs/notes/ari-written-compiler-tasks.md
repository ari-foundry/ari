# Ari-Written Compiler Tasks

This is a temporary split-out note for the Ari-written compiler bootstrap. It
keeps the large handoff content out of the main index while the Ari-written
compiler is incomplete. When the Ari-written compiler is complete, these
temporary bootstrap notes can be removed or folded into permanent docs.

Back to [Ari-Written Compiler](ari-written-compiler.md).

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
- Added the ninth text-backed keyword token, `if`, with source-root smoke
  coverage that exact `if` is a keyword and longer `iffy` remains an
  identifier.
- Added the tenth text-backed keyword token, `else`, with source-root smoke
  coverage that exact `else` is a keyword and longer `elsewhere` remains an
  identifier.
- Added the eleventh text-backed keyword token, `while`, with source-root smoke
  coverage that exact `while` is a keyword and longer `while1` remains an
  identifier.
- Added the twelfth text-backed keyword token, `init`, with source-root smoke
  coverage that exact `init` is a keyword and longer `initial` remains an
  identifier.
- Added the thirteenth text-backed keyword token, `next`, with source-root smoke
  coverage that exact `next` is a keyword and longer `next1` remains an
  identifier.
- Added the fourteenth text-backed keyword token, `continue`, with source-root
  smoke coverage that exact `continue` is a keyword and longer `continue1`
  remains an identifier.
- Added the fifteenth text-backed keyword token, `break`, with source-root smoke
  coverage that exact `break` is a keyword and longer `break1` remains an
  identifier.
- Added the sixteenth text-backed keyword token, `drop`, with source-root smoke
  coverage that exact `drop` is a keyword and longer `drop1` remains an
  identifier.
- Added the seventeenth text-backed keyword token, `forget`, with source-root
  smoke coverage that exact `forget` is a keyword and longer `forget1` remains
  an identifier.
- Added the eighteenth text-backed keyword token, `null`, with source-root smoke
  coverage that exact `null` is a keyword and longer `null1` remains an
  identifier.
- Added the nineteenth text-backed keyword token, `true`, with source-root smoke
  coverage that exact `true` is a keyword and longer `true1` remains an
  identifier.
- Added the twentieth text-backed keyword token, `false`, with source-root smoke
  coverage that exact `false` is a keyword and longer `false1` remains an
  identifier.
- Added the twenty-first text-backed keyword token, `const`, with source-root
  smoke coverage that exact `const` is a keyword and longer `constant` remains
  an identifier.
- Added the twenty-second text-backed keyword token, `as`, with source-root
  smoke coverage that exact `as` is a keyword and longer `ask` remains an
  identifier.
- Added the twenty-third text-backed keyword token, `meta`, with source-root
  smoke coverage that exact `meta` is a keyword and longer `metadata` remains
  an identifier.
- Recorded the no-assumption working rule: inspect actual repo structure,
  Ari source, tests, stdlib APIs, and stage0 behavior before judging design or
  host-compiler bugs.
- Recorded the bootstrap policy that Ari-written compiler code assumes
  `lib/std` is available and should use it first.
- Corrected the earlier keyword lookup note so `HashMap` availability is not
  treated as a bootstrap blocker.
- Added a reusable `KeywordTable` alias over std `HashMap[String, TokenKind]`
  and table-aware lexer source-text, cursor, significant-advance, and handoff
  helpers.
- Added source-root smoke coverage for the HashMap-backed keyword table,
  checking exact `meta`, longer `metadata`, significant cursor advance, and
  handoff EOF behavior.
- Routed parser source-text parsing through a reusable lexer `KeywordTable`
  helper and routed driver source-text/file-input parsing through that parser
  helper.
- Added source-root smoke coverage that checks table-backed parser keyword
  diagnostics and driver source-text keyword diagnostics.
- Replaced the raw per-character keyword comparison chain with one slice matcher
  helper while keeping width buckets, so adding keywords no longer duplicates
  manual indexing logic.
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
- Added `KwStruct` to the Ari-written token model and reusable keyword table,
  with focused smokes for exact `struct`, longer `structure`, and the
  table-backed parser/driver keyword path.
- Added `KwExtern` to the Ari-written token model and reusable keyword table,
  with focused smokes for exact `extern`, longer `external`, and the
  table-backed parser/driver keyword path.
- Added `KwEnum` to the Ari-written token model and reusable keyword table,
  with focused smokes for exact `enum`, longer `enumerate`, and the
  table-backed parser/driver keyword path.
- Added `KwTrait` to the Ari-written token model and reusable keyword table,
  with focused smokes for exact `trait`, longer `traitor`, and the
  table-backed parser/driver keyword path.
- Added `KwDyn` to the Ari-written token model and reusable keyword table,
  with focused smokes for exact `dyn`, longer `dynamic`, and the
  table-backed parser/driver keyword path.
- Added `KwMatch` to the Ari-written token model and reusable keyword table,
  with focused smokes for exact `match`, longer `matches`, and the
  table-backed parser/driver keyword path.
- Added `KwMod` to the Ari-written token model and reusable keyword table, with
  focused smokes for exact `mod`, longer `module`, and the table-backed
  parser/driver keyword path.
- Added `KwPub` to the Ari-written token model and reusable keyword table, with
  focused smokes for exact `pub`, longer `public`, and the table-backed
  parser/driver keyword path.
- Added `KwUse` to the Ari-written token model and reusable keyword table, with
  focused smokes for exact `use`, longer `user`, and the table-backed
  parser/driver keyword path.
- Added `KwImpl` to the Ari-written token model and reusable keyword table,
  with focused smokes for exact `impl`, longer `implicit`, and the
  table-backed parser/driver keyword path.
- Added `KwFor` to the Ari-written token model and reusable keyword table, with
  focused smokes for exact `for`, longer `forest`, and the table-backed
  parser/driver keyword path.
- Added `KwIn` to the Ari-written token model and reusable keyword table, with
  focused smokes for exact `in`, longer `inside`, and the table-backed
  parser/driver keyword path.
- Added a one-token file-input fixture for the compiled `compiler/main.ari`
  bootstrap run, so file input uses real loaded text without pretending the
  larger source-root smoke fixture is already parseable as a full Ari program.
- Raised file-input smoke allocation blocks to explicit `zone(65536)` after
  the growing source-root fixture exceeded the previous explicit zone capacity
  at runtime.

## Small Task Queue

- Keep `compiler/main.ari` thin; grow real entry behavior in `driver.ari` only
  when the underlying phases have checked handoff data.
- Backfill reusable keyword-table smoke coverage for existing `let`, preserving
  longer identifiers such as `letter`.

## Next Recommended Task

Backfill reusable keyword-table smoke coverage for existing `let`, preserving
longer identifiers such as `letter`.
