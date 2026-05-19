# std::thread Ok Tests

This folder contains positive tests for the first `thread` standard-library
slice.

- `std-thread-basic.ari`: runtime thread id helpers, function-pointer spawn,
  join result propagation, invalid-handle sentinel handling, and cooperative
  `yield_now`.
- `std-thread-runtime-helpers.ari`: hosted available-parallelism query,
  source `thread::sleep` wrapper, and use of both helpers from a spawned
  thread.
