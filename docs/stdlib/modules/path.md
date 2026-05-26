# std::path

`std::path` contains source-only lexical path helpers over path byte slices. It
exists so file and tooling code can split, join, lightly normalize, and classify
paths without opening the filesystem or depending on host state.

This first slice keeps hosted Linux/POSIX-style `/` behavior as the default
for normal path operations, and paths are byte strings, not validated UTF-8
strings. Ari's zone-backed `std::string::String` is also byte-oriented, while
`std::string::Utf8` is the explicit validated UTF-8 view. `std::string::OsStr`
is the borrowed OS-boundary byte view, and `PathBytes` is the borrowed
path-policy view over those bytes. `PathBuf` is the owned POSIX path buffer:
it is a distinct path type whose storage is an internal zone-backed
`std::string::String`.

Explicit `windows_*` helpers classify Windows drive prefixes, rooted drive
paths, and UNC prefixes lexically without changing the default POSIX
join/component/normalization policy. Verbatim paths, platform-specific
normalization, and target-specific owned path representations remain future
runtime/path policy work.
Filesystem-backed existing-path canonicalization lives in
`std::fs::canonicalize`; this module stays lexical unless a helper explicitly
says it reads hosted process state, such as `current_dir_join`.

Lexical helpers preserve interior NUL bytes because they work on `Slice[u8]`.
Hosted filesystem and C-string boundaries cannot safely pass such paths, so use
`path::contains_nul` before crossing into OS APIs when the bytes are not known
to be clean.

## Policy

Use these rules when choosing or extending path APIs:

| Value | Meaning | Ownership |
| --- | --- | --- |
| `Slice[u8]` | Generic borrowed bytes with no path policy. | Borrowed from caller. |
| `std::string::String` | Owned byte string, not guaranteed UTF-8. | Zone-backed owner. |
| `std::string::Utf8` | Borrowed bytes already validated as UTF-8. | Borrowed view. |
| `std::string::OsStr` | Borrowed host OS bytes. | Borrowed view. |
| `std::path::PathBytes` / `Path` | Borrowed bytes interpreted with POSIX lexical path rules. | Borrowed view. |
| `std::path::PathBuf` | Owned POSIX path bytes with path-specific methods. | Zone-backed owner wrapping an internal `String`. |

`String` and `PathBuf` are byte buffers. Do not assume UTF-8 unless the caller
validates through `std::string::Utf8` or `std::encoding`. `PathBytes` and
`PathBuf` do not reject NUL; they make NUL visible through `contains_nul` so
callers can reject it before `std::fs`, `std::c`, or other hosted boundaries.

Most path helpers are infallible because they only inspect or copy bytes.
Helpers that allocate take `ref mut Zone`. The only Result-returning helper in
this module today is `current_dir_join`, because reading the current directory
can fail before the lexical join happens.

## Choosing APIs

| Task | Preferred API | Notes |
| --- | --- | --- |
| Treat bytes as a path | `path::bytes(bytes)` or direct literal coercion to `PathBytes` | No allocation; keeps the original bytes borrowed. |
| Convert from OS boundary data | `path::from_os(os)` | Keeps bytes borrowed from `OsStr`; no UTF-8 validation. |
| Own a path | `path::from_bytes(ref mut zone, bytes)` or `path::from_string(ref mut zone, text)` | Copies bytes into the zone and returns `PathBuf`. |
| Copy a borrowed path to owned text | `path::to_string(ref mut zone, path)` or `path_buf.to_string(ref mut zone)` | Still byte-oriented text, not validated UTF-8. |
| Inspect components | `components`, `components_with_kinds`, `file_name`, `parent`, `extension`, `stem`, `file_stem` | Borrowed slices point into the original path; kinded components preserve root, `.`, and `..`. |
| Compare path prefixes/suffixes | `starts_with`, `strip_prefix`, `ends_with`, `strip_suffix` | Component-aware: `src` matches `src/main.ari` but not `src2/main.ari`. |
| Join paths | `path::join(ref mut zone, base, child)`, `base.join(ref mut zone, child)`, or `join_many` | Returns `PathBuf`; absolute children replace the accumulated base. |
| Join against the process cwd | `path::current_dir_join(ref mut zone, child)` | Returns `Result[PathBuf, Error]` because cwd lookup can fail. |
| Lexically clean a path | `normalize_in(ref mut zone, path)` or `path.normalize(ref mut zone)` | Collapses repeated separators and `.`, keeps `..`. |
| Check OS-boundary safety | `contains_nul` | Lexical helpers allow NUL; hosted boundaries should reject it. |
| Inspect Windows-shaped bytes | `is_windows_absolute`, `windows_drive`, `windows_unc_prefix` | Opt-in lexical classifiers; they do not affect POSIX default helpers. |

## API

```ari
path::Path
path::PathBytes
path::PathBuf
path::Component
path::ComponentKind
path::bytes(path) -> PathBytes
path::from_os(os) -> PathBytes
path::from_bytes(ref mut zone, path) -> PathBuf
path::from_string(ref mut zone, text) -> PathBuf
path::to_string(ref mut zone, path) -> String
path::is_separator(value: char) -> bool
path::is_empty(path) -> bool
path::contains_nul(path) -> bool
path::as_bytes(path) -> Slice[u8]
path::is_absolute(path) -> bool
path::is_relative(path) -> bool
path::is_windows_separator(value: char) -> bool
path::is_windows_absolute(path) -> bool
path::has_windows_drive_prefix(path) -> bool
path::windows_drive(path) -> Option[Slice[u8]]
path::is_windows_drive_absolute(path) -> bool
path::is_windows_drive_relative(path) -> bool
path::is_windows_unc(path) -> bool
path::windows_unc_prefix(path) -> Option[Slice[u8]]
path::trim_trailing_separators(path) -> Slice[u8]
path::components(path) -> Components
path::components_with_kinds(path) -> ComponentsWithKinds
component.kind() -> ComponentKind
component.as_slice() -> Slice[u8]
component.is_root() -> bool
component.is_current() -> bool
component.is_parent() -> bool
component.is_normal() -> bool
path::file_name(path) -> Option[Slice[u8]]
path::parent(path) -> Option[Slice[u8]]
path::extension(path) -> Option[Slice[u8]]
path::stem(path) -> Option[Slice[u8]]
path::file_stem(path) -> Option[Slice[u8]]
path::has_file_name(path, expected) -> bool
path::has_extension(path, expected) -> bool
path::has_stem(path, expected) -> bool
path::has_file_stem(path, expected) -> bool
path::starts_with(path, prefix) -> bool
path::strip_prefix(path, prefix) -> Option[Slice[u8]]
path::ends_with(path, suffix) -> bool
path::strip_suffix(path, suffix) -> Option[Slice[u8]]
path::with_file_name_in(ref mut zone, path, new_file_name) -> String
path::with_extension_in(ref mut zone, path, new_extension) -> String
path::join_in(ref mut zone, base, child) -> String
path::join(ref mut zone, base, child) -> PathBuf
path::join_many(ref mut zone, parts) -> PathBuf
path::current_dir_join(ref mut zone, child) -> Result[PathBuf, Error]
path::normalize_in(ref mut zone, path) -> String
path_buf.as_string() -> ref String
path_buf.as_bytes() -> Slice[u8]
path_buf.as_path() -> PathBytes
path_buf.to_string(ref mut zone) -> String
path_buf.len() -> i64
path_buf.is_empty() -> bool
```

`is_separator` takes `char` because it checks Ari's ASCII byte character
literal spelling such as `'/'`; `char` is the root alias for `u8`.

`Path` is a readability alias for `PathBytes`. `PathBytes` is the typed
borrowed view for bytes that should be interpreted as a path. It keeps path
logic out of generic byte strings and out of OS string handling:

```ari
let path = path::bytes(bytes);
let from_os = path::from_os(os);
let literal_path: std::path::Path = "/tmp/ari";
path.as_slice()
path.as_bytes()
path.to_string(ref mut zone)
path.len()
path.is_empty()
path.contains_nul()
path.is_absolute()
path.is_relative()
path.is_windows_absolute()
path.has_windows_drive_prefix()
path.windows_drive()
path.is_windows_drive_absolute()
path.is_windows_drive_relative()
path.is_windows_unc()
path.windows_unc_prefix()
path.components()
path.components_with_kinds()
path.file_name()
path.parent()
path.extension()
path.stem()
path.file_stem()
path.has_file_name(expected)
path.has_extension(expected)
path.has_stem(expected)
path.has_file_stem(expected)
path.starts_with(prefix)
path.strip_prefix(prefix)
path.ends_with(suffix)
path.strip_suffix(suffix)
path.with_file_name_in(ref mut zone, new_file_name)
path.with_extension_in(ref mut zone, new_extension)
path.join_in(ref mut zone, child)
path.join(ref mut zone, child)
path.normalize_in(ref mut zone)
path.normalize(ref mut zone)
```

Borrowed helpers return views into the original byte slice; they do not
allocate. Keep the source bytes alive while using returned `Slice[u8]` values
or a `Components` iterator. `PathBytes` methods are wrappers over the module
functions, so their boundary behavior is the same.
When a `PathBytes` value is expected, a string literal can be used directly;
the literal lowers to a borrowed path-byte view without requiring
`path::bytes("...")`. Literal receivers also work for path-specific methods
whose names do not overlap with generic byte-slice helpers:

```ari
"/tmp/bin".file_name()
"src/main.ari".extension()
"/tmp".join_in(ref mut zone, "bin")
```

`PathBuf` is the current owned POSIX path buffer. It is a distinct struct, not
a `String` type alias, so APIs can require an owned path without accepting every
byte string by accident. Internally it stores a zone-backed `String`; the bytes
live in the zone used to create the buffer and remain valid until that zone is
reset or destroyed. `PathBuf` does not validate UTF-8. Use `as_string` when a
caller explicitly needs the internal byte string view, `as_bytes` for a
borrowed slice, and `as_path` for a borrowed path-policy view:

```ari
let owned = path::from_string(ref mut zone, "src/main.ari");
owned.as_string()
owned.as_bytes()
owned.as_path()
owned.to_string(ref mut zone)
owned.len()
owned.is_empty()
owned.contains_nul()
owned.is_absolute()
owned.is_relative()
owned.is_windows_absolute()
owned.has_windows_drive_prefix()
owned.windows_drive()
owned.is_windows_drive_absolute()
owned.is_windows_drive_relative()
owned.is_windows_unc()
owned.windows_unc_prefix()
owned.components()
owned.components_with_kinds()
owned.file_name()
owned.parent()
owned.extension()
owned.stem()
owned.file_stem()
owned.join(ref mut zone, "cache")
owned.normalize(ref mut zone)
owned.with_file_name(ref mut zone, "lib.ari")
owned.with_extension(ref mut zone, "o")
```

`from_bytes` and `from_string` copy into the caller-provided zone. `to_string`
copies a borrowed path view into an owned byte string. `as_bytes` returns a
borrowed view and does not allocate.

`trim_trailing_separators` removes trailing `/` bytes while preserving root
`/`. `file_name` returns the last component after trimming trailing
separators, or `None` for empty input and root. `parent` returns the borrowed
path before the last separator, with `/usr` returning `/` and a single relative
component returning `None`.

`components(path)` returns an iterator over non-empty lexical components. It
skips leading, repeated, and trailing `/` separators. Root-only `/` and empty
paths produce no components. It does not normalize `.` or `..`; callers that
need lightweight lexical cleanup should call `normalize_in` first.

`components_with_kinds(path)` returns `Component` values. It yields a single
`RootDir` component for leading `/`, skips repeated separators, and then yields
`CurrentDir` for `.`, `ParentDir` for `..`, and `Normal` for every other
component. `Component::as_slice()` returns the borrowed bytes for the component:
`/` for `RootDir`, `.` for `CurrentDir`, `..` for `ParentDir`, or the normal
component bytes. Use `is_root`, `is_current`, `is_parent`, and `is_normal` for
branching without matching on the enum directly. `components_with_kinds` is the
path parser to use when a caller needs to preserve lexical meaning instead of
just the raw non-empty names.

`extension` and `stem` operate on the final component. `file_stem` is the same
operation as `stem`, kept under the more explicit name many path APIs use.
`.env` has no extension and its stem is `.env`. `main.ari` has extension `ari`
and stem `main`. Trailing dots do not count as extensions in this first slice.

`has_file_name`, `has_extension`, `has_stem`, and `has_file_stem` compare the
borrowed view helpers against an expected byte slice without allocating. They
return `false` when the corresponding view helper would return `None`.
`has_extension` expects only the extension bytes, not a leading dot.

`starts_with`, `strip_prefix`, `ends_with`, and `strip_suffix` are
component-aware path affix helpers. They trim trailing separators before
matching and require a path component boundary, so `src` matches
`src/main.ari` but not `src2/main.ari`. A root prefix `/` matches absolute
paths and strips to the relative remainder. Strip helpers return borrowed views
into the trimmed input path and return `None` when the affix is absent.

`with_file_name_in`, `with_extension_in`, `join_in`, and `normalize_in` are the
compatibility/string-returning allocation helpers. The natural owned-path
wrappers are `PathBytes::join`, `PathBytes::normalize`, `PathBuf::join`,
`PathBuf::normalize`, `PathBuf::with_file_name`, and
`PathBuf::with_extension`; they return `PathBuf`.

`join` copies into the caller-provided zone. If `child` is absolute, it returns
a copy of `child`. Otherwise it inserts one `/` between `base` and `child` when
needed. `join_many` skips empty parts and resets the accumulated path whenever
it sees an absolute part.

`current_dir_join` reads the hosted current directory with
`std::env::current_dir_path()` and returns `Result[PathBuf, Error]`, because
the current-directory lookup can fail. The join itself is still lexical.

`normalize_in` is lightweight lexical normalization. It collapses repeated
separators and removes `.` components, but intentionally keeps `..`
components. Removing `..` safely requires a stronger policy around roots,
symlinks, and platform behavior.

The Windows helpers are explicit opt-in byte classifiers. They accept both `/`
and byte value `92u8` (backslash) as Windows separators, but they do not
rewrite or normalize separators. `has_windows_drive_prefix(path)` recognizes ASCII letter
drive prefixes such as `C:`. `windows_drive(path)` returns that two-byte prefix
as a borrowed slice. `is_windows_drive_absolute(path)` requires a drive prefix
followed by a Windows separator, for example `C:/ari`; `is_windows_drive_relative`
recognizes drive-relative paths such as `C:ari`. `is_windows_unc(path)` requires
two leading Windows separators followed by non-empty server and share
components. `windows_unc_prefix(path)` returns the borrowed
`//server/share` prefix or the byte-equivalent backslash form.
`is_windows_absolute(path)` is
true for drive-absolute paths, UNC paths, and single-rooted Windows paths such
as `/temp`; it is false for drive-relative paths like `C:temp`.

## Examples

Collect arix-style command paths without hand-written byte plumbing:

```ari
fn manifest_path(zone: ref mut Zone) -> std::Result[std::path::PathBuf, std::error::Error] {
  return path::current_dir_join(zone, "Ari.toml");
}

fn cache_prefix(zone: ref mut Zone, home: std::path::Path) -> std::path::PathBuf {
  return home.join(zone, ".ari");
}
```

Build a target path from several path parts:

```ari
fn target_binary(zone: ref mut Zone, package: std::path::Path) -> std::path::PathBuf {
  var parts: [std::path::PathBytes, 4] = [
    "target",
    "debug",
    package,
    "main"
  ];
  return path::join_many(zone, parts.as_slice());
}
```

Reject NUL before a hosted filesystem call:

```ari
fn safe_to_open(path_bytes: std::path::Path) -> bool {
  if path_bytes.contains_nul() {
    return false;
  }
  return !path_bytes.is_empty();
}
```

Use borrowed component views without allocation:

```ari
fn module_name(path_bytes: Slice[u8]) -> Slice[u8] {
  return path::file_stem(path_bytes).unwrap_or(path_bytes);
}
```

Use owned path edits when the result must outlive the input view:

```ari
fn child_path(zone: ref mut Zone, dir: Slice[u8], name: Slice[u8]) -> String {
  return path::join_in(zone, dir, name);
}

fn owned_child_path(zone: ref mut Zone, dir: std::path::Path, name: std::path::Path) -> std::path::PathBuf {
  return path::join(zone, dir, name);
}

fn ari_to_object(zone: ref mut Zone, source: Slice[u8]) -> String {
  return path::with_extension_in(zone, source, "o");
}

fn score(path_bytes: Slice[u8]) -> i64 {
  var total = 0;
  var parts = path::components(path_bytes);
  for part in ref mut parts {
    total = total + part.len;
  }
  return total;
}
```

## Development Notes

Use this section when adding or reviewing path helpers.

- Keep pure lexical helpers source-only and filesystem-free. They should not
  call `std::fs`, `std::env`, or C/OS hooks unless their name explicitly says
  they read hosted state.
- Keep borrowed helpers allocation-free. Return `Slice[u8]`, `Option[Slice[u8]]`,
  or iterators whose slices point into the original input.
- Put allocating helpers behind an explicit `ref mut Zone`. Use `_in` when the
  historical return shape is a `String`; use natural owned-path wrappers such
  as `join`, `join_many`, `normalize`, `with_file_name`, and `with_extension`
  for `PathBuf`.
- Preserve the POSIX byte policy until a platform path model exists. Do not add
  Windows behavior to default helpers by guessing; keep it behind explicit
  `windows_*` helpers.
- Preserve NUL bytes lexically. Add explicit boundary validation near OS or C
  calls instead of making lexical path parsing silently reject bytes.
- Keep `normalize_in` conservative. It may collapse repeated `/` and remove
  `.` components, but it must not erase `..` until Ari has a symlink/root-aware
  normalization policy.
- Prefer `PathBytes`/`Path` in APIs when the value should be interpreted as a
  path. Prefer raw `Slice[u8]` when the helper is genuinely byte-generic.
- Treat `PathBuf` as an opaque owned path. Convert through `as_bytes`,
  `as_path`, `as_string`, or `to_string` instead of relying on its field layout.

## Tests

```text
tests/cases/standard-library/ok/path/std-path-basic.ari
tests/cases/standard-library/ok/path/std-path-components.ari
tests/cases/standard-library/ok/path/std-path-bytes.ari
tests/cases/standard-library/ok/path/std-path-predicates.ari
tests/cases/standard-library/ok/path/std-path-affixes.ari
tests/cases/standard-library/ok/path/std-path-edit.ari
tests/cases/standard-library/ok/path/std-path-buf.ari
tests/cases/standard-library/ok/path/std-path-windows-lexical.ari
```

The focused test covers separator policy, absolute/relative checks, trailing
separator trimming, final component views, parent/stem/extension behavior,
explicit `file_stem` aliases, zone-backed path editing, zone-backed join,
lightweight normalization, plain and kinded component iteration, typed `PathBytes` views, and
allocation-free final-component predicates plus component-aware path
prefix/suffix predicates and stripping. It also covers `Path`/`PathBuf`,
owned path construction from strings and bytes, byte access, NUL detection,
multi-part joins, current-directory joins, and `PathBuf` editing wrappers.
`std-path-windows-lexical.ari` covers explicit Windows drive and UNC
classifiers without changing POSIX default path semantics.
`make check-prelude` checks representative helper symbols and executable
results.

## Future Work

- platform-specific separator behavior and verbatim Windows path rules
- richer platform-owned path representations if Ari later separates POSIX byte
  paths from target OS strings
- deeper integration with `std::fs::canonicalize` and other filesystem APIs as
  more of them accept path-byte values directly
