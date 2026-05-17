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
spellings. Existing direct by-value `@repr(C)` exports and fixed arrays keep
their generated C-header surface. Tuples, fixed-capacity vectors, and
aggregate-layout enums are classified by the same policy, but still need an
explicit C representation before headers or imports can expose them directly.

The next backend work should use the same classifier when adding richer
generated C wrapper types, rather than re-encoding target, size, and alignment
checks in each backend surface.
