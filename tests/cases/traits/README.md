# Traits Tests

This folder contains focused fixtures for Ari traits behavior. Put valid
programs under `ok/` and expected diagnostics under `errors/` when both kinds
exist.

The `trait-minimum-*` files are the production-ready minimum subset fixtures:
they cover user-defined compiler-shaped traits, stdlib-like Eq/Ord/Hash/Debug
and iterator usage, generic bounds, generic impl bounds, deterministic static
dispatch, and stable diagnostics without hard-coding specific stdlib types.

Wire new cases into the matching target in `tests/Makefile` and keep each file
centered on one behavior.
