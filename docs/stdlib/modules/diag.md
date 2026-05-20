# std::diag

`std::diag` provides the first shared diagnostic vocabulary for Ari-written
compiler tools. It is intentionally small: a diagnostic has a severity, a code,
a message, a primary source span, and at most one attached label. That is
enough for lexer/parser pilots to stop passing loose tuples around while the
owned source-map and rich renderer are still roadmap work.

The module is source-only. It does not allocate and it stores borrowed
`Slice[u8]` message/code views.

## API

```ari
diag::Severity
diag::LabelStyle
diag::Label
diag::Diagnostic

diag::severity_rank(severity)
diag::severity_name(severity)
diag::is_error(severity)
diag::is_warning(severity)

diag::label(style, span, message)
diag::primary(span, message)
diag::secondary(span, message)

diag::new(severity, code, message, span)
diag::error(code, message, span)
diag::warning(code, message, span)
diag::note(code, message, span)
diag::help(code, message, span)
diag::with_label(ref diagnostic, label)
diag::location(ref diagnostic, ref source_file)
diag::label_location(ref label, ref source_file)
diag::write(ref diagnostic)
```

The same operations are available as methods where that reads better:

```ari
let file = source::file_id(1);
let input = source::file(file, "let = 1\n");
let span = input.line_span(1);

let base = diag::error("E0001", "expected identifier", span);
let diagnostic = base.with_label(diag::primary(span, "name goes here"));

let place = diagnostic.location(ref input);
if place.line() == 1 {
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

`write(ref diagnostic)` currently emits a single stderr summary through
`std::log`: `[error] message`, `[warn] message`, or `[info] message` for note
and help. It is a temporary stable summary for small tools, not the final
source-code renderer.

## Source Locations

`Diagnostic::location(ref source_file)` and
`Label::location(ref source_file)` use `std::source::SourceFile::locate` at the
start of the stored span. They assert that the span file id matches the
borrowed source file id.

Spans remain byte ranges. A diagnostic renderer that wants Unicode-aware caret
widths must decode UTF-8 separately through `std::encoding`.

## Current Limits

- Only one attached label is stored.
- Notes, related spans, fix-it edits, and multi-line rendering are future work.
- Messages and codes are borrowed byte slices; owned diagnostic messages should
  use `std::string::String` in a future owned builder.
- There is no JSON renderer yet. LSP/lint tooling still uses host-side
  diagnostic conversion.

## Tests

- `tests/cases/standard-library/ok/diag/std-diag-basic.ari` checks severity
  helpers, label construction, label attachment, source location lookup, and
  method wrappers.
- `tests/cases/standard-library/ok/diag/std-diag-write.ari` checks the first
  stable stderr summary output.
- `make check-diag` compiles the focused fixtures, inspects generated symbols,
  and runs the executables.
