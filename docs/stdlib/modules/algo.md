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
algo::binary_search_by<T>(values, target, less)
algo::lower_bound<T>(values, target)
algo::lower_bound_by<T>(values, target, less)
algo::upper_bound<T>(values, target)
algo::upper_bound_by<T>(values, target, less)
algo::equal_range<T>(values, target)
algo::equal_range_by<T>(values, target, less)
algo::partition_point<T>(values, predicate)
algo::is_sorted<T>(values)
algo::is_sorted_by<T>(values, less)
algo::reverse<T>(values)
algo::rotate_left<T>(values, count)
algo::rotate_right<T>(values, count)
algo::partition<T>(values, keep)
algo::stable_partition<T>(values, keep)
algo::min<T>(values)
algo::min_by<T>(values, less)
algo::max<T>(values)
algo::max_by<T>(values, less)
algo::clamp<T>(value, low, high)
algo::clamp_by<T>(value, low, high, less)
algo::swap<T>(values, left, right)
algo::fill<T>(values, value)
algo::copy<T>(target, source)
algo::dedup<T>(values)
algo::dedup_by<T>(values, same)
algo::dedup_by_key<T, K>(values, key)
```

`sort`, `stable_sort`, `binary_search`, `lower_bound`, `upper_bound`,
`equal_range`, `is_sorted`, `min`, `max`, and `clamp` use `cmp::Ord[T]`. Their source code
uses ordinary `<`, `>`, and `<=` operators; for generic element types those
operators dispatch to `Ord[T]::lt`. Define `impl cmp::Ord[YourType] for
YourType` before using those helpers with a custom type. The `*_by` variants
take an explicit comparator `fn(T, T) -> bool`, which is useful when a call
site needs a temporary ordering policy rather than a type-wide `Ord` impl.

`binary_search` returns `Option[i64]`: `Some(index)` when it finds an equal
value under `Ord`, and `None` otherwise. `lower_bound` returns the first sorted
insertion index whose value is not less than the target. `upper_bound` returns
the first index whose value is greater than the target. `equal_range` returns
the `(lower, upper)` pair for all values equal to the target, and
`partition_point` returns the first index where a borrowed predicate becomes
false. Call these search helpers only on sorted or predicate-partitioned
slices.

`partition(values, keep)` reorders the slice so values accepted by
`keep: fn(ref T) -> bool` appear before rejected values. It returns the split
index. The partition is not stable. `stable_partition(values, keep)` uses the
same predicate shape but preserves the relative order of accepted values and
the relative order of rejected values.

`copy(target, source)` copies the prefix that fits and returns the number of
elements copied. This shape keeps it ergonomic for buffers where the target may
be shorter than the source. Use exact length checks in your own code when a
short copy should be an error.

`dedup(values)` compacts consecutive duplicates into the front of the same
slice and returns the logical unique length. It does not resize the original
array or vector; use the returned length as the prefix boundary.
`dedup_by(values, same)` treats adjacent values as duplicates when
`same(ref previous, ref current)` returns `true`. `dedup_by_key(values, key)`
compares adjacent keys returned by `key(ref value)`, which is the practical
shape for records such as `users.dedup_by_key<i64>(user_id)`.

## Feature Status

| Need | Status |
| --- | --- |
| sort | Current: `sort(values)` and `sort_by(values, less)` over slices. |
| stable sort | Current: `stable_sort(values)` and `stable_sort_by(values, less)`. |
| binary search | Current: `binary_search(values, target) -> Option[i64]`, `lower_bound(values, target) -> i64`, `upper_bound(values, target) -> i64`, `equal_range(values, target) -> (i64, i64)`, and comparator `*_by` variants. |
| partition point | Current: `partition_point(values, predicate) -> i64` for predicate-partitioned slices. |
| reverse | Current: `reverse(values)`. |
| rotate | Current: `rotate_left(values, count)` and `rotate_right(values, count)`. |
| partition | Current: `partition(values, keep)` and stable `stable_partition(values, keep)` with borrowed predicates. |
| min/max | Current: `min(values)` and `max(values)` return `Option[T]`; `min_by` and `max_by` take explicit comparators. |
| clamp | Current: `clamp(value, low, high)` delegates to `std::cmp`; `clamp_by` delegates to comparator-based `std::cmp`. |
| swap | Current: `swap(values, left, right)`. |
| fill | Current: `fill(values, value)`. |
| copy | Current: `copy(target, source) -> copied_count`. |
| dedup | Current: `dedup`, `dedup_by`, and `dedup_by_key` consecutive in-place compaction returning logical length. |
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
let equal = view.equal_range(1);
```

Partition and compact:

```ari
fn even(value: ref i64) -> bool {
  let raw = (ref value) as ptr i64;
  return (*raw % 2) == 0;
}

fn below_four(value: ref i64) -> bool {
  let raw = (ref value) as ptr i64;
  return (*raw) < 4;
}

fn same_parity(left: ref i64, right: ref i64) -> bool {
  let left_raw = (ref left) as ptr i64;
  let right_raw = (ref right) as ptr i64;
  return (*left_raw % 2) == (*right_raw % 2);
}

var values = [1, 2, 3, 4, 5, 6];
let split = algo::partition<i64>(values.as_slice(), even);
let stable_split = algo::stable_partition<i64>(values.as_slice(), even);
let point = algo::partition_point<i64>(values.as_slice(), below_four);

var repeated = [1, 1, 2, 2, 3];
let unique_len = algo::dedup<i64>(repeated.as_slice());
let parity_len = algo::dedup_by<i64>(repeated.as_slice(), same_parity);
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
tests/cases/standard-library/ok/algo/std-algo-dedup-partition.ari
tests/cases/standard-library/ok/algo/std-algo-by-helpers.ari
tests/cases/standard-library/ok/vec/prelude-slice-sequence.ari
```

The focused test covers sorting, stable sorting, comparator-based sorting,
binary search, lower/upper/equal-range bounds, partition point, min/max/clamp,
reverse, rotation, partition, fill, copy, dedup, and swap over `Slice[i64]`.
`std-algo-dedup-partition.ari` covers `dedup_by`, `dedup_by_key`,
`stable_partition`, and the natural `Slice`/`Vec` receiver wrappers.
`std-algo-by-helpers.ari` covers comparator search, bounds, equal range,
partition point, min/max/clamp, and natural `Slice`/`Vec` receiver wrappers
over a custom value with no `Ord` impl.
`prelude-slice-sequence.ari` covers the ordered receiver wrappers over the same
algorithms.

## Next Work

- Add faster `sort`/`stable_sort` implementations after iterator and
  move-aware temporary storage policy are stronger.
- Grow `std::hash` and `std::collections` with trait-driven collection
  constructors once `Hash`/`Eq` and `Ord` dispatch policy is stronger. Hash
  containers should derive default policy from `Hash + Eq`; tree and heap
  containers should derive default policy from `Ord`.
- Grow `std::encoding` with URL-safe base64 or fallible owned decoders after
  zone-backed enum payloads and richer error values are supported.
- Keep compression optional unless Ari decides that a specific codec belongs
  in the core distribution.
