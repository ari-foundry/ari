# std::target

`std::target` reports facts the compiler knows about the current build target.
It exists so library authors can write readable platform checks without
scattering target triples or magic integer constants through source code.

This module is not a live kernel probe. It describes the selected compile
target, object/debug formats, ABI family, and Linux API families that should be
available on that target. Runtime failure, permissions, kernel version checks,
and handle ownership belong in future `std::os` wrappers.

## API

```ari
target::triple(ref mut zone) -> String
target::arch() -> target::Arch
target::arch_name(ref mut zone) -> String
target::os() -> target::Os
target::os_name(ref mut zone) -> String
target::env() -> target::Env
target::env_name(ref mut zone) -> String
target::object_format() -> target::ObjectFormat
target::debug_format() -> target::DebugFormat
target::errno_abi() -> target::ErrnoAbi
target::pointer_bits() -> i64
target::long_bits() -> i64
target::syscall_abi() -> target::SyscallAbi
```

Enums use explicit fallback cases because Ari enum constructors are currently
module-scoped:

```ari
Arch: UnknownArch, X86_64, Aarch64, Riscv64, X86, Arm
Os: UnknownOs, Linux, Macos, Windows
Env: UnknownEnv, Gnu, Musl, Msvc, Mingw, Apple
ObjectFormat: UnknownObjectFormat, Elf, MachO, Coff
DebugFormat: UnknownDebugFormat, Dwarf, CodeView
ErrnoAbi: UnknownErrnoAbi, PosixThreadLocal, WindowsLastError
SyscallAbi: UnknownSyscallAbi, LinuxX86_64, LinuxAarch64, LinuxRiscv64
```

Readable predicates are preferred at call sites:

```ari
target::is_x86_64()
target::is_aarch64()
target::is_riscv64()
target::is_linux()
target::is_macos()
target::is_windows()
target::is_unix()
target::is_gnu()
target::is_musl()
target::uses_glibc()
target::uses_musl()
target::uses_elf()
target::uses_dwarf()
target::uses_posix_errno()
target::has_tls()
```

Linux target-family predicates:

```ari
target::has_vdso()
target::has_procfs()
target::has_sysfs()
target::has_epoll()
target::has_inotify()
target::has_eventfd()
target::has_timerfd()
target::has_signalfd()
target::has_memfd()
target::has_pidfd_api()
target::has_fanotify_api()
target::has_io_uring_api()
target::has_cgroups_api()
target::has_namespaces_api()
target::has_seccomp_api()
target::has_capabilities_api()
```

The `_api` suffix marks families that are especially sensitive to kernel
version, configuration, privileges, or filesystem mounts. A future `std::os`
wrapper should still return `Option` or `Result` when opening or using the
resource.

## Example

```ari
fn can_use_fast_poll() -> bool {
  return target::is_linux() && target::has_epoll();
}

fn needs_posix_errno_path() -> bool {
  return target::is_unix() && target::uses_posix_errno();
}
```

Natural text helpers copy target names into the caller's region/zone as owned
`String` values. The compiler-owned static hooks stay private to the module so
raw `ptr c_char` values do not leak into ordinary user APIs.

For architecture-specific code, prefer matching the enum over parsing the
triple string:

```ari
fn syscall_family() -> target::SyscallAbi {
  return target::syscall_abi();
}
```

For declaration-level pruning, `@cfg(target("name"))` uses the same target
classifier. It recognizes architecture names such as `x86_64`, `aarch64`, and
`riscv64`, OS names such as `linux`, and Linux environment/format names such
as `gnu`, `glibc`, `musl`, `elf`, and `dwarf`.

## Build Modes

`std::target` deliberately does not report static linking, dynamic linking,
PIE, RELRO, or stack-protector settings yet. Those are build-profile and linker
policy choices, not just target-triple facts. They are tracked in the platform
roadmap until the Ari driver owns explicit flags for them.

## Tests

```text
tests/cases/standard-library/ok/target/std-target-basic.ari
tests/cases/standard-library/ok/target/std-target-linux64.ari
```

The focused executable test compiles with `--target x86_64-pc-linux-gnu`,
checks the target triple in LLVM IR, exercises the target enum hooks and source
predicates, and runs the resulting executable on the local Linux path. The
Linux64 fixture is LLVM-only coverage for x86_64, aarch64, and riscv64 target
classification without requiring cross-linking.

## Future Work

- expose explicit driver build-profile facts when Ari owns static/dynamic/PIE
  flags
- add target predicates for more environments only after `TargetInfo` tracks
  them deliberately
- move raw OS resources into `std::os::linux` with owned handles and fallible
  results
- extend cross-target checks beyond LLVM emission when CI has link/run support
  for those architectures
