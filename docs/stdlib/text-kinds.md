# Text And Path Kinds

Ari deliberately separates four byte-like values that many languages blur
together. Use this page when deciding which API a library should accept.

| Kind | Current Type | Meaning | Use It For |
| --- | --- | --- | --- |
| C string | `string`, `std::string::CStr`, `std::c::CStr`, `std::c::CString` | NUL-terminated bytes. Borrowed views exclude the terminator when viewed as a slice; owned `CString` stores one trailing NUL in a zone. | String literals, C ABI calls, dynamic loader names, and C-shaped owned buffers. |
| Owned byte string | `std::string::String` | Zone-backed mutable bytes with no UTF-8 promise. | Output buffers, parser buffers, byte-oriented file reads, formatting results. |
| UTF-8 string view | `std::string::Utf8` | Borrowed bytes that have already passed UTF-8 validation. | Unicode scalar counting and byte-offset scalar lookup. |
| OS string view | `std::string::OsStr` | Borrowed operating-system string bytes. On the current POSIX slice this is raw bytes and may not be UTF-8. | Environment/path boundary data before deciding whether it is text. |
| Path bytes | `std::path::PathBytes` | Borrowed bytes interpreted as a lexical path. | Splitting, joining, normalizing, and component inspection. |

## Rules Of Thumb

- Do not treat `std::string::String` as validated human text. It stores bytes.
- Validate with `std::string::utf8(bytes)` before using Unicode scalar APIs.
- Keep OS data as `OsStr` until the caller chooses either bytes, UTF-8, or a
  path interpretation.
- Prefer `std::env`'s `*_os` helpers for arguments, environment variables, and
  executable paths when byte-preserving OS-boundary behavior matters.
- Prefer `std::env::current_dir_path()` or `try_current_dir_path()` when the
  current directory will immediately be inspected as a path.
- Keep path manipulation in `std::path`; path bytes are not normal text even
  when they happen to be UTF-8.
- Use `std::string::c_str(text)` for the older text-kind view in the string
  module, and `std::c::from_string(text)` when the value is headed to C ABI
  helpers. Use `std::c::from_slice_in(ref mut zone, bytes)` when owned
  NUL-terminated storage is required.
- Use `cstr.as_slice()` or `std::string::c_bytes(text)` when byte helpers
  should ignore the trailing NUL.

## Current Limits

The first slice now includes `std::c::CString` for owned NUL-terminated byte
storage, but fallible construction and direct arbitrary C-call handoff for
zone-backed buffers still need explicit FFI escape policy. Owned `Utf8String`,
`OsString`, and `PathBuf` values remain roadmap work after owned path buffers,
fallible owned decoders, and OS-resource error policy are stronger.

Windows OS-string and path semantics are also future work. Today `OsStr` and
`PathBytes` use byte slices because the current runtime target is POSIX-style.
