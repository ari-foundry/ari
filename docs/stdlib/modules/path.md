# std::path

`std::path` contains source-only lexical path helpers over `Slice[u8]`. It
exists so file and tooling code can split, join, and lightly normalize paths
without opening the filesystem or depending on host state.

This first slice is deliberately POSIX-style: `/` is the only separator.
Windows drive prefixes, UNC paths, and platform-specific normalization belong
to future runtime/path policy work.

## API

```ari
path::is_separator(byte) -> bool
path::is_absolute(path) -> bool
path::is_relative(path) -> bool
path::trim_trailing_separators(path) -> Slice[u8]
path::file_name(path) -> Option[Slice[u8]]
path::parent(path) -> Option[Slice[u8]]
path::extension(path) -> Option[Slice[u8]]
path::stem(path) -> Option[Slice[u8]]
path::join_in(ref mut zone, base, child) -> String
path::normalize_in(ref mut zone, path) -> String
```

Borrowed helpers return views into the original byte slice; they do not
allocate. Keep the source bytes alive while using the returned `Slice[u8]`.

`trim_trailing_separators` removes trailing `/` bytes while preserving root
`/`. `file_name` returns the last component after trimming trailing
separators, or `None` for empty input and root. `parent` returns the borrowed
path before the last separator, with `/usr` returning `/` and a single relative
component returning `None`.

`extension` and `stem` operate on the final component. `.env` has no extension
and its stem is `.env`. `main.ari` has extension `ari` and stem `main`.
Trailing dots do not count as extensions in this first slice.

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
  return path::stem(path_bytes).unwrap_or(path_bytes);
}

fn child_path(zone: ref mut Zone, dir: Slice[u8], name: Slice[u8]) -> String {
  return path::join_in(zone, dir, name);
}
```

## Tests

```text
tests/cases/standard-library/ok/path/std-path-basic.ari
```

The focused test covers separator policy, absolute/relative checks, trailing
separator trimming, final component views, parent/stem/extension behavior,
zone-backed join, and lightweight normalization. `make check-prelude` checks
representative helper symbols and the executable result.

## Future Work

- platform-specific separators and Windows drive/UNC rules
- owned `Path`/`PathBuf` values after the string/path ownership policy is
  stronger
- canonicalization through `std::fs` runtime hooks
- component iterators after iterator protocol ergonomics are stronger
