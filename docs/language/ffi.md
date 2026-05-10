# C FFI And Libraries

## Extern C

Declare external C functions with `extern "C"`:

```ari
extern "C" fn puts(text: string) -> i32;

fn main() -> i64 {
  puts("hello from libc")
  return 0
}
```

When no explicit link name is provided, the C symbol is the final function name.
For example, `puts` links to `puts`.

Use an explicit link name when the Ari name should differ from the external
symbol:

```ari
extern "C" fn c_puts(text: string) -> i32 = "puts";
```

`extern fn` without an ABI string is also treated as C:

```ari
extern fn puts(text: string) -> i32;
```

Other foreign ABI strings, including `extern "C++"`, are rejected. C++ interop
should go through an explicit `extern "C"` wrapper function.

## Ari Builtin ABI

`extern "ari"` is not C FFI. It is reserved for compiler/runtime builtins that
Ari itself defines, such as the standard `std` header declarations for IO,
assertions, context, and zones:

```ari
extern "ari" fn write_i64(value: i64) -> i64 = "ari_builtin_write_i64";
```

These declarations must name a known `ari_builtin_*` symbol explicitly. They
are for Ari's own standard surface and should not be used to bind libc or other
foreign libraries. After semantic checking they lower as Ari builtin ABI
entries, not C extern declarations, so LLVM output calls the runtime hook
without emitting a foreign `declare` for it.

Extern declarations describe concrete C symbols. They cannot be generic because
C has no source-level generic ABI for Ari to instantiate:

```ari
extern "C" fn sort_i64(values: ptr i64, len: size_t) -> c_void;
```

When a foreign library exposes C++ templates, C macros, or another generic API,
provide concrete C wrapper symbols and bind those wrappers from Ari. Ari
generic functions can call concrete extern declarations, but the extern
declaration itself must not have a `[T]` parameter list.

## C Varargs

C variadic functions use a final `...` marker after at least one fixed
parameter:

```ari
extern "C" fn printf(format: string, ...) -> c_int;

fn main() -> i64 {
  printf("answer = %lld\n", 42i64);
  return 0;
}
```

The fixed parameters are checked normally. Extra variadic arguments may be
integer, float, bool, string, compact enum, pointer, or borrow-shaped values.
Ari applies C's default argument promotions at variadic call sites: `bool`,
`i8`, `u8`, `i16`, and `u16` become `i32`, and `f32` becomes `f64`. Wider
integer and float values keep their declared width, so pass `42i64` for `%lld`.

Variadic extern functions can be called directly but cannot be used as function
pointer values. Ari's current `fn(...) -> ...` function pointer type describes
fixed-arity calls only.

## C Callbacks

C-compatible function pointer parameters use the same `fn(...) -> ...` type as
ordinary Ari function pointer values:

```ari
extern "C" fn apply(op: fn(i64) -> i64, value: i64) -> i64;

fn add_ten(value: i64) -> i64 {
  value + 10
}

fn main() -> i64 {
  apply(add_ten, 5)
}
```

The LLVM host backend lowers function pointer parameters as C pointer-shaped
values. Passing an Ari function name to an `extern "C"` parameter gives C a
callable pointer to that function.

## Raw Pointer Helpers

Raw pointers are explicit FFI values. `ptr_offset(pointer, bytes)` performs
byte-wise address arithmetic, while `ptr_load(pointer)` and
`ptr_store(pointer, value)` perform unchecked scalar or plain Ari-layout
aggregate memory access through a `ptr T`. Scalar and plain aggregate `ptr T`
values can also be loaded and stored with `*pointer` dereference syntax. Raw
pointers to Ari aggregate layouts can address scalar fields and elements with
`(*pointer).field`, `(*pointer).0`, and `(*pointer)[index]`.
`size_of<T>()` and `align_of<T>()` expose the current scalar and Ari-layout
aggregate size/alignment model for explicit pointer code. The raw
`--freestanding` backend uses those same Ari byte offsets for tuple, struct,
tuple-struct, and fixed-array pointer field/element access; aggregate enum
pointer loads and stores can copy the whole aggregate enum value, and direct
enum-constructor stores such as `ptr_store(raw, Some(5))` or `*raw = Some(5)`
write the current Ari tag/payload layout. Direct payload field pointer access
remains planned.
The raw `--freestanding` backend does not link or call external C symbols yet;
calling an `extern "C"` function there is rejected with a backend diagnostic.
Use the LLVM host backend for C interop, or expose the operation through a
future Ari runtime shim for raw targets.
Host LLVM builds can allocate raw memory from explicit zones with
`zone::create`, `zone::alloc`, `zone::reset`, and `zone::destroy`; the result of
raw byte allocation is a `ptr u8` that can be cast and used with the same raw
pointer helpers. `zone::alloc<T>(ref mut zone)` computes `T`'s layout and
returns `ptr T` directly. `zone::new<T>(ref mut zone, value)` stores an initial
value into newly allocated zone memory and returns the typed pointer.
`zone::scratch<T>(capacity, value)` is local-binding sugar for creating a
hidden temporary zone, placing `value` in it, and using the resulting `ptr T`
inside the current lexical scope. `zone::promote<T>(ref mut target, source)`
copies a pointed-to value into an explicit target zone and returns a pointer
tied to that target zone.

```ari
extern "C" fn fill(value: ptr i64) -> c_void;

fn main() -> i64 {
  var value: i64 = 0;
  let raw: ptr i64 = (ref mut value) as ptr i64;
  fill(raw);
  *raw = *raw + 1;
  return ptr_load(raw);
}
```

## Modules

Extern declarations can live in modules. If no explicit link name is provided,
the final path segment is used as the C symbol:

```ari
mod libc {
  pub extern "C" fn puts(text: string) -> i32;
}

fn main() -> i64 {
  libc::puts("hello")
  return 0
}
```

## Linking Libraries

Pass library search paths and libraries through the Ari compiler:

```sh
./build/ari app.ari -o app -L ./lib -l mylib
./build/ari app.ari -o app --link m
```

The default LLVM driver is `clang`. Override it with:

```sh
./build/ari app.ari --llvm-cc clang -o app
```

## Building Shared Libraries

Use `--shared` to build a shared library. In this mode Ari does not require
`fn main() -> i64`.

```sh
./build/ari library.ari --shared -o libari_sample.so
```

Ari functions are emitted as C-ABI-compatible LLVM symbols with Ari mangled
names. The symbol encodes the source function path, but not parameter names,
parameter types, or return type:

```text
_ARNv3add
_ARNv4Math3add
```

Use `@export`, `@export("symbol")`, or `@no_mangle` to choose a C symbol
explicitly:

```ari
@export("ari_public_add")
pub fn add(left: i64, right: i64) -> i64 {
  return left + right;
}

@no_mangle
pub fn increment(value: i64) -> i64 {
  return value + 1;
}
```

`@export` and `@no_mangle` without arguments use the final source path segment.
Explicit symbols must be C identifiers and cannot collide with any other
emitted Ari function symbol. See
[Symbol Mangling](../dev/symbol-mangling.md) for the default encoding.

In `--shared` builds, `pub` Ari functions and explicit export/no-mangle
functions remain default-visible. Private helper functions are emitted with
hidden LLVM visibility, and Ari-owned runtime helpers are hidden as internal
implementation details.
Raw `--freestanding` ELF output records explicit export/no-mangle names in the
static symbol table, but imported `extern "C"` calls still require the LLVM host
backend until the raw backend has a native C ABI and link path.

## Type Mapping

Current LLVM host C FFI type mapping targets the default x86-64 Linux/glibc
backend:

```text
i8/i16/i32/i64    -> i8/i16/i32/i64
u8/u16/u32/u64    -> i8/i16/i32/i64
isize/usize       -> i64/u64
c_bool            -> i1
c_char/c_schar    -> i8
c_uchar           -> u8
c_short/c_ushort  -> i16/u16
c_int/c_uint      -> i32/u32
c_long/c_ulong    -> i64/u64
c_longlong/
  c_ulonglong     -> i64/u64
intmax_t/
  uintmax_t       -> i64/u64
c_intmax_t/
  c_uintmax_t     -> i64/u64
size_t/c_size_t   -> u64
ssize_t/c_ssize_t -> i64
intptr_t/
  uintptr_t       -> i64/u64
ptrdiff_t         -> i64
c_float/c_double  -> float/double
c_void            -> void
bool              -> i1
string            -> ptr to NUL-terminated bytes
ref T             -> ptr
ref mut T         -> ptr
ptr T             -> ptr
compact enum      -> i64 tagged union word
```

`string` values can be passed to `ptr c_char`, `ptr c_uchar`, or `ptr c_void`
parameters. Use `ptr c_void` for C `void*`; a by-value `c_void` parameter is
rejected.

The `null` literal can initialize or be passed to any `ptr T` type. `T?` is a
nullable raw-pointer spelling for the same type, so `c_void?` and `ptr c_void`
are equivalent in checked executable code. It does not create an `Option[T]` or
`Maybe[T]`; use those enum-like ADTs explicitly for nullable values that are
not raw pointers:

```ari
extern "C" fn takes_buffer(data: ptr c_void) -> i64;

fn main() -> i64 {
  let data: c_void? = null;
  return takes_buffer(data) + takes_buffer(null);
}
```

When `null` is used without an expected type, it defaults to `ptr c_void`.
Pointer-to-pointer casts, nullable raw-pointer casts, and pointer/integer
address casts use explicit `as` casts:

```ari
let raw: ptr c_void = null;
let bytes: ptr c_char = raw as ptr c_char;
let slot: i64? = (ref mut value) as i64?;
let addr: u64 = bytes as u64;
```

Use `ptr_offset(pointer, bytes)` for explicit byte-wise pointer arithmetic, or
`ptr_add(pointer, count)` for typed pointer arithmetic. `ptr_add` scales by the
current Ari layout size of scalar and aggregate `T`. Both keep the pointer type
of the first argument:

```ari
let raw: ptr c_void = 0u64 as ptr c_void;
let shifted: ptr c_void = ptr_offset(raw, 16);
let byte_ptr: ptr c_char = mem::ptr_offset(shifted as ptr c_char, 1);
let next_i64: ptr i64 = ptr_add(0u64 as ptr i64, 1);
let next_point: ptr Point = ptr_add(0u64 as ptr Point, 1);
let point_bytes = size_of<Point>();
let point_align = mem::align_of<Point>();
```

Enums with multi-payload or 64-bit payload aggregate layouts are not part of the
C ABI surface yet. Use fieldless or compact one-word enums for FFI-facing
values.

`*pointer` is available for raw-pointer load/store syntax:

```ari
var number: i64 = 3;
let raw: ptr i64 = (ref mut number) as ptr i64;
*raw = *raw + 9;
```

Aggregate raw-pointer dereference can copy a whole plain Ari-layout aggregate,
or reach a scalar field or element through the pointer.

```ari
struct Pair(i64, mut i64)

var pair = Pair(4, 5);
let raw: ptr Pair = (ref mut pair) as ptr Pair;
let copy = *raw;
*raw = Pair(copy.0 + 1, copy.1 + 1);
(*raw).1 = (*raw).0 + 6;
```

Like the helper functions, dereference syntax is unchecked and does not imply
null, bounds, alignment, aliasing, or lifetime validation. Whole raw-pointer
copies of aggregates that contain `own`, `ref`, or `ref mut` fields are
rejected until the ownership diagnostics for zone-backed memory are broadened.

Tuple parameters/returns, fixed arrays, vectors, structs, and generic types are not
ABI-lowered yet. Local stack tuples are an executable-language feature, not a C
FFI layout promise. Aggregate raw-pointer field/element access follows Ari's
current executable aggregate layout; it is not yet a `repr(C)` guarantee.

## Runtime Entry

The generated LLVM module keeps the host C entry point separate from Ari's
language entry point:

```text
@main -> @ari_entry(argc, argv) -> @"ari::main"() -> source fn main()
```

`@ari_entry` initializes the Ari runtime context before calling `ari::main` and
shuts the context down afterward.

## Planned FFI Surface

The next FFI pieces are `repr(C)` struct layout, aggregate-aware pointer layout
helpers, and non-C ABI adapters via explicit C-compatible shims.
