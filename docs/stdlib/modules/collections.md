# std::collections

`std::collections` contains source Ari collection handles that allocate through
an explicit `Zone`. The module now has several distinct families:

| Type | Data Structure | Use It For |
| --- | --- | --- |
| `Set[T]` | linear insertion-order buffer | tiny unique lists, stable insertion-order access, iteration |
| `Deque[T]` | growable circular buffer | push/pop at both ends, queue-like worklists, recent-front access |
| `RingBuffer[T]` | fixed-capacity circular buffer | bounded FIFO buffers, streaming windows, overwrite-oldest queues |
| `LinkedList[T]` | zone-backed doubly linked index nodes | stable front/back node links and cheap end insertion/removal |
| `BinaryHeap[T]` | comparator-driven binary heap | max-priority scheduling, repeated highest-priority pop |
| `PriorityQueue[T]` | binary-heap priority queue surface | user-facing priority queue naming over the same heap policy |
| `HashMap[K, V]`, `HashSet[T]` | open-addressed hash table with tombstones | fast average-case lookup when a hash function is available |
| `TreeMap[K, V]`, `TreeSet[T]` | red-black tree | ordered lookup when a strict less-than comparator is available |

These names are intentionally plain. Method names stay natural: `insert`,
`contains`, `get`, `try_get`, `remove`, `take`, `replace`, `reserve`, and
`clear`. Do not add type suffixes such as `insert_i64`; generic type arguments
and the container type carry that information.

Public source signatures still spell return types even when generic arguments
exist. That keeps docs, diagnostics, and API manifest checks stable.

## Zone Rule

Every collection handle is tied to the zone used by its constructor. Ari tracks
that provenance and rejects:

- using a collection after its zone is reset or destroyed
- growing or inserting with a different zone than the one that created it
- storing collection backing pointers into unrelated aggregates

Growth copies live elements into newly allocated zone storage. Old raw buffers
remain owned by the zone and are reclaimed when the zone resets or destroys.

## Linear Set

```ari
collections::new<T>(ref mut zone, capacity)
Set::new<T>(ref mut zone, capacity)
collections::from_slice_in<T>(ref mut zone, values)
```

`Set[T]` keeps values in insertion order and uses linear `==` search. It is
not a hash set; keep using it when order and small size matter more than hash
lookup.

Important methods:

```ari
set.len()
set.capacity()
set.is_empty()
set.first()
set.try_first()
set.last()
set.try_last()
set.get(index)
set.try_get(index)
set.index_of(value)
set.contains(value)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.remove(value)
set.take(value)
set.pop()
set.try_pop()
set.clear()
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
set.as_slice()
set.iter()
set.copy_to(ref mut target)
```

`insert` returns `true` only for a newly inserted value. `replace` returns
`Some(previous)` when an equal value already existed, otherwise it inserts and
returns `None`. `take` moves the removed value out, while `remove` drops it.
`iter` yields insertion-order values and `Set[T]` implements
`IntoIterator[T]`.

## Deque

`Deque[T]` is a growable double-ended queue. It stores values in a circular
buffer and grows by copying live values into a larger zone allocation in
logical front-to-back order.

```ari
collections::deque<T>(ref mut zone, capacity)
Deque::new<T>(ref mut zone, capacity)
```

Important methods:

```ari
deque.len()
deque.capacity()
deque.is_empty()
deque.front()
deque.try_front()
deque.back()
deque.try_back()
deque.get(index)
deque.try_get(index)
deque.push_front(ref mut zone, value)
deque.push_back(ref mut zone, value)
deque.pop_front()
deque.try_pop_front()
deque.pop_back()
deque.try_pop_back()
deque.clear()
deque.reserve(ref mut zone, capacity)
deque.reserve_extra(ref mut zone, additional)
deque.iter()
```

Use `Deque[T]` when both ends matter. `push_front` and `push_back` take the
same zone used by the constructor because growth may allocate. `iter` and
`for value in deque` yield logical front-to-back order, not physical storage
order.

## RingBuffer

`RingBuffer[T]` is fixed-capacity. It is useful when an application must bound
memory and either reject new values while full or intentionally overwrite the
oldest value.

```ari
collections::ring_buffer<T>(ref mut zone, capacity)
RingBuffer::new<T>(ref mut zone, capacity)
```

Important methods:

```ari
buffer.len()
buffer.capacity()
buffer.is_empty()
buffer.is_full()
buffer.push(value)
buffer.push_overwrite(value)
buffer.peek()
buffer.try_peek()
buffer.get(index)
buffer.try_get(index)
buffer.pop()
buffer.try_pop()
buffer.clear()
buffer.iter()
```

`push` returns `false` when the buffer is full. `push_overwrite` always keeps
the new value; it returns `Some(oldest)` when a full buffer overwrote the
oldest slot and `None` when it appended without overwriting. `peek` and `pop`
operate on the oldest value.

## LinkedList

`LinkedList[T]` uses doubly linked node indices over zone-backed arrays. It
does not allocate one host heap node at a time; Ari keeps allocation explicit
and zone-oriented. Removed node slots are put on a small free list and reused
by later pushes.

```ari
collections::linked_list<T>(ref mut zone, capacity)
LinkedList::new<T>(ref mut zone, capacity)
```

Important methods:

```ari
list.len()
list.capacity()
list.is_empty()
list.front()
list.try_front()
list.back()
list.try_back()
list.get(index)
list.try_get(index)
list.push_front(ref mut zone, value)
list.push_back(ref mut zone, value)
list.pop_front()
list.try_pop_front()
list.pop_back()
list.try_pop_back()
list.remove_at(index)
list.try_remove_at(index)
list.clear()
list.reserve(ref mut zone, capacity)
list.reserve_extra(ref mut zone, additional)
list.iter()
```

`get` and `remove_at` traverse from the head, so they are O(n). Prefer
front/back operations for queue-like code. `iter` yields front-to-back values.

## HashMap And HashSet

Hash collections are real hash tables, not aliases over `Set`. They use linear
probing, tombstones for removal, and a load-factor growth rule. Until Ari has a
standard `Hash` trait with dispatch through generic containers, constructors
take a hash function explicitly.

```ari
collections::hash_i64(value)
collections::hash_map<K, V>(ref mut zone, capacity, hash)
HashMap::new<K, V>(ref mut zone, capacity, hash)
collections::hash_set<T>(ref mut zone, capacity, hash)
HashSet::new<T>(ref mut zone, capacity, hash)
```

The hash function shape is `fn(K) -> u64` for `HashMap[K, V]` and
`fn(T) -> u64` for `HashSet[T]`. `collections::hash_i64` is the first built-in
helper for i64 keys.

```ari
map.len()
map.capacity()
map.is_empty()
map.contains(key)
map.get(key)
map.try_get(key)
map.insert(ref mut zone, key, value)
map.remove(key)
map.clear()
map.reserve(ref mut zone, capacity)
map.keys()
map.values()
```

`HashMap.insert` inserts or replaces and returns `Option[V]`: `Some(previous)`
on replacement, `None` on a new key. `remove` returns the removed value.
`keys` and `values` iterate live buckets. That order is deterministic for a
specific table state, but it is not insertion order and should not be used as
a stable sorting rule.

```ari
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.take(value)
set.remove(value)
set.clear()
set.reserve(ref mut zone, capacity)
set.iter()
```

`HashSet.insert` returns whether the value was newly inserted. `replace`
returns the previous equal value when present. `take` moves a removed value out;
`remove` drops it. `HashSet.iter()` yields live buckets, and `HashSet[T]`
implements `IntoIterator[T]` so `for value in set` works through the same
cursor.

## TreeMap And TreeSet

Tree collections are red-black trees. They provide ordered lookup without
needing a hash function. Until trait dispatch is strong enough to use
`std::cmp::Ord` directly in generic container internals, constructors take a
strict less-than comparator.

```ari
collections::less_i64(left, right)
collections::tree_map<K, V>(ref mut zone, capacity, less)
TreeMap::new<K, V>(ref mut zone, capacity, less)
collections::tree_set<T>(ref mut zone, capacity, less)
TreeSet::new<T>(ref mut zone, capacity, less)
```

The comparator shape is `fn(K, K) -> bool` for `TreeMap[K, V]` and
`fn(T, T) -> bool` for `TreeSet[T]`. `collections::less_i64` is the first
built-in helper for i64 keys.

```ari
map.len()
map.capacity()
map.is_empty()
map.contains(key)
map.get(key)
map.try_get(key)
map.insert(ref mut zone, key, value)
map.clear()
map.reserve(ref mut zone, capacity)
map.keys()
map.values()
```

`TreeMap.insert` inserts or replaces and returns `Option[V]`. `keys` yields
keys in ascending comparator order. `values` yields values in the same
key-sorted order.

```ari
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.clear()
set.reserve(ref mut zone, capacity)
set.iter()
```

`TreeSet.insert` returns `false` for an equal existing value. `TreeSet.replace`
returns the previous equal value or inserts a new one. `TreeSet.iter()` yields
values in ascending comparator order, and `TreeSet[T]` implements
`IntoIterator[T]`.

## BinaryHeap And PriorityQueue

`BinaryHeap[T]` and `PriorityQueue[T]` are comparator-driven max-priority
containers. The `less(a, b)` function means `a` has lower priority than `b`.
With `collections::less_i64`, larger integers pop first.

```ari
collections::binary_heap<T>(ref mut zone, capacity, less)
BinaryHeap::new<T>(ref mut zone, capacity, less)
collections::priority_queue<T>(ref mut zone, capacity, less)
PriorityQueue::new<T>(ref mut zone, capacity, less)
```

Important methods:

```ari
heap.len()
heap.capacity()
heap.is_empty()
heap.peek()
heap.try_peek()
heap.push(ref mut zone, value)
heap.pop()
heap.try_pop()
heap.clear()
heap.reserve(ref mut zone, capacity)
heap.reserve_extra(ref mut zone, additional)
```

The priority queue has the same method surface. `peek` reads the current
highest-priority value, while `pop` removes it and restores the heap invariant.
Use `BinaryHeap[T]` when the data-structure name matters; use
`PriorityQueue[T]` when the abstraction is an application queue.

## Examples

Hash table:

```ari
fn main() -> i64 {
  var zone = zone::create(2048);
  var map = HashMap::new<i64, i64>(ref mut zone, 8, collections::hash_i64);

  map.insert(ref mut zone, 7, 70);
  map.insert(ref mut zone, 11, 110);
  let value = map.try_get(7).unwrap_or(0);

  zone::destroy(zone);
  return value;
}
```

Ordered tree:

```ari
fn main() -> i64 {
  var zone = zone::create(2048);
  var set = TreeSet::new<i64>(ref mut zone, 8, collections::less_i64);

  set.insert(ref mut zone, 4);
  set.insert(ref mut zone, 2);
  set.insert(ref mut zone, 8);
  let found = set.contains(2);

  zone::destroy(zone);
  return if found { 1 } else { 0 };
}
```

## Tests

Focused positive coverage:

```text
tests/cases/standard-library/ok/collections/std-collections-set.ari
tests/cases/standard-library/ok/collections/std-collections-set-access.ari
tests/cases/standard-library/ok/collections/std-collections-set-replace.ari
tests/cases/standard-library/ok/collections/std-collections-set-iter.ari
tests/cases/standard-library/ok/collections/std-collections-hash.ari
tests/cases/standard-library/ok/collections/std-collections-hash-iter.ari
tests/cases/standard-library/ok/collections/std-collections-tree.ari
tests/cases/standard-library/ok/collections/std-collections-tree-iter.ari
tests/cases/standard-library/ok/collections/deque/std-collections-deque.ari
tests/cases/standard-library/ok/collections/ring-buffer/std-collections-ring-buffer.ari
tests/cases/standard-library/ok/collections/linked-list/std-collections-linked-list.ari
tests/cases/standard-library/ok/collections/heap/std-collections-heap.ari
```

Focused negative coverage:

```text
tests/cases/standard-library/errors/collections/std-collections-set-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-set-iter-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-set-insert-different-zone.ari
tests/cases/standard-library/errors/collections/std-collections-set-replace-different-zone.ari
tests/cases/standard-library/errors/collections/std-collections-set-reserve-different-zone.ari
tests/cases/standard-library/errors/collections/std-collections-set-reserve-extra-different-zone.ari
tests/cases/standard-library/errors/collections/std-collections-hash-map-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-hash-map-keys-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-hash-map-values-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-hash-set-iter-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-hash-map-insert-different-zone.ari
tests/cases/standard-library/errors/collections/std-collections-tree-map-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-tree-map-keys-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-tree-map-values-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-tree-set-iter-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-tree-set-insert-different-zone.ari
tests/cases/standard-library/errors/collections/std-collections-deque-iter-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-ring-buffer-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-linked-list-iter-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-deque-push-different-zone.ari
tests/cases/standard-library/errors/collections/std-collections-linked-list-push-different-zone.ari
tests/cases/standard-library/errors/collections/std-collections-binary-heap-push-different-zone.ari
tests/cases/standard-library/errors/collections/std-collections-priority-queue-push-different-zone.ari
```

`std-collections-hash.ari` forces collisions with a custom hash function so the
linear-probing and tombstone paths are exercised. `std-collections-hash-iter`
checks key, value, and set cursors after tombstones. `std-collections-tree.ari`
inserts mixed key order to exercise red-black rotations, while
`std-collections-tree-iter.ari` checks sorted successor traversal.
`std-collections-deque.ari` checks circular growth and both-end operations.
`std-collections-ring-buffer.ari` checks bounded push failure, overwrite, and
FIFO iteration. `std-collections-linked-list.ari` checks front/back operations,
indexed removal, and node-slot reuse. `std-collections-heap.ari` checks
max-priority pop order for `BinaryHeap` and `PriorityQueue`. The new negative
collection tests lock down reset invalidation for deque/ring/list handles and
same-zone diagnostics for deque, linked-list, heap, and priority-queue growth
paths.

`make check-prelude` compiles the positive tests, checks representative
monomorphized symbols, runs the executables, and checks the negative zone
diagnostics. Public declarations are tracked in `tests/std_api_manifest.txt`
and checked by `make check-std-api`.

## Current Limits

- `Set[T]` is still linear and insertion-order.
- Hash and tree containers are copy-style value APIs today, matching the
  current `Set`/`Vec` source style. Use simple copyable keys and values until
  trait bounds can express copy, hash, equality, and ordering requirements.
- Hash containers require an explicit hash function. The future API should
  prefer `HashMap[K: Hash[K], V]` and `HashSet[T: Hash[T]]` once trait
  dispatch is ready.
- Tree containers require an explicit comparator. The future API should use
  `Ord` when generic trait dispatch is strong enough.
- Tree removal is not implemented in this slice. It should land with focused
  red-black deletion tests before `TreeMap.remove` or `TreeSet.take` become
  public.
- `LinkedList[T]` is zone-backed index storage rather than one allocation per
  node. Spare node storage is reused by the list and reclaimed with the zone.
- `BinaryHeap[T]` and `PriorityQueue[T]` require an explicit comparator until
  generic `Ord` dispatch is available inside containers.
