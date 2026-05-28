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

c::from(text: Slice[u8]) -> CStr
c::from_ptr(data: ptr c_char) -> Result[CStr, std::error::Error]
c::from_ptr_unchecked(data: ptr c_char) -> CStr
c::from_slice_in(zone: ref mut Zone, bytes: Slice[u8]) -> Result[CString, std::error::Error]
c::from_slice_unchecked_in(zone: ref mut Zone, bytes: Slice[u8]) -> CString
c::from_slice_with_allocator(ref Allocator, bytes: Slice[u8]) -> Result[CString, std::error::Error]
c::from_slice_unchecked_with_allocator(ref Allocator, bytes: Slice[u8]) -> CString
c::from_cstr_in(zone: ref mut Zone, value: CStr) -> Result[CString, std::error::Error]
c::from_cstr_unchecked_in(zone: ref mut Zone, value: CStr) -> CString
c::from_cstr_with_allocator(ref Allocator, value: CStr) -> Result[CString, std::error::Error]
c::from_cstr_unchecked_with_allocator(ref Allocator, value: CStr) -> CString
c::is_null(value: ptr c_void) -> bool

c::errno() -> i64
c::error() -> std::error::Error

c::lazy() -> i64
c::now() -> i64
c::local() -> i64
c::global() -> i64
c::open(path: CStr, flags: i64) -> Result[Library, std::error::Error]
c::open_raw(path: CStr, flags: i64) -> Library
c::main_program(flags: i64) -> Result[Library, std::error::Error]
c::main_program_raw(flags: i64) -> Library
c::symbol(library: ref Library, name: CStr) -> Result[Symbol, std::error::Error]
c::symbol_raw(library: ref Library, name: CStr) -> Symbol
c::function[T](symbol: ref Symbol) -> T
c::close(library: ref mut Library) -> Result[(), std::error::Error]
c::close_bool(library: ref mut Library) -> bool
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

`CString` owns a NUL-terminated copy in an explicit allocation lifetime. Use
`region.cstring(bytes)` in ordinary region-first code, or the allocator
variants when a helper should receive allocation capability without reset or
destroy authority.

```ari
extern "C" fn strlen(text: ptr c_char) -> size_t;

fn main() -> i64 {
  let name = c::from("ari");
  let direct: CStr = "ari";
  assert_eq_i64(name.len(), 3);
  assert_eq_i64(direct.len(), 3);
  return strlen(direct.as_ptr()) as i64;
}
```

`CStr.as_slice()` returns the bytes before the trailing NUL. The terminator is
not included because normal Ari `Slice[u8]` APIs expect a length, not a
sentinel. When a `CStr` is the expected type, a string literal can coerce
directly to that borrowed C string view; use `c::from(text)` when the
conversion should be visually explicit.
`from_ptr(data)` returns `Result[CStr, Error]` and yields
`Error(InvalidInput)` for null. Use `from_ptr_unchecked(data)` only when the C
contract already proves the pointer is non-null; it asserts on null.

`CString` is a zone-backed owned C string buffer. It copies a byte slice,
appends one trailing NUL, and keeps the logical length without that terminator.
`from_slice_in` and `from_cstr_in` are the natural Result-returning
constructors. They return `Error(InvalidInput)` when an owned byte slice would
contain an interior NUL. `from_slice_unchecked_in` and
`from_cstr_unchecked_in` are the assert-on-invalid compatibility forms.

```ari
fn copy_name(zone: ref mut Zone, bytes: Slice[u8]) -> CString {
  let owned = c::from_slice_unchecked_in(zone, bytes);
  assert(owned.as_bytes_with_nul()[owned.len()] == 0u8);
  return owned;
}

fn try_copy_name(zone: ref mut Zone, bytes: Slice[u8]) -> Result[CString, Error] {
  return c::from_slice_in(zone, bytes);
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
  let text = c::from("hello");
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
  let fd = posix_open(c::from("/tmp/missing").as_ptr(), 0);
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
  var library = c::open(c::from("libc.so.6"), c::lazy()).unwrap();

  let symbol = library.symbol(c::from("strlen")).unwrap();

  library.close().unwrap();
  return 0;
}
```

To call a loaded function, ask the `Symbol` for a function-pointer type:

```ari
fn loaded_strlen() -> i64 {
  var library = c::open(c::from("libc.so.6"), c::lazy()).unwrap();
  let symbol = library.symbol(c::from("strlen")).unwrap();
  let strlen = symbol.function<fn(ptr c_char) -> size_t>();
  let text = c::from("ari");
  let result = strlen(text.as_ptr()) as i64;
  library.close().unwrap();
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
for symbols in the main program image. `open_raw`, `main_program_raw`, and
`symbol_raw` keep the old sentinel-returning compatibility behavior.
`Library::invalid()` and
`Symbol::invalid()` are explicit sentinels for code that needs a default value.
`Symbol.as_ptr()` is still available when a binding needs a raw address instead
of a callable function pointer.
`open`, `main_program`, `symbol`, and
`Library::symbol` convert loader null sentinels into `Error(Other)`.
Use `last_error()` after a loader failure when the platform string is useful
for diagnostics. `close` and `Library::close` return `Ok(())`
for an already-closed handle, so library close is idempotent at the Ari wrapper
level.

## Null Pointers

Use `null` for raw null pointers and `c::is_null(ptr)` when a generic-looking
predicate reads better at the call site:

```ari
let raw: ptr c_void = null;
assert(c::is_null(raw));
```

`c::from_ptr` returns `Error(InvalidInput)` for null. Use
`c::from_ptr_unchecked` only when a binding has already ruled null out.

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
