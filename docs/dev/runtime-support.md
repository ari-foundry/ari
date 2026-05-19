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
- process IO, input, environment, path, process, thread, sync, time, fs, and
  net-address support hooks used by current `std`
- panic/assert stop behavior through `exit(1)`
- LLVM atomic instructions for the first `AtomicI64` slice
- byte memory routines: `std::mem::copy_bytes`, `move_bytes`, and `set_bytes`
  lower to `llvm.memcpy`, `llvm.memmove`, and `llvm.memset`
- hosted page-size query: `std::mem::page_size` lowers to the platform runtime
  page-size hook

## Required Runtime Families

| Area | Purpose | Current Status | Roadmap |
| --- | --- | --- | --- |
| target ABI | Classify architecture, OS, libc/environment, pointer width, C ABI widths, object format, debug format, syscall ABI, and errno ABI. | `std::target` exposes the current compiler-known slice for x86_64/aarch64/riscv64 Linux and hosted Unix/Windows families recognized by `TargetInfo`. | Add cross-target tests and extend only as driver/linker support becomes real. |
| `_start` entry | Enter a process before libc `main`, receive raw stack/process state, and call Ari initialization. | Host executables use the platform CRT-provided `_start` and Ari emits `main`. | Add only for freestanding or no-libc targets. It needs stack layout, auxv/envp handling, and target-specific assembly/object support. |
| `crt0` / startup object | Own startup object files for targets that cannot rely on system CRT. | Not implemented; delegated to the LLVM driver and host CRT. | Create per-target startup objects when Ari supports freestanding/static runtime profiles. |
| init/fini array | Run module/global constructors and destructors in executable/shared-library load/unload order. | Not needed for current source surface because Ari has no global constructors. | Add when global initialization, plugin-style libraries, or runtime-owned constructors appear. |
| TLS initialization | Provide thread-local runtime state. | The LLVM IR uses `thread_local` for the Ari thread-id slot and the pthread trampoline stores spawned thread ids. | Freestanding `_start` or custom thread runtimes need explicit TLS setup and target documentation. |
| stack protector hook | Provide `__stack_chk_guard` and `__stack_chk_fail` when stack protectors are enabled. | Not emitted by Ari directly. The host toolchain may provide it if enabled externally. | Decide whether Ari enables stack protectors, then provide hooks or require the platform runtime. |
| panic runtime | Define what `panic`, `todo`, `unreachable`, assertions, and failed runtime checks do. | Current panic path exits with status `1`. | Add message payloads, stderr reporting, optional backtrace, and non-terminating policy only after owned strings and runtime diagnostics are stable. |
| unwind runtime | Support stack unwinding for panics, destructors, and foreign exceptions. | Not implemented; Ari panics do not unwind. | Keep abort/exit semantics until destructor and exception interop policy is designed. |
| personality function | Identify language-specific exception handling behavior to the unwinder. | Not emitted. | Required only if Ari gains unwinding or cleanup landing pads. |
| `.eh_frame` | Supply unwind metadata for stack walking and exception runtimes. | Left to LLVM/toolchain defaults for generated functions. | Document exact requirements before adding Ari-owned unwinding or freestanding targets. |
| backtrace support | Produce stack traces for panic diagnostics. | Not implemented. | Needs symbolization policy, frame metadata, and stderr/string formatting support. |
| dynamic linker compatibility | Keep shared-library output usable with the platform loader. | `--shared` emits LLVM IR/object through the LLVM driver with visibility rules for public/exported Ari functions. | Expand with init/fini arrays, exported runtime ABI versioning, and loader tests. |
| linker hardening profiles | Own static vs dynamic linking, PIE, RELRO, and stack-protector defaults deliberately. | Ari delegates these to the LLVM driver and host defaults today. | Add explicit driver flags and tests before exposing them through `std::target`. |
| `libgcc_s` / compiler-rt replacement | Provide helper routines for arithmetic, unwinding, atomics, and builtins when the host runtime is absent. | Delegated to the LLVM driver, glibc, libgcc_s, or compiler-rt. | Needed for no-libc/freestanding profiles or custom target triples. |
| atomic helper routines | Provide fallback calls for atomics that cannot lower inline on a target. | Current `AtomicI64` lowers to LLVM atomic instructions on the host path. | Add target-specific fallback policy if LLVM emits helper calls or if wider/generic atomics land. |
| memory builtins | Provide efficient `memcpy`, `memmove`, `memset` behavior. | `std::mem` byte helpers now lower through LLVM memory intrinsics. | Later optimize source library copies to these helpers and document ownership-safe use. |
| Linux descriptor APIs | Provide epoll, inotify, fanotify, eventfd, timerfd, signalfd, pidfd, memfd, and optional io_uring wrappers. | `std::target` only reports Linux API-family availability. No owned descriptors are exposed yet. | Add a small descriptor owner and errno/result policy before implementing wrappers. |
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
