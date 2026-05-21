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
- return or chain a stable three-way comparison result
- write generic helpers over a custom ordered type

For one-off primitive comparisons, plain operators such as `<`, `>`, `==`,
and `!=` are still the clearest spelling. For custom equality and ordering
types, implement `Eq[T]::eq` and `Ord[T]::lt`; the comparison operators use
those methods when no builtin comparison exists.

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

`Eq[T]::eq` backs `==` and `!=` for non-builtin comparable values. `!=` is
lowered as `!eq(...)`, so an `Eq` impl only needs one method. `Ord[T]::lt`
backs `<`, `>`, `<=`, and `>=` for non-builtin ordered values. `>` flips the
operands, and `<=`/`>=` negate the opposite `lt` call. The compiler does not
enforce ordering laws such as transitivity; trait impl authors are responsible
for making the comparison meaningful.

Three-way ordering:

```ari
cmp::Ordering
cmp::Less
cmp::Equal
cmp::Greater
cmp::compare<T>(left, right)
cmp::reverse(ordering)
cmp::then(first, second)
cmp::then_compare<T>(ordering, left, right)
cmp::is_less(ordering)
cmp::is_equal(ordering)
cmp::is_greater(ordering)
cmp::is_less_or_equal(ordering)
cmp::is_greater_or_equal(ordering)
```

`compare` returns `Less`, `Equal`, or `Greater` through the same `<` and `>`
operators users write. For generic values those operators dispatch to
`Ord[T]::lt`, so the source implementation stays readable without changing the
trait contract. `then` keeps the first non-`Equal` result, which is useful for
lexicographic comparison. `then_compare` performs the second comparison only as
part of this source helper shape; once lazy closures land, a future `then_with`
style API can avoid computing fallback comparisons eagerly.

`Ordering` also has method wrappers with the same names, which is the preferred
style for new code once a comparison result is already in hand:

```ari
ordering.reverse()
ordering.then(next)
ordering.then_compare<T>(left, right)
ordering.is_less()
ordering.is_equal()
ordering.is_greater()
ordering.is_less_or_equal()
ordering.is_greater_or_equal()
```

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

`std::cmp` provides `Eq` and `PartialEq` impls for `bool` and all fixed-width
integer types, plus `Ord` and `PartialOrd` impls for all fixed-width integer
types. Application code only needs to write an impl for custom value types.

## Example

```ari
struct Score {
  value: i64,
}

impl cmp::Ord[Score] for Score {
  fn lt(self, other: Score) -> bool {
    return self.value < other.value;
  }
}

impl cmp::Eq[Score] for Score {
  fn eq(self, other: Score) -> bool {
    return self.value == other.value;
  }
}

struct Point {
  x: i64,
  y: i64,
}

impl cmp::Ord[Point] for Point {
  fn lt(self, other: Point) -> bool {
    match cmp::compare<i64>(self.x, other.x) {
      cmp::Less => {
        return true;
      }
      cmp::Greater => {
        return false;
      }
      cmp::Equal => {
        return self.y < other.y;
      }
    }
  }
}

fn bounded_score(score: Score) -> Score {
  return cmp::clamp<Score>(
    score,
    Score { value: 0 },
    Score { value: 100 });
}

fn main() -> i64 {
  let first = Score { value: 7 };
  let second = Score { value: 7 };
  if first == second {
    println("scores match");
  }

  let value = cmp::min<i64>(12, 7);
  if cmp::is_between<i64>(value, 5, 10) {
    let ordering = cmp::compare<Point>(
      Point { x: 1, y: 9 },
      Point { x: 1, y: 4 });
    if ordering.is_greater() {
      return cmp::max<i64>(value, 9);
    }
  }
  return 0;
}
```

## Limits

Primitive and aggregate comparison impl coverage is still growing. If a
generic helper reports that an `Ord[T]` impl is missing, add a focused impl for
that type or use direct primitive operators in non-generic code.

The current comparison traits still do not provide derived ordering for every
aggregate shape or separate checked handling for partial orders. Those belong
in later trait and derive slices.

Custom operator glyph declarations are not part of `std::cmp` yet. Equality
and ordering are the first builtin operator bridges because they map cleanly to
the existing single-method `Eq[T]` and `Ord[T]` contracts.

## Tests

The focused positive behavior test is:

```text
tests/cases/standard-library/ok/cmp/std-cmp-value-helpers.ari
tests/cases/standard-library/ok/cmp/std-cmp-equality-operator.ari
tests/cases/standard-library/ok/cmp/std-cmp-order-operators.ari
tests/cases/standard-library/ok/cmp/std-cmp-ordering.ari
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
