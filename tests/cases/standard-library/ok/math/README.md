# Math Standard Library Tests

This folder contains positive tests for the `math` standard-library feature
group. Keep each `.ari` file focused on one API family or lowering behavior,
and update `tests/Makefile` whenever the fixture should be part of automated
checks.

- `std-math-integer-helpers.ari`: sign, predicate, parity, power, `gcd`, and
  `lcm` behavior.
- `std-math-division-rounding.ari`: floor/ceil signed division and paired floor
  remainder behavior.
- `std-math-checked-saturating.ari`: checked add/sub/neg/abs and saturating
  add/sub/neg/abs overflow policy behavior.
