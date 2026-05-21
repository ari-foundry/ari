# Ari Standard Library

This folder is the dedicated home for Ari standard library documentation. Use
it when writing Ari programs, changing `lib/std.arih`, or adding a new module
under `lib/std/`.

## Start Here

- [Overview](overview.md): library purpose, module map, and design rules.
- [API Reference](api-reference.md): current public APIs grouped by module.
- [Text And Path Kinds](text-kinds.md): when to use byte strings, UTF-8
  views, OS string bytes, path bytes, and C strings.
- [Module Guides](modules/README.md): focused notes for individual standard
  library modules.
- [Platform Notes](platform/README.md): target, ABI, linker, and OS-specific
  roadmap notes.
- [Library Development](library-development.md): how to add or change a
  standard library API.
- [Production Readiness](production-readiness.md): quality bar for stable,
  dependable standard library APIs, including module tiers, failure policy,
  platform policy, non-goals, and release acceptance checks.
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
`BufReader`/`BufWriter`, `read_exact`, `write_all`, and `flush` on top of the
raw process IO, descriptor, and file seek hooks.
`std::test` adds source executable unit-test reports, generic equality checks,
and scratch-zone helpers, `std::log` adds level-prefixed `stderr` diagnostic
lines, and `std::error` adds shared recoverable error categories,
compact error values, and POSIX errno mapping while richer runner,
compiler-tooling source maps, structured logging, direct
`Result[T, Error]`, and backtrace support remain roadmap work.
`std::c` adds the narrow C ABI boundary layer with borrowed
`CStr`, zone-backed `CString`, POSIX `errno`, and hosted dynamic loading
handles over `dlopen`/`dlsym`.
`std::process`
starts the OS-facing surface with current
process id, explicit exit helpers, and the first POSIX fork/wait slice,
`std::os` introduces non-owning `Fd` descriptor views and `OwnedFd` wrappers
for raw descriptor close, duplicate, close-on-exec, and nonblocking policy,
plus `Pipe` for owned read/write descriptor pairs,
`std::target` reports compiler-known target, object/debug format, libc/env,
errno, syscall, and Linux API-family facts,
`std::thread` adds
function-pointer spawn/join, scheduler sleep/yield helpers, available
parallelism, and per-thread runtime ids,
`std::sync` adds the first concrete atomic integer primitive plus source
`Mutex`, `RwLock`, and `Once` helpers, `std::time` adds monotonic instants,
wall-clock timestamps, non-negative
durations, elapsed-time helpers, monotonic deadlines, UTC calendar conversion,
and sleep, and `std::fs` adds the first
byte-oriented file handle slice with mode-string and `OpenOptions` opens for
read, write, append, and read/write access, raw-error open, mutation, and
byte-count results, plus
access-permission checks, source create,
read, write, append, position, seek, truncate, copy, rename, hard/symbolic
links, link-target reads, no-follow link metadata, single-directory and
recursive directory create/remove helpers, directory-entry metadata, and
metadata access/modification/status-change timestamps, and read-to-byte-string
helpers, `std::path`
adds source-only lexical path
splitting, joining, lightweight normalization, and typed `PathBytes` views,
and `std::net` adds
IPv4, IPv6, generic IP, and socket-address values plus hosted IPv4 DNS lookup,
IPv4 `TcpListener`/`TcpStream` bind/connect/accept helpers, IPv4
`UdpSocket` single-byte datagrams, TCP/UDP local-address helpers, TCP peer-address helpers, Unix stream
sockets, nonblocking flags, `std::time::Duration` socket timeouts with raw
millisecond compatibility helpers, stream shutdown, and TCP/Unix
`read_exact`/`write_all` stream buffer helpers. IPv6 socket handles,
buffer-oriented datagrams, broader socket options, UDP source address helpers, and
richer error payloads remain runtime roadmap work. Source
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
lookup, reverse/rotate, partition, min/max/clamp, swap, fill, copy, and dedup. `std::parse` adds whole-input decimal, radix, hex,
binary, octal integer, bool, and decimal float parsing,
and `std::encoding` adds ASCII/UTF-8/UTF-16 validation plus hex/base64 codecs
with fallible owned decoders for untrusted input.
A few declarations are still compiler-known because the
current language cannot express them directly: layout queries, typed raw
pointer operations, byte memory intrinsics, runtime IO hooks, explicit zone
allocation, formatting macro lowering, and some zone provenance checks.

The rule of thumb is simple: put behavior in Ari source first, and add
compiler support only when the language cannot safely model the primitive yet.
