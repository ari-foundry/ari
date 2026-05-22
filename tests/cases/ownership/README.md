# Ownership Tests

This folder contains focused fixtures for Ari ownership behavior. Put valid
programs under `ok/` and expected diagnostics under `errors/` when both kinds
exist.

Coverage is grouped by behavior:

- local owner moves, use-after-move, and explicit `drop`
- aggregate field moves, nested field moves, and partial-move diagnostics
- fixed-array and local `Vec[own T]` element moves
- aggregate and runtime enum payload drop lowering
- compiler-shaped parser state, generic aggregate, vector work-item, and
  result-like ownership flow
- owner state joins across loops, branches, `match`, `if let`, and `while let`
- explicit `forget` behavior for live or maybe-unavailable owners

`make check-ownership` is the small smoke target for this folder. It runs one
runtime field-move fixture, one borrow-reborrow fixture, the compiler-shaped
ownership fixture, LLVM drop-lowering checks, and the representative ownership
diagnostics. Broader ownership
coverage remains in `make check-variables` and `make check-errors`.

The compiler-development artifact bucket also locks source-aware ownership
diagnostics and review-sized LLVM drop fragments:

- `diagnostic-use-after-move.diagnostic`
- `diagnostic-borrow-after-move.diagnostic`
- `diagnostic-double-move.diagnostic`
- `diagnostic-move-borrowed-owner.diagnostic`
- `diagnostic-assignment-while-borrowed.diagnostic`
- `diagnostic-field-assignment-while-borrowed.diagnostic`
- `diagnostic-enum-payload-invalid-move.diagnostic`
- `diagnostic-ownership-partial-move.diagnostic`
- `diagnostic-ownership-vector-dynamic-move.diagnostic`
- `backend-ownership-drop-aggregate.llvm-frag`
- `backend-ownership-drop-runtime-enum.llvm-frag`
- `backend-ownership-compiler-shaped.llvm-frag`

Wire new cases into the matching target in `tests/Makefile` and keep each file
centered on one behavior.
