# std::mem

`std::mem` is Ari's low-level memory and layout module. It exists for standard
library code, FFI glue, allocators, and data structures that need explicit raw
pointer work. Most application code should prefer higher-level handles such as
`String`, `Vec`, `Box`, and collection types.

## Current Scope

Implemented now:

- layout queries: `size_of<T>`, `align_of<T>`
- raw pointer arithmetic: `ptr_offset<T>`, `ptr_add<T>`
- raw pointer scalar/plain-aggregate access: `ptr_load<T>`, `ptr_store<T>`
- byte memory routines: `copy_bytes`, `move_bytes`, `set_bytes`
- hosted runtime page query: `page_size`
- value helpers: `replace<T>`, `swap<T>`

The byte routines are runtime-owned Ari builtins lowered by the LLVM backend:
`copy_bytes` uses `llvm.memcpy`, `move_bytes` uses `llvm.memmove`, and
`set_bytes` uses `llvm.memset`. They deliberately work on `ptr u8` and a byte
length so call sites stay explicit about raw memory size.

## API

```ari
mem::size_of<T>() -> i64
mem::align_of<T>() -> i64
mem::ptr_offset<T>(base: ptr T, bytes: i64) -> ptr T
mem::ptr_add<T>(base: ptr T, count: i64) -> ptr T
mem::ptr_load<T>(source: ptr T) -> T
mem::ptr_store<T>(target: ptr T, value: T) -> void
mem::copy_bytes(target: ptr u8, source: ptr u8, len: i64) -> void
mem::move_bytes(target: ptr u8, source: ptr u8, len: i64) -> void
mem::set_bytes(target: ptr u8, value: u8, len: i64) -> void
mem::page_size() -> i64
mem::replace<T>(target: ref mut T, value: T) -> T
mem::swap<T>(left: ref mut T, right: ref mut T) -> void
```

Root aliases exist for the long-standing layout and typed pointer helpers:
`size_of`, `align_of`, `ptr_offset`, `ptr_add`, `ptr_load`, `ptr_store`,
`replace`, and `swap`. The byte routines stay under `mem::` so raw bulk memory
operations remain visible at the call site.

`copy_bytes(target, source, len)` copies `len` bytes from non-overlapping
memory. Use `move_bytes` when the regions may overlap. `set_bytes(target,
value, len)` fills `len` bytes with `value`. A negative `len` traps through
the runtime. The functions do not validate that pointers are non-null or that
the regions are initialized; callers own that invariant.

`page_size()` returns the hosted runtime page size in bytes. It is useful for
checking alignment and planning future mapping APIs, but it does not allocate,
map, protect, or lock memory by itself. On the current Linux/LLVM runtime path
it lowers through `getpagesize`.

## Example

```ari
fn main() -> i64 {
  var zone = zone::create(64);
  let source = zone::alloc_array<u8>(ref mut zone, 4);
  let target = zone::alloc_array<u8>(ref mut zone, 4);

  ptr_store(source, 65u8);
  ptr_store(ptr_add(source, 1), 66u8);
  ptr_store(ptr_add(source, 2), 67u8);
  ptr_store(ptr_add(source, 3), 68u8);

  mem::set_bytes(target, 0u8, 4);
  mem::copy_bytes(target, source, 4);
  mem::move_bytes(ptr_add(target, 1), target, 3);

  let first = ptr_load(target);
  let page = mem::page_size();
  assert(page > 0);
  zone::destroy(zone);
  return first as i64;
}
```

## Design Notes

The API names are short and unit-aware. `copy_bytes`, `move_bytes`, and
`set_bytes` say that the count is a byte count, while typed operations keep
their type in the pointer or generic argument. Avoid names like
`copy_u8_buffer` when the pointer and byte length already carry that meaning.

These functions are not a replacement for ownership-aware moves. They are raw
memory primitives and should not be used to copy values that own resources
unless the surrounding API has a clear ownership and drop policy.

## Tests

- `tests/cases/standard-library/ok/mem/std-mem-value-helpers.ari` checks
  `replace` and `swap` for scalar and plain aggregate values.
- `tests/cases/standard-library/ok/mem/std-mem-byte-ops.ari` checks
  `copy_bytes`, `move_bytes`, `set_bytes`, overlapping move behavior, and LLVM
  intrinsic lowering.
- `tests/cases/standard-library/ok/mem/std-mem-page-size.ari` checks
  `page_size`, hosted runtime lowering, and basic page-size invariants.

Run `make check-std-api` after public API edits. For this module, a focused
manual check is:

```sh
build/ari tests/cases/standard-library/ok/mem/std-mem-byte-ops.ari --emit-llvm build/prelude/std-mem-byte-ops.ll
build/ari tests/cases/standard-library/ok/mem/std-mem-byte-ops.ari -o build/prelude/std-mem-byte-ops.elf
build/prelude/std-mem-byte-ops.elf
```
