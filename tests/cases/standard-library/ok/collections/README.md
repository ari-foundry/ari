# Collections Standard Library Tests

This folder contains positive tests for the `collections` standard-library
feature group. Keep each `.ari` file focused on one API family or lowering
behavior, and update `tests/Makefile` whenever the fixture should be part of
automated checks.

- `std-collections-set.ari`: linear `Set[T]` construction, insertion,
  duplicate rejection, membership, removal, borrowed views, and zone copies.
- `std-collections-set-access.ari`: insertion-order accessors, optional
  accessors, reserve growth, `pop`, and `try_pop`.
- `std-collections-set-replace.ari`: replace-or-insert behavior and returned
  previous values.
- `std-collections-set-iter.ari`: insertion-order cursor iteration and direct
  `for value in set`.
- `std-collections-hash.ari`: real hash-table collision, replacement,
  tombstone, and removal behavior.
- `std-collections-hash-iter.ari`: hash map key/value cursors and hash set
  live-bucket iteration.
- `std-collections-tree.ari`: red-black tree insertion, replacement, lookup,
  and rotation paths.
- `std-collections-tree-iter.ari`: sorted tree map key/value cursors and tree
  set iteration.
