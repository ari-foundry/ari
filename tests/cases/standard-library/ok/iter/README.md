# std::iter Positive Tests

These cases cover source iterator adapters layered on top of the compiler's
existing `Iterator[T]`, `IntoIterator[T]`, and range loop protocol.

Files in this folder:

- `std-iter-adapters.ari`: lazy `map`, `filter`, `take`, `skip`,
  `enumerate`, `zip`, `fold`, `reduce`, and `collect` over source `Vec`
  cursors.
