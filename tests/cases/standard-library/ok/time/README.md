# std::time Positive Tests

These cases cover the first `std::time` slice:

- runtime-backed monotonic and wall-clock nanosecond hooks
- source `Duration`, `Instant`, and `SystemTime` wrappers
- natural duration constructors and metadata methods
- elapsed-time helpers and zero-duration sleep
- monotonic `Deadline` and timeout helpers
- UTC-only calendar conversion and leap-year/month-length policy

Keep time tests deterministic. They may check monotonic ordering and positive
wall-clock values, but they should not depend on exact host timings.
