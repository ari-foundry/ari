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
  the LLVM driver.
- `std::process` currently covers process id, user id, group id, explicit exit,
  explicit abort, and the first POSIX `fork`/`wait` slice.
- `std::mem::page_size()` reports the hosted runtime page size for alignment
  and future mapping work.
- `std::net` provides IP and socket-address values, but not sockets yet.
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
| errno ABI | `target::uses_posix_errno()` is true for Unix-style thread-local errno. | Add `std::os::errno` only when fallible wrappers need a stable error value. |
| ELF | `target::uses_elf()` is true on Linux targets. | Add object/dynamic symbol tests where ABI changes are observable. |
| DWARF | `target::uses_dwarf()` is true on Linux targets. | Backtrace and symbolization work should depend on the runtime-support roadmap. |

## Linux Filesystems And Kernel Views

| API Family | Current Status | Future Module Shape |
| --- | --- | --- |
| procfs | `target::has_procfs()` reports the Linux family; `std::env::executable_path()` currently reads `/proc/self/exe`. | `std::os::linux::proc` should expose typed reads only after file/path/error policy improves. |
| sysfs | `target::has_sysfs()` reports the Linux family. | Keep as roadmap until a safe text/file parser pattern exists. |
| cgroups | `target::has_cgroups_api()` reports the Linux family only. | Optional future wrapper; mount/layout differences should be explicit. |
| namespaces | `target::has_namespaces_api()` reports the Linux family only. | Optional future process/capability API; requires privilege and error modeling. |
| seccomp | `target::has_seccomp_api()` reports the Linux family only. | Optional future security API; keep out of portable process helpers. |
| capabilities | `target::has_capabilities_api()` reports the Linux family only. | Optional future security API; needs dedicated docs and tests. |

## Linux Event And Descriptor APIs

| API Family | Current Status | Future Module Shape |
| --- | --- | --- |
| raw syscall wrapper | Not exposed. | Future `std::os::linux::syscall` should return typed error values and stay behind target guards. |
| errno mapping | `std::target::uses_posix_errno()` reports the ABI family, but no runtime errno accessor exists. | Add `std::os::errno` together with the first fallible descriptor wrappers. |
| file descriptor abstraction | `std::fs::File` stores an internal descriptor value, but no public raw descriptor type exists. | Add an owned `Fd`/`OwnedFd` plus borrowed descriptor view before exposing `fcntl`, `poll`, or epoll. |
| close-on-exec | Not exposed. | Add after descriptor ownership exists; default new descriptors to close-on-exec where possible. |
| nonblocking mode | Not exposed. | Add descriptor methods backed by `fcntl` and document interaction with IO helpers. |
| fcntl | Not exposed. | Future low-level descriptor module with typed flag helpers. |
| ioctl | Not exposed. | Keep optional and narrow; prefer typed wrappers for common devices over a raw catch-all. |
| poll/select | Not exposed. | `poll` can be a portable readiness seed; `select` is legacy and should stay compatibility-oriented. |
| epoll | `target::has_epoll()` reports Linux family support. | `std::os::linux::epoll` with owned epoll fd, `add`, `modify`, `remove`, and `wait`. |
| inotify | `target::has_inotify()` reports Linux family support. | Owned inotify fd plus event buffer parsing. |
| fanotify | `target::has_fanotify_api()` reports Linux family support only. | Optional; permission-heavy and likely not a first runtime slice. |
| eventfd | `target::has_eventfd()` reports Linux family support. | Owned fd wrapper with read/write counter helpers. |
| timerfd | `target::has_timerfd()` reports Linux family support. | Timer handle integrated with `std::time::Duration`. |
| signalfd | `target::has_signalfd()` reports Linux family support. | Needs signal-mask policy before implementation. |
| pidfd | `target::has_pidfd_api()` reports Linux family support only. | Future process handle path; kernel version can still reject it. |
| memfd | `target::has_memfd()` reports Linux family support. | Owned anonymous file descriptor wrapper. |
| io_uring | `target::has_io_uring_api()` reports Linux family support only. | Optional advanced async API after ownership and buffer pinning rules exist. |

## Process, Signals, And Memory Mapping

| API Family | Current Status | Future Module Shape |
| --- | --- | --- |
| argv/env | Portable surface exists through `std::context` and `std::env`. | Keep user-facing wrappers portable; reserve raw `environ` access for `std::os` if needed. |
| current process info | `std::process::id`, `uid`, `gid`, and `is_root` exist. | Add parent id, session/process-group helpers only with clear platform policy. |
| exit/abort | `std::process::exit` and `abort` exist. | Document destructor/cleanup limits anywhere higher-level runtime teardown is added. |
| spawn | Not exposed. | Prefer a portable `std::process::spawn` builder before exposing POSIX-only `posix_spawn`. |
| fork | `std::process::fork` exists as a POSIX slice. | Keep marked as sharp; fork-with-threads and async-signal-safe limitations need more docs. |
| exec | Not exposed. | Add after argument/environment vector ownership and error reporting are stable. |
| wait | `std::process::wait` returns normal child exit status or `-1`. | Replace sentinel-only status with a richer status/result value. |
| kill | Not exposed. | Add with signal constants and permission/error mapping. |
| working directory | `std::env::current_dir` and `set_current_dir` exist. | Owned path values and canonicalization should live with `std::path`/`std::fs`. |
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
3. Introduce a small owned descriptor type and `errno`/error policy.
4. Add process expansion in order: richer wait status, `kill`, `exec`, then
   portable `spawn`.
5. Implement descriptor readiness primitives in order: `poll`, then Linux
   `epoll`, `eventfd`, `timerfd`, and `memfd`.
6. Add memory mapping only after descriptor/error policy and owned mapping
   cleanup are designed.
7. Add signal mask/action support only after signal-safe limitations are fully
   documented.
8. Add optional `pidfd`, `inotify`, and eventually `io_uring` once the runtime
   can test kernel-version-dependent behavior.
9. Keep cgroups, namespaces, seccomp, capabilities, and fanotify optional until
   there is a real application need and privilege-aware tests.
