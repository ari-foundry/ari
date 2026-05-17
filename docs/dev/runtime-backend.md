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
ari app.ari --emit-obj app.o
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

Sema caps functions and calls at 65,535 parameters/arguments. Public functions
and explicit `@export`/`@no_mangle` functions keep their requested symbol names
in LLVM IR, shared-library output, and LLVM object output.

## Prelude IO, Input, And Stops

Formatting and IO builtins lower to C stdio. `read_byte`
lowers to `getchar`, `read_line`/`input` lower to `fgets` over an internal
reusable line buffer, and assertion/stop helpers lower to `exit(1)` on failure.

The compiler keeps Ari-owned builtin source aliases and their `ari_builtin_*`
symbols in one runtime table. That table is used by `extern "ari"` validation,
and LLVM builtin calls, so root re-export forms such as
`std::write_i64` / `std::write_u64` and direct forms such as
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
carried through IR as a distinct builtin ABI.

## Known Backend Limits

The LLVM backend still intentionally rejects or does not ABI-lower:

- runtime-capacity vector values
- generic type declarations beyond simple function-call monomorphization
- C-facing tuple/vector/aggregate-enum wrappers
- raw pointer operations outside scalar and plain Ari-layout aggregate local layouts
- imported C aggregate calls beyond classifier-approved `@repr(C)` structs

## Next Backend Work

1. Add explicit generated C wrapper/header surfaces for tuple,
   fixed-capacity-vector, and aggregate-enum values that the classifier accepts.
2. Keep LLVM object output aligned with the library ABI surface as imported and
   exported aggregate wrappers grow.
3. Move compiler-known prelude stubs toward Ari source modules where possible.
