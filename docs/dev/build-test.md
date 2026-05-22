# Build And Test

## Build Modes

```sh
make release
make debug
make sanitize
make tools
make lint
make lsp
make build-lib
make check-lib
```

Outputs:

```text
build/ari
build/debug/ari
build/sanitize/ari
build/ari-lint
build/ari-lsp
```

`make` is the same as `make release`.

`make build-lib` delegates to `lib/Makefile` after building `build/ari`. It
checks the source standard library through the smoke program, writes
`build/lib/std-smoke.arimeta`, `build/lib/std-smoke.aricache`, and
`build/lib/std-smoke.ll`, verifies the metadata, and builds a small
shared-library sample as LLVM IR, object, and C header. If the configured
`LLVM_CC` exists, it also links `build/lib/libari_sample.so`.

`make check-lib` runs the same library build plus `make check-std-api`. Use it
as the narrow library-oriented check before broader `make check`.

Native C++ builds are object-based. Release objects live under
`build/obj/release`, debug objects under `build/obj/debug`, sanitizer objects
under `build/obj/sanitize`, and lint/LSP helper objects under
`build/obj/tools`. Object rules emit `.d` dependency files with `-MMD -MP`, so
changing one `.cpp` or an included header rebuilds only the affected objects
and then relinks the relevant executable.

## Test Commands

The repository-root `Makefile` owns compiler build targets. Test recipes live
in `tests/Makefile` and are included from the root makefile, so `make check`
and each focused `check-*` target still run from the repository root.

```sh
make check
make check-std-api
make check-language-docs
make check-tools
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

`make check-language-docs` keeps the user-facing language documentation usable
as a docs-only entry point. It verifies the beginner path, quick reference,
cookbook, front-end-only warnings, and test-layout navigation.

`make check-std-api` extracts the public source `std` surface from
`lib/std.arih` and `lib/std/*.arih`, then compares it with
`tests/std_api_manifest.txt`. A new source `std` API should update that manifest
with a focused coverage note, add or extend the relevant test, and update
`docs/dev/test-matrix.md` or user-facing language docs when the API is visible.

`make check-cli` runs just the compiler invocation and build-mode checks:
LLVM IR output, optional LLVM-driver linked output when `clang` is installed,
LLVM object output, and common bad CLI argument paths.

`make check-tools` builds `ari-lint` and `ari-lsp`, checks lint's human and JSON
diagnostic output, and runs a small JSON-RPC smoke test for
`textDocument/publishDiagnostics`.

The root `Makefile` and the compiler driver both look for `clang`,
versioned `clang-21` through `clang-14`, and common `/usr/lib/llvm-*` install
paths. Set `LLVM_CC=/path/to/clang` for make recipes or `ARI_LLVM_CC` for the
driver default when using a custom toolchain.

`make check-core-language` runs the focused readiness gate for the executable
core language: calls, multiple parameters, recursion, locals, assignment,
inference, scalar arithmetic/comparison/boolean/bitwise/unary operators,
explicit casts, blocks, `if`, `while`, `for` over ranges, `break`, `continue`,
and branch/block returns. It also rechecks representative stable diagnostics for
wrong calls, return errors, assignment errors, use-before-declaration, invalid
operators, invalid casts, loop-control misuse, and branch type mismatches.

`make check-operators` runs operator and literal-focused tests for integer and
float literal suffixes, integer base prefixes, string escapes, bitwise-not,
postfix `?`, Option/Result-style `??`, `f32`/`f64`/`f128` LLVM
arithmetic/comparisons, invalid `?`, invalid `??`, invalid base/escape literals,
and invalid suffix
diagnostics.

`make check-functions` runs function-focused tests for direct calls, recursion,
function pointer values and indirect calls, expression-body/block-body lambdas,
non-capturing lambdas lowered as function pointer values, capturing lambdas
lowered as closure values, parameter patterns, vector parameter ABI views, and
function error diagnostics.

`make check-generics` runs generic-focused tests for function
monomorphization, repeated specializations, arbitrary generic parameter names,
generic function pointer specialization from expected `fn(...) -> ...` types,
generic structs, generic enums, generic aliases, nested generic aggregate
substitution and identity, generic enum payload layout, ownership-qualified
generic payloads, generic trait impl syntax, generic inherent associated
functions, explicit and inferred method-level generic inherent impl and
associated calls, generic ADT surfaces, inference conflicts, uninferred
function pointer parameters, uninferred parameters, duplicate generic
parameters, aggregate payload mismatches, receiver mismatches, invalid field
access, and use-after-move through generic payloads. It also checks LLVM output
for nested aggregate enum layout.

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
names reserved by `meta fn`, exported C header prototypes and public
non-generic `@repr(C)` struct declarations with const-ref slots, by-value
non-generic `@repr(C)` struct export prototypes, opaque generic `@repr(C)`
struct typedefs for pointer-only APIs, concrete generic `@repr(C)` struct
typedefs for by-value exported instantiations, generic/non-generic fieldless
enum declarations, plus non-generic payload-bearing enum struct declarations,
shared aggregate ABI classifier diagnostics for by-value header target/size/layout guards,
one-parameter same-domain meta signatures over `token_stream`/`ast`/`type`,
empty and explicit `return input;` identity meta bodies,
attribute/expression/item/type macro domain checks, parser-stable
`ident!(...)` expression, item, and type token-tree macro invocation syntax,
expression/item/pattern-position `token_stream -> token_stream` output through
`tokens!(...)` plus empty-input, token-count, token-boundary, delimiter-wrapper,
indexed text branching, one-token-wildcard and named-span pattern branching, and
end-exclusive slice plus named token/span capture extraction and `~name`
capture splicing in token output,
expression-position identity expansion from a parsed expression input,
expression-position `ast -> ast` struct-literal/borrow/try/null-coalescing/control-flow/call/access/method-call output with input substitution and hygienic generated local/pattern bindings,
function/constant/struct/enum/trait/impl/inline-module/use item macro identity
expansion from parsed top-level declarations, item-position `ast -> ast`
declaration output, declaration input substitution, and structured declaration
input inspection through `decl!(...)`,
attribute macro function/struct/enum/trait/impl declaration rewrite and input
substitution through declaration output, attribute macro argument-token
inspection and capture splicing in token output, generated rewriting-attribute
expansion,
type-position identity expansion from a parsed type input, type-position
`type -> type` output and input substitution through `type!(...)`, pattern-position
identity expansion from a parsed pattern input, pattern-position `ast -> ast`
output and input substitution through `pattern!(...)`, and malformed pattern macro
input diagnostics.

`make check-prelude` runs prelude IO, formatting, input, assertion, source
header, and builtin macro tests. It covers function and macro assertion forms,
auto-loaded explicit `std::...` header calls, implicit Rust-like standard
aliases such as `Vec`/`Range`/`range`, explicit `mod std;` loading under
`--no-implicit-std`, `print!`/`println!`/`eprintln!`, host `read_line`/`input`,
explicit-zone `format_in!` string construction, formatted float output, and
`format!` no-implicit-zone diagnostics.

`make check-traits` runs trait-focused tests for generic trait declarations,
impl conformance, bare `self` signature inference, concrete method-call static
dispatch, prelude iterator traits, trait-object type syntax, generic trait
method bounds, method mismatch diagnostics, and ambiguous or unknown method-call
diagnostics.

`make check-match` runs enum pattern-matching tests for exhaustive arms,
payload binding, payload ignore, wildcard coverage, module-qualified cases,
LLVM branch lowering, execution, and planned expression-valued `match`
diagnostics.

`make check-modules` runs module name-resolution tests for inline modules,
file-backed modules, selected imports, glob imports, module aliases, public
re-exports, public glob re-exports, duplicate glob aliases, and private alias
visibility. It also checks package module search paths through `--module-path`
and `-I`, plus module metadata emission, source content hashes, and metadata
read-back validation, including non-`v0` metadata-version and duplicate-record
rejection. Source-snapshot module cache emission/use is covered here too, with
stale cfg, source-hash, import-resolution, AST declaration-fingerprint, and
AST declaration-payload rejection checks plus malformed duplicate source-record,
AST-summary, IR-summary, summary count/payload disagreement, and mismatched
IR lowered-function surface rejection. These cache-use tests also exercise the
structured IR body payload reader because validated sidecars now materialize the
body-shape and operand-tree sections, recheck their structural agreement, and
replay cached dependency `IrFunction` bodies so cache-use LLVM output matches a
fresh sema lowering byte-for-byte for summary-safe dependencies. Hash-valid
replay payloads whose body or type shape no longer fits the current IR model are
reported as module-cache IR replay diagnostics before backend emission. The
module-cache generics fixture also checks that generic free-function and
generated impl-method specializations are recorded in the IR sidecar, replayed
only once during cache use, and still execute correctly through the LLVM path. A
focused layout guard checks fixed-capacity `Vec[T; capacity]` type metadata by
requiring an explicit `vector-storage` IR layout descriptor, rejecting a
tampered descriptor, and comparing fresh and cache-use LLVM storage shapes.

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
promotions, nullable raw pointers, pointer casts, byte-wise and typed pointer
offsets, layout queries, raw pointer load/store/dereference helpers, aggregate
pointer field/index access, variadic function-pointer rejection, permanent
generic-extern rejection, aggregate pointer layout helpers, by-value direct
`@repr(C)` struct imports, aggregate import ABI diagnostics, and the by-value
`c_void` diagnostic. When `clang` and
`ar` are available it also builds and links a small C helper library.

`make check-functions` runs the function-focused suite: `main` rules, return
checking, recursion, void functions, argument counts/types, generic calls,
module visibility, LLVM arity behavior, and function symbol mangling.

`check-sanitize` runs the compiler under ASan/UBSan. Leak detection is disabled
in that target because LeakSanitizer can fail under this WSL/container-style
environment even when ASan/UBSan are still useful.

## Test Layout

```text
tests/cases/<feature>/ok       programs that should compile and run
tests/cases/<feature>/errors   programs that should fail with a specific diagnostic
tests/packages                 file-backed module and module-cache fixtures
tests/ffi                      C helpers for FFI/object tests
tests/tools                    editor, lint, and LSP smoke checks
examples                       small source files used by docs and smoke tests
```

## Adding A Positive Test

1. Add `tests/cases/<feature>/ok/name.ari`.
2. Add a compile/run block in the focused target in `tests/Makefile`.
3. Check the exit code or stdout.

Example expectation:

```make
$(TARGET) tests/cases/<feature>/ok/name.ari -o $(BUILD_DIR)/name.elf
$(BUILD_DIR)/name.elf; code=$$?; test $$code -eq 7
```

For glibc/prelude/FFI features, add an `--emit-llvm` check unless the local
environment guarantees an LLVM driver is installed. Prelude-specific additions
should also be wired into `check-prelude` so supported builtin lowering, host
execution where useful, and backend-boundary diagnostics stay together.

Control-flow changes should be wired into `check-control-flow`; prefer a
runtime execution check plus an LLVM IR smoke check when the lowering adds new
branch or loop shapes.

## Adding A Negative Test

1. Add `tests/cases/<feature>/errors/name.ari`.
2. Compile it in the focused target in `tests/Makefile` or in `check-errors`.
3. Fail the test if compilation succeeds.
4. `grep` for a stable diagnostic substring.

Keep diagnostics specific enough for the user to understand what rule failed,
but stable enough that tests do not need constant wording churn.

## Before Committing Code Changes

For documentation or small compiler-development fixture changes, run the
focused target that owns the changed surface first:

```sh
make check-language-docs
make check-compiler-dev-docs
make check-compiler-development
```

For larger compiler code changes, run:

```sh
make check
```

Sanitizer checks are useful for parser, semantic checker, ownership, and
codegen internals, but they are intentionally treated as a separate heavy pass
rather than part of every small documentation or model-fixture loop.
