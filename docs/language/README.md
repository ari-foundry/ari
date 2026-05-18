# Ari Language Docs

This folder is for people writing Ari programs.

Ari is still a compiler prototype, so each page separates three states:

- **implemented**: parsed, checked, and lowered to executable Linux x86-64 ELF
- **front-end only**: parsed or type-checked, but rejected before runtime lowering
- **planned**: reserved design direction, not implemented yet

## Reading Order

1. [Getting Started](getting-started.md)
2. [Quick Reference](quick-reference.md)
3. [Cookbook](cookbook.md)
4. [Functions](functions.md)
5. [Variables](variables.md)
6. [Literals](literals.md)
7. [Types](types.md)
8. [Operators](operators.md)
9. [Control Flow](control-flow.md)
10. [Attributes](attributes.md)
11. [Modules](modules.md)
12. [Enums And Pattern Matching](enums-patterns.md)
13. [Traits](traits.md)
14. [Prelude And Formatting](prelude.md)
15. [Standard Library](standard-library.md)
16. [C FFI And Libraries](ffi.md)
17. [Memory And Ownership](memory.md)
18. [Front-End Only Syntax](front-end-only.md)

## Docs-Only Workflow

When you need to write Ari code from documentation alone, use this path:

1. Open [Quick Reference](quick-reference.md) to choose the current syntax,
   standard-library entry point, and known gotchas.
2. Copy a nearby pattern from [Cookbook](cookbook.md).
3. Use the focused page for the feature you are changing when the short form is
   not enough.
4. Check [Front-End Only Syntax](front-end-only.md) before assuming a parsed
   feature lowers to executable code.

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
