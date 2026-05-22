# Core Language Tests

This folder locks down Ari's current executable core language surface. Keep
these fixtures small and boring: they should exercise existing syntax and
diagnostics across functions, locals, operators, and control flow without
pulling in stdlib containers, ownership-heavy features, or new language design.

- `ok/` contains runtime smoke programs for the whole core surface.
- `errors/` contains stable diagnostics for core mistakes not already owned by
  a narrower feature folder.

Run `make check-core-language` while iterating here.
