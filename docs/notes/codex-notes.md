# Codex Project Notes

## Current Read

Ari is in the useful prototype phase: the front end is broader than the backend,
and the semantic checker is doing the important work of turning design ideas
into enforceable rules.

The strongest part of the project right now is that unsupported features tend
to fail explicitly instead of silently generating nonsense. Keep that habit.

## Design Taste

Prefer:

- explicit ownership over hidden runtime policy
- traits over classes or interfaces
- tagged ADTs plus pattern matching over inheritance
- module paths with clear visibility
- compiler-known prelude first, source `std` declarations as the stable path
- capability-style allocation over a global heap primitive

Avoid:

- adding syntax without a semantic rejection path
- making the backend understand source-level parser details
- mixing host/glibc behavior into the raw freestanding backend
- turning `Vec[T]` into magic heap allocation before allocator rules exist

## Next Work I Would Pick

The best next implementation step is probably fixed-size tuple lowering.

Why:

- tuple syntax and typed IR already exist
- it teaches aggregate layout without heap allocation
- it helps future multi-payload enum layout
- it gives pattern matching and function returns a more realistic target

Second choice: source-level exports for `--shared` libraries.

Why:

- host shared-library output exists now
- Ari functions now export `_ARNv*` path-only symbols; parameter and return
  types are intentionally excluded until Ari has an overloading/export policy
- explicit export naming will matter before the ABI becomes sticky
- FFI is intentionally C ABI only now; C++ should come through wrapper functions

## Keep An Eye On

- `src/sema.cpp` is doing a lot. If it keeps growing, split name resolution,
  type checking, and ownership checking into separate files.
- The Makefile test list is explicit and clear, but it will get long. A small
  test runner may become worth it soon.
- Formatting is intentionally simple. LLVM host output can print through stdio,
  while freestanding still needs data-section work for richer strings.
- Debug builds are useful, but sanitizer runs are better for parser/sema churn.
