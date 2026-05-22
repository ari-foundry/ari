# Borrowing Tests

This folder contains focused fixtures for Ari borrowing behavior. Put valid
programs under `ok/` and expected diagnostics under `errors/` when both kinds
exist.

Coverage is grouped by behavior:

- direct shared and mutable borrows
- borrow conflicts on locals, fields, and reborrowed paths
- named borrow last-use shortening
- borrow-return source tracking and explicit source contracts
- aggregate borrow-valued field replacement
- method receiver borrowing and reborrowing
- control-flow borrow result merging

`make check-ownership` includes `borrow-reborrow-paths.ari` as the fast borrow
smoke. Broader borrow diagnostics and runtime checks remain in
`make check-errors` and `make check-variables`.

Wire new cases into the matching target in `tests/Makefile` and keep each file
centered on one behavior.
