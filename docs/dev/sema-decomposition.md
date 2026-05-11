# Semantic Checker Decomposition Roadmap

`src/sema.cpp` is still the compiler's largest coordination point. It owns name
resolution, type resolution, trait selection, ownership and borrow state,
pattern analysis, control-flow lowering, prelude lowering hooks, and IR
construction. Some helpers have already moved out to focused files:

- `attribute_semantics` for attribute validation helpers
- `prelude_resolver` for compiler-known standard-library spellings
- `try_model` for `?` residual shape helpers
- `constant_semantics` for the shared constant value model, scalar constant
  construction/checking, and static integer arithmetic/bitwise/shift folding
  used by constant-like local reasoning and local Vec capacity/length
  decisions, plus constant-cycle path formatting and constant pattern lowering
  for scalar matches and enum payload conditions
- `type_semantics` for shared type predicates, raw-pointer type checks,
  literal range checks, and assignability/operand diagnostics
- `vector_semantics` for local `Vec[T]` storage type/helpers, typed empty
  vector literal construction, frozen local method classification and shape
  diagnostics, shared `len`/`is_empty`/`as_slice` shape diagnostics, shared
  collection `len` lowering, `as_slice` data/view IR construction including
  Vec storage views, local Vec integer/non-negative and known-index operand
  diagnostics, direct local Vec index diagnostics, known-empty element/indexed
  method diagnostics, frozen API diagnostics, local Vec IR construction helpers
  such as `first`, `last`, and `push`, and `VectorKnownLength`/capacity
  transition helpers used by local `len`/`is_empty`, `as_slice` length, and
  stored-vector `for` loop bound constant lowering, plus local Vec source-AST
  known-length and storage-capacity merging with semantic local-name callbacks
  for direct `len`/`is_empty` folding, direct index diagnostics, local-vector
  initialization, assignment, stored-vector `for` loop bounds, local-binding
  copies inside `if`/block/labeled-block/`match`/`if let` expression results,
  typed labeled-block break result merging, and nested result storage-capacity
  reads
- `ir_builders` for basic IR node construction helpers such as local lvalues,
  var declarations, tuple/vector indexes, arbitrary-operand tuple/index access
  nodes, scalar/string/null literals, tuple/struct/tuple-struct/range
  aggregate construction, casts, bool conditions, pointer operations, direct
  calls, generic call specializations, inherent/trait associated calls,
  trait-qualified calls, method calls, zone helper calls, builtin calls, match
  expression arms, block/match/if expression nodes, and format-print payload
  assembly
- `control_flow_semantics` for product-pattern if-chain assembly shared by
  aggregate match, declaration, `if let`, and `while let` lowering, while
  still routing expression blocks and conditionals through `ir_builders`
- `module_metadata` and `module_cache` for package graph summaries and caches
- `product_coverage` for symbolic product-rectangle coverage math used by
  aggregate pattern exhaustiveness diagnostics
- `pattern_coverage` for scalar integer match interval coverage, including
  signed-pattern ordering, range interval merging, scalar exhaustiveness, and
  fully-shadowed scalar range/literal detection, scalar match exhaustiveness
  diagnostics, plus finite product coverage value encoding, finite scalar
  domain enumeration, product match coverage state, product duplicate/shadow
  detection, product exhaustiveness checks, product missing-case hint
  formatting, finite/symbolic product pattern domain lowering, enum match
  coverage state, enum duplicate detection, and enum exhaustiveness diagnostics
- `pattern_semantics` for pure pattern binding/or-pattern detection, positional
  product field mapping, or-pattern expansion helpers, and union-safe pattern
  cloning shared by match/or-pattern normalization and sema iterator-filter
  rewrites
- `for_pattern_semantics` for irrefutable non-iterator `for` loop-head pattern
  validation shared by range, list-literal, and stored-vector loops
- `move_semantics` for pure helpers around explicit ownership-consumption
  syntax such as `take(place)` place-shape validation
- `trait_semantics` for small trait display/key helpers and pure trait-method
  shape checks, including syntactic `self` receiver classification, shared by
  trait impl validation, method dispatch diagnostics, and future
  trait-resolution extraction
- `aggregate_literal_semantics` for pure expected-element selection and shared
  element diagnostics used by tuple, struct, fixed-array, and local `Vec[T]`
  literal lowering
- `enum_constructor_semantics` for expected-enum matching and final enum
  constructor IR node assembly after payload semantic checks
- `ast_clone` for union-safe AST expression cloning shared by parser compound
  assignment lowering and sema borrow-receiver synthesis
- `ast_builders` for small AST expression constructors that keep scalar/name,
  unary/binary/cast/try/index/field/call child wiring, tuple-index, borrow,
  string/null, tuple/vector/struct literal, block, and match payload
  initialization plus macro-call token payload allocation out of parser and sema

IR payload records should also stay compact as more pattern metadata moves out
of `sema.cpp`. `IrPayloadLiteralCondition` now stores its integer-or-bool
literal payload in a tagged union, `Expr`/`IrExpr` scalar literal and
tuple-index payloads now share anonymous unions, and AST `Pattern` integer/bool
literal payloads share a union-backed slot. IR match arms and nested
enum-payload literal conditions also use one active integer-or-bool payload
slot. Parser and sema now use `ast_builders` for scalar/name/tuple-index,
borrow, string/null, composite literal, block, and match AST expression
construction paths that touch those union-backed fields. Broader `Stmt` and
expression child/vector payload packing remains a separate refactor because
those nodes are mutated across parser cloning, semantic lowering, and IR builder
paths. AST macro-call token trees and IR format-print string parts are already
lazy rare payloads, so ordinary expression nodes no longer carry those vector
slots directly. AST `if let`/`while let` condition patterns and `for` loop-head
patterns are lazy payloads too, leaving normal condition expressions and
non-loop statements without embedded `Pattern` objects. AST and IR match
statement arm vectors also live behind match-statement payload pointers, so
non-match statement nodes no longer carry those vector slots directly. Explicit
`drop` statement names, `break` labels/values, and assignment statement
names/targets/RHS expressions are lazy payloads as well. Statement labels are
also lazy on AST/IR statement nodes, leaving unlabeled blocks and loops without
an eager label string. AST/IR block, branch, and loop statement bodies now share
lazy body-vector payloads, so ordinary non-control statements no longer carry
empty body vectors. AST/IR block-expression labels, statement bodies, and final
values now live behind block-expression payloads too, leaving ordinary
expression nodes without those block-only fields. `if` expression conditions,
condition patterns, branch bodies, and branch final values are lazy AST/IR
payloads now as well. Match-expression subjects and expression arm vectors are
lazy AST/IR payloads too, matching the earlier statement-match arm-vector split.
IR trait-object call erased parameter type vectors are also behind a
`TraitObjectCall`-only payload, with sema setting the payload once and the LLVM
backend reading it through `ir_expr_call_param_types`. AST qualified-call
receiver type argument vectors are lazy now as well, so parser, AST clone,
module-summary filtering, and sema trait-qualified lookup share
`expr_receiver_type_args`/`set_expr_receiver_type_args` accessors instead of
touching the storage directly. AST struct-literal field names also live behind
a lazy vector payload, with parser/builders, AST clone, module summaries,
constant evaluation, and sema field mapping using `expr_field_names` accessors.
Direct AST expression type arguments now use the lazy `expr_type_args` payload
as well. Parser generic-call construction, AST cloning, module-summary constant
serialization, constant evaluation, and sema generic function/constructor lookup
all read or move the payload through `expr_type_args`, `set_expr_type_args`, and
`take_expr_type_args`.
AST and IR expression argument vectors now use the shared `LazyVector` wrapper
from `lazy_vector.hpp`. It preserves the existing `expr.args` API while moving
empty tuple/vector/struct/call child lists out of ordinary scalar/name/control
expression nodes. This keeps the current parser, sema, and backend call sites
stable while still shrinking common AST/IR nodes; stored-vector for-loop lowering
uses `take()` when it transfers an IR vector literal's children into the lowered
statement.
AST operand child storage now lives behind `ExprChildPayload`, and the parser
builds unary/binary/cast/try/index/field/call/method-call nodes through
`ast_builders` and `expr_operand`/`expr_left`/`expr_right` accessors. AST
cloning, declaration summary materialization, compound-assignment target
cloning, and sema's synthetic borrow-receiver cloning also use those helpers.
Semantic AST lowering, constant folding/evaluation, explicit move-place
validation, local Vec method receiver handling, indirect calls, borrowed method
receivers, and binary/try/coalesce lowering now read AST child expressions
through the same helpers. Childless AST expressions avoid three eager
`unique_ptr` members. IR `operand`/`left`/`right` child storage now follows the
same shape behind `IrExprChildPayload`, with IR builders and sema construction
writing through setters and constant folding plus both backends reading through
accessors. `IrExpr::payload` remains separate because vector replacement and
removal lowerings use it as a distinct rare side input rather than a common
unary/binary child slot.

The next refactors should keep behavior unchanged and move one responsibility at
a time behind small data-oriented APIs. Prefer patches that add focused tests or
reuse the existing feature check target for the moved responsibility.

## Phase 1: Pure Helpers

These areas can move first because they mostly depend on `IrType`, `Pattern`,
or `SourceLocation`, not on the whole `SemanticChecker` state.

1. Keep basic IR construction centralized in `ir_builders`.
   - Local lvalues, IR var declarations, tuple/vector index helpers, scalar
     literals, string/null literal helpers, tuple/struct/tuple-struct/range
     aggregate construction, arbitrary-operand tuple/index access nodes, casts,
     bool binary conditions, pointer operation nodes, direct call nodes, generic
     call specializations, inherent/trait associated call nodes,
     trait-qualified call nodes, method call nodes, zone helper call nodes, and
     direct builtin call nodes are already outside `sema.cpp`; format-print
     node assembly is also centralized there so its rare string-part payload is
     not a field on every IR expression. Enum constructor IR assembly now lives
     with `enum_constructor_semantics` because it shares enum-specific layout
     decisions.
   - Block, match, and if expression node assembly now also goes through
     `ir_builders`. Future builder moves should be opportunistic and tied to a
     nearby semantic extraction, rather than treated as a standalone phase.
   - AST expression cloning for assignment targets and synthetic borrow
     receivers now lives in `ast_clone`, so future AST payload packing has one
     place to preserve variant-specific copy rules.
2. Keep constant evaluation helpers in `constant_semantics`.
   - Static integer arithmetic/bitwise/shift folding, `ConstantValue`, scalar
     constant construction/range helpers, scalar literal folding, constant
     binary result evaluation, and constant-to-IR literal construction have
     moved into
     `constant_semantics`.
   - Constant-cycle diagnostics now use a small stack guard and path formatter
     in `constant_semantics`.
   - Constant pattern conversion for scalar match arms, compact enum payload
     literals, aggregate enum payload literal conditions, and nested enum
     payload literal conditions also lives in `constant_semantics`.
   - The constant declaration table intentionally remains owned by
     `SemanticChecker` until declaration collection moves out as a stateful
     table extraction.
3. Continue extracting pattern coverage helpers into `pattern_coverage`.
   - Product rectangle math now lives in `product_coverage`.
   - Pure pattern binding/or-pattern detection, positional tuple/array field
     mapping for rest patterns, and expansion helpers now live in
     `pattern_semantics`.
   - Scalar integer range interval math now lives in `pattern_coverage`,
     including signed ordering, interval merging, scalar exhaustiveness, and
     scalar fully-shadowed checks.
   - Scalar bool/integer match exhaustiveness diagnostics now also live in
     `pattern_coverage`; `sema.cpp` only raises the returned message.
   - Finite product coverage value encoding, finite scalar product domains,
     finite aggregate product domain recursion, and finite product domain
     combination now also live in `pattern_coverage`.
   - Symbolic product universe/domain recursion for tuple, array, and struct
     values also lives in `pattern_coverage`.
   - Finite/symbolic product pattern domain lowering for tuple, array, named
     struct, and tuple-struct patterns now lives in `pattern_coverage` behind
     semantic callbacks for constant lookup, struct field lookup, and
     tuple-struct validation.
   - Product match coverage state and product missing-case hint formatting now
     live in `pattern_coverage`; `sema.cpp` only supplies tuple-struct names for
     user-facing struct-pattern spelling.
   - Product duplicate/shadow detection and final product coverage
     exhaustiveness checks now live in `pattern_coverage`.
   - Enum match coverage state, duplicate case/payload detection,
     bool-payload literal classification, bool-payload coverage promotion, and
     enum exhaustiveness diagnostics now live in `pattern_coverage`;
     `sema.cpp` still performs enum case resolution and payload type
     validation.
   - Irrefutable non-iterator `for` loop-head validation now lives in
     `for_pattern_semantics`; `sema.cpp` only wires struct lookup callbacks and
     emits the actual loop binding locals while broader pattern-binding
     lowering stays stateful.
   - Move the remaining semantic-table callbacks for product pattern coverage
     after struct/type declaration tables are extracted.
   - Leave binding emission in `SemanticChecker` until local-scope mutation is
     abstracted.

## Phase 2: Stateful Subsystems

These need a narrow context object because they mutate scopes, locals, or
pending IR.

1. Extract lexical scopes and local state into `local_state`.
   - Own `LocalInfo`, scope push/pop/discard, state snapshots, local lookup,
     and mutable/immutable local checks.
   - Expose a small API for declaration, assignment, move/drop, and scope
     cleanup.
2. Extract borrow checking into `borrow_semantics`.
   - Move named borrow tracking, aggregate borrow source tracking, temporary
     borrow promotion/release, and path borrow conflicts.
   - Keep it layered over `local_state` so later NLL work has one entry point.
3. Extract ownership/drop checking into `ownership_semantics`.
   - Move owned field state tracking, partial move/reinitialization checks,
     `drop` lowering, destructor lookup glue, and return-owner checks.
   - Keep trait lookup calls injected from `SemanticChecker` until trait tables
     move.
4. Extract zone pointer provenance into `zone_semantics`.
   - Move zone pointer source/generation tracking, temp-zone escape checks,
     reset/destroy invalidation, scratch/promotion validation, and automatic
     temporary-zone cleanup.
   - Keep actual `zone::*` call typing in expression lowering until prelude
     special calls are split.

## Phase 3: Name And Type Tables

These moves should happen after pure helpers and local state have settled.

1. Extract module/use resolution into `name_resolution`.
   - Own module declarations, use aliases, glob imports, visibility checks,
     implicit `std` aliases, and `self::`/`super::` resolution.
   - Provide resolved names for functions, types, traits, constants, enum
     cases, and modules.
2. Extract declaration collection into `declaration_tables`.
   - Own function signatures, struct/enum/trait tables, impl tables,
     constants, extern functions, metadata needed for IR export, and duplicate
     diagnostics.
   - Keep monomorphization queues in `SemanticChecker` until generic lowering
     is split.
3. Extract generic and trait resolution into `trait_semantics`.
   - Move generic binding/inference, trait bound validation, impl coherence,
     inherent and trait method selection, associated calls, trait object
     conversion, vtable planning, and object-safety checks.
   - Pure trait-call helpers already live in `trait_semantics`, including
     display keys, `Self` receiver recognition, and whether an expected value
     type can select the implementing type for a trait-qualified associated
     call.
   - Use `name_resolution` and `declaration_tables` as read-only inputs where
     possible.

## Phase 4: Lowering Passes

After the tables and state trackers are separated, split IR lowering by syntax
surface.

1. Extract expression lowering into `expr_lowering`.
   - Move scalar operators, casts, calls, method calls, aggregate literals,
     index/field access, `if`/`match`/block expressions, macros, and prelude
     special call lowering.
2. Extract statement lowering into `stmt_lowering`.
   - Move `let`/`var`, assignment, return, loops, `break`/`continue`, statement
     `match`, block statements, and cleanup insertion.
3. Extract pattern binding lowering into `pattern_binding`.
   - Move `let`/`var`, function parameter, for-loop, if-let/while-let, and
     match-arm binding emission onto one shared engine.
4. Extract monomorphization into `monomorphization`.
   - Move pending generic function/impl method queues, specialization naming,
     type substitution, and repeated lowering of specialized bodies.

## Suggested Order

1. `pattern_coverage`
2. `local_state`
3. `borrow_semantics`
4. `zone_semantics`
5. `name_resolution`
6. `trait_semantics`
7. `expr_lowering` / `stmt_lowering`

This order keeps early patches mostly pure, then moves stateful pieces only
after their dependencies have a stable API. The end state should make
`SemanticChecker` a coordinator that wires together tables, local state,
diagnostics, and lowering passes rather than directly owning every rule.
