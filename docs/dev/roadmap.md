# Roadmap

This page tracks unfinished work only. Completed compiler milestones are kept in
[Completed Milestones](completed-milestones.md), and the supported language
surface is documented in the language guide and [Feature Test Matrix](test-matrix.md).

Keep roadmap items small enough to land in a 0.x series. Do not describe an
item as 1.0 work unless the whole language release is being scoped.

## Near-Term Compiler Work

These are the next compiler-sized slices that should be possible without
changing the long-term language contract.

Phase-oriented sema decomposition is now tracked as ongoing maintenance in
[Semantic Checker Decomposition](sema-decomposition.md) instead of as a finite
near-term deliverable.

Source standard library planning is tracked in
[Standard Library Roadmap](standard-library-roadmap.md). Add compiler work here
only when a library slice needs parser, semantic checker, IR, runtime, or
backend changes that cannot be expressed in Ari source.

No active near-term compiler work is queued right now. Add the next concrete
0.x-sized compiler slice here when it is ready to implement.

## Backend Work

No active backend work is queued right now. Add the next concrete 0.x-sized
backend slice here when it is ready to implement.

## Long-Term Kernel-Grade Freestanding Track

This is a deliberately long-term direction, not current implementation work.
The goal is to make Ari capable of building kernel-scale freestanding systems
after the hosted compiler, standard library, and runtime contracts are mature.
Ari does not need to start a kernel from pure Ari source on day one: the
expected bootstrap path is a small target-specific assembly entry, followed by
progressively porting startup, runtime, and higher-level kernel subsystems to
Ari as the compiler proves each layer.

Required compiler and runtime capabilities:

- freestanding target profiles that do not require glibc, musl, or a host CRT
- custom linker-script support and driver flags for kernel images
- target-specific assembly or object startup for the first boot entry
- Ari-owned `_start` or kernel entry lowering after assembly bootstrap
- panic runtime that can abort, halt, or report through a platform hook
- compiler-rt/libgcc-style helper replacement for no-libc targets
- required builtins for `memcpy`, `memmove`, `memset`, integer arithmetic,
  atomics, and stack-protector hooks where enabled
- volatile loads/stores, MMIO-safe pointer APIs, fences, and explicit memory
  ordering
- inline assembly or a small set of architecture intrinsics for privileged
  instructions, CPU registers, interrupt control, and barriers
- target ABI documentation for x86_64, aarch64, and riscv64 kernel profiles
- interrupt/exception ABI support, including stack frame layout and calling
  convention rules
- TLS or per-CPU/per-task runtime state policy for kernel execution
- frame/unwind/backtrace policy that works without a userspace dynamic linker
- allocator interfaces suitable for page, slab, zone, and bootstrap allocators
- `core`/`alloc`/`std` layering so kernel code can avoid hosted `std`
- raw-pointer, ownership, and drop rules that are explicit enough for device
  buffers, DMA, page tables, and intrusive kernel data structures
- synchronization primitives suitable for kernels, including spin locks,
  interrupt-aware locking, atomics, and eventually wait queues
- build/test fixtures for freestanding LLVM IR, objects, linker scripts, and
  runnable emulator smoke tests

Non-goals for the early part of this track:

- replacing Linux or matching Linux performance
- hiding the first boot entry behind unsupported Ari syntax
- exposing hosted `std::fs`, `std::thread`, or libc-backed process APIs to
  freestanding kernel code
- starting broad driver or scheduler work before the freestanding ABI,
  allocator, volatile/MMIO, and synchronization contracts are stable

## Bootstrap Direction

1. Keep the C++ implementation compact while the language design stabilizes.
2. Reimplement isolated front-end pieces in Ari once structs, strings, and
   vectors lower.
3. Reimplement parser and semantic passes in Ari.
4. Compile the Ari compiler with Ari.
5. Compare outputs from the current compiler and self-hosted compiler.

## Non-Goals For The Current Milestone

- class syntax
- hidden inheritance
- garbage collection
- C++ ABI dependency as a source-level FFI surface
- ambient global heap as a language primitive
- non-local ownership for bare root `Vec[T]` without an explicit allocation
  capability
- adding a second backend during the current 0.x compiler/library stabilization
  work
