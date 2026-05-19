# std::sync Ok Tests

These tests cover synchronization primitives that are small enough to express
before Ari has full shared ownership, send/share traits, or channel support.

- `std-sync-atomic-i64.ari`: checks the first `AtomicI64` surface, including
  module functions, method wrappers, root aliasing, and LLVM atomic lowering.
- `std-sync-mutex-once.ari`: checks source `Mutex` try/lock/unlock helpers,
  source `Once` call-once state transitions, root aliases, and method wrappers.
