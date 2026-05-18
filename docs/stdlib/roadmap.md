# Standard Library Roadmap

This page summarizes the library implementation plan. The detailed compiler
roadmap remains in `docs/dev/standard-library-roadmap.md`.

## Phase 1: Stabilize The Current Seed

- Keep `lib/std.arih` as the public root.
- Keep child modules under `lib/std/`.
- Keep all public declarations in `tests/std_api_manifest.txt`.
- Keep focused tests under `tests/cases/standard-library/`.
- Keep this `docs/stdlib/` folder current with every public API.

Current source families: `option`, `result`, `mem`, `zone` raw allocation plus
source typed array allocation, `boxed`, `string` byte access/search/ASCII
helpers including case search, prefix parsers, and owned trim copies, `ascii`
byte classification, case-insensitive comparison/search, slice helpers, and
prefix parsers, `vec`, `iter`, `fmt`, `cmp` comparison helpers, `convert`
identity/from/into helpers, `context` runtime hooks plus the source
`has_arg` helper, `env` source argument wrappers with `try_arg` and
`program_name` plus current-process environment `get`/`has`/`try_get`/`set`/
`remove` and path-state helpers `current_dir`/`try_current_dir`/
`set_current_dir`/`executable_path`/`try_executable_path`,
`input` runtime hooks plus the source `try_read_byte` EOF helper,
`io` runtime hooks plus source byte-slice output, current `process`
id/exit/status helpers,
`collections::Set[T]` as the first linear insertion-order set with
`try_*` accessors, `pop`/`try_pop`, replace-or-insert updates, explicit
reserve growth, direct iterator support, and the first `math` sign
predicate/arithmetic/division-rounding and `bits` numeric helper slices,
including zero/one-run bit scans.

## Essential Library Families

These are the library families Ari needs for modern application and systems
work. Each one should land in small tested slices with natural API names.

| Family | Purpose | Current Or Planned APIs |
| --- | --- | --- |
| `std::env` | Read startup and environment state without exposing raw runtime hooks. | Current `arg_count`, `arg`, `has_arg`, `try_arg`, `program_name`, `get`, `has`, `try_get`, `set`, `remove`, `current_dir`, `try_current_dir`, `set_current_dir`, `executable_path`, `try_executable_path`; future path normalization and platform-specific expansion. |
| `std::process` | Represent the current process and child processes explicitly. | Current `id`, `exit`, `success`, `failure`, status predicates; future `spawn`, `wait`, platform `fork`, status/result handles. |
| `std::fs` | Work with files and directories through owned handles. | Future open/read/write/close, metadata, directory iteration, path helpers. |
| `std::time` | Access monotonic and wall-clock time for CLIs, servers, and tests. | Future `Instant`, `Duration`, `now`, elapsed arithmetic, sleep. |
| `std::thread` | Start and join OS threads with clear ownership transfer. | Future `spawn`, `join`, thread id, stack/runtime context setup. |
| `std::sync` | Share state between threads deliberately. | Future atomics, `Mutex`, `Shared`, `Weak`, and possibly channels after ownership rules are stable. |
| `std::collections` | Store keyed and set-like data beyond vectors. | Current linear `Set[T]` with insertion-order access, optional access, replacement, removal, copy, reserve helpers, and direct iteration; future `HashMap` and `HashSet`. |
| `std::os` | Hold platform-specific syscall wrappers that are too sharp for portable modules. | Future Unix/Windows gated modules, raw descriptors/handles, error-code translation. |

## Phase 2: Pull More Behavior Into Ari Source

- Replace compiler hooks with source code when generic aggregates, trait
  dispatch, and module cache summaries can model them safely.
- Keep compiler-known declarations as compatibility shims only when required.
- Improve diagnostics for partial custom `std` packages.

## Phase 3: Grow Collections And Text

- Stabilize `std::vec::Vec[T]` and document the distinction from bare local
  `Vec[T]`.
- Keep shared collection access vocabulary aligned across `Slice[T]` and
  `std::vec::Vec[T]`, including `try_*` methods for `Option`-based absence.
- Add collection helpers in small slices: slice methods, vector methods, the
  current linear `Set[T]` access/update/reserve/iteration surface, iterator
  adapters, then hash maps/sets/deques.
- Add `HashMap` and `HashSet` only after generic aggregate layouts,
  hashing/equality trait policy, and explicit allocation-zone ownership are
  testable together.
- Keep `std::string::String` byte-oriented until a Unicode/text policy is
  designed.
- Expose small `String` conveniences only when they preserve byte-string
  semantics, such as ASCII case comparison/search, borrowed ASCII trim views,
  owned trim copies, and whole/prefix ASCII parsers.
- Keep ASCII-only helpers in `std::ascii` so byte-oriented classification,
  comparison, search, trimming, and parsing behavior is explicit at call sites.

## Phase 4: Numerics

- Expand `std::math` from i64 signatures to generic numeric helpers when the
  language has the right trait vocabulary. Preserve the existing natural names
  for signs, sign predicates, parity, powers, division rounding, and divisor
  helpers.
- Expand `std::bits` from u64 signatures to generic integer mask, rotation,
  power-of-two, low-mask, and bit-scan helpers when the same trait vocabulary
  exists.
- Add checked, saturating, and wrapping operations only after overflow policy
  is documented.
- Add floating helpers only where LLVM lowering and runtime behavior are
  stable.

## Phase 5: OS-Facing Libraries

- Grow `std::env` from the current argument, variable, current-directory, and
  executable-path base into path normalization and platform-specific policy
  once owned-string behavior is stable.
- Keep `std::context` as the low-level runtime state boundary: arguments and
  main-thread identity are implemented now; future thread support should extend
  the thread-id slot instead of changing the public context API shape.
- Grow `std::process` from current-process helpers into child-process handles,
  then add `std::fs`, `std::time`, `std::thread`, and `std::sync` as thin
  explicit wrappers after C FFI conventions are stable.
- Keep syscall-facing helpers minimal and modern: process arguments and
  environment, current directory, file descriptors/handles, time, process
  spawn/fork where the platform supports it, thread creation/join, atomics or
  shared ownership handles, and error-code conversion.
- Keep handles visible and owned; do not hide OS resources behind global state.

## Phase 6: Library Developer Experience

- Add source-level test helpers when the language can express them.
- Build richer `@test` integration around library test cases.
- Keep examples short enough to serve as contributor templates.
