# Ari Language Docs

This folder is for people writing Ari programs.

Ari is still a compiler prototype, so each page separates three states:

- **implemented**: parsed, checked, and lowered to executable Linux x86-64 ELF
- **front-end only**: parsed or type-checked, but rejected before runtime lowering
- **planned**: reserved design direction, not implemented yet

## Reading Order

1. [Getting Started](getting-started.md)
2. [Language Tour](language-tour.md)
3. [Quick Reference](quick-reference.md)
4. [Cookbook](cookbook.md)
5. [Feature Status](feature-status.md)
6. [Feature Crosswalk](feature-crosswalk.md)
7. [Examples And Tests](examples-and-tests.md)
8. [Functions](functions.md)
9. [Variables](variables.md)
10. [Literals](literals.md)
11. [Types](types.md)
12. [Generic Aggregates](generic-aggregates.md)
13. [Operators](operators.md)
14. [Control Flow](control-flow.md)
15. [Attributes](attributes.md)
16. [Modules](modules.md)
17. [Enums And Pattern Matching](enums-patterns.md)
18. [Traits](traits.md)
19. [Prelude And Formatting](prelude.md)
20. [Standard Library](standard-library.md)
21. [C FFI And Libraries](ffi.md)
22. [Memory And Ownership](memory.md)
23. [Front-End Only Syntax](front-end-only.md)

## First-Hour Path

If you are new to Ari, start with a tiny executable before reading every
feature page:

1. Build `build/ari` with `make`.
2. Compile the first program from [Getting Started](getting-started.md).
3. Run the cross-feature [Language Tour](language-tour.md) example.
4. Run `./build/ari your-file.ari --check` after each small edit.
5. Keep [Quick Reference](quick-reference.md) open for current spellings.
6. Check [Feature Status](feature-status.md) when you are unsure whether a
   feature is executable, partial, front-end only, or planned.
7. Use [Feature Crosswalk](feature-crosswalk.md) to connect a feature to the
   exact docs, example, tests, and small check that protect it.
8. Use [Examples And Tests](examples-and-tests.md) to find runnable examples
   and the feature test family that protects a behavior.
9. Copy a complete pattern from [Cookbook](cookbook.md) when you need modules,
   ownership, zones, iterators, FFI, or tests.

That path is intentionally executable-first. Ari still has front-end-only and
planned surfaces, so always check the focused page before assuming parsed
syntax already lowers to LLVM.

## Docs-Only Workflow

When you need to write Ari code from documentation alone, use this path:

1. Open [Quick Reference](quick-reference.md) to choose the current syntax,
   standard-library entry point, and known gotchas.
2. Read [Language Tour](language-tour.md) when you need the main executable
   features in one narrative.
3. Check [Feature Status](feature-status.md) to confirm the feature state and
   matching test family.
4. Use [Feature Crosswalk](feature-crosswalk.md) to find the focused docs,
   example, tests, and small check for that feature.
5. Use [Examples And Tests](examples-and-tests.md) to find a runnable example
   or a focused compiler test.
6. Copy a nearby pattern from [Cookbook](cookbook.md).
7. Use the focused page for the feature you are changing when the short form is
   not enough.
8. Check [Front-End Only Syntax](front-end-only.md) before assuming a parsed
   feature lowers to executable code.
9. Use [Getting Started](getting-started.md#common-first-errors) when a small
   program fails in a way that looks like syntax, ownership, or module setup.

## Small Program

```ari
fn main() -> i64 {
  println("answer={}", 42);
  return 42;
}
```

Compile it with:

```sh
make
./build/ari example.ari -o build/example.elf
./build/example.elf
```
