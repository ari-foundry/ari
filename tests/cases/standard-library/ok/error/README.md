# Error Standard Library Tests

This folder contains focused positive tests for `std::error`, the shared
recoverable-error value layer used with `Result[T, Error]`.

- `std-error-basic.ari` covers `Kind`, `Error`, POSIX errno mapping, root
  aliases, predicate helpers, method wrappers, string names, and `Result`
  integration.
