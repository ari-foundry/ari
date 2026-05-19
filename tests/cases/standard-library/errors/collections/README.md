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
- `std-collections-deque-iter-after-reset.ari`
- `std-collections-ring-buffer-after-reset.ari`
- `std-collections-linked-list-iter-after-reset.ari`

The different-zone tests make sure growable collection methods keep using the
zone that created the handle:

- `std-collections-set-insert-different-zone.ari`
- `std-collections-set-replace-different-zone.ari`
- `std-collections-set-reserve-different-zone.ari`
- `std-collections-set-reserve-extra-different-zone.ari`
- `std-collections-hash-map-insert-different-zone.ari`
- `std-collections-tree-set-insert-different-zone.ari`
- `std-collections-deque-push-different-zone.ari`
- `std-collections-linked-list-push-different-zone.ari`
- `std-collections-binary-heap-push-different-zone.ari`
- `std-collections-priority-queue-push-different-zone.ari`
