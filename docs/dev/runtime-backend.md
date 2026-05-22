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

The context stores `argc`, `argv`, and a thread-id slot. `ari_context_init`
sets the main thread id to `0`; the `std::thread` trampoline installs a
nonzero id before a spawned Ari thread calls source code. The slot is
thread-local so thread runtime work keeps per-thread context state without
changing the public `std::context::thread_id()` API.

`std::thread::available_parallelism()` is also runtime-backed. On the hosted
Linux path it calls `sysconf(_SC_NPROCESSORS_ONLN)` and clamps failure to `1`
so callers can use it as a loop bound without special casing unavailable host
information.

Hosted output still relies on the platform CRT for the process `_start`,
startup objects, dynamic linker setup, and low-level compiler runtime support.
The detailed ownership plan for `_start`, `crt0`, TLS setup, init/fini arrays,
stack protector hooks, unwinding, backtraces, compiler-rt/libgcc-style helper
routines, atomic fallbacks, and memory builtins lives in
[Runtime Support Roadmap](runtime-support.md).

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
in LLVM IR, shared-library output, and LLVM object output. Object output is a
library artifact: it is produced as PIC, uses the same hidden/private visibility
rules as `--shared`, can be emitted together with a C header, and preserves
module-cache IR replay exactly enough for cached dependency bodies to appear in
the object symbol table. Object fixtures cover scalar exports, generated
aggregate C-header wrappers, cached generic dependency replay, and direct
by-value aggregate exports that also reference unresolved `extern "C"` helpers.

## Prelude IO, Input, And Stops

Formatting and IO builtins lower to C stdio. `read_byte`
lowers to `getchar`, `read_line`/`input` lower to `fgets` over an internal
reusable line buffer, and assertion/stop helpers lower to `exit(1)` on failure.

Environment and process-local path hooks stay in the compiler-owned runtime
table too. `std::env` lowers environment variables through `getenv`, `setenv`,
and `unsetenv`; current-directory helpers lower through `getcwd`/`chdir`; and
the current Linux executable-path helper reads `/proc/self/exe` with
`readlink` into a fixed runtime buffer. `std::fs::try_read_link` uses the same
host `readlink` ABI but copies the stored symbolic-link target into the
caller's zone before returning it. No-follow filesystem metadata uses hosted
`lstat` so `std::fs::try_symlink_metadata` can report the link object rather
than the link target.

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

- bare root `Vec[T]` non-local ownership values
- public/non-local ABI exposure for some concrete generic aggregate values
- raw pointer operations outside scalar and plain Ari-layout aggregate local layouts
- imported C aggregate calls beyond classifier-approved `@repr(C)` structs

## Future Direction

The next backend work should follow the staged runtime support plan. Hosted
Linux can continue leaning on the LLVM driver and platform CRT, while
freestanding/no-libc work should start with `_start`, `crt0`, TLS, panic, and
compiler-runtime helper policy before exposing more public library APIs.
