# Deque Tests

This folder covers `std::collections::Deque[T]`, the growable double-ended
queue backed by a circular buffer.

- `std-collections-deque.ari`: checks front/back pushes, pops, indexed access,
  growth after wraparound, optional pop/access helpers, iteration, and the root
  `Deque` alias.
