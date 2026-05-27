# Standard Library Example Index

This page is the quick route from "what module should I use?" to "what does a
real call look like?" The focused module guides hold the detailed examples and
the test files hold executable examples that should keep compiling as the
library evolves.

## How To Use This Page

1. Pick the module by task, not by type name.
2. Read the focused module guide for the ownership, allocation, and failure
   rules.
3. Open the representative test when you need an executable example.
4. Check [stability.md](stability.md) before depending on platform-backed or
   experimental behavior.
5. Use [generated/api-index.md](generated/api-index.md) when you need the exact
   public spelling.

## First Examples To Read

| Task | Start Here | Why |
| --- | --- | --- |
| Handle absence or recoverable failure | [std::option and std::result](modules/option-result.md) | Almost every collection, parser, and OS wrapper uses these shapes. |
| Build a growable buffer | [std::vec](modules/vec.md) and [Slice[T]](modules/slice.md) | `Vec[T]` owns storage, while `Slice[T]` borrows contiguous ranges. |
| Format values and diagnostics | [std::fmt](modules/fmt.md), [std::io](modules/io.md), [std::log](modules/log.md) | These show trait-backed formatting, writers, and stderr diagnostics. |
| Work with files | [std::fs](modules/fs.md), [std::path](modules/path.md), [std::error](modules/error.md) | File APIs should teach typed `Result[..., Error]` first. |
| Run a command | [std::process](modules/process.md) | `Command`, `Child`, `ExitStatus`, temp paths, and output capture live here. |
| Open a socket | [std::net](modules/net.md) | TCP, UDP, DNS, timeout, nonblocking, shutdown, and socket options are grouped here. |
| Write a test | [std::test](modules/test.md) and [testing.md](testing.md) | Use `@test`, reports, assertions, temp paths, snapshots, and filters. |
| Share ownership or interior state | [std::rc](modules/rc.md), [std::cell](modules/cell.md), [std::sync](modules/sync.md) | These modules explain shared handles, interior mutation, and thread-safe coordination. |

## Module Example Map

| Module | Main Example Theme | Representative Test |
| --- | --- | --- |
| root `std` | Prelude aliases, assertions, slices, ranges, formatting entry points. | `prelude-option-result-methods.ari`, `prelude-slice-sequence.ari` |
| `std::algo` | Sorting, binary search, bounds, partition, copy/fill, dedup. | `std-algo-slice-helpers.ari`, `std-algo-by-helpers.ari` |
| `std::ascii` | Byte classification, ASCII case operations, prefix parsers. | `std-ascii-class-helpers.ari`, `std-ascii-prefix-parsers.ari` |
| `std::bits` | Bit masks, rotations, alignment, population counts, scans. | `std-bits-rotate-helpers.ari`, `std-bits-alignment-policy.ari` |
| `std::boxed` | Zone-backed single-value ownership, move-out, refill, copy-to-zone. | `std-boxed-box.ari`, `std-boxed-try-take.ari` |
| `std::c` | C strings, errno, dynamic loading, typed symbols. | `std-c-interop.ari`, `std-c-dynamic-function.ari` |
| `std::cell` | `Cell`, `RefCell`, `OnceCell`, and lazy initialization. | `std-cell-basic.ari` |
| `std::cmp` | Ordering traits, min/max/clamp, inclusive predicates. | `std-cmp-value-helpers.ari` |
| `std::collections` | Sets, maps, deques, lists, heaps, map entry handles. | `std-collections-map-entry-api.ari`, `std-collections-tree.ari` |
| `std::context` | Runtime arguments, startup paths, thread id. | `std-context-args.ari` |
| `std::convert` | `from`, `into`, `identity`, and conversion trait patterns. | `std-convert-value-helpers.ari` |
| `std::encoding` | UTF-8/UTF-16 validation, hex, base64. | `std-encoding-text.ari`, `std-encoding-codec.ari` |
| `std::env` | Arguments, environment variables, current directory, executable path. | `std-env-vars.ari`, `std-env-paths.ari` |
| `std::error` | Shared error kinds, errno mapping, module aliases, typed results. | `std-error-basic.ari`, `std-error-integration.ari` |
| `std::fmt` | `Debug`, `Display`, format specs, runtime templates, writer-backed formatting. | `std-fmt-format-spec.ari`, `std-fmt-concat-format-value.ari`, `std-fmt-debug-values.ari` |
| `std::fs` | Open options, file handles, metadata, directories, links, typed errors. | `std-fs-open-result.ari`, `std-fs-read-dir.ari` |
| `std::hash` | Deterministic hashing and collection hash helpers. | `std-hash-basic.ari` |
| `std::input` | Stdin byte and line input with `Option` EOF handling. | `std-input-byte-option.ari` |
| `std::io` | Reader/writer/seek traits, buffers, cursors, pipes, exact reads. | `std-io-traits-cursor.ari`, `std-io-buffered.ari` |
| `std::iter` | Iterator traits, adapters, consumers, collect and extend. | `std-iter-adapters.ari`, `std-iter-consumers.ari` |
| `std::log` | Level-prefixed stderr diagnostics. | `std-log-basic.ari` |
| `std::math` | Checked, saturating, wrapping, overflowing, and rounded integer math. | `std-math-checked-saturating.ari` |
| `std::mem` | Layout, pointer operations, byte memory, page size. | `std-mem-pointer-ops.ari`, `std-mem-byte-ops.ari` |
| `std::net` | Addresses, DNS, TCP, UDP, Unix streams, timeout and nonblocking options. | `std-net-tcp-loopback.ari`, `std-net-udp-socket.ari` |
| `std::os` | File-descriptor ownership, close-on-exec, nonblocking, pipes. | `std-os-owned-fd.ari`, `std-os-pipe.ari` |
| `std::parse` | Whole-input integer, radix, bool, and float parsers. | `std-parse-basic.ari` |
| `std::path` | Path byte views, components, join, lexical normalization. | `std-path-basic.ari`, `std-path-components.ari` |
| `std::process` | `Command`, `Child`, status, output capture, fork/wait, temp paths. | `std-process-command.ari`, `std-process-output.ari` |
| `std::random` | OS entropy and deterministic `Prng`. | `std-random-basic.ari`, `std-random-result.ari` |
| `std::rc` | `Rc`, `Arc`, `Weak`, counts, downgrade, upgrade. | `std-rc-arc-weak.ari` |
| `std::string` | Owned byte strings, UTF-8/OS/C string views, splitting, joining, parsing. | `std-string-natural-api.ari`, `std-string-text-kinds.ari` |
| `std::sync` | Atomics, mutexes, rwlocks, once, once-lock, condvar, barrier, channels. | `std-sync-concurrency-api.ari` |
| `std::target` | Target triple, architecture, OS, ABI, object/debug formats. | `std-target-basic.ari`, `std-target-linux64.ari` |
| `std::test` | Reports, equality checks, temp paths, snapshots, benchmarks, `@test`. | `std-test-report.ari` |
| `std::thread` | Spawn, join, raw-data entries, scoped join groups, builder, sleep, yield, thread-local values. | `std-thread-basic.ari`, `std-thread-builder.ari`, `std-thread-scope-raw.ari` |
| `std::time` | Durations, instants, deadlines, UTC calendar conversion. | `std-time-basic.ari`, `std-time-timeout.ari` |
| `std::vec` | Capacity, reserve, append, extend, drain, splice, split, dedup. | `std-vec-complete-convenience-api.ari` |
| `std::zone` | Explicit allocation capability, placement, promotion, reset, destroy. | `std-zone-alloc-array.ari` |

If a module guide does not include a small source snippet yet, treat that as a
documentation bug. Add the snippet and connect it to a focused test in the same
change.
