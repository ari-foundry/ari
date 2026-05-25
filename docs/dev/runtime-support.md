# Runtime Support Roadmap

This page tracks the low-level runtime and ABI pieces that sit below the
source standard library. They are not ordinary `std` modules in the same way
`std::fs` or `std::net` are; some require compiler output, linker objects, or
platform-specific startup code. Keeping them documented here helps library
authors know which capabilities are stable and which ones need backend work.

## Current Host Model

Ari currently emits LLVM IR and asks an LLVM driver such as `clang` to link the
program. On Linux, that means glibc and the host CRT provide `_start`, startup
objects, dynamic linker setup, and most compiler runtime details. Ari emits a
host `main`, an `ari_entry(argc, argv)`, and an `ari::main` bridge. The entry
path initializes Ari context, runs source `main`, shuts context down, and
returns the `i64` result as the process exit status.

Implemented runtime support today:

- host `main` wrapper and `ari_entry`
- context initialization for `argc`, `argv`, and thread-local Ari thread id
- compiler-emitted `std::target` hooks for target triple, architecture, OS,
  libc/environment, object/debug format, errno ABI, pointer width, C `long`
  width, and Linux syscall/API-family classification
- process IO, input, environment, path, process, thread, sync, time, fs,
  net-address, IPv4 DNS lookup and host-port endpoint resolution, IPv4 TCP/UDP
  sockets, Unix stream sockets, socket options, socket timeout, and socket
  shutdown hooks used by current `std`
- panic/assert stop behavior through `exit(1)`
- LLVM atomic instructions for the first `AtomicI64` slice
- byte memory routines: `std::mem::copy_bytes`, `move_bytes`, and `set_bytes`
  lower to `llvm.memcpy`, `llvm.memmove`, and `llvm.memset`
- hosted page-size query: `std::mem::page_size` lowers to the platform runtime
  page-size hook

## Freestanding And Kernel-Scale Direction

Freestanding support is a long-term runtime track. It should begin with a
minimal target-specific assembly entry and a linker script, then port the
startup path into Ari only after the compiler can model the required ABI pieces
without relying on libc or the host CRT. This keeps the early boot boundary
honest while still giving Ari a path toward kernel-scale systems.

The first freestanding profile should prove:

- no-libc object and executable/image emission through explicit driver flags
- assembly startup that passes a documented machine state into Ari code
- Ari runtime initialization that does not assume `argc`, `argv`, envp, TLS, or
  pthreads exist
- panic and assertion behavior that can call a platform hook or halt
- compiler runtime helpers for memory builtins, integer helpers, atomics, and
  stack-protector hooks when the target requires them
- volatile/MMIO pointer operations and memory fences with clear ownership and
  aliasing rules
- allocator hooks that let a kernel provide bootstrap, page, slab, or zone
  allocation without an ambient global heap
- a `core`-like library subset that excludes hosted process, filesystem,
  network, and thread APIs
- focused tests for LLVM IR, object output, linker behavior, and emulator smoke
  boot once the toolchain path exists

## Required Runtime Families

| Area | Purpose | Current Status | Roadmap |
| --- | --- | --- | --- |
| target ABI | Classify architecture, OS, libc/environment, pointer width, C ABI widths, object format, debug format, syscall ABI, and errno ABI. | `std::target` exposes the current compiler-known slice for x86_64/aarch64/riscv64 Linux and hosted Unix/Windows families recognized by `TargetInfo`. | Add cross-target tests and extend only as driver/linker support becomes real. |
| `_start` entry | Enter a process before libc `main`, receive raw stack/process state, and call Ari initialization. | Host executables use the platform CRT-provided `_start` and Ari emits `main`. | Add only for freestanding or no-libc targets. It needs stack layout, auxv/envp handling, and target-specific assembly/object support. |
| `crt0` / startup object | Own startup object files for targets that cannot rely on system CRT. | Not implemented; delegated to the LLVM driver and host CRT. | Create per-target startup objects when Ari supports freestanding/static runtime profiles. |
| init/fini array | Run module/global constructors and destructors in executable/shared-library load/unload order. | Not needed for current source surface because Ari has no global constructors. | Add when global initialization, plugin-style libraries, or runtime-owned constructors appear. |
| TLS initialization | Provide thread-local runtime state. | The LLVM IR uses `thread_local` for the Ari thread-id slot and the pthread trampoline stores spawned thread ids. `std::thread::ThreadId` wraps those ids, `JoinHandle` owns the join/detach right for spawned threads, and `std::thread::ThreadLocal[T]` exposes explicit zone-backed per-thread handle storage keyed by Ari's runtime thread id. | Freestanding `_start` or custom thread runtimes need explicit TLS setup and target documentation. Compiler-owned `thread_local` declarations still need generic static storage, destructor policy, and ownership rules. |
| stack protector hook | Provide `__stack_chk_guard` and `__stack_chk_fail` when stack protectors are enabled. | Not emitted by Ari directly. The host toolchain may provide it if enabled externally. | Decide whether Ari enables stack protectors, then provide hooks or require the platform runtime. |
| panic runtime | Define what `panic`, `todo`, `unreachable`, assertions, and failed runtime checks do. | Current panic path exits with status `1`. | Add message payloads, stderr reporting, optional backtrace, and non-terminating policy only after owned strings and runtime diagnostics are stable. |
| unwind runtime | Support stack unwinding for panics, destructors, and foreign exceptions. | Not implemented; Ari panics do not unwind. | Keep abort/exit semantics until destructor and exception interop policy is designed. |
| personality function | Identify language-specific exception handling behavior to the unwinder. | Not emitted. | Required only if Ari gains unwinding or cleanup landing pads. |
| `.eh_frame` | Supply unwind metadata for stack walking and exception runtimes. | Left to LLVM/toolchain defaults for generated functions. | Document exact requirements before adding Ari-owned unwinding or freestanding targets. |
| backtrace support | Produce stack traces for panic diagnostics. | Not implemented. | Needs symbolization policy, frame metadata, and stderr/string formatting support. |
| dynamic linker compatibility | Keep shared-library output usable with the platform loader. | `--shared` emits LLVM IR/object through the LLVM driver with visibility rules for public/exported Ari functions. | Expand with init/fini arrays, exported runtime ABI versioning, and loader tests. |
| linker hardening profiles | Own static vs dynamic linking, PIE, RELRO, and stack-protector defaults deliberately. | Ari delegates these to the LLVM driver and host defaults today. | Add explicit driver flags and tests before exposing them through `std::target`. |
| `libgcc_s` / compiler-rt replacement | Provide helper routines for arithmetic, unwinding, atomics, and builtins when the host runtime is absent. | Delegated to the LLVM driver, glibc, libgcc_s, or compiler-rt. | Needed for no-libc/freestanding profiles or custom target triples. |
| atomic and wait helper routines | Provide fallback calls for atomics that cannot lower inline on a target and blocking wait/wake helpers for future locks. | Current `AtomicI64` lowers to LLVM atomic instructions on the host path; `AtomicBool`, `AtomicUsize`, and `AtomicPtr[T]` are source wrappers; source no-poison `Mutex`, `RwLock`, `Once`, `OnceLock`, generation `Condvar`, `Barrier`, and capacity-1 channels spin/yield over that primitive. | Add target-specific fallback policy if LLVM emits helper calls or if wider/generic atomics land. Use Linux futex only as an internal implementation detail once blocking `Mutex`, `Condvar`, `RwLock`, barriers, semaphores, or channels have a public ownership policy. |
| memory builtins | Provide efficient `memcpy`, `memmove`, `memset` behavior. | `std::mem` byte helpers now lower through LLVM memory intrinsics. | Later optimize source library copies to these helpers and document ownership-safe use. |
| Linux descriptor APIs | Provide epoll, inotify, fanotify, eventfd, timerfd, signalfd, pidfd, memfd, and optional io_uring wrappers. | `std::os::OwnedFd` owns ordinary descriptors; `std::net` uses owned descriptors for hosted IPv4 TCP/UDP and Unix stream sockets. `std::target` reports Linux API-family availability for readiness APIs that are not exposed yet. | Add readiness APIs, richer errno/result policy, and target guards before implementing epoll/eventfd/timerfd/signalfd/memfd wrappers. |
| Linux kernel views | Provide safe access to procfs, sysfs, cgroups, namespaces, seccomp, and capabilities where appropriate. | `std::env::executable_path()` reads `/proc/self/exe`; the rest are platform-roadmap items. | Keep optional and privilege-aware; do not make them portable `std` APIs. |

## Implementation Rules

- Prefer the host CRT/toolchain for hosted Linux output until Ari has a
  documented freestanding profile.
- Keep runtime hooks explicit as `extern "ari"` declarations in `lib/std`.
- Keep public user APIs in `docs/stdlib` and low-level backend obligations in
  this file.
- Add a focused test whenever a hook becomes observable from Ari source.
- Do not add a broad raw syscall or runtime module until ownership, handle
  lifetime, and error policy are clear.
