# std::collections

`std::collections` contains source Ari collection handles that allocate through
an explicit `Zone`. The module now has three distinct families:

| Type | Data Structure | Use It For |
| --- | --- | --- |
| `Set[T]` | linear insertion-order buffer | tiny unique lists, stable insertion-order access, iteration |
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
```

`HashMap.insert` inserts or replaces and returns `Option[V]`: `Some(previous)`
on replacement, `None` on a new key. `remove` returns the removed value.

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
```

`HashSet.insert` returns whether the value was newly inserted. `replace`
returns the previous equal value when present. `take` moves a removed value out;
`remove` drops it.

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
```

`TreeMap.insert` inserts or replaces and returns `Option[V]`.

```ari
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.clear()
set.reserve(ref mut zone, capacity)
```

`TreeSet.insert` returns `false` for an equal existing value. `TreeSet.replace`
returns the previous equal value or inserts a new one.

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
tests/cases/standard-library/ok/collections/std-collections-tree.ari
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
tests/cases/standard-library/errors/collections/std-collections-hash-map-insert-different-zone.ari
tests/cases/standard-library/errors/collections/std-collections-tree-map-after-reset.ari
tests/cases/standard-library/errors/collections/std-collections-tree-set-insert-different-zone.ari
```

`std-collections-hash.ari` forces collisions with a custom hash function so the
linear-probing and tombstone paths are exercised. `std-collections-tree.ari`
inserts mixed key order to exercise red-black rotations.

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
- Tree removal and iterators are not implemented in this slice. They should
  land with focused red-black deletion and traversal tests.
