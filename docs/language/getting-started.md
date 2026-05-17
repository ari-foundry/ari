# Getting Started

## Build The Compiler

```sh
make
```

This creates:

```text
build/ari
```

## Compile A Program

```sh
./build/ari examples/count.ari -o build/count.elf
```

The compiler writes a Linux x86-64 executable ELF file. It currently prints a
short line when the output file is written:

```text
wrote build/count.elf (...)
```

## Run The Program

```sh
./build/count.elf
echo $?
```

The generated executable enters `@ari_entry`, initializes Ari runtime context,
calls `ari::main`, and uses the source `main` return value as the process exit
code.

## CLI

```sh
ari <input.ari> [-o output]
```

If `-o` is omitted, the output path is `a.out`. By default Ari emits LLVM IR and
invokes an LLVM driver, usually `clang`, so the output is a normal glibc-linked
executable on Linux.

Use `--check` to run parsing, module loading, and semantic checks without
writing LLVM IR, linking, or requiring an executable `main`. This is the
front-end-only mode intended for editor tooling and quick diagnostics.

Useful options:

```sh
--check             check source diagnostics only; do not emit or link
--emit-llvm path    write generated LLVM IR and stop
--emit-obj path     write an LLVM-driver object file and stop
--module-path path  add a file-backed module search path
-I path             same as --module-path
--llvm-cc compiler  choose the LLVM IR compiler driver, usually clang
--target triple     choose the LLVM target triple and C ABI alias layout
-L path             add a library search path
-l name             link a library
--link name         same as -l name
--shared            build a shared library instead of an executable
--test              build a generated test runner for @test functions
--no-implicit-std   require explicit mod std; instead of auto-loading lib/std.arih
--cfg-feature name  enable feature("name") inside @cfg(...)
```

## Runtime Model

The generated binary uses LLVM, the host dynamic linker, and glibc. Use
`--emit-llvm` to inspect the IR before linking, or `--emit-obj` when you need a
relocatable object for library/linker experiments:

```sh
./build/ari examples/count.ari --emit-llvm build/count.ll
./build/ari examples/count.ari --emit-obj build/count.o
```
