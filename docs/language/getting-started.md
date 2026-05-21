# Getting Started

This page is the first-hour path for writing and running Ari code. It assumes
you are in the repository root and want a small program that goes through the
same LLVM-backed compiler path as the tests.

## Mental Model

Ari is a small systems-language compiler prototype. The current compiler is
written in C++17, but Ari programs are not translated to C++; the compiler
emits LLVM IR and asks an LLVM driver such as `clang` to produce Linux/glibc
artifacts.

Keep these rules in mind while reading the examples:

- `fn main() -> i64` is the executable entry point.
- The returned `i64` becomes the process exit code.
- `let` is immutable and `var` is mutable.
- Use explicit casts with `as`; Ari does not silently change integer widths.
- Allocation is explicit. Zone-backed library handles need a visible `Zone`.
- The source standard library is loaded as `std` by default.
- There is no `class` or `interface` keyword. Use `struct`, `enum`, and
  `trait`.

## Build The Compiler

```sh
make
```

This creates:

```text
build/ari
```

## Write A First Program

Create `hello.ari`:

```ari
fn main() -> i64 {
  println("hello from Ari");
  return 0;
}
```

Compile and run it:

```sh
./build/ari hello.ari -o build/hello.elf
./build/hello.elf
echo $?
```

The compiler writes a Linux x86-64 executable ELF file. It currently prints a
short line when the output file is written:

```text
wrote build/hello.elf (...)
```

The generated executable enters `@ari_entry`, initializes Ari runtime context,
calls `ari::main`, and returns the source `main` value as the process status.

## Use The Fast Check Loop

Use `--check` while editing syntax, types, modules, or ownership. It runs the
front end without producing LLVM or linking:

```sh
./build/ari hello.ari --check
```

Use `--emit-llvm` when you want to inspect the backend input without linking:

```sh
./build/ari hello.ari --emit-llvm build/hello.ll
```

Use a full executable only when you need runtime behavior:

```sh
./build/ari hello.ari -o build/hello.elf
./build/hello.elf
```

## Add Values And Control Flow

```ari
fn score(value: i64) -> i64 {
  if value > 10 {
    return value;
  }
  return 10;
}

fn main() -> i64 {
  var total = 0;

  for value in 0..5 {
    total += score(value);
  }

  println("total={total}");
  return total;
}
```

Statement forms such as `let`, `var`, assignment, calls used as statements,
`return`, `break`, `continue`, `drop`, and `forget` should use `;`. A final
expression without `;` produces the value of a function, block, `if`
expression, `match` arm, or labeled block.

## Split Code Into A Module

For a small file-backed module:

```text
app.ari
math.ari
```

`app.ari`:

```ari
mod math;

fn main() -> i64 {
  return math::double(21);
}
```

`math.ari`:

```ari
pub fn double(value: i64) -> i64 {
  return value + value;
}
```

Compile from the directory that contains both files:

```sh
./build/ari app.ari -o build/app.elf
```

Use `-I path` or `--module-path path` when child modules live outside the input
file's directory.

## CLI Reference

```sh
ari <input.ari> [-o output]
```

If `-o` is omitted, the output path is `a.out`.

Useful options:

```sh
--check             check source diagnostics only; do not emit or link
--emit-llvm path    write generated LLVM IR and stop
--emit-obj path     write an LLVM-driver object file and stop
--module-path path  add a file-backed module search path
-I path             same as --module-path
--llvm-cc compiler  choose the LLVM IR compiler driver, usually clang
--target triple     choose the LLVM target triple and C ABI alias layout
--target-info       print resolved target facts and active target(...) names
-L path             add a library search path
-l name             link a library
--link name         same as -l name
--shared            build a shared library instead of an executable
--test              build a generated test runner for @test functions
--no-implicit-std   require explicit mod std; instead of auto-loading lib/std.arih
--cfg-feature name  enable feature("name") inside @cfg(...)
--list-artifacts    list compiler-development artifact producers
--explain-artifact option
                    explain one compiler artifact owner, first check, and purpose
```

Use `ari --target-info` or `ari --target x86_64-pc-linux-gnu --target-info`
when checking which `target("...")` predicates and ABI facts the compiler sees.
Use `ari --explain-artifact --emit-tokens` when choosing the smallest
compiler artifact that proves a frontend change.

## Runtime Artifacts

The generated binary uses LLVM, the host dynamic linker, and glibc. Use
`--emit-obj` when you need a relocatable object for linker or library
experiments:

```sh
./build/ari hello.ari --emit-obj build/hello.o
```

Use `--shared` when exporting public functions from a shared library:

```ari
@export("ari_add")
pub fn add(left: i64, right: i64) -> i64 {
  return left + right;
}
```

```sh
./build/ari library.ari --shared -o build/libari_sample.so
```

## Common First Errors

- Missing `;` after a statement, or an accidental `;` after a final value
  expression.
- A `main` function that does not return `i64`.
- An integer literal that needs a suffix or explicit `as` cast.
- Assigning to a `let` binding instead of a `var` binding.
- Assigning through a struct field that was not declared `mut`.
- Moving, dropping, or assigning through a value while it is borrowed.
- Letting an `own` value reach scope exit without `drop`, `return`, `move`, or
  `forget`.
- Trying to use a feature documented as front-end-only in executable code.

## Next Reading

Read [Language Tour](language-tour.md) for a cross-feature walkthrough,
[Quick Reference](quick-reference.md) for the compact language sheet, then copy
patterns from [Cookbook](cookbook.md). Use focused pages when you need a rule
in detail: [Functions](functions.md), [Variables](variables.md),
[Types](types.md), [Control Flow](control-flow.md),
[Enums And Pattern Matching](enums-patterns.md), [Traits](traits.md),
[Prelude And Formatting](prelude.md), [Standard Library](standard-library.md),
[Memory And Ownership](memory.md), and [C FFI And Libraries](ffi.md).
