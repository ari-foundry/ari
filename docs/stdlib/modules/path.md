# std::path

`std::path` contains source-only lexical path helpers over `Slice[u8]`. It
exists so file and tooling code can split, join, and lightly normalize paths
without opening the filesystem or depending on host state.

This first slice is deliberately POSIX-style: `/` is the only separator.
Windows drive prefixes, UNC paths, and platform-specific normalization belong
to future runtime/path policy work. Filesystem-backed existing-path
canonicalization lives in `std::fs::try_canonicalize`.

## API

```ari
path::PathBytes
path::bytes(path) -> PathBytes
path::from_os(os) -> PathBytes
path::is_separator(value: char) -> bool
path::is_absolute(path) -> bool
path::is_relative(path) -> bool
path::trim_trailing_separators(path) -> Slice[u8]
path::components(path) -> Components
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
path::normalize_in(ref mut zone, path) -> String
```

`is_separator` takes `char` because it checks Ari's ASCII byte character
literal spelling such as `'/'`; `char` is the root alias for `u8`.

`PathBytes` is the typed borrowed view for bytes that should be interpreted as
a path. It keeps path logic out of generic byte strings and out of OS string
handling:

```ari
let path = path::bytes(bytes);
let from_os = path::from_os(os);
let literal_path: std::path::PathBytes = "/tmp/ari";
path.as_slice()
path.len()
path.is_empty()
path.is_absolute()
path.is_relative()
path.components()
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
path.normalize_in(ref mut zone)
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

`trim_trailing_separators` removes trailing `/` bytes while preserving root
`/`. `file_name` returns the last component after trimming trailing
separators, or `None` for empty input and root. `parent` returns the borrowed
path before the last separator, with `/usr` returning `/` and a single relative
component returning `None`.

`components(path)` returns an iterator over non-empty lexical components. It
skips leading, repeated, and trailing `/` separators. Root-only `/` and empty
paths produce no components. It does not normalize `.` or `..`; callers that
need lightweight lexical cleanup should call `normalize_in` first.

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

`with_file_name_in` and `with_extension_in` allocate a new owned
`std::string::String` in the provided zone. `with_file_name_in` keeps the
current parent when one exists, preserves root for paths such as `/`, and
otherwise returns a copy of the new final component. `with_extension_in`
replaces the final extension, appends one when the path has no extension, and
removes the extension when `new_extension` is empty. Paths without a final
component, such as `/`, are copied unchanged by `with_extension_in`.

`join_in` copies into the caller-provided zone. If `child` is absolute, it
returns a copy of `child`. Otherwise it inserts one `/` between `base` and
`child` when needed.

`normalize_in` is lightweight lexical normalization. It collapses repeated
separators and removes `.` components, but intentionally keeps `..`
components. Removing `..` safely requires a stronger policy around roots,
symlinks, and platform behavior.

## Example

```ari
fn module_name(path_bytes: Slice[u8]) -> Slice[u8] {
  return path::file_stem(path_bytes).unwrap_or(path_bytes);
}

fn child_path(zone: ref mut Zone, dir: Slice[u8], name: Slice[u8]) -> String {
  return path::join_in(zone, dir, name);
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

## Tests

```text
tests/cases/standard-library/ok/path/std-path-basic.ari
tests/cases/standard-library/ok/path/std-path-components.ari
tests/cases/standard-library/ok/path/std-path-bytes.ari
tests/cases/standard-library/ok/path/std-path-predicates.ari
tests/cases/standard-library/ok/path/std-path-affixes.ari
tests/cases/standard-library/ok/path/std-path-edit.ari
```

The focused test covers separator policy, absolute/relative checks, trailing
separator trimming, final component views, parent/stem/extension behavior,
explicit `file_stem` aliases, zone-backed path editing, zone-backed join,
lightweight normalization, component iteration, typed `PathBytes` views, and
allocation-free final-component predicates plus component-aware path
prefix/suffix predicates and stripping.
`make check-prelude` checks representative helper symbols and executable
results.

## Future Work

- platform-specific separators and Windows drive/UNC rules
- owned `Path`/`PathBuf` values after the string/path ownership policy is
  stronger
- deeper integration with `std::fs::try_canonicalize` once owned `PathBuf`
  exists
- richer component kinds such as root, current directory, and parent directory
  if Ari later adds an owned `Path` model
