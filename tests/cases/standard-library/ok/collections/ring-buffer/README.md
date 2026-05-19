# Ring Buffer Tests

This folder covers `std::collections::RingBuffer[T]`, the fixed-capacity FIFO
queue for bounded buffering.

- `std-collections-ring-buffer.ari`: checks push failure when full,
  overwrite behavior, FIFO pop order, indexed reads after wraparound,
  optional helpers, iteration, and the root `RingBuffer` alias.
