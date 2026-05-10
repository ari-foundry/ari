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
  used by constant-like local reasoning and local Vec capacity/length decisions
- `type_semantics` for shared type predicates, raw-pointer type checks,
  literal range checks, and assignability/operand diagnostics
- `vector_semantics` for local `Vec[T]` storage helpers and known length/capacity
  state transitions
- `ir_builders` for basic IR node construction helpers such as local lvalues,
  var declarations, tuple/vector indexes, literals, casts, bool conditions,
  pointer operations, and direct builtin calls
- `control_flow_semantics` for product-pattern if-chain assembly shared by
  aggregate match, declaration, `if let`, and `while let` lowering
- `module_metadata` and `module_cache` for package graph summaries and caches
- `product_coverage` for symbolic product-rectangle coverage math used by
  aggregate pattern exhaustiveness diagnostics
- `pattern_semantics` for pure pattern binding/or-pattern detection and
  expansion helpers
- `move_semantics` for pure helpers around explicit ownership-consumption
  syntax such as `take(place)` place-shape validation
- `trait_semantics` for small trait display/key helpers and pure trait-method
  shape checks shared by trait impl validation, method dispatch diagnostics,
  and future trait-resolution extraction

The next refactors should keep behavior unchanged and move one responsibility at
a time behind small data-oriented APIs. Prefer patches that add focused tests or
reuse the existing feature check target for the moved responsibility.

## Phase 1: Pure Helpers

These areas can move first because they mostly depend on `IrType`, `Pattern`,
or `SourceLocation`, not on the whole `SemanticChecker` state.

1. Continue extracting basic IR construction into `ir_builders`.
   - Local lvalues, IR var declarations, tuple/vector index helpers, scalar
     literals, casts, bool binary conditions, pointer operation nodes, and
     direct builtin call nodes are already outside `sema.cpp`.
   - Next small targets are enum constructors and block/match/if expression
     assembly once their semantic checks have narrow inputs.
2. Extract constant evaluation into `constant_semantics`.
   - Static integer arithmetic/bitwise/shift folding, `ConstantValue`, scalar
     constant construction/range helpers, scalar literal folding, constant
     binary result evaluation, and constant-to-IR literal construction have
     moved into
     `constant_semantics`.
   - Move constant pattern conversion and cycle diagnostics next.
   - Keep the constant declaration table owned by `SemanticChecker` initially.
3. Continue extracting pattern coverage helpers into `pattern_coverage`.
   - Product rectangle math now lives in `product_coverage`.
   - Pure pattern binding/or-pattern detection and expansion helpers now live
     in `pattern_semantics`.
   - Move scalar range interval math, finite product domain enumeration,
     duplicate/shadow detection helpers, and the remaining sema-bound
     exhaustiveness formatting.
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

1. `ir_builders` continuation
2. `constant_semantics`
3. `pattern_coverage`
4. `local_state`
5. `borrow_semantics`
6. `zone_semantics`
7. `name_resolution`
8. `trait_semantics`
9. `expr_lowering` / `stmt_lowering`

This order keeps early patches mostly pure, then moves stateful pieces only
after their dependencies have a stable API. The end state should make
`SemanticChecker` a coordinator that wires together tables, local state,
diagnostics, and lowering passes rather than directly owning every rule.
