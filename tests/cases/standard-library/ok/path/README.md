# Standard Library Path Tests

These cases cover source-only lexical path manipulation in `std::path`.

Current files:

- `std-path-basic.ari`: separator policy, absolute/relative checks,
  trailing-separator trimming, file-name/parent/stem/extension views, join,
  and lightweight normalization.
- `std-path-components.ari`: borrowed component iteration over absolute paths,
  repeated separators, trailing separators, and root-only paths.
