# std::cmp

`std::cmp` defines Ari's comparison vocabulary and small source-only helpers
for generic code. Use it when a function should work over any type that can be
ordered through a trait impl instead of hard-coding integer operators.

The public helper names intentionally avoid type suffixes. The module path
already says the domain, and the same names can grow with better generic
numeric and derive support later.

## When To Use It

Use `std::cmp` when code needs to choose or test values by order:

- pick the smaller or larger value
- clamp a value into an inclusive range
- check whether a value is inside an inclusive range
- write generic helpers over a custom ordered type

For one-off primitive comparisons, plain operators such as `<`, `>`, `==`,
and `!=` are still the clearest spelling.

## API

Comparison traits:

```ari
cmp::Eq[T]
cmp::PartialEq[T]
cmp::Ord[T]
cmp::PartialOrd[T]
```

Current trait methods are deliberately minimal:

```ari
fn eq(self, other: T) -> bool
fn lt(self, other: T) -> bool
```

`Ord[T]::lt` is enough for the current value helpers. The compiler does not
enforce ordering laws such as transitivity; trait impl authors are responsible
for making the comparison meaningful.

Value helpers:

```ari
cmp::min<T>(left, right)
cmp::max<T>(left, right)
cmp::clamp<T>(value, low, high)
cmp::is_between<T>(value, low, high)
```

`min` and `max` return one of the two inputs. `clamp` returns `low` when the
value is below the range, `high` when it is above the range, and the original
value otherwise. `is_between` returns true for `low <= value <= high`, so both
range endpoints are included.

`clamp` and `is_between` assert that `low <= high`. Passing an inverted range
is treated as a programmer error.

The root prelude re-exports these helpers, so `min<T>`, `max<T>`, `clamp<T>`,
and `is_between<T>` are aliases for the `std::cmp` implementations.

## Example

```ari
impl cmp::Ord[i64] for i64 {
  fn lt(self, other: i64) -> bool {
    return self < other;
  }
}

struct Score {
  value: i64,
}

impl cmp::Ord[Score] for Score {
  fn lt(self, other: Score) -> bool {
    return self.value < other.value;
  }
}

fn bounded_score(score: Score) -> Score {
  return cmp::clamp<Score>(
    score,
    Score { value: 0 },
    Score { value: 100 });
}

fn main() -> i64 {
  let value = cmp::min<i64>(12, 7);
  if cmp::is_between<i64>(value, 5, 10) {
    return cmp::max<i64>(value, 9);
  }
  return 0;
}
```

## Limits

Primitive and aggregate comparison impl coverage is still growing. If a
generic helper reports that an `Ord[T]` impl is missing, add a focused impl for
that type or use direct primitive operators in non-generic code.

The current comparison traits do not yet provide a richer `compare` result,
derived ordering for every aggregate shape, or separate checked handling for
partial orders. Those belong in later trait and derive slices.

## Tests

The focused positive behavior test is:

```text
tests/cases/standard-library/ok/std-cmp-value-helpers.ari
```

`make check-prelude` emits LLVM for that file, checks representative public
helper symbols, builds an executable, and verifies the exit code. The public
API is tracked in `tests/std_api_manifest.txt` and checked by
`make check-std-api`.

## Future Work

Potential next slices:

- derived comparison impl coverage for more aggregate shapes
- clearer diagnostics when generic comparison helpers lack an `Ord[T]` impl
- richer comparison result types after enum and trait ergonomics are ready
- partial-order helpers after the language has a documented floating and
  partial-comparison policy
