# std::vec

`std::vec` contains Ari's source growable sequence handle. It exists to give
programs an explicit-zone collection that can grow, copy, expose borrowed
views, and participate in Ari's iterator surface without hiding allocation.

There are two vector-shaped surfaces today:

- Bare `Vec[T]` is the compiler-known local vector/literal storage surface.
- `std::vec::Vec[T]` and root alias `std::Vec[T]` are source handles backed by
  a `Zone`.

Use the source handle when a collection must grow or outlive local literal
storage under an explicit allocation capability.

## Constructors And Copies

Constructors allocate in an explicit zone:

```ari
std::vec::new<T>(ref mut zone, capacity)
Vec!(T, ref mut zone, capacity)
std::vec::from_slice_in<T>(ref mut zone, values)
```

`new` creates an empty vector with starting capacity. `Vec!` is shorthand for
the same constructor. `from_slice_in` copies a borrowed `Slice[T]` into a new
target-zone vector.

Copies also require a target zone:

```ari
vec.copy_to(ref mut zone)
slice.copy_to(ref mut zone)
```

The returned handle is tied to the target zone. Ari rejects use of a tracked
vector, its raw pointer, or a derived slice after that zone is reset or
destroyed.

## Metadata And Access

Metadata and asserting accessors:

```ari
vec.len()
vec.capacity()
vec.is_empty()
vec.first()
vec.last()
vec.get(index)
```

`first`, `last`, and `get` assert when the requested element is absent. Use
them when a bad index is a programmer error.

Option-returning accessors:

```ari
vec.try_first()
vec.try_last()
vec.try_get(index)
```

These return `Option[T]`. They are the preferred shape when an empty vector or
out-of-range index is normal input.

Borrowed element access:

```ari
vec.get_ref(index)
vec.get_mut(index)
```

These return shared or mutable element borrows and participate in the normal
borrow checker rules.

## Mutation And Growth

Fixed-capacity mutation:

```ari
vec.push(value)
vec.pop()
vec.try_pop()
vec.set(index, value)
vec.replace(index, value)
vec.swap(left, right)
vec.insert(index, value)
vec.remove(index)
vec.try_remove(index)
vec.truncate(length)
vec.retain(keep)
vec.dedup()
vec.fill(value)
vec.copy_from(source)
vec.partition(keep)
vec.clear()
```

`try_pop` and `try_remove` return `Option[T]`; `pop` asserts when empty and
`remove` asserts when the index is absent. Use `try_remove` when a missing
index is ordinary input. `retain` keeps values accepted by `keep: fn(ref T) ->
bool`, preserves the order of kept values, and drops rejected values. `set`,
`clear`, `truncate`, and shrink paths drop removed live elements before
reducing the logical length. `dedup` compacts consecutive duplicates, truncates
the vector to the unique prefix, and returns the new length. `fill` overwrites
the live prefix with one value, `copy_from` copies the source prefix that fits
and returns the copied count, and `partition` reorders live values by a
borrowed predicate and returns the split index.

Growth-capable mutation uses the owning zone:

```ari
vec.push_in(ref mut zone, value)
vec.insert_in(ref mut zone, index, value)
vec.reserve(ref mut zone, capacity)
vec.reserve_extra(ref mut zone, additional)
vec.extend_from_slice_in(ref mut zone, values)
vec.resize_in(ref mut zone, length, value)
```

The explicit `_in` forms make the allocation capability visible. For tracked
local source vectors, Ari can infer the same source zone for common
non-`_in` convenience calls documented in the language guide.

## Search, Slices, And Iteration

Search and slice comparison:

```ari
vec.index_of(value)
vec.contains(value)
vec.count(value)
vec.find(needle)
vec.contains_slice(needle)
vec.starts_with(values)
vec.ends_with(values)
vec.equals(values)
vec.compare(values)
vec.ordering(values)
```

`find` searches for a borrowed `Slice[T]` inside the vector and returns the
first index or `-1`; an empty needle matches at `0`. `contains_slice` is the
boolean wrapper. `compare` is lexicographic and returns `-1`, `0`, or `1` for
compatibility; prefer `ordering` in new code because it returns the typed
`cmp::Ordering` values used by the comparison helpers.

Borrowed views and raw pointers:

```ari
vec.slice(start, end)
vec.split_at(index)
vec.chunks(size)
vec.windows(size)
vec.split(delimiter)
vec.reverse()
vec.rotate_left(count)
vec.rotate_right(count)
vec.sort()
vec.sort_by(less)
vec.stable_sort()
vec.stable_sort_by(less)
vec.is_sorted()
vec.is_sorted_by(less)
vec.binary_search(value)
vec.binary_search_by(value, less)
vec.lower_bound(value)
vec.lower_bound_by(value, less)
vec.upper_bound(value)
vec.upper_bound_by(value, less)
vec.min()
vec.min_by(less)
vec.max()
vec.max_by(less)
vec.as_slice()
vec.as_ptr()
vec.as_mut_ptr()
```

`slice` and `split_at` return borrowed `Slice[T]` views into the live vector
storage. `chunks`, `windows`, and delimiter `split` return lazy iterators that
yield borrowed `Slice[T]` views, so they do not allocate. `reverse`, the
rotation helpers, `sort`, and `stable_sort` mutate the existing storage in
place through the same algorithm module helpers used for borrowed slices.
`is_sorted`, `binary_search`, `lower_bound`, `upper_bound`, `min`, and `max`
are available when `T` implements `Ord[T]`. The `*_by` variants take an
explicit `fn(T, T) -> bool` less-than callback when a vector needs a temporary
ordering policy without a type-wide `Ord` impl. The bound helpers return sorted
insertion indexes, making them useful for keeping a vector sorted after a
lookup miss. `as_slice` is the whole-vector view. The pointer helpers preserve
zone provenance in the checker.

Iterator entry point:

```ari
vec.iter()
```

`std::vec::Iter[T]` implements `Iterator[T]`, and `std::vec::Vec[T]`
implements `IntoIterator[T]` for the current for-loop lowering.

## Slice Accessors

`Slice[T]` is defined at the `std` root, but it shares the same read vocabulary
with source vectors:

```ari
view.is_empty()
view.first()
view.try_first()
view.last()
view.try_last()
view.get(index)
view.try_get(index)
view.index_of(value)
view.contains(value)
view.count(value)
view.find(needle)
view.contains_slice(needle)
view.starts_with(other)
view.ends_with(other)
view.equals(other)
view.compare(other)
view.ordering(other)
view.copy_to(ref mut zone)
```

Use `Slice[T]` when you only need a borrowed view. `is_empty` checks the
view's stored length without touching elements. Use `try_*` accessors when
absence is a normal branch and `first`/`last`/`get` when absence should trap.

Borrowed view shaping and lazy splitting:

```ari
view.slice(start, end)
view.split_at(index)
view.chunks(size)
view.windows(size)
view.split(delimiter)
```

`slice` and `split_at` return borrowed views into the same storage. `chunks`,
`windows`, and delimiter `split` are lazy iterators that yield `Slice[T]`
views, so they do not allocate. The dedicated [Slice guide](slice.md) covers
their exact boundary behavior.

## Example

```ari
fn main() -> i64 {
  var zone = zone::create(128);
  var values = std::vec::new<i64>(ref mut zone, 2);

  values.push(5);
  values.push_in(ref mut zone, 8);

  let first = values.try_first().unwrap_or(0);
  let second = values.try_get(1).unwrap_or(0);
  let view = values.as_slice();

  let total = first + second + view.try_last().unwrap_or(0);
  zone::destroy(zone);
  return total;
}
```

## Tests

Focused positive tests include:

```text
tests/cases/standard-library/ok/vec/prelude-slice-methods.ari
tests/cases/standard-library/ok/vec/prelude-slice-sequence.ari
tests/cases/standard-library/ok/vec/prelude-slice-metadata.ari
tests/cases/standard-library/ok/vec/prelude-slice-option-access.ari
tests/cases/standard-library/ok/vec/prelude-slice-copy-to.ari
tests/cases/standard-library/ok/vec/std-vec-metadata-methods.ari
tests/cases/standard-library/ok/vec/std-vec-fixed-ops.ari
tests/cases/standard-library/ok/vec/std-vec-try-access.ari
tests/cases/standard-library/ok/vec/std-vec-try-pop.ari
tests/cases/standard-library/ok/vec/std-vec-try-remove.ari
tests/cases/standard-library/ok/vec/std-vec-retain.ari
tests/cases/standard-library/ok/vec/std-vec-slice-compare.ari
tests/cases/standard-library/ok/vec/std-vec-sequence.ari
tests/cases/standard-library/ok/vec/std-vec-growth-paths.ari
tests/cases/standard-library/ok/vec/std-vec-iter.ari
```

`make check-prelude` compiles these to LLVM, checks representative symbols,
and runs executable checks where behavior is observable. Public methods are
tracked in `tests/std_api_manifest.txt` and checked by `make check-std-api`.

## Future Work

Potential next slices:

- iterator adapters after general iterator protocol diagnostics mature
- richer borrow-returning slice APIs after generic lifetime rules are clearer
- maps, sets, and deques after generic aggregate monomorphization is stronger
- eventual unification plan for bare local `Vec[T]` and source `std::Vec[T]`
