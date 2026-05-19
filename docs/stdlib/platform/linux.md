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
| epoll | `target::has_epoll()` reports Linux family support. | `std::os::linux::epoll` with owned epoll fd, `add`, `modify`, `remove`, and `wait`. |
| inotify | `target::has_inotify()` reports Linux family support. | Owned inotify fd plus event buffer parsing. |
| fanotify | `target::has_fanotify_api()` reports Linux family support only. | Optional; permission-heavy and likely not a first runtime slice. |
| eventfd | `target::has_eventfd()` reports Linux family support. | Owned fd wrapper with read/write counter helpers. |
| timerfd | `target::has_timerfd()` reports Linux family support. | Timer handle integrated with `std::time::Duration`. |
| signalfd | `target::has_signalfd()` reports Linux family support. | Needs signal-mask policy before implementation. |
| pidfd | `target::has_pidfd_api()` reports Linux family support only. | Future process handle path; kernel version can still reject it. |
| memfd | `target::has_memfd()` reports Linux family support. | Owned anonymous file descriptor wrapper. |
| io_uring | `target::has_io_uring_api()` reports Linux family support only. | Optional advanced async API after ownership and buffer pinning rules exist. |

## Implementation Roadmap

1. Keep `std::target` current with compiler target support.
2. Add explicit `std::os` docs before adding raw wrappers.
3. Introduce a small owned descriptor type and `errno`/error policy.
4. Implement `epoll`, `eventfd`, `timerfd`, and `memfd` before more complex
   security or namespace APIs.
5. Add optional `pidfd`, `inotify`, and eventually `io_uring` once the runtime
   can test kernel-version-dependent behavior.
6. Keep cgroups, namespaces, seccomp, capabilities, and fanotify optional until
   there is a real application need and privilege-aware tests.
