# Ari Standard Library

This folder is the dedicated home for Ari standard library documentation. Use
it when writing Ari programs, changing `lib/std.arih`, or adding a new module
under `lib/std/`.

## Start Here

- [Overview](overview.md): library purpose, module map, and design rules.
- [API Reference](api-reference.md): current public APIs grouped by module.
- [Module Guides](modules/README.md): focused notes for individual standard
  library modules.
- [Platform Notes](platform/README.md): target, ABI, linker, and OS-specific
  roadmap notes.
- [Library Development](library-development.md): how to add or change a
  standard library API.
- [Testing](testing.md): test names, check targets, and coverage expectations.
- [Roadmap](roadmap.md): staged implementation plan and next library families.

## Current Shape

The standard library is ordinary Ari source whenever possible. It lives at
`lib/std.arih`, with child modules in `lib/std/`. User-facing argument,
environment-variable, current-directory, and executable-path helpers now live
in `std::env`, while `std::context` stays the low-level runtime context layer.
`std::io` now has source `Reader`/`Writer`/`Seek` contracts, `Stdin`,
`Stdout`, `Stderr`, `Cursor`, caller-buffered `BufReader`/`BufWriter`,
`read_exact`, `write_all`, and `flush` on top of the raw process IO hooks.
`std::test` adds source executable unit-test reports, generic equality checks,
and scratch-zone helpers, and `std::log` adds level-prefixed `stderr`
diagnostic lines while richer runner, structured logging, source-location, and
backtrace support remain roadmap work.
`std::process`
starts the OS-facing surface with current
process id, explicit exit helpers, and the first POSIX fork/wait slice,
`std::target` reports compiler-known target, object/debug format, libc/env,
errno, syscall, and Linux API-family facts,
`std::thread` adds
function-pointer spawn/join, scheduler yield, and per-thread runtime ids,
`std::sync` adds the first concrete atomic integer primitive, `std::time` adds
monotonic instants, wall-clock timestamps, non-negative
durations, elapsed-time helpers, and sleep, and `std::fs` adds the first
byte-oriented file handle slice with mode-string opens for read, write,
append, and read/write access plus access-permission checks, source create,
read, write, append, truncate, copy, rename, hard/symbolic links,
single-directory create/remove, and read-to-byte-string helpers, `std::path`
adds source-only lexical path
splitting, joining, and lightweight normalization, and `std::net` adds
source-only IPv4, IPv6,
generic IP, and socket-address values while DNS and socket handles remain
runtime roadmap work. Source
`std::hash` adds deterministic non-cryptographic `Hasher`/`Hash[T]` helpers
for primitive values and byte slices. Source collection work includes
`std::collections::Set[T]` as a linear explicit-zone
set with insertion-order access, optional access, replace-or-insert updates,
iterator support, and reserve growth; growable `Deque`, bounded `RingBuffer`,
zone-backed `LinkedList`, and comparator-driven `BinaryHeap`/`PriorityQueue`
handles; real hash-table `HashMap`/`HashSet` handles with live-bucket
iterators; and red-black-tree `TreeMap`/`TreeSet` handles with sorted
key/value iteration. `std::algo` adds source slice algorithms for sorting,
binary search, reverse/rotate, partition, min/max/clamp, swap, fill, copy, and
dedup. `std::parse` adds whole-input integer, bool, and decimal float parsing,
and `std::encoding` adds ASCII/UTF-8/UTF-16 validation plus hex/base64 codecs.
A few declarations are still compiler-known because the
current language cannot express them directly: layout queries, typed raw
pointer operations, byte memory intrinsics, runtime IO hooks, explicit zone
allocation, formatting macro lowering, and some zone provenance checks.

The rule of thumb is simple: put behavior in Ari source first, and add
compiler support only when the language cannot safely model the primitive yet.
