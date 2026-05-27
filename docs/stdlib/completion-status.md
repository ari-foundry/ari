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
| `std::fs` | 99% | Result-first file open/read/write/copy/mutation, `OpenOptions`, metadata and no-follow metadata, POSIX owner/group ids, explicit creation-time fallback policy, permissions/mode, directory entries with lazy metadata, per-entry `DirEntryInfo` Result snapshots, structured `PathError`/`TwoPathError` detailed helpers, symlinks, canonicalization, temp files/dirs, POSIX advisory locks, IO adapters, EOF/error-splitting host read hooks, and focused tests/docs/API manifest coverage. | Remaining work is outside the basic hosted slice: ACLs, owner/group name lookup, platform-specific owned path values, and Windows path/symlink/locking policy. |
| `std::io` | 99% | `Reader`/`Writer`/`Seek`, `ReadByte` one-byte status with EOF versus adapter-error propagation, OS-backed EOF/error-splitting read hooks, natural `print`/`println`/`eprint`/`eprintln`, `read_one`, `read`, `write`, `write_all`, `flush`, `read_exact`, `read_all`, `read_to_string`, pipes, `PipeWriter` close-on-exec control, cursors, borrowed-buffer and zone-backed buffered readers/writers, writer methods, and explicit `BufWriter` flush behavior. | Target/runtime-specific errno categories beyond the current adapter error and broader streaming formatter integration. |
| `std::process` | 95% | `Command`, `Child`, typed `ExitStatus`/`ExitCode`/`Signal`, POSIX wait-status detail accessors for stopped/continued/core-dump states, mutating and chainable builder helpers, environment inherit/clear policy, current-dir setup, parent-visible setup/exec errors through close-on-exec error pipes, typed status/output capture with readiness-drained stdout/stderr pipes, UTF-8 output conversion, temp paths, child stdin file and `/dev/null` redirection, bounded pipe-backed stdin for status-style child runs, interactive `ChildPipes` stdin/stdout/stderr handles, and explicit `detach` lifecycle vocabulary. | Windows process mapping, process groups/daemon helpers or a stronger background-reaping policy, and possible ref-mut fluent aliases once the command builder owns an allocation policy that does not require passing a zone. |
| `std::net` | 95% | IPv4/IPv6 address values, hosted IPv4/IPv6 DNS lookup, bracketed IPv6 endpoint parsing, `resolve_all`/`to_socket_addrs`, deterministic well-known service-name port lookup, service-name resolver wrappers, IPv4/IPv6 TCP and UDP sockets, Unix streams and Unix datagrams, socket IO adapters, UDP `send_to`/`recv_from`/`peek_from`, common options, close-on-exec, nonblocking, timeouts, timeout/would-block/interrupted/refused error predicates, single-descriptor readiness probes, keepalive, linger, reuse port, broadcast, IPv4 multicast loop/TTL/membership helpers, buffer sizes, IPv4 TTL, and IPv6 hop-limit. | Full DNS result iteration beyond the current small resolver list, host service-database lookup, canonical names, multi-descriptor poll/event-loop APIs, IPv6 multicast policy, Unix peer credentials, and package-layer TLS policy. |
| `std::thread` | 87% | Result-returning spawn/join for `fn() -> i64`, `JoinHandle`, `JoinError`, typed `ThreadResult` wrappers, `ThreadId`, `Builder`, detach, advisory completion, sleep/yield, available parallelism, and explicit `ThreadLocal[T]` with capacity introspection and recoverable `try_set`/`get_or_try_init`. | Generic `JoinHandle[T]`, captured closures, scoped threads, compiler-level thread-local declarations, generic thread return payloads, and less zone/capacity-heavy TLS ergonomics. |
| `std::sync` | 95% | Concrete atomics with Result compare-exchange, no-poison `RawMutex`/`RawRwLock` manual guards with explicit drop unlock, value-protecting `Mutex[T]`/`RwLock[T]` payload guards, `Once`, `OnceLock` fallible init status, `Condvar` timeout shape, `Barrier`, `Semaphore` permit guards, and configurable bounded MPSC channels with Result send/recv/recv-timeout errors, FIFO receives, clonable sender handles, and sender-counted close semantics. | Optional poison-aware types, futex or OS blocking waits, unbounded channel policy, and target-native generic atomics. |
| Text, string, encoding, path, C boundary | 95% | Byte-oriented `String`, parser-friendly string helpers, UTF-8 codepoint helpers, explicit borrowed `Utf8`/owned `Utf8String`, borrowed `OsStr`/owned `OsString`, `CStr`/`CString`, UTF-8 validation/decode/encode with named diagnostics, hex/base64 including MIME and URL-safe shapes with named codec diagnostics, POSIX `PathBytes`, distinct owned POSIX `PathBuf`, joins, normalization, explicit Windows drive/UNC lexical classifiers, and kinded path components for Windows prefixes. | Platform-specific path and OS-string values, verbatim Windows path/OS-string policy, Unicode normalization/transcoding, grapheme/locale policy. No separate string-builder type is planned for the basic stdlib slice; byte `String` and `Vec[u8]` cover that role. |
| Collections and iterators | 91% | `Vec`, slices, iterators, algorithms, `HashMap`/`HashSet`, `TreeMap`/`TreeSet`, `Deque`, `RingBuffer`, `LinkedList`, `BinaryHeap`, `PriorityQueue`, entry APIs, drains, ordered boundaries, natural default-policy `new` constructors, explicit `with_hash`/`with_less` custom policy constructors, trait-driven `with_capacity` constructors, String-key borrowed byte lookup, mutable value cursors, and tree split/append helpers. | General borrowed lookup beyond String keys, lazy set algebra, tree retain/range mutation, and first-class reference-valued iterator items for tighter mutable iteration. |
| Formatting and logging | 86% | Display/Debug traits, scalar formatting, strict/fallible `FormatSpec`, `format_in!`, stdout/stderr value printing, writer-backed helpers, `concat2`, `concat3`, `format_value`, fixed-arity runtime template `format`/`format2`/`format3`, and matching writer helpers. | Variadic/default-zone formatting policy, fully streaming template writers, structured logging, scoped filters, and capture/backtrace integration. |
| Parse and encoding | 95% | Natural Result numeric parsers, bool/float parsing, underscore-aware numeric parsers, parse diagnostics with integer and float range offsets plus stable names/messages/predicates, finite/subnormal float boundary diagnostics, UTF-8 and codec diagnostics with stable names/messages, fallible owned decoders. | Richer parse error taxonomy only where callers prove they need it, normalization/transcoding, and optional compression outside the core encoding module. |
| Hosted-program basics | 92% | Args/env/current-dir/executable path, fs/io/parse/path/process/time/thread/sync/net basics are enough for small CLI tools and arix-style prototypes, including status-style child stdin and interactive process pipes. | Package-manager-specific polish, full resolver iteration, platform-specific path/process behavior, and scoped concurrency ergonomics. |
| Docs, manifest, examples, tests | 94% | Module guides, generated API index, manifest checks, focused normal regression tests, example index, and current completion tracking exist for the implemented stdlib surface. | Keep this page, module docs, generated API docs, and ordinary tests synced with future API slices; add doctests only after source-span/example execution policy is ready. |

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
  helpers, readiness-drained stdout/stderr capture, output UTF-8 conversion,
  execution helpers for file-backed stdin and `/dev/null`, bounded pipe-backed
  stdin, interactive stdin/stdout/stderr `ChildPipes`, explicit detach
  vocabulary, and parent-visible child setup/exec errors for fork-based command
  helpers.
- `std::net` has IPv6 TCP/UDP hooks, bracketed IPv6 endpoint parsing,
  `resolve_all`/`to_socket_addrs`, natural `bind`/`connect`, UDP source
  address receive helpers, single-descriptor readiness probes, and common
  socket options such as keepalive, linger, reuse-port, broadcast, buffer sizes, TTL,
  and hop-limit.
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
| `std::fs` | ACLs, owner/group name lookup, platform-specific path values, and Windows path/symlink/locking policy. |
| `std::io` | Target/runtime-specific errno categories beyond the current adapter error and deeper streaming formatter integration. |
| `std::process` | Windows mapping, process groups/daemon helpers or stronger background-reaping policy, and possible ref-mut fluent command aliases that do not hide allocation. |
| `std::net` | Fuller DNS iteration, host service-database lookup, canonical names, multi-descriptor poll/event-loop APIs, IPv6 multicast policy, Unix peer credentials, and keeping TLS as a future package-layer decision. |
| `std::sync` | Optional poison-aware lock types, futex or OS blocking waits, unbounded channel policy, and target-native generic atomics. |
| `std::thread` | Generic `JoinHandle[T]`, captured closures, scoped threads, compiler-level thread-local declarations, generic thread return payloads, and lower-friction compiler-owned TLS ergonomics. |
| `std::path` | Platform-specific `PathBuf`, verbatim Windows path policy, and OS-string conversion policy beyond POSIX bytes. |
| `std::collections` | General borrowed lookup beyond String keys, lazy set algebra, tree retain/range mutation, and first-class reference-valued iterator items. |
| `std::fmt` | Variadic/default-zone formatting policy, per-value streaming `Display` writers beyond the current `Display::format_in` string step, structured logging, scoped filters, and capture/backtrace integration. |
| `std::string` | Platform-specific `OsString` storage beyond the current POSIX byte wrapper, Unicode normalization/transcoding, grapheme iteration, and locale-sensitive case policy. A dedicated string-builder type is intentionally not planned for the basic slice. |
| `std::parse` | Future taxonomy splits backed by real caller needs; the current basic slice already covers natural Result parsers, stable diagnostic names/messages, byte offsets, and finite/subnormal float boundary checks. |
| `std::encoding` | Unicode normalization/transcoding and optional compression policy outside the core encoding module. |
| `union by` language idea | Syntax is chosen and the parser now reserves `union by` with a targeted diagnostic, but construction, exhaustiveness, active-arm drop, narrowing, layout, and positive execution support remain compiler work. |
| Structural capability parameters | The `fn save(x: has serialize() -> String)` idea is tracked as compiler capability `structural-capability-parameters`; the parser now emits a targeted diagnostic for the reserved spelling, while type checking, trait-quality diagnostics, lowering, and final interaction with normal trait bounds remain compiler work. |

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

It remains unusable in programs. The parser reserves the spelling and points
users back to ordinary enum payloads today. Future compiler work must define
construction, exhaustiveness, active-arm drop, narrowing after discriminant
checks, layout, and positive execution support before stdlib code can depend
on it.

Structural capability parameters are also language work rather than a stdlib
API. The exploratory shape is:

```ari
fn save(x: has serialize() -> String) {
  file.write(x.serialize())
}
```

It remains unusable in programs. The parser reserves the spelling and points
users back to named traits today. Future compiler work must define type
checking, dispatch, diagnostics, lowering, and the final interaction with
normal generic trait bounds before stdlib code can depend on it.

The compiler must still define whether this syntax desugars to anonymous trait
bounds, how method names are resolved, how diagnostics point users toward named
traits when that is clearer, and how it avoids introducing an `interface`
keyword or accidental dynamic dispatch.
