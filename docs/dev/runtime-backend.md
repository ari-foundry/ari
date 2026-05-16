# Runtime And Backend

## Default Host Output

The default backend writes LLVM IR and invokes an LLVM compiler driver, usually
`clang`. On Linux this produces a normal dynamically linked glibc executable.

Default path:

```text
Ari IR
  -> LLVM IR
  -> clang or --llvm-cc driver
  -> glibc-linked executable or shared library
```

Useful CLI:

```sh
ari app.ari --check
ari app.ari -o app
ari app.ari --emit-llvm app.ll
ari app.ari --llvm-cc clang -o app
ari app.ari --shared -o libari_app.so
ari app.ari -L ./lib -l mylib
```

The host path does not generate C++ and never invokes `c++`. Pass `--emit-llvm`
to stop after writing IR, `--check` to stop after front-end diagnostics without
writing backend artifacts, or `--llvm-cc` to choose a different LLVM IR driver.

## Entry Flow

The host wrapper initializes runtime context, calls Ari `main`, shuts the
context down, and returns Ari `main`'s `i64` as the process exit code.

```text
host main(argc, argv)
  -> ari_context_init
  -> _ARNv4main
  -> ari_context_shutdown
```

The context stores `argc`, `argv`, and a thread-id slot. It is thread-local so a
future thread runtime can install a per-thread context.

## Calling Convention

Host output uses LLVM IR symbols directly, and Ari's external FFI surface is C
ABI only. Ari functions are emitted with Ari mangled names such as
`_ARNv4main`; the name encodes the source function path with a Rust-inspired
length-prefixed scheme. Parameter names, parameter types, and return types are
not encoded because Ari intentionally does not support argument-based
source-level function overloading.

For `--shared` host output, sema marks public functions plus explicit
`@export`/`@no_mangle` functions as ABI-visible. The LLVM backend emits private
Ari helper functions and Ari-owned runtime helper functions with hidden
visibility in that mode, while normal executable and plain `--emit-llvm` output
keep the previous default visibility.

The freestanding backend still uses the small internal integer/bool calling
convention: the first six scalar arguments use registers and later arguments use
caller-provided stack slots. Sema caps functions and calls at 65,535
parameters/arguments. Raw ELF output records `@export`/`@no_mangle` function
names in `.symtab`, but imported `extern "C"` calls remain rejected until the
native backend has a real platform C ABI and link path.

## Prelude IO, Input, And Stops

On host output, formatting and IO builtins lower to C stdio. `read_byte`
lowers to `getchar`, `read_line`/`input` lower to `fgets` over an internal
reusable line buffer, and assertion/stop helpers lower to `exit(1)` on failure.
On `--freestanding`, output lowers to direct Linux `write` syscalls,
`read_byte` lowers to a direct Linux `read` syscall on stdin, and assertion/stop
helpers use the Linux `exit` syscall. Freestanding line input is rejected until
the raw backend has runtime string storage.

The compiler keeps Ari-owned builtin source aliases and their `ari_builtin_*`
symbols in one runtime table. That table is used by `extern "ari"` validation,
LLVM builtin calls, and freestanding builtin offsets, so root re-export forms
such as `std::write_i64` / `std::write_u64` and direct forms such as
`write_i64` / `write_u64` share the same backend hook. Semantic lowering also
marks those declarations with an explicit Ari builtin ABI in IR, separate from
ordinary C extern functions, so host LLVM output never has to guess from the
symbol spelling whether a hook is foreign C or Ari runtime-owned.

`print` and `println` are special IR forms after semantic checking because the
format string must be known at compile time.

## FFI

The LLVM host backend supports:

- non-generic `extern "C" fn ...;`
- non-generic `extern fn ...;` as shorthand for C ABI
- reserved `extern "ari"` declarations for compiler/runtime builtin symbols
- explicit link names with `= "symbol"`
- `-L`, `-l`, and `--link` library options

Other ABI strings, including `extern "C++"`, are intentionally rejected. C++
interop should be exposed through C wrapper functions. Generic C extern
declarations are rejected permanently because Ari binds concrete C symbols
rather than foreign template/generic definitions. `extern "ari"` is not FFI; it
names known `ari_builtin_*` hooks supplied by the Ari runtime/backend and is
carried through IR as a distinct builtin ABI. The freestanding backend does not
link external symbols.

## Freestanding Output

Use:

```sh
ari app.ari --freestanding -o app.raw
```

This path emits raw x86-64 machine code and wraps it in a minimal Linux ELF64
file without glibc, a dynamic linker, an assembler, or an external linker.
The driver marks the file with normal executable permissions when it is written.

## Known Backend Limits

The backends still intentionally reject or do not ABI-lower:

- tuple values outside fixed-size local stack tuples
- vector values
- struct values outside fixed-size local stack structs
- generic type declarations beyond simple function-call monomorphization
- multi-word enum payloads
- raw pointer operations outside scalar and plain Ari-layout aggregate local layouts
- tuple/vector/struct/fixed-array function and FFI ABI layout

## Next Backend Work

1. Add object-file output for the freestanding/backend-native path.
2. Define non-local aggregate ABI layouts for tuples, structs, and vectors.
3. Move compiler-known prelude stubs toward Ari source modules where possible.
