# Literals Errors Tests

This folder contains expected diagnostics for Ari literals behavior. File names should describe the exact feature or diagnostic being locked down.

Add new fixtures to the relevant `tests/Makefile` target when they should run in CI.

- `byte-char-too-long.ari`: single-quoted byte literals must contain one byte.
- `byte-char-unicode.ari`: byte literal Unicode escapes must fit in ASCII.
