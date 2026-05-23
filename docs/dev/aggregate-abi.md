# Aggregate ABI Classification

Ari classifies non-local aggregate values in `src/aggregate_abi.*`. The
classifier is the shared policy gate for public function boundaries, C-header
emission, direct C aggregate imports, and library collection handles.

The current 0.x policy recognizes value tuples, fixed arrays, structs,
fixed-capacity `Vec` storage values, and aggregate-layout enums. Scalars,
pointers, borrows, owned handles, and trait objects are outside this aggregate
classifier and use their own ABI rules.

## Current Policy

On 64-bit Unix targets, an aggregate is classified as direct when Ari can
compute its layout, the size is non-zero, the size is at most 16 bytes, and the
alignment is at most 8 bytes.

Aggregates that are larger than 16 bytes or require alignment above 8 bytes are
classified as indirect. Current C-header emission and direct C imports reject
those by-value signatures and ask the source API to expose an explicit pointer
ABI instead.

Non-64-bit Unix targets, zero-sized aggregates, and aggregates whose layout
cannot be computed are classified as unsupported for by-value C-header emission
and direct C imports. This keeps generated headers and extern declarations from
promising a C ABI that Ari does not yet lower consistently across targets.

## Users

Extern C signature collection and C-header emission now call the shared
classifier before accepting by-value aggregate boundaries. Direct C imports
accept classifier-approved `@repr(C)` structs by value, including small struct
returns, and reject larger, target-unsupported, or non-`repr(C)` aggregate
spellings.

Unsupported FFI aggregate boundaries are source-aware ABI diagnostics. The
diagnostic catalog uses `A0001` for ABI layout, C FFI declarations, C-header
emission, and link-boundary checks; `make check-compiler-artifacts` locks
non-`@repr(C)`, oversized, and target-unsupported by-value import rejections as
golden diagnostic artifacts.

C-header emission exposes direct by-value aggregate exports through generated C
wrapper structs when Ari's source spelling is not already C-spellable. Fixed
arrays use `AriArray_*` wrappers with an `elements` array field. Tuples use
`AriTuple_*` wrappers with `field0`, `field1`, and later positional fields.
Fixed-capacity vector storage values use `AriVec_*` wrappers with the current
`len` plus local `data[N]` storage layout. Aggregate-layout enums use
`AriEnum_*` wrappers with the hidden `tag` field followed by scalar,
pointer-shaped, or generated-wrapper `payloadN` storage slots. These wrappers
are header surfaces for Ari's current LLVM ABI; they are not a promise that the
same spelling is accepted as a direct C import type.

`make check-compiler-artifacts` compares C header goldens for `@repr(C)` struct
fields, fieldless enum tags, payload-bearing enum structs, and generated
tuple/array/fixed-vector/aggregate-enum wrapper types. This keeps layout-facing
header spellings deterministic instead of relying only on individual grep
checks in `make check-ffi`.

Future backend work should keep using this classifier when growing imported C
aggregate support, rather than re-encoding target, size, and alignment checks in
each backend surface.
