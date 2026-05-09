# Ari

Ari is a small systems-language compiler prototype written in C++17.

It explores a C/C++ replacement shape with value types, explicit ownership,
traits, modules, ADT enums, pattern matching, local inference, C FFI, and a
compiler-known prelude. The compiler implementation is C++17, but Ari programs
are no longer generated as C++: host output goes through LLVM IR and an LLVM
driver such as `clang`. The raw Linux x86-64 ELF backend is still available
with `--freestanding`.

## Quick Start

Build the compiler:

```sh
make
```

Compile and run an Ari program:

```sh
./build/ari examples/count.ari -o build/count.elf
./build/count.elf
echo $?
```

Run the full test suite:

```sh
make check
```

## Build Modes

```sh
make release
make debug
make sanitize
make check
make check-debug
make check-sanitize
```

## Tiny Example

```ari
fn main() -> i64 {
  println("count={}", 5)
  return 5
}
```

## What Works Now

- optional `--freestanding` Linux x86-64 ELF output without an assembler or linker
- glibc-backed host output by default through LLVM IR
- LLVM IR output with `--emit-llvm`
- functions, locals, assignment, `if`, `while`, `break`, `continue`
- `for value in range(start, end)` loops
- integer and bool scalar codegen
- `i8/i16/i32/i64` and `u8/u16/u32/u64`
- explicit integer casts with `as`
- modulo, bit operations, and short-circuit boolean logic
- `own`, `ref`, `ref mut`, `drop`, move/drop checks, and temporary borrow checks
- ADT enums as one-word tagged unions
- exhaustive `match` statements over enums
- inline and file-backed `mod`, `pub`, `use`, and `A::B` paths
- compiler-known prelude IO and formatting `print` / `println`
- compiler-known prelude trait names for `Iterable` and `Iterator`
- `extern "C"` FFI declarations
- explicit external C link names with `= "symbol"`
- `--shared` library output
- tuple and `Vec[T]` syntax in typed IR, with runtime lowering still blocked

## Docs

- [docs/README.md](docs/README.md): documentation index
- [docs/language/README.md](docs/language/README.md): how to write Ari
- [docs/dev/README.md](docs/dev/README.md): how the compiler is built
- [docs/notes/README.md](docs/notes/README.md): design notes and handoff notes
- [AGENTS.md](AGENTS.md): root project orientation for coding agents

Legacy entry points still exist:

- [docs/SYNTAX.md](docs/SYNTAX.md)
- [docs/PROJECT.md](docs/PROJECT.md)
- [docs/MEMORY.md](docs/MEMORY.md)

## Runtime Notes

Generated Ari executables use the glibc-backed LLVM path by default. The
compiler writes temporary LLVM IR, invokes `clang` or the driver selected with
`--llvm-cc`, and links a normal dynamic Linux executable. Use `--freestanding`
when you want the direct-syscall raw ELF backend.

Useful host options:

```sh
./build/ari app.ari -o app
./build/ari app.ari --emit-llvm app.ll
./build/ari lib.ari --shared -o libari_app.so
./build/ari app.ari -L ./lib -l mylib
./build/ari app.ari --freestanding -o app.raw
```
