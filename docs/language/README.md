# Ari Language Docs

This folder is for people writing Ari programs.

Ari is still a compiler prototype, so each page separates three states:

- **implemented**: parsed, checked, and lowered to executable Linux x86-64 ELF
- **front-end only**: parsed or type-checked, but rejected before runtime lowering
- **planned**: reserved design direction, not implemented yet

## Reading Order

1. [Getting Started](getting-started.md)
2. [Functions](functions.md)
3. [Variables](variables.md)
4. [Literals](literals.md)
5. [Types](types.md)
6. [Operators](operators.md)
7. [Control Flow](control-flow.md)
8. [Attributes](attributes.md)
9. [Modules](modules.md)
10. [Enums And Pattern Matching](enums-patterns.md)
11. [Traits](traits.md)
12. [Prelude And Formatting](prelude.md)
13. [C FFI And Libraries](ffi.md)
14. [Memory And Ownership](memory.md)
15. [Front-End Only Syntax](front-end-only.md)

## Small Program

```ari
fn main() -> i64 {
  println("answer={}", 42)
  return 42
}
```

Compile it with:

```sh
make
./build/ari example.ari -o build/example.elf
./build/example.elf
```
