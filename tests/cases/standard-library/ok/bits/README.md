# Bit Helpers Standard Library Tests

This folder contains positive tests for the `bits` standard-library feature
group. Keep each `.ari` file focused on one API family or lowering behavior,
and update `tests/Makefile` whenever the fixture should be part of automated
checks.

- `std-bits-mask-helpers.ari`: masks, set/clear/toggle, power-of-two checks,
  and alignment helpers.
- `std-bits-rotate-helpers.ari`: left/right rotation and count normalization.
- `std-bits-scan-helpers.ari`: count/leading/trailing zero helpers and zero
  edge cases.
- `std-bits-one-run-helpers.ari`: leading/trailing one-run helpers.
- `std-bits-width-helpers.ari`: bit width, low masks, and power-of-two
  rounding.
- `std-bits-byte-population.ari`: byte-order reversal and the
  `population_count` alias.
