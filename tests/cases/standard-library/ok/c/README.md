# Standard Library C Interop Tests

These cases cover `std::c`.

Current files:

- `std-c-interop.ari`: C ABI string views, owned NUL-terminated `CString`,
  libc type aliases through extern calls, POSIX errno capture, and raw
  `dlopen`/`dlsym`/`dlclose` dynamic loading handles.
- `std-c-dynamic-function.ari`: typed function-pointer extraction from a
  dynamic loader `Symbol` and indirect calls through the resolved C function.
