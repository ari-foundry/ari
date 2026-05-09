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

`.arih` is Ari's header-like source surface. It is parsed like normal Ari, but
the convention is to keep declarations, extern bindings, ABI declarations, and
small public module surfaces there.

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
- Compiled package metadata is planned.
