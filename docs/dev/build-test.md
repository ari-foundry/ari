# Build And Test

## Build Modes

```sh
make release
make debug
make sanitize
```

Outputs:

```text
build/ari
build/debug/ari
build/sanitize/ari
```

`make` is the same as `make release`.

## Test Commands

The repository-root `Makefile` owns compiler build targets. Test recipes live
in `tests/Makefile` and are included from the root makefile, so `make check`
and each focused `check-*` target still run from the repository root.

```sh
make check
make check-debug
make check-sanitize
make check-cli
make check-operators
make check-generics
make check-meta
make check-traits
make check-structs
make check-variables
make check-functions
```

`make check` builds the compiler, runs negative tests, compiles positive tests,
executes generated ELF files, and checks exit codes or stdout where needed.

`make check-cli` runs just the compiler invocation and build-mode checks:
LLVM IR output, optional LLVM-driver linked output when `clang` is installed,
freestanding output, and common bad CLI argument paths.

The root `Makefile` and the compiler driver both look for `clang`,
versioned `clang-21` through `clang-14`, and common `/usr/lib/llvm-*` install
paths. Set `LLVM_CC=/path/to/clang` for make recipes or `ARI_LLVM_CC` for the
driver default when using a custom toolchain.

`make check-operators` runs operator and literal-focused tests for integer and
float literal suffixes, integer base prefixes, string escapes, bitwise-not,
postfix `?`, Maybe/Result-style `??`, `f32`/`f64`/`f128` LLVM
arithmetic/comparisons, invalid `?`, invalid `??`, invalid base/escape literals,
and invalid suffix
diagnostics.

`make check-generics` runs generic-focused tests for function
monomorphization, repeated specializations, arbitrary generic parameter names,
generic function pointer specialization from expected `fn(...) -> ...` types,
generic structs, generic trait impl syntax, generic inherent associated
functions, explicit and inferred method-level generic inherent impl and
associated calls, generic ADT surfaces, inference conflicts, uninferred
function pointer parameters, uninferred parameters, and duplicate generic
parameters.

`make check-traits` covers concrete trait impls, trait-bound static dispatch,
generic trait impl method and associated-function specialization, generic trait
method type-argument inference, method-level bounds on generic trait methods,
generic impl bounds, `dyn Trait[...]` type surface validation, concrete and
generic-impl-specialized LLVM dyn dispatch, dyn object-safety diagnostics for
generic trait methods, and trait impl coherence diagnostics.

`make check-meta` runs attribute and meta-syntax tests for built-in attributes,
`@repr(C)` layout guard diagnostics, `@cfg(true/false)` declaration pruning,
boolean/target/feature cfg predicate parsing, command-line cfg feature flags,
deprecated use warnings, `@test` runner generation, user-defined attribute
names reserved by `meta fn`, meta signatures over `token_stream`/`ast`, and
planned Rust-style `name!(...)` macro invocation diagnostics.

`make check-prelude` runs prelude IO, formatting, input, assertion, source
header, and builtin macro tests. It covers function and macro assertion forms,
auto-loaded explicit `std::...` header calls, implicit Rust-like standard
aliases such as `Vec`/`Range`/`range`, explicit `mod std;` loading under
`--no-implicit-std`, `print!`/`println!`, host `read_line`/`input`,
freestanding byte IO, and planned `format!` string allocation diagnostics.

`make check-traits` runs trait-focused tests for generic trait declarations,
impl conformance, bare `self` signature inference, concrete method-call static
dispatch, prelude iterator traits, trait-object type syntax, generic trait
method bounds, method mismatch diagnostics, and ambiguous or unknown method-call
diagnostics.

`make check-match` runs enum pattern-matching tests for exhaustive arms,
payload binding, payload ignore, wildcard coverage, module-qualified cases,
LLVM branch lowering, freestanding execution, and planned expression-valued
`match` diagnostics.

`make check-modules` runs module name-resolution tests for inline modules,
file-backed modules, selected imports, glob imports, module aliases, public
re-exports, public glob re-exports, duplicate glob aliases, and private alias
visibility. It also checks package module search paths through `--module-path`
and `-I`, plus module metadata emission, source content hashes, and metadata
read-back validation, including old metadata-version and duplicate-record
rejection. Source-snapshot module cache emission/use is covered here too, with
stale cfg, source-hash, and import-resolution rejection checks plus malformed
duplicate source-record rejection.

`make check-structs` runs struct and ADT-focused tests for struct declarations,
generic struct field resolution, named struct literals, tuple-struct
constructors, inherent associated constructors such as `T::new(...)`, local
field/index access, field `mut` assignment checks, module visibility, enum ADT
pattern matching, and planned aggregate destructuring syntax.

`make check-variables` runs variable-focused tests for immutable and mutable
bindings, type inference, assignment, local shadowing rejection, scalar width
bindings, host string/float bindings, and pattern bindings inside `match` arms.

`make check-ffi` runs C FFI tests for libc declarations, explicit C link names,
module externs, x86-64 C ABI type aliases, `ptr c_char` string arguments,
`c_void` returns, `ref mut` pointer parameters, C varargs with default
promotions, variadic function-pointer rejection, permanent generic-extern
rejection, and the by-value `c_void` diagnostic. When `clang` and `ar` are
available it also builds and links a small C helper library.

`make check-functions` runs the function-focused suite: `main` rules, return
checking, recursion, void functions, argument counts/types, generic calls,
module visibility, host/freestanding arity behavior, and function symbol
mangling.

`check-sanitize` runs the compiler under ASan/UBSan. Leak detection is disabled
in that target because LeakSanitizer can fail under this WSL/container-style
environment even when ASan/UBSan are still useful.

## Test Layout

```text
tests/ok       programs that should compile and run
tests/errors   programs that should fail with a specific diagnostic
examples       small source files used by docs and smoke tests
```

## Adding A Positive Test

1. Add `tests/ok/name.ari`.
2. Add a compile/run block in the focused target in `tests/Makefile`.
3. Check the exit code or stdout.

Example expectation:

```make
$(TARGET) --freestanding tests/ok/name.ari -o $(BUILD_DIR)/name.elf
$(BUILD_DIR)/name.elf; code=$$?; test $$code -eq 7
```

For glibc/prelude/FFI features, add an `--emit-llvm` check unless the local
environment guarantees an LLVM driver is installed. Prelude-specific additions
should also be wired into `check-prelude` so supported builtin lowering, host
execution where useful, and backend-boundary diagnostics stay together.

Control-flow changes should be wired into `check-control-flow`; prefer a
freestanding execution check plus an LLVM IR smoke check when the lowering adds
new branch or loop shapes.

## Adding A Negative Test

1. Add `tests/errors/name.ari`.
2. Compile it in the focused target in `tests/Makefile` or in `check-errors`.
3. Fail the test if compilation succeeds.
4. `grep` for a stable diagnostic substring.

Keep diagnostics specific enough for the user to understand what rule failed,
but stable enough that tests do not need constant wording churn.

## Before Committing Code Changes

Run:

```sh
make check
```

For semantic, ownership, parser, or codegen work, also run:

```sh
make check-sanitize
```
