# Target Tests

These tests cover `std::target`, the compile-target fact module. They keep the
checks focused on compiler-known target metadata and Linux platform-family
predicates; raw OS wrappers such as epoll or pidfd stay roadmap work until
`std::os` owns their handles and errors.

- `std-target-basic.ari` runs the x86_64 Linux GNU slice.
- `std-target-linux64.ari` is LLVM-only cross-target coverage for x86_64,
  aarch64, and riscv64 Linux target classification.
