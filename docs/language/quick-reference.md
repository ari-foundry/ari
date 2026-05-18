# Ari Quick Reference

This page is the short, docs-only sheet for writing Ari code. Use it when you
already know roughly what you want to build and need the current spelling,
runtime rule, or standard-library entry point.

For longer explanations, follow the focused pages linked from
[Language Docs](README.md). For examples that combine several features, see
[Cookbook](cookbook.md).

## Mental Model

- Ari is a small systems-language compiler prototype.
- The executable path emits LLVM IR and links with an LLVM driver such as
  `clang`.
- `main` returns `i64`; the process exit code is the returned value.
- Allocation is explicit. Use `Zone` and pass it as a visible capability.
- Ownership is checked before LLVM lowering. `own` values must be moved,
  returned, dropped, or forgotten exactly where the source says so.
- The source standard library is auto-loaded as `std` unless
  `--no-implicit-std` is passed.
- There is no `class` or `interface` keyword. Use `struct`, `enum`, and
  `trait`.

## Build And Run

```sh
make
./build/ari program.ari -o build/program.elf
./build/program.elf
```

Useful compiler modes:

```sh
./build/ari program.ari --check
./build/ari program.ari --emit-llvm build/program.ll
./build/ari program.ari --emit-obj build/program.o
./build/ari program.ari --shared -o build/libprogram.so
./build/ari program.ari --test -o build/program-tests.elf
```

Use `-I path` or `--module-path path` for file-backed modules.

## File Skeleton

```ari
fn helper(value: i64) -> i64 {
  return value + 1;
}

fn main() -> i64 {
  println("value={}", helper(41));
  return 0;
}
```

Prefer semicolons for statement forms such as `let`, `var`, assignment, calls
used as statements, `return`, `break`, `continue`, `drop`, and `forget`.
Omit the semicolon only for the final value expression in a function, block,
`if` expression, `match` arm, or labeled block.

## Common Syntax

| Need | Spell It |
| --- | --- |
| immutable local | `let value = 42;` |
| mutable local | `var count = 0; count += 1;` |
| explicit local type | `let value: i32 = 7i32;` |
| top-level constant | `const LIMIT: i64 = 10;` |
| function | `fn add(a: i64, b: i64) -> i64 { a + b }` |
| void function | `fn log() { println("done"); return; }` |
| unit-returning function | `fn mark() -> () { () }` |
| generic function | `fn id[T](value: T) -> T { value }` |
| struct | `struct Point { x: i64, mut y: i64, }` |
| tuple struct | `struct Pair(i64, mut i64)` |
| enum | `enum OptionI64 { None, Some(i64), }` |
| trait | `trait Score { fn score(self) -> i64 }` |
| impl | `impl Score for i64 { fn score(self) -> i64 { self } }` |
| inherent impl | `impl Point { fn new(x: i64, y: i64) -> Point { Point { x: x, y: y } } }` |
| module | `mod Math { pub fn value() -> i64 { 7 } }` |
| import | `use Math::value; use Math::value as answer;` |
| file module | `mod math;` with `math.ari` or `math.arih` on the module path |
| C import | `extern "C" fn puts(text: string) -> i32 = "puts";` |
| shared export | `@export("ari_add") pub fn add(a: i64, b: i64) -> i64 { a + b }` |
| test | `@test fn adds() { assert_eq!(add(1, 2), 3); return; }` |

## Types At A Glance

| Family | Examples | Notes |
| --- | --- | --- |
| signed integers | `i8`, `i16`, `i32`, `i64` | No implicit width casts. |
| unsigned integers | `u8`, `u16`, `u32`, `u64` | Suffix literals like `255u8`. |
| floats | `f32`, `f64`, `f128` | `f32`/`f64` arithmetic lowers today; `f128` storage is limited. |
| bool | `bool` | Used by logical operators and normal conditions. |
| borrowed string pointer | `string` | NUL-terminated pointer-shaped string for literals and C-style text. |
| owned byte string handle | `String`, `std::string::String` | Zone-backed explicit allocation handle. |
| tuple | `(i64, bool)`, `()` | Single-element tuples are not supported. |
| fixed array | `[i64, 3]` | Constant length, stack/aggregate storage. |
| local vector | `Vec[i64]` | Compiler-known local vector surface from list literals. |
| source vector handle | `std::vec::Vec[i64]`, `std::Vec[i64]` | Zone-backed growable handle. |
| slice view | `Slice[i64]`, `std::Slice[i64]` | Non-owning pointer plus length view. |
| raw pointer | `ptr i64`, `i64?`, `ptr c_void` | Use explicit casts and `std::mem` helpers. |
| shared borrow | `ref i64` | Many shared borrows may overlap. |
| mutable borrow | `ref mut i64` | Requires mutable source and exclusive access. |
| owner | `own i64`, `own Zone` | Must be consumed, returned, moved, dropped, or forgotten. |
| function pointer | `fn(i64) -> i64` | Function names can become pointer values. |
| trait object | `dyn Trait` forms | See [Traits](traits.md) and [Front-End Only Syntax](front-end-only.md). |

## Statements And Expressions

```ari
let value = if flag { 1 } else { 0 };

let scoped = {
  let base = 40;
  base + 2
};

match maybe {
  Some(value) => {
    return value;
  },
  None => {
    return 0;
  },
}
```

Use `return value;` for explicit returns. A final expression without `;` is an
implicit return from the current value-producing block.

## Control Flow

```ari
if ready {
  println("ready");
} else {
  println("waiting");
}

while index < limit {
  index += 1;
}

for value in 0..5 {
  total += value;
}

for value in 0..=5 {
  total += value;
}
```

Use `if let`, `while let`, and `match` for enum and aggregate patterns:

```ari
if let Some(value) = maybe {
  total += value;
}

while let Some(value) = next(index) {
  total += value;
  index += 1;
}

let score = match point {
  Point { x: 0, y } => y,
  Point { x, y } => x + y,
};
```

`for` works with ranges, list literals, stored local vectors, direct
`Iterator[T]` values, and accepted `IntoIterator[T]` values. Use
`for let pattern in iterator` when non-matching iterator items should be
skipped.

## Pattern Forms

| Pattern | Example |
| --- | --- |
| wildcard | `_` |
| literal | `0`, `true` |
| range | `10..=20` |
| enum case | `Some(value)` |
| or-pattern | `Left(value) \| Right(value)` |
| alias | `whole @ Some(value)` |
| tuple | `(left, right)` |
| tuple rest | `(first, .., last)` |
| fixed array or sequence | `[head, .., tail]` |
| rest alias | `[head, rest @ .., tail]` |
| struct | `Point { x, y: renamed, .. }` |
| tuple struct | `Rgb(red, _)` |
| reference binding | `let ref Point { x, .. } = point;` |
| mutable reference binding | `let ref mut [first, ..] = values;` |

Or-pattern alternatives must bind the same names with compatible types.

## Operators

| Kind | Operators |
| --- | --- |
| arithmetic | `+`, `-`, `*`, `/`, `%` |
| bit | `&`, `|`, `^`, `~`, `<<`, `>>` |
| compound assignment | `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=` |
| comparison | `==`, `!=`, `<`, `<=`, `>`, `>=` |
| logical | `&&`, `||`, `!` |
| cast | `value as Type` |
| range | `start..end`, `start..=end` |
| try propagation | `value?` |
| null coalescing | `value ?? fallback` |

`?` and `??` work with Option/Result-shaped enums, including generic
`std::Option[T]` and `std::Result[T, E]`.

## Prelude And Standard Library Choices

| Need | Use |
| --- | --- |
| print text or values | `print`, `println`, `print!`, `println!` |
| assertions | `assert!`, `debug_assert!`, `assert_eq!`, `assert_ne!` |
| panic placeholders | `panic!()`, `todo!()`, `unreachable!()` |
| optional value | `Option[T]`, `Some(value)`, `None<T>()` |
| result value | `Result[T, E]`, `Ok<T, E>(value)`, `Err<T, E>(error)` |
| optional/result helpers | Option: `.is_some()`, `.is_none()`, `.filter(fn(ref T) -> bool)`, `.flatten()` for nested options, `.transpose()` for optional results; Result: `.is_ok()`, `.is_err()`, `.transpose()` for result-wrapped options; both: `.unwrap_or(fallback)`, `.map<U>(fn)`, `.and_then<U>(fn)` |
| process args | `arg_count()`, `arg(index)`, `has_arg(index)`, `context::argc()`, `context::arg(index)`, `context::has_arg(index)` |
| input | `input::try_read_byte()`, `input()`, `read_line()`, `input_owned(ref mut zone)` |
| raw IO | `io::write_i64`, `io::write_u64`, `io::write_bool`, `io::write_byte`, `io::write_bytes`, `io::newline` |
| layout | `size_of<T>()`, `align_of<T>()` |
| raw pointers | `ptr_offset`, `ptr_add`, `ptr_load`, `ptr_store` |
| replace/swap | `mem::replace(ref mut place, value)`, `mem::swap(ref mut a, ref mut b)` |
| explicit allocation | `zone::create`, `zone::alloc<T>`, `zone::alloc_array<T>`, `zone::new<T>`, `zone::reset`, `zone::destroy` |
| temporary allocation | `zone::scratch<T>`, `zone::temp`, `zone::promote<T>` |
| single-value handle | `std::boxed::Box[T]`, `Box!(T, ref mut zone, value)` |
| owned byte string | `std::string::String`, `std::string::from_string(ref mut zone, "text")`, `.try_get(index)`, `.index_of_ignore_case(bytes)`, `.parse_decimal_prefix()`, `.trim_to(ref mut zone)` |
| ASCII byte helpers | `ascii::is_digit`, `ascii::equals_ignore_case`, `ascii::index_of_ignore_case`, `ascii::trim`, `ascii::parse_decimal`, `ascii::parse_decimal_prefix` |
| integer math helpers | `math::abs`, `math::is_positive`, `math::is_zero`, `math::div_floor`, `math::div_ceil`, `math::mod_floor`, `math::gcd` |
| bit helpers | `bits::is_set`, `bits::rotate_left`, `bits::bit_width`, `bits::leading_zeros`, `bits::leading_ones` |
| source growable vector | `std::vec::Vec[T]`, `std::vec::new<T>(ref mut zone, capacity)` |
| borrowed view | `Slice[T]`, `.as_slice()`, `slice(data, len)` |
| comparison helpers | `cmp::min`, `cmp::max`, `cmp::clamp` with `Ord` impls |
| iteration traits | `Iterator[T]`, `IntoIterator[T]`, `iter::range`, `iter::range_inclusive` |

Use lowercase `string` for borrowed pointer-shaped text. Use uppercase
`String`/`std::string::String` when the text needs owned zone-backed storage.
Use bare `Vec[T]` for local compiler-known vectors and `std::vec::Vec[T]` for
the source standard-library growable handle.

## Ownership And Borrowing Checklist

- Use `own T` only when a value needs explicit ownership checking.
- After moving an `own` binding, do not read, assign, borrow, or drop the old
  binding.
- Use `drop value;` to consume an owner and run its `Drop` impl when one exists.
- Use `forget value;` only when an owner may be unavailable after control flow
  and leaking any remaining live value is the intended behavior.
- Use `ref value` for shared borrows and `ref mut value` for mutable borrows.
- `ref mut` requires the source binding and selected struct field to be mutable.
- Do not assign, move, drop, or reset through a place while it is borrowed.
- Zone-backed pointers and handles cannot be used after their source zone has
  been reset or destroyed.
- `zone::destroy(zone);` is the visible release operation for `own Zone`.

## Attributes

| Attribute | Use |
| --- | --- |
| `@derive(...)` | Generate supported trait impls such as `Debug`, `Copy`, `Clone`, `Default`, `Eq`, `PartialEq`, `Ord`, and `PartialOrd`. |
| `@repr(C)` | Expose C-layout structs and supported enum shapes. |
| `@export` / `@export("symbol")` | Export a function symbol from shared-library output. |
| `@no_mangle` | Keep a function symbol unmangled. |
| `@deprecated("message")` | Emit use-site warnings. |
| `@cfg(...)` | Include/exclude declarations by target or feature. |
| `@test` | Include a function in `--test` runner builds. |
| `@borrow_return(source.path)` | Declare which parameter a returned borrow comes from. |

## FFI And Libraries

```ari
extern "C" fn puts(text: string) -> i32 = "puts";

@export("ari_add")
pub fn add(left: i64, right: i64) -> i64 {
  return left + right;
}
```

Use `--shared` for shared libraries and `--emit-c-header path` for supported
C header output. C-facing aggregate values should use `@repr(C)`. Ownership
does not cross C directly; expose owned resources through explicit `ptr`,
`ref`, or wrapper APIs.

## Current Gotchas

- No local shadowing: one local name maps to one lowered slot.
- No overload selection by parameter type or count.
- No implicit numeric casts; write `as`.
- Empty `[]` needs an expected `Vec[T]` or non-empty element list.
- `format!` without an explicit zone is reserved; use `print`/`println` for
  output or `format_in!(ref mut zone, ...)` for owned strings.
- Most standard-library allocation APIs are explicit-zone APIs.
- Raw pointers are available, but Ari's ownership diagnostics are not a full
  memory-safety proof once raw pointer escape hatches are used.
- If a feature is documented as front-end-only, expect parsing or checking but
  not executable lowering.

## Where To Go Next

- Program setup and CLI: [Getting Started](getting-started.md)
- Function, generic, and call rules: [Functions](functions.md)
- Bindings and destructuring: [Variables](variables.md)
- Type details and aggregate layout: [Types](types.md)
- Operators: [Operators](operators.md)
- Loops, matches, and block expressions: [Control Flow](control-flow.md)
- Enums and pattern matching: [Enums And Pattern Matching](enums-patterns.md)
- Traits and dispatch: [Traits](traits.md)
- Prelude, formatting, input, and standard helpers: [Prelude](prelude.md)
- Source standard library map: [Standard Library](standard-library.md)
- Ownership, borrowing, zones, raw pointers: [Memory](memory.md)
- C FFI and shared libraries: [C FFI And Libraries](ffi.md)
- Reserved or partial syntax: [Front-End Only Syntax](front-end-only.md)
