# std::diag

`std::diag` is a tiny diagnostic value layer for libraries and small tools. It
keeps only the parts that are generally useful at runtime: severity, code,
message, one optional note, and a stable stderr summary writer.

Source-code spans, line maps, labels, fix-its, and rich compiler renderers do
not belong in the production standard library surface. Put those in a future
compiler/tooling package so ordinary programs do not pay for APIs they never
use.

The module is source-only. It does not allocate and stores borrowed
`Slice[u8]` code/message views.

## API

```ari
diag::Severity
diag::Diagnostic

diag::severity_rank(severity)
diag::severity_name(severity)
diag::is_error(severity)
diag::is_warning(severity)

diag::new(severity, code, message)
diag::error(code, message)
diag::warning(code, message)
diag::note(code, message)
diag::help(code, message)
diag::with_note(ref diagnostic, message)
diag::write(ref diagnostic)
```

The same operations are available as methods where that reads better:

```ari
let base = diag::error("E0001", "expected identifier");
let diagnostic = base.with_note("identifiers start with a letter or underscore");

if diag::is_error(diagnostic.severity()) {
  diagnostic.write();
}
```

## Severity Policy

`Severity` has four values:

- `diag::Note`
- `diag::Help`
- `diag::Warning`
- `diag::Error`

`severity_rank` orders them as note, help, warning, error. The rank is for
thresholds and tests, not for serialization compatibility. Use
`severity_name` for user-facing text.

`write(ref diagnostic)` emits a single stderr summary through `std::log`:
`[error] message`, `[warn] message`, or `[info] message` for note/help. If a
diagnostic carries a note, `write` emits the note as a second `[info] note`
line.

## Current Limits

- There is no source span, label, or source-map integration in `std::diag`.
- Only one borrowed note message is stored.
- Messages and codes are borrowed byte slices; owned diagnostic messages should
  use `std::string::String` in a future owned builder.
- JSON, LSP, golden source rendering, and fix-it output belong in
  compiler/tooling packages.

## Tests

- `tests/cases/standard-library/ok/diag/std-diag-basic.ari` checks severity
  helpers, constructors, note attachment, accessors, and method wrappers.
- `tests/cases/standard-library/ok/diag/std-diag-write.ari` checks stable stderr
  summary output.
- `tests/cases/standard-library/ok/diag/std-diag-write-note.ari` checks summary
  output with an attached note line.
- `make check-diag` compiles the focused fixtures, inspects generated symbols,
  and runs the executables.
