# Standard Library Module Guides

This folder holds focused guides for individual `std` modules. Use these pages
when the compact API reference is not enough and you need the purpose, current
limits, examples, and test files for one module.

## Available Guides

- [std::option and std::result](option-result.md): ADT helpers for absence,
  failure, conversion, and lazy fallback.
- [std::string](string.md): zone-backed owned byte-string handles, growth,
  borrowed views, byte search, ASCII trim, and parsing helpers.
- [std::vec](vec.md): zone-backed growable sequence handles, borrowed slices,
  safe accessors, growth, copy, and iterator entry points.
- [std::ascii](ascii.md): source-only ASCII byte classification,
  printable/control predicates, case conversion, borrowed-slice trimming, and
  digit parsing helpers.
- [std::bits](bits.md): source-only `u64` bit-mask, power-of-two, low-mask,
  alignment, and bit-scan helpers.

## Guide Shape

Each module guide should answer:

- why the module exists
- what the current APIs do
- which behavior is intentionally not promised yet
- which tests cover the public surface
- what compiler work would be needed for the next larger slice
