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

Useful options:

```sh
--emit-llvm path    write generated LLVM IR and stop
--module-path path  add a file-backed module search path
-I path             same as --module-path
--llvm-cc compiler  choose the LLVM IR compiler driver, usually clang
--target triple     choose the LLVM target triple and C ABI alias layout
-L path             add a library search path
-l name             link a library
--link name         same as -l name
--shared            build a shared library instead of an executable
--freestanding      use the raw direct-syscall ELF backend
--test              build a generated test runner for @test functions
--no-implicit-std   require explicit mod std; instead of auto-loading lib/std.arih
--cfg-feature name  enable feature("name") inside @cfg(...)
```

## Runtime Model

The default generated binary uses LLVM, the host dynamic linker, and glibc. The
freestanding backend is still useful for inspecting direct machine-code output:

```sh
./build/ari examples/count.ari --freestanding -o build/count.raw
```
