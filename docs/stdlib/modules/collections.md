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
Every zone-backed collection handle implements `std::zone::ZoneBacked` once it
has backing storage, so `zone::of(ref handle)` and `handle.zone()` expose the
same allocation-header metadata as the raw buffer.

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
set.equals(ref other)
set.is_subset(ref other)
set.is_superset(ref other)
set.is_disjoint(ref other)
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
`equals`, `is_subset`, `is_superset`, and `is_disjoint` compare membership,
not insertion order, and borrow the other set explicitly.
When a local `Set[T]` comes from a tracked zone allocation, the common growth
calls can omit the repeated zone argument: `set.insert(value)`,
`set.replace(value)`, `set.reserve(capacity)`, and
`set.reserve_extra(additional)` infer the set's source zone. Manually assembled
or otherwise untracked sets must keep the explicit `ref mut zone` argument.
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

Use `Deque[T]` when both ends matter. For tracked local deque handles,
`push_front(value)`, `push_back(value)`, `reserve(capacity)`, and
`reserve_extra(additional)` infer the constructor zone. Keep the explicit
`ref mut zone` form for manually assembled handles. `iter` and
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
front/back operations for queue-like code. For tracked local linked-list
handles, `push_front(value)`, `push_back(value)`, `reserve(capacity)`, and
`reserve_extra(additional)` infer the constructor zone. `iter` yields
front-to-back values.

## HashMap And HashSet

Hash collections are real hash tables, not aliases over `Set`. They use linear
probing, tombstones for removal, and a load-factor growth rule. Until Ari has a
standard `Hash` trait with dispatch through generic containers, constructors
take a hash function explicitly.
For tracked local hash handles, `map.insert(key, value)`,
`map.reserve(capacity)`, `map.reserve_extra(additional)`, `set.insert(value)`,
`set.replace(value)`, `set.reserve(capacity)`, and
`set.reserve_extra(additional)` infer the constructor zone.

```ari
collections::hash_i64(value)
collections::hash_map<K, V>(ref mut zone, capacity, hash)
HashMap::new<K, V>(ref mut zone, capacity, hash)
collections::hash_set<T>(ref mut zone, capacity, hash)
HashSet::new<T>(ref mut zone, capacity, hash)
```

The hash function shape is `fn(K) -> u64` for `HashMap[K, V]` and
`fn(T) -> u64` for `HashSet[T]`. `collections::hash_i64` is kept as a
compatibility helper for i64 keys and delegates to `std::hash::value<i64>`.

```ari
map.len()
map.capacity()
map.is_empty()
map.contains(key)
map.contains_key(key)
map.contains_value(value)
map.get(key)
map.get_or(key, fallback)
map.try_get(key)
map.insert(ref mut zone, key, value)
map.remove(key)
map.clear()
map.reserve(ref mut zone, capacity)
map.reserve_extra(ref mut zone, additional)
map.keys()
map.values()
map.entries()
```

`HashMap.insert` inserts or replaces and returns `Option[V]`: `Some(previous)`
on replacement, `None` on a new key. `contains_key` is the preferred
key-membership spelling; `contains` remains available for compatibility with
older examples. `contains_value` scans live bucket values and ignores
tombstones. `get_or(key, fallback)` is the compact spelling when a missing key
has an ordinary fallback value; use `try_get` when absence should stay visible
as `Option[V]`. `remove` returns the removed value. `keys` and `values`
iterate live buckets. That order is deterministic for a specific table state,
but it is not insertion order and should not be used as a stable sorting rule.
`entries` yields `MapEntry[K, V]` values with `.key` and `.value` fields over
the same live buckets. `reserve_extra(additional)` reserves enough hash buckets
for the requested live length without immediately violating the table's load
factor rule.

```ari
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.equals(ref other)
set.is_subset(ref other)
set.is_superset(ref other)
set.is_disjoint(ref other)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.take(value)
set.remove(value)
set.clear()
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
set.iter()
```

`HashSet.insert` returns whether the value was newly inserted. `replace`
returns the previous equal value when present. `take` moves a removed value out;
`remove` drops it. `equals`, `is_subset`, `is_superset`, and `is_disjoint`
compare membership over live buckets and ignore tombstones. `HashSet.iter()`
yields live buckets, and `HashSet[T]` implements `IntoIterator[T]` so
`for value in set` works through the same cursor.

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
map.contains_key(key)
map.contains_value(value)
map.first_key()
map.try_first_key()
map.last_key()
map.try_last_key()
map.first_value()
map.try_first_value()
map.last_value()
map.try_last_value()
map.first_entry()
map.try_first_entry()
map.last_entry()
map.try_last_entry()
map.get(key)
map.get_or(key, fallback)
map.try_get(key)
map.insert(ref mut zone, key, value)
map.remove(key)
map.clear()
map.reserve(ref mut zone, capacity)
map.keys()
map.values()
map.entries()
```

`TreeMap.insert` inserts or replaces and returns `Option[V]`. `contains_key`
is the preferred key-membership spelling; `contains` remains available for
compatibility with older examples. `contains_value` scans stored values without
using key order. `get_or(key, fallback)` returns the stored value or the
fallback without forcing every call site to unwrap `Option[V]`. `keys` yields
keys in ascending comparator order. `values` yields values in the same
key-sorted order. `first_key`,
`last_key`, `first_value`, and `last_value` assert when the tree is empty;
use the `try_*` forms for empty-safe boundary access. `first_entry` and
`last_entry` return `MapEntry[K, V]` so callers can read the boundary key and
value without doing two boundary lookups. `entries` yields `MapEntry[K, V]`
values with `.key` and `.value` fields in the same sorted key order. `remove`
returns the removed value as `Option[V]`; the current
implementation compacts the live node arrays and rebuilds tree links in place,
so removal does not allocate through a zone. For tracked local tree maps,
`map.insert(key, value)`, `map.reserve(capacity)`, and
`map.reserve_extra(additional)` infer the constructor zone.

```ari
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.first()
set.try_first()
set.last()
set.try_last()
set.equals(ref other)
set.is_subset(ref other)
set.is_superset(ref other)
set.is_disjoint(ref other)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.take(value)
set.remove(value)
set.clear()
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
set.iter()
```

`TreeSet.insert` returns `false` for an equal existing value. `TreeSet.replace`
returns the previous equal value or inserts a new one. `equals`, `is_subset`,
`is_superset`, and `is_disjoint` compare ordered-set membership, not the
internal tree shape. `first` and `last` read the smallest and largest values
in comparator order and assert when the tree is empty; use `try_first` and
`try_last` when emptiness is ordinary control flow. `take` moves a removed
value out as `Option[T]`; `remove` drops the removed value and returns whether
anything was removed. Tree-set removal also compacts live nodes and rebuilds
links in place without allocating. `TreeSet.iter()` yields values in ascending
comparator order, and `TreeSet[T]` implements `IntoIterator[T]`. For tracked
local tree sets, `set.insert(value)`, `set.replace(value)`, and
`set.reserve(capacity)`/`set.reserve_extra(additional)` infer the constructor
zone.

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
For tracked local heap and priority-queue handles, `push(value)`,
`reserve(capacity)`, and `reserve_extra(additional)` infer the constructor
zone. Use `BinaryHeap[T]` when the data-structure name matters; use
`PriorityQueue[T]` when the abstraction is an application queue.

## Examples

Hash table:

```ari
fn main() -> i64 {
  var zone = zone::create(2048);
  var map = HashMap::new<i64, i64>(ref mut zone, 8, collections::hash_i64);

  map.insert(7, 70);
  map.insert(11, 110);
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

  set.insert(4);
  set.insert(2);
  set.insert(8);
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
tests/cases/standard-library/ok/collections/std-collections-set-implicit-zone.ari
tests/cases/standard-library/ok/collections/std-collections-implicit-zone.ari
tests/cases/standard-library/ok/collections/std-collections-set-relations.ari
tests/cases/standard-library/ok/collections/std-collections-set-iter.ari
tests/cases/standard-library/ok/collections/std-collections-hash.ari
tests/cases/standard-library/ok/collections/std-collections-hash-set-relations.ari
tests/cases/standard-library/ok/collections/std-collections-hash-iter.ari
tests/cases/standard-library/ok/collections/std-collections-map-entries.ari
tests/cases/standard-library/ok/collections/std-collections-map-natural-api.ari
tests/cases/standard-library/ok/collections/std-collections-map-value-predicates.ari
tests/cases/standard-library/ok/collections/std-collections-tree.ari
tests/cases/standard-library/ok/collections/std-collections-tree-boundaries.ari
tests/cases/standard-library/ok/collections/std-collections-tree-entry-boundaries.ari
tests/cases/standard-library/ok/collections/std-collections-tree-remove.ari
tests/cases/standard-library/ok/collections/std-collections-tree-set-relations.ari
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
tests/cases/standard-library/errors/collections/std-collections-set-implicit-zone-untracked.ari
tests/cases/standard-library/errors/collections/std-collections-implicit-zone-untracked.ari
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

`std-collections-implicit-zone.ari` checks tracked-local zone inference for
hash, tree, deque, linked-list, heap, and priority-queue growth calls.
`std-collections-hash.ari` forces collisions with a custom hash function so the
linear-probing and tombstone paths are exercised.
`std-collections-hash-set-relations.ari` keeps that collision pressure and
checks set relationship predicates after a tombstone. `std-collections-hash-iter`
checks key, value, and set cursors after tombstones.
`std-collections-map-entries.ari` checks hash entries over live buckets and
tree entries in sorted key order.
`std-collections-map-natural-api.ari` keeps compatibility `contains` calls
working while locking down the preferred `contains_key` and fallback `get_or`
spellings for hash and tree maps. `std-collections-map-value-predicates.ari`
checks `contains_value` for hash live buckets after a tombstone and for tree
map values independent of key order. `std-collections-tree.ari` inserts mixed
key order to exercise red-black rotations. `std-collections-tree-boundaries.ari`
checks empty-safe and asserting ordered boundary access for tree maps and sets.
`std-collections-tree-entry-boundaries.ari` checks key/value boundary entry
helpers before and after tree removal.
`std-collections-tree-remove.ari` removes root/internal tree nodes, checks
missing-removal paths, and verifies sorted entries/boundaries after link
rebuild.
`std-collections-tree-set-relations.ari` inserts the same values in different
orders to verify relationship predicates are membership-based, while
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
- Tree removal compacts live storage and rebuilds links in place. A future
  direct red-black deletion path can improve asymptotic removal cost without
  changing the public API.
- `LinkedList[T]` is zone-backed index storage rather than one allocation per
  node. Spare node storage is reused by the list and reclaimed with the zone.
- `BinaryHeap[T]` and `PriorityQueue[T]` require an explicit comparator until
  generic `Ord` dispatch is available inside containers.
