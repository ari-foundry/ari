# std::collections

`std::collections` is the home for source collection handles that go beyond a
plain growable sequence. The first implemented slice is `Set[T]`: a
zone-backed, insertion-order, linear set for small unique collections and
deduplication.

The current `Set[T]` is intentionally not named `HashSet`. It does linear
search with `==`, so the API is useful today while leaving the future hash
table policy open.

## Naming Policy

Collection APIs should read naturally:

```ari
set.insert(ref mut zone, value)
set.contains(value)
set.take(value)
set.try_get(index)
set.iter()
set.reserve(ref mut zone, capacity)
set.copy_to(ref mut target)
```

Do not add type suffixes such as `insert_i64`. The type comes from `Set[T]`
and explicit generic calls such as `collections::new<i64>(...)`.

Generic type parameters do not replace function result types in today's Ari
source signatures. Public constructors still spell the return type so module
summaries, docs, and diagnostics have one stable contract to check.

## Constructors

```ari
collections::new<T>(ref mut zone, capacity)
Set::new<T>(ref mut zone, capacity)
collections::from_slice_in<T>(ref mut zone, values)
```

`new` creates an empty set with explicit starting capacity. `Set::new` is the
same constructor as an associated function. `from_slice_in` copies unique
values from a borrowed `Slice[T]` into a new target-zone set.

The handle is tied to the zone used for construction. Ari rejects use of a
tracked set after that zone is reset or destroyed.

## Metadata, Search, Views, And Iteration

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
set.as_slice()
set.iter()
```

`first`, `last`, and `get` are asserting accessors for cases where absence is a
programmer error. The `try_*` forms return `Option[T]` for empty or
out-of-range reads and are the preferred API for ordinary absence. All access
is insertion-order based.

`index_of` returns the insertion-order index of the value or `-1` when absent.
`contains` is the usual membership predicate. `as_slice` returns a borrowed
`Slice[T]` over the current insertion-order elements.

`iter` returns a `std::collections::Iter[T]` cursor over the same
insertion-order elements. `Iter[T]` implements `Iterator[T]`, and `Set[T]`
implements `IntoIterator[T]`, so both `for value in set.iter()` and
`for value in set` use the standard iterator lowering.

## Mutation

```ari
set.insert(ref mut zone, value)
set.remove(value)
set.take(value)
set.pop()
set.try_pop()
set.clear()
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
```

`insert` returns `true` when the value was newly inserted and `false` when the
set already contained it. Because insertion may grow storage, the method takes
the same explicit zone used to create the set.

`remove` returns `true` when it found and dropped the value. `take` returns
`Option[T]` and moves the removed value out instead of dropping it. `pop`
removes and returns the last insertion-order value, while `try_pop` returns
`None` for an empty set.

`clear` drops every live value and keeps the allocated capacity for later
reuse. `reserve` grows to an absolute capacity, and `reserve_extra` grows so
the set can hold at least `len() + additional` elements. Both reserve methods
take the same explicit zone as construction because they may allocate new
backing storage.

## Copies And Lifetimes

```ari
set.copy_to(ref mut target)
```

`copy_to` creates a new set in the target zone and copies the current live
values in insertion order. The copied set is independent from the source
zone, but it is invalidated when the target zone is reset or destroyed.

## Example

```ari
fn main() -> i64 {
  var zone = zone::create(256);
  var seen = collections::new<i64>(ref mut zone, 0);

  seen.insert(ref mut zone, 3);
  seen.insert(ref mut zone, 5);
  seen.insert(ref mut zone, 3);

  let found = seen.contains(5);
  let first = seen.try_first().unwrap_or(0);
  var iter_total = 0;
  for value in seen.iter() {
    iter_total = iter_total + value;
  }
  let removed = seen.take(3).unwrap_or(0);
  let total = seen.len() + first + iter_total + removed + if found { 10 } else { 0 };

  zone::destroy(zone);
  return total;
}
```

## Tests

Focused positive coverage:

```text
tests/cases/standard-library/ok/std-collections-set.ari
tests/cases/standard-library/ok/std-collections-set-access.ari
tests/cases/standard-library/ok/std-collections-set-iter.ari
```

Focused negative coverage:

```text
tests/cases/standard-library/errors/std-collections-set-after-reset.ari
tests/cases/standard-library/errors/std-collections-set-iter-after-reset.ari
tests/cases/standard-library/errors/std-collections-set-insert-different-zone.ari
tests/cases/standard-library/errors/std-collections-set-reserve-different-zone.ari
tests/cases/standard-library/errors/std-collections-set-reserve-extra-different-zone.ari
```

`std-collections-set.ari` covers construction, insertion, duplicate rejection,
membership, removal, borrowed views, copying, and target/source zone behavior.
`std-collections-set-access.ari` covers insertion-order accessors,
`Option`-returning reads, explicit reserve growth, `pop`, and `try_pop`.
`std-collections-set-iter.ari` covers explicit cursors, direct set iteration,
and insertion-order iterator lowering. The iterator reset test confirms that
derived cursors preserve the set's allocation-zone provenance.

`make check-prelude` compiles the positive tests, checks representative
monomorphized symbols, runs the executable results, and checks the negative
zone diagnostics. Public declarations are tracked in
`tests/std_api_manifest.txt` and checked by `make check-std-api`.

## Current Limits

- Operations are linear time.
- `HashMap` and `HashSet` are future modules. They need hashing and equality
  traits, table growth policy, collision/tombstone behavior, iterator behavior,
  and explicit-zone ownership tests.
- The compiler recognizes `std::collections::Set[T]` and
  `std::collections::Iter[T]` as zone-backed handles only for provenance
  checking. The collection behavior itself is Ari source.
