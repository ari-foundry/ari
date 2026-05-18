# Standard Library Module Guides

This folder holds focused guides for individual `std` modules. Use these pages
when the compact API reference is not enough and you need the purpose, current
limits, examples, and test files for one module.

## Available Guides

- [std::option and std::result](option-result.md): ADT helpers for absence,
  failure, conversion, and lazy fallback.
- [std::ascii](ascii.md): source-only ASCII byte classification, case
  conversion, borrowed-slice trimming, and digit parsing helpers.
- [std::bits](bits.md): source-only `u64` bit-mask, power-of-two alignment,
  and bit-scan helpers.

## Guide Shape

Each module guide should answer:

- why the module exists
- what the current APIs do
- which behavior is intentionally not promised yet
- which tests cover the public surface
- what compiler work would be needed for the next larger slice
