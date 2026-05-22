# Standard Library Verification Matrix

This page defines the checks that make the standard library feel trustworthy:
API documentation, examples, platform coverage, fuzz/property coverage, and CI
coverage. It is a planning matrix, not a replacement for focused tests.

## Local Checks

| Check | Command | Use When |
| --- | --- | --- |
| Build compiler | `make` | Compiler, runtime, or generated test-runner changes. |
| Public std API drift | `python3 tests/check_std_api_manifest.py` | Any `lib/std.arih` or `lib/std/*.arih` change. |
| Generated std API docs | `python3 tools/generate_std_api_docs.py --check` | Any std API manifest change. |
| Std docs readiness | `python3 tests/check_stdlib_docs.py` | Any `docs/stdlib` change. |
| Language docs readiness | `python3 tests/check_language_docs.py` | Any `docs/language` change. |
| Focused executable or IR test | `build/ari <case>.ari ...` | A small library slice with one behavior under review. |
| Whitespace safety | `git diff --check` | Before committing. |
| Full suite | `make check` | Before broad integration or when explicitly requested. |
| Sanitized suite | `make check-sanitize` | Parser, sema, ownership, or codegen risk when explicitly requested. |

Run the narrowest check that proves the current change first. Save `make check`
and sanitizer checks for larger integration passes or explicit requests.

## Platform Support Matrix

| Target Family | Current Support Reading | Verification Needed Before Calling It Stable |
| --- | --- | --- |
| Linux x86_64 glibc hosted | Primary executable target for local hosted std work. | Compile, link, run, descriptor/socket/thread/time behavior, and errno mapping. |
| Linux x86_64 musl | Target facts and LLVM emission can be checked when toolchains exist. | Link/run matrix, libc differences, errno and dynamic-loader policy. |
| Linux aarch64/riscv64 | Target classification is covered; local cross-run is not assumed. | Cross LLVM/object checks, ABI fixtures, optional emulator or native runner. |
| macOS | Planned hosted platform. | Object/link driver policy, libc/errno mapping, process/fs/net/thread behavior. |
| Windows | Planned hosted platform. | Path/OS string policy, process spawning, sockets, handles, error-code mapping. |
| Freestanding/kernel | Roadmap-only for runtime std. | Startup ABI, panic policy, explicit allocation, atomics, syscalls, and no-libc split. |

Do not write "portable" in module docs unless the matrix says how that API is
checked outside the current host. Prefer "hosted Linux today" or
"platform-specific" when that is the real promise.

## CI Matrix

| Job | Purpose | Minimum Contents |
| --- | --- | --- |
| docs-api | Keep docs and the generated API index in sync. | `python3 tests/check_std_api_manifest.py`, `python3 tools/generate_std_api_docs.py --check`, `python3 tests/check_stdlib_docs.py`. |
| compiler-build | Keep the compiler build healthy. | `make`. |
| focused-stdlib | Exercise the changed std family quickly. | The relevant `build/ari` compile/run or Makefile subtarget. |
| hosted-linux | Prove runtime-backed APIs on the primary platform. | `fs`, `io`, `process`, `thread`, `sync`, `time`, `net`, `env`, `os`. |
| target-facts | Guard cross-target classifications. | x86_64, aarch64, riscv64 Linux target facts and LLVM-only fixtures. |
| sanitizer | Catch memory errors in compiler/runtime changes. | `make check-sanitize` on explicit sanitizer jobs. |
| fuzz-nightly | Search for parser, formatter, codec, and collection invariant bugs. | Seeded fuzz/property runs, minimized corpus, regression fixture export. |

CI should publish the command that failed and the fixture path. A failure that
cannot be reproduced locally should be treated as an infrastructure bug until
the command and seed are visible.

## Fuzz And Property Test Plan

Fuzzing should create small, reproducible fixtures. A failing random case
should become a deterministic test under `tests/cases/`.

| Area | Properties |
| --- | --- |
| Lexer/parser | Tokenization is deterministic, parser errors do not crash, accepted code round-trips through stable summaries. |
| Semantic checker | Invalid ownership, trait, generic, and zone cases fail with diagnostics instead of assertions. |
| Formatting | `format_in!`, `Debug`, and `Display` do not double-evaluate inputs and produce stable bytes for built-ins. |
| Encoding | UTF-8, UTF-16, hex, and base64 decode either round-trip or return a typed error. |
| Parse | Valid strings parse to expected values; invalid strings return `Option`/`Result` absence without partial success. |
| Algorithms | Sort output is ordered, stable sort preserves equal-key order, binary search bounds are consistent, partition results satisfy predicates. |
| Collections | Map/set length, lookup, retain, entry, split/append, and iterator order invariants hold after mixed operations. |
| Vec/String | Drain, splice, truncate, split, append, reserve, shrink, and zone recovery keep live ranges valid. |
| FS/process/net | Temp paths are cleaned, child statuses are typed, sockets return `Error` categories for expected failures. |
| Sync/thread | Atomics obey documented ordering, once/once-lock run once, channels and barriers preserve state invariants. |

## Documentation Review Checklist

Before committing docs, re-read the changed pages and check:

- API names match generated spellings.
- Examples use features that exist today.
- Stable, usable, platform-backed, platform-specific, and experimental claims
  match [stability.md](stability.md).
- Platform notes say Linux/macOS/Windows/freestanding support precisely.
- Error examples prefer `Result[..., Error]` and avoid teaching raw `i64`
  compatibility as the normal path.
- Allocation examples describe zones and do not reintroduce hidden global heap
  language.
- Future work is named as roadmap work, not as already-supported behavior.
