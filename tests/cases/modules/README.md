# Modules Tests

This folder contains focused fixtures for Ari modules behavior. Put valid
programs under `ok/` and expected diagnostics under `errors/` when both kinds
exist.

Use `ok/project-compiler-main.ari` for project-shaped coverage: it imports a
parent facade plus file-backed `source`, `symbols`, `diag`, and `parse` child
modules, then checks cross-file structs, enums, generic payloads, aliases,
visibility, runtime behavior, and module graph output.

Use `ok/inline-file-module-main.ari`, `ok/file-inline-bridge-main.ari`,
`ok/diamond-project-main.ari`, and `ok/trait-project-main.ari` for focused
coverage of inline/file-backed interaction, shared parent-imported dependencies,
and cross-file trait bounds.

Use `errors/` for one failure mode per entry file. The current module model has
separate diagnostics for missing modules, duplicate aliases, private access,
cyclic imports, self-imports, ambiguous file candidates, invalid module syntax,
namespace confusion, and imported-file parse or semantic failures.

Wire new cases into the matching target in `tests/Makefile` and keep each file
centered on one behavior.
