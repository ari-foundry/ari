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
vec.truncate(length)
vec.clear()
```

`try_pop` returns `Option[T]`; `pop` asserts when empty. `set`, `clear`,
`truncate`, and shrink paths drop removed live elements before reducing the
logical length.

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
vec.starts_with(values)
vec.ends_with(values)
vec.equals(values)
```

Borrowed views and raw pointers:

```ari
vec.as_slice()
vec.as_ptr()
vec.as_mut_ptr()
```

`as_slice` returns a borrowed `Slice[T]` over the live elements. The pointer
helpers preserve zone provenance in the checker.

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
view.first()
view.try_first()
view.last()
view.try_last()
view.get(index)
view.try_get(index)
view.index_of(value)
view.contains(value)
view.count(value)
view.starts_with(other)
view.ends_with(other)
view.equals(other)
view.copy_to(ref mut zone)
```

Use `Slice[T]` when you only need a borrowed view. Use `try_*` accessors when
absence is a normal branch and `first`/`last`/`get` when absence should trap.

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
tests/cases/standard-library/ok/prelude-slice-methods.ari
tests/cases/standard-library/ok/prelude-slice-option-access.ari
tests/cases/standard-library/ok/prelude-slice-copy-to.ari
tests/cases/standard-library/ok/std-vec-metadata-methods.ari
tests/cases/standard-library/ok/std-vec-fixed-ops.ari
tests/cases/standard-library/ok/std-vec-try-access.ari
tests/cases/standard-library/ok/std-vec-try-pop.ari
tests/cases/standard-library/ok/std-vec-slice-compare.ari
tests/cases/standard-library/ok/std-vec-growth-paths.ari
tests/cases/standard-library/ok/std-vec-iter.ari
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
