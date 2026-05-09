# Modules

## Inline Modules

Inline modules create namespaces:

```ari
mod Math {
  pub fn add(left: i64, right: i64) -> i64 {
    return left + right
  }
}
```

The item above has the full path:

```ari
Math::add
```

## Qualified Paths

```ari
fn main() -> i64 {
  return Math::add(2, 3)
}
```

`A::B` paths can currently name constants, functions, structs, enums, traits,
enum case constructors, and associated functions such as `Point::new`.

## File Modules

`mod name;` loads a sibling module file and places its declarations under the
`name` namespace:

```ari
mod mathlib;

fn main() -> i64 {
  return mathlib::add(20, 2)
}
```

The compiler first searches relative to the importing file:

```text
mathlib.ari
mathlib.arih
mathlib/mod.ari
mathlib/mod.arih
```

Package search paths can be added from the command line:

```sh
ari app.ari --module-path packages
ari app.ari -I packages
ari app.ari -Ipackages
```

For each `mod name;`, Ari checks the importing file's directory first, then
each module search path in command-line order using the same four candidate
forms. Loaded module names are cached during one compiler run, so repeated
imports of the same module path are resolved once and cycles are diagnosed
before parsing loops indefinitely.

The standard library prelude is special-cased before general library loading:
when `lib/std.arih` exists, the compiler auto-loads it as the public `std`
module. User code can write `std::io::write_i64` or
`use std::mem::size_of;` without declaring `mod std;`. Rust-like implicit
prelude aliases also bring the public `std` root items and root re-exports into
ordinary module scopes, so names such as `Vec`, `Range`, `Iterator`, `range`,
`size_of`, `write_i64`, `create`, and `new` can be used without `std::`.
Local declarations or explicit `use` aliases take priority. If code wants a
namespace handle for clarity or collision management, write `use std as core`
and call through `core::...`. Other library modules still require normal
`mod name;` declarations for now.

Use `--no-implicit-std` to disable that special case. Then `std` behaves like
any other file-backed module, so code must declare `mod std;` and pass a search
path such as `--module-path lib` if it wants the repository `lib/std.arih`
header.

`.arih` is Ari's header-like source surface. It is parsed like normal Ari, but
the convention is to keep declarations, extern bindings, ABI declarations, and
small public module surfaces there. Ari's own `lib/std.arih` uses
`extern "ari"` for compiler/runtime builtin declarations; C libraries should
still use `extern "C"`.

Example header module:

```ari
pub extern "C" fn puts(text: string) -> i64 = "puts";
```

Then:

```ari
mod libc;

fn main() -> i64 {
  libc::puts("hello")
  return 0
}
```

## Visibility

Items inside a module are private to that module unless marked `pub`:

```ari
mod Secret {
  fn value() -> i64 {
    return 1
  }

  pub fn visible() -> i64 {
    return value()
  }
}
```

Code outside `Secret` can call `Secret::visible`, but cannot call
`Secret::value`.

The same rule applies to structs, enums, traits, and impl methods. `pub struct`,
`pub enum`, and `pub trait` expose the type-level name. Impl methods can be
made public one at a time, or by marking the whole impl block public:

```ari
mod Geometry {
  pub struct Point {
    x: i64,
  }

  pub impl Point {
    fn new(x: i64) -> Point {
      Point { x: x }
    }
  }
}

let point = Geometry::Point::new(7)
```

Nested modules also have visibility. A private nested module can be used by its
parent module, while code outside that parent must go through a `pub mod`:

```ari
mod Api {
  pub mod V1 {
    pub fn value() -> i64 {
      return 1
    }
  }
}

fn main() -> i64 {
  return Api::V1::value()
}
```

Root-level modules are visible within the current package. `pub mod` matters
once a module is nested under another module path.

## Same-Module Names

Inside the same module, names can be used without qualification:

```ari
mod Math {
  fn double(value: i64) -> i64 {
    return add(value, value)
  }

  pub fn add(left: i64, right: i64) -> i64 {
    return left + right
  }
}
```

## Use

`use` imports a qualified item under its final path segment:

```ari
use Math::add

fn main() -> i64 {
  return add(2, 3)
}
```

Aliases are explicit:

```ari
use Math::add as sum
```

Several items can be imported from the same module with a use group:

```ari
use Math::{add, sub as minus}
```

`*` imports all public items from the target module into the current module
scope, including constants:

```ari
use Math::*
```

Module paths can be aliased too:

```ari
use Very::Long::Module as Short

fn main() -> i64 {
  return Short::value()
}
```

`use` is scoped to the module where it appears. A use inside one module does
not leak into the root module or sibling modules.

`pub use` re-exports an alias through the containing module:

```ari
mod Core {
  pub fn value() -> i64 {
    return 5
  }
}

mod Api {
  pub use Core::value as answer
}

fn main() -> i64 {
  return Api::answer()
}
```

Public glob re-exports are also supported:

```ari
mod Api {
  pub use Core::*
}
```

File-backed modules can be loaded under a different module name:

```ari
mod mathlib as Math;
```

The compiler searches for `mathlib.ari`, `mathlib.arih`, `mathlib/mod.ari`, or
`mathlib/mod.arih` in the importing file's directory and then in package search
paths, then places the loaded declarations under `Math`.

## Module Metadata

Ari can emit a compact module graph summary for file-backed package work:

```sh
ari app.ari -I packages --emit-module-metadata build/app.arimeta --emit-llvm build/app.ll
```

The metadata file records the module search paths, active cfg features, source
files with stable content hashes, resolved file-backed imports, and declaration
names seen in each source file. It is intentionally a summary format; use a
module cache when you want to reuse a validated package source snapshot.
Current metadata is written as `ari-module-metadata-v2`; older v1 summaries can
be parsed for diagnostics, but `--check-module-metadata` asks you to regenerate
them because v1 lacks source content hashes.
Malformed summaries that repeat an exact source, import, or item record are
rejected when read.

Use `--check-module-metadata` to read an existing summary and verify that the
current source graph still matches it:

```sh
ari app.ari -I packages --check-module-metadata build/app.arimeta --emit-llvm build/app.ll
```

This is the validation layer for later package caching. If source contents,
search paths, cfg features, imports, or declarations change, regenerate the
metadata.
Stale metadata diagnostics name the first changed input class they can identify:
search path, cfg feature, implicit standard-library option, source file, resolved
import, declaration item, or source content hash. That keeps package-cache
failures tied to the module, import, item, or source file that actually changed
instead of a generic cache miss.

## Module Cache

Ari can also write a source-snapshot module cache:

```sh
ari app.ari -I packages --emit-module-cache build/app.aricache --emit-llvm build/app.ll
```

The cache embeds the same metadata summary, the source text for every file in
the resolved graph, and a compact AST summary for each cached source. Current
caches are written as `ari-module-cache-v4`; older v1/v2/v3 caches are treated
as stale because they do not carry the current AST-summary declaration
fingerprints and declaration payloads. A later build can validate the cache and
parse from that snapshot:

```sh
ari app.ari -I packages --use-module-cache build/app.aricache --emit-llvm build/app.ll
```

Cache validation checks the root input, module search paths, active cfg
features, implicit `std` mode, current source content hashes, and whether each
cached `mod` import still resolves to the same file. If any input changed, Ari
rejects the cache and asks you to regenerate it with `--emit-module-cache`.
Malformed caches that repeat a source snapshot or AST summary for the same
module/path/root record are rejected before validation. The embedded metadata
summary is parsed with the same duplicate-record checks as a standalone
`.arimeta` file.
After validation succeeds, file-backed module imports are resolved from the
validated cache import table instead of searching candidate paths again.
After reading from the cached source snapshot, Ari also rebuilds the module
metadata and per-source AST summaries, then compares them with the data embedded
in the cache. AST summaries include counts, declaration fingerprints, and a
compact declaration payload for the source-level item surface. Cache loading
parses that payload and checks its hash and counts, so edited or corrupted
summaries are caught before semantic checking relies on them.

This first cache format skips dependency source discovery after validation and
reads module source text from the cached snapshot. It still parses the cached
source snapshot; the AST summary records are the stable bridge toward a future
cache that can skip dependency parsing after validation.

## Nested Modules

Nested modules are addressed with their full path:

```ari
mod Outer {
  mod Inner {
    pub fn value() -> i64 {
      return 5
    }
  }
}

fn main() -> i64 {
  return Outer::Inner::value()
}
```

Inside a module, `self::` starts from the current module and `super::` starts
from the parent module. These relative paths work in item paths, type paths,
enum case and constant patterns, and `use` declarations:

```ari
mod Outer {
  pub enum Choice {
    A(i32),
    B,
  }

  pub fn base() -> i64 {
    return 3
  }

  mod Inner {
    use super::base as parent_base

    pub fn total(value: super::Choice) -> i64 {
      return parent_base() + match value {
        super::A(inner) => inner as i64,
        super::B => 0
      }
    }
  }
}
```

`super::` cannot escape the root module.

## Limits

- Module declarations themselves do not have runtime values.
- Duplicate `use` aliases in the same module scope are rejected.
- Module caches currently store source snapshots. They validate dependency
  inputs before reuse, but they do not yet store AST or IR summaries that skip
  dependency parsing.
