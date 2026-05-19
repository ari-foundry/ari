# std::iter

`std::iter` is the shared loop protocol layer. It currently keeps the surface
small: range constructors for compiler-lowered numeric loops and traits used
by source collection cursors.

## Public API

```ari
iter::range<T>(start, end)
iter::range_inclusive<T>(start, end)

trait Iterator[T] {
  fn next(self: ref mut Self) -> Option[T];
}

trait IntoIterator[T] {
  fn into_iter(self: ref mut Self) -> Self;
}

trait Iterable[T]
```

Root aliases expose `range(start, end)` and `range_inclusive(start, end)`.
Range syntax such as `start..end` and `start..=end` lowers through the same
range model.

## How To Use It

For collection cursors, implement `Iterator[T]::next` and return `Some(value)`
until the cursor is exhausted, then return `None<T>()`.

```ari
var cursor = values.iter();
for value in cursor {
  // value is the T yielded by Iterator[T].
}
```

Collections that implement `IntoIterator[T]` can be used directly in `for`
loops. Today `std::collections::Set[T]`, `Deque[T]`, `RingBuffer[T]`,
`LinkedList[T]`, `HashSet[T]`, and `TreeSet[T]` expose that path. `HashMap`
and `TreeMap` intentionally expose `keys()` and `values()` instead of a pair
iterator until tuple or pair conventions are stable. `BinaryHeap[T]` and
`PriorityQueue[T]` expose priority removal through `pop()` rather than an
iterator so callers do not mistake heap storage order for priority order.

## Current Limits

- `Iterable[T]` is a marker surface for future adapter design.
- General iterator adapters such as `map`, `filter`, and `fold` are roadmap
  work.
- `IntoIterator[T]` is intentionally minimal while the compiler's general
  iterator lowering is still growing.

## Tests

Representative coverage lives in:

```text
tests/cases/standard-library/ok/vec/std-vec-iter.ari
tests/cases/standard-library/ok/collections/std-collections-set-iter.ari
tests/cases/standard-library/ok/collections/std-collections-hash-iter.ari
tests/cases/standard-library/ok/collections/std-collections-tree-iter.ari
tests/cases/standard-library/ok/collections/deque/std-collections-deque.ari
tests/cases/standard-library/ok/collections/ring-buffer/std-collections-ring-buffer.ari
tests/cases/standard-library/ok/collections/linked-list/std-collections-linked-list.ari
```
