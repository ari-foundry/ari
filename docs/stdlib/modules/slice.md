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
view.first_mut()
view.last()
view.try_last()
view.last_mut()
view.get(index)
view.try_get(index)
view.get_mut(index)
view[index]
view.as_ptr()
view.iter()
view.iter_mut()
```

`first`, `last`, `get`, and indexing assert when the element is missing. Use
the `try_*` forms for ordinary absence; they return `Option[T]`. `first_mut`,
`last_mut`, and `get_mut` assert on absence and return mutable borrows into
the same backing storage, so they are for writable slices derived from mutable
arrays, local vectors, or source `std::vec::Vec[T]` storage.
`iter()` returns `SliceIter[T]`, a borrowed value cursor that implements
`Iterator[T]`, `ExactSizeIterator[T]`, and `DoubleEndedIterator[T]`.
`iter_mut()` returns `SliceIterMut[T]`, which yields `SliceValueMut[T]`
handles. Call `handle.value()` to copy the current value or
`handle.value_mut()` to borrow the element mutably in place.

Search and comparison:

```ari
view.index_of(value)
view.contains(value)
view.count(value)
view.find_if(predicate)
view.position(predicate)
view.rposition(predicate)
view.any(predicate)
view.all(predicate)
view.count_if(predicate)
view.find(needle)
view.contains_slice(needle)
view.starts_with(prefix)
view.ends_with(suffix)
view.strip_prefix(prefix)
view.strip_suffix(suffix)
view.equals(other)
view.compare(other)
view.ordering(other)
```

`index_of` and `contains` search for one value. `find` searches for a borrowed
subslice and returns the first index or `-1`; an empty needle matches at `0`.
`contains_slice` is the boolean wrapper. `find_if`, `position`, `rposition`,
`any`, `all`, and `count_if` accept `fn(ref T) -> bool`, matching the predicate
shape used by `partition` and `partition_point`. `find_if` returns the first
matching value as `Option[T]`; `position` and `rposition` return the first or
last matching index, or `-1` when no element matches. `strip_prefix` and
`strip_suffix` return the remaining borrowed view as `Option[Slice[T]]`, so a
missing prefix or suffix can be handled without sentinel indexes. `compare` is
lexicographic and returns `-1`, `0`, or `1` for compatibility with existing
code. Prefer `ordering` in new code; it returns `cmp::Less`, `cmp::Equal`, or
`cmp::Greater`, which composes with `cmp::Ordering` helpers such as `is_less`
and `then`.

## Views And Lazy Splitting

Borrowed subviews:

```ari
view.slice(start, end)
view.split_at(index)
view.split_first()
view.split_last()
```

`slice(start, end)` asserts `0 <= start <= end <= len` and returns a borrowed
view over that range. `split_at(index)` returns `SlicePair[T]`; call
`left()` and `right()` to inspect the two borrowed halves. `split_first()`
returns `Option[(T, Slice[T])]`, where the tuple is `(first, rest)`.
`split_last()` returns `Option[(Slice[T], T)]`, where the tuple is
`(init, last)`. Empty slices return `None`.

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
view.reverse_range(start, end)
view.rotate_left(count)
view.rotate_right(count)
view.rotate_range(start, end, count)
view.fill(value)
view.fill_range(start, end, value)
view.copy_from(source)
view.copy_within(start, end, target)
view.partition(keep)
view.stable_partition(keep)
view.dedup()
view.dedup_by(same)
view.dedup_by_key(key)
view.sort()
view.sort_by(less)
view.stable_sort()
view.stable_sort_by(less)
view.is_sorted()
view.is_sorted_by(less)
view.binary_search(target)
view.binary_search_by(target, less)
view.lower_bound(target)
view.lower_bound_by(target, less)
view.upper_bound(target)
view.upper_bound_by(target, less)
view.equal_range(target)
view.equal_range_by(target, less)
view.partition_point(predicate)
view.min()
view.min_by(less)
view.max()
view.max_by(less)
```

These methods forward to `std::algo`, so the algorithm policy still lives in
one module while call sites can use the natural receiver form. `copy_from`
copies the common prefix from `source` into the receiver and returns the number
of elements written. `dedup`, `dedup_by`, and `dedup_by_key` compact
consecutive duplicate values in place and return the logical unique length;
callers decide whether to truncate an owning container. `partition` accepts a
borrowed predicate and returns the split index; `stable_partition` keeps the
relative order of both accepted and rejected elements.
The range forms use half-open indexes: `copy_within(start, end, target)` copies
`[start, end)` into the same slice starting at `target`, `fill_range` overwrites
`[start, end)`, `reverse_range` reverses that span, and `rotate_range` left
rotates only that span.
`lower_bound` and `upper_bound` return sorted insertion indexes, which is useful
when missing values should be inserted or duplicate ranges must be counted.
`equal_range` returns the `(lower, upper)` duplicate range directly, and
`partition_point` returns the first index where a borrowed predicate turns
false in an already predicate-partitioned slice.
The ordered methods require `T: std::cmp::Ord[T]`; the `*_by` methods take an
explicit `fn(T, T) -> bool` less-than callback for temporary ordering policies.

The free functions remain useful for generic utility code and for cases where
the target/source role is clearer as `algo::copy(target, source)`.

## Tests

Focused positive tests:

```text
tests/cases/standard-library/ok/vec/prelude-slice-methods.ari
tests/cases/standard-library/ok/vec/prelude-slice-sequence.ari
tests/cases/standard-library/ok/vec/prelude-slice-convenience.ari
tests/cases/standard-library/ok/algo/std-algo-by-helpers.ari
tests/cases/standard-library/ok/vec/prelude-slice-option-access.ari
tests/cases/standard-library/ok/vec/prelude-slice-copy-to.ari
tests/cases/standard-library/ok/iter/std-iter-slice-vec.ari
tests/cases/standard-library/ok/vec/std-vec-range-mutation.ari
```

`prelude-slice-convenience.ari` covers `strip_prefix`, `strip_suffix`,
`split_first`, `split_last`, `first_mut`, `last_mut`, `get_mut`, `find_if`,
`position`, `rposition`, `any`, `all`, and `count_if`.
`prelude-slice-sequence.ari` covers `slice`, `split_at`, `find`,
`contains_slice`, `compare`, `ordering`, `chunks`, `windows`, delimiter `split`, in-place
reordering, copying/filling, partition/dedup, sorting, binary search,
lower/upper/equal-range bounds, partition point, and min/max wrappers.
`std-iter-slice-vec.ari` covers `iter()` and mutable value cursors over borrowed
slices; `std-vec-range-mutation.ari` covers the half-open range mutation
wrappers shared with `std::vec::Vec[T]`.

## Limits And Roadmap

- `Slice[T]` is borrowed. Returning or storing it past the source storage is
  subject to Ari's usual provenance and borrow checks.
- `split` currently splits by one delimiter value. Predicate-based splitting
  should wait for richer closure/capture support.
- `compare` and `ordering` rely on the element type having an `lt`
  method/`Ord`-style behavior at monomorphization time. Generic trait
  diagnostics around this will improve with the broader trait-bound roadmap.
- Chunk and window iterators yield borrowed views, not owning vectors.
