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
| Error API model | 88% | Natural `Result[..., std::error::Error]` APIs are the default for `fs`, `io`, `net`, `process`, path-state `env`, parse, and encoding. Compatibility APIs are explicit through `_optional`, `_or_default`, `_bool`, `_unchecked`, `_raw`, or `_raw_result`. | Keep retiring or quarantining old `*_result` migration aliases as callers move. Add Windows error mapping and optional owned messages or structured fields only after platform policy is ready. |
| `std::fs` | 90% | Result-first file open/read/write/copy/mutation, `OpenOptions`, metadata and no-follow metadata, timestamps, permissions/mode, directory entries with lazy metadata, symlinks, canonicalization, temp files/dirs, advisory locks, and IO adapters. | Richer per-entry directory errors, owner/group/ACL policy, portable creation/birth time, portable mandatory or Windows locking policy, platform-specific symlink policy, and richer structured filesystem errors. |
| `std::io` | 92% | `Reader`/`Writer`/`Seek`, natural `print`/`println`/`eprint`/`eprintln`, `write`, `write_all`, `flush`, `read_exact`, `read_all`, `read_to_string`, pipes, cursors, buffered readers/writers, and best-effort `BufWriter` drop flush. | Zone-owning buffered constructors, richer EOF versus read-error policy for low-level byte hooks, and broader streaming formatter integration. |
| `std::process` | 86% | `Command`, `Child`, typed `ExitStatus`/`ExitCode`/`Signal`, chainable `with_*` builder helpers, environment inherit/clear policy, current-dir setup, typed status/output capture, UTF-8 output conversion, temp paths, child stdin file and `/dev/null` redirection. | Readiness-based large stdout/stderr draining, streaming pipe-backed stdin, parent-visible child setup error pipe, Windows process mapping, richer platform status fields, and stronger lifecycle policy for un-waited children. |
| `std::net` | 84% | IPv4/IPv6 address values, hosted IPv4/IPv6 DNS lookup, bracketed IPv6 endpoint parsing, `resolve_all`/`to_socket_addrs`, IPv4/IPv6 TCP and UDP sockets, Unix streams, socket IO adapters, UDP `send_to`/`recv_from`/`peek_from`, common options, close-on-exec, nonblocking, timeouts, keepalive, reuse port, broadcast, and buffer sizes. | Full DNS result iteration, service-name ports, canonical names, timeout-specific errors, readiness/poll, linger, TTL/hop-limit, multicast, Unix datagram, peer credentials, and package-layer TLS policy. |
| `std::thread` | 80% | Result-returning spawn/join for `fn() -> i64`, `JoinHandle`, `JoinError`, `ThreadId`, `Builder`, detach, advisory completion, sleep/yield, available parallelism, and explicit `ThreadLocal[T]`. | Generic `JoinHandle[T]`, captured closures, scoped threads, compiler-level thread-local declarations, richer thread result/status values, and less zone/capacity-heavy TLS ergonomics. |
| `std::sync` | 84% | Concrete atomics with Result compare-exchange, no-poison `Mutex`/`RwLock` guards with drop unlock, `Once`, `OnceLock` fallible init status, `Condvar` timeout shape, `Barrier`, and capacity-1 MPSC channels with Result send/recv/recv-timeout errors and clonable sender handles. | Value-protecting `Mutex[T]`/`RwLock[T]`, optional poison-aware types, futex or OS blocking waits, configurable channel capacity, sender-counted close semantics, semaphores, and target-native generic atomics. |
| Text, string, encoding, path, C boundary | 88% | Byte-oriented `String`, parser-friendly string helpers, explicit `Utf8`, `OsStr`, `CStr`/`CString`, UTF-8 validation/decode/encode, hex/base64 including MIME and URL-safe shapes, POSIX `PathBytes`/`PathBuf` alias, joins and normalization. | Owned `Utf8String`/`OsString`, platform-specific `PathBuf`, Windows drive/UNC/OS-string policy, richer codec errors, Unicode normalization/transcoding, grapheme/locale policy. No separate builder type is planned for the basic stdlib slice; byte `String` and `Vec[u8]` cover that role. |
| Collections and iterators | 85% | `Vec`, slices, iterators, algorithms, `HashMap`/`HashSet`, `TreeMap`/`TreeSet`, `Deque`, `RingBuffer`, `LinkedList`, `BinaryHeap`, `PriorityQueue`, entry APIs, drains, ordered boundaries, explicit hash/comparator constructors, mutable value cursors. | Trait-driven default constructors such as `HashMap::new`, borrowed lookup, lazy set algebra, tree retain/range mutation, split/append operations, and first-class reference-valued iterator items for tighter mutable iteration. |
| Formatting and logging | 83% | Display/Debug traits, scalar formatting, strict/fallible `FormatSpec`, `format_in!`, stdout/stderr value printing, writer-backed helpers, `concat2`, `concat3`, and `format_value`. | General runtime template formatting, default-zone formatting policy, richer streaming writer formatter, structured logging, scoped filters, and capture/backtrace integration. |
| Parse and encoding | 90% | Natural Result numeric parsers, bool/float parsing, underscore-aware numeric parsers, parse diagnostics, UTF-8 and codec diagnostics, fallible owned decoders. | More precise float range diagnostics, richer parse error taxonomy where it matters, normalization/transcoding, and optional compression outside the core encoding module. |
| Hosted-program basics | 89% | Args/env/current-dir/executable path, fs/io/parse/path/process/time/thread/sync/net basics are enough for small CLI tools and arix-style prototypes. | Package-manager-specific polish, process streaming, full resolver iteration, platform-specific path/process behavior, and scoped concurrency ergonomics. |
| Docs, manifest, examples, tests | 91% | Module guides, generated API index, manifest checks, focused normal regression tests, and example index exist for the implemented stdlib surface. | Keep this page, module docs, generated API docs, and ordinary tests synced with future API slices; add doctests only after source-span/example execution policy is ready. |

Overall basic hosted stdlib completion is about **87%**. The remaining work is
mostly final polish, trait-driven ergonomics, platform policy, and runtime
blocking/readiness behavior rather than missing first-use APIs.

## Already Completed From Older Gap Lists

Several old gap lists still mention work that is now done:

- `std::io::print`, `println`, `eprint`, and `eprintln` are natural
  `Result[(), Error]` APIs. The old `*_text` forms remain compatibility
  helpers.
- `BufWriter` has explicit `flush()` and best-effort drop-time flushing.
- `std::fs` has natural Result names for open/read/write/mutation/query paths,
  temp file and temp directory wrappers, and hosted POSIX advisory file locks.
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
