# Aggregate ABI Classification

Ari classifies non-local aggregate values in `src/aggregate_abi.*`. The
classifier is the shared policy gate for public function boundaries, C-header
emission, future C aggregate imports, and library collection handles.

The current 0.x policy recognizes value tuples, fixed arrays, structs,
fixed-capacity `Vec` storage values, and aggregate-layout enums. Scalars,
pointers, borrows, owned handles, and trait objects are outside this aggregate
classifier and use their own ABI rules.

## Current Policy

On 64-bit Unix targets, an aggregate is classified as direct when Ari can
compute its layout, the size is non-zero, the size is at most 16 bytes, and the
alignment is at most 8 bytes.

Aggregates that are larger than 16 bytes or require alignment above 8 bytes are
classified as indirect. Current C-header emission rejects those by-value
signatures and asks the source API to expose an explicit pointer ABI instead.

Non-64-bit Unix targets, zero-sized aggregates, and aggregates whose layout
cannot be computed are classified as unsupported for by-value C-header emission.
This keeps generated headers from promising a C ABI that Ari does not yet lower
consistently across targets.

## Users

C-header emission now calls the shared classifier before rendering exported
function prototypes. Existing direct by-value `@repr(C)` structs and fixed
arrays keep their generated C surface. Tuples, fixed-capacity vectors, and
aggregate-layout enums are classified by the same policy, but most of those
source shapes still need an explicit C representation before headers can expose
them directly.

The next backend work should use the same classifier when adding C aggregate
imports or richer generated C wrapper types, rather than re-encoding target,
size, and alignment checks in each backend surface.
