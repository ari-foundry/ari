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

## Borrow-Returning Externs

Extern declarations that return `ref T` or `ref mut T` must state where the
borrow comes from with `@borrow_return(source)`. Ari cannot inspect a C body,
so an unannotated borrow-returning extern is rejected when called.

```ari
@borrow_return(value)
extern "C" fn identity_ref(value: ref i64) -> ref i64;

@borrow_return(value)
extern "C" fn identity_mut_ref(value: ref mut i64) -> ref mut i64;
```

The contract keeps the caller's source borrowed for as long as the result
binding lives. If the call passes `ref pair.left`, only `pair.left` remains
borrowed, so unrelated fields can still be borrowed or assigned normally.

## Ari Builtin ABI

`extern "ari"` is not C FFI. It is reserved for compiler/runtime builtins that
Ari itself defines, such as the standard `std` header declarations for IO,
assertions, context, and zones:

```ari
extern "ari" fn write_i64(value: i64) -> i64 = "ari_builtin_write_i64";
extern "ari" fn write_u64(value: u64) -> i64 = "ari_builtin_write_u64";
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
`std::mem::replace<T>(ref mut target, value)` and
`std::mem::swap<T>(ref mut left, ref mut right)` are source helpers over the
same scalar/plain-aggregate raw-place materialization rules. They intentionally
reject ownership- or borrow-valued `T` for the 0.x library-prep surface because
they use raw-copy materialization rather than a move-aware place contract.
`size_of<T>()` and `align_of<T>()` expose the current scalar and Ari-layout
aggregate size/alignment model for explicit pointer code. The raw
`--freestanding` backend uses those same Ari byte offsets for tuple, struct,
tuple-struct, and fixed-array pointer field/element access; aggregate enum
pointer loads and stores can copy the whole aggregate enum value, and direct
enum-constructor stores such as `ptr_store(raw, Some(5))` or `*raw = Some(5)`
write the current Ari tag/payload layout. Aggregate enum payload slots can also
be addressed directly with tuple-index syntax on a local or raw-pointer-backed
enum value. For example, `value.0` and `(*raw).0` address payload slot 0 while
the hidden tag remains an implementation field. This access is intentionally
low-level: it does not check which case is currently active, and
scalar/pointer-shaped payload slots expose their stored `u64` payload word.
Raw `--freestanding --emit-obj` output can emit direct imported `extern "C"`
calls for scalar and raw-pointer signatures as undefined C symbols with ELF
relocations. The supported raw-import slice covers integer and bool values,
lowercase `string`/function-pointer slots, `ptr`/`ref`/`ref mut` pointer-shaped
slots, and `c_void` returns. Raw executable output still has no linker phase, so
the same imported call is rejected unless object output is used and an external
linker supplies the C symbol.
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
static symbol table. Relocatable raw object output can additionally reference
scalar/raw-pointer imported `extern "C"` symbols through `.rela.text` call
relocations; direct raw executable output still rejects imported symbols.

## Type Mapping

Current LLVM host C FFI type mapping follows the selected `--target` triple.
Without `--target`, Ari uses the host default target triple. The selected
triple is canonicalized before semantic checking, LLVM emission, and module
metadata/cache emission, so changing targets invalidates cached package graphs.

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
ref T             -> ptr (C headers spell this as const T*)
ref mut T         -> ptr
ptr T             -> ptr
compact enum      -> i64 tagged union word
```

`string` values can be passed to `ptr c_char`, `ptr c_uchar`, or `ptr c_void`
parameters, and can be explicitly cast to raw pointer types when low-level
byte access is needed. Use `ptr c_void` for C `void*`; a by-value `c_void`
parameter is rejected.

For the current supported target tables, `c_char` is signed (`i8`). Use
`c_schar` or `c_uchar` when an API needs explicit signedness. Pointer-width
aliases such as `size_t`, `ptrdiff_t`, `intptr_t`, `usize`, and `isize`, plus
`c_long`/`c_ulong`, follow the selected target's ILP32/LP64/LLP64 layout. For
example, `c_long` is 64-bit on LP64 Unix targets, but 32-bit on ILP32 and
LLP64 Windows targets.

The `null` literal can initialize or be passed to any `ptr T` type. `T?` is a
nullable raw-pointer spelling for the same type, so `c_void?` and `ptr c_void`
are equivalent in checked executable code. It does not create an `Option[T]`;
use that enum-like ADT explicitly for nullable values that are not raw pointers:

```ari
extern "C" fn takes_buffer(data: ptr c_void) -> i64;

fn main() -> i64 {
  let data: c_void? = null;
  return takes_buffer(data) + takes_buffer(null);
}
```

When `null` is used without an expected type, it defaults to `ptr c_void`.
Pointer-to-pointer casts, nullable raw-pointer casts, lowercase `string` to raw
pointer casts, and pointer/integer address casts use explicit `as` casts:

```ari
let raw: ptr c_void = null;
let bytes: ptr c_char = raw as ptr c_char;
let slot: i64? = (ref mut value) as i64?;
let addr: u64 = bytes as u64;
```

Use `ptr_offset(pointer, bytes)` for explicit byte-wise pointer arithmetic, or
`ptr_add(pointer, count)` for typed pointer arithmetic. `ptr_add` scales by the
current Ari layout size of scalar and aggregate `T`. Both keep the pointer type
of the first argument and may also take an explicit `<T>` type argument when
the call should validate the pointee type:

```ari
let raw: ptr c_void = 0u64 as ptr c_void;
let shifted: ptr c_void = ptr_offset(raw, 16);
let byte_ptr: ptr c_char = mem::ptr_offset(shifted as ptr c_char, 1);
let next_i64: ptr i64 = ptr_add(0u64 as ptr i64, 1);
let next_point: ptr Point = ptr_add(0u64 as ptr Point, 1);
let explicit_next: ptr i64 = ptr_add<i64>(next_i64, 1);
let point_bytes = size_of<Point>();
let point_align = mem::align_of<Point>();
```

Enums with multi-payload or 64-bit payload aggregate layouts are not part of the
C ABI surface yet. Use fieldless or compact one-word enums for FFI-facing
values.

`*pointer` is available for raw-pointer load/store syntax.
`ptr_load<T>(pointer)` and `ptr_store<T>(pointer, value)` are also accepted
when the pointer element type should be checked explicitly at the call site.

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
copies of values that contain `own`, `ref`, or `ref mut` state are rejected
until the ownership diagnostics for zone-backed memory are broadened.

Tuple parameters/returns, fixed arrays, vectors, generic types, and non-`repr(C)`
structs are not ABI-lowered yet. Local stack tuples are an executable-language
feature, not a C FFI layout promise. Aggregate raw-pointer field/element access
follows Ari's current executable aggregate layout; it is not yet a `repr(C)`
guarantee.

## C Header Emission

`--emit-c-header path` writes a C header for Ari declarations that are visible
from the LLVM/shared-library ABI surface. The first supported slice covers
exported scalar/raw-pointer functions, public non-generic `@repr(C)` structs
whose fields are scalar, raw pointer, `ref`, or `ref mut` slots, and public
fieldless `@repr(C)` enums. Exported functions may take or return public
non-generic `@repr(C)` structs by value in the header. Public generic
`@repr(C)` structs are exposed as opaque typedefs for pointer-only APIs, and
exported by-value instantiations get concrete C names such as
`GenericHandle_i64`:

```sh
ari api.ari --shared --emit-c-header api.h --emit-llvm api.ll
```

Explicit `@export("symbol")` and `@no_mangle` functions use that C symbol.
Public functions without an explicit export use Ari's mangled symbol, matching
the shared-library output. Immutable `ref` slots are written as `const T*` in
C headers, while `ref mut T` and `ptr T` are written as mutable `T*`. Fixed-size
array fields inside public `@repr(C)` structs are emitted as C array
declarators, and explicit `ptr [T, N]`, `ref [T, N]`, and `ref mut [T, N]`
parameters or fields are emitted as pointer-to-array declarators. Direct
by-value `[T, N]` exported parameters and returns use generated wrapper structs
such as `AriArray_i64_2`, whose single `elements` field carries the C array
layout. Fieldless enum declarations are emitted as
`typedef int64_t Name;` plus prefixed integer constants such as `Name_Case = 0`
so the header matches Ari's current fixed tag ABI instead of relying on C's
implementation-defined enum width. Generic fieldless enum type parameters do
not affect layout, so `WireStatus[i64]` and `WireStatus[bool]` share one C
typedef named `WireStatus`. Private helpers and private `@repr(C)` aggregates
are not emitted. Generic `@repr(C)` structs still keep the source-name opaque
`typedef struct Name Name;` declaration for pointer-only APIs; concrete
by-value instantiations use separate typedefs/definitions keyed by their type
arguments. By-value struct parameters and returns are emitted only for direct
aggregate ABI values on 64-bit Unix targets, currently up to 16 bytes with at
most 8-byte alignment; direct fixed-size array parameters and returns use the
same size, alignment, and target guard. Larger records, larger arrays, and
non-Unix targets should expose an explicit pointer ABI. Header generation
currently rejects Ari-only values such as `string`, ownership-qualified values,
and non-`repr(C)` aggregate parameters or returns; expose a `ptr c_char`,
`ptr c_void`, or other scalar/raw pointer C ABI type until those layouts are
defined.

## Runtime Entry

The generated LLVM module keeps the host C entry point separate from Ari's
language entry point:

```text
@main -> @ari_entry(argc, argv) -> @"ari::main"() -> source fn main()
```

`@ari_entry` initializes the Ari runtime context before calling `ari::main` and
shuts the context down afterward.

## Planned FFI Surface

The next FFI pieces are payload-bearing enum layouts, broader target-specific
aggregate ABI support, and non-C ABI adapters via explicit C-compatible shims.
