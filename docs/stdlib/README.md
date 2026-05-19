# Ari Standard Library

This folder is the dedicated home for Ari standard library documentation. Use
it when writing Ari programs, changing `lib/std.arih`, or adding a new module
under `lib/std/`.

## Start Here

- [Overview](overview.md): library purpose, module map, and design rules.
- [API Reference](api-reference.md): current public APIs grouped by module.
- [Module Guides](modules/README.md): focused notes for individual standard
  library modules.
- [Library Development](library-development.md): how to add or change a
  standard library API.
- [Testing](testing.md): test names, check targets, and coverage expectations.
- [Roadmap](roadmap.md): staged implementation plan and next library families.

## Current Shape

The standard library is ordinary Ari source whenever possible. It lives at
`lib/std.arih`, with child modules in `lib/std/`. User-facing argument,
environment-variable, current-directory, and executable-path helpers now live
in `std::env`, while `std::context` stays the low-level runtime context layer.
`std::process` starts the OS-facing surface with current process id and
explicit exit helpers. Source
collection work includes `std::collections::Set[T]` as a linear explicit-zone
set with insertion-order access, optional access, replace-or-insert updates,
iterator support, and reserve growth; real hash-table `HashMap`/`HashSet`
handles with live-bucket iterators; and red-black-tree `TreeMap`/`TreeSet`
handles with sorted key/value iteration.
A few declarations are still compiler-known because the
current language cannot express them directly: layout queries, typed raw
pointer operations, runtime IO hooks, explicit zone allocation, formatting
macro lowering, and some zone provenance checks.

The rule of thumb is simple: put behavior in Ari source first, and add
compiler support only when the language cannot safely model the primitive yet.
