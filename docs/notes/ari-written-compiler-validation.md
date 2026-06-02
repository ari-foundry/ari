# Ari-Written Compiler Validation And Follow-Ups

This is a temporary split-out note for the Ari-written compiler bootstrap. It
keeps the large handoff content out of the main index while the Ari-written
compiler is incomplete. When the Ari-written compiler is complete, these
temporary bootstrap notes can be removed or folded into permanent docs.

Back to [Ari-Written Compiler](ari-written-compiler.md).

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

Bootstrap source-root fixtures live under `tests/cases/ari-compiler-bootstrap/`.

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
a hosted compiler fix. The reusable keyword-table smoke checked `KwStruct`,
`KwExtern`, `KwEnum`, `KwTrait`, `KwDyn`, `KwMatch`, `KwMod`, `KwPub`, `KwUse`,
`KwImpl`, `KwFor`, `KwIn`, `KwLet`, `KwVar`, `KwOwn`, `KwRef`, `KwMut`,
`KwPtr`, `KwReturn`, `KwIf`, `KwElse`, `KwWhile`, `KwInit`, `KwNext`,
`KwContinue`, `KwBreak`, `KwDrop`, `KwForget`, `KwNull`, `KwTrue`, and
`KwFalse`, plus longer `structure`, `external`, `enumerate`, `traitor`,
`dynamic`, `matches`, `module`, `public`, `user`, `implicit`, `forest`,
`inside`, `letter`, `variant`, `owner`, `reference`, `mutable`, `ptrace`,
`returning`, `iffy`, `elsewhere`, `while1`, `initial`, `next1`, `continue1`,
`break1`, `drop1`, `forget1`, `null1`, `true1`, and `false1`, and the
source-text parser/driver keyword path without requiring a hosted compiler fix.
The source-text string literal span smoke checked closed quoted spans, escaped
quote bytes, empty quoted strings, and unterminated EOF/newline lexer
diagnostics preserved through parser and driver source-text paths without
requiring a hosted compiler fix. The same string-literal smoke now checks
unsupported `\q` escape diagnostics preserved through parser and driver
source-text paths without requiring a hosted compiler fix. It also checks
source-text string escape digit-shape diagnostics for `\x`, fixed-width `\u`,
fixed-width `\U`, invalid braced `\u{...}` digits, and empty braced `\u{}`
spellings, plus a valid digit-escape span covering `\x`, `\u`, `\U`, braced
`\u{...}`, and octal-leading escape spellings, without requiring a hosted
compiler fix. It now also checks source-text string escape value-range
diagnostics for byte `\x`, octal, fixed-width Unicode, and braced Unicode
spellings without requiring a hosted compiler fix. It now also checks
source-text line comment and nested block comment skipping as whitespace spans,
plus unterminated block comment diagnostics preserved through parser and driver
source-text paths, including CRLF line comments, without requiring a hosted
compiler fix.
The AST statement-kind query and parser payload-shape smoke checked successful
statement output without requiring a hosted compiler fix. The AST node
span-length query and parser payload-span smoke checked successful statement
spans without requiring a hosted compiler fix. The AST node value query and
parser payload-value smoke checked successful statement values without
requiring a hosted compiler fix. The source span-start, AST node start-offset,
and parser payload-start helpers checked successful
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
The lexer keyword smoke checked exact `if` source-text classification and
preserved `iffy` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `else` source-text classification and
preserved `elsewhere` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `while` source-text classification and
preserved `while1` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `init` source-text classification and
preserved `initial` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `next` source-text classification and
preserved `next1` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `continue` source-text classification and
preserved `continue1` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `break` source-text classification and
preserved `break1` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `drop` source-text classification and
preserved `drop1` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `forget` source-text classification and
preserved `forget1` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `null` source-text classification and
preserved `null1` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `true` source-text classification and
preserved `true1` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `false` source-text classification and
preserved `false1` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `const` source-text classification and
preserved `constant` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `as` source-text classification and
preserved `ask` as an identifier without requiring a hosted compiler fix.
The lexer keyword smoke checked exact `meta` source-text classification and
preserved `metadata` as an identifier without requiring a hosted compiler fix.
The working-rule update recorded that bootstrap decisions must come from
inspecting actual repo structure, stdlib APIs, tests, and stage0 behavior
rather than inference; this required no hosted compiler fix.
The keyword lookup review now records that `lib/std` is assumed available and
should be used first, and that std `HashMap` exists, while the compatibility
stateless lexer path still keeps the allocation-free matcher without requiring
a hosted compiler fix.
The HashMap-backed keyword table checked through `KeywordTable` as a type alias
over `std::collections::HashMap[String, TokenKind]`, plus table-aware scanner,
cursor, significant-advance, and handoff helpers, without requiring a hosted
compiler fix.
The parser and driver source-text paths now check through the HashMap-backed
keyword table path without requiring a hosted compiler fix. `driver::run_source_text`
uses a small explicit current-zone block for the transient keyword table; this
is allocation-policy pressure only, not a confirmed hosted compiler bug.
The private parser `KeywordTable` helper keeps `parse_text_with_keywords`' table
handoff centralized without requiring a hosted compiler fix. The source-root
parser keyword smoke uses a small per-call zone helper because root
`lexer::KeywordTable` and parser-local `lexer::KeywordTable` are distinct
module-path types, and `parser::lexer` is intentionally private; this is
existing module-boundary pressure, not a confirmed hosted compiler bug.
The keyword matcher helper refactor kept the width-bucket keyword path checked
through the source-root smoke without requiring a hosted compiler fix.
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
The unterminated byte-character diagnostic slice re-hit the same rule inside
`scan_non_identifier_text_result`; it uses a distinct
`byte_char_unterminated_end` binding. This is the same ergonomics pressure, not
a confirmed hosted compiler bug.

This slice also reconfirmed the existing cross-module type identity pressure:
a value constructed as root `source::LoadedSourceSummary` is not the same type
as `driver::source::LoadedSourceSummary`, and a root `lexer::TokenCursor` is
not the same type as `parser::lexer::TokenCursor`. That is not classified as a
hosted compiler bug in this slice; public driver helpers use scalar fields
while the nested summary remains an internal driver handoff, and the parser EOF
smoke uses a parser-local helper so it does not pass nested lexer cursor values
across module paths.
The lexer diagnostic handoff also avoids passing `lexer::diagnostic::Diagnostic`
directly into `parser::diagnostic::Diagnostic`; it uses a lexer-owned
`LexFailure` code/span payload and lets parser reconstruct its local diagnostic
value. This is the same cross-module type identity pressure, not a confirmed
hosted compiler bug.
The source-text empty byte character diagnostic for `''` now preserves code
`1014` through lexer, parser, and driver paths without requiring a hosted
compiler fix.
The source-text unterminated byte character diagnostic now preserves code
`1015` for direct EOF/newline and escaped EOF/newline spellings through lexer,
parser, and driver paths without requiring a hosted compiler fix.
The source-text unsupported byte character escape diagnostic now preserves code
`1016` for spellings such as `'\q'` through lexer, parser, and driver paths
without requiring a hosted compiler fix.
The source-text byte character escape digit-shape diagnostic now preserves code
`1017` for spellings such as `'\x'`, `'\xG'`, `'\u'`, `'\U'`, `'\u12'`,
`'\U1234'`, `'\u{Q}'`, and `'\u{}'` through lexer, parser, and driver paths
without requiring a hosted compiler fix.
The source-text byte character escape value-range diagnostic now preserves code
`1018` for spellings such as `'\x100'`, `'\777'`, `'\u0080'`,
`'\U00000080'`, and `'\u{80}'` through lexer, parser, and driver paths without
requiring a hosted compiler fix.
The source-text byte character exactly-one-byte diagnostic now preserves code
`1019` for too-long spellings such as `'ab'` and `'\nA'`, plus EOF after one
direct byte payload, through lexer, parser, and driver paths without requiring
a hosted compiler fix.
The source-text direct non-ASCII byte character diagnostic now preserves code
`1020` for UTF-8 byte spellings through lexer, parser, and driver paths without
requiring a hosted compiler fix.
The source-text carriage-return whitespace span now matches stage0's whitespace
skipper without requiring a hosted compiler fix.
The source-text EOF line-comment span smoke now matches stage0's scanner by
treating `//` through end of input as one whitespace token and advancing to EOF
without requiring a hosted compiler fix.
The source-text comment-only parser and driver smokes now check that skipped
line and block comments preserve parser EOF diagnostic code `2001`, with
parser failure offsets at the end of the skipped comment, without requiring a
hosted compiler fix.
The source-text trailing-comment handoff smoke now checks line and block
comments after a statement token through lexer EOF placement, parser success,
and driver `Ok(0)` paths without requiring a hosted compiler fix.
The source-text trailing unterminated block-comment smoke now checks both
`name/* open` and `name /* open` style failures through lexer handoff, parser,
and driver paths, preserving diagnostic code `1008` and failure spans without
requiring a hosted compiler fix.
The source-text post-comment extra-token smoke now checks line and block
comments followed by another identifier through lexer handoff, parser, and
driver paths, preserving parser missing-EOF diagnostic code `2003` and the
extra-token span without requiring a hosted compiler fix.
The source-text post-comment lexer-failure smoke now checks line and block
comments followed by an unterminated string through lexer handoff, parser, and
driver paths, preserving lexer diagnostic code `1004` and failure spans without
requiring a hosted compiler fix.
The source-text leading-comment parser and driver handoff smoke now checks line
and block comments before an identifier, preserving parser statement spans and
driver `Ok(0)` results without requiring a hosted compiler fix.
The source-text leading-comment lexer-failure smoke now checks line and block
comments before an unterminated string through lexer handoff, parser, and
driver paths, preserving lexer diagnostic code `1004` and failure spans without
requiring a hosted compiler fix.
The source-text leading-comment extra-token smoke now checks line and block
comments before a statement followed by another identifier through lexer
handoff, parser, and driver paths, preserving parser missing-EOF diagnostic
code `2003` and the extra-token span without requiring a hosted compiler fix.
The source-text leading-comment unterminated block-comment smoke now checks
line and block comments before an unterminated block comment through lexer
handoff, parser, and driver paths, preserving lexer diagnostic code `1008` and
failure spans without requiring a hosted compiler fix.
The source-text leading-comment keyword smoke now checks line and block
comments before `false` through HashMap-backed keyword-table handoff, parser,
and driver paths, preserving parser unsupported-token diagnostic code `2006`
and keyword spans without requiring a hosted compiler fix.
The source-text leading-comment unknown-token smoke now checks line and block
comments before `$` through lexer handoff, parser, keyword-table parser, and
driver paths, preserving parser unknown-token diagnostic code `2005` and
unknown-token spans without requiring a hosted compiler fix.
The source-text leading-comment punctuation smoke now checks line and block
comments before `;` through lexer handoff, parser, keyword-table parser, and
driver paths, preserving parser unsupported-token diagnostic code `2006` and
punctuation spans without requiring a hosted compiler fix.
The source-text leading-comment operator smoke now checks line and block
comments before `+` through lexer handoff, parser, keyword-table parser, and
driver paths, preserving parser unsupported-token diagnostic code `2006` and
operator spans without requiring a hosted compiler fix.
The source-text leading-comment string-literal smoke now checks line and block
comments before `"ari"` through lexer handoff, parser, keyword-table parser,
and driver paths, preserving parser unsupported-token diagnostic code `2006`
and string-literal spans without requiring a hosted compiler fix.
The source-text leading-comment numeric missing-base-digit smoke now checks
line and block comments before `0x` through lexer handoff, parser,
keyword-table parser, and driver paths, preserving lexer diagnostic code
`1009` and failure spans without requiring a hosted compiler fix.
The source-text leading-comment invalid base-digit smoke now checks line and
block comments before `0b102` through lexer handoff, parser, keyword-table
parser, and driver paths, preserving lexer diagnostic code `1010` and failure
spans without requiring a hosted compiler fix.
The source-text leading-comment unsupported number-suffix smoke now checks line
and block comments before `42abc` through lexer handoff, parser, keyword-table
parser, and driver paths, preserving lexer diagnostic code `1011` and failure
spans without requiring a hosted compiler fix.
The source-text leading-comment non-decimal float-suffix smoke now checks line
and block comments before `0b1010f32` through lexer handoff, parser,
keyword-table parser, and driver paths, preserving lexer diagnostic code `1012`
and failure spans without requiring a hosted compiler fix.
The source-text leading-comment non-decimal float-dot smoke now checks line and
block comments before `0x2.0` through lexer handoff, parser, keyword-table
parser, and driver paths, preserving lexer diagnostic code `1013` and failure
spans without requiring a hosted compiler fix.
The source-text leading-comment byte-character literal smoke now checks line
and block comments before `'a'` through lexer handoff, parser, keyword-table
parser, and driver paths, preserving the number-token span and driver `Ok(0)`
without requiring a hosted compiler fix.
The source-text leading-comment byte-character escape smoke now checks line and
block comments before `'\n'` through lexer handoff, parser, keyword-table
parser, and driver paths, preserving the number-token span and driver `Ok(0)`
without requiring a hosted compiler fix.
The source-text leading-comment byte-character numeric escape smoke now checks
line and block comments before `'\x41'` and `'\101'` through lexer handoff,
parser, keyword-table parser, and driver paths, preserving the number-token
span and driver `Ok(0)` without requiring a hosted compiler fix.
The source-text leading-comment byte-character fixed-width Unicode escape smoke
now checks line and block comments before `'\u0041'` and `'\U00000041'`
through lexer handoff, parser, keyword-table parser, and driver paths,
preserving the number-token span and driver `Ok(0)` without requiring a hosted
compiler fix.
The review pass after the recent leading-comment smoke work replaced copied
line/block driver, lexer handoff, and parser checks with explicit line/block
helpers. This was test-structure cleanup in the Ari bootstrap fixture, not a
hosted compiler bug or stage0 behavior change.
A follow-up review pass moved the older comment-only, trailing-comment, and
post-comment source-text driver smokes onto the same line/block helper where
their expected-code shape matched it. The trailing unterminated-block driver
smoke intentionally stays separate because it compares direct and spaced block
comment forms instead of line and block comment equivalents.
A later review pass removed the only remaining manual `std::zone::create` /
`std::zone::destroy` pair from the source-root smoke and used the existing
lexical `zone(65536)` style for the keyword-table fixture. The same pass moved
copied parser keyword failure checks into one helper-scored keyword set and
backfilled table/parser coverage for `fn`, `const`, `as`, and `meta`; this was
fixture cleanup and coverage tightening, not a hosted compiler bug.
The keyword-table fixture was then split again into a flow helper and a
keyword-set helper so the smoke no longer carries a long `var score` /
`if score == 0` chain. This was another source-root smoke readability cleanup,
not a hosted compiler behavior change.

When Ari-written compiler work exposes behavior that looks wrong in the current
C++ hosted compiler, keep it separate from the Ari-written compiler task list.
Record the smallest Ari repro, expected behavior, actual behavior, focused
check target, and whether it belongs in parser, modules, generics, ownership,
codegen, diagnostics, or another hosted compiler area.

Desired stage0 pressure that is not yet classified as a bug:

- The compatibility keyword lookup still lives in stateless
  `identifier_kind_from_text`; keep it only for focused legacy smokes while
  adding future source-text keywords to the reusable `KeywordTable` path.
- Keep the reusable keyword-table source-root smoke data-driven through its
  table-case helper as future keywords are backfilled; new cases should add one
  helper call rather than another copied cursor-check block.
- The CRLF line-comment parser/driver, lexer-failure, extra-token, handoff,
  and post-comment extra-token/lexer-failure slices added more narrow
  source-root smoke helpers. That is acceptable for these tiny coverage steps,
  but the fixture is continuing to grow by scenario-specific score helpers; once the
  hosted compiler makes richer table data easier to express, these comment
  path smokes should move toward table-shaped cases instead of one helper per
  spelling.
- `score`-named helpers are acceptable only inside source-root smoke fixtures
  as test-only exit-code arithmetic. Production `compiler/*.ari` APIs should
  use named accessors, predicates, or stable classifier/query names instead.
  The loaded-source score helper was removed from `compiler/source.ari`; the
  diagnostic aggregate score helper was removed from `compiler/diagnostic.ari`;
  diagnostic severity queries now use rank naming; the AST aggregate score
  helper was removed from `compiler/ast.ari` and replaced by explicit
  node-kind-rank/span/value queries; the token aggregate score helper was
  removed from `compiler/token.ari` and replaced by explicit token-kind-rank
  and token-kind-class-rank queries; lexer score helpers were renamed to
  ranked-width/ranked-position queries; parser kind score helpers were renamed
  to kind-rank queries, and parser result aggregate arithmetic was moved into
  the source-root smoke as a test-only helper. No public `score`-named helpers
  should remain in `compiler/*.ari`; remaining `score` names are fixture-local
  or language/example uses unless a future audit proves otherwise.
- Wrapping a zone-backed `HashMap` in a new Ari struct was awkward in this
  slice: mutating a `HashMap` through a helper/field lost tracked-zone receiver
  information, and returning a wrapper with a raw zone pointer or embedded map
  triggered zone-escape diagnostics. The accepted implementation uses a public
  type alias instead. This is recorded as zone-backed wrapper ergonomics
  pressure, not a confirmed hosted compiler bug.
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
- Zone ergonomics are now a major bootstrap pressure point. The source-root
  smoke has many tiny helper functions that each open their own `zone(65536)`
  only so they can build a `String`, read a file, or construct a keyword table.
  `compiler/driver.ari` also opens nested zones for `run_source_text` and
  `run_from_context`, even though those calls are conceptually part of one
  short-lived compilation request. The upside of the current explicit spelling
  is that lifetime and allocation capacity stay visible, and tests do not hide
  allocation behind a process-global heap. The downside is real: the repeated
  zone blocks are noisy, magic-capacity-driven, easy to cargo-cult into every
  small helper, and make otherwise simple source-text tests look worse than the
  compiler logic being tested. Prefer future work that lets allocation-backed
  stdlib calls derive a lexical current zone/region when exactly one allocation
  lifetime is in scope, while still requiring explicit zone arguments when
  multiple regions or escaping ownership would make inference ambiguous. Avoid
  a runtime-global heap/current-zone API as the default model; the goal is a
  compiler-checkable lexical allocation context, not hidden ambient state.
- `std::ascii` has many useful byte classification helpers, but no direct
  `is_ascii` or `is_non_ascii` helper. The Ari-written lexer currently keeps a
  local `is_non_ascii_byte` helper for direct byte-character literal checks.
  This is stdlib ergonomics pressure, not a confirmed hosted compiler bug.
- Clearer match-arm binding scoping ergonomics; today a helper that matches
  both `std::Ok(code)` and `std::Err(code)`, or sibling enum cases with the
  same payload spelling, must use distinct payload names.
- Richer Ari-written diagnostic identity is still needed before these
  diagnostics can carry stage0-style stable string codes such as `L0001`; this
  slice keeps numeric bootstrap codes and does not classify that as a hosted
  compiler bug.
- The Ari-written token model still has a single `Number` token without
  stage0-style integer/float value payloads or literal suffix metadata. Simple
  byte character literal spans are therefore modeled as number tokens for now;
  this is Ari-written compiler model pressure, not a confirmed hosted compiler
  bug.
- Numeric base-prefix handling such as `0x`, `0o`, and `0b` should stay
  separated from decimal floating literal handling. Decimal float scanning
  should not accidentally inherit octal/binary/hex prefix behavior; keep this
  as lexer design pressure until the Ari-written numeric literal model is
  richer.
- The byte-character span helper now uses an enum-shaped scan result instead
  of a sentinel fallback end. Malformed byte-character diagnostics can grow on
  that shape later without adding more sentinel meanings.
- Ari-written string escape scanning now distinguishes supported
  single-character escape heads from unsupported ones and validates the digit
  shape of `\x`, fixed-width `\u`, fixed-width `\U`, and braced `\u{...}`
  spellings plus byte, Unicode scalar, and octal value ranges. Source-text
  comment skipping now covers line comments, nested block comments, and
  unterminated block comment diagnostics. Numeric base-prefix literal spans now
  cover valid lowercase and uppercase `0x`, `0o`, and `0b` prefixes, and
  exact-width integer suffix spans cover decimal and base-prefixed integers.
  Numeric base-prefix diagnostics now cover missing prefix digits, invalid
  binary/octal digits, and non-decimal float dot spellings. Numeric suffix
  diagnostics now cover unsupported suffixes and non-decimal float suffixes.
  Decimal floating literal spans now cover fractional and exponent spellings.
  Simple byte character literal spans now cover ASCII spellings such as `'a'`,
  and simple byte character escape spans now cover spellings such as `'\n'`
  and `'\\'`. Numeric byte character escape spans now cover hex and octal
  spellings such as `'\x41'` and `'\101'`. Fixed-width byte character Unicode
  escape spans now cover ASCII-valued spellings such as `'\u0041'` and
  `'\U00000041'`. Braced Unicode byte character escape spans now cover
  ASCII-valued spellings such as `'\u{41}'`, including the leading-comment
  source-text driver, lexer handoff, parser, and keyword-table parser paths.
  Empty byte character diagnostics now cover `''`, and unterminated byte
  character diagnostics now cover direct EOF/newline plus escaped EOF/newline
  spellings. Unsupported byte character escape diagnostics now cover spellings
  such as `'\q'`. Byte character escape digit-shape diagnostics now cover
  missing/invalid `\x`, fixed-width Unicode, and braced Unicode digits. Byte
  character escape value-range diagnostics now cover oversized hex/octal byte
  escapes and non-ASCII Unicode escapes. Byte character exactly-one-byte
  diagnostics now cover too-long direct and escaped byte spellings plus EOF
  after one direct byte payload. Direct non-ASCII byte character diagnostics
  now cover UTF-8 byte spellings. Carriage-return whitespace spans now match
  stage0's whitespace skipper.
  CRLF line-comment spans now match stage0's line comment scanner. EOF
  line-comment span coverage now has source-text parser, keyword-table parser,
  and driver coverage for `// eof`, preserving the EOF diagnostic offset at
  byte 6. CRLF line-comment source-text parser, keyword-table parser, and
  driver coverage now checks that carriage return stays inside the skipped
  comment span while the following statement starts at byte 9. CRLF
  line-comment lexer-failure parser, keyword-table parser, and driver coverage
  now checks that `// skip\r\n"open` preserves diagnostic code `1004` at span
  `9..14`. CRLF line-comment extra-token parser, keyword-table parser, and
  driver coverage now checks that `// skip\r\nname other` preserves diagnostic
  code `2003` at span `14..19`. CRLF line-comment extra-token handoff coverage
  now checks that the handoff first token remains at `9..13` and the extra
  token stays visible in the handoff EOF slot at `14..19`. CRLF post-comment
  extra-token parser, keyword-table parser, and driver coverage now checks
  that `name// skip\r\nother` preserves diagnostic code `2003` at span
  `13..18`. CRLF post-comment extra-token handoff coverage now checks that
  the first token remains at `0..4` and the extra token stays visible in the
  handoff EOF slot at `13..18`. CRLF post-comment lexer-failure parser,
  keyword-table parser, and driver coverage now checks that
  `name// skip\r\n"open` preserves diagnostic code `1004` at span `13..18`.
  CRLF post-comment lexer-failure handoff coverage now checks the same
  diagnostic code and span through the lexer handoff result path. The next
  source-model step now has a minimal source table entry shape with a scalar
  path id placeholder, and entry-local span validation now checks source-id
  ownership and byte bounds. Entry-local span construction now avoids manually
  repeating the source id at call sites. The source model now also has a
  minimal line-start summary shape with first line start, optional second line
  start, and count metadata, checked through source-root smoke coverage without
  requiring a hosted compiler fix. It now has a one-based byte-to-line-number
  helper, and the helper forced the minimal summary validity rule to reject
  counts above two because the current shape only stores first and second line
  starts. It also rejects stale second-line metadata on one-line summaries.
  A one-based byte-column helper now derives columns from the current line
  start and clamps offsets past EOF without requiring a hosted compiler fix.
  A minimal source location summary now carries source id, clamped byte offset,
  one-based line number, and one-based byte column for source-model handoff
  checks without requiring a hosted compiler fix. Source-table-entry-local
  location summaries now reuse the entry's `SourceFile` and preserve the same
  invalid-line-table and EOF-clamp behavior without requiring a hosted compiler
  fix. Span-start location summaries now reject source-id mismatches, reversed
  spans, and out-of-range spans before deriving line and byte-column metadata,
  without requiring a hosted compiler fix. Span-end location summaries share
  the same validity rule and preserve EOF insertion-point locations without
  requiring a hosted compiler fix. Source-table entry span-start location
  summaries delegate through the entry's `SourceFile`, avoiding duplicated
  entry unwrapping without requiring a hosted compiler fix. Source-table entry
  span-end location summaries use the same delegation and preserve EOF
  insertion-point behavior without requiring a hosted compiler fix. Full source
  table ownership, file path strings, loaded text ownership, owned line-start
  arrays, CRLF visual-column handling, UTF-8 byte-column policy, and diagnostic
  location mapping remain pending. Loaded-source-local location summaries reuse
  the placeholder's `SourceFile` metadata and do not require a hosted compiler
  fix. Loaded-source-local span-start location summaries share the same
  `SourceFile` delegation and do not require a hosted compiler fix.
  Loaded-source-local span-end location summaries preserve EOF insertion-point
  behavior through the same delegation and do not require a hosted compiler fix.
  Loaded-source span construction now reuses the placeholder source id without
  requiring a hosted compiler fix. Source-file byte-length validity is now an
  explicit non-negative invariant used by containment checks and does not
  require a hosted compiler fix. Source-table-entry byte-length validity now
  reuses the wrapped source-file invariant and does not require a hosted
  compiler fix. Loaded-source byte-length validity now reuses the wrapped
  source-file invariant and does not require a hosted compiler fix. Loaded-source
  first-offset validity now makes one-byte availability explicit through the
  loaded source's `SourceFile` containment rule without requiring a hosted
  compiler fix. Loaded-source span validity now reuses the same file-local
  source-id, ordering, and byte-bound checks as source-table entries, without
  requiring a hosted compiler fix. Loaded-source byte containment now delegates
  through the wrapped `SourceFile` without requiring a hosted compiler fix.
  Loaded-source first-byte spans now derive `first_offset..first_offset + 1`
  from the summary without requiring a hosted compiler fix. Loaded-source
  first-byte location summaries now reuse that span plus the loaded-source
  span-start location helper without requiring a hosted compiler fix.
