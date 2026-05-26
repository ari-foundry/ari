# Standard Library Completion Status

This page tracks the basic standard-library completion slice: ordinary
application and systems-programming APIs, coherent error names, resource
lifecycle docs, text/path boundaries, collections, hosted-program basics,
module docs, the API manifest, and focused regression tests.

It deliberately does not score long-term stability validation. Fuzz tests,
property tests, randomized stress tests, sanitizer infrastructure, soak tests,
long-running concurrency tests, and broad CI matrices belong to
[verification-matrix.md](verification-matrix.md) and later hardening work.

## Current Scorecard

The percentages are coarse planning gauges, not release promises. They compare
the current Ari stdlib against a practical basic hosted standard library.

| Area | Gauge | What is usable now | Remaining basic-to-standard work |
| --- | ---: | --- | --- |
| Error API model | 100% | Natural `Result[..., std::error::Error]` APIs are the default for `fs`, `io`, `net`, `process`, path-state `env`, parse, and encoding. Compatibility APIs are explicit through `_optional`, `_or_default`, `_bool`, `_unchecked`, and `_raw`; `env::var` intentionally uses `Option` because missing variables are ordinary configuration state. Ordinary old result-suffixed migration aliases have been retired from these core hosted modules. | Future work is not naming cleanup: add Windows error mapping and optional owned messages or structured fields after platform policy is ready. Low-level `std::c`, `std::os`, and strict entropy helpers still document their separate boundary policies. |
| `std::fs` | 98% | Result-first file open/read/write/copy/mutation, `OpenOptions`, metadata and no-follow metadata, POSIX owner/group ids, explicit creation-time fallback policy, permissions/mode, directory entries with lazy metadata, per-entry `DirEntryInfo` Result snapshots, structured `PathError`/`TwoPathError` detailed helpers, symlinks, canonicalization, temp files/dirs, POSIX advisory locks, IO adapters, and focused tests/docs/API manifest coverage. | Remaining work is outside the basic hosted slice: ACLs, owner/group name lookup, platform-specific owned path values, Windows path/symlink/locking policy, and exact partial-read error reporting after the runtime distinguishes EOF from read failure below `read_byte`. |
| `std::io` | 95% | `Reader`/`Writer`/`Seek`, natural `print`/`println`/`eprint`/`eprintln`, `write`, `write_all`, `flush`, `read_exact`, `read_all`, `read_to_string`, pipes, cursors, buffered readers/writers, writer methods, and explicit `BufWriter` flush behavior. | Zone-owning buffered constructors, richer EOF versus read-error policy for low-level byte hooks, and broader streaming formatter integration. |
| `std::process` | 89% | `Command`, `Child`, typed `ExitStatus`/`ExitCode`/`Signal`, mutating and chainable builder helpers, environment inherit/clear policy, current-dir setup, typed status/output capture, UTF-8 output conversion, temp paths, child stdin file and `/dev/null` redirection. | Readiness-based large stdout/stderr draining, streaming pipe-backed stdin, parent-visible child setup error pipe, Windows process mapping, richer platform status fields, stronger lifecycle policy for un-waited children, and possible ref-mut fluent aliases once the command builder owns an allocation policy that does not require passing a zone. |
| `std::net` | 88% | IPv4/IPv6 address values, hosted IPv4/IPv6 DNS lookup, bracketed IPv6 endpoint parsing, `resolve_all`/`to_socket_addrs`, IPv4/IPv6 TCP and UDP sockets, Unix streams, socket IO adapters, UDP `send_to`/`recv_from`/`peek_from`, common options, close-on-exec, nonblocking, timeouts, keepalive, reuse port, broadcast, and buffer sizes. | Full DNS result iteration beyond the current small resolver list, service-name ports, canonical names, timeout-specific errors, readiness/poll, linger, TTL/hop-limit, multicast, Unix datagram, peer credentials, and package-layer TLS policy. |
| `std::thread` | 87% | Result-returning spawn/join for `fn() -> i64`, `JoinHandle`, `JoinError`, typed `ThreadResult` wrappers, `ThreadId`, `Builder`, detach, advisory completion, sleep/yield, available parallelism, and explicit `ThreadLocal[T]` with capacity introspection and recoverable `try_set`/`get_or_try_init`. | Generic `JoinHandle[T]`, captured closures, scoped threads, compiler-level thread-local declarations, generic thread return payloads, and less zone/capacity-heavy TLS ergonomics. |
| `std::sync` | 86% | Concrete atomics with Result compare-exchange, no-poison `Mutex`/`RwLock` guards with drop unlock, `Once`, `OnceLock` fallible init status, `Condvar` timeout shape, `Barrier`, and capacity-1 MPSC channels with Result send/recv/recv-timeout errors and clonable sender handles. | Value-protecting `Mutex[T]`/`RwLock[T]`, optional poison-aware types, futex or OS blocking waits, configurable channel capacity, sender-counted close semantics, semaphores, and target-native generic atomics. |
| Text, string, encoding, path, C boundary | 91% | Byte-oriented `String`, parser-friendly string helpers, UTF-8 codepoint helpers, explicit `Utf8`, `OsStr`, `CStr`/`CString`, UTF-8 validation/decode/encode with named diagnostics, hex/base64 including MIME and URL-safe shapes with named codec diagnostics, POSIX `PathBytes`/`PathBuf` alias, joins and normalization. | Owned `Utf8String`/`OsString`, non-alias platform-specific `PathBuf`, Windows drive/UNC/OS-string policy, Unicode normalization/transcoding, grapheme/locale policy. No separate string-builder type is planned for the basic stdlib slice; byte `String` and `Vec[u8]` cover that role. |
| Collections and iterators | 85% | `Vec`, slices, iterators, algorithms, `HashMap`/`HashSet`, `TreeMap`/`TreeSet`, `Deque`, `RingBuffer`, `LinkedList`, `BinaryHeap`, `PriorityQueue`, entry APIs, drains, ordered boundaries, explicit hash/comparator constructors, mutable value cursors. | Trait-driven default constructors such as `HashMap::new`, borrowed lookup, lazy set algebra, tree retain/range mutation, split/append operations, and first-class reference-valued iterator items for tighter mutable iteration. |
| Formatting and logging | 86% | Display/Debug traits, scalar formatting, strict/fallible `FormatSpec`, `format_in!`, stdout/stderr value printing, writer-backed helpers, `concat2`, `concat3`, `format_value`, fixed-arity runtime template `format`/`format2`/`format3`, and matching writer helpers. | Variadic/default-zone formatting policy, fully streaming template writers, structured logging, scoped filters, and capture/backtrace integration. |
| Parse and encoding | 92% | Natural Result numeric parsers, bool/float parsing, underscore-aware numeric parsers, parse diagnostics with integer and float range offsets, UTF-8 and codec diagnostics with stable names/messages, fallible owned decoders. | Exact IEEE-754 boundary rounding diagnostics, richer parse error taxonomy where it matters, normalization/transcoding, and optional compression outside the core encoding module. |
| Hosted-program basics | 91% | Args/env/current-dir/executable path, fs/io/parse/path/process/time/thread/sync/net basics are enough for small CLI tools and arix-style prototypes. | Package-manager-specific polish, process streaming, full resolver iteration, platform-specific path/process behavior, and scoped concurrency ergonomics. |
| Docs, manifest, examples, tests | 93% | Module guides, generated API index, manifest checks, focused normal regression tests, example index, and current completion tracking exist for the implemented stdlib surface. | Keep this page, module docs, generated API docs, and ordinary tests synced with future API slices; add doctests only after source-span/example execution policy is ready. |

Overall basic hosted stdlib completion is about **90%**. The remaining work is
mostly final polish, trait-driven ergonomics, platform policy, and runtime
blocking/readiness behavior rather than missing first-use APIs.

## Already Completed From Older Gap Lists

Several old gap lists still mention work that is now done:

- `std::io::print`, `println`, `eprint`, and `eprintln` are natural
  `Result[(), Error]` APIs. The old `*_text` forms remain compatibility
  helpers.
- `BufWriter` has explicit `flush()` and best-effort drop-time flushing.
- `std::fs` has natural Result names for open/read/write/mutation/query paths,
  temp file and temp directory wrappers, hosted POSIX advisory file locks,
  structured `PathError`/`TwoPathError` detailed helpers, per-entry
  `DirEntryInfo` metadata snapshots, POSIX owner/group ids, and an explicit
  portable creation-time fallback policy.
- `std::process` has environment clear/inherit policy, typed exit status
  helpers, small stdout/stderr capture, output UTF-8 conversion, and execution
  helpers for file-backed stdin and `/dev/null`.
- `std::net` has IPv6 TCP/UDP hooks, bracketed IPv6 endpoint parsing,
  `resolve_all`/`to_socket_addrs`, natural `bind`/`connect`, UDP source
  address receive helpers, and common socket options such as keepalive,
  reuse-port, broadcast, and buffer sizes.
- `std::sync` has guard types, no-poison policy docs, Result compare-exchange,
  fallible `OnceLock` initialization status, timeout-shaped condvar waiting,
  and Result channel send/receive errors.
- `std::thread` has a separate owning `JoinHandle`, detach, join errors, and
  Result available-parallelism helpers for the current `fn() -> i64` entry
  model.
- `std::string`, `std::parse`, and `std::encoding` have the parser-friendly and
  Result-returning helpers needed for small Ari manifest/config parsers.

## Current Remaining Work

This is the live remaining list after the current source/docs/API-manifest
audit. Items here are not fuzzing, property testing, sanitizer setup, soak
tests, or CI matrix work.

| Area | Still remaining |
| --- | --- |
| `std::fs` | ACLs, owner/group name lookup, non-alias platform-specific path values, Windows path/symlink/locking policy, and exact partial-read error reporting after the runtime exposes read failure separately from EOF. |
| `std::io` | Zone-owning buffered constructors, low-level byte-hook read-error taxonomy, and deeper streaming formatter integration. |
| `std::process` | Readiness-based large stdout/stderr draining, pipe-backed streaming stdin, parent-visible child setup error reporting, Windows mapping, richer platform status fields, un-waited child lifecycle policy, and possible ref-mut fluent command aliases that do not hide allocation. |
| `std::net` | Fuller DNS iteration, service-name ports, canonical names, timeout-specific errors, readiness/poll, linger, TTL/hop-limit, multicast, Unix datagram, peer credentials, and keeping TLS as a future package-layer decision. |
| `std::sync` | Value-protecting `Mutex[T]`/`RwLock[T]`, optional poison-aware lock types, futex or OS blocking waits, configurable channel capacity, sender-counted close semantics, semaphores, and target-native generic atomics. |
| `std::thread` | Generic `JoinHandle[T]`, captured closures, scoped threads, compiler-level thread-local declarations, generic thread return payloads, and lower-friction compiler-owned TLS ergonomics. |
| `std::path` | Non-alias platform-specific `PathBuf`, Windows drive/UNC rules, OS-string conversion policy beyond POSIX bytes, and richer component kinds. |
| `std::collections` | Trait-driven default constructors, borrowed lookup, lazy set algebra, tree retain/range mutation, split/append operations, and first-class reference-valued iterator items. |
| `std::fmt` | Variadic/default-zone formatting policy, fully streaming template writers, structured logging, scoped filters, and capture/backtrace integration. |
| `std::string` | Owned validated `Utf8String`/`OsString` policy, Unicode normalization/transcoding, grapheme iteration, and locale-sensitive case policy. A dedicated string-builder type is intentionally not planned for the basic slice. |
| `std::parse` | Exact IEEE-754 boundary rounding diagnostics for decimal spellings adjacent to max finite/min subnormal values, and richer parse error taxonomy only where callers need it. |
| `std::encoding` | Unicode normalization/transcoding and optional compression policy outside the core encoding module. |
| `union by` language idea | Syntax is chosen for roadmap discussion, but construction, exhaustiveness, active-arm drop, narrowing, layout, and diagnostics remain compiler work. |

## Language Roadmap Interaction

The discriminant-linked struct field idea is compiler roadmap work, not a
stdlib API. The chosen spelling is:

```ari
fragment: union by security.cipher_type {
  stream => GenericStreamCipher,
  block => GenericBlockCipher,
  aead => GenericAEADCipher,
}
```

It remains unimplemented. Future compiler work must define construction,
exhaustiveness, active-arm drop, narrowing after discriminant checks, layout,
and diagnostics before stdlib code can depend on it.
