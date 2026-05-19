# std::c

`std::c` is the small standard-library layer for C ABI boundaries. The
compiler already understands C ABI scalar aliases such as `c_int`, `c_char`,
`c_void`, `size_t`, and `c_long`; this module adds source Ari helpers for
borrowed C strings, zone-backed NUL-terminated buffers, POSIX `errno`, and the
first hosted dynamic-loading handle wrappers.

Use this module when Ari code needs to call C APIs, pass a NUL-terminated name
to a C function, read the current POSIX error code, or look up a symbol through
the platform dynamic loader. It is intentionally not a full libc binding.
Portable process, file-descriptor, socket, signal, and memory-mapping APIs
belong in their own modules as they become stable.

## C ABI Types

C ABI type aliases are compiler-known type names, not structs in `std::c`:

```text
c_char, c_schar, c_uchar
c_short, c_ushort
c_int, c_uint
c_long, c_ulong
c_longlong, c_ulonglong
intmax_t, uintmax_t
size_t, ssize_t
c_size_t, c_ssize_t
intptr_t, uintptr_t
c_float, c_double
c_void
```

The aliases follow the selected target. For example, `c_int` maps to `i32`,
`c_void` maps to `void`, and `size_t` follows pointer width. `c_long` is
64-bit on LP64 Unix targets and 32-bit on LLP64 or ILP32 targets. Use
`std::target::long_bits()` and `std::target::pointer_bits()` when a library
needs to document or branch on that layout.

For C `void*`, write `ptr c_void`. A by-value `c_void` parameter is rejected;
only `c_void` returns and `ptr c_void` values make sense at the C boundary.

## API

```ari
std::c::CStr
std::c::CString
std::c::Library
std::c::Symbol

c::from_string(text: string) -> CStr
c::from_ptr(data: ptr c_char) -> CStr
c::from_slice_in(zone: ref mut Zone, bytes: Slice[u8]) -> CString
c::from_cstr_in(zone: ref mut Zone, value: CStr) -> CString
c::is_null(value: ptr c_void) -> bool

c::errno() -> i64
c::error() -> std::error::Error

c::lazy() -> i64
c::now() -> i64
c::local() -> i64
c::global() -> i64
c::open(path: CStr, flags: i64) -> Library
c::main_program(flags: i64) -> Library
c::symbol(library: ref Library, name: CStr) -> Symbol
c::close(library: ref mut Library) -> bool
c::last_error() -> CStr
```

Root aliases are available for the value types:

```ari
CStr
CString
Library
Symbol
```

## CStr And CString

`CStr` is a borrowed view of a NUL-terminated C string. It does not own bytes.
It can wrap an Ari `string` literal or a non-null `ptr c_char` from C.

```ari
extern "C" fn strlen(text: ptr c_char) -> size_t;

fn main() -> i64 {
  let name = c::from_string("ari");
  assert_eq_i64(name.len(), 3);
  return strlen(name.as_ptr()) as i64;
}
```

`CStr.as_slice()` returns the bytes before the trailing NUL. The terminator is
not included because normal Ari `Slice[u8]` APIs expect a length, not a
sentinel.

`CString` is a zone-backed owned C string buffer. It copies a byte slice,
asserts that the input has no interior NUL byte, appends one trailing NUL, and
keeps the logical length without that terminator.

```ari
fn copy_name(zone: ref mut Zone, bytes: Slice[u8]) -> CString {
  let owned = c::from_slice_in(zone, bytes);
  assert(owned.as_bytes_with_nul()[owned.len()] == 0u8);
  return owned;
}
```

Use `CString.as_slice()` for bytes without the terminator and
`CString.as_bytes_with_nul()` when the C boundary needs the exact storage.
`CString.as_c_str()` creates a borrowed `CStr` view over the owned buffer.

Current limitation: Ari's zone provenance checker still rejects passing some
zone-backed pointers directly into arbitrary `extern "C"` calls. That is a
deliberate safety fence until the language has an explicit FFI escape policy
for owned buffers. Borrowed `CStr` values made from string literals work for
immediate C calls today; owned `CString` is already useful for C-shaped
storage, round-tripping, dynamic-loader names inside Ari helpers, and future
fallible FFI handoff rules.

## Extern C Functions

Declare imported C functions with `extern "C"` and C ABI aliases:

```ari
extern "C" fn abs(value: c_int) -> c_int;
extern "C" fn strlen(text: ptr c_char) -> size_t;

fn main() -> i64 {
  let text = c::from_string("hello");
  let count = strlen(text.as_ptr());
  return (abs(-7i32) as i64) + (count as i64);
}
```

When the Ari name should differ from the C symbol, add an explicit link name:

```ari
extern "C" fn posix_open(path: ptr c_char, flags: c_int) -> c_int = "open";
```

## Errno

`c::errno()` reads the current POSIX thread-local `errno` value on the hosted
Linux/glibc path. `c::error()` maps that value through `std::error::from_errno`
so library code can use stable Ari error categories:

```ari
extern "C" fn posix_open(path: ptr c_char, flags: c_int) -> c_int = "open";

fn missing_file_error() -> std::error::Error {
  let fd = posix_open(c::from_string("/tmp/missing").as_ptr(), 0);
  if fd < 0 {
    return c::error();
  }
  return std::error::new(std::error::Other);
}
```

`errno()` is low-level. Prefer module-specific error helpers when a higher
level module owns the operation. Use it directly when binding a C API that
reports failure through sentinel values and `errno`.

## Dynamic Loading

`Library` and `Symbol` are thin wrappers over the platform dynamic loader:

```ari
fn load_strlen() -> i64 {
  var library = c::open(c::from_string("libc.so.6"), c::lazy());
  if !library.is_open() {
    return 1;
  }

  let symbol = library.symbol(c::from_string("strlen"));
  if !symbol.is_valid() {
    let reason = c::last_error();
    assert(!reason.is_empty());
    library.close();
    return 2;
  }

  library.close();
  return 0;
}
```

Flag helpers use natural names because the module path already says this is
the C dynamic loader:

| Ari Helper | C Meaning |
| --- | --- |
| `c::lazy()` | `RTLD_LAZY` |
| `c::now()` | `RTLD_NOW` |
| `c::local()` | `RTLD_LOCAL` |
| `c::global()` | `RTLD_GLOBAL` |

`c::main_program(flags)` passes a null path to the loader and returns a handle
for symbols in the main program image. `Library::invalid()` and
`Symbol::invalid()` are explicit sentinels for code that needs a default value.

The current `Symbol` API exposes `as_ptr()` only. Typed symbol casting and a
safe callable wrapper need a deliberate function-pointer cast policy, so they
remain roadmap work.

## Null Pointers

Use `null` for raw null pointers and `c::is_null(ptr)` when a generic-looking
predicate reads better at the call site:

```ari
let raw: ptr c_void = null;
assert(c::is_null(raw));
```

`CStr::from_ptr` rejects null pointers with `assert`. Use a nullable raw
pointer and branch before constructing `CStr` when null is an ordinary C
result.

## Roadmap

- Add a fallible `Result`-based C string constructor once `Result[T, Error]`
  can carry richer payloads naturally.
- Add explicit, documented FFI escape rules for passing zone-backed `CString`
  storage to imported C calls.
- Add typed dynamic-symbol wrappers after Ari has a stable function-pointer
  cast policy.
- Move OS-specific descriptor, syscall, signal, mmap, and socket wrappers into
  `std::os` or the appropriate portable modules rather than expanding
  `std::c` into a broad libc surface.
- Split `errno` loading for non-glibc Unix targets if their ABI needs a
  different runtime symbol.

## Tests

```text
tests/cases/standard-library/ok/c/std-c-interop.ari
```

The focused test covers borrowed C string views, owned NUL-terminated
`CString` storage, C ABI aliases through imported libc calls, POSIX `errno`
mapping, dynamic loading through `dlopen`/`dlsym`/`dlclose`, and LLVM symbol
checks for the hosted C loader functions.
