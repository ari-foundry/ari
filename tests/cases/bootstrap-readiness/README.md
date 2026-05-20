# Bootstrap Readiness Tests

These fixtures prove that normal Ari code can express compiler-shaped data
before the real `bootstrap/` tree exists.

Keep files grouped by the pressure point they exercise:

- `ok/model/`: token, span, syntax, symbol, and diagnostic data shapes.
- `ok/errors/`: `Result[T, E]`-style expected compiler failures.
- `ok/formatting/`: deterministic text output for compiler artifacts.
- `errors/`: rejected programs with diagnostics that matter for compiler-scale
  code.
- `golden/`: future checked text outputs for token, syntax, report, HIR, and
  IR artifacts.

These are not bootstrap-only programs. If a fixture needs awkward syntax, fix
the normal Ari language or compiler surface instead of adding a private stage1
shortcut.
