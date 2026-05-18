# Standard Library Module Guides

This folder holds focused guides for individual `std` modules. Use these pages
when the compact API reference is not enough and you need the purpose, current
limits, examples, and test files for one module.

## Available Guides

- [std::option and std::result](option-result.md): ADT helpers for absence,
  failure, conversion, and lazy fallback.
- [std::string](string.md): zone-backed owned byte-string handles, growth,
  borrowed views, byte search, ASCII case search, trim views/copies, and
  whole/prefix parsing helpers.
- [std::io](io.md): low-level process IO hooks plus source byte-slice output.
- [std::input](input.md): stdin-facing byte and line input, including
  `try_read_byte` for `Option[u8]` EOF handling.
- [std::vec](vec.md): zone-backed growable sequence handles, borrowed slices,
  safe accessors, growth, copy, and iterator entry points.
- [std::collections](collections.md): source linear `Set[T]` for unique
  insertion-order values, optional access, iterator support, reserve growth,
  copied views, and explicit-zone provenance.
- [std::cmp](cmp.md): source comparison traits, generic value selection,
  inclusive clamping, and inclusive range predicates.
- [std::convert](convert.md): explicit conversion traits plus source
  `identity`, `from`, and `into` helper calls for generic code.
- [std::context](context.md): runtime-backed program argument and thread-id
  access plus source predicates and root argument aliases.
- [std::env](env.md): user-facing process argument and environment-variable
  helpers, including `try_arg`, `program_name`, `try_get`, `set`,
  `current_dir`, `set_current_dir`, and `try_executable_path`.
- [std::process](process.md): current process id, explicit exit, and status
  helper functions.
- [std::zone](zone.md): explicit allocation capability, raw typed allocation,
  `alloc_array`, placement construction, promotion, reset, and destroy rules.
- [std::ascii](ascii.md): source-only ASCII byte classification,
  printable/control predicates, case conversion, borrowed-slice
  case-insensitive comparison/search, trimming, and digit/prefix parsing
  helpers.
- [std::math](math.md): source-only `i64` arithmetic helpers for signs, sign
  predicates, parity, powers, division rounding, greatest common divisor, and
  least common multiple.
- [std::bits](bits.md): source-only `u64` bit-mask, rotation, power-of-two,
  low-mask, alignment, and zero/one-run bit-scan helpers.

## Guide Shape

Each module guide should answer:

- why the module exists
- what the current APIs do
- which behavior is intentionally not promised yet
- which tests cover the public surface
- what compiler work would be needed for the next larger slice
