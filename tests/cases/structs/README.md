# Structs Tests

This folder contains focused fixtures for Ari structs behavior. Put valid
programs under `ok/` and expected diagnostics under `errors/` when both kinds
exist. Representative literal and field-access failures also have golden
diagnostic artifacts under `tests/cases/compiler-development/artifact/errors/`
so source spans stay stable.

Wire new cases into the matching target in `tests/Makefile` and keep each file centered on one behavior.
