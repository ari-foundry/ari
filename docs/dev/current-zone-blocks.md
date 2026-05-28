# Current-Zone Blocks

This note documents the compiler-side contract for `zone { ... }` so parser,
semantic-checker, stdlib, and docs changes stay aligned.

## User Contract

`zone { ... }` creates a short-lived temporary allocation zone for the block.
Inside that block, stdlib allocation APIs that normally take exactly one
`ref mut Zone` can omit that argument:

```ari
zone {
  let name = std::string::from("ari");
  let line = format!("package {}", name);
  let bytes = std::encoding::decode_hex("617269")?;
}
```

`zone(capacity) { ... }` uses the same rule with an explicit byte capacity.
`zone { ... }` uses `std::zone::default_capacity()`, currently 4096 bytes.
Explicit zone arguments remain valid and are required whenever a value should
be allocated into a longer-lived caller-owned zone.

If a temporary zone cannot satisfy an allocation, the hosted runtime writes a
short diagnostic to stderr before exiting. Invalid capacities name the valid
range, and capacity exhaustion suggests `zone(capacity) { ... }` or a larger
explicit zone. This is still a runtime trap rather than a recoverable
`Result`; callers that need recovery should size the zone deliberately or use
an API that reports capacity failure before allocating.

## Parser Contract

`zone` is contextual statement syntax, not a lexer keyword. The parser should
recognize a zone block only for these forms:

```ari
zone { statements... }
zone(capacity_expression) { statements... }
```

Other uses of an identifier named `zone` remain ordinary expression/name
syntax. The AST statement kind is `ZoneBlock`; its optional expression is the
capacity expression, and its body is the statement list.

## Semantic Contract

The checker lowers a zone block as a normal block with a hidden `own Zone`
local initialized through the existing `zone::temp` builtin. The hidden local
is pushed on the current-zone stack while the body is checked.

When checking a call while the stack is non-empty, the checker may synthesize a
`ref mut` borrow of the innermost hidden zone if all of these are true:

- the call has exactly one fewer explicit argument than the selected function
  or method signature requires after receiver handling
- exactly one missing candidate parameter is typed as `ref mut Zone`
- inserting the zone makes the remaining explicit arguments type-check in
  their normal positions

The same rule applies to ordinary function calls, generic function calls,
ordinary method calls, associated function calls, trait-qualified method
calls such as `Display::format_in(value)`, and object-safe dyn calls such as
`object.render()`. It also applies to callable values whose `fn(...)` or
closure signature contains the unique omitted zone parameter. The rule is
deliberately arity-based and conservative. Calls with two zone parameters, no
zone parameter, or an otherwise ambiguous omitted argument keep the ordinary
diagnostic and must be written explicitly.

When a call outside a current-zone block is missing exactly one `ref mut Zone`
argument, the checker reports the allocation-zone issue directly instead of
showing only a wrong-argument-count error. The diagnostic should tell users to
wrap local temporary work in `zone { ... }` or pass an explicit zone when the
result needs a longer lifetime.

`format!` is special compiler syntax. Inside a current-zone block it lowers as
`format_in!(ref mut current_zone, ...)`. Outside a current-zone block it stays
a targeted diagnostic.

## Cleanup And Escape Contract

The hidden zone receives the same automatic cleanup as a lexical
`zone::temp(...)` local:

- cleanup is emitted on normal fallthrough
- cleanup is emitted before returns that leave the scope
- cleanup is emitted before `break`, `continue`, or labeled-block exits that
  leave the scope
- ordinary zone-provenance checks still reject pointers or zone-backed handles
  escaping the block

The syntax does not create a process-global heap and does not relax reset or
destroy invalidation. It only shortens the spelling for a local allocation
capability whose lifetime is already obvious from the block.

## Design Direction

The current zone is a lexical compiler capability, not a runtime global that
library code can fetch arbitrarily. That keeps allocation visible in source:
`zone { ... }` means "use this block's temporary zone unless I pass a
different one." Future ergonomics should keep that property. Good next steps
are cleaner resource-block spelling and capacity planning helpers; a hidden
global allocator or long-lived ambient heap would work against Ari's explicit
allocation model.

## Stdlib Guidance

Zone-taking stdlib APIs should keep using a real `ref mut Zone` parameter.
Prefer one clear zone parameter per allocating operation so current-zone
insertion remains predictable. Use `_in` names when the target zone is a
semantic part of the operation, but document that callers may omit the zone
inside a current-zone block.

When adding a public zone-backed API:

1. Add the source declaration.
2. Add or update focused ordinary tests that compile the explicit-zone spelling
   and, when useful, a `zone { ... }` spelling.
3. Update `tests/std_api_manifest.txt` and regenerate
   `docs/stdlib/generated/api-index.md` if the public API changed.
4. Update the hand-written module guide with lifetime and current-zone notes.

For stdlib implementation work, prefer `zone::of(ref handle)` or
`ZoneMetadata` when a heap-backed handle needs to grow later. The handle should
recover its zone from backing allocation metadata instead of storing an extra
zone field. Use `zone::capacity`, `zone::used`, and `zone::remaining` only for
planning, diagnostics, and tests; allocation still goes through a real zone
capability or recovered metadata.

## Remaining Work

- a future expression form only if the language gets a clean block-expression
  story; the current feature is intentionally statement-only
- optional compile-time or library-side sizing helpers for common scratch
  workloads, so large parser/formatter operations can choose a capacity before
  entering the block
