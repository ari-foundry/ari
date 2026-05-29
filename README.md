# Ari

Ari is a systems language designed and implemented by AI. It targets LLVM and
explores a C/C++ replacement shape: value types, explicit ownership,
region-based memory, traits, ADT-style enums, pattern matching, modules, and
C FFI — with no garbage collector and no hidden runtime.

The compiler, standard library, and language design were produced through
AI-driven development, making Ari an experiment in what end-to-end AI language
engineering looks like.

```ari
fn main() -> i64 {
  println("hello from Ari");
  return 0;
}
```

## Language Features

**Values and ownership** — every value has a clear owner; borrows are explicit
with `ref` and `ref mut`; `own` annotations track unique ownership through
function boundaries.

**Region memory** — allocation lives in caller-supplied zones or `region {}`
blocks; no GC pause, no hidden allocator, memory freed in bulk at region end.

**Traits and generics** — parametric polymorphism with trait bounds; `impl
Trait for Type` dispatch; generic structs, enums, and functions.

**ADT enums and pattern matching** — sum types with payloads matched
exhaustively with `match`; compiler-checked coverage.

**Modules** — `mod Name { }` inline blocks and file-level `mod name;`
declarations; `pub` visibility; qualified paths.

**C FFI** — `extern "C"` declarations; `ptr T` and `c_char` interop; emit
object files or shared libraries for use from C.

**LLVM backend** — compiles to native code via LLVM IR; supports Linux
x86-64 ELF today.

## Code Examples

### Pattern matching

```ari
enum Command {
  Stop,
  Add(i64),
  Multiply(i64),
}

fn apply(total: i64, cmd: Command) -> i64 {
  match cmd {
    Stop        => { return total; }
    Add(n)      => { return total + n; }
    Multiply(n) => { return total * n; }
  }
}

fn main() -> i64 {
  var total = 1;
  total = apply(total, Add(4));
  total = apply(total, Multiply(3));
  total = apply(total, Stop);
  println("result={}", total);
  return 0;
}
```

### Traits and generics

```ari
trait Score {
  fn score(self) -> i64;
}

struct Point { x: i64, y: i64 }

impl Score for Point {
  fn score(self) -> i64 { return self.x + self.y; }
}

fn identity[T](value: T) -> T { return value; }

fn main() -> i64 {
  let p = Point { x: 2, y: 3 };
  println("score={}", identity(p.score()));
  return 0;
}
```

### Ownership and borrowing

```ari
fn consume(value: own i64) -> i64 {
  drop value;
  return 1;
}

fn observe(a: ref i64, b: ref mut i64) -> i64 {
  return *a + *b;
}

fn main() -> i64 {
  var x = 10;
  var y = 20;
  let result = observe(ref x, ref mut y);
  println("observe={}", result);
  return 0;
}
```

## Requirements

- C++17 compiler
- `make`
- `python3` (repository checks)
- `clang` or another LLVM driver for linked output

Set `ARI_LLVM_CC=/path/to/clang` if `clang` is not on `PATH`.

## Quick Start

```sh
make
./build/ari examples/hello.ari -o build/hello
./build/hello
```

Check without linking:

```sh
./build/ari examples/hello.ari --check
```

## Build Targets

```sh
make              # release compiler
make debug        # debug build
make sanitize     # address + UB sanitizers
make examples     # compile all bundled examples
make check        # run full test suite
make check-debug
make check-sanitize
```

## Compiler Flags

```sh
./build/ari app.ari -o app                 # link executable
./build/ari app.ari --emit-llvm app.ll     # emit LLVM IR
./build/ari app.ari --emit-obj app.o       # emit object file
./build/ari lib.ari --shared -o libari.so  # shared library
./build/ari app.ari -L ./lib -l mylib      # link against library
```

## Install

```sh
make install                                        # default prefix /usr/local
make install PREFIX=/opt/ari
make install DESTDIR=/tmp/stage PREFIX=/usr/local   # staged install
make uninstall
```

Installs `bin/ari`, `share/ari/lib/std.arih`, and `share/ari/lib/std/*.arih`.
The compiler resolves the stdlib via `ARI_STDLIB_PATH`, then adjacent source
tree, then installed path.

## Project Status

Ari has a broad executable compiler prototype today: core control flow,
integers, floats, bools, ownership checks, structs, enums, pattern matching,
modules, generics, traits, source stdlib, C FFI, LLVM IR / object /
shared-library output, lint tooling, and LSP tooling. It is not
production-stable yet.

- [Language Feature Status](docs/language/feature-status.md)
- [Compiler Readiness Inventory](docs/dev/compiler-readiness-inventory.md)
- [Standard Library Completion](docs/stdlib/completion-status.md)
- [Standard Library Roadmap](docs/stdlib/roadmap.md)

## Documentation

- [Language Overview](docs/language/README.md)
- [Language Tour](docs/language/language-tour.md)
- [Quick Reference](docs/language/quick-reference.md)
- [Cookbook](docs/language/cookbook.md)
- [Standard Library Overview](docs/stdlib/README.md)
- [C FFI Guide](docs/language/ffi.md)
- [Memory and Ownership](docs/language/memory.md)
- [Developer Overview](docs/dev/README.md)
- [Editor Tooling](editors/README.md)

Agent-oriented project notes live in [AGENTS.md](AGENTS.md).

## License

Apache License 2.0. See [LICENSE](LICENSE).
