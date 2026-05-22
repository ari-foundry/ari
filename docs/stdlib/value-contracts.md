# Standard Library Value Movement Contracts

This page records the current value movement contract for standard-library
sequence helpers. It exists so library authors can decide whether an operation
is a plain copy, a move, a drop, or future work before putting resource-owning
types such as files, sockets, or boxed values inside collections.

Ari's current source algorithms mostly operate through `ptr_load` and
`ptr_store`. That means the production-safe contract today is intentionally
narrow:

- Element-moving sequence helpers are for copyable scalar values and plain
  Ari-layout aggregates that do not contain `own`, `ref`, or `ref mut` fields.
- Ownership- or borrow-valued elements are rejected by the checker before raw
  pointer copying can duplicate or lose an owner.
- A type implementing `Drop` is only enough to describe destruction. It does
  not make a type copyable, cloneable, sortable, or safely movable through raw
  place operations.
- Ari has no unwinding runtime today. `assert`/`panic` paths abort rather than
  unwind through collection cleanup code, so "panic safety" means ordinary early
  stop and explicit `drop`, not exception-style unwinding.

## Current Contract Matrix

| Operation | Current Contract | Overlap | Drop Policy |
| --- | --- | --- | --- |
| `algo::copy(target, source)` / `slice.copy_from(source)` / `vec.copy_from(source)` | Copies `min(target.len, source.len)` elements forward with raw element loads and stores. Requires copyable/plain element materialization. | Not a typed `memmove`; overlapping views are not guaranteed except for identical no-op-style cases the implementation happens to tolerate. | Does not drop overwritten target values separately. Use it for initialized copyable/plain buffers, not owners. |
| `algo::copy_within(values, start, end, target)` / `slice.copy_within(...)` / `vec.copy_within(...)` | Copies a half-open range inside the same storage and chooses forward or backward direction for overlap. Requires copyable/plain `T` today. | Overlap-safe within one slice/vector for copyable/plain values. | Does not drop overwritten target values separately. It is not a move-only resource transfer. |
| `Slice.copy_to(zone)`, `Vec.copy_to(zone)`, `vec.extend(values)`, `vec.extend_from_slice(values)`, `vec.insert_many(index, values)`, `vec.splice(..., replacement)` | Copies borrowed source elements into target-zone or target-vector storage. Requires copyable/plain `T` today. | Source and destination should be treated as distinct unless the specific API states otherwise. | Removed `splice` range elements are dropped before replacement; copied source elements are not consumed. |
| `vec.extend_iter(iter)` | Consumes an iterator and pushes each yielded value into the vector. Current production-safe use still depends on the iterator yielding copyable/plain values or true owned values that were constructed for this transfer. | Target vector grows in place or into a new zone allocation. | Already-yielded values become vector elements; no source container drop contract is implied by the iterator protocol alone. |
| `HashMapEntry.insert_entry(value)` / `TreeMapEntry.insert_entry(value)` / `or_default()` | Entry helpers ensure a map slot exists and return either the entry handle or `ref mut V`. `or_default` requires `V: Default` and constructs a default only for missing keys. | Not relevant. | `insert_entry` drops the replaced value when the key already exists because it returns the entry handle rather than the old value. Use `insert(value)` when the old value must be preserved. |
| `iter::empty()`, `iter::once(value)` | `empty` yields no values. `once` yields its stored value once and then stops. The current `OnceIter` implementation stores the value in a field, so resource-owned field-take semantics remain tied to future move-aware place work. | Not relevant. | `once` transfers one yielded value to the consumer; `empty` transfers none. |
| `iter::count`, `count_if`, `nth`, `last`, `find_if`, `position`, `any`, `all`, `fold`, `reduce`, `collect` | Eagerly advance an iterator until the answer is known or the source is exhausted. Predicate forms borrow each yielded value for inspection. | Not relevant. | Values pulled from the source are consumed by the iterator protocol. `nth`, `last`, `find_if`, `reduce`, and `collect` may return or store yielded values; skipped or non-matching yielded values follow the current copy/plain-value contract until move-aware place transfers are complete. |
| `iter::repeat_with(make_value)` | Calls `make_value()` for each yielded item. The iterator is intentionally infinite, so callers should bound it with `take`, `zip`, or another terminating consumer before `collect` or `extend_iter`. | Not relevant. | Each generated value is yielded once and then follows the consumer's ownership contract. |
| `algo::fill(values, value)` / `slice.fill(value)` / `vec.fill(value)` / `fill_range` wrappers | Stores the same `value` into every selected slot. This is not a clone factory. It is valid for copyable/plain values. | Not relevant. | Overwritten owners are not a supported use case. Future APIs should separate `fill_clone` or `fill_with`. |
| `vec.resize(length, value)` / `resize_in` | Shrinking drops removed live elements. Growing repeatedly pushes `value`, so growing is only production-safe for copyable/plain values today. | Not relevant. | Shrink paths drop removed values before reducing logical length. Grow paths do not call a clone constructor. |
| `vec.resize_with(length, make_value)` | Shrinking shares the normal resize/truncate drop path. Growing calls `make_value()` once per new slot and pushes each returned value through the vector's stored zone metadata. | Not relevant. | Each generated value becomes one live vector element. Final placement still uses today's raw storage model, so broader move-only resource policy remains future work. |
| `algo::swap`, `reverse`, `reverse_range`, `rotate_left`, `rotate_right`, `rotate_range`, `sort`, `sort_by`, `stable_sort`, `stable_sort_by`, `stable_sort_in`, `stable_sort_by_in`, `try_stable_sort`, `try_stable_sort_by`, `partition`, `stable_partition` | Reorders in place using raw element swaps, copies, or temporary buffers. Requires copyable/plain `T` today. `sort`/`sort_by` are unstable introsort paths with no heap allocation. Stable sort paths use merge-sort temporary storage of `len` elements. `*_by` comparators currently receive values by copy, so they are also copy-oriented. | Reordering is in-place within one slice. Stable sort's temporary buffer must not overlap the sorted slice. | Does not create or destroy logical elements. Stable sort drops the temporary zone after copied-back values are in place. Future move-aware versions must use explicit take/store place operations. |
| `algo::dedup`, `dedup_by`, `dedup_by_key` | Compacts consecutive unique values into the front of the slice and returns the logical prefix length. Requires copyable/plain `T` today. | In-place within one slice. | Slice helpers do not drop the suffix because a `Slice[T]` does not own it. The caller must use only the returned prefix or let the owning container clean up. |
| `Vec::dedup`, `dedup_by`, `dedup_by_key` | Runs the same consecutive compaction and then truncates the vector to the returned prefix. | In-place within one vector. | Drops values in the removed suffix through normal `Drop` lowering before reducing `len`. |
| `vec.retain(keep)`, `vec.remove_range(start, end)`, `vec.clear()`, `vec.truncate(length)` | Mutates an owning vector by removing selected live elements. | In-place within one vector. | Drops removed values before the vector stops considering those slots live. |
| `vec.swap_remove(index)` | Removes one element without preserving order by moving the last live element into the removed slot. Current implementation still depends on raw place materialization, so use it with copyable/plain values until move-aware place contracts land. | In-place within one vector. | Returns the removed value to the caller. The old last slot stops being live when `len` is reduced. |
| `vec.append(ref mut other)` | Moves live values from `other` into `self` and sets `other.len()` to zero. Current implementation still depends on raw place materialization, so use it with copyable/plain values until move-aware place contracts land. | Source and target must be distinct vectors. | Source vector no longer drops appended elements because its length becomes zero. |
| `vec.split_off(index)` | Moves the tail `[index, len)` into a new vector backed by the same zone metadata. Current implementation still depends on raw place materialization, so use it with copyable/plain values until move-aware place contracts land. | Source and returned vector are distinct handles over distinct backing allocations. | The source length is reduced before the returned vector owns the tail. The returned vector drops any live tail elements it still owns. |
| `vec.drain()` / `vec.drain_range(start, end)` | Immediately removes the selected live range and returns a cursor over removed values. `drain()` selects the whole vector; `drain_range` shifts the retained tail left. | Cursor owns the removed logical range. | Consumed items are returned by value. Unconsumed items are dropped when the drain cursor is dropped, so early stop is covered in normal control flow. No unwind guarantee exists yet. |
| `min`, `max`, binary search and bound helpers | Read elements by value for comparisons and returned `Option[T]` results where applicable. | Read-only view. | No element destruction. Current comparator forms are copy-oriented because `less` takes `T, T`. |

## Rules For New Library APIs

When adding sequence or collection APIs, choose one of these contracts and say
so in the module docs:

1. **Copy contract**: the API duplicates `T` by value. It must require
   copyable/plain `T` today and should eventually use an explicit `Copy[T]`
   bound once trait bounds participate in these helpers.
2. **Clone contract**: the API may create multiple independent values from one
   source value. Do not fake this with repeated raw stores. Add a `Clone[T]`
   bound or a generator callback first.
3. **Move contract**: the API transfers each element exactly once from an old
   live slot to a new live slot. It must clear or mark the old slot as no longer
   live before it can be dropped.
4. **Borrow contract**: the API observes elements through `ref T` or
   `ref mut T` and does not materialize new values. Predicate APIs should prefer
   this shape when they do not need to own values.
5. **Drop contract**: the API removes ownership of a live element and must run
   normal `Drop` lowering exactly once for that value.

Avoid adding a helper whose name hides the contract. Current
`resize_with(length, make_value)` is deliberately separate from
`resize(length, value)` because it creates a fresh value per new slot. Future
resource-transfer APIs should use similarly explicit names, such as
`extend_move(source)`, rather than making today's copy-oriented helpers silently
consume values many times.

## Future Move-Aware Work

Production support for non-copy resource types inside `Vec`, maps, sets, and
algorithms needs compiler and library work together:

- first-class place operations for "take from slot", "write uninitialized slot",
  and "mark old slot moved" without going through copy-oriented `ptr_load`
  semantics;
- trait bounds that can distinguish `Copy`, `Clone`, `Move`, `Drop`, `Ord`, and
  `Eq` requirements in public API diagnostics;
- by-reference comparators such as `sort_by_ref(fn(ref T, ref T) -> bool)` so
  sorting does not require copying elements just to compare them;
- a fallible zone allocation primitive so `try_stable_sort*` can report actual
  allocation failure instead of only preflight layout errors;
- move-aware vector growth and shrink internals that preserve exactly-once drop
  behavior while moving storage;
- a documented panic/unwind story before promising RAII cleanup across panics.

Until then, treat the current sequence APIs as production-grade for scalar and
plain copyable data, and as deliberately rejected or future work for resource
owners.
