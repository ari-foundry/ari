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
`KwPtr`, `KwReturn`, and `KwIf`, plus longer `structure`, `external`,
`enumerate`, `traitor`, `dynamic`, `matches`, `module`, `public`, `user`,
`implicit`, `forest`, `inside`, `letter`, `variant`, `owner`, `reference`,
`mutable`, `ptrace`, `returning`, and `iffy`, and the source-text parser/driver
keyword path without requiring a hosted compiler fix. The AST statement-kind
query and parser payload-shape smoke checked successful statement output without
requiring a hosted compiler fix. The AST node span-length query and
parser payload-span smoke checked
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

- The compatibility keyword lookup still lives in stateless
  `identifier_kind_from_text`; keep it only for focused legacy smokes while
  adding future source-text keywords to the reusable `KeywordTable` path.
- Keep the reusable keyword-table source-root smoke data-driven through its
  table-case helper as future keywords are backfilled; new cases should add one
  helper call rather than another copied cursor-check block.
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
- Clearer match-arm binding scoping ergonomics; today a helper that matches
  both `std::Ok(code)` and `std::Err(code)` must use distinct payload names.
