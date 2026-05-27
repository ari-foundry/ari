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
same allocation-header metadata as the raw buffer. Map update-entry handles
also implement `ZoneBacked`; `HashMapEntry` and `TreeMapEntry` store the map
pointer and key only, then recover the growth zone from the backing map with
`map.zone()` when insertion has to allocate.

## Iteration And Mutation

Collection iterators are lightweight cursors over the collection state at the
time they are created. Do not mutate a collection while a live iterator over the
same collection is still being used, unless the method name is itself a
mutation cursor such as `drain`, `entries_mut`, `values_mut`, or `range_mut`.
Those cursors own the mutation path they expose and should be consumed before
normal collection methods are called again.

Linear containers (`Set`, `Deque`, `RingBuffer`, `LinkedList`) iterate in their
documented storage order. Tree containers iterate by sorted key or value order.
Hash containers walk currently live hash buckets; that order is stable only for
an unchanged table and is not insertion order or sorted order.

## Linear Set

```ari
collections::new<T>(ref mut zone, capacity)
Set::new<T>(ref mut zone, capacity)
Set::from_iter<T, I>(ref mut zone, iter)
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
set.retain(keep)
set.extend_iter(iter)
set.extend(iter)
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
set.as_slice()
set.iter()
set.drain()
set.copy_to(ref mut target)
```

`insert` returns `true` only for a newly inserted value. `replace` returns
`Some(previous)` when an equal value already existed, otherwise it inserts and
returns `None`. `take` moves the removed value out, while `remove` drops it.
`retain(fn(ref T) -> bool)` keeps insertion order while dropping values whose
predicate returns `false`. `Set::from_iter(ref mut zone, iter)` and
`extend_iter(iter)` insert only new values from an `Iterator[T]`; duplicates
from the iterator are dropped, preserving the first representative already in
the set.
`equals`, `is_subset`, `is_superset`, and `is_disjoint` compare membership,
not insertion order, and borrow the other set explicitly.
When a local `Set[T]` comes from a tracked zone allocation, the common growth
calls can omit the repeated zone argument: `set.insert(value)`,
`set.replace(value)`, `set.reserve(capacity)`, and
`set.reserve_extra(additional)` infer the set's source zone. Manually assembled
or otherwise untracked sets must keep the explicit `ref mut zone` argument.
`iter` yields insertion-order values and `Set[T]` implements
`IntoIterator[T]`. `drain()` returns a cursor over the current insertion-order
values and immediately leaves the source set empty; if the cursor is dropped
before it is fully consumed, it drops the remaining drained values.

## Deque

`Deque[T]` is a growable double-ended queue. It stores values in a circular
buffer and grows by copying live values into a larger zone allocation in
logical front-to-back order.

```ari
collections::deque<T>(ref mut zone, capacity)
Deque::new<T>(ref mut zone, capacity)
Deque::from_iter<T, I>(ref mut zone, iter)
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
deque.extend_iter(iter)
deque.extend(iter)
deque.reserve(ref mut zone, capacity)
deque.reserve_extra(ref mut zone, additional)
deque.copy_to(ref mut target)
deque.iter()
```

Use `Deque[T]` when both ends matter. For tracked local deque handles,
`push_front(value)`, `push_back(value)`, `reserve(capacity)`, and
`reserve_extra(additional)` infer the constructor zone. `Deque::from_iter` and
`extend_iter` append iterator values at the back in logical order. Keep the
explicit `ref mut zone` form for manually assembled handles. `copy_to(ref mut target)`
copies the logical front-to-back contents into the target zone, so the copy can
outlive a reset or destroy of the source zone. `iter` and `for value in deque`
yield logical front-to-back order, not physical storage order.

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
buffer.copy_to(ref mut target)
buffer.iter()
```

`push` returns `false` when the buffer is full. `push_overwrite` always keeps
the new value; it returns `Some(oldest)` when a full buffer overwrote the
oldest slot and `None` when it appended without overwriting. `copy_to(ref mut
target)` preserves the fixed capacity and FIFO logical order in the target
zone. `peek` and `pop` operate on the oldest value.

## LinkedList

`LinkedList[T]` uses doubly linked node indices over zone-backed arrays. It
does not allocate one host heap node at a time; Ari keeps allocation explicit
and zone-oriented. Removed node slots are put on a small free list and reused
by later pushes.

```ari
collections::linked_list<T>(ref mut zone, capacity)
LinkedList::new<T>(ref mut zone, capacity)
LinkedList::from_iter<T, I>(ref mut zone, iter)
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
list.extend_iter(iter)
list.extend(iter)
list.reserve(ref mut zone, capacity)
list.reserve_extra(ref mut zone, additional)
list.copy_to(ref mut target)
list.iter()
```

`get` and `remove_at` traverse from the head, so they are O(n). Prefer
front/back operations for queue-like code. `LinkedList::from_iter` and
`extend_iter` append values at the back, preserving iterator order. For tracked
local linked-list handles, `push_front(value)`, `push_back(value)`,
`reserve(capacity)`, and `reserve_extra(additional)` infer the constructor
zone. `iter` yields
front-to-back values. `copy_to(ref mut target)` rebuilds only the live
front-to-back values in the target zone, leaving free-list holes behind.

## HashMap And HashSet

Hash collections are real hash tables, not aliases over `Set`. They use linear
probing, tombstones for removal, and a load-factor growth rule. For ordinary
keys that implement `std::hash::Hash`, use `new` to select the stdlib default
hash policy, or `with_capacity` when the initial bucket count should be
explicit. For caller-specific hash policy, use `with_hash` or
the lower-level `collections::hash_map` / `collections::hash_set` constructors.
For tracked local hash handles, `map.insert(key, value)`, `map.entry(key)`,
`map.reserve(capacity)`, `map.reserve_extra(additional)`, `set.insert(value)`,
`set.replace(value)`, `set.reserve(capacity)`, and
`set.reserve_extra(additional)` infer the constructor zone.

```ari
collections::hash_i64(value)
collections::hash_string(value)
collections::hash_map<K, V>(ref mut zone, capacity, hash)
collections::string_hash_map<V>(ref mut zone, capacity)
HashMap::new<K: std::hash::Hash[K], V>(ref mut zone)
HashMap::with_capacity<K: std::hash::Hash[K], V>(ref mut zone, capacity)
HashMap::with_hash<K, V>(ref mut zone, capacity, hash)
HashMap::from_iter<K, V, I>(ref mut zone, capacity, hash, iter)
collections::hash_set<T>(ref mut zone, capacity, hash)
collections::string_hash_set(ref mut zone, capacity)
HashSet::new<T: std::hash::Hash[T]>(ref mut zone)
HashSet::with_capacity<T: std::hash::Hash[T]>(ref mut zone, capacity)
HashSet::with_hash<T>(ref mut zone, capacity, hash)
HashSet::from_iter<T, I>(ref mut zone, capacity, hash, iter)
```

The hash function shape is `fn(K) -> u64` for `HashMap[K, V]` and
`fn(T) -> u64` for `HashSet[T]`. `collections::hash_i64` is kept as a
compatibility helper for i64 keys and delegates to `std::hash::value<i64>`.
`collections::hash_string` delegates to `std::hash::string`, and
`string_hash_map` / `string_hash_set` wire that policy in for the common
`String` key case. Owned `String` equality is content-based through
`std::cmp::Eq[String]`, so independently allocated strings with the same bytes
find the same hash map entry.
`HashMap::new` and `HashSet::new` are the common constructors. They call
`std::hash::value<T>` for the key/value type, use a small default starting
capacity, and leave equality to the same `==`/`std::cmp::Eq` behavior used by
lookups. Use `HashMap::with_capacity` and `HashSet::with_capacity` when the
default hash policy should start with caller-chosen capacity. Use
`HashMap::with_hash` and `HashSet::with_hash` when a custom hash policy is
intentional.

```ari
map.len()
map.capacity()
map.is_empty()
map.contains(key)
map.contains_key(key)
map.contains_value(value)
map.contains_key_bytes(bytes) // HashMap[String, V]
map.get(key)
map.get_bytes(bytes) // HashMap[String, V]
map.get_or(key, fallback)
map.get_or_bytes(bytes, fallback) // HashMap[String, V]
map.try_get(key)
map.try_get_bytes(bytes) // HashMap[String, V]
map.get_mut(key)
map.get_mut_bytes(bytes) // HashMap[String, V]
map.try_get_mut(key)
map.try_get_mut_bytes(bytes) // HashMap[String, V]
map.insert(ref mut zone, key, value)
map.replace(ref mut zone, key, value)
map.entry(ref mut zone, key)
map.entry(key)
map.remove(key)
map.remove_bytes(bytes) // HashMap[String, V]
map.remove_entry(key)
map.remove_entry_bytes(bytes) // HashMap[String, V]
map.clear()
map.retain(keep)
map.extend_iter(iter)
map.extend(iter)
map.reserve(ref mut zone, capacity)
map.reserve_extra(ref mut zone, additional)
map.copy_to(ref mut target)
map.keys()
map.values()
map.values_mut()
map.entries()
map.iter()
map.iter_mut()
map.drain()
```

`HashMap.insert` inserts or replaces and returns `Option[V]`: `Some(previous)`
on replacement, `None` on a new key. `contains_key` is the preferred
key-membership spelling; `contains` remains available for compatibility with
older examples. `contains_value` scans live bucket values and ignores
tombstones. `get_or(key, fallback)` is the compact spelling when a missing key
has an ordinary fallback value; use `try_get` when absence should stay visible
as `Option[V]`. `get_mut(key)` asserts that the key exists and returns
`ref mut V`; `try_get_mut(key)` returns `Option[MapValueMut[V]]` so callers can
branch on absence and then call `value_mut() -> ref mut V` on the handle.
`replace(ref mut zone, key, value)` is the named insert-or-replace spelling and
returns the previous value just like `insert`. `remove` returns the removed
value. `keys` and `values` iterate live buckets. That order is deterministic
for a specific table state, but it is not insertion order and should not be
used as a stable sorting rule. `entries` yields copied `MapEntry[K, V]` values
with `.key`/`.value` fields and `key()`/`value()` accessors over the same live
buckets. `iter()` is the same entry cursor under the shorter common collection
spelling, and `HashMap[K,V]` implements `IntoIterator[MapEntry[K,V]]`, so
`for entry in map` walks copied entries directly. `values_mut()` yields a
mutable value cursor with `has_next()` and `next() -> ref mut V`; use it when
keys must stay fixed but stored values should be updated in place. `iter_mut()`
yields `MapEntryMut[K,V]` handles with `key()`, `value()`, and `value_mut()` so
code can read the copied key while mutating the stored value.
`retain(fn(ref K, ref mut V) -> bool)` scans live buckets in place. The
predicate can mutate retained values; a `false` result drops the key/value pair
and leaves a tombstone so later collision probes remain correct.
`HashMap::from_iter(ref mut zone, capacity, hash, iter)` and `extend_iter(iter)`
consume `Iterator[MapEntry[K,V]]`; an incoming duplicate key replaces the stored
value and drops the previous key/value pair. `drain()`
marks the current live entries as drained, leaves the map empty, and returns a
draining entry cursor.
`reserve_extra(additional)` reserves enough hash buckets
for the requested live length without immediately violating the table's load
factor rule. `copy_to(ref mut target)` copies only live entries into the target
zone and leaves tombstones behind.

String-key hash maps also have borrowed byte-slice lookup and removal helpers:
`contains_key_bytes`, `try_get_bytes`, `get_bytes`, `get_or_bytes`,
`try_get_mut_bytes`, `get_mut_bytes`, `remove_bytes`, and
`remove_entry_bytes`. They are available on `HashMap[String, V]` values created
with `string_hash_map`, `HashMap::new<String,V>`, or an explicit
`HashMap::with_hash(..., hash_string)` policy. These helpers hash the borrowed
bytes directly and compare them against
stored `String` keys by byte content, so parser-style code can look up
`Slice[u8]` keys from an input line without allocating a temporary `String`.

`map.entry(key)` returns a mutable update handle. It is separate from
`MapEntry[K, V]`: `MapEntry` is a copied key-value result from iterators,
boundaries, and `remove_entry`, while `HashMapEntry[K, V]` is a short-lived
handle used to update a stored value in place. The explicit spelling is
`map.entry(ref mut zone, key)`, but tracked local maps infer the same zone that
created the map. The returned entry does not store that zone separately; it
uses the map's allocation-header metadata through `map.zone()`.

```ari
map.entry(word).or_insert(0) += 1
map.entry(key).and_modify(update).or_insert(fallback) += 0
map.entry(key).or_insert_with(make_value) += 0
map.entry(key).or_default() += 1
map.entry(key).insert(value)
map.entry(key).insert_entry(value).value_mut() += 1
map.entry(key).value_mut() += 1
map.entry(key).remove()
map.remove_entry(key)
```

`or_insert(value)` inserts `value` when the key is absent and returns
`ref mut V`. `or_insert_with(make_value)` calls `make_value` only when the key
is absent. `or_default()` is available when `V: Default`; it inserts
`Default::default<V>()` only when the key is absent and returns `ref mut V`.
`and_modify(fn(ref mut V) -> void)` runs only when the key already exists and
returns the entry handle for chaining. `insert(value)` replaces the stored
value and returns the previous value as `Option[V]`, or inserts a new value and
returns `None`. `insert_entry(value)` stores `value` and returns the same
entry handle so callers can continue with `key()`, `value()`, or
`value_mut()`; if a value was already present, the old value is dropped instead
of returned. `remove()` removes through the entry handle and returns
`Option[V]`. `key()` returns the handle key; `value()` and
`value_mut()` assert that the key exists, then return the stored value by value
or by mutable reference. `remove_entry(key)` returns
`Option[MapEntry[K, V]]`, preserving both the removed key and value.

```ari
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.contains_bytes(bytes) // HashSet[String]
set.get(value)
set.get_bytes(bytes) // HashSet[String]
set.try_get(value)
set.try_get_bytes(bytes) // HashSet[String]
set.equals(ref other)
set.is_subset(ref other)
set.is_superset(ref other)
set.is_disjoint(ref other)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.take(value)
set.take_bytes(bytes) // HashSet[String]
set.remove(value)
set.remove_bytes(bytes) // HashSet[String]
set.clear()
set.retain(keep)
set.extend_iter(iter)
set.extend(iter)
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
set.copy_to(ref mut target)
set.iter()
set.drain()
```

`HashSet.insert` returns whether the value was newly inserted. `replace`
returns the previous equal value when present. `take` moves a removed value out;
`remove` drops it. `equals`, `is_subset`, `is_superset`, and `is_disjoint`
compare membership over live buckets and ignore tombstones. `try_get(value)`
returns the stored equal representative as `Option[T]`, and `get(value)` asserts
that such a representative exists. `HashSet.iter()`
yields live buckets, and `HashSet[T]` implements `IntoIterator[T]` so
`for value in set` works through the same cursor. `retain(fn(ref T) -> bool)`
drops values whose predicate returns `false` and marks those buckets as
tombstones; later inserts can still reuse the table normally. `HashSet.drain()`
returns a live-bucket draining cursor and leaves the set empty.
`HashSet::from_iter` and `extend_iter` consume `Iterator[T]`; duplicate values
are dropped and the first stored representative is retained.
`copy_to(ref mut target)` copies live values into a fresh target-zone hash
table without tombstones.

String hash sets mirror the map borrowed lookup surface with
`contains_bytes`, `try_get_bytes`, `get_bytes`, `take_bytes`, and
`remove_bytes`. These helpers let parser and lexer code test or remove a
borrowed `Slice[u8]` spelling against an owned `String` set without allocating
a temporary key.

## TreeMap And TreeSet

Tree collections are red-black trees. They provide ordered lookup without
needing a hash function. For ordinary keys that implement `std::cmp::Ord`, use
`new` to select the stdlib default less-than policy, or `with_capacity` when
the initial tree storage should be explicit. Custom orderings stay explicit
through `with_less` or the lower-level constructor functions.

```ari
collections::less_i64(left, right)
collections::tree_map<K, V>(ref mut zone, capacity, less)
TreeMap::new<K: std::cmp::Ord[K], V>(ref mut zone)
TreeMap::with_capacity<K: std::cmp::Ord[K], V>(ref mut zone, capacity)
TreeMap::with_less<K, V>(ref mut zone, capacity, less)
TreeMap::from_iter<K, V, I>(ref mut zone, capacity, less, iter)
collections::tree_set<T>(ref mut zone, capacity, less)
TreeSet::new<T: std::cmp::Ord[T]>(ref mut zone)
TreeSet::with_capacity<T: std::cmp::Ord[T]>(ref mut zone, capacity)
TreeSet::with_less<T>(ref mut zone, capacity, less)
TreeSet::from_iter<T, I>(ref mut zone, capacity, less, iter)
```

The comparator shape is `fn(K, K) -> bool` for `TreeMap[K, V]` and
`fn(T, T) -> bool` for `TreeSet[T]`. `collections::less_i64` is the first
built-in helper for i64 keys.
`TreeMap::new` and `TreeSet::new` are the common constructors. They use the
ordinary `<` operation available through `std::cmp::Ord[T]` and a small default
starting capacity. `TreeMap::with_capacity` and `TreeSet::with_capacity` keep
that default ordering while letting callers choose initial storage. Keep custom
orderings explicit with `TreeMap::with_less`, `TreeSet::with_less`,
`collections::tree_map`, or `collections::tree_set`, so caller-specific sort
policy remains readable without making the common ordered path noisy.

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
map.get_mut(key)
map.try_get_mut(key)
map.lower_bound(key)
map.upper_bound(key)
map.range(start, end)
map.range_mut(start, end)
map.insert(ref mut zone, key, value)
map.replace(ref mut zone, key, value)
map.entry(ref mut zone, key)
map.entry(key)
map.remove(key)
map.remove_entry(key)
map.pop_first()
map.pop_last()
map.split_off(ref mut zone, start)
map.split_off(start)
map.append(ref mut zone, ref mut other)
map.append(ref mut other)
map.clear()
map.extend_iter(iter)
map.extend(iter)
map.reserve(ref mut zone, capacity)
map.reserve_extra(ref mut zone, additional)
map.copy_to(ref mut target)
map.keys()
map.values()
map.values_mut()
map.entries()
map.iter()
map.iter_mut()
map.drain()
```

`TreeMap.insert` inserts or replaces and returns `Option[V]`. `contains_key`
is the preferred key-membership spelling; `contains` remains available for
compatibility with older examples. `contains_value` scans stored values without
using key order. `get_or(key, fallback)` returns the stored value or the
fallback without forcing every call site to unwrap `Option[V]`. `get_mut(key)`
asserts that the key exists and returns `ref mut V`; `try_get_mut(key)` returns
`Option[MapValueMut[V]]` with `value()` and `value_mut()` helpers for
miss-safe mutation. `replace(ref mut zone, key, value)` is the named
insert-or-replace spelling and returns the previous value just like `insert`.
`keys` yields keys in ascending comparator order. `values` yields values in
the same key-sorted order. `first_key`,
`last_key`, `first_value`, and `last_value` assert when the tree is empty;
use the `try_*` forms for empty-safe boundary access. `first_entry` and
`last_entry` return `MapEntry[K, V]` so callers can read the boundary key and
value without doing two boundary lookups. `lower_bound(key)` returns the first
entry whose key is not less than `key`, and `upper_bound(key)` returns the
first entry whose key is greater than `key`; both return `None` when no such
entry exists. `range(start, end)` yields copied `MapEntry[K, V]` values in
the half-open key interval `[start, end)`. `range_mut(start, end)` yields
`MapEntryMut[K, V]` handles over the same key interval for in-place value
updates without copying the whole map. `entries` yields `MapEntry[K, V]`
values with `.key` and `.value` fields in the same sorted key order. `iter()`
is an alias for `entries()`, and direct `for entry in map` uses the same
copied-entry order. `values_mut()` walks values in sorted key order with a
mutable cursor; call `has_next()` before `next() -> ref mut V`. `iter_mut()`
walks sorted `MapEntryMut[K,V]` handles for copied-key plus mutable-value
updates.
`drain()` returns entries in sorted key order and immediately leaves the source
tree map empty. `pop_first()` and `pop_last()` remove and return the smallest
or largest entry as `Option[MapEntry[K,V]]`. `split_off(start)` moves every
entry in `[start, +inf)` into a same-zone tree map and leaves smaller keys in
the source. `append(ref mut other)` drains `other` into `self`, replacing
duplicate keys in `self` and dropping the replaced key/value pair.
`TreeMap::from_iter` and `extend_iter` consume sorted-map style
`Iterator[MapEntry[K,V]]` values with the same replacement rule. `remove`
returns the removed value as `Option[V]`; deletion now uses red-black delete
fixup, then compacts the live node arrays by moving the final live slot into
the removed slot and updating only the affected parent/child/root references.
Removal does not allocate through a zone and no longer rebuilds every tree
link. For tracked local tree maps,
`map.insert(key, value)`, `map.entry(key)`, `map.split_off(start)`,
`map.append(ref mut other)`, `map.reserve(capacity)`, and
`map.reserve_extra(additional)` infer the constructor zone. `copy_to(ref mut
target)` rebuilds the map in the target zone with the same comparator.

`TreeMap.entry(key)` mirrors `HashMap.entry(key)`, but lookup follows the map's
strict less-than comparator. `TreeMapEntry[K, V]` supports the same
`or_insert`, `or_insert_with`, `or_default`, `and_modify`, `insert`,
`insert_entry`, `remove`, `key`, `value`, and `value_mut` methods, and
recovers allocation metadata from the backing tree map instead of carrying a
separate zone pointer. `TreeMapEntry.zone()` exposes that same metadata.
`remove_entry(key)` returns
`Option[MapEntry[K, V]]` after the same direct red-black deletion and storage
compaction path.

```ari
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.get(value)
set.try_get(value)
set.first()
set.try_first()
set.last()
set.try_last()
set.lower_bound(value)
set.upper_bound(value)
set.range(start, end)
set.equals(ref other)
set.is_subset(ref other)
set.is_superset(ref other)
set.is_disjoint(ref other)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.take(value)
set.remove(value)
set.split_off(ref mut zone, start)
set.split_off(start)
set.append(ref mut zone, ref mut other)
set.append(ref mut other)
set.clear()
set.extend_iter(iter)
set.extend(iter)
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
set.copy_to(ref mut target)
set.iter()
set.drain()
```

`TreeSet.insert` returns `false` for an equal existing value. `TreeSet.replace`
returns the previous equal value or inserts a new one. `equals`, `is_subset`,
`is_superset`, and `is_disjoint` compare ordered-set membership, not the
internal tree shape. `first` and `last` read the smallest and largest values
in comparator order and assert when the tree is empty; use `try_first` and
`try_last` when emptiness is ordinary control flow. `lower_bound(value)`
returns the first stored value that is not less than `value`, and
`upper_bound(value)` returns the first stored value greater than `value`; both
return `None` past the right edge. `range(start, end)` yields values in the
half-open comparator interval `[start, end)`. `try_get(value)` returns the
stored equal representative as `Option[T]`, and `get(value)` asserts that it exists. `take`
moves a removed value out as
`Option[T]`; `remove` drops the removed value and returns whether anything was
removed. Tree-set removal uses direct red-black delete fixup and compacting
slot movement without allocating or rebuilding all links. `TreeSet.iter()` yields values in ascending comparator
order, and `TreeSet[T]` implements `IntoIterator[T]`. `TreeSet.drain()` yields
values in ascending comparator order and leaves the source tree set empty.
`split_off(start)` moves values in `[start, +inf)` into a same-zone ordered
set. `append(ref mut other)` drains `other` into `self`; duplicate values are
dropped and the existing representative stays. `TreeSet::from_iter` and
`extend_iter` use the same insertion rule. For tracked local tree sets,
`set.insert(value)`, `set.replace(value)`, `set.split_off(start)`,
`set.append(ref mut other)`, and `set.reserve(capacity)`/
`set.reserve_extra(additional)` infer the constructor zone.
`copy_to(ref mut target)` rebuilds the ordered set in the target zone
with the same comparator.

## BinaryHeap And PriorityQueue

`BinaryHeap[T]` and `PriorityQueue[T]` are max-priority containers. For
ordinary values that implement `std::cmp::Ord`, use `new` to select the stdlib
default ordering, or `with_capacity` when the initial heap storage should be
explicit. For caller-specific priority policy, keep the comparator explicit:
`less(a, b)` means `a` has lower priority than `b`. With
`collections::less_i64`, larger integers pop first.

```ari
collections::binary_heap<T>(ref mut zone, capacity, less)
BinaryHeap::new<T: std::cmp::Ord[T]>(ref mut zone)
BinaryHeap::with_capacity<T: std::cmp::Ord[T]>(ref mut zone, capacity)
BinaryHeap::with_less<T>(ref mut zone, capacity, less)
BinaryHeap::from_iter<T, I>(ref mut zone, capacity, less, iter)
collections::priority_queue<T>(ref mut zone, capacity, less)
PriorityQueue::new<T: std::cmp::Ord[T]>(ref mut zone)
PriorityQueue::with_capacity<T: std::cmp::Ord[T]>(ref mut zone, capacity)
PriorityQueue::with_less<T>(ref mut zone, capacity, less)
PriorityQueue::from_iter<T, I>(ref mut zone, capacity, less, iter)
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
heap.extend_iter(iter)
heap.extend(iter)
heap.reserve(ref mut zone, capacity)
heap.reserve_extra(ref mut zone, additional)
heap.copy_to(ref mut target)
queue.copy_to(ref mut target)
```

`PriorityQueue` keeps the same basic push, peek, pop, reserve, clear, and
copy surface. `peek` reads the current highest-priority value, while `pop`
removes it and restores the heap invariant. `BinaryHeap` also exposes
`peek_mut()` and `into_sorted_vec()`. `BinaryHeap.peek_mut()` returns a
`BinaryHeapPeekMut[T]` guard; `value()` reads the current top, `value_mut()`
edits it, and dropping the guard sifts the root down so the heap invariant is
restored. The guard keeps only heap/value pointers and does not store a zone
handle. `into_sorted_vec()` drains the heap into a `Vec[T]` ordered from low
priority to high priority for the comparator; with `less_i64`, the resulting
vector is ascending. `BinaryHeap::from_iter`/`PriorityQueue::from_iter` and
`extend_iter` consume `Iterator[T]`, append every value, and sift it into the
heap invariant.
`BinaryHeap::new` and `PriorityQueue::new` are the common constructors. They
use the ordinary `<` operation available through `std::cmp::Ord[T]` and a
small default starting capacity. `BinaryHeap::with_capacity` and
`PriorityQueue::with_capacity` keep that default ordering while letting callers
choose initial storage. Use `BinaryHeap::with_less`,
`PriorityQueue::with_less`, `collections::binary_heap`, or
`collections::priority_queue` when a custom priority relation should be visible
at the construction site.
For tracked local heap and priority-queue handles, `push(value)`,
`reserve(capacity)`, and `reserve_extra(additional)` infer the constructor
zone. `copy_to(ref mut target)` copies the heap storage and comparator into the
target zone, preserving priority pop order without requiring the original zone.
Use `BinaryHeap[T]` when the data-structure name matters; use
`PriorityQueue[T]` when the abstraction is an application queue.

## Examples

Hash table:

```ari
fn main() -> i64 {
  var zone = zone::create(2048);
  var map = HashMap::with_hash<i64, i64>(ref mut zone, 8, collections::hash_i64);

  map.insert(7, 70);
  map.insert(11, 110);
  let value = map.try_get(7).unwrap_or(0);

  zone::destroy(zone);
  return value;
}
```

String-key hash table:

```ari
fn main() -> i64 {
  var zone = zone::create(2048);
  var names = collections::string_hash_map<String>(ref mut zone, 8);

  let name_key = string::from(ref mut zone, "name");
  let hello_value = string::from(ref mut zone, "hello");
  names.insert(name_key, hello_value);

  let lookup_key = string::from(ref mut zone, "name");
  let value = names.get(lookup_key);
  let result = value.len();

  zone::destroy(zone);
  return result;
}
```

For arix-style manifest parsing, prefer `string_hash_map` for section and
key/value tables today:

```ari
fn main() -> i64 {
  var zone = zone::create(4096);
  var package = collections::string_hash_map<String>(ref mut zone, 4);

  let version_key = string::from(ref mut zone, "version");
  let version_value = string::from(ref mut zone, "0.1.0");
  package.insert(version_key, version_value);

  let query = string::from(ref mut zone, "version");
  let fallback = string::from(ref mut zone, "0.0.0");
  let version = package.get_or(query, fallback);
  let borrowed_version = package.get_bytes(string::bytes("version"));
  let result = version.len() + borrowed_version.len();

  zone::destroy(zone);
  return result;
}
```

`HashMap::new(ref mut zone)` is the common constructor for ordinary hashable
keys. `HashMap::with_capacity(ref mut zone, capacity)` uses the same default
hash policy with caller-chosen storage. `HashMap::with_hash(ref mut zone,
capacity, hash)` is the named custom-hasher constructor. `string_hash_map`
remains the concise
`HashMap[String, V]` constructor and uses content hashing plus content equality
for independently allocated `String`
values. For parser code, use the `_bytes` methods on `HashMap[String, V]` and
`HashSet[String]` when the query key is already a borrowed `Slice[u8]`.

Ordered tree:

```ari
fn main() -> i64 {
  var zone = zone::create(2048);
  var set = TreeSet::new<i64>(ref mut zone);

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
tests/cases/standard-library/ok/collections/std-collections-copy-to.ari
tests/cases/standard-library/ok/collections/std-collections-structure-copy-to.ari
tests/cases/standard-library/ok/collections/std-collections-hash.ari
tests/cases/standard-library/ok/collections/std-collections-hash-set-relations.ari
tests/cases/standard-library/ok/collections/std-collections-hash-iter.ari
tests/cases/standard-library/ok/collections/std-collections-set-representatives.ari
tests/cases/standard-library/ok/collections/std-collections-map-entries.ari
tests/cases/standard-library/ok/collections/std-collections-view-api.ari
tests/cases/standard-library/ok/collections/std-collections-map-natural-api.ari
tests/cases/standard-library/ok/collections/std-collections-map-mut-access.ari
tests/cases/standard-library/ok/collections/std-collections-map-value-predicates.ari
tests/cases/standard-library/ok/collections/std-collections-tree.ari
tests/cases/standard-library/ok/collections/std-collections-tree-boundaries.ari
tests/cases/standard-library/ok/collections/std-collections-tree-entry-boundaries.ari
tests/cases/standard-library/ok/collections/std-collections-tree-bounds.ari
tests/cases/standard-library/ok/collections/std-collections-tree-remove.ari
tests/cases/standard-library/ok/collections/std-collections-tree-set-relations.ari
tests/cases/standard-library/ok/collections/std-collections-tree-iter.ari
tests/cases/standard-library/ok/collections/std-collections-polish-api.ari
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
`std-collections-copy-to.ari` checks target-zone copies for hash and tree
maps/sets, including tombstone skipping for hash collections and post-source
destroy reads from target-zone storage.
`std-collections-structure-copy-to.ari` checks target-zone copies for deque,
ring buffer, linked list, binary heap, and priority queue after source-zone
destroy, with logical order checks for sequence-like collections and priority
order checks for heap-backed collections.
`std-collections-hash.ari` forces collisions with a custom hash function so the
linear-probing and tombstone paths are exercised.
`std-collections-hash-set-relations.ari` keeps that collision pressure and
checks set relationship predicates after a tombstone. `std-collections-hash-iter`
checks key, value, and set cursors after tombstones.
`std-collections-map-entries.ari` checks hash entries over live buckets and
tree entries in sorted key order.
`std-collections-view-api.ari` checks map `values_mut()` cursors, map
`iter()` aliases, `MapEntryMut`-based `iter_mut()` cursors, direct map
`IntoIterator`, and draining cursors for linear, hash, and tree maps/sets.
`std-collections-map-natural-api.ari` keeps compatibility `contains` calls
working while locking down the preferred `contains_key` and fallback `get_or`
spellings for hash and tree maps. `std-collections-map-mut-access.ari` checks
asserting `get_mut`, optional `try_get_mut` value handles, named `replace`, and
removal for both hash and tree maps. `std-collections-map-value-predicates.ari`
checks `contains_value` for hash live buckets after a tombstone and for tree
map values independent of key order. `std-collections-set-representatives.ari`
checks `HashSet.get`/`try_get` and `TreeSet.get`/`try_get` before and after
replacement/removal paths. `std-collections-tree.ari` inserts mixed
key order to exercise red-black rotations. `std-collections-tree-boundaries.ari`
checks empty-safe and asserting ordered boundary access for tree maps and sets.
`std-collections-tree-entry-boundaries.ari` checks key/value boundary entry
helpers before and after tree removal.
`std-collections-tree-bounds.ari` checks comparator-order lower/upper bound
lookup for tree maps and sets.
`std-collections-tree-remove.ari` removes root/internal tree nodes, checks
missing-removal paths, and verifies sorted entries/boundaries after direct
red-black deletion with compacting slot movement.
`std-collections-tree-set-relations.ari` inserts the same values in different
orders to verify relationship predicates are membership-based, while
`std-collections-tree-iter.ari` checks sorted successor traversal plus tree
map/set range cursors and mutable range value updates.
`std-collections-polish-api.ari` checks iterator-driven constructors and
extension across set/map/sequence/heap collections, plus tree boundary pops,
`split_off`, and `append`.
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
- Hash containers now have natural `new` constructors for ordinary `Hash[T]`
  keys, plus `with_capacity` for explicit default-policy capacity. Use
  `with_hash` only when custom hash policy is intentional.
- Tree containers now have natural `new` constructors for ordinary `Ord[T]`
  keys, plus `with_capacity` for explicit default-policy capacity. Use
  `with_less` only when custom ordering policy is intentional.
- `values_mut()` uses a `has_next()`/`next()` cursor because Ari does not yet
  lower `Iterator[ref mut T]`/`Option[ref mut T]` as a stable public iterator
  item shape. Map `iter_mut()` therefore yields `MapEntryMut[K,V]` handles
  rather than `Option[ref mut (K,V)]`. Set `iter_mut()` stays future because
  mutating set values can break hash and ordering invariants.
- `try_get_mut()` returns `Option[MapValueMut[V]]` rather than
  `Option[ref mut V]` for the same reference-valued generic payload reason.
- Tree removal uses direct red-black delete fixup plus compacting slot
  movement. It preserves the public API while avoiding the older full-link
  rebuild path.
- `LinkedList[T]` is zone-backed index storage rather than one allocation per
  node. Spare node storage is reused by the list and reclaimed with the zone.
- `BinaryHeap[T]` and `PriorityQueue[T]` now have natural `new` constructors
  for ordinary `Ord[T]` values, plus `with_capacity` for explicit
  default-policy capacity. Use `with_less` only when custom priority policy is
  intentional.
