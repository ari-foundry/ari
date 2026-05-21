# std::iter

`std::iter` is the shared loop protocol layer. It contains range constructors,
the iterator trait surface, lazy source adapters, and eager consumers for
turning an iterator into a value.

## Public API

```ari
iter::range<T>(start, end)
iter::range_inclusive<T>(start, end)

iter::map<T, U, I: std::Iterator[T]>(iter, op)
iter::filter<T, I: std::Iterator[T]>(iter, keep)
iter::take<T, I: std::Iterator[T]>(iter, count)
iter::skip<T, I: std::Iterator[T]>(iter, count)
iter::enumerate<T, I: std::Iterator[T]>(iter)
iter::zip<T, U, I: std::Iterator[T], J: std::Iterator[U]>(left, right)
iter::fold<T, U, I: std::Iterator[T]>(iter, initial, op)
iter::reduce<T, I: std::Iterator[T]>(iter, op)
iter::collect<T, I: std::Iterator[T]>(ref mut zone, iter)
iter::DoubleEndedIterator[T]
iter::ExactSizeIterator[T]

trait Iterator[T] {
  fn next(self: ref mut Self) -> Option[T];
}

trait DoubleEndedIterator[T]: Iterator[T] {
  fn next_back(self: ref mut Self) -> Option[T];
}

trait ExactSizeIterator[T]: Iterator[T] {
  fn len(self: ref Self) -> i64;
}

trait IntoIterator[T] {
  fn into_iter(self: ref mut Self) -> Self;
}

trait Iterable[T]
```

Root aliases expose `range(start, end)` and `range_inclusive(start, end)`.
`iter::collect` is a public alias backed by `std::vec::collect`, so collection
always names the target allocation zone explicitly.

`skip` is the Ari standard name for the usual drop-count adapter because
`drop` is already a language operation.

`DoubleEndedIterator[T]` is a supertrait child of `Iterator[T]`. Use it when a
cursor can yield from both the front and the back without allocating a reversed
copy. A generic bound such as `I: DoubleEndedIterator[i64]` can call
`next_back()` from the child trait and `next()` from the parent `Iterator`
trait. Current double-ended cursors include `std::vec::Iter[T]`, slice
`chunks()` and `windows()` cursors, linear `Set[T]` cursors, `Deque[T]`
cursors, and `RingBuffer[T]` cursors.

`ExactSizeIterator[T]` is a supertrait child of `Iterator[T]`. Use it when a
cursor can report the exact number of values still available without advancing
the cursor. A generic bound such as `I: ExactSizeIterator[i64]` can call both
`len()` from the child trait and `next()` from the parent `Iterator` trait.
Current exact-size cursors include `std::vec::Iter[T]`, slice `chunks()` and
`windows()` cursors, and the linear/circular collection cursors whose remaining
length is stored directly.

## Lazy And Eager Operations

`map`, `filter`, `take`, `skip`, `enumerate`, and `zip` are lazy. Constructing
one of these adapters stores the source iterator and any callback, but does not
pull values. Work happens only when `next` is called, normally through a `for`
loop.

`fold`, `reduce`, and `collect` are eager consumers. They advance the iterator
until it is exhausted. `reduce` returns `None<T>()` for an empty iterator, while
`collect` returns a `std::vec::Vec[T]` allocated in the zone passed by the
caller.

## How To Use It

For collection cursors, implement `Iterator[T]::next` and return `Some(value)`
until the cursor is exhausted, then return `None<T>()`.

```ari
var cursor = values.iter();
for value in cursor {
  // value is the T yielded by Iterator[T].
}
```

Adapters are ordinary iterator values. Use a mutable local plus `ref mut` when
the adapter stores progress:

```ari
fn double(value: i64) -> i64 {
  return value * 2;
}

var mapped = iter::map<i64, i64, std::vec::Iter[i64]>(values.iter(), double);
var total = 0;
for value in ref mut mapped {
  total = total + value;
}
```

`filter` predicates take a borrowed value so the predicate can inspect an item
without consuming it before the adapter decides whether to yield it.

```ari
fn even(value: ref i64) -> bool {
  let raw: ptr i64 = value as ptr i64;
  return (*raw % 2) == 0;
}
```

`enumerate` yields `(index, value)` tuples and `zip` yields `(left, right)`
tuples. These are small product values where both fields are always present,
so tuple slots/destructuring stay lighter than dedicated wrapper structs.
`zip` stops when either side is exhausted.

Collections that implement `IntoIterator[T]` can be used directly in `for`
loops. Today `std::collections::Set[T]`, `Deque[T]`, `RingBuffer[T]`,
`LinkedList[T]`, `HashSet[T]`, and `TreeSet[T]` expose that path. `HashMap`
and `TreeMap` expose `keys()`, `values()`, and `entries()`; map entries are
dedicated `MapEntry[K,V]` values so callers can use `.key` and `.value` without
depending on tuple conventions. `BinaryHeap[T]` and `PriorityQueue[T]` expose
priority removal through `pop()` rather than an iterator so callers do not
mistake heap storage order for priority order.

## Current Limits

- `Iterable[T]` is a marker surface for future adapter design.
- Adapter callback values are plain function pointers. Closures and captures
  are future work.
- `collect` currently builds a `std::vec::Vec[T]`; other collection targets
  should be added as explicit functions after their ownership contracts are
  stable.
- Map entries are copied `MapEntry[K,V]` values. Borrowed mutable entry views
  remain future work because they need a stronger collection-view policy.

## Tests

Representative coverage lives in:

```text
tests/cases/standard-library/ok/iter/std-iter-adapters.ari
tests/cases/standard-library/ok/iter/std-iter-double-ended.ari
tests/cases/standard-library/ok/iter/std-iter-exact-size.ari
tests/cases/standard-library/ok/vec/std-vec-iter.ari
tests/cases/standard-library/ok/collections/std-collections-set-iter.ari
tests/cases/standard-library/ok/collections/std-collections-hash-iter.ari
tests/cases/standard-library/ok/collections/std-collections-tree-iter.ari
tests/cases/standard-library/ok/collections/deque/std-collections-deque.ari
tests/cases/standard-library/ok/collections/ring-buffer/std-collections-ring-buffer.ari
tests/cases/standard-library/ok/collections/linked-list/std-collections-linked-list.ari
```
