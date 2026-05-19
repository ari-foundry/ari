# Process Standard Library Tests

This folder contains positive tests for the `process` standard-library feature
group. Keep each `.ari` file focused on one API family or lowering behavior,
and update `tests/Makefile` whenever the fixture should be part of automated
checks.

- `std-process-basic.ari`: current-process id and status helper predicates.
- `std-process-exit.ari`: explicit process termination status.
- `std-process-fork-wait.ari`: POSIX child-process fork/wait behavior.
