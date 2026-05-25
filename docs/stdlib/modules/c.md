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
c::from_ptr_result(data: ptr c_char) -> Result[CStr, std::error::Error]
c::from_slice_in(zone: ref mut Zone, bytes: Slice[u8]) -> CString
c::from_slice_result_in(zone: ref mut Zone, bytes: Slice[u8]) -> Result[CString, std::error::Error]
c::from_cstr_in(zone: ref mut Zone, value: CStr) -> CString
c::from_cstr_result_in(zone: ref mut Zone, value: CStr) -> Result[CString, std::error::Error]
c::is_null(value: ptr c_void) -> bool

c::errno() -> i64
c::error() -> std::error::Error

c::lazy() -> i64
c::now() -> i64
c::local() -> i64
c::global() -> i64
c::open(path: CStr, flags: i64) -> Library
c::open_result(path: CStr, flags: i64) -> Result[Library, std::error::Error]
c::main_program(flags: i64) -> Library
c::main_program_result(flags: i64) -> Result[Library, std::error::Error]
c::symbol(library: ref Library, name: CStr) -> Symbol
c::symbol_result(library: ref Library, name: CStr) -> Result[Symbol, std::error::Error]
c::function[T](symbol: ref Symbol) -> T
c::close(library: ref mut Library) -> bool
c::close_result(library: ref mut Library) -> Result[(), std::error::Error]
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
  let direct: CStr = "ari";
  assert_eq_i64(name.len(), 3);
  assert_eq_i64(direct.len(), 3);
  return strlen(direct.as_ptr()) as i64;
}
```

`CStr.as_slice()` returns the bytes before the trailing NUL. The terminator is
not included because normal Ari `Slice[u8]` APIs expect a length, not a
sentinel. When a `CStr` is the expected type, a string literal can coerce
directly to that borrowed C string view; use `c::from_string(text)` when the
conversion should be visually explicit.
`from_ptr(data)` asserts that the pointer is non-null; use
`from_ptr_result(data)` for C APIs where null is an ordinary failure result.
The result form returns `Error(InvalidInput)` for null.

`CString` is a zone-backed owned C string buffer. It copies a byte slice,
asserts that the input has no interior NUL byte, appends one trailing NUL, and
keeps the logical length without that terminator.
`from_slice_result_in` and `from_cstr_result_in` are the non-asserting
constructors. They return `Error(InvalidInput)` when an owned byte slice would
contain an interior NUL.

```ari
fn copy_name(zone: ref mut Zone, bytes: Slice[u8]) -> CString {
  let owned = c::from_slice_in(zone, bytes);
  assert(owned.as_bytes_with_nul()[owned.len()] == 0u8);
  return owned;
}

fn try_copy_name(zone: ref mut Zone, bytes: Slice[u8]) -> Result[CString, Error] {
  return c::from_slice_result_in(zone, bytes);
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

To call a loaded function, ask the `Symbol` for a function-pointer type:

```ari
fn loaded_strlen() -> i64 {
  var library = c::open(c::from_string("libc.so.6"), c::lazy());
  let symbol = library.symbol(c::from_string("strlen"));
  let strlen = symbol.function<fn(ptr c_char) -> size_t>();
  let text = c::from_string("ari");
  let result = strlen(text.as_ptr()) as i64;
  library.close();
  return result;
}
```

The generic type argument is the C function signature. Ari does not infer this
from a later indirect call yet, so write the signature at the boundary where
the raw dynamic symbol becomes callable. `c::function<T>(ref symbol)` is the
same operation in top-level form.

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
`Symbol.as_ptr()` is still available when a binding needs a raw address instead
of a callable function pointer.
`open_result`, `main_program_result`, `symbol_result`, and
`Library::symbol_result` convert loader null sentinels into `Error(Other)`.
Use `last_error()` after a loader failure when the platform string is useful
for diagnostics. `close_result` and `Library::close_result` return `Ok(())`
for an already-closed handle, so library close is idempotent at the Ari wrapper
level.

## Null Pointers

Use `null` for raw null pointers and `c::is_null(ptr)` when a generic-looking
predicate reads better at the call site:

```ari
let raw: ptr c_void = null;
assert(c::is_null(raw));
```

`c::from_ptr` rejects null pointers with `assert`. Use
`c::from_ptr_result` when null is an ordinary C result and the caller wants a
recoverable `Error(InvalidInput)` branch.

## Roadmap

- Add explicit, documented FFI escape rules for passing zone-backed `CString`
  storage to imported C calls.
- Add richer dynamic-symbol wrappers for non-function data symbols and for
  APIs that need loader-specific error payloads beyond the current shared
  `Error(Other)` category.
- Move OS-specific descriptor, syscall, signal, mmap, and socket wrappers into
  `std::os` or the appropriate portable modules rather than expanding
  `std::c` into a broad libc surface.
- Split `errno` loading for non-glibc Unix targets if their ABI needs a
  different runtime symbol.

## Tests

```text
tests/cases/standard-library/ok/c/std-c-interop.ari
tests/cases/standard-library/ok/c/std-c-dynamic-function.ari
```

The focused tests cover borrowed C string views, owned NUL-terminated
`CString` storage, Result-returning C string boundary constructors, C ABI
aliases through imported libc calls, POSIX `errno` mapping, dynamic loading
through `dlopen`/`dlsym`/`dlclose`, Result-returning loader wrappers, typed
dynamic function-pointer extraction, indirect calls through resolved C
functions, and LLVM symbol checks for the hosted C loader functions.
