# Collections Standard Library Tests

This folder contains diagnostic tests for the `collections` standard-library feature group. Keep each `.ari` file focused on one API family or lowering behavior, and update `tests/Makefile` whenever the fixture should be part of automated checks.

The reset-after-iterator tests make sure every zone-backed cursor keeps the
source collection's zone provenance:

- `std-collections-set-iter-after-reset.ari`
- `std-collections-hash-map-keys-after-reset.ari`
- `std-collections-hash-map-values-after-reset.ari`
- `std-collections-hash-set-iter-after-reset.ari`
- `std-collections-tree-map-keys-after-reset.ari`
- `std-collections-tree-map-values-after-reset.ari`
- `std-collections-tree-set-iter-after-reset.ari`
