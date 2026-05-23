# Language Tour

This tour is the bridge between the short [Quick Reference](quick-reference.md)
and the pattern-heavy [Cookbook](cookbook.md). It walks through the executable
core of Ari in the order a new user usually needs it.

The matching runnable example is `examples/language-tour.ari`.

## One-File Program

Every executable starts with `main() -> i64`. The returned value is the process
exit code.

```ari
fn main() -> i64 {
  println("hello from Ari");
  return 0;
}
```

Check, compile, and run:

```sh
./build/ari examples/language-tour.ari --check
./build/ari examples/language-tour.ari -o build/examples/language-tour
./build/examples/language-tour
```

Use `--check` for the fast edit loop. Use `--emit-llvm path` when you need to
inspect backend lowering without linking.

## Values And Control Flow

Use `let` for immutable locals and `var` for mutable locals. No local shadowing
is allowed, so choose stable names instead of reusing the same one.

```ari
fn range_bonus(limit: i64) -> i64 {
  var total = 0;
  for value in 0..limit {
    total += value;
  }
  return total;
}
```

Conditions and loops use `bool`. Range loops currently cover the common
integer cases used by examples and tests.

## Functions And Generics

Functions are explicit about parameter and return types. Generic functions use
square brackets.

```ari
fn identity[T](value: T) -> T {
  return value;
}
```

Start with concrete functions while debugging a feature. Add generics when the
shape is already clear and the focused test says which type family matters.

## Structs, Enums, And Match

Use structs for product data and enums for named alternatives.

```ari
struct Point {
  x: i64,
  y: i64,
}

enum ScoreResult {
  Missing,
  Value(i64),
}
```

Use `match` when each enum case must be handled explicitly:

```ari
match classify(value) {
  Value(inner) => {
    total = inner;
  },
  Missing => {
    total = 1;
  },
}
```

For absence and expected failure in normal code, prefer `Option[T]` and
`Result[T, E]` from the prelude. They also work with `?` and `??` where the
shape is supported.

## Traits And Methods

Ari has `trait`; it does not have `class` or `interface`.

```ari
trait Score {
  fn score(self) -> i64;
}

impl Score for Point {
  fn score(self) -> i64 {
    return self.x + self.y;
  }
}
```

Use traits for behavior that multiple types should share. Use inherent `impl`
blocks for constructors or methods that belong only to one type.

## Modules And Names

Inline modules are useful for small examples and tests:

```ari
mod Math {
  pub fn double(value: i64) -> i64 {
    return value * 2;
  }
}
```

Call public items with qualified names:

```ari
let value = Math::double(base);
```

For larger programs, use file-backed modules with `mod name;`, `-I path`, or
`--module-path path`. See [Modules](modules.md) for the layout rules.

## Ownership And Allocation

Ari checks ownership before LLVM lowering. Owned values must be moved,
returned, dropped, or forgotten explicitly. Allocation is capability-oriented:
pass a zone or another explicit owner instead of assuming a hidden global heap.

For simple numeric examples, no zone is needed. For growable source-library
containers, use a zone:

```ari
var zone = zone::create(1024);
var values = std::vec::new<i64>(ref mut zone, 4);
values.push(10);
zone::destroy(zone);
```

Keep ownership tests focused. A single fixture should show the move, borrow, or
drop rule it protects.

## Tests As Examples

Read `ok/` tests as executable examples and `errors/` tests as expected
diagnostics. Compiler artifact fixtures add committed text-output buckets:

- `tests/cases/compiler-development/artifact/ok/`: golden compiler artifacts.
- `tests/cases/compiler-development/artifact/errors/`: expected diagnostic or
  artifact mismatch outputs.

When changing language behavior, update both the focused feature test and the
docs page that a new user would read first.

## What To Avoid

- Do not use `class` or `interface`; use `struct`, `enum`, and `trait`.
- Do not rely on unsupported front-end-only syntax lowering to LLVM.
- Do not hide allocation behind a global heap.
- Do not use sentinel integers when `Option[T]` or `Result[T, E]` expresses the
  state naturally.
- Do not solve ordinary compiler-development pressure with bootstrap-only syntax.

## Next Steps

- Use [Feature Status](feature-status.md) before choosing a feature.
- Use [Examples And Tests](examples-and-tests.md) to find the matching test
  family.
- Use [Cookbook](cookbook.md) for copyable patterns.
- Use [Front-End Only Syntax](front-end-only.md) when parsed syntax does not
  yet execute.
