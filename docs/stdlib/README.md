# Ari Standard Library

This folder is the dedicated home for Ari standard library documentation. Use
it when writing Ari programs, changing `lib/std.arih`, or adding a new module
under `lib/std/`.

## Start Here

- [Overview](overview.md): library purpose, module map, and design rules.
- [API Reference](api-reference.md): current public APIs grouped by module.
- [Generated API Index](generated/api-index.md): exhaustive manifest-derived
  public API spelling and coverage index.
- [Example Index](examples.md): task-oriented entry points and representative
  executable examples for each module.
- [Completion Status](completion-status.md): current basic stdlib completion
  gauges and the remaining standard-library work, excluding long-term
  stability validation.
- [Stability Policy](stability.md): stable, usable, platform-backed,
  platform-specific, and experimental API rules.
- [Text And Path Kinds](text-kinds.md): when to use byte strings, UTF-8
  views, OS string bytes, path bytes, and C strings.
- [Value Movement Contracts](value-contracts.md): copy, move, clone, drop, and
  drain contracts for sequence and collection helpers.
- [Module Guides](modules/README.md): focused notes for individual standard
  library modules.
- [Platform Notes](platform/README.md): target, ABI, linker, and OS-specific
  roadmap notes.
- [Library Development](library-development.md): how to add or change a
  standard library API.
- [Production Readiness](production-readiness.md): quality bar for stable,
  dependable standard library APIs, including module tiers, failure policy,
  platform policy, non-goals, and release acceptance checks.
- [Verification Matrix](verification-matrix.md): local checks, platform
  support, CI jobs, and fuzz/property-test strategy.
- [Testing](testing.md): test names, check targets, and coverage expectations.
- [Roadmap](roadmap.md): staged implementation plan and next library families.

## Current Shape

The standard library is ordinary Ari source whenever possible. It lives at
`lib/std.arih`, with child modules in `lib/std/`. User-facing argument,
environment-variable, current-directory, executable-path, OS-string, and
path-byte helpers now live in `std::env`, while `std::context` stays the
low-level runtime context layer.
`std::io` now has source `Reader`/`Writer`/`Seek` contracts, `Stdin`,
`Stdout`, `Stderr`, `PipeReader`/`PipeWriter`, `Cursor`, caller-buffered
`BufReader`/`BufWriter`, `ReadByte` one-byte status values, direct `Error`
helpers for partial reads, exact reads, whole-slice writes, stream copies, and
flushes, plus bool/Option compatibility wrappers on top of the raw process IO,
descriptor, and file seek hooks.
`std::test` adds source executable unit-test reports, generic equality checks,
scratch-zone helpers, temporary path helpers, snapshot/golden comparisons,
minimal benchmark timers, and compiler `@test` runner integration with stderr
progress/failure markers. `std::log` adds level-prefixed `stderr` diagnostic
lines, and `std::error` adds shared recoverable error categories, compact error
values, POSIX errno mapping, module-local `Error` aliases, and stable
`Display`/`Debug` output while per-test panic/log capture, doctests,
compiler-tooling source maps, structured logging, remaining `Result[T, Error]`
rollout, and backtrace support remain roadmap work.
`std::c` adds the narrow C ABI boundary layer with borrowed
`CStr`, zone-backed `CString`, POSIX `errno`, and hosted dynamic loading
handles over `dlopen`/`dlsym`.
`std::process`
starts the OS-facing surface with current
process id, explicit exit helpers, and the first POSIX fork/wait slice with
direct `Error` results plus raw compatibility helpers, then adds the first
`Command`/`Child`/`ExitCode`/`ExitStatus`/`Signal`/`Output` builder surface
for argument passing, child environment setup, working-directory setup,
`spawn`, `status`, `exit_status`, `output`, `output_in`, `exec`, `kill`,
`kill_signal`, typed status inspection, child stream aliases, temp file/temp
dir helpers, file-backed or `/dev/null` child stdin redirection, inherited or
cleared environment policy, and readiness-drained stdout/stderr capture,
`std::os` introduces non-owning `Fd` descriptor views and `OwnedFd` wrappers
for raw descriptor close, duplicate, close-on-exec, and nonblocking policy,
plus `Pipe` for owned read/write descriptor pairs,
`std::target` reports compiler-known target, object/debug format, libc/env,
errno, syscall, and Linux API-family facts,
`std::thread` adds
function-pointer spawn/join, scheduler sleep/yield helpers, available
parallelism, and per-thread runtime ids,
`std::sync` adds concrete atomic wrappers, source no-poison `Mutex`/`RwLock`
guard helpers, `Once`/`OnceLock`, generation `Condvar`, `Barrier`, and
capacity-1 channel helpers with Result errors, `std::cell` adds `Cell`,
runtime checked `RefCell`, zone-backed `OnceCell`, and `Lazy`, `std::rc` adds
`Rc`, `Arc`, and `Weak` shared ownership handles, `std::time` adds monotonic
instants,
wall-clock timestamps, non-negative
durations, elapsed-time helpers, monotonic deadlines, UTC calendar conversion,
and sleep, and `std::fs` adds the first
byte-oriented file handle slice with mode-string and `OpenOptions` opens for
read, write, append, and read/write access, direct `Error` result forms for
open, mutation, and byte-count operations with raw compatibility bridges, plus
access-permission checks, source create,
read, write, append, position, seek, truncate, copy, rename, hard/symbolic
links, link-target reads, no-follow link metadata, single-directory and
recursive directory create/remove helpers, directory-entry metadata, and
metadata access/modification/status-change timestamps, and read-to-byte-string
helpers, `std::path`
adds source-only lexical path
splitting, joining, lightweight normalization, and typed `PathBytes` views,
and `std::net` adds
IPv4, IPv6, generic IP, and socket-address values plus hosted IPv4/IPv6 DNS
lookup and `"host:port"`/`"[host]:port"` endpoint resolution,
IPv4/IPv6 `TcpListener`/`TcpStream` bind/connect/accept helpers, IPv4/IPv6
`UdpSocket` datagrams with source-address receive helpers, connected UDP
send/receive, TCP/UDP local-address helpers, TCP peer-address helpers, Unix
stream sockets, direct `Error` result helpers with raw compatibility variants,
nonblocking flags, `std::time::Duration` socket timeouts with raw
millisecond compatibility helpers, TCP listener/UDP reuse-address options,
TCP nodelay and keepalive options, UDP broadcast, reuse-port, send/receive
buffer-size options, stream shutdown, TCP/Unix `read`/`write`/
`read_exact`/`write_all`/`read_to_end`/`read_to_string` helpers,
buffer-oriented UDP datagrams, UDP source address helpers, and
zone-backed resolver lists, plus single-descriptor readiness probes for
listeners, streams, and UDP sockets. Full DNS iteration, multi-descriptor
poll/event loops, richer timeout errors, and platform socket extensions remain
runtime roadmap work.
Source
`std::hash` adds deterministic non-cryptographic `Hasher`/`Hash[T]` helpers
for primitive values and byte slices, and `std::random` adds OS entropy plus a
deterministic non-cryptographic `Prng` with booleans, unbiased bounded
integers, unit floats, byte filling, and shuffling. Source collection work
includes `std::collections::Set[T]` as a linear explicit-zone set with
insertion-order access, optional access, replace-or-insert updates,
iterator support, and reserve growth; growable `Deque`, bounded `RingBuffer`,
zone-backed `LinkedList`, and comparator-driven `BinaryHeap`/`PriorityQueue`
handles; real hash-table `HashMap`/`HashSet` handles with live-bucket
iterators, map entry update handles, key-value removal, and set representative
lookup; and red-black-tree `TreeMap`/`TreeSet` handles with sorted key/value
iteration, entry update handles, key-value removal, and set representative
lookup.
Tracked local growable collection handles infer their
constructor zone for common mutating growth calls. Root `Slice[T]` now has
borrowed range views, split
views, subsequence search, lexicographic comparison, lazy chunks/windows, and
delimiter splitting, while `std::string` mirrors those byte-view operations
and adds allocator-backed `join_in` plus typed borrowed `Utf8`/`OsStr` views
and shared `std::c::CStr` construction so code can distinguish validated
UTF-8, OS bytes, and NUL-terminated C strings. `std::algo` adds source slice algorithms
for sorting, binary search, lower/upper/equal-range bounds, partition-point
  lookup, reverse/rotate, partition, min/max/clamp, swap, fill, copy, and dedup.
  The current sequence value movement contract is documented separately so
  copy-oriented helpers stay distinct from future move-aware resource handling.
  `std::parse` adds whole-input signed/unsigned decimal and radix integer
  parsing, integer and float `ParseError` diagnostics, hex/binary/octal signed
  wrappers, bool and decimal float parsing, plus trait-backed `parse<T>`,
  `parse_or<T>`, and `is_parse<T>` helpers,
and `std::encoding` adds ASCII/UTF-8/UTF-16 validation, detailed UTF-8
byte-string decoding, plus hex, standard base64, MIME base64, and URL-safe
base64 codecs with fallible owned decoders and structured diagnostic
names/messages for untrusted input.
A few declarations are still compiler-known because the
current language cannot express them directly: layout queries, typed raw
pointer operations, byte memory intrinsics, runtime IO hooks, explicit zone
allocation, formatting macro lowering, and some zone provenance checks.

The rule of thumb is simple: put behavior in Ari source first, and add
compiler support only when the language cannot safely model the primitive yet.
