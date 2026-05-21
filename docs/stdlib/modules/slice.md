# Slice[T]

`Slice[T]` is Ari's borrowed contiguous view. It does not own storage and does
not allocate. Use it when a function needs to read or rearrange a span of
values without caring whether the values came from a local array, a local
vector, a `std::vec::Vec[T]`, or a byte-oriented `String`.

The API is intentionally natural and unsuffixed. The type parameter tells Ari
which element type is being used, so call sites should read like
`view.find(needle)`, `view.chunks(4)`, and `view.ordering(other)`.
For byte-oriented APIs, a string literal can be used directly as a
`Slice[u8]` value or receiver:

```ari
"ari".starts_with("ar")
"ari".slice(1, 3).equals("ri")
```

That receiver form is read-only and uses the literal's borrowed static bytes,
with the same first-NUL length rule as literal-to-`Slice[u8]` coercion.

## Access And Search

Metadata and element access:

```ari
view.len()
view.is_empty()
view.first()
view.try_first()
view.last()
view.try_last()
view.get(index)
view.try_get(index)
view[index]
view.as_ptr()
```

`first`, `last`, `get`, and indexing assert when the element is missing. Use
the `try_*` forms for ordinary absence; they return `Option[T]`.

Search and comparison:

```ari
view.index_of(value)
view.contains(value)
view.count(value)
view.find(needle)
view.contains_slice(needle)
view.starts_with(prefix)
view.ends_with(suffix)
view.equals(other)
view.compare(other)
view.ordering(other)
```

`index_of` and `contains` search for one value. `find` searches for a borrowed
subslice and returns the first index or `-1`; an empty needle matches at `0`.
`contains_slice` is the boolean wrapper. `compare` is lexicographic and returns
`-1`, `0`, or `1` for compatibility with existing code. Prefer `ordering` in
new code; it returns `cmp::Less`, `cmp::Equal`, or `cmp::Greater`, which
composes with `cmp::Ordering` helpers such as `is_less` and `then`.

## Views And Lazy Splitting

Borrowed subviews:

```ari
view.slice(start, end)
view.split_at(index)
```

`slice(start, end)` asserts `0 <= start <= end <= len` and returns a borrowed
view over that range. `split_at(index)` returns `SlicePair[T]`; call
`left()` and `right()` to inspect the two borrowed halves.

Lazy view iterators:

```ari
view.chunks(size)
view.windows(size)
view.split(delimiter)
```

`chunks(size)` yields non-overlapping `Slice[T]` views; the last chunk may be
shorter. `windows(size)` yields overlapping fixed-size views. `split(delimiter)`
yields views between delimiter values, including empty segments at boundaries.
All three assert that size arguments are positive where applicable and keep
the original storage borrowed.

## Copying And Algorithms

Copy into an owning vector:

```ari
view.copy_to(ref mut zone)
```

This returns `std::vec::Vec[T]` tied to the target zone.

Common in-place algorithms are available directly on the borrowed view:

```ari
view.reverse()
view.rotate_left(count)
view.rotate_right(count)
view.fill(value)
view.copy_from(source)
view.partition(keep)
view.dedup()
view.sort()
view.stable_sort()
view.is_sorted()
view.binary_search(target)
view.lower_bound(target)
view.upper_bound(target)
view.min()
view.max()
```

These methods forward to `std::algo`, so the algorithm policy still lives in
one module while call sites can use the natural receiver form. `copy_from`
copies the common prefix from `source` into the receiver and returns the number
of elements written. `dedup` compacts consecutive duplicate values in place and
returns the logical unique length; callers decide whether to truncate an owning
container. `partition` accepts a borrowed predicate and returns the split index.
`lower_bound` and `upper_bound` return sorted insertion indexes, which is useful
when missing values should be inserted or duplicate ranges must be counted.
The ordered methods require `T: std::cmp::Ord[T]`.

The free functions remain useful for generic utility code and for cases where
the target/source role is clearer as `algo::copy(target, source)`.

## Tests

Focused positive tests:

```text
tests/cases/standard-library/ok/vec/prelude-slice-methods.ari
tests/cases/standard-library/ok/vec/prelude-slice-sequence.ari
tests/cases/standard-library/ok/vec/prelude-slice-option-access.ari
tests/cases/standard-library/ok/vec/prelude-slice-copy-to.ari
```

`prelude-slice-sequence.ari` covers `slice`, `split_at`, `find`,
`contains_slice`, `compare`, `ordering`, `chunks`, `windows`, delimiter `split`, in-place
reordering, copying/filling, partition/dedup, sorting, binary search,
lower/upper bounds, and min/max wrappers.

## Limits And Roadmap

- `Slice[T]` is borrowed. Returning or storing it past the source storage is
  subject to Ari's usual provenance and borrow checks.
- `split` currently splits by one delimiter value. Predicate-based splitting
  should wait for richer closure/capture support.
- `compare` and `ordering` rely on the element type having an `lt`
  method/`Ord`-style behavior at monomorphization time. Generic trait
  diagnostics around this will improve with the broader trait-bound roadmap.
- Chunk and window iterators yield borrowed views, not owning vectors.
