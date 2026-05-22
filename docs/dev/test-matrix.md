# Feature Test Matrix

This is the project checklist for testing one language feature at a time from
multiple directions. Every feature should eventually have positive, negative,
cross-feature, and backend tests.

## Per-Feature Checklist

For each feature:

- [ ] syntax parses the intended surface
- [ ] minimal positive program compiles and runs
- [ ] result is checked by exit code or stdout
- [ ] invalid syntax or semantic misuse has a negative test
- [ ] LLVM/glibc backend behavior is covered
- [ ] interaction with modules or visibility is covered when names are involved
- [ ] interaction with ownership/borrowing is covered when values move or borrow
- [ ] generated LLVM IR is inspected when ABI or linking is involved
- [ ] docs mention the behavior and known limits

## ABI Coverage Note

Non-local aggregate ABI diagnostics are covered by C-header fixtures. The
shared classifier recognizes value tuples, fixed arrays, structs,
fixed-capacity vector storage, and aggregate-layout enums, then distinguishes
direct 64-bit Unix values from indirect or unsupported target/layout cases.
Header rendering is narrower than classification: public `@repr(C)` structs,
public non-generic payload-bearing `@repr(C)` enum structs without `own`
payload slots, and generated fixed-array, tuple, fixed-capacity-vector, and
aggregate-enum wrapper typedefs are exposed for direct aggregate ABI values.
Oversized,
indirect, or target-unsupported aggregate header surfaces remain rejected.

## Standard Library Coverage Note

Public source `std` declarations are tracked in `tests/std_api_manifest.txt`
and checked by `make check-std-api`; production-readiness docs are checked by
`make check-stdlib-docs`. Standard library behavior is split by test prefix
under `tests/cases/standard-library/`: `std-<module>-<feature>.ari` for source
module APIs, `prelude-<feature>.ari` for implicit root aliases and macros, and
`std-library-smoke.ari` for cross-library integration. Zone and pointer
provenance hooks live under `tests/cases/memory/`. See
[Library Testing](library-testing.md) for the full policy.

## Compiler And Bootstrap Documentation Coverage Note

Compiler development is tracked before bootstrapping work starts. The general
language-docs entry path is checked by `make check-language-docs`, the general
compiler roadmap is checked by `make check-compiler-dev-docs`, the later
bootstrap start gate is checked by `make check-bootstrap-docs`, and the first
compiler-shaped Ari fixtures are
checked by `make check-bootstrap-readiness`. The docs target runs
`tests/check_language_docs.py`,
`tests/check_compiler_development_docs.py` and
`tests/check_bootstrap_readiness_docs.py` and verifies that
[Getting Started](../language/getting-started.md),
[Quick Reference](../language/quick-reference.md),
[Cookbook](../language/cookbook.md),
[Compiler Development Roadmap](compiler-development-roadmap.md),
[Compiler Implementation Playbook](compiler-implementation-playbook.md),
[Compiler Next Slices](compiler-next-slices.md),
[Compiler Change Checklist](compiler-change-checklist.md),
[Compiler Maturity Gates](compiler-maturity-gates.md),
[Compiler Pass Contracts](compiler-pass-contracts.md),
[Compiler Project Model](compiler-project-model.md),
[Compiler Source And Diagnostics](compiler-source-diagnostics.md),
[Compiler Artifact Testing](compiler-artifact-testing.md),
[Production Compiler Design](production-compiler-design.md),
[Compiler Bootstrap Fixture Plan](bootstrap-fixture-plan.md),
[Bootstrap Readiness](bootstrap-readiness.md),
[Self-Host Roadmap](self-host-roadmap.md), and the docs indexes stay linked and
keep the production-language contract, readiness scorecard, start gate, first
implementation slices, fixture groups, roadmap, estimate, and test-plan
sections.

## Feature Plan

| Feature | Positive Directions | Negative Directions | Backend Directions | Status |
| --- | --- | --- | --- | --- |
| Standard library | source `std` root and child modules, prelude aliases, `Option`/`Result` methods, `Slice` safe access/search/copy/subslice/split/chunk/window/compare helpers plus `dedup_by`/`dedup_by_key`/`stable_partition` receiver wrappers, `Box`, `String` byte/try/search/split/chunk/window/join/typed text-kind/ASCII case/UTF-8 scalar/trim-view/trim-copy/whole-parse/prefix-parse helpers, `ascii` byte classification/case-compare/case-search/slice/prefix-parser helpers, `parse` whole-input integer/bool/float helpers, `encoding` ASCII/UTF-8/UTF-16 validation plus UTF-8 scalar decode/encode and hex/base64 codecs, `path` lexical separator/PathBytes/component/join/normalization helpers, `bits` mask/rotate/power/alignment/zero-run/one-run scan helpers, source `Vec` safe access/growth/direct borrowed sequence/iterator helpers plus owned `dedup_by`/`dedup_by_key`/`stable_partition`, source `std::collections::Set` linear membership/access/optional access/replace/reserve/iteration/copy helpers, real `HashMap`/`HashSet` hash table and live-bucket iterator helpers, real `TreeMap`/`TreeSet` red-black tree and sorted iterator helpers, `Deque`/`RingBuffer`/`LinkedList` queue and list helpers, `BinaryHeap`/`PriorityQueue` priority helpers, `iter`, `cmp`, `convert`, `fmt`, `hash`, `algo` stable partition and comparator/key dedup helpers, `math` sign predicate/i64/checked division/remainder and saturating division/division-rounding helpers, IO/input/context/test/log/target/env/process/thread/sync/time/fs/net/c including `Reader`/`Writer`/`Seek`, `Cursor`, `BufReader`, `BufWriter`, `stdin`/`stdout`/`stderr`, `read_exact_result`, `copy_result`, `write_all_result`, `flush_result`, `read_exact`, `write_all`, `flush`, executable `test::Report` aggregation, scratch zones, temp path helpers, snapshot/golden byte comparison, and minimal benchmark timers, level-prefixed `std::log` stderr diagnostics, compiler-known target triple/arch/OS/libc/object/debug/errno/syscall facts, C ABI string/errno/dynamic loading helpers, current-process environment variables, current directory, executable path, process id/uid/gid/exit/abort/fork_result/wait_result/raw fork/raw wait, module-level process spawn/status/exit_status/output_in/exec wrappers, thread spawn/join/yield/runtime ids, concrete atomic i64 load/store/swap/fetch-add/compare-exchange plus source mutex/once, monotonic/wall-clock time, deadlines/timeouts, UTC calendar conversion, sleep, file existence/permissions/open/read/read_result/try_read/write/append/read_to_string/read_to_string_result/try_read_to_string/position/seek/close/remove, network IP/socket-address values, IPv4 DNS lookup and host-port endpoint resolution, module-level TCP listen/connect wrappers, IPv4 TCP bind/connect/accept/local-port/local-address/peer-address helpers, module-level UDP bind wrapper, IPv4 UDP local-address and single-byte datagrams, module-level Unix listen/connect wrappers, Unix stream sockets, socket nonblocking, `std::time::Duration` timeout, raw millisecond timeout, and shutdown helpers, restricted-host socket error bridging, explicit zone allocation, `std_api_manifest` guard, and a cross-library smoke program | missing or partial `std`, invalid builtin declarations, wrong helper arity/type, invalid ownership and borrow use, wrong zone provenance, same-zone growth violations, unsupported smart-pointer names, invalid formatting inputs, future per-test capture/structured-logging/backtrace gaps | source function monomorphization, reserved `extern "ari"` runtime hooks, LLVM layout and pointer helpers, IO trait helper monomorphization, test helper generic monomorphization, log helper stderr output, target constants, environment/path, process id/uid/gid/exit/abort/fork/wait/spawn/status/output, thread pthread spawn/join/yield hooks, LLVM atomic load/store/RMW/CAS hooks, time clock/sleep hooks, filesystem access/descriptor/seek hooks, socket/listen/connect/accept/getsockname/getpeername/getaddrinfo/sendto/recvfrom/setsockopt/getsockopt/shutdown hooks, zone allocation calls, Drop lowering for handles, iterator lowering, source math/ascii/parse/encoding/path byte/slice/bits/hash/test/log/target/env/collections/process/thread/sync/time/fs/net/c helper symbols, executable and optional LLVM-driver checks | good first pass |
| CLI/build modes | explicit output, `--check`, `--emit-llvm`, `--module-path`/`-I`, `--shared` | incompatible flags, removed C++ options, missing input/output/compiler/search-path arg, unexpected arg | check-only front-end diagnostics, LLVM IR, optional LLVM link, and LLVM object including aggregate export/extern C relocation fixtures | good first pass |
| Functions | `main`, calls, named params, unit-returning functions, tuple/fixed-array/`Slice[T]` runtime-sequence including `name @ ..` rest-binding/struct/tuple-struct/alias/enum/wildcard parameter patterns, ownership-carrying tuple/fixed-array/struct/tuple-struct value parameter patterns, `ref` / `ref mut` and `&` / `&mut` name/wildcard/tuple/fixed-array/struct and addressable enum-case payload reference parameter patterns over hidden function-entry storage, root `Vec[T]` direct and function-pointer parameters through the Slice-shaped view ABI including named local array/Vec bindings and temporary Vec literal/control-flow expressions, root `Vec[T]` impl receivers through the same view ABI, return values, recursion, void functions, function pointer values/calls, expression-body and block-body `fn(...)` lambdas, explicit lambda parameter/result types without an expected type, non-capturing lambdas optimized as function pointer values, by-value pointer-free scalar/product/closure captures lowered as closure environment fields, generics surface | missing `main`, wrong `main`, wrong arg count, duplicate concrete function paths, bad parameter pattern type, missing return, bad return type, private module function, untyped lambda without expected function type, lambda arity mismatch, capturing lambda coerced to plain function pointer, owning, borrow-carrying, raw-pointer, zone-pointer, or local-Vec lambda capture before full closure lifetime support, bare root `Vec[T]` in function return and function pointer return signatures because root Vec is local storage or a borrowed parameter view only | LLVM calls, function-entry parameter destructuring, function-entry reference parameter binding, indirect calls, lambda lowering as generated internal functions plus function refs, closure aggregate construction with hidden function pointer and captured environment fields, closure-valued captures, unit return lowering, LLVM narrow scalar return normalization, LLVM direct `f32`/`f64` parameters and returns, LLVM direct/function-pointer aggregate parameters and returns through hidden pointers, LLVM local/temporary-Vec-to-Slice parameter and impl receiver views | good first pass |
| Variables | `let`, `var`, `let mut` declaration-level pattern mutability, local ownership-carrying tuple/fixed-array/struct/tuple-struct value pattern moves plus hidden-storage `Vec[own T]` exact/known-length-suffix and direct unknown-length suffix value pattern moves, skipped owned element cleanup, dynamic rest-gap cleanup, and known-length owned rest aliases as `Slice[own T]` views, local `let ref` / `let ref mut` and `let &` / `let &mut` name/wildcard/tuple/fixed-array/struct reference pattern bindings including ownership-carrying tuple/fixed-array/struct places and exact local `Vec[own T]` element slots plus nested owned fields inside aggregate Vec elements and known-length local `Vec[own T]` `..` prefix/suffix element slots plus shared and mutable unknown-length suffix synthetic owner borrow paths without rest aliases, direct local `Vec[T]`, `Slice[T]`, and non-owning `Slice[own T]` prefix/rest/suffix reference sequence bindings over tracked local places/views including shared nested `Slice[T]` element subpatterns and addressable enum payload slots, annotations, assignment, scalar inference, unit `()` values, fixed-size array locals, default non-empty `[]` array literals, constant and local dynamic array indexing with runtime bounds checks, typed empty local `Vec[T]` literals, local `Vec[T]` literal reassignment with changing runtime length, fixed-capacity local literal/const/static-expr/known-local/runtime-checked `Vec.reserve(n)` / static-length-tracked `Vec.truncate(n)` / `Vec.push(value)` / `Vec.insert(index, value)` / `Vec.pop()` / `Vec.remove(index)` / `Vec.first()` / `Vec.last()` / `Vec.get(index)` / `Vec.contains(value)` / `Vec.index_of(value)` / `Vec.count(value)` / `Vec.as_ptr()` / `Vec.capacity()` / `Vec.is_empty()` / `Vec.clear()` / `Vec.set(index, value)` / `Vec.swap(a, b)`, `len(value)` / `value.len()` array, vector, and `Slice[T]` length queries, mutable local array/Vec `as_slice()` view creation, `Slice[T]` indexing, indexed assignment, and range slicing with runtime bounds checks, tuple/wildcard/unit/array/struct `let` and `var` pattern bindings, runtime length-checked `Vec[T]`/`Slice[T]` `[ ... ]` `let` and `var` bindings including `name @ ..` rest slices, refutable tuple/fixed-array/named-struct/tuple-struct or-pattern `let`/`var` bindings, refutable single-payload enum `let`/`var` bindings, refutable enum or-pattern `let`/`var` bindings, match payload bindings including direct temporary `own i64`/`own u64` enum payload bindings, direct-constructor stored enum owned-payload move/drop checking, runtime-dependent whole enum owned-payload drop cleanup, tag-conditioned runtime-dependent enum payload-slot moves after match/if-let/while-let narrowing, and uniform owner-layout enum payload-slot moves | immutable assignment, shadowing, type mismatch, fixed array size/index/pattern mismatch, untyped empty `[]` literals, vector length/type mismatch, vector reserve/shrink bounds trap, tuple pattern arity/type mismatch, struct pattern non-struct/field mismatch, mutable reference pattern over immutable source, reference pattern over non-place temporaries, unknown-length owning vector rest aliases | stack/local lowering, host string/float slots, unit `zeroinitializer`, fixed arrays, tuple/array/struct destructuring on LLVM, local reference pattern lowering on LLVM borrow paths, refutable aggregate, runtime sequence, and enum binding panic paths, vector LLVM storage, and stored local vector literals/copies/indexing/methods/loops | good first pass |
| Constants | top-level scalar constants, file-backed module constants, module-cache scalar and aggregate constant summary materialization, module paths/imports, scalar folding, scalar constant patterns, tuple/struct/tuple-struct/fixed-array/enum constant values, generic enum constant values from concrete annotations | cycle path diagnostics, private constants, scalar pattern type mismatch, aggregate arity mismatch, invalid constant shifts, generic enum constant type mismatch, ownership/borrow-valued aggregate constants | inline scalar and aggregate IR lowering on LLVM where the aggregate backend is supported | good first pass |
| Generics | generic functions, explicit `<T>` generic call type arguments, angle-bracket generic type annotations including nested `Type<Other<T>>`, generic type aliases with `type Name[T] = Target[T];`, repeated function specializations, generic function names specialized as function pointer values from expected `fn(...) -> ...` types, generic structs, explicit and inferred generic struct literal / tuple-struct construction, generic inherent impl methods with `self` receivers, explicit and inferred method-level generic calls on inherent impl methods, generic inherent associated functions, explicit and inferred method-level generic associated calls, generic trait impl methods with `self` receivers, explicit and inferred generic trait method calls, method-level bounds on generic trait methods, generic trait impl associated functions, trait-qualified associated function calls with explicit implementing type arguments and expected-result inference, generic impl bounds, generic trait impl coherence, generic traits, generic ADT surface, explicit/payload-inferred/expected-type generic enum constructors, generic enum constants, generic enum `?` / `??`, generic enum match and refutable payload bindings | inference conflicts, uninferred parameters, uninferred generic function pointer parameters, explicit type-argument arity/mismatch, duplicate generic parameters, type alias arity mismatch, recursive type aliases, unsatisfied generic impl bounds, overlapping generic/concrete trait impls, generic method arity mismatch, uninferred generic method parameters, generic trait method bound mismatch, generic associated arity mismatch, uninferred generic associated parameters, ambiguous trait associated functions, generic enum constructor errors, generic enum `??` fallback type mismatch, generic enum payload pattern type mismatch, generic enum constant mismatch | monomorphized function calls, alias expansion before layout and codegen, specialized generic function pointer refs, specialized generic impl method and associated calls, aggregate surface validation, generic enum LLVM aggregate construction, try/coalesce extraction, compact payload pattern extraction, and inline generic enum constants | partial |
| Primitive types | signed/unsigned widths, `char` as an ASCII `u8` alias, suffix literals, bool, float/string host values, LLVM local `f32`/`f64` literal storage and raw pointer copies | literal ranges, invalid casts, invalid suffixes | exact host types, width-aware LLVM scalar local loads/stores, byte-packed standalone scalar locals, raw `f32`/`f64` literal and pointer bit storage, LLVM backend tuple/struct/fixed-array byte layout, local aggregate enum layout, external ABI still pending | partial |
| Operators and literals | arithmetic, modulo, bit ops, bitwise-not, shifts, comparisons, logical ops, postfix `?` with compatible residual conversion, generic prelude `Option`/`Result` and non-generic Option/Result-style `??`, integer base prefixes, C/Unicode string escapes, integer, float-width, and integer/float explicit casts, `f32`/`f64`/`f128` LLVM ops, `f32`/`f64` LLVM arithmetic/comparisons/casts | non-bool logic, non-integer bit ops, invalid `?`, incompatible `?` residuals, invalid `??`, invalid base/escape literals, invalid casts, LLVM f128 ops | LLVM IR, with raw `f32`/`f64` support through local literal, raw pointer bit storage, SSE arithmetic, ordered comparisons, width casts, integer/float casts, and direct call scalar ABI slots | good first pass |
| Control flow | `if`/`else if`, literal-bool statement `if` flow and loop-jump pruning, expression-valued `if`, expression-valued enum and aggregate `if let`, enum `if let` and `while let` with alias-wrapped or-pattern alternatives, aggregate and runtime sequence `if let` statement/expression and aggregate/runtime sequence `while let` or-pattern alternatives for tuple/fixed-array/named-struct/tuple-struct/`Vec[T]`/`Slice[T]` bindings, value-producing block expressions, aggregate-valued `if`/`match`/block expression results with expected result type propagation into result arms, borrow-valued block/`if`/`match`/labeled-block results when every arm preserves source path and mode, owning labeled-block `break label value` results, loop `break` ownership-state merge checks including maybe-zero `Alive`/moved-or-dropped `maybe-unavailable` owner exits, literal/known-immutable-bool-alias `while true` break ownership exits including moved/dropped unavailable-owner and same-provenance borrow-release merges, no-zero plain `while`, `init while`, irrefutable aggregate/runtime sequence `while let`, direct/immutable-local-or-alias enum-constructor `while let` with statically satisfied payload literal/range/nested-enum tests, runtime-dependent refutable enum `while let`, and direct iterator loops with next-iteration owner widening rechecked under candidate moved/dropped states, known-nonempty range/list/stored-`Vec` `for` break owner exits plus exact-once body/continue/break owner exits, continue next-iteration owner merges, same-provenance borrow-release merge checks for `break`/`continue`/fallthrough loop states, return flow, and no-break non-fallthrough flow, literal/known-immutable-bool-alias `while false` zero-iteration owner and borrow-state flow, literal/known-immutable-bool-alias false `init while` zero-iteration owner flow, literal/known-immutable-bool-alias true `init while` non-fallthrough, return flow, and value-continue owner merges, loop `continue` ownership-state merge checks, `break`/`continue` live-owner scope-exit checks, exact loop borrow-state fixed points, final-expression function returns, required `;` for non-final expression statements and non-block statements, typed `break label value`, tuple/struct-valued labeled block results, `if let`, `while`, `for range`, integer `Range[T]` and `RangeInclusive[T]`, `0..end`, `0..=end`, range `for` loops with irrefutable alias heads, list literal and stored-vector `for` loops with irrefutable alias/tuple/struct/tuple-struct heads and source-known Vec control-flow loop bounds, direct `Iterator[T]` and `IntoIterator[T]` for loops including mutable `Iterator.next` receivers, iterator item scalar literal/range/or, fieldless enum-case, compact enum payload, and nested aggregate enum multi-payload item patterns with nested or-pattern slots, owning `init while` update bindings, labeled loop/block break | bad conditions, missing `;`, removed `let while` loop-state spelling, if-expression branch type mismatch, borrow-result source mismatch, borrow-result local escape, if-let-expression branch type mismatch, irrefutable aggregate/runtime sequence if-let with else, final value accidentally terminated with `;`, labeled-block break type mismatch, labeled-block ownership exit mismatch, loop-break ownership exit mismatch, loop-continue ownership mismatch, loop-borrow-state mismatch, maybe-unavailable owner use/overwrite/owned-field-overwrite/return/scope-exit diagnostics, break/continue over live owning loop-local bindings, live owning init-while update overwrite, plain continue in owning init-while, non-integer range types, continue outside loop, bad labels, unsupported iterator/refutable for-loop shapes | LLVM loops plus aggregate/control-flow borrow result materialization, signed and unsigned range compares, multi-alternative if-let/while-let dispatch, aggregate/runtime sequence product if-chain lowering, iterator `while let Some(pattern)` lowering | good first pass |
| Ownership | `own`, move, explicit `drop`, explicit `forget` for live or maybe-unavailable owners, `Drop::drop` destructor lowering, aggregate field destructor lowering, tag-guarded whole-value drop cleanup for runtime-dependent owner-carrying aggregate enums, non-owned field read/assignment from owning structs, owned struct/tuple-struct field move and reinitialization, nested owned field paths, constant fixed-array/vector element move and reinitialization, hidden-storage `Vec[own T]` exact/known-length-suffix and direct unknown-length suffix value pattern element moves plus selected `_`, known and runtime rest-gap element drops, known-length owned rest aliases with hidden-source cleanup, direct temporary `own i64`/`own u64` enum payload match binding and explicit drop, direct-constructor stored enum active payload move/drop tracking, runtime-dependent tag-conditioned enum payload moves across match/if-let/while-let and moved-or-dropped branch-state merges, return ownership, whole-value tracking for aggregates containing owned fields | use-after-move, use-after-forget, forgetting non-owners, owned field/element use-after-move, dynamic indexed owned element move, temporary owned field/element move, partial aggregate move, unknown-length owning vector rest aliases, moves or indexed owner replacement through non-owning `Slice[own T]` views, undropped owner, overwrite live owner, undropped owning aggregate | sema-focused, LLVM destructor call, LLVM aggregate storage, moved-field destructor skip, forget no-drop lowering | good first pass |
| Borrowing | `&`, `&mut`, `ref`, `ref mut`, `&T`/`&mut T` borrow type spellings, `*T` raw pointer type spelling, call borrows, named borrow bindings, local `let ref` / `let ref mut` and `let &` / `let &mut` reference pattern bindings including ownership-carrying tuple/fixed-array/struct places, exact local `Vec[own T]` element slots and nested owned fields inside aggregate Vec elements plus known-length local `Vec[own T]` `..` prefix/suffix element slots and shared or mutable unknown-length suffix synthetic owner borrow paths without rest aliases, direct local `Vec[T]`, `Slice[T]`, and non-owning `Slice[own T]` prefix/suffix element borrows with `name @ ..` Slice rest aliases, ownership-carrying direct `Vec[own T]` rest aliases as whole-source-borrowed `Slice[own T]` views, nested shared/ref-mut `Slice[T]` element subpattern borrows, and addressable enum payload slot borrows, function parameter `ref` / `ref mut` and `&` / `&mut` reference pattern bindings, named-borrow last-use release in straight-line statement scopes, local reborrows from `ref`/`ref mut` bindings, read-only field projection through shared struct borrows, `ref mut Self` method receivers calling `ref Self` inherent and trait-qualified methods, field/element reborrows through borrow bindings, borrow-valued control-flow result provenance, constrained borrow-valued function and method returns from a single borrow parameter, explicit `@borrow_return(source.path)` contracts for multi-parameter and extern borrow-returning functions, returned borrow subpath contracts for function/method calls, lexical block release, aggregate bindings that retain borrowed source paths per target field, borrow-valued aggregate field reads/reassignment and whole-local reassignment, shared borrow-valued aggregate copies, direct struct/tuple/constant array/vector field borrows, independent field-path borrow tracking | mut borrow immutable, mutable reference pattern over immutable source, compact/non-addressable enum payload reference patterns, moved owned field borrow through an ownership-carrying reference pattern, moved-owner unknown-length suffix borrow or live dynamic suffix borrow blocking whole-vector mutation/drop, mutable reborrow from immutable borrow binding, mutable path reborrow from immutable borrow binding, overlapping field borrow, active reborrow conflict while a later use keeps the prior borrow live, borrow result source mismatch/local escape, borrow-return local escape, missing extern borrow-return contract, borrow-return path mismatch, explicit borrow-return path mismatch, function and method call-site conflict after binding a borrow return that is still visibly live, borrowed field assignment, source assignment while named-borrowed or aggregate-borrowed, reassigned aggregate source remains borrowed, discard borrow, borrow binding reassignment | host pointer lowering, LLVM address lowering, LLVM aggregate field pointers, local and function-entry reference pattern lowering to named borrow bindings, local reborrow pointer forwarding, borrow-result pointer forwarding, borrow-return function/method/extern call subpath pointer forwarding, named-borrow source release after last visible use, borrow-valued aggregate field-source release/reacquire, pointer-backed field/array/Vec path reborrow lowering | good first pass |
| Structs and tuples | comma-separated fields, named struct literals, generic struct declarations, tuple structs, inherent associated constructors, module-visible field types, field `mut` assignment, unit/empty tuple type and literal, fixed-size local tuples, tuple index access, struct field/index access | semicolon field separators, duplicate structs, duplicate fields, unknown/private field types, bad generic arity, immutable field assignment, immutable binding field assignment, unknown/private associated constructors, unknown fields, tuple index range | front-end validation, tuple stack lowering, empty tuple lowering, local struct stack lowering on LLVM, LLVM aggregate-return constructor calls, LLVM direct aggregate returns | good first pass |
| Enums | comma-separated cases, constructors, compact payloads, LLVM aggregate layout for multi-payload and i64/u64 payload cases, aggregate enum pointer-shaped payloads (`string`, `ptr T`, `fn(...) -> ...`), `own i64`/`own u64` payload storage for direct temporary constructor matches and direct-constructor stored locals, tag-guarded whole-value drop cleanup for runtime-dependent owner-carrying aggregate enums, runtime-dependent owner payload-slot moves after match/if-let/while-let tag narrowing, homogeneous nested aggregate-enum payload slots on LLVM, mixed payload-word/nested aggregate-enum slots and mixed payload-word/plain-aggregate slots when the aggregate layout has a first `i64`/`u64` scalar lane, fixed-capacity `Vec[T; N]` aggregate payload slots, direct local/raw-pointer-backed aggregate enum payload slot access with `.0`/`.1`, LLVM direct aggregate enum match inputs from constructors, calls, control-flow values, and raw-pointer loads, equality for compact enums, generic ADT surface, generic enum type applications, explicit, payload-inferred, and expected-type generic enum constructors, generic enum constants | semicolon case separators, payload count, unsupported aggregate payload types, incompatible mixed aggregate enum payload slots, duplicate cases, generic enum constructor arity, uninferred generic enum constructor type arguments, generic enum constructor type mismatch, generic enum constant mismatch | tagged-union lowering, LLVM aggregate-enum lowering with pointer-to-slot casts, LLVM local aggregate-enum constructor/copy storage, aggregate-enum parameter passing, pointer-backed aggregate-enum copies, direct aggregate-enum constructor stores through raw pointers, direct aggregate enum materialization into hidden raw stack slots before tag/payload reads, direct payload slot lvalue loads/stores/copies on LLVM, and tag/payload-binding/literal/range/nested compact, homogeneous aggregate-enum, fixed-vector aggregate-enum, or mixed-lane aggregate-enum matches, external aggregate enum ABI still pending | partial |
| Pattern matching | exhaustive enum match, generic and non-generic enum payload bindings, wildcard, grouped patterns, unit `()` patterns, or-patterns with same-name/same-type bindings across enum, tuple, fixed-array, runtime sequence, named-struct, and tuple-struct match arms, alias patterns, alias-wrapped or-patterns, payload binding, enum payload literal/range/or/alias patterns, aggregate enum payload literal/range/or/alias patterns on LLVM, direct temporary `own i64`/`own u64` enum payload value bindings with explicit drop, tag-conditioned runtime-dependent aggregate enum owner payload bindings and payload-slot moves in statement/expression match plus if-let/while-let narrowing, local aggregate enum payload literal/range/or/alias patterns on LLVM, nested compact-enum subpatterns inside aggregate enum payloads on LLVM and local LLVM values, homogeneous and mixed-lane nested aggregate-enum subpatterns inside aggregate enum payloads on LLVM, local aggregate enum positional/rest payload bindings on LLVM, bool payload literal exhaustiveness, multi-payload enum positional/rest binding patterns, module-qualified cases, integer/bool literal and range patterns, ordered first-match range overlap, scalar shadow warnings, finite integer range exhaustiveness, nested tuple match patterns, tuple `..` rest patterns, named struct patterns, tuple-struct patterns, runtime length-checked `Vec[T]`/`Slice[T]` `[ ... ]` patterns, finite product exhaustiveness, symbolic high-cardinality product exhaustiveness, aggregate shadow warnings, expression-valued match, nested shared reference binding modes in enum statement/expression match and enum if-let/while-let patterns including enum while-let or-pattern alternatives, mutable enum payload reference bindings in enum statement/expression match and enum if-let/while-let patterns against addressable subjects, mutable tuple/fixed-array/struct field reference bindings in statement/expression match and aggregate if-let/while-let patterns against addressable subjects, mutable runtime-sequence `Vec[T]`/`Slice[T]` element reference bindings in statement/expression match and if-let/while-let patterns against addressable subjects | non-exhaustive enum/integer/bool/tuple/struct/runtime-sequence matches, missing payload pattern, duplicate payload literals, invalid range order, mismatched or-pattern bindings, incomplete integer range coverage, incomplete bool payload coverage, incomplete product coverage with missing-case hints, ref mut control-flow binding over non-addressable temporaries, stored ownership-carrying runtime-sequence owner paths before ABI support, incompatible expression arms, wrong literal kind | branch lowering, expression phi/scalar and aggregate result lowering, runtime sequence len/index lowering, aggregate enum LLVM extraction, LLVM aggregate enum tag tests and payload binding/literal/range/nested compact, homogeneous aggregate-enum, owned-word temporary, owner-payload, or mixed-lane aggregate-enum loads, control-flow ref-mut payload/field/sequence-element bindings borrow the original addressable subject while hidden match/while-let/product/sequence storage drives tests | good first pass |
| Modules | inline `mod`, file `mod`, `.arih`, package search paths, module load cache, compact module metadata emission/read-back validation, source content hashes, metadata format-version gating, source-snapshot module cache emission/use, per-source AST-summary cache records, per-source IR-summary sidecar records including fixed-capacity vector enum-payload length conditions, IR-summary layout descriptors, declaration fingerprints, self-validated declaration-summary payloads, simple function body shape and operand-tree summary payloads, `pub`, `pub mod`, `use`, use groups, glob imports, module aliases, `pub use` re-exports, `A::B`, `self::`/`super::` relative paths | private access, private nested modules, duplicate aliases, missing file modules, stale module metadata, stale source hash, stale module cache cfg/source/import/AST-summary/IR-summary resolution, old metadata/cache versions without hashes or AST summaries, malformed/tampered IR summaries, malformed/tampered IR layout descriptors, private re-export aliases, root `super::` | name resolution, module graph summary serialization, module cache source snapshot and AST/IR-summary validation | good first pass |
| Source std API coverage gate | `check-std-api` extracts public modules, re-exports, types, constants, functions, traits, trait methods, and public inherent methods from `lib/std.arih` and `lib/std/*.arih`; `tests/std_api_manifest.txt` records each entry with the focused coverage note that should be updated with any new source `std` API | stale manifest entries, missing coverage notes, duplicate entries, unsorted entries | no backend lowering; this is a lightweight policy gate wired into `make check` before broad source `std` expansion | good first pass |
| Prelude IO/format/input/assert | `print`, `println`, `eprintln`, `std::print`, `std::println`, `std::eprintln`, `std`-alias qualified print names, `{}`, `{name}`, `{name.field}`, `{name.0}`, `{:?}`, `{name:?}`, `{:.N}`, and `{name:.N}` format placeholders, escaped braces, formatted lowercase `string`/`char`/`u64`/bool/`f32`/`f64` output on LLVM, stderr formatting through `eprintln!`, LLVM static lowercase `string` literal storage with raw-pointer byte reads and `write_byte`, source `io::write_bytes`/root `write_bytes`, source `io::Reader`/`Writer`/`Seek`, `Cursor`, `stdin`, `stdout`, `stderr`, `read_exact_result`, `copy_result`, `write_all_result`, `flush_result`, `read_exact`, `write_all`, `flush`, `print!`, `println!`, `eprintln!`, `format_in!` explicit-zone source `String` construction for `{}` and named captures over local fields/tuple indexes with lowercase `string`/byte-character/signed and unsigned integer/bool/`f32`/`f64`/Display values, `{:?}` and `{name:?}` Debug values, and `{:.N}`/`{name:.N}` float precision with once-evaluated function-call/computed arguments plus named local capture, `matches!`, IO aliases, `write_i64`, `write_u64`, `read_byte`, source `input::try_read_byte`, host `read_line`/`input`, explicit-zone owned line helpers returning source `std::string::String`, root `String` / `std::String` aliases for the source string handle, source string append helpers for lowercase `string`, `char`, `i64`, `u64`, bool, `f32`, and `f64`, source `Option[T]` / `Result[T, E]` / `Slice[T]` and `slice(data, len)`, Option/Result borrowed view handles, borrowed and consuming payload predicates, `unwrap`/`expect`/`unwrap_or`/`unwrap_or_else`, Option `as_ref`/`as_mut`/`take`/`replace`/`map`/`or`/`or_else`/`xor`/`and_then`/`filter`/`flatten`/`transpose`/`ok_or`/`ok_or_else`, and Result `as_ref`/`as_mut`/`ok`/`err`/`map`/`map_err`/`and_then`/`or`/`or_else`/`transpose`/`unwrap_err`/`expect_err` methods, Slice `len`/`is_empty`/indexing/indexed assignment/range slicing/read/mutable access/search/predicate search/affix stripping/comparison/copy-to-Vec methods, mutable local array/Vec `as_slice()` view creation, `std::mem::replace` / `std::mem::swap` plus root aliases for scalar/plain aggregate values, assertion function and macro helpers | bad placeholder count, nonliteral format, bad precision placeholder syntax/type, bad named-capture syntax, `format!` without an implicit allocation zone, unsupported `format_in! f128`/non-Display/non-Debug values, line input, owned line/root String use after zone reset, mismatched string append/growth zone, bad assert type, bad assert macro type, bad `matches!` arity/patterns, ownership-valued `std::mem::replace`/`swap`, removed `Maybe[T]` alias | stdio, getchar/fgets, LLVM string constants, decimal integer, named capture lowering, byte-character display/debug formatting, debug string quoting, float formatting, and stderr routing, source byte-slice output, source IO trait/cursor/buffered/stderr exact-read/write-all helper monomorphization, source input EOF-to-Option helper, source string copying/appending for owned line and explicit text construction, statement-position and expression-arm stop helpers lowering as noreturn/bottom-like control flow, Option/Result predicate, borrowed-view, take/replace, unwrap/expect, conversion, borrowed-predicate filtering, flattening, bidirectional transposition, eager/lazy fallback, and combinator method lowering on source generic enums, `std::mem` value helper lowering through pointer materialization, assertion exits, pattern-engine macro lowering | partial |
| Standard algorithms | `std::algo::sort`, `sort_by`, `stable_sort`, `stable_sort_by`, `binary_search`, `binary_search_by`, `lower_bound`, `lower_bound_by`, `upper_bound`, `upper_bound_by`, `equal_range`, `equal_range_by`, `partition_point`, `is_sorted`, `reverse`, `rotate_left`, `rotate_right`, `partition`, `min`, `max`, `clamp`, `swap`, `fill`, `copy`, and `dedup` over `Slice[T]` views, plus receiver-form `Slice[T]` and `std::vec::Vec[T]` wrappers for reverse/rotate/fill/copy_from/partition/dedup/sort/stable_sort/is_sorted/binary_search/lower_bound/upper_bound/equal_range/partition_point/min/max | future large-slice sort stress tests and ownership-valued algorithm rejection/move policy tests | source helper monomorphization over borrowed slices; future fast generic sorts may need move-aware temporary storage and stricter `Copy`/ownership constraints | partial |
| Standard hashing | `std::hash::Hasher`, `Hash[T]`, `new`, `reset`, `finish`, generic `write<T>`/`value<T>`, byte-slice `bytes`, primitive `write_byte`/`write_bytes`/`write_i64`/`write_u64`/`write_bool`, and the `collections::hash_i64` compatibility wrapper | future aggregate impls, trait-driven default constructors for hash collections, seed/collision-policy tests, and encoding-module split tests | pure source helper monomorphization today; future collection defaults may need richer trait dispatch and default hasher function selection | partial |
| Standard random | `std::random::entropy`, direct OS byte `fill`, recoverable `entropy_result`/`fill_result` plus raw compatibility result forms, deterministic non-cryptographic `Prng`, `seed`, `from_entropy`, `from_entropy_result`, `seed_from_os`, `seed_from_os_result`, raw `next`, `boolean`, unbiased bounded integer `below`/`range`, unit `float`, PRNG byte `fill_from`, and generic `shuffle` over `Slice[T]` views | future larger byte-fill tests, statistical bounded integer tests, CSPRNG stream policy, and distribution helpers if they belong in core std | `entropy` and `fill` are hosted runtime hooks using Linux `getrandom` with `/dev/urandom` fallback; result helpers bridge hosted errno failures through `std::error`; PRNG helpers, deterministic booleans, unbiased bounded integer sampling, and shuffle are source Ari | partial |
| Standard parsing | `std::parse::Parse`, `parse<T>`, `parse_or<T>`, `is_parse<T>`, signed `integer`/`integer_radix`, unsigned `unsigned`/`unsigned_radix`, hex/binary/octal signed wrappers, `boolean`, `is_float`, `float_or`, and `float` over ASCII-trimmed `Slice[u8]` values | future exponent edge, richer parse-error, and locale-rejection tests | pure source helper monomorphization today; generic calls now accept string-literal-to-slice coercion, and `Option[f64]`/`Result[f64,E]` needs float enum payload lowering | partial |
| Standard encoding | `std::encoding::is_ascii`, `utf8_count`, `is_utf8`, `utf8_width`, `utf8_at`, `utf8_next_index`, `utf8_encoded_len`, `encode_utf8_in`, `try_encode_utf8_in`, `utf16_count`, `is_utf16`, hex length/encode/decode/fallible-decode guards, and base64 length/encode/decode/fallible-decode guards | future URL-safe base64, MIME line wrapping, richer decode error values, normalization/transcoding, and optional compression tests | pure source helper monomorphization today; `Option[String]` decoders use compiler-tracked zone-backed enum payloads | partial |
| Standard paths | `std::path::is_separator`, `is_absolute`, `is_relative`, `trim_trailing_separators`, `components`, `file_name`, `parent`, `extension`, `stem`, component-aware `starts_with`/`ends_with` and `strip_prefix`/`strip_suffix`, `join_in`, and `normalize_in` over byte paths | future Windows drive/UNC paths, owned path buffers, and richer component kinds | pure source helper monomorphization today; filesystem-backed existing-path canonicalization is covered by `std::fs` | partial |
| Context, environment, process, thread, sync, and filesystem runtime | `context::argc`, `context::arg`, `context::thread_id`, startup `context::cwd`, startup `context::executable_path`, source `context::has_args`, `context::has_arg`, `context::user_arg_count`, `context::has_user_args`, `context::is_main_thread`, startup cwd/executable optional and OS/path view helpers, user-facing `env::arg_count`, `env::arg`, `env::has_arg`, `env::try_arg`, `env::program_name`, `env::get`, `env::has`, `env::try_get`, `env::set`, `env::remove`, `env::current_dir`, `env::try_current_dir`, `env::set_current_dir`, `env::executable_path`, `env::try_executable_path`, `process::id`, `process::uid`, `process::gid`, `process::exit`, `process::abort`, `process::fork_result`, `process::wait_result`, raw `process::fork`, raw `process::wait`, source process command builders, module-level `process::spawn`, `process::status`, `process::exit_status`, `process::output_in`, `process::exec`, source process status/root/branch/error helpers, `thread::spawn`, `thread::join`, `thread::yield_now`, `thread::id`, `thread::is_main`, `thread::is_join_error`, `thread::builder`, `thread::is_finished`, `thread::thread_local`, `thread::thread_local_with_capacity`, `Thread` method wrappers, `Builder`, `ThreadLocal`, `sync::load`, `sync::store`, `sync::swap`, `sync::fetch_add`, `sync::compare_exchange`, `sync::try_lock`, `sync::lock`, `sync::unlock`, `sync::call_once`, `Ordering`, `AtomicI64`, `AtomicBool`, `AtomicUsize`, `AtomicPtr`, `Mutex`, `RwLock`, `Once`, `OnceLock`, `Condvar`, `Barrier`, `Channel`, `Sender`, and `Receiver` method wrappers, `fs::exists`, `fs::can_read`, `fs::can_write`, `fs::can_execute`, `fs::permissions`, `fs::metadata`, `fs::try_metadata`, `fs::symlink_metadata`, `fs::try_symlink_metadata`, `fs::mode`, `fs::try_mode`, `fs::set_mode`, `fs::set_permissions`, `fs::canonicalize`, `fs::try_canonicalize`, `Metadata`, `Metadata::accessed`, `Metadata::modified`, `Metadata::changed`, `FileKind`, `fs::remove`, `fs::remove_dir_all`, `fs::rename`, `fs::hard_link`, `fs::symbolic_link`, `fs::read_link`, `fs::try_read_link`, `fs::create_dir`, `fs::remove_dir`, `fs::open_dir`, `fs::try_open_dir`, `fs::read_dir`, `fs::try_read_dir`, `fs::read_dir_entries`, `fs::try_read_dir_entries`, `fs::read_dir_next`, `fs::close_dir`, `Dir`, `DirEntry`, `DirEntry::metadata`, `DirEntry::try_metadata`, `DirEntry::symlink_metadata`, `DirEntry::try_symlink_metadata`, `DirEntry::try_file_type`, `DirEntry::is_file`, `DirEntry::is_dir`, `DirEntry::is_symlink`, `DirEntry::is_other`, mode-string `fs::open`/`fs::try_open`/`fs::open_result` with `"r"`, `"w"`, `"a"`, `"rw"`, `"r+"`, `"w+"`, and `"a+"`, `OpenOptions`, `fs::open_options`, `OpenOptions::read`, `OpenOptions::write`, `OpenOptions::append`, `OpenOptions::truncate`, `OpenOptions::create`, `OpenOptions::create_new`, `OpenOptions::open`, `OpenOptions::open_result`, `OpenOptions::try_open`, `fs::create`, `fs::create_result`, `fs::try_create`, compatibility `fs::open_*`/`fs::try_open_*` wrappers, `File` byte read/write/position/seek/close helpers, source `fs::read`/`fs::read_result`/`fs::try_read`/`fs::write`/`fs::try_write`/`fs::append`/`fs::try_append`/`fs::truncate`/`fs::copy`/`fs::try_copy`/`fs::read_to_string`/`fs::read_to_string_result`/`fs::try_read_to_string`, aliases, `@ari_entry`/`ari::main` host wrapper | shared-library context use, process stdin and large-output draining, signal mask/action handling, memory mapping, future path normalization, future richer thread statuses, futex-backed blocking policy, true OS condvars/channels, compiler-level thread-local declaration sugar, future owned filesystem resource policy, future richer per-entry directory-error/temp-file/locking filesystem slices plus richer owner/group/ACL, portable creation/birth-time, and platform symlink policy | host runtime init, thread-local `@ari_thread_id`, `getenv`, `setenv`, `unsetenv`, `getcwd`, `chdir`, `readlink`, `getpid`, `getuid`, `getgid`, `exit`, `abort`, `fork`, `waitpid`, `pthread_create`, `pthread_join`, `pthread_kill`, `sched_yield`, LLVM atomic load/store/RMW/CAS, source thread-local/mutex/once/atomic/condvar/barrier/channel helpers, standard thread/sync zone-pointer provenance helpers, `access`, `stat`, `lstat`, `chmod`, `realpath`, `readlink`, `unlink`, `rename`, `link`, `symlink`, `mkdir`, `rmdir`, `opendir`, `readdir`, `closedir`, `open`, `read`, `write`, `lseek`, `close`, plus source helper monomorphization, stack-local byte-path adapters for `DirEntry` metadata methods, metadata timestamps, recursive tree removal, direct `OpenOptions` flag lowering, process command wrappers, and compact `std::error` bridges for open/read failures | partial |
| Environment OS/path views | `env::arg_os`, `env::try_arg_os`, `env::program_name_os`, `env::get_os`, `env::try_get_os`, `env::current_dir_os`, `env::try_current_dir_os`, `env::current_dir_path`, `env::try_current_dir_path`, `env::executable_path_os`, and `env::try_executable_path_os` | future owned `OsString`/`PathBuf`, Windows OS-string semantics, and canonicalization/error policy | source wrappers over existing runtime C-string hooks plus `std::string::OsStr` and `std::path::PathBytes` boundary helpers | partial |
| C FFI | libc calls, aliases, module externs, concrete C ABI bindings only, C ABI type aliases, C varargs with default promotions, C callback/function-pointer params, helper library, LLVM object scalar/raw-pointer C import relocations plus aggregate-export object fixtures, direct by-value `@repr(C)` struct imports accepted by the shared aggregate ABI classifier, `ref mut` pointers, borrow-returning extern declarations with `@borrow_return`, `ptr c_char` strings, `@repr(C)` structs with value/fixed-array/raw-pointer/borrow fields, generic `@repr(C)` structs with value and pointer-sized generic fields, generic fieldless `@repr(C)` enums, `null` raw-pointer literals, nullable `T?` raw-pointer type spelling, raw-pointer casts including lowercase `string as ptr u8`, byte-wise `ptr_offset` with inferred or explicit `<T>`, typed scalar/aggregate `ptr_add` with inferred or explicit `<T>`, natural Ari aggregate `size_of<T>()` / `align_of<T>()`, scalar/plain-aggregate `ptr_load`/`ptr_store` with inferred or explicit `<T>`, `std::mem::copy_bytes`/`move_bytes`/`set_bytes`, `std::mem::replace` / `std::mem::swap` over scalar/plain aggregates, `f32`/`f64` raw-pointer load/store bit copies, scalar/plain-aggregate `*pointer` dereference load/store, scalar struct/tuple/fixed-array field and element access through raw pointers, aggregate enum payload slot access through raw pointers, scalar/raw-pointer exported C header prototypes, public non-generic `@repr(C)` struct declarations with `ref` as `const` C pointers and fixed-array fields as C array declarators, pointer-to-fixed-array C header prototypes for `ptr/ref/ref mut [T, N]`, by-value public non-generic `@repr(C)` struct export prototypes plus generated fixed-array, tuple, fixed-capacity-vector, and aggregate-enum wrapper typedefs for direct 64-bit Unix aggregate ABI values, opaque public generic `@repr(C)` struct typedefs for pointer-only APIs, concrete public generic-struct typedefs for by-value exported instantiations, public generic/non-generic fieldless enum tag typedefs, and public non-generic payload-bearing `@repr(C)` enum struct typedefs with scalar, pointer-shaped, and generated-wrapper non-scalar payload fields without `own` in C headers, `c_void` returns | rejected ABI, body, generic extern declarations, bad link symbol, unknown or mismatched Ari builtin declarations, by-value `c_void` param, non-extern varargs, unsupported aggregate varargs, oversized or non-`repr(C)` by-value extern C aggregate imports, variadic extern function pointer values, missing extern borrow-return contracts, `@repr(C)` owning fields or enum payloads, generic payload-bearing `@repr(C)` enums, oversized or non-Unix by-value C-header aggregates including fixed arrays, null used as non-pointer, nullable `?` combined with `own`/`ref`/`ptr`, non-pointer value initializers for `T?`, bad layout query arity, dereference of non-pointer values, whole raw-pointer copies of ownership/borrow-valued values, LLVM executable imported C extern calls and unsupported aggregate/varargs/float C ABI imports, C header prototypes for unsupported/oversized Ari aggregate values | LLVM declarations, explicit IR ABI split for Ari builtins vs C externs, signature-checked Ari builtin declarations, function pointer params as `ptr`, direct aggregate extern declarations, vararg bool/narrow-int/f32 promotion casts, `ptrtoint`/`inttoptr`, pointer-shaped lowercase string casts, `getelementptr i8`, typed scalar and aggregate pointer GEP, scalar and natural aggregate layout literals, scalar and plain-aggregate raw pointer load/store, LLVM memory intrinsic lowering for byte copy/move/set, float raw-pointer bit load/store, pointer dereference load/store, aggregate pointer GEP, aggregate enum payload GEP, LLVM byte field/array/enum-payload addressing, LLVM backend address add, LLVM object scalar imported C call relocations and aggregate exported symbol/relocation fixtures, LLVM executable imported C diagnostics, host linker, tracked extern borrow-return calls, and C header emitter | good first pass |
| Explicit memory zones | `Zone`, `zone::create`, lexical `zone::temp`, raw `zone::alloc`, source `zone::alloc_array<T>`, allocation-header zone-handle metadata (`zone::allocation_zone`), typed `std::zone::ZoneMetadata`, `std::zone::ZoneBacked` and `zone::of(ref value)` for heap-backed std handles, typed `zone::alloc<T>`, placement `zone::new<T>`, local scratch `zone::scratch<T>`, explicit scratch promotion `zone::promote<T>`, zone-backed associated `T::new(ref mut Zone, ...)`, `own dyn Trait` construction from tracked `ptr T`, `std::boxed::new<T>` / `Box!(T, ref mut Zone, value)` / `std::boxed::Box<T>` with value-drop handle `Drop` plus `take`/`is_empty`, `std::string::alloc_buffer`, `std::string::with_capacity` / `RawString`, `std::string::new` / `std::string::String`, `std::string::from_string`, `std::vec::alloc_buffer<T>`, `std::vec::with_capacity<T>` / `RawVec<T>`, `std::vec::new<T>` / `Vec!(T, ref mut Zone, capacity)` / `std::vec::Vec<T>` / `std::Vec<T>` with value-drop handle `Drop` plus overwrite/shrink element drops, `std::collections::new<T>` / `Set::new<T>` / `std::collections::Set<T>` with explicit-zone insertion, replace-or-insert, reserve growth, iteration, copy, and reset/destroy provenance, `HashMap<K,V>`/`HashSet<T>` open-addressed storage and iterators plus `TreeMap<K,V>`/`TreeSet<T>` red-black storage and iterators, `Deque<T>`/`LinkedList<T>`/`BinaryHeap<T>`/`PriorityQueue<T>` same-zone growth provenance, and fixed-capacity `RingBuffer<T>`, borrowed `Slice<T>` exact/prefix/suffix checks, and `Slice<T>.copy_to(ref mut Zone)` target-zone copies, `zone::reset`, `zone::destroy`, raw `ptr u8` and typed `ptr T` allocation used with layout queries and pointer load/store | missing destroy for non-temporary `own Zone`, rejected `drop zone`, rejected movement of temporary zones, rejected temporary-zone pointer escape through returns, outer bindings, aggregates, or call arguments, `zone::scratch<T>` rejected outside local pointer binding initializers, bad typed allocation arity, zero-sized typed allocation, bad placement-construction/promotion arity, bad `Box!` / `Vec!` constructor shape, ownership/borrow-valued placement/promotion rejection, direct local, single-zone wrapper, raw `std::string::alloc_buffer` byte pointers, tracked `own dyn`, tracked `std::string::RawString` and `std::string::String`, tracked `std::boxed::Box<T>`, tracked `RawVec<T>`, tracked `std::vec::Vec<T>` / `std::Vec<T>`, tracked `std::collections::Set<T>`, `Deque<T>`, `RingBuffer<T>`, `LinkedList<T>`, `BinaryHeap<T>`, `PriorityQueue<T>`, `HashMap<K,V>`, `HashSet<T>`, `TreeMap<K,V>`, and `TreeSet<T>` plus all collection iterator handles use after reset/destroy, set/hash/tree/deque/list/heap insertion/replace/push/reserve with a different zone, reset invalidation after `if`/`match`/loop joins, aggregate/FFI/no-zone/multi-zone zone-pointer escape rejection, LLVM zone allocation rejection | LLVM host runtime backed by `malloc`/`free`, fixed host allocation header storing only the zone handle at `ptr - 8`, ownership move into `zone::destroy`, automatic destroy insertion for temporary-zone scope fallthrough, returns, and escaping `break`/`continue`/labeled-block exits with value materialization before cleanup, `zone::scratch<T>` lowered to hidden `zone::temp` plus placement `zone::new<T>`, `zone::promote<T>` lowered to pointer load plus placement `zone::new<T>` in the target zone, typed allocation lowered to explicit zone allocation with compile-time layout, `zone::alloc_array<T>` lowered as source Ari over layout queries and raw zone allocation, placement construction lowered to allocation plus store, generic `Drop` impl specialization for zone-backed Box, source Vec, source String, source Set, deque, ring-buffer, linked-list, heap, priority-queue, hash-table, and red-black tree handle drops, source Vec and Set shrink/remove helpers drop removed live values before reducing length, Box `take` emptying before later handle drop, inherent associated call to zone placement, raw string allocation seed and source string handle lowering, direct and single-zone wrapper pointer provenance invalidation with named temporary-zone escape diagnostics, tracked own dyn/Box/source String/RawVec/source Vec/source Set/source deque/source ring-buffer/source linked-list/source heap/source hash/source tree/source collection iterator handle invalidation, signature-level single-zone return contracts, control-flow zone generation merge, own dyn erased drop thunks, deliberate host-only zone policy | good first pass |
| Shared libraries | compile library, Ari-mangled symbols, `@export`, `@export("symbol")`, `@no_mangle`, `--shared` public/default and private/hidden LLVM visibility, LLVM object export/no-mangle symbol table names including aggregate exports | main not required only with `--shared`, invalid export symbols, export-vs-mangled symbol collisions | dynamic symbol table and LLVM symbol table | good first pass |
| Comments | line and nested block comments | unterminated block comment | lexer | partial |
| Attributes and meta | built-in attributes, `@derive(Debug)` / `@derive(Copy)` / `@derive(Clone)` expansion for structs/enums/generic declarations, `@derive(Default)` expansion for named/tuple structs and generic declarations, `@derive(Default(CaseName))` expansion for enums with defaulted payload construction, `@derive(Eq)` / `@derive(PartialEq)` expansion for structs and fieldless/payload-bearing enums, `@derive(Ord)` / `@derive(PartialOrd)` lexicographic `lt` expansion for structs and source-order/payload-lexicographic enum ordering, `@repr(C)` layout guards including borrow-field, value/pointer-sized generic slots, generic fieldless enums, public payload enum payload slots without `own`, and emitted public non-generic struct with const-ref header slots and fixed-array fields, direct by-value struct/fixed-array header ABI guards, opaque and concrete generic-struct typedefs, generic/non-generic fieldless-enum declarations, plus non-generic payload-bearing enum struct declarations in C headers, `@cfg(true/false)` declaration pruning, boolean/target/feature cfg predicates, command-line cfg feature flags, deprecated use warnings, `@test` runner generation, `@export`/`@no_mangle` symbol controls, exported C header prototypes, user-reserved attribute names via concrete non-generic syntax-rewriting `meta fn`, attribute macro declaration rewrite/input substitution for top-level function/struct/enum/trait/impl declarations, attribute macro argument-token inspection and capture splicing in token output, generated rewriting-attribute expansion, one-parameter same-domain meta signatures over `token_stream`/`ast`/`type`, empty and explicit `return input;` identity meta bodies, expression-position `ast -> ast` expression returns with input substitution, expression-position `ast -> ast` struct-literal/borrow/try/null-coalescing/control-flow/call/access/method-call output with input substitution and hygienic generated local/pattern bindings, expression AST input kind branching through compile-time-only `input.kind()` / `ast_kind(input)` and `input.is("kind")` / `ast_is(input, "kind")` conditions, item-position `ast -> ast` declaration output, declaration input substitution, structured declaration input inspection through `decl!(...)`, declaration-member shape/name/type summary branching, and dynamic declaration identifier construction through `meta_ident!(...)`, pattern-position `ast -> ast` pattern output and pattern input substitution through `pattern!(...)`, balanced user attribute token-tree arguments, parser-stable `ident!(...)` expression, item, type, and pattern macro token-tree syntax, expression-position `token_stream`/`ast` macro identity expansion from parsed expression input, expression/item/pattern-position `token_stream -> token_stream` output through `tokens!(...)` and empty/count/boundary/wrapper/indexed-text token branching, one-token-wildcard and named-span pattern branching, plus end-exclusive slice, named token/span capture extraction, and `~name` capture splicing in token output, function/constant/struct/enum/trait/impl/inline-module/use item macro identity expansion from parsed top-level declarations, type-position `type -> type` macro identity expansion from parsed type input, type-position `type -> type` output and type input substitution through `type!(...)`, pattern-position `token_stream`/`ast` macro identity expansion from parsed pattern input, AST-summary preservation for full attribute token payloads and pattern macro payloads, sema validation for active expression/item/type/pattern macro names/domains | unknown attributes, mismatched attribute token-tree delimiters, bad attribute placement, unsupported or duplicate derives, enum `Default` derives without a case marker, enum `Default` derives with a missing case marker, struct `Default` derives with an enum case marker, bad `repr`, `@repr(C)` owning fields or enum payloads, generic payload-bearing `@repr(C)` enums, oversized or non-Unix by-value C-header aggregates, bad `@cfg` predicate, bad deprecated/export arguments, bad export symbol collisions, bad `@test` signature, bad meta signatures, generic meta functions, empty `type!(...)` type constructors, non-token and unsupported AST meta bodies before the evaluator exists, non-input name references in AST meta expression returns, expression AST value helpers outside ast meta if conditions, declaration-returning AST meta functions at expression/pattern/type macro sites, pattern-returning AST meta functions outside pattern macro sites, invalid `meta_ident!(...)` generated identifiers, generated meta functions from item or attribute macros, nested item macro outputs, file-backed module imports from item/attribute macros, `type -> type` meta functions rejected at attribute/expression-macro/item-macro/pattern-macro sites, non-`type -> type` meta functions rejected at type-macro sites, malformed expression/item/type/pattern macro inputs, unknown expression/item/type/pattern macros, planned macro invocation | parser pruning, sema validation, derive impl expansion, expression/item/type/pattern macro input parsing/lowering, warning emission, LLVM test runner, LLVM/shared symbol selection, C header emitter | good first pass |
| Front-end surfaces | structs, traits, trait generics, `dyn Trait[...]` trait-object type syntax, associated type declarations `trait T { type Item }`, impl witnesses `type Item = T`, and projection syntax `Trait[T]::Item` in type positions including unique inherited generic-supertrait associated types, explicit `value as dyn Trait[...]` impl checks, concrete and generic-impl-specialized copyable LLVM dyn dispatch, borrowed `ref dyn` / `ref mut dyn` dispatch and supertrait upcasts with preserved borrow provenance, `own dyn Trait[...]` construction from zone-tracked pointers plus owning supertrait upcasts, impl conformance, concrete method dispatch, generic function trait bounds, constrained static dispatch including same-name inherent/trait receiver selection, trait-backed comparison operators over `eq` and `lt`, generic source `std::cmp` `Ordering`/`compare`/`then_compare`/`min`/`max`/`clamp`/`is_between` helpers over `cmp::Ord[T]`, generics, meta syntax, Rust-like prelude trait names and ADTs, required `Drop::drop` method | removed class/interface syntax, unknown traits and trait bounds, invalid trait object arity/qualifiers, missing/extra/duplicate associated type witnesses, ambiguous direct or inherited associated type projections, implicit concrete-to-dyn assignment, missing dyn-conversion impls, unrelated dyn-to-dyn upcasts, bad borrowed dyn source mode, non-zone `own dyn` sources, non-object-safe generic trait methods under dyn dispatch, duplicate impls, missing/mismatched trait methods, missing impls for generic trait bounds, unknown/ambiguous method calls, planned aggregate destructuring syntax, owned prelude ADTs | parser/sema validation, associated type projection lowering through unique impl witnesses and unique generic supertrait projection targets, static method-call lowering, constrained generic method selection, LLVM dyn vtable globals and LLVM vtables with erased receiver thunks for concrete and generic impls, dyn multi-argument calls, aggregate argument views, borrowed dyn original-address data pointers, supertrait upcasts, own dyn three-word owner objects and erased drop thunks, source `std::cmp` helper monomorphization on LLVM | partial |
| Unsupported aggregates | vector type checking, list literal constant indexing, fixed-size array surface, non-local aggregate ABI gaps | backend rejection, vector index bounds, non-local aggregate ABI rejection | clear diagnostics | partial |

Note: runtime-dependent owner enum payload moves are covered for tracked
local/parameter subjects in statement and expression `match`, `if let`, and
`while let` arms. Uniform owner layouts also support direct payload-slot moves
outside narrowing control flow. The remaining owner-path gaps are dynamic
indexed owner element moves and broader non-static runtime sequence owner paths
beyond the supported Vec/Slice pattern cases.

Process coverage note: `std-process-command.ari` covers the first
`std::process::Command`/`Child` slice on the hosted POSIX path: argv wrappers,
child environment setup, child working-directory setup, `spawn`, `status`,
`exec` lowering, child wait, and non-destructive `kill(0)` probes.
`std-process-output.ari` covers small `Command::output_in` stdout/stderr
capture, `Output` accessors, missing-command status behavior, `pipe`/`dup2`
lowering, and compiler zone provenance for the zone-backed `Output` handle.
`std-process-exit-status.ari` covers typed `ExitStatus` values,
`Command::exit_status`, `Child::wait_status`, normal exit-code access, signal
termination access, and compatibility `status`. `std-process-high-level.ari`
covers explicit-zone `Command::arg`/`env_var`, typed `ExitCode`, typed
`Signal`, child stream endpoint aliases, current/executable path wrappers, and
temp file/temp dir constructors. Remaining process runtime matrix work is
large-output readiness or nonblocking draining, stdin redirection,
richer platform status fields, and non-POSIX mapping.

## Completed Sprint: C FFI

Goal: C FFI must be boring and reliable. Ari supports only C ABI for foreign
libraries at the source level; C++ interop should go through explicit C wrapper
functions. Ari-owned runtime hooks use the separate reserved `extern "ari"`
ABI and validated `ari_builtin_*` symbols.

Checklist:

- [x] `extern "C" fn puts(...)`
- [x] `extern fn puts(...)` shorthand
- [x] `extern "C" fn local(...) = "external_symbol"`
- [x] extern declarations inside modules
- [x] generated LLVM IR declares external C symbols with explicit link names
- [x] external numeric arguments and return values
- [x] external string arguments
- [x] external `ref mut` argument lowered as pointer
- [x] nullable raw-pointer literals for `ptr T`
- [x] nullable raw-pointer type suffix `T?`
- [x] raw-pointer casts lower to LLVM `ptrtoint`/`inttoptr`
- [x] byte-wise `ptr_offset` lowers to LLVM `getelementptr i8` and LLVM address arithmetic
- [x] typed scalar `ptr_add` lowers to LLVM typed `getelementptr` and LLVM typed address arithmetic
- [x] typed aggregate `ptr_add` lowers to LLVM aggregate `getelementptr`
- [x] `size_of<T>()` and `align_of<T>()` expose scalar and Ari-layout aggregate byte counts
- [x] sema, IR helpers, and LLVM aggregate field lists/offsets use the shared Ari layout service
- [x] scalar `ptr_load`/`ptr_store` lower on LLVM
- [x] plain aggregate `ptr_load`/`ptr_store` and `*pointer` whole-copy lower on LLVM
- [x] `std::mem::copy_bytes`/`move_bytes`/`set_bytes` lower through LLVM
      memory intrinsics, and `std::mem::page_size` lowers through the hosted
      runtime page-size hook
- [x] scalar field/element access through raw aggregate pointers lowers on LLVM
- [x] C variadic extern declarations, default promotions, calls, and function-pointer rejection
- [x] C callback/function-pointer parameters lower as `ptr` and accept Ari function names
- [x] reserved `extern "ari"` builtin declarations for the source `std` header
- [x] `extern "ari"` declarations lower through a distinct IR ABI and do not
      emit foreign C declarations for builtin hooks
- [x] `extern "ari"` declarations validate builtin arity, parameter types, and
      return type against compiler-owned metadata
- [x] `--emit-obj` emits direct scalar/raw-pointer
      `extern "C"` calls as ELF relocations, while LLVM executable output
      rejects unresolved imported C symbols instead of treating them as missing
      Ari symbols
- [x] reject `extern "C++"`
- [x] reject extern functions with bodies
- [x] reject generic extern declarations as a permanent concrete-C-symbol policy
- [x] reject invalid external link symbols

## Completed Sprint: CLI And Build Modes

Goal: compiler invocation should fail early and predictably, and every output
mode should leave an artifact that can be checked by exit code, stdout, or file
presence.

Checklist:

- [x] compile a normal glibc-backed executable with `-o`
- [x] execute the normal output and check its exit code
- [x] emit generated LLVM IR with `--emit-llvm`
- [x] inspect generated LLVM IR for Ari `main` and host wrapper
- [x] run parser/module/sema diagnostics without backend output using `--check`
- [x] reject removed generated-C++ options
- [x] compile and run a Linux executable with the LLVM backend
- [x] LLVM output is executable without a test-side `chmod +x`
- [x] emit LLVM-driver relocatable objects with
      `--emit-obj`
- [x] inspect LLVM object `.text`, `.symtab`, mangled Ari symbols, and
      `@export`/`@no_mangle` symbol names
- [x] inspect LLVM object `.rela.text` imported C call relocations and link the
      object with a small C helper
- [x] inspect LLVM object aggregate export symbols with an unresolved
      `extern "C"` helper relocation
- [x] emit LLVM objects as PIC library artifacts with shared-library visibility
      rules, so private Ari helpers and Ari-owned runtime helpers are hidden
- [x] emit matching C headers and LLVM objects in one invocation, including
      generated tuple/enum/vector C wrapper types and explicit export symbols
- [x] emit LLVM objects after module-cache IR replay and inspect cached generic
      dependency body symbols in the resulting object
- [x] compile a shared library with `--shared`
- [x] emit hidden LLVM visibility for private Ari helpers under `--shared`
- [x] inspect linked shared-library dynamic symbols for public/exported-only
      user ABI surface
- [x] reject missing input
- [x] reject missing `-o` path
- [x] reject missing `--emit-llvm` path
- [x] reject missing `--module-path` and `-I` paths
- [x] reject missing `--llvm-cc` compiler path
- [x] reject removed `--backend`
- [x] reject removed `--cc`
- [x] reject the LLVM backend combined with LLVM output options
- [x] reject unexpected extra input arguments

## Completed Sprint: Functions

Goal: function calls should be trustworthy before widening the language around
methods, trait-bound dispatch, generics, and richer patterns.

Checklist:

- [x] require executable `main`
- [x] reject `main` parameters
- [x] require `main -> i64`
- [x] require non-void functions to return on all paths
- [x] check return expression type against the function return type
- [x] accept functions with omitted return type as `void`
- [x] accept bare `return` in void functions
- [x] reject value returns from void functions
- [x] check function argument count
- [x] check function argument types
- [x] support direct recursion on LLVM IR backend
- [x] support direct recursion on LLVM backend
- [x] support LLVM calls beyond six arguments with stack slots
- [x] confirm seven-argument calls work
- [x] support LLVM direct function returns for tuple, struct,
      fixed-array, and aggregate enum values through hidden result pointers
- [x] support LLVM direct and function-pointer call parameters and
      returns for tuple, struct, fixed-array, and aggregate enum values through
      hidden argument/result pointers copied into callee-local storage or
      caller-provided result storage
- [x] store plain tuple, fixed-array, and struct values inline in aggregate
      enum payload slots
- [x] store explicit fixed-capacity `Vec[T; N]` values inline in aggregate
      enum payload slots
- [x] bind plain aggregate enum payload slots through tuple, fixed-array, and
      struct payload subpatterns
- [x] bind explicit fixed-capacity `Vec[T; N]` aggregate enum payload slots
      through exact array-style element subpatterns with a runtime length check
- [x] bind and explicitly drop `own i64`/`own u64` aggregate enum payloads from
      direct temporary constructor matches
- [x] seed active `own i64`/`own u64` aggregate enum payload owner paths for
      direct constructor locals and whole-local assignments, including inactive
      fieldless cases plus stored-case payload move/drop lowering
- [x] drop runtime-dependent aggregate enum locals, parameters, and call/return
      results by testing the runtime tag and cleaning only the active
      owner-carrying payload slots
- [x] bind and explicitly drop tracked runtime-dependent statement `match`
      `own i64`/`own u64` aggregate enum payloads from local and parameter
      subjects
- [x] move runtime-dependent aggregate enum owner payload slots outside
      statement `match` when every case has the same owner payload paths
- [x] move tag-conditioned runtime-dependent aggregate enum owner payload slots
      after statement `match`, expression `match`, `if let`, and `while let`
      seed the active case
- [x] merge moved-or-dropped owner payload states across statement `match` arms
      and skip consumed payloads during later tag-guarded whole-value cleanup
- [x] reject `@repr(C)` enum payload slots that contain `own` so public C-facing
      layouts expose ownership through explicit pointer or borrow wrappers
- [x] materialize discarded LLVM aggregate-returning direct and
      function-pointer calls into hidden temporaries
- [x] preserve LLVM caller pointer bases while aggregate-valued
      block/if/match results evaluate aggregate-returning callees
- [x] accept generic function syntax as a parsed surface
- [x] monomorphize simple generic function calls
- [x] monomorphize explicit generic function calls such as `identity<i64>(value)`
- [x] specialize generic function names into function pointer values from expected `fn(...) -> ...` types
- [x] lower explicit generic struct literals and tuple-struct constructors
- [x] infer generic struct literal and tuple-struct constructor type arguments from field/argument values
- [x] specialize generic inherent impl methods with `self` receivers
- [x] specialize generic trait impl methods with `self` receivers
- [x] enforce generic impl bounds at specialization sites
- [x] specialize generic inherent associated functions with explicit associated call type arguments
- [x] reject overlapping concrete and generic trait impls
- [x] specialize method-level generic inherent impl methods with explicit method call type arguments
- [x] infer method-level generic inherent impl arguments from call arguments
- [x] specialize method-level generic inherent associated functions with explicit or inferred call arguments
- [x] emit mangled function symbols in LLVM IR and LLVM object `.symtab`
- [x] reject argument-based source-level function overloading by arity or
      parameter type
- [x] allow explicit generic companions such as `alloc<T>(...)` beside a
      concrete same-path function
- [x] lower function parameter patterns at function entry
- [x] lower local/function-entry reference pattern binding modes and document
      the remaining nested control-flow limits
- [x] call `extern "C"` functions from normal Ari functions
- [x] expose public module functions
- [x] hide private module functions from outside modules

## Completed Sprint: Control Flow

Goal: loops and branches should be easy to trust before the language grows more
syntax around iterators and pattern-driven control flow.

Checklist:

- [x] `while` with `continue`
- [x] `while` with `break`
- [x] `if` / `else if` / `else` branch chains
- [x] expression-valued `if` / `else if` / `else` with branch type unification
- [x] expression-valued `if let` with enum payload binding
- [x] aggregate-valued `if`, `match`, and block expression results on the
      LLVM backend
- [x] integer-to-bool condition coercion as `value != 0`
- [x] `if let EnumCase(payload) = value`
- [x] `while let EnumCase(payload) = value`
- [x] `for value in range(start, end)`
- [x] `for value in start..end`
- [x] `for value in start..=end`
- [x] `for value in range_inclusive(start, end)`
- [x] integer `Range[T]` and `RangeInclusive[T]` local values with field and tuple-index access
- [x] `for value in stored_range`
- [x] irrefutable alias loop heads for range calls, range operators, and stored range values
- [x] `for _ in range(start, end)` wildcard pattern
- [x] empty ranges do not run the loop body
- [x] `iter::range(start, end)` qualified prelude form
- [x] `for value in [a, b, c]` list literal iteration
- [x] `for value in stored_vec` local vector iteration on the LLVM backend,
      using the current vector length rather than reserved capacity
- [x] `for _ in [a, b, c]` list literal wildcard pattern
- [x] irrefutable tuple, named struct, tuple-struct, and alias loop heads for list literal iteration
- [x] irrefutable tuple, named struct, tuple-struct, and alias loop heads for stored local vector iteration
- [x] list literal `for` with `break` and `continue`
- [x] stack-backed local `Vec[T]` literals on the LLVM backend
- [x] typed empty local `Vec[T]` literals through annotations and assignments
- [x] checked dynamic indexing of stored local `Vec[T]`, including
      compiler-known current-length static index diagnostics and known-empty
      dynamic index rejection, plus direct control-flow Vec expression static
      index diagnostics from source-known local branch lengths
- [x] local `Vec[T]` literal reassignment with changing runtime length
- [x] local `Vec[T]` initialization and assignment from another local vector
      preserve compiler-known current length when the source length is precise
- [x] local `Vec[T]` assignment from another local vector widens the target's
      fixed local storage to the source storage capacity
- [x] local `Vec[T]` assignment/initialization from Vec-valued `if`, block,
      and labeled-block expressions preserves nested fixed storage capacity and
      same-length literal, block, typed-break, or local-binding known length
- [x] local `Vec[T]` initialization from Vec-valued `match` and `if let`
      expressions preserves fixed storage capacity and same-length literal,
      block, labeled-block, typed-break, or local-binding known length across
      branch result materialization
- [x] `len(value)` and `value.len()` for local vectors, fixed arrays,
      constant-folded literal length, and compiler-known local Vec length,
      including local Vec `as_slice()` view length construction and
      source-known Vec control-flow expression result lengths
- [x] fixed-capacity local literal/const/static-signed-expr/known-local/runtime-checked `Vec.reserve(n)`, plus auto-widening local `Vec.push(value)` on the LLVM backend
- [x] checked local `Vec.insert(index, value)` on the LLVM backend
- [x] checked local `Vec.pop()` on the LLVM backend
- [x] checked local `Vec.remove(index)` on the LLVM backend, including
      known-empty runtime-index rejection
- [x] checked local `Vec.first()` and `Vec.last()` on the LLVM backend
- [x] checked local `Vec.get(index)` on the LLVM backend, including known-empty
      runtime-index rejection
- [x] checked local `Vec.contains(value)`, `Vec.index_of(value)`, and `Vec.count(value)` on the LLVM backend
- [x] fixed-capacity local `Vec.capacity()` on the LLVM backend
- [x] `value.is_empty()` for local vectors, fixed arrays, constant-folded
      literals, and compiler-known local Vec length
- [x] fixed-capacity local `Vec.clear()` on the LLVM backend
- [x] fixed-capacity local `Vec.truncate(n)` on the LLVM backend, including static/known-local length tracking for later local growth
- [x] checked local `Vec.set(index, value)` on the LLVM backend, including
      known-empty runtime-index rejection
- [x] checked local `Vec.swap(a, b)` on the LLVM backend, including known-empty
      runtime-index rejection
- [x] frozen local `Vec[T]` API rejects unsupported compiler-known methods with allocator-backed std guidance
- [x] root `Vec[T]` direct/function-pointer parameters, trait/impl method
      parameter slots, and impl receivers lower through the Slice-shaped view
      ABI. Returns, extern parameters/returns, trait method return types, and
      direct or generic-instantiated struct fields stay rejected by the
      local/view-only policy; dyn dispatch erases trait method parameters to the
      same view ABI.
- [x] `std::vec::alloc_buffer<T>(ref mut Zone, capacity)` seeds future
      allocator-backed Vec storage with a tracked zone-backed element pointer;
      source `std::vec::Vec<T>` drops live elements on handle Drop and now
      drops overwritten or shrink-removed values through the shared shrink
      helper used by `set`, `clear`, `truncate`, and shrinking `resize_in`
- [x] expose `std::Vec[T]` as the root explicit-zone alias for
      `std::vec::Vec[T]`, including `Vec!` constructor sugar and
      single-zone-return source handles passed through ordinary function
      parameters
- [x] `std::boxed::new<T>(ref mut Zone, value)` wraps placement construction in
      a tracked source `std::boxed::Box<T>` handle with
      `get`/`set`/`replace`/`take`/`try_take`/`clear`/`put_in`/`is_empty`/
      `copy_to`/`as_ref`/`as_mut`/`swap`/`as_ptr`/`as_mut_ptr` methods,
      borrowed receiver lowering for read-only `get`/`copy_to`/`as_ref`/
      `as_ptr` and mutable-borrowed receiver lowering for `as_mut`/
      `as_mut_ptr`,
      value-drop overwrite through `set`, an empty-handle state after `take`,
      Option-returning empty-handle move-out through `try_take`, same-zone
      empty-handle refill through `put_in(ref mut Zone, value)`, no-op clear for empty handles, and
      reset/destroy invalidation, plus a generic Drop impl that consumes the
      handle binding, skips empty handles, runs `Drop` for the stored value when
      one exists, and rejects later use
- [x] expose root `Box[T]` / `std::Box[T]` as aliases for the explicit-zone
      source `std::boxed::Box<T>` handle, including `Box::new<T>` /
      `std::Box::new<T>` associated construction and
      `Box!(T, ref mut Zone, value)` constructor sugar with reset/destroy
      provenance, while leaving allocator-backed unique ownership for the
      smart-pointer roadmap
- [x] `std::string::alloc_buffer(ref mut Zone, capacity)` seeds future owned
      string storage with tracked zone-backed `ptr u8` byte allocation and
      reset/destroy invalidation
- [x] `std::string::with_capacity(ref mut Zone, capacity)` and
      `std::string::new(ref mut Zone, capacity)` wrap byte storage in tracked
      `RawString` / source `std::string::String` handles with backing metadata,
      borrowed receiver lowering for metadata, byte reads, and `as_ptr`,
      fixed-capacity byte operations, `as_slice`, top-level
      `std::string::copy_to(ref value, ref mut Zone)`, and no-op Drop handling
- [x] `std::string::from_string(ref mut Zone, string)` copies today's borrowed
      lowercase `string` into independent source `std::string::String` storage
      tied to the target zone
- [x] `std::string::from_slice_in(ref mut Zone, Slice[u8])` copies a borrowed
      byte slice into independent source `std::string::String` storage tied to
      the target zone
- [x] `std::string::from(ref mut Zone, string)`, `copy(ref mut Zone, Slice[u8])`,
      and `empty(ref mut Zone)` provide the natural constructor spellings for
      everyday string code while preserving the explicit target zone
- [x] byte character literals such as `'c'`, `'\n'`, and `'\x63'` lower as
      `u8`, and `std::string::bytes(string)` exposes borrowed literal bytes as
      `Slice[u8]` without the trailing NUL
- [x] source `std::string::String` exposes natural append helpers
      `append`, `append_byte`, and `append_bytes` so literal text, one byte, and
      borrowed byte views do not need the lower-level `_in` names in normal code
- [x] source `std::string::String` exposes borrowed-receiver endpoint byte
      reads through `first` and `last`
- [x] source `std::string::String` exposes `try_first`, `try_last`,
      `try_get(index)`, and `try_pop()` as `Option[u8]` helpers for empty or
      out-of-range byte access
- [x] source `std::string::String` exposes byte-search methods `contains`,
      `index_of`, and `count` with borrowed receiver lowering
- [x] source `std::string::String` exposes `starts_with(Slice[u8])` and
      `ends_with(Slice[u8])` prefix/suffix checks over borrowed byte views
- [x] source `std::string::String.equals(Slice[u8])` checks exact byte-view
      equality with borrowed receiver lowering
- [x] source `std::string::String` exposes literal-oriented
      `find_text`, `contains_text`, `starts_with_text`, `ends_with_text`, and
      `equals_text` wrappers so most text checks can accept Ari string literals
      directly
- [x] source `std::string::String` exposes borrowed ASCII `trim_start`,
      `trim_end`, and `trim` views plus whole-string `parse_decimal` and
      `parse_hex` helpers through the `std::ascii` source policy
- [x] source `std::string::String` exposes `parse_decimal_prefix` and
      `parse_hex_prefix` helpers through the `std::ascii::ParsedInt` source
      policy
- [x] source `std::string::String` exposes ASCII-only
      `equals_ignore_case`, `starts_with_ignore_case`,
      `ends_with_ignore_case`, `index_of_ignore_case`, and
      `contains_ignore_case` helpers through the `std::ascii` source policy
- [x] source `std::string::String` exposes `_text_ignore_case` wrappers for the
      ASCII case-insensitive comparison/search helpers
- [x] source `std::string::String` exposes UTF-8 `is_utf8`,
      `codepoint_count`, `codepoint_at`, and `push_codepoint_in` helpers
      through the `std::encoding` source policy
- [x] source `std::string::String.try_utf8()` returns the typed
      `std::Option[std::string::Utf8]` view directly when bytes are valid UTF-8
- [x] source `std::string::String` exposes `trim_start_to`,
      `trim_end_to`, and `trim_to` owned ASCII trim copies that remain valid
      after the source zone is reset, while target-zone reset invalidates the
      copied handle
- [x] source `std::string::String` exposes natural owned trim aliases
      `trimmed_start`, `trimmed_end`, and `trimmed`
- [x] source `std::ascii` exposes byte classification predicates for
      alphanumeric, blank, control, printable, graphic, punctuation,
      whitespace, and hexadecimal parser code
- [x] source `std::ascii` exposes borrowed-slice
      `equals_ignore_case`, `starts_with_ignore_case`, and
      `ends_with_ignore_case` helpers with ASCII-only case folding
- [x] source `std::ascii` exposes borrowed-slice `index_of_ignore_case` and
      `contains_ignore_case` helpers with ASCII-only first-match search
- [x] source `std::ascii` exposes `ParsedInt`, `parse_decimal_prefix`, and
      `parse_hex_prefix` for leading digit runs with consumed-byte counts
- [x] source `std::ascii` decimal, signed decimal, and hexadecimal parsers
      reject `i64` overflow without wrapping, including exact min/max boundary
      coverage
- [x] source `std::parse` exposes ASCII-trimmed whole-input `integer`,
      `integer_radix`, `unsigned`, `unsigned_radix`, `hex_integer`,
      `binary_integer`, `octal_integer`, `boolean`, `is_float`, `float_or`,
      `float`, and trait-backed `parse<T>`/`parse_or<T>`/`is_parse<T>`
      helpers with natural names and focused invalid-input coverage
- [x] source `std::encoding` exposes ASCII, UTF-8, and UTF-16 validation plus
      hex/base64 length, encode, decode, and invalid-input guard helpers
- [x] source `std::path` exposes POSIX-style lexical path separator,
      component, component-iterator, component-aware prefix/suffix, join, and
      lightweight normalization helpers
- [x] source `std::bits` exposes `leading_ones` and `trailing_ones` as
      complement pairs to the zero-run bit-scan helpers, including `0u64` and
      `~0u64` edge cases
- [x] source `std::bits` exposes checked and wrapping alignment helpers with
      invalid-alignment and `u64` overflow policy tests
- [x] source `std::bits` exposes `byte_swap` and `population_count` with
      focused byte-order and alias tests
- [x] source `std::math` exposes natural sign predicate names
      `is_positive`, `is_negative`, and `is_zero` without type suffixes, while
      keeping the current concrete `i64` signatures
- [x] source `std::math` exposes checked add/sub/mul/div/rem/neg/abs and
      saturating add/sub/mul/div/neg/abs helpers
      with `Option[i64]` overflow reporting and edge-case saturation behavior
- [x] source `std::math` exposes wrapping add/sub/mul and overflowing
      add/sub/mul tuple-result helpers without adding public type suffixes
- [x] runtime-backed `std::thread` exposes function-pointer spawn/join,
      cooperative yield, duration sleep, hosted available parallelism, runtime
      thread ids, invalid-handle checks, advisory completion, `Builder`, and
      source handle wrappers
- [x] source `std::thread` exposes explicit zone-backed `ThreadLocal[T]`
      handles keyed by Ari runtime thread id
- [x] source `std::sync` exposes primitive `RwLock` read/write lock helpers,
      reader-count diagnostics, root aliasing, and atomic compare-exchange /
      fetch-add-backed lowering
- [x] source `std::sync` exposes explicit `Ordering`, `AtomicBool`,
      `AtomicUsize`, `AtomicPtr`, `OnceLock`, `Condvar`, `Barrier`, and a
      single-slot MPSC channel shape without redundant zone handles
- [x] source `std::string::String.copy_to(ref mut Zone)` mirrors the
      top-level target-zone copy helper while preserving reset/destroy
      provenance on the returned handle
- [x] source `std::string::String` same-zone `reserve`, `reserve_extra`,
      `push_in`, `insert_in`, `extend_from_slice_in`, and `resize_in` grow the
      zone-backed byte buffer through one private capacity/copy path while
      rejecting a mismatched zone argument
- [x] source `std::string::String` same-zone `append_string_in`,
      `append_i64_in`, `append_u64_in`, `append_bool_in`, `append_f32_in`,
      `append_f64_in`, generic `append_value_in[T: std::fmt::Display]`, and
      generic `append_debug_in[T: std::fmt::Debug]` build explicit-zone owned
      text through that same private byte-append growth path
- [x] tracked source `std::string::String` receiver locals infer the same
      source zone for `push`, `insert`, one-argument `reserve`/`reserve_extra`,
      `extend_from_slice`, `resize`, and the non-`_in` append helpers including
      `append_value(value)` and `append_debug(value)`, while untracked
      receivers get a targeted diagnostic instead of a hidden
      allocation capability
- [x] expose the source `std::string::String` handle as root `String` and
      `std::String` while preserving zone reset/destroy invalidation
- [x] `std::vec::with_capacity<T>(ref mut Zone, capacity)` wraps that pointer
      in a tracked `RawVec<T>` handle whose field access is invalidated after
      zone reset/destroy
- [x] `std::vec::new<T>(ref mut Zone, capacity)` wraps `RawVec<T>` in the
      tracked source `std::vec::Vec<T>` allocator/capability creation surface
- [x] `Vec!(T, ref mut Zone, capacity)` lowers to that source
      `std::vec::new<T>` constructor without changing zone provenance
- [x] zone pointer reset invalidation also recognizes `zone::reset` through a
      named `ref mut Zone` binding
- [x] source `std::zone::alloc_array<T>` exposes typed raw buffer allocation
      through `count * size_of<T>()` and `align_of<T>()`, including the root
      `alloc_array` alias and zero-count null behavior
- [x] source `std::zone::ZoneMetadata` and `std::zone::ZoneBacked` expose
      allocation-header zone recovery through `zone::metadata(data)`,
      `zone::from_zone(ref mut zone)`, `metadata.alloc_array<T>(count)`,
      `metadata.as_zone_ptr()`, `zone::of(ref value)`, and `value.zone()` for
      backed std handles, including map update-entry handles
- [x] control-flow expressions that select source `std::vec::Vec<T>` handles
      from the same zone keep reset/destroy provenance on the selected handle
- [x] source `std::vec::Vec<T>` exposes tracked read-only metadata methods
      `len`, `capacity`, and `is_empty` with borrowed receiver lowering
- [x] source `std::vec::Vec<T>` exposes fixed-capacity checked read/write,
      replace, push/pop, insert/remove, swap, truncate/clear, and
      linear-search methods over its zone-backed buffer, with borrowed receiver
      lowering for read/search methods and value-drop removal for overwrite or
      shrink operations
- [x] source `std::vec::Vec<T>` exposes `try_first`, `try_last`, and
      `try_get(index)` as `Option<T>`-returning read helpers for empty or
      out-of-range access
- [x] source `std::vec::Vec<T>` exposes `get_ref(index)` and `get_mut(index)`
      element borrow views through tracked pointer-offset borrows, with
      conservative receiver borrow conflict tracking
- [x] source `std::vec::Vec<T>` exposes `equals(Slice<T>)`,
      `starts_with(Slice<T>)`, and `ends_with(Slice<T>)` over borrowed
      Slice views with borrowed receiver lowering
- [x] source `std::vec::Vec<T>` exposes in-place `reverse`,
      `rotate_left(count)`, and `rotate_right(count)` wrappers over its live
      element storage
- [x] source `std::vec::Vec<T>` exposes `Ord[T]`-bounded introsort-backed
      `sort`, merge-sort-backed `stable_sort`, explicit-zone and `Result`
      stable-sort wrappers, `is_sorted`, `binary_search`, `min`, and `max`
      wrappers over its live element storage
- [x] source `std::vec::Vec<T>.reserve(capacity)` grows the handle with a
      larger allocation from the backing allocation's recovered `ZoneMetadata`
- [x] source `std::vec::Vec<T>.reserve_extra(additional)` grows capacity to at
      least `len + additional` through the recovered backing zone metadata
- [x] source `std::vec::Vec<T>.push_in(ref mut Zone, value)` appends through the
      explicit compatibility capability and grows capacity on demand
- [x] source `std::vec::Vec<T>.insert_in(ref mut Zone, index, value)` inserts
      through the explicit compatibility capability and grows capacity on
      demand
- [x] source `std::vec::Vec<T>.extend_from_slice_in(ref mut Zone, Slice<T>)`
      appends a slice through the explicit compatibility capability
- [x] `std::vec::from_slice_in<T>(ref mut Zone, Slice<T>)` copies a borrowed
      slice into a new target-zone source `Vec<T>` handle
- [x] source `std::vec::Vec<T>.resize_in(ref mut Zone, length, value)` shrinks
      by dropping removed tail values or grows through the explicit
      compatibility capability
- [x] source `std::vec::Vec<T>.resize_with(length, make_value)` grows through
      recovered backing-zone metadata and calls the zero-argument maker once
      per new slot, covered by `std-vec-resize-with`
- [x] source `std::vec::Vec<T>.try_pop()` returns `Option<T>` for empty-aware
      last-element move-out without an assertion
- [x] source `std::vec::Vec<T>` owning-zone `push`, `insert`, `reserve`,
      `reserve_extra`, `extend_from_slice`, `resize`, and `resize_with` share
      one private capacity/copy growth path, covered by `std-vec-growth-paths`
      and `std-vec-resize-with`
- [x] source `std::vec::Vec<T>` recovers `ZoneMetadata` from its backing
      allocation header so natural growth calls work without
      compiler-synthesized zone arguments
- [x] source `std::vec::Vec<T>.as_slice()` returns a mutable `Slice<T>` view
      whose zone provenance is invalidated after reset/destroy
- [x] source `std::vec::Vec<T>.copy_to(ref mut Zone)` copies the current
      elements through a borrowed receiver into a new target-zone handle and
      invalidates with that target zone after reset/destroy
- [x] source `std::vec::Vec<T>.as_ptr()` returns the stored element pointer
      through a borrowed receiver while preserving receiver-zone reset/destroy
      provenance
- [x] source `std::vec::Vec<T>.as_mut_ptr()` returns the stored element pointer
      through a mutable borrowed receiver while preserving receiver-zone
      reset/destroy provenance
- [x] source `std::vec::Vec<T>.iter()` returns a tracked
      `std::vec::Iter<T>` that implements `Iterator[T]`, and
      `std::vec::Vec<T>` implements `IntoIterator[T]` for direct
      `for value in vec` lowering while preserving receiver-zone
      reset/destroy provenance
- [x] root `Slice<T>.iter()` returns a `SliceIter<T>` value cursor, and
      `Slice<T>.iter_mut()` / `std::vec::Vec<T>.iter_mut()` return
      `SliceIterMut<T>` mutable value cursors with `value()` /
      `value_mut()` handles until reference-valued iterator items land
- [x] source `std::iter` adapters cover finite `empty`/`once` sources,
      generator-backed `repeat_with`, lazy `map`, `filter`, `take`, `skip`,
      `enumerate`, and `zip` plus eager `count`, `count_if`, `nth`, `last`,
      `find_if`, `position`, `any`, `all`, `fold`, `reduce`, and
      explicit-zone `collect` over `std::vec::Iter<T>`, `OnceIter<T>`,
      `EmptyIter<T>`, `RepeatWith<T>`, and root `SliceIter<T>`
- [x] source `std::vec::Vec<T>` and root `Slice<T>` expose half-open range
      mutation helpers for overlap-safe `copy_within`, `fill_range`,
      `reverse_range`, and left-rotation `rotate_range`; source Vec also
      exposes `extend_iter` and `drain_range`
- [x] source `std::vec::Vec<T>` covers the practical convenience set with
      `try_reserve`, `shrink_to_fit`, `drain`, `splice`, `split_off`,
      `append`, slice and iterator extension, `swap_remove`, `dedup_by`, and
      `dedup_by_key`, covered by `std-vec-complete-convenience-api`
- [x] source `std::algo` covers production-shaped sort engines with
      insertion-sort cutoff, duplicate-heavy 3-way partitioning, heapsort
      fallback symbols, merge-sort stable sort, explicit temporary-zone stable
      sort, `try_stable_sort*` result wrappers, large 10,000-element sorted,
      reverse, random, and duplicate-heavy cases, and stable duplicate-key
      ordering
- [x] source `std::vec::Vec<T>` has a value-drop handle `Drop` impl that drops
      each current element while leaving storage release to the explicit zone,
      with shared shrink/drop lowering for `set`, `clear`, `truncate`, and
      shrinking `resize_in`
- [x] source `std::collections::Set<T>` exposes a tracked linear
      insertion-order set with `new`, `Set::new`, `from_slice_in`, metadata,
      `first`/`last`/`get`, `try_first`/`try_last`/`try_get`, `contains`,
      `index_of`, `insert`, `replace`, `remove`, `take`, `pop`, `try_pop`,
      `clear`, `retain`, `reserve`, `reserve_extra`, `as_slice`, `iter`,
      and `copy_to`, plus reset/destroy invalidation, iterator invalidation,
      and same-zone growth diagnostics
- [x] source `std::collections::HashMap<K,V>`/`HashSet<T>` and
      `TreeMap<K,V>`/`TreeSet<T>` expose tracked hash-table and red-black-tree
      containers with explicit hash/comparator functions, lookup, insertion,
      replacement, `HashMapEntry`/`TreeMapEntry` update handles with insert,
      `insert_entry`, `or_default`, remove, access, and mutable-value
      borrowing, entry-handle zone recovery from backing-map metadata,
      `remove_entry`, reserve growth, tracked-local zone inference for common
      mutation calls, in-place `HashMap`/`HashSet` retain
      filtering, hash/tree
      iterator invalidation, reset/destroy invalidation, and same-zone growth
      diagnostics
- [x] source `std::collections::Deque<T>`, `RingBuffer<T>`,
      `LinkedList<T>`, `BinaryHeap<T>`, and `PriorityQueue<T>` expose tracked
      queue/list/priority containers with circular growth, bounded overwrite,
      node reuse, comparator-driven pop order, iterator coverage where
      applicable, tracked-local zone inference for growable mutation calls,
      reset/destroy provenance, and same-zone growth checks
- [x] LLVM backend lowers stored local `Vec[T]` literals, local
      copies, scalar indexing, the fixed-capacity method surface, and
      stored-vector `for` loops
- [x] `init ... while ... next ...` normal update path
- [x] reject removed `let ... while ... next ...` loop-state spelling
- [x] `continue value, ...` inside init-while
- [x] labeled loop `break label`
- [x] labeled block `break label`
- [x] LLVM/glibc backend loop lowering
- [x] LLVM loop execution across while, while-let, range-for, vector-for, and init-while
- [x] reject non-bool/non-integer loop conditions
- [x] reject `break` outside loops
- [x] reject unknown or duplicate active loop labels
- [x] reject `continue` outside loops
- [x] reject value `continue` outside init-while
- [x] reject non-`range` for iterables
- [x] lower direct copyable `Iterator[T]` for-loop values through `next`
- [x] lower direct copyable `Iterator[T]` for-loop values whose `next` receiver
      is `self: ref mut Self`
- [x] lower explicit `ref mut` `Iterator[T]` for-loop values by advancing the
      original iterator binding and releasing the hidden borrow after the loop
- [x] lower self-returning copyable `IntoIterator[T]` for-loop values through
      `into_iter` and `next`
- [x] lower distinct-result copyable `IntoIterator[T]` for-loop values when
      `into_iter` returns another `Iterator[T]`
- [x] lower copyable `IntoIterator[T]` for-loop values whose `into_iter`
      receiver is `self: ref mut Self`
- [x] lower owning `Iterator[T]` values by moving them into hidden loop storage,
      calling `next(self: ref mut Self)`, and dropping the hidden owner at loop
      exit
- [x] drop hidden owning iterator storage before `return` exits a function from
      inside the loop
- [x] drop hidden owning iterator storage before postfix `?` residual returns
      from inside the loop
- [x] lower refutable scalar literal/range, or-pattern, fieldless enum-case,
      compact enum payload, homogeneous nested aggregate-enum payload, and
      nested aggregate-enum multi-payload item patterns with nested
      or-pattern slots for `Iterator[T]` loops with stop-on-first-mismatch
      semantics
- [x] lower `for let pattern in iterator` as a filter loop that skips
      non-matching `Some(_)` items
- [x] reject `for let` filter syntax on range/list/vector loops until those
      collections lower through `Iterator[T]`
- [x] reject unsupported iterator for-loop shapes with trait-aware planned
      diagnostics
- [x] reject enum-case list/vector `for` patterns with a planned diagnostic
- [x] reject wrong `range` arity
- [x] reject direct untyped empty `[]` iteration
- [x] reject mixed-type list literal iteration
- [x] reject wrong init-while `next` arity
- [x] reject wrong init-while `continue` arity

## Completed Sprint: Pattern Matching

Goal: enum `match` should be predictable across payload extraction, wildcard
coverage, branch flow, and backend lowering.

Checklist:

- [x] exhaustive enum match with all arms returning
- [x] payload binding in a match arm
- [x] payload ignore with `_`
- [x] multi-payload enum binding and `..` rest payload ignore on LLVM
- [x] wildcard arm covers remaining cases
- [x] match where one arm returns and another continues
- [x] match over module-qualified enum cases
- [x] scalar constant patterns in integer and bool matches
- [x] module-qualified constant patterns in enum payloads
- [x] scalar constant initializer arithmetic, bitwise/shift, bool, and comparison folding
- [x] aggregate constant values for tuples, structs, tuple structs, enums, and fixed arrays
- [x] constant cycle diagnostics report the full dependency path
- [x] fixed-size array binding and match patterns with `..` rest
- [x] runtime `Vec[T]`/`Slice[T]` sequence patterns can bind `name @ ..`
      rest ranges as `Slice[T]`
- [x] expression-valued `match` can bind payloads and produce a scalar result
- [x] expression-valued `match` lowers on LLVM/glibc backends
- [x] LLVM/glibc backend match lowering
- [x] LLVM backend match execution
- [x] reject empty match
- [x] reject non-exhaustive match
- [x] reject missing payload pattern
- [x] reject duplicate enum case arm
- [x] reject unreachable arm after wildcard
- [x] reject payload pattern on no-payload case
- [x] reject case from a different enum
- [x] reject match over non-enum values
- [x] reject incompatible ownership states after match arms
- [x] reject incompatible ownership states after match expression arms

## Completed Sprint: Modules

Goal: module namespaces should stay boring as more language features start to
create names. Public access, aliases, and qualified names should resolve the
same way across functions and enum constructors.

Checklist:

- [x] `mod` creates a qualified namespace
- [x] public module functions are callable with `A::B`
- [x] private module functions are callable from the same module
- [x] private module functions are rejected from outside
- [x] root-scope `use` imports a function
- [x] root-scope `use ... as ...` aliases a function
- [x] root-scope `use A::{x, y as z}` imports selected items
- [x] root-scope `use A::*` imports public constants, functions, enum types, and enum cases
- [x] module path aliases work as qualified prefixes
- [x] module-scope `use ... as ...` stays local to that module
- [x] `pub use` re-exports functions, enum types, and enum cases
- [x] `pub use A::*` re-exports public module surfaces
- [x] file-backed `mod source as Alias;` loads one file under another module path
- [x] nested module functions are callable with full `A::B::C`
- [x] `pub mod` exposes nested module paths outside their parent module
- [x] `self::` and `super::` resolve in item, type, pattern, and `use` paths
- [x] public module enums are usable with qualified enum types
- [x] enum type aliases through `use ... as ...`
- [x] enum constructor aliases through `use ... as ...`
- [x] qualified enum constructors still work beside aliases
- [x] LLVM/glibc backend module name lowering
- [x] LLVM backend module name lowering
- [x] file-backed `mod name;` loads `name.ari`
- [x] header-like `mod name;` loads `name.arih`
- [x] package search paths work with `--module-path`, `-I path`, and `-Ipath`
- [x] repeated file-backed module imports are cached by resolved module name
- [x] compact module metadata can be emitted and read back for source-graph validation
- [x] metadata records stable source content hashes for cache invalidation
- [x] metadata/cache checks accept only the centralized current `v0` headers
- [x] metadata/cache parsing rejects duplicate source, import, item summary,
  while member-origin impl item keys let metadata represent intentional split
  inherent impl blocks for the same type,
      and AST-summary records
- [x] stale module metadata reports changed cfg, source, import, and item records
- [x] source-snapshot module caches can be emitted and reused after validating
      search paths, cfg features, selected target triples, implicit `std`,
      source hashes, and import resolution
- [x] validated source-snapshot module cache use resolves module imports from
      the cached import table instead of rediscovering candidate files
- [x] source-snapshot module cache use rechecks the parsed cached source graph
      against the cache's embedded metadata
- [x] source-snapshot module cache parsing rejects duplicate source snapshot records
- [x] source-snapshot module cache emits and reloads per-source AST summary records
- [x] sema-checked module cache emission writes per-source IR summary sidecar
      records with required lowered body-shape and operand-tree payloads, and
      validates duplicate, version, hash, and function-count mismatches when
      present
- [x] AST-summary cache skip parses matching IR sidecars and rejects lowered
      free-function surfaces that no longer cover summary-loaded executable
      bodies
- [x] IR summary body payload parsing materializes lowered body-shape maps and
      operand-tree statement/expression summaries, then rechecks that the shape
      inventory matches the materialized tree
- [x] validated IR sidecars replay summary-safe dependency `IrFunction` bodies,
      preserve named aggregate/enum/fixed-array and local-Vec capacity type
      shapes, skip cached semantic body lowering, and match fresh cache-emission
      LLVM output byte-for-byte
- [x] IR sidecars replay generic free-function and generated impl-method
      specializations from cached dependencies, validate and preserve their
      origin/type-arg specialization metadata including generic argument names,
      validate trait-vs-inherent impl origin keys when the cached source
      declares that trait method surface, and prevent duplicate lowering when
      the root directly calls cached generic APIs
- [x] IR sidecar cache-use rejects corrupted lowered `impl::...` body call
      targets during module-cache loading instead of letting backend symbol
      lookup fail
- [x] hash-valid but malformed IR sidecar replay payloads are rejected with a
      module-cache replay diagnostic before backend emission
- [x] IR sidecar cache-use replay writes explicit `vector-storage` layout
      descriptors for fixed-capacity `Vec[T; capacity]` type metadata,
      rejects mismatched descriptors, and keeps the resulting LLVM layout
      byte-for-byte identical to fresh lowering
- [x] AST summary records include declaration fingerprints and declaration
      payloads, then reject tampered fingerprints, payloads, or count/payload
      disagreement
- [x] AST summary declaration payloads materialize back into declaration-only
      `Program` records and round-trip before cache use
- [x] AST summary declaration payloads materialize scalar constant
      initializers for header-like cached modules
- [x] AST summary declaration payloads materialize tuple, struct, fixed-array,
      enum, and generic enum constant initializers for header-like cached modules
- [x] AST summary declaration payloads preserve explicit casts plus arithmetic,
      bitwise, unary bitwise-not, and shift constant expressions for
      header-like cached modules
- [x] AST summary declaration payloads preserve field, tuple-index, and
      fixed-array index access over materialized aggregate constants
- [x] cached header-like dependencies can load declaration-safe materialized
      summaries instead of reparsing cached source text
- [x] AST summary declaration payloads preserve function parameter and local
      binding patterns
- [x] AST summary declaration payloads materialize simple executable function
      bodies made from empty unit bodies, local declarations, local binding patterns,
      assignments, `if`/`else`, block/labeled-block statements, loops,
      `continue`, `break`, statement `match`, `drop`, return/final-expression
      statements, method calls, qualified calls with receiver type arguments,
      borrow expressions, pointer dereferences, postfix `?`, `??`, indirect
      function-pointer calls, prelude macro invocations, `if`/block/`match`
      expressions, summary-safe tuple/vector/aggregate expressions, and
      integer, bool, float, string, and null literals
- [x] AST package cache summaries skip parsing summary-safe dependencies after
      validation
- [x] AST/IR scalar payload packing preserves integer, bool, float, tuple-index,
      and indexed-assignment lowering paths
- [x] parser and sema share union-safe AST expression clone helpers for
      compound assignment targets and synthetic borrow receivers
- [x] AST pattern literal payload packing preserves integer, bool, signed range,
      or-pattern, alias, and product-pattern match lowering paths
- [x] sema iterator filter rewrites reuse the shared union-safe pattern clone
      helper
- [x] IR match-arm and nested enum-payload literal packing preserves scalar
      match lowering and aggregate enum nested-payload tests on LLVM
      lowering paths
- [x] AST expression builders preserve compound-assignment name cloning,
      matches! bool arms, tuple-index borrowing, and negative scalar literals
- [x] AST composite builders preserve string/null, tuple/vector/struct literal,
      labeled-block, and match expression lowering paths
- [x] IR literal builders preserve integer sign, float, string, bool, null,
      pointer fallback defaults, product-match fallback defaults, and tuple
      default lowering paths
- [x] IR access builders preserve struct field assignment/access, tuple-index
      reads, fixed-array static/dynamic indexing, and destructor call lowering
- [x] IR aggregate builders preserve tuple, struct, tuple-struct, expected
      aggregate, and range constructor lowering paths
- [x] IR call builders preserve direct, generic, inherent associated, trait
      associated, trait-qualified method, method, and zone helper call lowering
      paths
- [x] IR control-flow builders preserve aggregate/product if-let and match
      expression chain lowering paths
- [x] rare AST/IR vector payload packing preserves macro token parsing and
      format-print string-part, precision spec, and newline lowering paths
- [x] AST condition-pattern payload packing preserves enum and aggregate
      if-let/while-let statement and expression lowering paths
- [x] AST for-loop pattern payload packing preserves range, list literal,
      stored-vector, and iterator filter lowering paths
- [x] AST/IR match statement arm payload packing preserves scalar, enum,
      aggregate, while-let, LLVM, and LLVM lowering paths
- [x] AST/IR explicit-drop name payload packing preserves explicit destructor
      lowering on LLVM paths
- [x] AST/IR break statement payload packing preserves unlabeled break,
      labeled-loop break, and labeled-block value break lowering paths
- [x] AST/IR assignment statement payload packing preserves direct binding,
      compound, field, and indexed assignment lowering paths
- [x] AST/IR statement label payload packing preserves labeled block, loop,
      and range-for break lowering paths
- [x] AST/IR statement body-vector payload packing preserves block, if/else,
      while, range-for, init-while, and tuple-match if-chain lowering paths
- [x] AST/IR block-expression payload packing preserves nested block values,
      labeled block-expression breaks, and LLVM lowering paths
- [x] AST/IR if-expression payload packing preserves plain `if`, `else if`,
      and expression-valued `if let` lowering paths
- [x] AST/IR match-expression payload packing preserves enum, integer, and
      bool expression-valued match lowering paths
- [x] IR trait-object call parameter payload packing preserves multi-argument
      LLVM dyn dispatch
- [x] AST receiver type-argument payload packing preserves trait-qualified
      associated calls with explicit and inferred result type arguments
- [x] AST struct-literal field-name payload packing preserves out-of-order
      generic struct literal mapping for constants and locals
- [x] AST direct type-argument payload packing preserves explicit generic
      function calls, generic struct literals, and generic enum constructors
- [x] AST argument-vector payload packing preserves tuple, vector, struct
      literal, and call argument lowering on LLVM paths
- [x] IR argument-vector payload packing preserves tuple, vector, struct literal,
      call, and stored-vector for-loop lowering on LLVM paths
- [x] AST operand child payload packing preserves unary, binary, cast, try,
      tuple-index, field, index, call, compound-assignment clone, and synthetic
      borrow receiver lowering on LLVM paths, including
      method-call receiver lowering through sema helper accessors
- [x] IR operand child payload packing preserves pointer casts/load/store/
      addition, pointer dereference assignment, indirect calls, and binary
      lowering on LLVM paths
- [x] IR rare expression payload packing preserves compact enum constructors
      and vector set/swap/insert/search side-input lowering paths
- [x] IR string payload packing preserves string literals, local loads,
      borrows, direct calls, function pointer refs, and indirect calls through
      `ir_builders`, sema, and LLVM lowering, plus trait-object cast/call
      builder paths
- [x] reject duplicate `use` aliases in one module scope
- [x] reject duplicate aliases introduced by glob imports
- [x] reject private function access through a `use` alias
- [x] reject public items behind private nested modules from outside the parent
- [x] reject private non-`pub use` aliases through qualified access
- [x] private items are not imported by glob use from outside the module
- [x] reject private enum type access
- [x] reject private enum case access
- [x] reject private constant access
- [x] reject missing file-backed modules
- [x] reject `super::` paths that escape the root module

## Completed Sprint: Prelude Surface

Goal: make common Rust-like prelude names part of the compiler-known ABI
surface so library code can refer to one stable spelling before trait-bound
dispatch, source standard modules, macro expansion, and allocator-backed types are
implemented.

Checklist:

- [x] add byte input builtins: `read_byte`, `io::read_byte`, `input::read_byte`
- [x] add source byte-slice output helper: `io::write_bytes` and root
      `write_bytes`
- [x] add source IO contracts and cursor helpers: `io::Reader`,
      `io::Writer`, `io::Seek`, `io::stdin`, `io::stdout`, `io::cursor`,
      `io::read_exact_result`, `io::copy_result`, `io::write_all_result`,
      `io::flush_result`, `io::read_exact`, `io::write_all`, and `io::flush`
- [x] add source stderr writer handle: `io::Stderr` and `io::stderr`
- [x] add caller-buffered IO wrappers: `io::BufReader`, `io::BufWriter`,
      `io::buf_reader`, and `io::buf_writer`
- [x] add source byte input helper: `input::try_read_byte() -> Option[u8]`
- [x] add host line input builtins: `read_line`, `io::read_line`, `input`,
      `input::line`
- [x] add explicit-zone owned line helpers: `read_line_owned`,
      `io::read_line_owned`, `input_owned`, and `input::owned_line`
- [x] add assertion/stop function builtins: `assert`, `debug_assert`,
      `assert_eq_i64`, `assert_ne_i64`, `assert_eq_bool`, `assert_ne_bool`,
      source generic `assert_equal`/`assert_not_equal`, `panic`, `todo`,
      `unreachable`
- [x] add executable prelude macro forms: `assert!`, `debug_assert!`,
      `assert_eq!`, `assert_ne!`, `panic!`, `todo!`, `unreachable!`,
      `print!`, `println!`, and `eprintln!`
- [x] lower `matches!` through the pattern engine
- [x] reserve `format!` with targeted no-implicit-allocation-zone diagnostics
- [x] lower `format_in!(ref mut Zone, "...", values...)` to source
      `std::string::String` construction plus same-zone string/i64/bool/f64
      append helpers or borrowed-receiver `Display::format_in`, with value
      expressions evaluated once before type-directed append selection and
      `{:.N}` float precision matching `print`
- [x] lower `{:?}` debug placeholders in `print`/`println` for built-in
      printable values and in `format_in!` through borrowed-receiver
      `Debug::debug_in`
- [x] add source `std::fmt::FormatSpec` helpers for unsigned
      binary/octal/decimal/hex formatting, width, integer precision,
      fallible width/precision validation, left/right/center alignment,
      alternate prefixes, debug text quoting, explicit-zone strings, generic
      Display `write_value`, and `std::io::Writer` output plus generic Display stdout
      `print_value`/`println_value`
- [x] source `std::fmt::Debug` has built-in impl coverage for `i64`, `u64`,
      `char`, `bool`, `f32`, `f64`, lowercase `string`, and owned
      `std::string::String`, plus generic `debug_value`, `write_debug`,
      `print_debug`, and `println_debug`
- [x] source `std::fmt::Display` has built-in impl coverage for `i64`, `u64`,
      `char`, `bool`, `f32`, `f64`, lowercase `string`, and owned
      `std::string::String`, plus custom user impl coverage through
      `format_in!` and `String.append_value(value)`
- [x] root `Display` and `Debug` are public aliases for `std::fmt::Display` and
      `std::fmt::Debug`, so prelude and module-qualified formatting traits do
      not diverge
- [x] expose root `String` as the source `std::string::String` handle while
      keeping lowercase `string` as today's borrowed pointer-shaped text value
- [x] reserve Rust-like standard traits: debug/display/default/clone/copy/drop,
      equality/order, conversion, iterator, string/owned surfaces
- [x] reserve `Iterable[T]`
- [x] reserve `Iterator[T]` with `next(self: ref mut Self) -> Option[T]`
- [x] keep value-self `next(self) -> Option[T]` impls accepted as a copyable
      compatibility subset
- [x] lower direct copyable `Iterator[T]` `for` loops through `next`
- [x] lower direct `ref mut` `Iterator[T]` `for` loops through `next`
- [x] reserve `IntoIterator[T]` with
      `into_iter(self: ref mut Self) -> Self`
- [x] keep value-self `into_iter(self)` impls accepted as a copyable
      compatibility subset
- [x] lower self-returning copyable `IntoIterator[T]` `for` loops through
      `into_iter` and `next`
- [x] allow `IntoIterator[T].into_iter` impls to return a distinct iterator
      type and validate that result at `for` lowering sites
- [x] lower generic `IntoIterator[T]` impls that return a distinct generic
      iterator type
- [x] reject `IntoIterator[T].into_iter` impls whose concrete result does not
      implement `Iterator[T]`
- [x] keep generic direct `Iterator[T]` impls valid while the
      `IntoIterator[T]` result contract pass scans generic impl headers
- [x] reject generic `IntoIterator[T]` impls whose placeholder result type does
      not implement `Iterator[T]`
- [x] lower iterator item literal/range/or, fieldless enum-case, compact enum
      payload, homogeneous nested aggregate-enum payload, and nested
      aggregate-enum multi-payload patterns with nested or-pattern slots
      through the `while let Some(pattern) = next()` path
- [x] reserve `iter::Iterable[T]`
- [x] reserve `iter::Iterator[T]`
- [x] reserve `iter::IntoIterator[T]`
- [x] accept impls against prelude iterator traits
- [x] reject unknown trait impls
- [x] reject wrong trait type-argument counts
- [x] reject duplicate trait impls for the same trait/type pair
- [x] validate trait impl method completeness and signatures
- [x] parse `trait Child: Parent` supertraits and require matching
      supertrait impls for the same receiver type
- [x] resolve associated type projections through unique generic supertrait
      applications and reject ambiguous inherited associated type names
- [x] allow generic bounds on a child trait to statically dispatch methods
      declared by its supertraits
- [x] lower trait-qualified calls such as `Trait::method(receiver, ...)` and
      `Trait<T>::method(receiver, ...)` for explicit static dispatch
- [x] lower trait-qualified associated function calls such as
      `Trait::make<SelfType>(...)` and `Trait<T>::make<SelfType>(...)`, plus
      expected-result forms such as `let x: SelfType = Trait<T>::make(...)`
      in control-flow result arms, aggregate literal elements, and generic enum
      constructor payloads, including primitive `Self` impls, to disambiguate
      same-named trait associated functions
- [x] include object-safe supertrait methods in LLVM `dyn Child` vtables and
      reject ambiguous inherited dyn method names
- [x] allow LLVM dyn-to-dyn upcasts from a child trait object to the same trait
      or a supertrait, and reject unrelated dyn-to-dyn casts
- [x] accept bare `self` in trait/impl method signatures as `Self`
- [x] lower concrete `value.method(...)` calls through matching impl methods
- [x] reject unknown and ambiguous method calls
- [x] expose source `Option[T]` and `Result[T, E]` through the source prelude,
      with borrowed-receiver `is_some`/`is_none`/`is_ok`/`is_err`, consuming
      `is_some_and`/`is_none_or`/`is_ok_and`/`is_err_and` payload predicates,
      borrowed payload view handles, consuming `unwrap_or`, Option
      `take`/`replace`/`map`/`and_then`/`filter`/`flatten`/`transpose`, and
      Result `map`/`and_then`/`transpose`/`map_err`/`or` combinator methods
      implemented in `std::option` and `std::result`
- [x] add source `std::error` recoverable error values with stable kinds,
      POSIX errno mapping, predicate helpers, root `Error`/`ErrorKind`
      aliases, direct `Result[T, Error]` conversion, and compact raw scalar
      compatibility bridges for runtime/FFI error flow
- [x] add source `std::c` C ABI boundary helpers with borrowed `CStr`,
      zone-backed NUL-terminated `CString`, POSIX `errno`/`Error` bridging,
      root C boundary type aliases, and hosted dynamic loading wrappers over
      `dlopen`/`dlsym`/`dlclose`
- [x] add typed dynamic function symbol extraction through
      `Symbol.function<fn(...) -> ...>()` and raw-pointer-to-function-pointer
      cast lowering
- [x] expose source `Slice[T]` and `slice(data, len)` as a non-owning
      pointer/length view through the source prelude, with `len(view)`,
      `view.len()`, source `view.is_empty()`, `view[index]`, and `view[index] = value`
      helpers, mutable local array/Vec `as_slice()` view creation,
      `view[start..end]` / `view[start..=end]` range slicing, checked
      `first`/`last`/`get`, mutable `first_mut`/`last_mut`/`get_mut`,
      Option-returning `try_first`/`try_last`/`try_get`, element search,
      predicate search/count, endpoint splitting, and `Slice[T]`
      exact/prefix/suffix checks and stripping, provenance-preserving `as_ptr`,
      plus `copy_to(ref mut Zone)` into
      source `std::vec::Vec<T>`; the same positive read/view surface now runs
      through the LLVM
      backend to cover local Slice pointer/length lowering and
      aggregate-argument register spills
- [x] reject reserved root smart-pointer surfaces `Unique` and `Shared` with
      roadmap-backed diagnostics; expose `Rc`, `Arc`, and `Weak` as current
      root aliases for `std::rc` shared ownership
- [x] reject non-`i64` prelude range bounds until generic range lowering exists
- [x] add `lib/std.arih` source declarations for the stable
      declaration-shaped prelude surface, auto-load it as `std`, and verify
      explicit `std::...` calls through the LLVM backend
- [x] support `--no-implicit-std` so the same header can be tested only through
      ordinary `mod std;` file-backed module loading
- [x] resolve source `std` child modules as package files under `lib/std/` for
      both implicit `std` loading and explicit `mod std;` loading, including
      `std::boxed`, `std::c`, `std::cmp`, `std::collections`, `std::context`,
      `std::convert`, `std::env`, `std::error`, `std::fmt`, `std::process`,
      `std::thread`, `std::sync`, `std::time`, `std::fs`, `std::ascii`,
      `std::bits`, `std::input`, `std::io`, `std::iter`, `std::mem`,
      `std::option`, `std::result`, `std::string`, `std::vec`, and
      `std::zone`
- [x] reject compiler-known prelude helpers under `--no-implicit-std` until a
      real source `std` module is loaded
- [x] reject bare and prelude-path compiler-known function signatures such as
      `write_i64`, `write_u64`, and `io::write_i64` under `--no-implicit-std`
      until source `std` is loaded
- [x] reject compiler-known prelude trait names such as `Debug` under
      `--no-implicit-std` until source `std` is loaded
- [x] require source generic declarations before lowering declaration-shaped
      `std::mem`, `std::iter`, and typed `std::zone` helpers through compiler
      hooks
- [x] remove compiler-injected concrete prelude function signatures and trait
      fallback declarations; ordinary calls and impls now resolve against
      source `std` declarations plus implicit aliases
- [x] split Ari-owned builtins from C FFI with `extern "ari"` declarations and
      reject unknown `ari_builtin_*` symbols
- [x] reject mismatched `extern "ari"` builtin signatures before IR lowering
- [x] add Rust-like implicit prelude aliases for public `std` root items and
      root re-exports, including type/trait names, IO helpers, zone helpers,
      and sema-lowered helpers while preserving local shadowing
- [x] add implicit aliases for public `std` child modules so nested prelude
      forms such as `fmt::Display` and `iter::Iterator[T]` resolve through
      source `std` declarations
- [x] allow root re-exports such as `input()` to coexist with child-module
      paths such as `input::read_byte()`
- [x] centralize Ari builtin source aliases so LLVM lowering
      accept the same root `std::...` re-export spellings
- [x] keep `Hash` out of the prelude
