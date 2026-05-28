# Ari

Ari is a prototype systems-language compiler written in C++17. It explores a
C/C++ replacement shape with value types, explicit ownership, traits, modules,
ADT-style enums, pattern matching, C FFI, and a source standard library.

The compiler emits LLVM IR and invokes an LLVM driver such as `clang` for
linked executable, object, and shared-library output. Ari is still experimental;
language and standard-library APIs may change while the compiler and package
story settle.

## Requirements

- A C++17 compiler
- `make`
- `python3` for repository checks
- `clang` or another LLVM driver for linked executable/object output

Set `ARI_LLVM_CC=/path/to/clang` or pass `--llvm-cc /path/to/clang` when the
LLVM driver is not discoverable on `PATH`.

## Quick Start

Build the release compiler:

```sh
make
```

Compile and run an Ari program:

```sh
./build/ari examples/count.ari -o build/count.elf
./build/count.elf
echo $?
```

Check a source file without linking:

```sh
./build/ari examples/count.ari --check
```

## Install

The current install target is intentionally small and easy to replace later
with a package manager:

```sh
make install
```

It installs:

```text
$(PREFIX)/bin/ari
$(PREFIX)/share/ari/lib/std.arih
$(PREFIX)/share/ari/lib/std/*.arih
```

The installed compiler first honors `ARI_STDLIB_PATH`, then falls back to a
source-tree `lib/std.arih`, then to `../share/ari/lib/std.arih` relative to the
`ari` executable. Remove the temporary install layout with:

```sh
make uninstall
```

For staged packaging-style tests:

```sh
make install DESTDIR=/tmp/ari-stage PREFIX=/usr/local
/tmp/ari-stage/usr/local/bin/ari --help
```

## Common Commands

```sh
make release
make debug
make sanitize
make tools
make examples
make build-lib
make check-lib
make check-examples
make check-tools
make check
make check-debug
make check-sanitize
```

Useful compiler invocations:

```sh
./build/ari app.ari -o app
./build/ari app.ari --emit-llvm app.ll
./build/ari app.ari --emit-obj app.o
./build/ari lib.ari --shared -o libari_app.so
./build/ari app.ari -L ./lib -l mylib
```

## Examples

```ari
fn main() -> i64 {
  println("count={}", 5)
  return 5
}
```

Build bundled examples:

```sh
make examples
make run-example EXAMPLE=hello
```

## Project Status

Ari currently has a broad executable compiler prototype: core control flow,
integer/float/bool values, ownership checks, structs, enums, pattern matching,
modules, generics, a minimum trait system, source stdlib modules, C FFI, LLVM
IR/object/shared-library output, lint tooling, and LSP tooling.

The project is not production-stable yet. The most useful status documents are:

- [Language Feature Status](docs/language/feature-status.md)
- [Compiler Readiness Inventory](docs/dev/compiler-readiness-inventory.md)
- [Standard Library Completion Status](docs/stdlib/completion-status.md)
- [Standard Library Roadmap](docs/stdlib/roadmap.md)

## Documentation

- [Documentation Index](docs/README.md)
- [Language Overview](docs/language/README.md)
- [Developer Overview](docs/dev/README.md)
- [Standard Library Overview](docs/stdlib/README.md)
- [Editor Tooling](editors/README.md)
- [Project Notes](docs/notes/README.md)

Agent-oriented project notes live in [AGENTS.md](AGENTS.md).

## License

Ari is licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE).
