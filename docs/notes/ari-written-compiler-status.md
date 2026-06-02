# Ari-Written Compiler Status

This is a temporary split-out note for the Ari-written compiler bootstrap. It
keeps the large handoff content out of the main index while the Ari-written
compiler is incomplete. When the Ari-written compiler is complete, these
temporary bootstrap notes can be removed or folded into permanent docs.

Back to [Ari-Written Compiler](ari-written-compiler.md).

## Current Status

- `compiler/` has been started as a direct Ari source root.
- The initial files model source spans, token kinds, diagnostics, and a tiny
  one-character lexer classification path.
- `compiler/token.ari` models simple delimiter punctuation tokens for
  parentheses, braces, comma, colon, and semicolon.
- `compiler/token.ari` models simple one-character operator tokens for
  assignment and arithmetic operators.
- `compiler/token.ari` exposes token-kind rank and class-rank queries, and no
  longer exposes the old aggregate token score helper.
- `compiler/token.ari` separates `Integer` and `Float` token kinds while
  keeping a shared number class query so the early parser skeleton can still
  accept either numeric literal family through one phase-boundary predicate.
- `compiler/source.ari` exposes small span query helpers for downstream phase
  payload smokes.
- `compiler/lexer.ari` now has a small `LexResult` flow for one-character scans
  and source-text scans that can return either a token or a shared
  `diagnostic::Diagnostic` invalid-character/string-literal failure.
- `compiler/lexer.ari` has a tiny `TokenCursor` shape over one scanned token
  and an EOF advance path. It is a placeholder for phase handoff, not a real
  source-text stream.
- `compiler/lexer.ari` exposes small cursor accessors for the current token
  span, ranked token width, and done state.
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
- `compiler/lexer.ari` scans closed source-text quoted spans as
  `StringLiteral` tokens, including escaped quote bytes, while preserving the
  one-character `DoubleQuote` delimiter path for one-character scans.
- `compiler/lexer.ari` returns source-text lexer diagnostics for unterminated
  string literals at EOF or newline, and the parser/driver source-text paths
  preserve that diagnostic instead of falling back to `DoubleQuote`.
- `compiler/lexer.ari` returns source-text lexer diagnostics for unsupported
  single-character string escapes such as `\q`, while preserving supported
  common escapes, quote/backslash escapes, `\x`, `\u`, `\U`, and octal-leading
  escape spellings as string-literal spans for now. Source-root smoke coverage
  now checks all stage0 simple string escape spellings through lexer spans and
  parser success.
- `compiler/lexer.ari` validates source-text string escape digit shapes for
  `\x`, fixed-width `\u`, fixed-width `\U`, braced `\u{...}`, and octal digit
  runs before accepting a string-literal span, including invalid digit offsets
  inside braced Unicode escapes.
- `compiler/lexer.ari` reports braced string Unicode escapes that hit EOF or a
  newline before `}` as `lexer.unterminated-unicode-escape`, matching stage0's
  diagnostic identity instead of grouping them with invalid escape digits.
- `compiler/lexer.ari` validates source-text string escape value ranges for
  byte `\x`, octal, fixed-width Unicode, and braced Unicode escapes before
  accepting a string-literal span.
- `compiler/lexer.ari` treats source-text line comments and nested block
  comments as whitespace spans, and reports unterminated block comments through
  the parser/driver source-text paths.
- `compiler/lexer.ari` keeps carriage return inside CRLF line-comment spans,
  matching stage0's line comment scanner.
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
- `compiler/lexer.ari` classifies exact source-text `fn`, `const`, `as`,
  `meta`, `struct`, `extern`, `enum`, `trait`, `dyn`, `match`, `mod`, `pub`,
  `use`, `impl`, `for`, `in`, `let`, `var`, `own`, `ref`, `mut`, `ptr`,
  `return`, `if`, `else`, `while`, `init`, `next`, `continue`, `break`,
  `drop`, `forget`, `null`, `true`, and `false` as keywords while preserving
  longer identifier runs such as `fn1`, `constant`, `ask`, `metadata`,
  `structure`, `external`, `enumerate`, `traitor`, `dynamic`, `matches`,
  `module`, `public`, `user`, `implicit`, `forest`, `inside`, `letter`,
  `variant`, `owner`, `reference`, `mutable`, `ptrace`, `returning`, `iffy`,
  `elsewhere`, `while1`, `initial`, `next1`, `continue1`, `break1`, `drop1`,
  `forget1`, `null1`, `true1`, and `false1` as identifiers through the
  HashMap-backed keyword-table path.
- Ari-written compiler code assumes `lib/std` is available and should use it
  directly. `HashMap` and byte-slice string lookup helpers are available in
  `lib/std/collections.arih`.
- `compiler/lexer.ari` now exposes `KeywordTable` as a public alias for
  `std::collections::HashMap[String, TokenKind]`, builds it once with
  `std::collections::string_hash_map`, and uses `get_or_bytes` for borrowed
  source-slice keyword lookup.
- `compiler/lexer.ari` exposes table-aware source-text scanning, cursor
  advance, significant-token advance, and handoff helpers so later parser and
  driver work can reuse one keyword table instead of rebuilding lookup state
  per token. The older stateless text helpers remain for focused lexer smokes,
  but they now classify identifier spellings as `Identifier` instead of keeping
  a second keyword list.
- `compiler/parser.ari` exposes a zone-backed `parse_text_with_keywords`
  helper that builds one reusable lexer `KeywordTable` for a source-text parse
  and consumes the lexer table-aware handoff path.
- `compiler/driver.ari` routes source-text and file-input parsing through the
  parser keyword-table helper, so real compiler text input now uses the
  HashMap-backed keyword path. The older parser `parse_text` helper remains as
  a stateless identifier-only compatibility path for focused smokes.
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
- `compiler/lexer.ari` scans valid `0x`/`0X`, `0o`/`0O`, and `0b`/`0B`
  source-text integer prefixes as one number span instead of stopping after the
  leading decimal `0`.
- `compiler/lexer.ari` includes valid exact-width integer suffixes such as
  `i64` and `u8` in decimal and base-prefixed source-text `Integer` spans.
- `compiler/lexer.ari` reports source-text numeric base-prefix diagnostics for
  missing prefix digits and invalid binary/octal digits, preserving those
  failures through parser and driver source-text paths.
- `compiler/lexer.ari` reports source-text numeric suffix diagnostics for
  unsupported suffixes and non-decimal float suffixes, while accepting decimal
  float suffix spans as `Float` tokens for the current bootstrap parser.
- `compiler/lexer.ari` keeps stage0 parity for numeric suffix starts: suffixes
  must start with an ASCII letter, so inputs such as `42_abc` stop the numeric
  token at `42` and leave `_abc` as the following identifier instead of
  reporting an unsupported suffix.
- `compiler/lexer.ari` scans source-text decimal floating literal spans for
  fractional forms such as `1.5` and exponent forms such as `1e3`, including
  valid decimal float suffixes, as `Float` tokens.
- `compiler/lexer.ari` reports source-text non-decimal float dot diagnostics
  for base-prefixed spellings such as `0x2.0`, preserving those failures
  through parser and driver source-text paths.
- `compiler/lexer.ari` scans simple ASCII byte character spellings such as
  `'a'` as `Integer` spans, matching stage0's integer-literal treatment for
  byte character literals at the current Ari token-model level.
- `compiler/token.ari` carries literal base and suffix-rank metadata, and
  `compiler/lexer.ari` fills it for decimal, hexadecimal, octal, binary,
  typed numeric suffixes, and byte-character integer tokens.
- `compiler/token.ari` carries numeric literal value payloads: `u64` integer
  values for decimal/base-prefixed integers and byte-character literals, plus
  `f64` values for decimal float literals and decimal integer spellings with
  float suffixes.
- `compiler/lexer.ari` reports integer literal overflow plus decimal float
  overflow/underflow diagnostics using `std::parse`, and keeps decimal integer
  spellings with float suffixes on the float parsing path instead of rejecting
  them as integer overflow first.
- `compiler/token.ari` groups literal metadata in `LiteralPayload` and carries
  source-backed literal text and suffix spans. Numeric literals can now expose
  the token's full span separately from the numeric core and typed suffix
  spans without storing owned token text in each token. Byte-character tokens
  keep the synthetic byte-character suffix rank with zero-width text/suffix
  spans because the `"char"` marker is not source-backed.
- `compiler/lexer.ari` now emits `StringLiteral` tokens with source-backed raw
  content spans inside the quotes through `LiteralPayload`, using a zero-width
  suffix span at the closing quote. This records recoverable source text for
  strings without adding decoded owned string payloads yet.
- `compiler/lexer.ari` accepts stage0-style string line-continuation escapes
  for backslash-LF and backslash-CRLF as part of a closed `StringLiteral` span.
- `compiler/lexer.ari` can now return the stage0-style decoded byte length
  of a valid `StringLiteral` cursor from the original source slice as
  `Option[i64]`, including simple, byte, octal, fixed Unicode, braced Unicode,
  and line-continuation escapes. This is a checked parity accessor, not an
  owned decoded string payload.
- `compiler/ast.ari` and `compiler/parser.ari` preserve numeric and
  source-backed string statement literal metadata/value payloads as an
  AST-owned snapshot, so parser-facing accessors can inspect literal base,
  suffix rank, checked values, and source-backed literal/suffix spans without
  passing nested `token` module types across phase boundaries.
- `compiler/ast.ari` and `compiler/parser.ari` now also preserve
  `Option[i64]` decoded string byte length for parser results produced from
  source text, using the lexer string decoded-length accessor while keeping
  non-string literals as `None`.
- `compiler/ast.ari` and `compiler/parser.ari` now preserve source-text
  `KwTrue`, `KwFalse`, and `KwNull` as statement literal payloads. Bool
  literals carry `Option[bool]`, null literals use the `LiteralNull` kind
  variant, and the remaining keyword statements stay on the unsupported-token
  diagnostic path for now.
- `compiler/lexer.ari` scans simple byte character escape spellings such as
  `'\n'` and `'\\'` as `Integer` spans, and source-root smoke coverage now
  checks stage0 simple escape value payloads for alert, backspace, escape,
  form-feed, newline, carriage return, tab, vertical tab, quote,
  single-quote, question-mark, and backslash spellings.
- `compiler/lexer.ari` scans byte character hex and octal numeric escape
  spellings such as `'\x41'` and `'\101'` as `Integer` spans.
- `compiler/lexer.ari` scans ASCII-valued fixed-width byte character Unicode
  escape spellings such as `'\u0041'` and `'\U00000041'` as `Integer` spans.
- `compiler/lexer.ari` scans ASCII-valued braced byte character Unicode escape
  spellings such as `'\u{41}'` as `Integer` spans.
- `compiler/lexer.ari` reports source-text empty byte character diagnostics for
  spellings such as `''`, preserving those failures through parser and driver
  source-text paths.
- `compiler/lexer.ari` reports source-text unterminated byte character
  diagnostics for direct EOF/newline and escaped EOF/newline spellings such as
  a lone opening quote or a backslash at the end of a byte character literal.
- `compiler/lexer.ari` reports source-text unsupported byte character escape
  diagnostics for spellings such as `'\q'`, preserving those failures through
  parser and driver source-text paths.
- `compiler/lexer.ari` reports source-text byte character escape digit-shape
  diagnostics for spellings such as `'\x'`, `'\u'`, `'\U'`, `'\u12'`,
  `'\U1234'`, `'\u{Q}'`, and `'\u{}'`, preserving those failures through
  parser and driver source-text paths.
- `compiler/lexer.ari` reports braced byte-character Unicode escapes that hit
  EOF or a newline before `}` as `lexer.unterminated-unicode-escape`, sharing
  the same stage0-parity diagnostic identity as string Unicode escapes.
- `compiler/lexer.ari` reports source-text byte character escape value-range
  diagnostics for spellings such as `'\x100'`, `'\777'`, `'\u0080'`,
  `'\U00000080'`, and `'\u{80}'`, preserving those failures through parser and
  driver source-text paths.
- `compiler/lexer.ari` reports source-text byte character exactly-one-byte
  diagnostics for too-long spellings such as `'ab'` and `'\nA'`, plus EOF after
  one direct byte payload, preserving those failures through parser and driver
  source-text paths.
- `compiler/lexer.ari` reports source-text direct non-ASCII byte character
  diagnostics for UTF-8 byte spellings, preserving those failures through
  parser and driver source-text paths.
- `compiler/lexer.ari` treats carriage return as source-text whitespace,
  matching stage0's whitespace skipper.
- `compiler/lexer.ari` keeps EOF-only line-comment spans as whitespace through
  the end of source text, matching stage0's line comment scanner.
- `compiler/lexer.ari` skips trailing line and block comments after a
  statement token when building source-text handoffs, so the handoff EOF cursor
  lands at the end of the skipped comment.
- `compiler/lexer.ari` keeps post-comment non-whitespace tokens visible in the
  source-text handoff EOF slot instead of swallowing them as part of comment
  trivia.
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
  spans can be checked without relying on old aggregate node-score arithmetic.
- `compiler/ast.ari` exposes a node value query helper so parser payload values
  can be checked without relying on old aggregate node-score arithmetic.
- `compiler/ast.ari` exposes a node start-offset query helper so parser payload
  offsets can be checked without relying on old aggregate node-score arithmetic.
- `compiler/ast.ari` exposes a node end-offset query helper so parser payload
  offsets can be checked without relying on old aggregate node-score arithmetic.
- `compiler/ast.ari` exposes a node source-id query helper so parser payload
  source identities can be checked without relying on old aggregate node-score
  arithmetic.
- `compiler/ast.ari` exposes a node-kind rank query and no longer exposes the
  test-only aggregate node score helper.
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
- `compiler/diagnostic.ari` exposes a diagnostic severity-rank accessor, and
  `compiler/parser.ari` exposes a parser failure severity-rank helper for
  phase boundaries that need diagnostic severity metadata without rendering
  diagnostics.
- `compiler/diagnostic.ari` now carries `DiagnosticKind` in addition to the
  numeric compatibility code, and exposes diagnostic name/message text
  accessors so bootstrap failures can report names such as
  `parser.unexpected-whitespace` and `lexer.unterminated-string-literal`
  instead of only numeric codes such as `2004` and `1004`.
- `compiler/lexer.ari` exposes lexer failure name/message helpers for both
  direct `LexResult` failures and handoff-level `LexFailure` payloads, so
  lexer callers do not have to interpret raw diagnostic numbers first.
- `compiler/driver.ari` now returns diagnostic payloads in `Err` results while
  preserving the old `result_code` helper for focused smokes. Driver result
  helpers can expose diagnostic name/message text, and driver-local failures
  such as input bounds have distinct kind ranks even when their compatibility
  numeric code overlaps older lexer codes.
- `compiler/parser.ari` exposes a tiny `parse_one_eof` helper so EOF-cursor
  diagnostics can be tested without exporting or passing nested lexer cursor
  types across module paths.
- `compiler/parser.ari` exposes a tiny `parse_one_without_eof` helper so
  malformed handoff diagnostics can be tested without exposing handoff internals
  to the bootstrap fixture.
- `compiler/parser.ari` reports missing-EOF diagnostic code `2003` at the
  handoff extra token span instead of using a placeholder `0..0` location.
- The bootstrap source-root smoke checks the parser empty-input diagnostic code
  through `parser::parse_failure_code` instead of relying only on diagnostic
  smoke-score arithmetic.
- The bootstrap source-root smoke checks the parser EOF-cursor diagnostic code
  through `parser::parse_failure_code` instead of relying only on diagnostic
  smoke-score arithmetic.
- The bootstrap source-root smoke checks source-text comment-only parser EOF
  diagnostics after skipped line and block comments, including failure span
  offsets at the end of the skipped comment.
- The bootstrap source-root smoke checks an explicit EOF line-comment spelling
  through the source-text parser and keyword-table parser paths, preserving
  parser EOF diagnostic code `2001` at byte offset 6.
- The bootstrap source-root smoke checks source-text parser success with
  trailing line and block comments after a statement token, preserving the
  statement span while skipping comments to EOF.
- The bootstrap source-root smoke checks CRLF line-comment parser success
  before a statement token, preserving the carriage return inside the skipped
  comment span and starting the statement at byte offset 9.
- The bootstrap source-root smoke checks CRLF line-comment parser
  lexer-failure diagnostics before an unterminated string, preserving
  diagnostic code `1004` and span `9..14` through the parser and keyword-table
  parser paths.
- The bootstrap source-root smoke checks CRLF line-comment parser extra-token
  diagnostics before a statement followed by another identifier, preserving
  parser missing-EOF diagnostic code `2003` and span `14..19` through the
  parser and keyword-table parser paths.
- The bootstrap source-root smoke checks CRLF line-comment extra-token handoff
  before a statement followed by another identifier, preserving the first token
  span `9..13` and the extra token span `14..19` in the handoff EOF slot.
- The bootstrap source-root smoke checks source-text parser trailing
  unterminated block-comment diagnostics after a statement token, including
  both immediate `name/* open` and whitespace-separated `name /* open`
  spellings.
- The bootstrap source-root smoke checks source-text parser post-comment
  extra-token diagnostics after a statement token, including both line-comment
  and block-comment trivia before the second identifier.
- The bootstrap source-root smoke checks CRLF post-comment parser extra-token
  diagnostics after a statement token, preserving parser missing-EOF
  diagnostic code `2003` and span `13..18` through the parser and
  keyword-table parser paths.
- The bootstrap source-root smoke checks CRLF post-comment extra-token handoff
  after a statement token, preserving the first token span `0..4` and the
  extra token span `13..18` in the handoff EOF slot.
- The bootstrap source-root smoke checks source-text parser post-comment lexer
  failure diagnostics after a statement token, including line-comment and
  block-comment trivia before an unterminated string literal.
- The bootstrap source-root smoke checks CRLF post-comment parser
  lexer-failure diagnostics after a statement token, preserving diagnostic
  code `1004` and span `13..18` through the parser and keyword-table parser
  paths.
- The bootstrap source-root smoke checks CRLF post-comment lexer-failure
  handoff diagnostics after a statement token, preserving diagnostic code
  `1004` and span `13..18` through the lexer handoff result path.
- The bootstrap source-root smoke checks source-text parser leading-comment
  handoff success before a statement token, including line-comment and
  block-comment trivia before an identifier while preserving statement spans.
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
  severity through `parser::parse_failure_severity_rank` instead of relying
  only on diagnostic smoke-score arithmetic.
- The bootstrap source-root smoke checks the parser number statement success
  path through `parser::parse_is_success` instead of relying only on parser
  smoke-score arithmetic.
- The bootstrap source-root smoke checks successful parser payloads are
  statement nodes without relying on old aggregate node-score arithmetic.
- The bootstrap source-root smoke checks number parser payloads are statement
  nodes without relying on parser score arithmetic.
- The bootstrap source-root smoke checks successful parser payload span length
  without relying on old aggregate node-score arithmetic.
- The bootstrap source-root smoke checks number parser payload span length
  without relying on parser score arithmetic.
- The bootstrap source-root smoke checks successful parser payload values
  without relying on old aggregate node-score arithmetic.
- The bootstrap source-root smoke checks number parser payload values without
  relying on parser score arithmetic.
- The bootstrap source-root smoke checks successful parser payload start offsets
  without relying on old aggregate node-score arithmetic.
- The bootstrap source-root smoke checks number parser payload start offsets
  without relying on parser score arithmetic.
- The bootstrap source-root smoke checks successful parser payload end offsets
  without relying on old aggregate node-score arithmetic.
- The bootstrap source-root smoke checks number parser payload end offsets
  without relying on parser score arithmetic.
- The bootstrap source-root smoke checks successful parser payload source ids
  without relying on old aggregate node-score arithmetic.
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
- `compiler/source.ari` has a minimal `SourceTableEntry` that wraps a
  `SourceFile` plus a scalar path id placeholder, giving the Ari-written
  compiler its first source-table-shaped entry without owning path strings,
  source text, line starts, or diagnostic display names yet.
- `compiler/source.ari` can validate that a span belongs to a
  `SourceTableEntry` and stays inside that entry's byte length.
- `compiler/source.ari` can construct a span from a `SourceTableEntry`, so
  callers do not need to manually repeat the entry source id.
- `compiler/source.ari` has a minimal `SourceLineStarts` summary with up to
  two line starts, count metadata, validity checks, and a one-based
  byte-to-line-number helper plus a one-based byte-column helper. It is a
  checked source-model placeholder, not a full owned line-start table or
  visual-column lookup yet.
- `compiler/source.ari` can validate that a `SourceFile` has a non-negative
  byte length, and byte containment checks now make that source-file invariant
  explicit.
- `compiler/source.ari` exposes source-file id and byte-length query helpers,
  so downstream source-model code does not need to read `SourceFile` fields
  directly.
- `compiler/source.ari` exposes a source-file span constructor helper, so
  source-table and loaded-source wrappers can share the same source id handoff
  path.
- `compiler/source.ari` can check byte containment through a loaded source
  summary, so callers do not need to unwrap its `SourceFile`.
- `compiler/source.ari` has a minimal `SourceLocationSummary` that carries a
  source id, clamped byte offset, one-based line number, and one-based byte
  column for source-model handoff checks.
- `compiler/source.ari` can construct a source-table-entry-local location
  summary, so callers do not need to unwrap the entry's `SourceFile` before
  computing line and byte-column metadata.
- `compiler/source.ari` can derive a source-table-entry-local span-start
  location summary, so callers do not need to unwrap the entry's `SourceFile`
  before validating source ownership and computing start line/column metadata.
- `compiler/source.ari` can derive a source-table-entry-local span-end
  location summary with the same source ownership, byte-bound, and EOF
  insertion-point behavior as file-local span end summaries.
- `compiler/source.ari` can derive a loaded-source-local location summary,
  reusing the loaded source's `SourceFile` metadata while the real owned source
  text and line-start table remain future work.
- `compiler/source.ari` can derive a loaded-source first-byte location summary
  through the first-byte span and loaded-source-local span-start path.
- `compiler/source.ari` can construct a span from a loaded source summary, so
  callers do not need to manually repeat the loaded source id.
- `compiler/source.ari` can construct a loaded-source first-byte span from the
  summary's first-byte offset.
- `compiler/source.ari` can validate that a span belongs to a loaded source
  summary and stays inside that summary's byte length.
- `compiler/source.ari` can derive a loaded-source-local span-start location
  summary, reusing the loaded source's `SourceFile` metadata for source
  ownership and byte-bound validation.
- `compiler/source.ari` can derive a loaded-source-local span-end location
  summary with the same EOF insertion-point and span validity behavior as the
  file-local helper.
- `compiler/source.ari` can derive a span-start location summary after checking
  that the span belongs to the file and stays inside the file byte bounds.
- `compiler/source.ari` can derive a span-end location summary with the same
  source-id and byte-bound checks, including EOF insertion-point spans.
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
- The bootstrap source-root smoke checks the source-text driver comment-only
  path and verifies skipped line and block comments preserve parser EOF
  diagnostic code `2001`.
- The bootstrap source-root smoke checks the source-text driver EOF
  line-comment path and verifies `// eof` preserves parser EOF diagnostic code
  `2001`.
- The bootstrap source-root smoke checks the source-text driver CRLF
  line-comment path and verifies `// skip\r\np` preserves `Ok(0)`.
- The bootstrap source-root smoke checks the source-text driver CRLF
  line-comment lexer-failure path and verifies `// skip\r\n"open` preserves
  lexer diagnostic code `1004`.
- The bootstrap source-root smoke checks the source-text driver CRLF
  line-comment extra-token path and verifies `// skip\r\nname other` preserves
  parser missing-EOF diagnostic code `2003`.
- The bootstrap source-root smoke checks the source-text driver trailing
  comment path and verifies line and block comments after a statement token
  preserve `Ok(0)`.
- The bootstrap source-root smoke checks the source-text driver trailing
  unterminated block-comment path and verifies those failures preserve lexer
  diagnostic code `1008` after an initial statement token.
- The bootstrap source-root smoke checks the source-text driver post-comment
  extra-token path and verifies skipped comments followed by another
  identifier preserve parser missing-EOF diagnostic code `2003`.
- The bootstrap source-root smoke checks the source-text driver CRLF
  post-comment extra-token path and verifies `p// skip\r\nq` preserves parser
  missing-EOF diagnostic code `2003`.
- The bootstrap source-root smoke checks the source-text driver post-comment
  lexer-failure path and verifies skipped comments followed by an unterminated
  string preserve lexer diagnostic code `1004`.
- The bootstrap source-root smoke checks the source-text driver CRLF
  post-comment lexer-failure path and verifies `p// skip\r\n"open` preserves
  lexer diagnostic code `1004`.
- The bootstrap source-root smoke checks the source-text driver leading-comment
  path and verifies skipped comments before a statement token preserve `Ok(0)`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment lexer-failure path and verifies skipped comments before an
  unterminated string preserve lexer diagnostic code `1004`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment extra-token path and verifies skipped comments before a
  statement followed by another identifier preserve parser missing-EOF
  diagnostic code `2003`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment unterminated block-comment path and verifies skipped comments
  before an unterminated block comment preserve lexer diagnostic code `1008`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment keyword path and verifies skipped comments before `while`
  preserve parser unsupported-token diagnostic code `2006`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment unknown-token path and verifies skipped comments before `$`
  preserve parser unknown-token diagnostic code `2005`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment punctuation path and verifies skipped comments before `;`
  preserve parser unsupported-token diagnostic code `2006`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment operator path and verifies skipped comments before `+`
  preserve parser unsupported-token diagnostic code `2006`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment string-literal path and verifies skipped comments before
  `"ari"` preserve parser and driver success.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment numeric missing-base-digit path and verifies skipped
  comments before `0x` preserve lexer diagnostic code `1009`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment invalid base-digit path and verifies skipped comments before
  `0b102` preserve lexer diagnostic code `1010`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment unsupported number-suffix path and verifies skipped comments
  before `42abc` preserve lexer diagnostic code `1011`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment non-decimal float-suffix path and verifies skipped comments
  before `0b1010f32` preserve lexer diagnostic code `1012`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment non-decimal float-dot path and verifies skipped comments
  before `0x2.0` preserve lexer diagnostic code `1013`.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment byte-character literal path and verifies skipped comments
  before `'a'` preserve the number-token span through parser and driver paths.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment byte-character escape path and verifies skipped comments
  before `'\n'` preserve the number-token span through parser and driver paths.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment byte-character numeric escape path and verifies skipped
  comments before `'\x41'` and `'\101'` preserve the number-token span through
  parser and driver paths.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment byte-character fixed-width Unicode escape path and verifies
  skipped comments before `'\u0041'` and `'\U00000041'` preserve the
  number-token span through parser and driver paths.
- The bootstrap source-root smoke checks the source-text driver
  leading-comment byte-character braced Unicode escape path and verifies
  skipped comments before `'\u{41}'` preserve the number-token span through
  parser and driver paths.
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
- The source model now has source-table-entry byte-length validity checks that
  reuse the wrapped source-file invariant.
- The source model now has loaded-source byte-length validity checks that
  reuse the wrapped source-file invariant.
- The source model now has loaded-source first-offset validity checks that
  make one-byte availability explicit through the existing containment rule.
- `compiler/source.ari` no longer exposes the test-only loaded-source score
  helper; the source-root smoke keeps score arithmetic local and marked as
  test-only.
- `compiler/diagnostic.ari` now uses severity-rank naming and no longer
  exposes the test-only diagnostic aggregate score helper.
- `compiler/ast.ari` now uses node-kind-rank naming and no longer exposes the
  test-only AST aggregate score helper.
- `compiler/token.ari` now uses token-kind-rank and token-kind-class-rank
  naming and no longer exposes the test-only aggregate token score helper.
- `compiler/token.ari` also exposes token-kind name text, and
  `compiler/lexer.ari` forwards that through cursors so smoke checks and
  debugging do not have to rely only on numeric token ranks.
- Token class name text is also available from `compiler/token.ari` and lexer
  cursors, so callers can distinguish identifier, number, punctuation,
  operator, keyword, string-literal, EOF, and unknown classes without decoding
  class ranks.
- Token kind rank, class rank, token name text, and class name text now share a
  single private token-kind metadata mapping in `compiler/token.ari`, reducing
  the duplicated full-token `match` blocks that had to be kept synchronized
  after every lexer token addition.
- Hot token predicates such as identifier, number, whitespace, EOF, unknown,
  punctuation, operator, keyword, and string-literal checks use direct matches
  instead of routing through the aggregate token-kind metadata helper.
- `compiler/lexer.ari` now keeps `scan_one_result` as a thin diagnostic wrapper
  around `scan_one`, so one-character token construction has one source of
  truth and the result path only adds the invalid-character failure case.
- One-character lexer stream and handoff constructors now reuse local cursor
  values instead of rebuilding the same cursor/EOF cursor more than once.
- Result-producing text handoff constructors now use private significant-token
  helpers, keeping whitespace skipping and lexer-failure conversion in one
  place for the plain and keyword-table paths.
- Non-result text cursor advancement and handoff construction now share private
  significant-cursor helpers, so whitespace skipping is no longer open-coded in
  every handoff constructor.
- `compiler/lexer.ari` now uses ranked-width and ranked-position query names
  and no longer exposes public `score` helpers.
- `compiler/parser.ari` now uses parser kind-rank query names and no longer
  exposes public `score` helpers; parser result aggregate arithmetic is kept
  local to the source-root smoke as test-only exit-code logic.
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
