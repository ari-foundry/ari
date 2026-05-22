# Standard Library Platform Notes

This folder separates platform and ABI planning from ordinary module guides.
Use it when a feature depends on the target triple, object format, linker,
kernel, libc, or runtime startup contract.

## Pages

- [Linux Platform Plan](linux.md): target ABI, libc, ELF/DWARF, syscall and
  errno ABI, procfs/sysfs, and Linux-specific event/file-descriptor families.
- [Verification Matrix](../verification-matrix.md): current platform support
  reading, CI jobs, and the checks needed before calling an API portable.

## Policy

Portable APIs should stay in modules such as `std::fs`, `std::net`,
`std::process`, `std::thread`, `std::sync`, and `std::time`. Platform-specific
APIs should land under a future `std::os` tree only after the handle ownership,
error, and lifetime rules are clear.

`std::target` is the small cross-cutting exception: it reports compiler-known
target facts so library code can choose the right portable or platform-specific
path without parsing target triples by hand.
