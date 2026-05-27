# Compiler-Bound Standard Library Gaps

This page tracks the remaining standard-library completion items that cannot be
finished by editing `lib/std/` alone. They need compiler syntax, typing,
ownership, ABI, or macro-lowering work before the public library surface can be
made final.

Do not treat this page as production hardening work. It does not call for fuzz
tests, property tests, sanitizer setup, soak tests, long-running concurrency
tests, or broad CI matrices. It is a source-tree map for the current hosted
compiler and stdlib boundary.

## Current Compiler-Bound Items

| Area | Current usable surface | Why stdlib-only work stops here | Main files |
| --- | --- | --- | --- |
| `std::thread` | `thread::spawn(fn() -> i64) -> Result[JoinHandle, Error]`, `thread::spawn_raw(fn(ptr u8) -> i64, ptr u8) -> Result[JoinHandle, Error]`, nonblocking `try_join`, `ThreadScope` fixed-capacity join owners, `JoinHandle`, `JoinError`, `ThreadResult`, `Builder`, explicit `ThreadLocal[T]`. | Generic `JoinHandle[T]`, captured thread entries, borrowed scoped threads, generic return payloads, and compiler-owned `thread_local` declarations require closure environment transfer, send/share rules, result storage ownership, drop paths, borrow-scoped lifetime proofs, and TLS codegen. | `lib/std/thread.arih`, `src/std_thread_semantics.cpp`, `src/sema.cpp`, `src/llvm_codegen.cpp`, `docs/stdlib/modules/thread.md`, `tests/cases/standard-library/ok/thread/`, `tests/cases/functions/` |
| `std::fmt` | `format_in!`, `Display::format_in`, `Debug::debug_in`, fixed-arity runtime `format`/`format2`/`format3`/`format4`, matching writer helpers, direct scalar/text streaming writer helpers, `concat2`/`concat3`, and variable-count concat/template helpers. | Variadic/default-zone formatting needs compiler lowering or variadic generic support plus an allocation-zone policy. Generic per-value streaming display needs a writer-facing formatting trait plus compiler support for selecting generic trait impls whose type parameter is carried by a trait argument such as `WriteDisplay[W]`. | `lib/std/fmt.arih`, `src/prelude_macros.cpp`, `src/sema.cpp`, `docs/stdlib/modules/fmt.md`, `tests/cases/standard-library/ok/format/`, `tests/cases/standard-library/errors/format/` |
| `union by` | Parser and AST preserve `union by <selector> { arm => Type, ... }`; syntax/declaration tooling can print it. Sema validates earlier-field selector roots, nested struct-field selector segments, unique arm names, arm payload type references, and exact enum-case arm coverage for enum selectors. Enum-selector fields lower to internal enum storage, struct literals construct them with `field: arm => payload`, statically visible selector values must match the constructor arm, direct enum selector fields and selector-only nested paths such as `security.cipher_type` can be inferred from an omitted selector plus a union constructor arm, direct field matches use the same arm names (`match packet.fragment { stream(stream_payload) => ... }`), and direct assignment to the selector path, an ancestor selector field, or the `union by` payload field itself is rejected. | Remaining compiler work covers non-enum discriminant policy, selector inference through intermediate structs with unrelated required fields, public active-arm narrowing outside direct field matches, richer active-arm mutation/drop diagnostics, and stable ABI naming. | `src/parser.cpp`, `src/ast.hpp`, `src/sema.cpp`, `src/llvm_codegen.cpp`, `docs/language/generic-aggregates.md`, `tests/cases/structs/ok/union-by-constructor.ari`, `tests/cases/structs/ok/union-by-match.ari`, `tests/cases/structs/ok/union-by-selector-inferred.ari`, `tests/cases/structs/ok/union-by-nested-selector-inferred.ari`, `tests/cases/structs/errors/union-by-constructor-unknown-arm.ari`, `tests/cases/structs/errors/union-by-constructor-selector-mismatch.ari`, `tests/cases/structs/errors/union-by-selector-inference-conflict.ari`, `tests/cases/structs/errors/union-by-nested-selector-inference-conflict.ari`, `tests/cases/structs/errors/union-by-selector-assignment.ari`, `tests/cases/structs/errors/union-by-selector-root-assignment.ari`, `tests/cases/structs/errors/union-by-field-assignment.ari`, `tests/cases/compiler-development/artifact/ok/syntax-union-by-field.*`, `tests/cases/compiler-development/artifact/errors/diagnostic-union-by-*.{ari,diagnostic}` |
| Structural capability parameters | Ordinary free functions and inherent `impl` methods can write `fn f(x: has method(...) -> Type)` for one method requirement, `fn f(x: has { a() -> T, b(i64) -> U })` for grouped method requirements on one parameter, `fn f[T: has method(...) -> Type](x: T)` when the structural type must be named, or `type Alias = has method(...) -> Type;` plus `fn f[T: Alias](x: T)` when the same non-generic requirement set should be reused. The parser lowers anonymous parameters to hidden generics, sema checks concrete call-site types for every matching static method, and the body monomorphizes through normal method dispatch. Hidden capability generics do not count as visible `<T>` method type arguments. Unsupported type positions still emit a targeted diagnostic. | Remaining compiler work covers generic capability aliases, richer diagnostics that point users toward named traits, trait-method policy, and any future extension beyond method requirements. | `src/parser.cpp`, `src/ast.hpp`, `src/sema.cpp`, `src/module_ast_summary.cpp`, `docs/language/traits.md`, `docs/dev/roadmap.md`, `tests/cases/traits/ok/structural-capability-parameter.ari`, `tests/cases/traits/ok/structural-capability-multi-method.ari`, `tests/cases/traits/ok/structural-capability-impl-method.ari`, `tests/cases/traits/ok/structural-capability-generic-bound.ari`, `tests/cases/traits/ok/structural-capability-alias-bound.ari`, `tests/cases/traits/errors/structural-capability-*.ari` |

## Thread Implementation Path

The current thread ABI starts a native thread with either a plain
`fn() -> i64` entry or an explicit raw-data `fn(ptr u8) -> i64` entry. The
plain path keeps the trampoline simple: the runtime sets Ari's thread id, calls
the entry function, stores the integer status in the native thread result, and
lets `JoinHandle::join` map host failures to `JoinError`. The raw-data path
adds a small runtime-owned start packet that carries the entry function and a
caller-owned `ptr u8` payload. It is enough for low-level library bridges and
zone-backed payload tests, but it intentionally does not own or drop the
payload and does not perform send/share checking.

Generic or captured thread entries need a different ownership shape:

1. Define the user-facing API shape, likely `thread::spawn[T](entry: fn() -> T)
   -> Result[JoinHandle[T], Error>` for non-capturing entries first, with a
   later captured-closure overload only after closure send rules exist.
2. Add a compiler-owned thread packet that contains the entry function, optional
   closure environment, result storage, type-specific drop hooks, and lifecycle
   state.
3. Teach semantic analysis which captured values may cross a thread boundary.
   Plain pointer-free values are not enough for the final rule; owning values,
   borrowed values, zones, raw pointers, and shared handles need explicit
   send/share diagnostics.
4. Teach LLVM lowering to build and pass the packet to the pthread trampoline,
   write a `T` result into owned storage, and run the right drop path if spawn
   fails, the thread panics later, the handle is detached, or a join consumes
   the payload.
5. Keep `Thread` as non-owning thread information and make `JoinHandle[T]` the
   only value that owns the join/result right.
6. Add focused tests for plain generic returns, invalid captured values,
   detach-before-join, join-once behavior, and result drop behavior.

`ThreadScope` is a source-level lifecycle owner for a fixed group of join
handles. It closes a practical leak in current Ari programs by joining a group
explicitly, or best-effort in `Drop`, but it is not the borrowed scoped-thread
feature. Borrowed scoped threads remain a separate compiler feature. They
require a lexical scope API and borrow checker rules proving that borrowed data
outlives all spawned workers in that scope. Do not model borrowed scoped
threads as ordinary detached threads.

Compiler-level `thread_local` declarations are also separate from
`std::thread::ThreadLocal[T]`. They need parser/AST syntax, static TLS storage
in LLVM, initializer rules, destructor/drop policy, and target documentation.
The current explicit `ThreadLocal[T]` handle remains the portable source-level
tool until those rules exist.

## Formatting Implementation Path

The current source formatting contract is allocation-explicit:
`Display::format_in(ref mut Zone) -> String`. Runtime template helpers parse
their template at runtime, but each placeholder still calls `format_in` and
therefore builds a temporary value string.

Scalar/text streaming helpers now cover known concrete value kinds directly.
Generic per-value streaming still needs a writer-facing contract. A
conservative path is:

1. Add a new trait instead of breaking every existing `Display` impl, for
   example a future writer trait whose method writes to `std::io::Writer` and
   returns `Result[(), Error]`.
2. Provide blanket or adapter-style helpers that keep existing
   `Display::format_in` values usable while native streaming impls are added
   for common scalar/text types.
3. Teach trait resolution to select generic impls where the writer type is a
   trait argument, for example `impl[W: io::Writer] WriteDisplay[W] for i64`.
4. Rework `write_value`, `write_debug`, `write_format*`, and `write_concat*`
   to use writer-native formatting when available and fall back to
   `format_in`.
5. Add tests that a writer failure stops the formatter without losing the
   error category and that literal bytes written before a later failure are
   documented as not rolled back.

Variadic/default-zone formatting is compiler work, not just a new function:

1. Decide whether Ari gets variadic generics, a compiler-known format macro
   lowering, or both.
2. Decide the default allocation-zone policy before enabling executable
   `format!`. Today `format!` intentionally reports that no implicit allocation
   zone exists; `format_in!(ref mut zone, ...)` is the explicit-zone form.
3. Keep runtime `fmt::format` functions Result-returning because invalid
   templates are recoverable input/configuration errors.
4. Keep compiler-assisted literal format strings type-checked at compile time
   where possible.

## `union by` Implementation Path

The chosen spelling is:

```ari
fragment: union by security.cipher_type {
  stream => GenericStreamCipher,
  block => GenericBlockCipher,
  aead => GenericAEADCipher,
}
```

Positive compiler support needs these pieces in order:

1. Parse the field type into an AST node and preserve the selector and arms in
   syntax/declaration metadata. This is implemented.
2. Resolve the selector to a stable earlier field path and reject arbitrary
   expressions until lifetime and mutation rules are clear. This is implemented
   for same-struct selector paths that traverse known struct fields.
3. Validate arm payload type references and reject duplicate arm names. This is
   implemented.
4. If the selector type is an enum, require every arm name to be a real enum
   case and require every enum case to have an arm. This is implemented.
5. For enum selectors, synthesize internal enum storage for the union field.
   This is implemented and reuses the aggregate enum backend.
6. Construct enum-selector fields in struct literals with
   `field: arm => payload`. This is implemented for one payload expression per
   arm and rejects unknown arms.
7. Define selector consistency diagnostics that prove the chosen arm agrees
   with a statically visible discriminant value. This is implemented for
   same-literal selector paths.
8. Infer enum selector fields from omitted selector fields when all union
   constructors using that selector choose the same arm. This is implemented
   for direct selector fields such as `kind` and selector-only nested paths
   such as `security.cipher_type`.
9. Define reads and borrows: the active arm should be usable only after a
   discriminant check, match, or equivalent narrowing operation. Direct
   `match` over the union field is implemented for enum-selector fields.
10. Reject direct selector or payload mutation after construction. This is
   implemented for local field-assignment targets that write the selector path,
   an ancestor of that path, or the union payload field itself, such as
   `packet.security.cipher_type = block`,
   `packet.security = SecurityParameters { ... }`, or
   `packet.fragment = packet.fragment`.
11. Define layout and ABI. The payload storage is union-like, but ownership/drop
   must run only for the active arm.
12. Extend positive fixtures after broader narrowing and active-arm drop
    behavior are real. Until then, direct `match` over the field is the
    supported way to read active payloads.

## Structural Capability Parameter Path

The implemented seed shape for free functions is:

```ari
fn save(x: has serialize() -> i64) -> i64 {
  x.serialize()
}
```

This is not an `interface` feature and should not silently become dynamic
dispatch. The grouped method shape is executable too:

```ari
fn save(x: has { serialize() -> i64, add(i64) -> i64 }, amount: i64) -> i64 {
  x.serialize() + x.add(amount)
}
```

Explicit generic bounds are executable when the structural type needs a name:

```ari
fn save[T: has serialize() -> i64](x: T) -> i64 {
  x.serialize()
}
```

Non-generic capability aliases are executable for reusable function and
inherent-method generic bounds:

```ari
type Serializable = has serialize() -> i64;

fn save[T: Serializable](x: T) -> i64 {
  x.serialize()
}
```

The compiler records these aliases in AST summaries and module caches as
capability requirements rather than ordinary storage types. Using such an alias
as a parameter type is rejected; using it on structs, enums, traits, or trait
impl methods is also rejected so the initial subset stays tied to function
specialization.

The same syntax is executable on inherent associated functions and inherent
methods. The compiler treats the `has` parameter as a hidden method generic,
infers it from non-receiver arguments, and keeps visible method type arguments
stable:

```ari
struct Packet {
  value: i64,
}

impl Packet {
  fn serialize(self) -> i64 {
    self.value
  }
}

struct Runner {
  base: i64,
}

impl Runner {
  fn save(value: has serialize() -> i64) -> i64 {
    value.serialize()
  }

  fn pick[Tag](self, tag: Tag, value: has serialize() -> i64) -> i64 {
    value.serialize()
  }
}

fn main() -> i64 {
  let runner = Runner { base: 0 };
  return Runner::save(Packet { value: 1 }) +
      runner.pick<i64>(7, Packet { value: 2 });
}
```

The current implemented slice covers steps 1, 2, 4, grouped method
requirements, explicit generic structural bounds, inherent impl
method/associated-function positions, and positive/negative fixtures for free
functions and inherent impl functions:

1. Parse `has` capability parameter syntax into a hidden generic parameter with
   one or more attached method requirements.
2. Lower the requirement to a method-set constraint during semantic analysis.
3. Prefer named trait diagnostics when a named trait would be clearer or when
   multiple methods/associated types are involved.
4. Resolve `x.serialize()` through the same method lookup and monomorphization
   machinery used by named trait bounds.
5. Emit targeted diagnostics for missing methods, wrong receiver kind, wrong
   result type, ambiguous methods, and accidental dynamic-dispatch expectations.
6. Decide whether capability aliases belong in the language.

Unsupported positions such as type aliases, struct fields, extern functions,
trait methods, trait impl methods, meta functions, lambdas, and struct/enum/trait
generic bounds still use the targeted `has` diagnostic and should keep pointing
users toward named traits.

## Checks

Use focused checks for these items:

- `make check-compiler-docs` after roadmap/readiness doc edits.
- `make check-compiler-artifacts` after changing reserved parser diagnostics
  or their golden artifacts.
- `make check-functions` after closure typing or lambda lowering changes.
- `make check-generics` and `make check-traits` after generic return,
  monomorphization, or structural capability work.
- `make check-prelude` after `format_in!`, `format!`, or prelude macro
  lowering changes.
- `make check-std-api` after adding or renaming public stdlib APIs.

Run full `make check` only for broad handoffs. This page is meant to keep the
next implementation slice focused enough that the smallest owning check can
fail near the edited layer.
