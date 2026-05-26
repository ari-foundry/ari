# Linux Platform Plan

This page tracks Linux-specific standard library and runtime work. It keeps
sharp OS concepts out of portable modules until Ari can model owned file
descriptors, errors, nonblocking behavior, and kernel-version differences
cleanly.

## Current Slice

Implemented now:

- `std::target` reports target triple, architecture, OS, environment/libc,
  object format, debug format, errno ABI, pointer width, C `long` width, TLS
  family, Linux syscall ABI family, and Linux API-family predicates.
- `std::env`, `std::fs`, `std::process`, `std::thread`, `std::sync`, and
  `std::time` provide the first hosted Linux/glibc-backed runtime hooks through
  the LLVM driver. `std::thread::available_parallelism_raw()` currently uses
  `sysconf(_SC_NPROCESSORS_ONLN)`, while the natural
  `std::thread::available_parallelism()` wrapper returns `Result[u64, Error]`
  and `available_parallelism_or(default)` applies a caller-chosen fallback.
- `std::process` currently covers process id, user id, group id, explicit exit,
  explicit abort, POSIX `fork`/`wait`, and the first
  `Command`/`Child`/`ExitStatus`/`Output` builder over `fork`, `waitpid`,
  `execvp`, `setenv`, `chdir`, `pipe`, `dup2`, and `kill`.
- `std::mem::page_size()` reports the hosted runtime page size for alignment
  and future mapping work.
- `std::random::entropy()` / `fill(values)` return `Result` values, use the
  hosted Linux `getrandom` path first, and fall back to `/dev/urandom` for OS
  seed material; `_unchecked` helpers keep strict hard-fail compatibility.
- `std::c` provides the first hosted C boundary helpers: POSIX `errno`
  reading through the glibc errno location, `std::error` mapping, borrowed and
  owned C strings, and dynamic loading wrappers over `dlopen`, `dlsym`,
  `dlclose`, and `dlerror`.
- `std::net` provides IP and socket-address values plus hosted IPv4/IPv6 DNS
  lookup, IPv4/IPv6 TCP listener/stream handles, IPv4/IPv6 UDP single-byte
  datagrams, Unix stream sockets, nonblocking flags, `std::time::Duration`
  timeouts with raw millisecond compatibility helpers, and stream shutdown
  over Linux sockets.
- Hosted executables currently rely on the platform CRT and dynamic linker;
  Ari emits LLVM IR and lets the LLVM driver link.

## Target ABI

The compiler target layer currently recognizes these important Linux 64-bit
families:

| Target Family | `std::target` Arch | Syscall ABI Class | Current Status |
| --- | --- | --- | --- |
| `x86_64-*-linux-*` | `X86_64` | `LinuxX86_64` | Runtime and execution tests focus here today. |
| `aarch64-*-linux-*` | `Aarch64` | `LinuxAarch64` | Target classification exists; runtime/link testing is roadmap work. |
| `riscv64-*-linux-*` | `Riscv64` | `LinuxRiscv64` | Target classification exists; runtime/link testing is roadmap work. |

32-bit x86/ARM can be classified by the compiler, but the standard library
roadmap should prioritize 64-bit Linux first because pointer width, aggregate
ABI, atomics, TLS, and syscall calling conventions stay simpler and more
useful for modern systems work.

## Libc And Linking

| Topic | Current Status | Roadmap |
| --- | --- | --- |
| glibc target | `*-linux-gnu` is recognized as `Env::Gnu`, and `target::uses_glibc()` is true on Linux GNU targets. | Keep hosted Linux tests here until musl CI exists. |
| musl target | `*-linux-musl` is recognized as `Env::Musl`, and `target::uses_musl()` is true on Linux musl targets. | Add link tests when a musl toolchain is available. |
| dynamic linking | Current normal executables are linked by the LLVM driver and host defaults. | Add explicit driver flags only after the build-profile contract is designed. |
| static linking | Not owned by Ari yet. | Add a driver flag, target checks, and runtime support notes before promising it. |
| PIE | Not owned by Ari yet. | Expose only after linker flags and default profile behavior are stable. |
| RELRO | Not owned by Ari yet. | Treat as a linker hardening profile, not a source stdlib API. |
| stack protector | Not emitted by Ari directly. | Needs stack protector runtime hooks or a documented toolchain dependency. |

## Runtime And Object Formats

| Topic | Current Status | Roadmap |
| --- | --- | --- |
| TLS | Ari emits a `thread_local` runtime thread-id slot for hosted targets. | Freestanding startup and custom thread runtimes need explicit TLS initialization. |
| vDSO | `target::has_vdso()` is true for Linux targets as an API-family fact. | Runtime clock wrappers may use vDSO through libc/toolchain paths; direct vDSO use is future work. |
| syscall ABI | `target::syscall_abi()` classifies x86_64, aarch64, and riscv64 Linux. | Raw syscall wrappers should wait for `std::os::linux` error and handle policy. |
| errno ABI | `target::uses_posix_errno()` is true for Unix-style thread-local errno, and `std::c::errno()` reads the hosted glibc errno location. | Generalize beyond the hosted glibc path when more libc targets are tested; add `std::os::errno` only when fallible wrappers need a stable error value. |
| ELF | `target::uses_elf()` is true on Linux targets. | Add object/dynamic symbol tests where ABI changes are observable. |
| DWARF | `target::uses_dwarf()` is true on Linux targets. | Backtrace and symbolization work should depend on the runtime-support roadmap. |

## Linux Filesystems And Kernel Views

| API Family | Current Status | Future Module Shape |
| --- | --- | --- |
| procfs | `target::has_procfs()` reports the Linux family; `std::env::executable_path()` currently reads `/proc/self/exe` and reports lookup failure as `Result`. | `std::os::linux::proc` should expose typed reads only after file/path/error policy improves. |
| sysfs | `target::has_sysfs()` reports the Linux family. | Keep as roadmap until a safe text/file parser pattern exists. |
| cgroups | `target::has_cgroups_api()` reports the Linux family only. | Optional future wrapper; mount/layout differences should be explicit. |
| namespaces | `target::has_namespaces_api()` reports the Linux family only. | Optional future process/capability API; requires privilege and error modeling. |
| seccomp | `target::has_seccomp_api()` reports the Linux family only. | Optional future security API; keep out of portable process helpers. |
| capabilities | `target::has_capabilities_api()` reports the Linux family only. | Optional future security API; needs dedicated docs and tests. |

## Linux Event And Descriptor APIs

| API Family | Current Status | Future Module Shape |
| --- | --- | --- |
| raw syscall wrapper | Not exposed. | Future `std::os::linux::syscall` should return typed error values and stay behind target guards. |
| errno mapping | `std::target::uses_posix_errno()` reports the ABI family, and `std::c::errno()`/`std::c::error()` provide the first hosted runtime accessor and `std::error` bridge. | Add portable `std::os::errno` together with the first fallible descriptor wrappers and non-glibc tests. |
| getrandom | `std::random::entropy()` and `std::random::fill(values)` use the hosted libc/syscall path and return `std::error::Error` on recoverable failure; `entropy_unchecked()` and `fill_unchecked(values)` hard-fail if entropy cannot be read. | Add broader failure injection tests and keep cryptographic streams behind a separate policy. |
| `/dev/urandom` | Used as the random fallback when `getrandom` cannot make progress. | Keep as fallback; do not expose raw device reads as portable API. |
| file descriptor abstraction | `std::os::Fd` is a public non-owning descriptor view, `std::os::OwnedFd` owns exactly-one-close responsibility, `OwnedFd::try_clone()` duplicates that ownership through `dup`, `std::os::pipe()` creates an owned read/write `Pipe`, `std::os::poll_read`/`poll_write` expose single-descriptor readiness probes, and `std::fs::File.descriptor()` exposes file handles through the borrowed view. | Add duplicate-with-flags and richer error policy before exposing broad `fcntl` or epoll. |
| file open options | `std::fs::OpenOptions` lowers read/write/append/truncate/create/create-new policy to hosted `open(2)` flags. | Add typed filesystem errors before exposing more platform-specific open flags. |
| file open errors | `std::fs::open`, `create`, and `OpenOptions::open` return direct `Result[File, Error]`; `*_optional`/`try_*` helpers intentionally discard the reason, `_unchecked` helpers preserve invalid-handle compatibility, `*_raw` variants preserve compact raw errno values for compatibility, and `_detailed` helpers add operation/path context through `PathError` or `TwoPathError`. | Add broader platform-specific open flags in `std::os` before growing the portable `OpenOptions` surface. |
| file read errors | `std::fs::read` and `read_to_string` return direct `Result[String, Error]` for open, mid-read, and close failures while preserving the byte-oriented read loop; the hosted `read_byte` hook now reports `read(2) == 0` as EOF and `read(2) < 0` as an adapter error sentinel. | Add richer typed filesystem errors once the portable error-field policy is ready. |
| file query errors | `metadata`, `symlink_metadata`, `file_type`, `mode`, `canonicalize`, `read_link`, `open_dir`, `read_dir`, and `read_dir_entries` preserve hosted `stat(2)`, `lstat(2)`, `realpath(3)`, `readlink(2)`, and directory-open/close failures as `Error`; `_detailed` helpers add path/action fields, and `read_dir_info` keeps per-entry metadata/file-type `Result` snapshots. | Add richer query fields and platform-specific metadata policy. |
| file mutation errors | `std::fs::remove`, `rename`, `create_dir`, and `remove_dir` return direct `Result[(), Error]`; raw variants stay compatibility-only, and `_detailed` variants preserve the failing operation and path/source-target arguments. | Add richer platform-specific mutation flags in `std::os` if needed. |
| file byte-count errors | `std::fs::write`, `append`, and `copy` return `Result[i64, Error]`, preserving successful byte counts while surfacing hosted open/read/write/close failures as `Error`. | Raw byte-count variants remain compatibility-only. |
| file seek | `std::fs::position(file)` and `std::fs::seek(file, offset)` are backed by hosted `lseek(2)` and surface through `std::io::Seek` on `File`. | Add richer seek origins and typed errors after the filesystem layer grows `Result`-returning operations. |
| symbolic-link target reads | `std::fs::read_link(ref mut zone, path)` is backed by hosted `readlink(2)` and copies the stored link target into a zone-owned `String`; `try_read_link` discards the reason, while `symlink_metadata(path)` and `is_symlink(path)` use hosted `lstat(2)` for no-follow metadata, including access/modification/status-change timestamps and POSIX owner/group ids. | Add richer link policy before exposing platform-specific link management. |
| close-on-exec | `OwnedFd::close_on_exec()` and `set_close_on_exec(enabled)` are backed by hosted `fcntl(F_GETFD/F_SETFD)`. | Add close-on-exec-at-creation/dup where host APIs support it, then default new descriptors to close-on-exec where possible. |
| nonblocking mode | `OwnedFd::is_nonblocking()` and `set_nonblocking(enabled)` are backed by hosted `fcntl(F_GETFL/F_SETFL)`. `std::net` exposes those helpers on TCP, UDP, and Unix socket handles. | Add readiness APIs and document retry behavior once richer IO/net error values land. |
| DNS lookup | `std::net::lookup_v4`, `lookup_v6`, `"host:port"` and bracketed `"[host]:port"` `resolve`, `to_socket_addrs`, service-name helpers, and the `ToSocketAddrs` seed use hosted `getaddrinfo` for a one-address IPv4/IPv6 slice; `_optional`/`try_*` helpers intentionally discard error detail and `_raw` helpers expose compact raw compatibility errors. Service-name helpers use a deterministic stdlib table, not `/etc/services`. | Add multi-address result storage, host service-database lookup, canonical-name policy, and detailed resolver error mapping. |
| TCP sockets | `std::net::listen`/`connect`, explicit `tcp_listen`/`tcp_connect`, IPv6-specific `tcp_listen_v6`/`tcp_connect_v6`, host-port `connect_host`/`tcp_connect_host`, `TcpListener`, and `TcpStream` use hosted `socket`, `bind`, `listen`, `accept`, `connect`, `getsockname`, `getpeername`, `setsockopt`, `getsockopt`, and `shutdown` for IPv4 and IPv6 handles. TCP listeners/streams expose local socket-address lookup, IPv6-specific local/peer helpers, TCP listener reuse-address policy, TCP stream peer-address lookup, nodelay, keepalive, linger, buffer sizes, IPv4 TTL, IPv6 hop-limit, and `TcpStream` adapts to `std::io::Reader`/`Writer` through descriptor byte reads/writes and exposes method-style `read_exact`/`write_all` stream buffer helpers. | Add lower-overhead bulk send/recv runtime hooks, multicast-style remaining options, and more operation-specific errors. |
| UDP sockets | `std::net::udp_bind`, `udp_bind_v6`, and `UdpSocket` use hosted IPv4/IPv6 datagram sockets, `bind`, `getsockname`, `sendto`, `recvfrom`, and timeout/nonblocking/reuse-address descriptor policy. It exposes local socket-address lookup after bind, including `local_addr_v6`, source-address receive helpers, connected UDP send/recv, buffer sizes, broadcast, IPv4 TTL, and IPv6 hop-limit. | Add multicast membership and richer operation-specific errors. |
| Unix stream sockets | `std::net::unix_listen`/`unix_connect`, `UnixListener`, and `UnixStream` use hosted `AF_UNIX` stream sockets with path-based bind/connect/accept plus common descriptor flags, IO adapters, and method-style `read_exact`/`write_all` helpers. | Add datagram sockets, abstract namespace policy, peer credentials, lower-overhead bulk send/recv runtime hooks, and platform guards for non-Linux Unix targets. |
| pipe | `std::os::pipe()` is backed by the hosted `pipe(2)` ABI and returns an owning `Pipe` value with separate read/write ends; `std::io::pipe()` adapts it into `Reader`/`Writer` ends. | Add `pipe2`-style close-on-exec/nonblocking-at-creation policy where host APIs support it. |
| fcntl | Not exposed. | Future low-level descriptor module with typed flag helpers. |
| ioctl | Not exposed. | Keep optional and narrow; prefer typed wrappers for common devices over a raw catch-all. |
| poll/select | `std::os::poll_read` and `poll_write` expose a small `poll(2)` readiness seed over one descriptor, and `std::net` forwards it through listener/stream/socket `*_ready` helpers. | Multi-descriptor poll/event loops and Linux `epoll` remain future work; `select` is legacy and should stay compatibility-oriented. |
| epoll | `target::has_epoll()` reports Linux family support. | `std::os::linux::epoll` with owned epoll fd, `add`, `modify`, `remove`, and `wait`. |
| inotify | `target::has_inotify()` reports Linux family support. | Owned inotify fd plus event buffer parsing. |
| fanotify | `target::has_fanotify_api()` reports Linux family support only. | Optional; permission-heavy and likely not a first runtime slice. |
| eventfd | `target::has_eventfd()` reports Linux family support. | Owned fd wrapper with read/write counter helpers. |
| timerfd | `target::has_timerfd()` reports Linux family support. | Timer handle integrated with `std::time::Duration`. |
| signalfd | `target::has_signalfd()` reports Linux family support. | Needs signal-mask policy before implementation. |
| pidfd | `target::has_pidfd_api()` reports Linux family support only. | Future process handle path; kernel version can still reject it. |
| memfd | `target::has_memfd()` reports Linux family support. | Owned anonymous file descriptor wrapper. |
| futex | Not exposed as portable `std`. | Internal runtime primitive for future blocking `Mutex`, `Condvar`, `RwLock`, barriers, semaphores, and channels after ownership and wait/wake policy are documented. |
| io_uring | `target::has_io_uring_api()` reports Linux family support only. | Optional advanced async API after ownership and buffer pinning rules exist. |

## Process, Signals, And Memory Mapping

| API Family | Current Status | Future Module Shape |
| --- | --- | --- |
| argv/env | Portable surface exists through `std::context` and `std::env`. | Keep user-facing wrappers portable; reserve raw `environ` access for `std::os` if needed. |
| current process info | `std::process::id`, `uid`, `gid`, and `is_root` exist. | Add parent id, session/process-group helpers only with clear platform policy. |
| exit/abort | `std::process::exit` and `abort` exist. | Document destructor/cleanup limits anywhere higher-level runtime teardown is added. |
| spawn | `Command::spawn` and module-level `process::spawn(ref command)` are exposed through a portable-looking builder backed by POSIX `fork`/`execvp` today. A close-on-exec setup pipe reports child `chdir`, `dup2`, and `execvp` failures back to the parent. | Add Windows mapping and decide interactive stdin ownership before broadening the API. |
| output capture and stdin | `Command::output_in(ref mut zone)` captures child stdout/stderr into a zone-backed `Output` handle using `pipe(2)`, `dup2(2)`, and descriptor readiness to drain both streams. `status_with_stdin`, `status_with_stdin_string`, `spawn_with_stdin_file`, `status_with_stdin_file`, and their `/dev/null`/byte/path variants redirect child stdin at execution time and surface setup errors to the parent. | Add interactive stdin/stdout/stderr handles after ownership policy is clear. |
| fork | `std::process::fork` exists as a POSIX `Result` slice, with `fork_raw` for old sentinel-style code. | Keep marked as sharp; fork-with-threads and async-signal-safe limitations need more docs. |
| exec | `Command::exec` replaces the current process after applying child setup. | Add richer setup policy and document noreturn behavior in more examples. |
| wait | `std::process::wait_status`, `Command::exit_status`, and `Child::wait_status` preserve typed `ExitStatus` values for normal exits and signal termination. Compatibility `wait_raw`, `Command::status`, and `Child::wait` still expose normal exit-code oriented paths. | Add richer platform-specific status fields and Windows mapping. |
| kill | `process::kill`, `process::terminate`, `Child::kill`, and `Child::terminate` are exposed. | Add signal constants and more structured permission/error mapping. |
| working directory | `std::env::current_dir`, `set_current_dir`, `std::fs::try_canonicalize`, and `std::fs::canonicalize` exist. | Owned path values should wrap the existing `std::path`/`std::fs` split. |
| daemon helpers | Not exposed. | Optional; should be policy-heavy and probably separate from core process APIs. |
| signal mask | Not exposed. | Future `std::os::signal` with mask values and clear thread/process scope. |
| sigaction | Not exposed. | Needs function-pointer ABI, signal-safe restrictions, and handler lifetime policy. |
| raise/kill signals | Not exposed. | Add after signal constants and status/error mapping. |
| SIGINT/SIGTERM handling | Not exposed. | Prefer small, documented helpers after signal action policy exists. |
| sigaltstack | Not exposed. | Optional advanced API; depends on raw memory and stack lifetime policy. |
| signal-safe docs | Not complete yet. | Document async-signal-safe limits before allowing user handlers. |
| mmap/anonymous mapping | Not exposed. | Future owned mapping type with `munmap` in drop-like cleanup policy. |
| file mapping | Not exposed. | Depends on public descriptor ownership and path/file error reporting. |
| mprotect/msync/mlock/madvise | Not exposed. | Add as methods on owned mappings with platform flags. |
| page size | `std::mem::page_size()` returns the hosted runtime page size. | Use it as the alignment seed for future owned mapping APIs. |

## Implementation Roadmap

1. Keep `std::target` current with compiler target support.
2. Add explicit `std::os` docs before adding raw wrappers.
3. Grow `OwnedFd` with duplicate-with-flags and `errno`/error policy.
4. Add process expansion in order: interactive stdin/stdout/stderr handles,
   richer wait status, then portable `spawn` mapping.
5. Expand descriptor readiness from single-descriptor `poll_read`/`poll_write`
   into Linux `epoll`, `eventfd`, `timerfd`, and `memfd`.
6. Add memory mapping only after descriptor/error policy and owned mapping
   cleanup are designed.
7. Add signal mask/action support only after signal-safe limitations are fully
   documented.
8. Add optional `pidfd`, `inotify`, and eventually `io_uring` once the runtime
   can test kernel-version-dependent behavior.
9. Keep cgroups, namespaces, seccomp, capabilities, and fanotify optional until
   there is a real application need and privilege-aware tests.
