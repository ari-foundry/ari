# Symbol Mangling

Ari function symbols use a path-only v0 mangling shape inspired by Rust's
length-prefixed path encoding:

```text
_ARNv4main
_ARNv4Math3add
```

The prefix `_ARNv` means "Ari name, v0". Each source path segment is encoded as
`<byte-length><segment>`, so `Math::add` becomes `4Math3add`.

Function parameter names, parameter types, and return types are not part of the
public symbol name. Ari currently has no function overloading, so the function
path is the stable identity. Generic specializations still receive distinct
lowered function names before mangling, so their specialization identity remains
visible without encoding every call ABI detail into the symbol.

Non-identifier bytes inside synthesized names are escaped as `_uXX`, where
`XX` is the lowercase hexadecimal byte value.

Use `@export`, `@export("symbol")`, or `@no_mangle` on a non-generic Ari
function to bypass Ari mangling and emit a C symbol directly. The argument form
chooses an exact symbol; the no-argument forms use the final source path
segment. Exported symbols must be C identifiers and cannot collide with any
other emitted Ari function symbol, including a normal mangled helper symbol.

When emitting host LLVM IR for `--shared`, public Ari functions and explicit
`@export`/`@no_mangle` functions keep default visibility. Private Ari helper
functions and Ari-owned runtime helpers are emitted with hidden LLVM
visibility, so they are not intended to be part of the dynamic library ABI.
Raw `--freestanding` ELF output also records `@export`/`@no_mangle` names in
the static symbol table. The raw backend still rejects imported `extern "C"`
calls until a real native C ABI and link path exists.

This mangling is intentionally still a v0 ABI. Future overloading or ABI
versioning work may revise the symbol identity rules.
