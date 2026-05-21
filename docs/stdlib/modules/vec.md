# std::vec

`std::vec` contains Ari's source growable sequence handle. It exists to give
programs an explicit-zone collection that can grow, copy, expose borrowed
views, and participate in Ari's iterator surface while keeping the allocation
zone attached to the handle.

There are two vector-shaped surfaces today:

- Bare `Vec[T]` is the compiler-known local vector/literal storage surface.
- `std::vec::Vec[T]` and root alias `std::Vec[T]` are source handles that keep
  their owning `Zone` pointer internally.

Use the source handle when a collection must grow or outlive local literal
storage under an explicit allocation capability.

## Constructors And Copies

Constructors allocate in an explicit zone:

```ari
std::vec::new<T>(ref mut zone, capacity)
Vec::with_capacity<T>(ref mut zone, capacity)
Vec!(T, ref mut zone, capacity)
std::vec::from_slice_in<T>(ref mut zone, values)
```

`new` creates an empty vector with starting capacity. `Vec::with_capacity` is
the associated-constructor spelling for the same operation, and `Vec!` is the
macro shorthand. `from_slice_in` copies a borrowed `Slice[T]` into a new
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

Natural mutation uses the vector's owning zone:

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
vec.swap_remove(index)
vec.truncate(length)
vec.retain(keep)
vec.dedup()
vec.dedup_by(same)
vec.dedup_by_key(key)
vec.fill(value)
vec.fill_range(start, end, value)
vec.copy_from(source)
vec.copy_within(start, end, target)
vec.partition(keep)
vec.stable_partition(keep)
vec.clear()
vec.reserve(capacity)
vec.try_reserve(capacity)
vec.reserve_extra(additional)
vec.shrink_to_fit()
vec.extend(values)
vec.extend_iter(iter)
vec.extend_from_slice(values)
vec.append(ref mut other)
vec.resize(length, value)
vec.drain()
vec.drain_range(start, end)
vec.insert_many(index, values)
vec.remove_range(start, end)
vec.splice(start, end, replacement)
vec.split_off(index)
```

`try_pop` and `try_remove` return `Option[T]`; `pop` asserts when empty and
`remove` asserts when the index is absent. Use `try_remove` when a missing
index is ordinary input. `swap_remove(index)` removes one value in O(1) by
moving the last live element into the removed slot, so it does not preserve
order. `retain` keeps values accepted by `keep: fn(ref T) -> bool`, preserves
the order of kept values, and drops rejected values. `set`, `clear`,
`truncate`, and shrink paths drop removed live elements before reducing the
logical length. `dedup`, `dedup_by`, and `dedup_by_key` compact consecutive
duplicates, truncate the vector to the unique prefix, and return the new
length. `fill` overwrites the live prefix with one value, `copy_from` copies
the source prefix that fits and returns the copied count, `partition` reorders
live values by a borrowed predicate and returns the split index, and
`stable_partition` preserves the relative order of both partitions.
`copy_within(start, end, target)` copies the half-open source range inside the
same vector with overlap-safe direction, while `fill_range` overwrites only
`[start, end)`. `extend` is the natural alias for `extend_from_slice`;
`extend_iter<I: Iterator[T]>(iter)` consumes any Ari iterator and pushes each
yielded value into the receiver. `append` moves every live value from another
vector into the receiver and leaves the source vector empty. `insert_many`
inserts a borrowed slice at one index, `remove_range` drops the selected
half-open range and shifts the tail left, and `splice` replaces a half-open
range with a borrowed replacement slice. `split_off(index)` moves the tail
`[index, len)` into a new `Vec[T]` backed by the same zone metadata and leaves
the receiver with the prefix. The source declaration carries an inferred
same-zone parameter so the compiler can keep the returned vector tied to the
receiver's allocation zone; normal callers still write `vec.split_off(index)`.
`drain_range(start, end)` removes a half-open range and returns a drain cursor
over exactly those removed values. `drain()` empties the vector and returns a
`std::vec::Drain[T]` cursor over the removed live values; unconsumed drain
items are dropped when the cursor is dropped. `shrink_to_fit` moves live values
into a new backing allocation whose logical capacity equals `len()`. Since
Ari zones are bump-style allocation capabilities, this shrinks the handle's
capacity but does not reclaim old bytes until the zone is reset or destroyed.
`try_reserve` returns `false` for negative capacities and otherwise follows
the same allocation policy as `reserve`; allocation failure is still governed
by the runtime zone policy. `push`, `insert`, `reserve`, `try_reserve`,
`reserve_extra`, `extend`, `extend_from_slice`, `append`, `insert_many`,
`splice`, and growing `resize` use `ZoneMetadata` captured at construction.
The metadata can also be recovered from a non-empty backing allocation header,
so ordinary callers do not need to pass `ref mut zone` after construction.

### Element Movement Contract

`std::vec::Vec[T]` is the growable sequence handle, but its current growth,
copy, sort, fill, and range-edit internals still move values with the same
copy-oriented raw place operations documented in
[Value Movement Contracts](../value-contracts.md). Those helpers are
production-safe today for scalar and plain copyable elements. That means:

- `copy_to`, `extend`, `extend_from_slice`, `insert_many`, and replacement
  slices copy borrowed elements; they do not consume the source.
- `copy_within` and `fill_range` are copy-oriented range edits. They are not
  move-only resource transfer or clone-generation APIs.
- Growing `resize(length, value)` writes the same value repeatedly. It is not a
  clone or generator API for move-only resources.
- Shrinking `resize`, `truncate`, `clear`, `remove_range`, `retain`, and
  `Vec::dedup*` drop removed live values exactly once through normal `Drop`
  lowering. `swap_remove` returns the removed value and moves the old tail
  slot into the hole, so only the returned value is consumed by the caller.
- `drain()` immediately makes the vector empty and owns the removed live range
  through the drain cursor; `drain_range(start, end)` does the same for only
  that half-open range after shifting the retained tail left. Dropping either
  cursor drops any unconsumed values.
- `append(ref mut other)` leaves `other` empty after transferring its live
  prefix, but it still relies on the current raw place model and should not be
  treated as the final resource-owner move API.
- `split_off(index)` transfers the tail into another vector handle with the
  same zone metadata. The source no longer owns those slots after its length is
  shortened.

Resource-owning elements should wait for explicit move-aware place operations,
`Clone`/generator-based growth APIs, and by-reference comparator forms.

Explicit-zone compatibility forms remain available:

```ari
vec.push_in(ref mut zone, value)
vec.insert_in(ref mut zone, index, value)
vec.reserve_in(ref mut zone, capacity)
vec.reserve_extra_in(ref mut zone, additional)
vec.extend_from_slice_in(ref mut zone, values)
vec.resize_in(ref mut zone, length, value)
```

The explicit `_in` forms are mostly for compatibility and low-level tests. New
code should prefer the natural owning-zone names unless it is deliberately
testing zone capability plumbing.

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
vec.reverse_range(start, end)
vec.rotate_left(count)
vec.rotate_right(count)
vec.rotate_range(start, end, count)
vec.stable_partition(keep)
vec.dedup_by(same)
vec.dedup_by_key(key)
vec.sort()
vec.sort_by(less)
vec.stable_sort()
vec.stable_sort_by(less)
vec.stable_sort_in(ref mut zone)
vec.stable_sort_by_in(less, ref mut zone)
vec.try_stable_sort()
vec.try_stable_sort_by(less)
vec.is_sorted()
vec.is_sorted_by(less)
vec.binary_search(value)
vec.binary_search_by(value, less)
vec.lower_bound(value)
vec.lower_bound_by(value, less)
vec.upper_bound(value)
vec.upper_bound_by(value, less)
vec.equal_range(value)
vec.equal_range_by(value, less)
vec.partition_point(predicate)
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
rotation helpers, their half-open `reverse_range`/`rotate_range` variants,
`sort`, and `stable_sort` mutate the existing storage in place through the same
algorithm module helpers used for borrowed slices. `stable_sort` uses an
internal temporary zone for merge-sort storage, while `stable_sort_in` and
`stable_sort_by_in` use a caller-provided temporary zone. The `try_stable_sort`
forms return `Result[(), Error]` for the same preflight contract exposed by
`std::algo`.
The borrowed-view `dedup_by*` forms return a logical length without truncating;
the owning mutation forms above truncate the vector.
`is_sorted`, `binary_search`, `lower_bound`, `upper_bound`, `equal_range`,
`min`, and `max` are available when `T` implements `Ord[T]`. The `*_by` variants take an
explicit `fn(T, T) -> bool` less-than callback when a vector needs a temporary
ordering policy without a type-wide `Ord` impl. The bound helpers return sorted
insertion indexes, making them useful for keeping a vector sorted after a
lookup miss. `equal_range` returns the matching `(lower, upper)` duplicate
range, and `partition_point` finds the first false predicate index in a
predicate-partitioned vector. `as_slice` is the whole-vector view. The pointer helpers preserve
zone provenance in the checker.

Iterator entry point:

```ari
vec.iter()
vec.iter_mut()
vec.into_iter()
```

`std::vec::Iter[T]` implements `Iterator[T]`, and `std::vec::Drain[T]`
implements `Iterator[T]`, `ExactSizeIterator[T]`, and
`DoubleEndedIterator[T]`. `vec.iter_mut()` returns the same
`SliceIterMut[T]` mutable value cursor used by borrowed slices; call
`handle.value_mut()` on each yielded handle to edit an element in place.
`std::vec::Vec[T]` implements `IntoIterator[T]` for the current for-loop
lowering, so direct `for value in vec` uses the vector iterator path.

## Slice Accessors

`Slice[T]` is defined at the `std` root, but it shares the same read vocabulary
with source vectors:

```ari
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
view.starts_with(other)
view.ends_with(other)
view.strip_prefix(other)
view.strip_suffix(other)
view.equals(other)
view.compare(other)
view.ordering(other)
view.copy_to(ref mut zone)
```

Use `Slice[T]` when you only need a borrowed view. `is_empty` checks the
view's stored length without touching elements. Use `try_*` accessors when
absence is a normal branch and `first`/`last`/`get` when absence should trap.
The `_mut` accessors return writable element borrows, and the `*_if` /
position helpers scan with `fn(ref T) -> bool` predicates.

Borrowed view shaping and lazy splitting:

```ari
view.slice(start, end)
view.split_at(index)
view.split_first()
view.split_last()
view.chunks(size)
view.windows(size)
view.split(delimiter)
```

`slice`, `split_at`, `split_first`, `split_last`, `strip_prefix`, and
`strip_suffix` return borrowed views into the same storage. `chunks`, `windows`,
and delimiter `split` are lazy iterators that yield `Slice[T]` views, so they
do not allocate. The dedicated [Slice guide](slice.md) covers their exact
boundary behavior.

## Example

```ari
fn main() -> i64 {
  var zone = zone::create(128);
  var values = std::vec::new<i64>(ref mut zone, 2);

  values.push(5);
  values.push(8);

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
tests/cases/standard-library/ok/algo/std-algo-final-sort.ari
tests/cases/standard-library/ok/vec/std-vec-growth-paths.ari
tests/cases/standard-library/ok/vec/std-vec-convenience-api.ari
tests/cases/standard-library/ok/vec/std-vec-complete-convenience-api.ari
tests/cases/standard-library/ok/vec/std-vec-range-mutation.ari
tests/cases/standard-library/ok/vec/std-vec-iter.ari
tests/cases/standard-library/ok/iter/std-iter-slice-vec.ari
```

`make check-prelude` compiles these to LLVM, checks representative symbols,
and runs executable checks where behavior is observable. Public methods are
tracked in `tests/std_api_manifest.txt` and checked by `make check-std-api`.

## Future Work

Potential next slices:

- broader iterator adapters after general iterator protocol diagnostics mature
- richer borrow-returning slice APIs after generic lifetime rules are clearer
- move-aware range-drain APIs that can transfer resource-owning elements
  without relying on today's copy-oriented place operations
- maps, sets, and deques after generic aggregate monomorphization is stronger
- eventual unification plan for bare local `Vec[T]` and source `std::Vec[T]`
