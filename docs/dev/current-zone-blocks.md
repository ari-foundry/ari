# Current Allocation Blocks

This note documents the compiler-side contract for `region { ... }` and
compatibility `zone { ... }` so parser, semantic-checker, stdlib, and docs
changes stay aligned.

## User Contract

`region { ... }` creates a short-lived temporary allocation region for the
block. This is the preferred user-facing spelling: the hidden owner is a
`std::region::Region`, and the current-source bridge keeps old
`ref mut Zone` APIs callable while the stdlib migrates. `zone { ... }` remains
accepted as compatibility syntax and still creates a hidden low-level `Zone`.
Inside either block, stdlib allocation APIs that normally take exactly one
`ref mut Zone` can omit that argument:

```ari
region {
  let name = std::string::from("ari");
  let line = format!("package {}", name);
  let bytes = std::encoding::decode_hex("617269")?;
}
```

`region(capacity) { ... }` and `zone(capacity) { ... }` use the same current
argument insertion rule with an explicit byte capacity. Default-capacity forms
use 4096 bytes today. Explicit allocation arguments remain valid and are
required whenever a value should be allocated into a longer-lived caller-owned
region or zone.

If a temporary region or zone cannot satisfy an allocation, the hosted runtime
writes a short diagnostic to stderr before exiting. Invalid capacities name the
valid range, and capacity exhaustion suggests `region(capacity) { ... }`,
`zone(capacity) { ... }`, or a larger explicit allocation source. This is
still a runtime trap rather than a recoverable `Result`; callers that need
recovery should size the region deliberately or use an API that reports
capacity failure before allocating.

## Parser Contract

`region` and `zone` are contextual statement syntax, not lexer keywords. The
parser should recognize allocation blocks only for these forms:

```ari
region { statements... }
region(capacity_expression) { statements... }
zone { statements... }
zone(capacity_expression) { statements... }
```

Other uses of identifiers named `region` or `zone` remain ordinary
expression/name syntax. The AST statement kind is still `ZoneBlock` for
compatibility; `zone_block_uses_region` distinguishes the preferred Region
lowering from the compatibility Zone lowering. Its optional expression is the
capacity expression, and its body is the statement list.

## Semantic Contract

The checker lowers a region block as a normal block with a hidden
`own std::region::Region` local initialized through `std::region::create`.
The compatibility zone block keeps the older hidden `own Zone` initialized
through the existing `zone::temp` builtin. The hidden owner is pushed on the
current allocation-source stack while the body is checked.

When checking a call while the stack is non-empty, the checker may synthesize a
`ref mut` borrow of the innermost hidden allocation source if all of these are
true:

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
diagnostic and must be written explicitly. When the hidden source is a Region,
the synthesized argument lowers through `std::region::as_zone`.

When a call outside a current allocation block is missing exactly one
`ref mut Zone` argument, the checker reports the allocation-source issue
directly instead of showing only a wrong-argument-count error. The diagnostic
should tell users to wrap local temporary work in `region { ... }`, use
compatibility
`zone { ... }`, or pass an explicit allocation argument when the result needs a
longer lifetime.

`format!` is special compiler syntax. Inside a current allocation block it
lowers as `format_in!(ref mut current_source, ...)`, with Region blocks using
the same `as_zone` bridge. Outside a current allocation block it stays a
targeted diagnostic.

## Cleanup And Escape Contract

The hidden owner receives automatic cleanup:

- cleanup is emitted on normal fallthrough
- cleanup is emitted before returns that leave the scope
- cleanup is emitted before `break`, `continue`, or labeled-block exits that
  leave the scope
- compatibility Zone blocks emit `zone::destroy`; Region blocks emit
  `std::region::destroy`
- ordinary zone-provenance checks still reject pointers or zone-backed handles
  escaping the block

The syntax does not create a process-global heap and does not relax reset or
destroy invalidation. It only shortens the spelling for a local allocation
capability whose lifetime is already obvious from the block. Do not add a
library-level `zone::current()` or `region::current()` escape hatch unless the
type system also gains a way to express the lifetime of that ambient region.

## Design Direction

The current allocation source is a lexical compiler capability, not a runtime
global that library code can fetch arbitrarily. That keeps allocation visible
in source:
`region { ... }` means "use this block's temporary region unless I pass a
different one." Future ergonomics should keep that property. Good next steps
are capacity planning helpers and broader stdlib examples using `std::region`;
a hidden global allocator or long-lived ambient heap would work against Ari's
explicit allocation model.

## Stdlib Guidance

Zone-taking stdlib APIs should keep using a real `ref mut Zone` parameter
until the module migrates to `Region` or `Allocator`. Prefer one clear zone
parameter per allocating operation so current-source insertion remains
predictable. Use `_in` names when the target zone is a semantic part of the
operation, but document that callers may omit the zone inside a current
allocation block.

When adding a public zone-backed API:

1. Add the source declaration.
2. Add or update focused ordinary tests that compile the explicit-zone spelling
   and, when useful, a `region { ... }` spelling. Keep `zone { ... }` tests for
   compatibility and low-level zone coverage.
3. Update `tests/std_api_manifest.txt` and regenerate
   `docs/stdlib/generated/api-index.md` if the public API changed.
4. Update the hand-written module guide with lifetime and current allocation
   notes.

For stdlib implementation work, prefer `std::allocator::of(ref handle)`,
`std::allocator::of_mut(ref mut handle)`,
`std::allocator::from_region(ref mut region)`, or
`std::region::allocator(ref mut region)` when heap-backed code needs to grow
later. The handle should recover an allocator from backing allocation metadata
instead of storing an extra zone field. Use `region::capacity`,
`region::used`, `region::remaining`, `region::can_alloc`, and
`region::can_alloc_array` for direct user-facing region planning,
diagnostics, and tests; use `zone::*` when touching low-level compatibility
or runtime internals.
For handle methods, prefer `allocator.can_alloc(...)` and
`allocator.can_alloc_array<T>(...)` so the public handle does not need to carry
a duplicate zone pointer and user docs do not have to expose `ZoneMetadata`.

## Remaining Work

- a future expression form only if the language gets a clean block-expression
  story; the current feature is intentionally statement-only
- keep `zone { ... }` as compatibility syntax while docs and examples migrate
  to `region { ... }`
- optional compile-time or library-side sizing helpers for common scratch
  workloads, so large parser/formatter operations can choose a capacity before
  entering the block
- a cleaner named-source spelling for cases where code wants to bind the
  temporary Region and still keep allocation lexical
