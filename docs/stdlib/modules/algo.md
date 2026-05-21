# std::algo

`std::algo` holds source-only algorithms over borrowed `Slice[T]` views. It is
for operations that do not own storage by themselves: sorting a view, rotating
or reversing it, copying between views, filling values, partitioning by a
predicate, finding min/max values, and compacting consecutive duplicates.

The API names stay short because the module path already says the domain:
write `algo::sort(values)`, `algo::binary_search(values, target)`, and
`algo::reverse(values)`, not type-suffixed names.
For everyday slice call sites, the same common policies are also exposed as
receiver methods such as `values.sort()`, `values.binary_search(target)`,
`values.copy_from(source)`, and `values.dedup()`.

## API

```ari
algo::sort<T>(values)
algo::sort_by<T>(values, less)
algo::stable_sort<T>(values)
algo::stable_sort_by<T>(values, less)
algo::binary_search<T>(values, target)
algo::lower_bound<T>(values, target)
algo::upper_bound<T>(values, target)
algo::is_sorted<T>(values)
algo::reverse<T>(values)
algo::rotate_left<T>(values, count)
algo::rotate_right<T>(values, count)
algo::partition<T>(values, keep)
algo::min<T>(values)
algo::max<T>(values)
algo::clamp<T>(value, low, high)
algo::swap<T>(values, left, right)
algo::fill<T>(values, value)
algo::copy<T>(target, source)
algo::dedup<T>(values)
```

`sort`, `stable_sort`, `binary_search`, `is_sorted`, `min`, `max`, and
`clamp` use `cmp::Ord[T]`. Their source code uses ordinary `<`, `>`, and
`<=` operators; for generic element types those operators dispatch to
`Ord[T]::lt`. Define `impl cmp::Ord[YourType] for YourType` before using those
helpers with a custom type. The `*_by` variants take an explicit comparator
`fn(T, T) -> bool`, which is useful while trait-driven constructor and
comparator inference are still young.

`binary_search` returns `Option[i64]`: `Some(index)` when it finds an equal
value under `Ord`, and `None` otherwise. `lower_bound` returns the first sorted
insertion index whose value is not less than the target. `upper_bound` returns
the first index whose value is greater than the target. Call these search
helpers only on a sorted slice.

`partition(values, keep)` reorders the slice so values accepted by
`keep: fn(ref T) -> bool` appear before rejected values. It returns the split
index. The partition is not stable.

`copy(target, source)` copies the prefix that fits and returns the number of
elements copied. This shape keeps it ergonomic for buffers where the target may
be shorter than the source. Use exact length checks in your own code when a
short copy should be an error.

`dedup(values)` compacts consecutive duplicates into the front of the same
slice and returns the logical unique length. It does not resize the original
array or vector; use the returned length as the prefix boundary.

## Feature Status

| Need | Status |
| --- | --- |
| sort | Current: `sort(values)` and `sort_by(values, less)` over slices. |
| stable sort | Current: `stable_sort(values)` and `stable_sort_by(values, less)`. |
| binary search | Current: `binary_search(values, target) -> Option[i64]`, `lower_bound(values, target) -> i64`, and `upper_bound(values, target) -> i64`. |
| reverse | Current: `reverse(values)`. |
| rotate | Current: `rotate_left(values, count)` and `rotate_right(values, count)`. |
| partition | Current: `partition(values, keep)` with borrowed predicates. |
| min/max | Current: `min(values)` and `max(values)` return `Option[T]`. |
| clamp | Current: `clamp(value, low, high)` delegates to `std::cmp`. |
| swap | Current: `swap(values, left, right)`. |
| fill | Current: `fill(values, value)`. |
| copy | Current: `copy(target, source) -> copied_count`. |
| dedup | Current: consecutive in-place compaction returning logical length. |
| hashing | Current: `std::hash` has `Hasher`, `Hash[T]`, primitive hashing, byte-slice hashing, and `collections::hash_i64` compatibility. |
| base64 | Current: `std::encoding` has standard base64 length, encode, decode, and validation guards. |
| hex encoding | Current: `std::encoding` has lowercase hex length, encode, decode, and validation guards; `std::ascii` still owns scalar hex digit parsing. |
| compression | Optional roadmap: likely package/extension boundary before becoming core std. |

## Examples

Sort and search:

```ari
var values = [5, 1, 4, 1, 3];
algo::sort<i64>(values.as_slice());
let found = algo::binary_search<i64>(values.as_slice(), 4);

let view = values.as_slice();
view.sort();
let again = view.binary_search(4);
let insert_at = view.lower_bound(2);
```

Partition and compact:

```ari
fn even(value: ref i64) -> bool {
  let raw = (ref value) as ptr i64;
  return (*raw % 2) == 0;
}

var values = [1, 2, 3, 4, 5, 6];
let split = algo::partition<i64>(values.as_slice(), even);

var repeated = [1, 1, 2, 2, 3];
let unique_len = algo::dedup<i64>(repeated.as_slice());
```

Use borrowed sub-slices for buffer work:

```ari
var target = [0, 0, 0, 0];
var source = [8, 7, 6];
let copied = algo::copy<i64>(target.as_slice(), source.as_slice());
algo::fill<i64>(target.as_slice()[copied..target.len()], -1);
```

## Current Limits

- The current implementations are source-level and optimized for clarity.
  `sort` is selection-sort based, while `stable_sort` is adjacent-swap
  insertion sort. Faster large-slice algorithms are future work.
- The helpers use the current plain generic value model. They are intended for
  scalar/plain copyable values today; ownership-carrying element policy should
  become stricter once Ari has a first-class `Copy`/move-aware algorithm
  contract.
- `copy` is a forward element copy, not a guaranteed memmove for overlapping
  views. Use `std::mem::move_bytes` for raw byte overlap until typed overlap
  policy is documented.
- `dedup` only compacts consecutive duplicates. Sort first if you want to
  remove all equal values regardless of original position.

## Tests

```text
tests/cases/standard-library/ok/algo/std-algo-slice-helpers.ari
tests/cases/standard-library/ok/vec/prelude-slice-sequence.ari
```

The focused test covers sorting, stable sorting, comparator-based sorting,
binary search, lower/upper bounds, min/max/clamp, reverse, rotation, partition,
fill, copy, dedup, and swap over `Slice[i64]`. `prelude-slice-sequence.ari`
covers the natural receiver wrappers over the same algorithms.

## Next Work

- Add faster `sort`/`stable_sort` implementations after iterator and
  move-aware temporary storage policy are stronger.
- Grow `std::hash` with trait-driven collection constructors once `Hash`/`Eq`
  dispatch policy is stronger.
- Grow `std::encoding` with URL-safe base64 or fallible owned decoders after
  zone-backed enum payloads and richer error values are supported.
- Keep compression optional unless Ari decides that a specific codec belongs
  in the core distribution.
