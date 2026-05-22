# Ari Cookbook

This page shows practical patterns that combine the language features described
elsewhere. Treat these as starting points for writing new Ari programs from the
docs alone.

Unless a section says otherwise, examples assume the implicit `std` package is
loaded and compile through the default LLVM path.

## Minimal Program

```ari
fn main() -> i64 {
  println("hello from Ari");
  return 0;
}
```

Build and run:

```sh
make
./build/ari hello.ari -o build/hello.elf
./build/hello.elf
```

## Return A Process Status

```ari
fn main() -> i64 {
  let ok = true;
  if ok {
    return 0;
  }
  return 1;
}
```

The returned `i64` becomes the process exit code.

## Use Variables, Blocks, And Final Expressions

```ari
fn score(seed: i64) -> i64 {
  let adjusted = {
    let base = seed + 2;
    var total = base * 3;
    total += 1;
    total
  };

  if adjusted > 10 {
    adjusted
  } else {
    10
  }
}
```

Use `;` for statements. Omit `;` only for the final expression that should
produce the block or function value.

## Parse Arguments

```ari
fn main() -> i64 {
  let count = arg_count();
  println("argc={}", count);

  if has_arg(0) {
    println("arg0={}", arg(0));
  }

  return count;
}
```

`arg(index)` returns lowercase `string`, a borrowed pointer-shaped string value.
Out-of-range argument access returns an empty string. Use `has_arg(index)` when
missing arguments are an expected branch.

## Define Structs And Methods

```ari
struct Point {
  x: i64,
  mut y: i64,
}

impl Point {
  fn new(x: i64, y: i64) -> Point {
    return Point { x: x, y: y };
  }

  fn sum(self) -> i64 {
    return self.x + self.y;
  }
}

fn main() -> i64 {
  var point = Point::new(4, 5);
  point.y = point.y + 1;
  return point.sum();
}
```

Assigning to a field requires both a `var` binding and a `mut` field.

## Use Enums, Option, And Match

```ari
fn maybe_value(flag: bool) -> Option[i64] {
  if flag {
    return Some(42);
  }
  return None<i64>();
}

fn main() -> i64 {
  match maybe_value(true) {
    Some(value) => {
      return value;
    },
    None => {
      return 0;
    },
  }
}
```

Use `Option[T]` when absence is ordinary data. Use `match` when you need to
handle both cases explicitly.

## Use Result-Style Early Return

```ari
enum Calc {
  Err(i32),
  Ok(i32),
}

fn checked_add(left: i32, right: i32) -> Calc {
  return Ok(left + right);
}

fn add_three(value: i32) -> Calc {
  let first = checked_add(value, 1i32)?;
  let second = checked_add(first, 2i32)?;
  return Ok(second);
}

fn main() -> i64 {
  match add_three(4i32) {
    Ok(value) => {
      return value as i64;
    },
    Err(code) => {
      return code as i64;
    },
  }
}
```

Postfix `?` unwraps the success payload and returns the residual case early.
It works with Option/Result-shaped enums, including `std::Option[T]` and
`std::Result[T, E]`.

## Use Fallback Values With `??`

```ari
fn maybe(flag: bool) -> Option[i64] {
  if flag {
    return Some(7);
  }
  return None<i64>();
}

fn main() -> i64 {
  let success = maybe(true) ?? 3;
  let fallback = maybe(false) ?? 5;
  return success + fallback;
}
```

`??` evaluates to the success payload when present, otherwise to the fallback.

## Iterate Ranges And Local Vectors

```ari
fn main() -> i64 {
  var total = 0;

  for value in 0..5 {
    total += value;
  }

  for value in 0..=3 {
    total += value;
  }

  let values: Vec[i64] = [10, 20, 30];
  for value in values {
    total += value;
  }

  return total;
}
```

Bare `Vec[T]` is the compiler-known local vector surface. Use it for simple
fixed or local list-literal workflows.

## Write A Custom Iterator

```ari
struct Counter {
  mut current: i64,
  end: i64,
}

impl Iterator[i64] for Counter {
  fn next(self: ref mut Self) -> Option[i64] {
    if self.current < self.end {
      let value = self.current;
      self.current += 1;
      return Some(value);
    }
    return None<i64>();
  }
}

fn main() -> i64 {
  let counter = Counter { current: 1, end: 4 };
  var total = 0;

  for value in counter {
    total += value;
  }

  return total;
}
```

Stateful iterators should prefer `next(self: ref mut Self) -> Option[T]`.

## Filter Iterator Items With `for let`

```ari
struct Counter {
  mut current: i64,
  end: i64,
}

impl Iterator[i64] for Counter {
  fn next(self: ref mut Self) -> Option[i64] {
    if self.current < self.end {
      let value = self.current;
      self.current += 1;
      return Some(value);
    }
    return None<i64>();
  }
}

fn main() -> i64 {
  let counter = Counter { current: 2, end: 6 };
  var total = 0;

  for let 3 | 5 in counter {
    total += 10;
  }

  return total;
}
```

Use `for let pattern in iterator` when non-matching items should be skipped.
Plain `for pattern in iterator` stops when the item pattern does not match.

## Borrow And Reborrow Locals

```ari
fn inspect(value: ref i64) -> i64 {
  return value;
}

fn touch(value: ref mut i64) -> i64 {
  return 5;
}

fn main() -> i64 {
  var value = 41;
  var total = 0;

  {
    let shared = ref value;
    let seen = inspect(shared);
    total += seen;
  }

  {
    let unique = ref mut value;
    total += touch(unique);
  }

  value += 1;
  return value + total;
}
```

Shared borrows may overlap. A mutable borrow requires exclusive access and a
mutable source.

## Consume Owned Values

```ari
struct TokenBox {
  token: own i64,
  salt: i64,
}

fn make_owned(value: i64) -> own i64 {
  return value;
}

fn consume(value: own i64) -> i64 {
  drop value;
  return 1;
}

fn main() -> i64 {
  let token = make_owned(42);
  let score = consume(token);

  let boxed = TokenBox { token: make_owned(7), salt: 3 };
  drop boxed;

  return score;
}
```

Every `own` value must be moved, returned, dropped, or forgotten. Leaving a live
owner at return or scope exit is rejected.

## Allocate With A Zone

```ari
fn main() -> i64 {
  var zone = zone::create(128);

  let item = zone::new<i64>(ref mut zone, 42);
  let value = ptr_load(item);

  zone::destroy(zone);
  return value;
}
```

Use `zone::destroy(zone);` to release an `own Zone`. Pointers allocated from a
zone cannot be used after that zone is reset or destroyed.

## Use `std::boxed::Box`

```ari
fn main() -> i64 {
  var zone = zone::create(128);

  var boxed = std::boxed::new<i64>(ref mut zone, 7);
  let before = boxed.get();
  boxed.set(9);
  let after = boxed.take();

  zone::destroy(zone);
  return before + after;
}
```

`Box[T]` is a zone-backed handle. Taking a value leaves the handle empty.
`try_take()` returns `Option[T]` instead of asserting on an empty handle.

## Use Owned Byte Strings

```ari
fn main() -> i64 {
  var zone = zone::create(128);

  var text = std::string::from_string(ref mut zone, "ari");
  text.push(33u8);

  let first = text.first() as i64;
  let length = text.len();

  println("string-bytes={}", length);

  zone::destroy(zone);
  return first + length;
}
```

`std::string::String` is a byte string handle. It stores `u8` values and uses
the source zone for its buffer. Use lowercase `string` for borrowed
NUL-terminated text pointers.

## Use Source `std::vec::Vec`

```ari
fn main() -> i64 {
  var zone = zone::create(256);

  var values = std::vec::new<i64>(ref mut zone, 2);
  values.push(10);
  values.push(20);
  values.push(30);

  let total = values.len() + values.get(0) + values.last();

  zone::destroy(zone);
  return total;
}
```

`std::vec::Vec[T]` is the source standard-library growable vector handle. It is
different from bare local `Vec[T]` list-literal storage.

## Copy Between Zones

```ari
fn main() -> i64 {
  var source = zone::create(128);
  var target = zone::create(128);

  var text = std::string::from_string(ref mut source, "copy");
  let copied = text.copy_to(ref mut target);
  let result = copied.len();

  zone::destroy(target);
  zone::destroy(source);
  return result;
}
```

Use `copy_to(ref mut target_zone)` or module-level copy helpers when a handle
must outlive the zone that currently owns its storage.

## Compare With Traits And Helpers

```ari
fn main() -> i64 {
  let low = cmp::min<i64>(7, 3);
  let high = cmp::max<i64>(7, 3);
  let clamped = cmp::clamp<i64>(99, 0, 10);
  return low + high + clamped;
}
```

`cmp::min`, `cmp::max`, and `cmp::clamp` require an `Ord[T]` implementation.
The standard library already provides that impl for integer types; custom
structs and enums should define their own `cmp::Ord[T]` impl.

## Derive Common Traits

```ari
impl Eq[i64] for i64 {
  fn eq(self, other: i64) -> bool {
    return self == other;
  }
}

impl Ord[i64] for i64 {
  fn lt(self, other: i64) -> bool {
    return self < other;
  }
}

@derive(Eq, Ord)
struct Point {
  x: i64,
  y: i64,
}
```

Derives are source-level conveniences over supported trait surfaces. They do
not make ownership or allocation implicit.

## Split Code Into Modules

```ari
mod Math {
  pub fn double(value: i64) -> i64 {
    return value + value;
  }

  fn hidden() -> i64 {
    return 1;
  }
}

use Math::double;

fn main() -> i64 {
  return double(21);
}
```

Use `pub` for items that other modules may name. Private module items are
visible only inside their module.

For file-backed modules, keep `mod math;` in the parent source and place the
child module in `math.ari` or `math.arih` on the module path.

## Call C

```ari
extern "C" fn puts(text: string) -> i32 = "puts";

fn main() -> i64 {
  puts("hello from libc");
  return 0;
}
```

Use lowercase `string` for C-style string pointers. For `@repr(C)` aggregates
and header emission details, see [C FFI And Libraries](ffi.md).

## Export A Shared Library Function

```ari
@export("ari_add")
pub fn add(left: i64, right: i64) -> i64 {
  return left + right;
}
```

Build it with:

```sh
./build/ari library.ari --shared -o build/libari_sample.so
```

Add `--emit-c-header build/library.h` when the public C surface is supported by
the current header emitter.

## Write Tests

```ari
fn add(left: i64, right: i64) -> i64 {
  return left + right;
}

@test
fn adds() {
  assert_eq!(add(2, 3), 5);
  return;
}
```

Build and run:

```sh
./build/ari sample_test.ari --test -o build/sample_test.elf
./build/ari test sample_test.ari --filter adds -o build/sample_test.elf
./build/sample_test.elf
```

`@test` functions cannot take parameters, be generic, be extern, or be named
`main`. They may return `void` or `i64`; a non-zero `i64` return stops the
generated runner with that process status.

## Choose The Right Collection

| Need | Use |
| --- | --- |
| small local literal sequence | `Vec[T]` from `[a, b, c]` |
| borrowed view into local/source collection | `Slice[T]` |
| growable source collection | `std::vec::Vec[T]` with `Zone` |
| owned byte text | `std::string::String` with `Zone` |
| borrowed C-style text | `string` |
| single value placed in a zone | `std::boxed::Box[T]` |
| raw placement pointer | `zone::new<T>(ref mut zone, value)` |

## Choose The Right Control Flow

| Situation | Pattern |
| --- | --- |
| return from function on failure | `value?` |
| fallback for missing value | `value ?? fallback` |
| inspect every enum case | `match value { ... }` |
| run only when pattern matches | `if let Pattern = value { ... }` |
| loop while pattern keeps matching | `while let Pattern = next() { ... }` |
| count through integers | `for value in start..end { ... }` |
| iterate a custom state machine | implement `Iterator[T]` and use `for` |
| skip non-matching iterator items | `for let Pattern in iterator { ... }` |
| leave nested loop early | labeled `break label;` |
| produce an early block value | labeled block with `break label value;` |

## Before You Blame The Compiler

Check these first:

- Is a statement missing `;`, or did a final value accidentally get one?
- Did an integer literal need a suffix or an explicit `as` cast?
- Is an empty `[]` missing an expected `Vec[T]` type?
- Did an `own` value remain live at return or scope exit?
- Did a borrow remain visible when assigning, moving, or dropping its source?
- Did a zone-backed pointer or handle survive `zone::reset` or
  `zone::destroy`?
- Are you using bare local `Vec[T]` where the code needs source
  `std::vec::Vec[T]`, or the reverse?
- Are you trying to use a front-end-only feature in executable code?
