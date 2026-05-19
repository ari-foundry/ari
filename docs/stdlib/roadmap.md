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
id/exit/status helpers, `time` monotonic/wall-clock/sleep hooks plus source
`Duration`/`Instant`/`SystemTime` helpers, `fs` byte-oriented file existence,
open/read/write/close/remove hooks plus source `File` methods and
`Option[File]` open helpers,
`collections::Set[T]` as the linear insertion-order set with `try_*`
accessors, `pop`/`try_pop`, replace-or-insert updates, explicit reserve
growth, direct iterator support, open-addressed `HashMap`/`HashSet` with
tombstones and live-bucket iterators, red-black-tree `TreeMap`/`TreeSet` with
sorted iterators, and the first `math` sign predicate/arithmetic/
division-rounding and `bits` numeric helper slices, including zero/one-run bit
scans.

## Essential Library Families

These are the library families Ari needs for modern application and systems
work. Each one should land in small tested slices with natural API names.

| Family | Purpose | Current Or Planned APIs |
| --- | --- | --- |
| `std::env` | Read startup and environment state without exposing raw runtime hooks. | Current `arg_count`, `arg`, `has_arg`, `try_arg`, `program_name`, `get`, `has`, `try_get`, `set`, `remove`, `current_dir`, `try_current_dir`, `set_current_dir`, `executable_path`, `try_executable_path`; future path normalization and platform-specific expansion. |
| `std::process` | Represent the current process and child processes explicitly. | Current `id`, `exit`, `success`, `failure`, status predicates; future `spawn`, `wait`, platform `fork`, status/result handles. |
| `std::fs` | Work with files and directories through explicit handles. | Current `File`, `exists`, `remove`, `open_read`, `open_write`, `try_open_read`, `try_open_write`, byte `read_byte`/`write_byte`/`write_bytes`, and `close`; future owned resource policy, metadata, directory iteration, path helpers, append/read-write modes. |
| `std::time` | Access monotonic and wall-clock time for CLIs, servers, and tests. | Current `Duration`, `Instant`, `SystemTime`, `nanoseconds`, `microseconds`, `milliseconds`, `seconds`, `now`, `system_now`, `elapsed`, `sleep`; future timers, interruption-aware sleep, and calendar formatting. |
| `std::thread` | Start and join OS threads with clear ownership transfer. | Future `spawn`, `join`, thread id, stack/runtime context setup. |
| `std::sync` | Share state between threads deliberately. | Future atomics, `Mutex`, `Shared`, `Weak`, and possibly channels after ownership rules are stable. |
| `std::collections` | Store keyed and set-like data beyond vectors. | Current linear `Set[T]`, open-addressed `HashMap`/`HashSet`, red-black-tree `TreeMap`/`TreeSet`, explicit hash/comparator constructors, lookup/update/removal where implemented, live-bucket hash iterators, sorted tree iterators; future tree deletion, deques, and trait-driven constructors. |
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
  current linear `Set[T]` access/update/reserve/iteration surface, hash/table
  lookup and tombstones, tree insertion/lookup, hash and tree iterators,
  red-black deletion, then deques.
- Keep `HashMap`/`HashSet` and `TreeMap`/`TreeSet` on explicit hash or
  comparator constructors until generic trait-driven `Hash`, `Eq`, and `Ord`
  selection is testable.
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
- Grow `std::process` from current-process helpers into child-process handles.
  `std::time` and `std::fs` now have their first thin wrappers; time should
  grow toward timers and interruption-aware sleep, while filesystem work should
  next add stronger owned-resource policy, metadata, directory iteration, and
  path helpers. Add `std::thread` and `std::sync` as thin explicit wrappers
  after ownership-transfer and shared-state rules are stable.
- Keep syscall-facing helpers minimal and modern: process arguments and
  environment, current directory, file descriptors/handles, time, process
  spawn/fork where the platform supports it, thread creation/join, atomics or
  shared ownership handles, and error-code conversion.
- Keep handles visible and owned; do not hide OS resources behind global state.

## Phase 6: Library Developer Experience

- Add source-level test helpers when the language can express them.
- Build richer `@test` integration around library test cases.
- Keep examples short enough to serve as contributor templates.
