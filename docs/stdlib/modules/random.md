# std::random

`std::random` separates two jobs that should not be confused:

- OS entropy for seeds and security-sensitive randomness sources.
- A deterministic, seedable non-cryptographic PRNG for simulations, tests,
  randomized algorithms, and reproducible shuffling.

The public names stay short because the module path already says the domain:
write `random::entropy()`, `random::seed(123u64)`, and `rng.range(0, 10)`.

## API

```ari
struct Prng

random::entropy()
random::entropy_raw_result()
random::entropy_result()
random::seed(value)
random::from_entropy()
random::from_entropy_result()
random::seed_from_os()
random::seed_from_os_result()
random::next(ref mut rng)
random::boolean(ref mut rng)
random::below(ref mut rng, upper)
random::try_below(ref mut rng, upper)
random::range(ref mut rng, start, end)
random::try_range(ref mut rng, start, end)
random::float(ref mut rng)
random::fill(values)
random::fill_raw_result(values)
random::fill_result(values)
random::fill_from(ref mut rng, values)
random::shuffle<T>(ref mut rng, values)

Prng::seed(value)
Prng::from_entropy()
Prng::from_entropy_result()
Prng::seed_from_os()
Prng::seed_from_os_result()
rng.next()
rng.boolean()
rng.below(upper)
rng.try_below(upper)
rng.range(start, end)
rng.try_range(start, end)
rng.float()
rng.fill(values)
rng.shuffle<T>(values)
```

`entropy()` returns one `u64` from the operating system. On the current hosted
Linux runtime it tries the `getrandom` libc/syscall path first and then falls
back to reading eight bytes from `/dev/urandom`. If both fail, the strict helper
terminates instead of returning weak entropy. `entropy_result()` exposes the
same policy as `Result[u64, std::error::Error]`; `entropy_raw_result()` keeps
the compatibility `Result[u64, i64]` shape used by lower-level wrappers.

`fill(values)` fills a `Slice[u8]` directly from OS entropy. On hosted Linux
it uses the same `getrandom`-first policy as `entropy()` and falls back to
`/dev/urandom` only when the first path cannot make progress. Use it for seed
bytes or small random tokens. `fill_result(values)` returns `Result[(),
std::error::Error]`, and `fill_raw_result(values)` returns `Result[(), i64]`.
The strict `fill(values)` helper remains available for code that treats missing
host entropy as unrecoverable.

`Prng` is deterministic and non-cryptographic. `seed(value)` and
`Prng::seed(value)` create a repeatable generator. `from_entropy()` and
`seed_from_os()` seed that generator from OS entropy, which is useful for games,
randomized algorithms, and tests that do not need reproducibility.
`from_entropy_result()` and `seed_from_os_result()` return recoverable
`Result[Prng, std::error::Error]` values when entropy failure should be handled
by the caller.

`next()` returns the next raw `u64`. `boolean()` consumes one raw word and
returns a deterministic `bool`. `below(upper)` returns an `i64` in `0..upper`;
`range(start, end)` returns an `i64` in `start..end`; both assert on invalid
bounds. The bounded integer path redraws rejected values instead of using raw
modulo, so every result in the requested range has the same probability. Use
`try_below(upper)` and `try_range(start, end)` when bounds come from user
input; they return `None` for invalid bounds and advance the generator only
when they return `Some(value)`. `float()` returns an `f64` in `[0.0, 1.0)`.
`fill_from` and `rng.fill(values)` fill bytes from the deterministic PRNG.

`shuffle(rng, values)` implements an in-place Fisher-Yates shuffle over a
borrowed `Slice[T]`.

## Examples

Repeatable random integers:

```ari
fn main() -> i64 {
  var rng = random::seed(42u64);
  let value = rng.range(10, 20);
  if value >= 10 && value < 20 {
    return 0;
  }
  return 1;
}
```

OS-seeded shuffle:

```ari
fn main() -> i64 {
  var rng = random::from_entropy();
  var values = [1, 2, 3, 4];
  rng.shuffle<i64>(values.as_slice());
  return 0;
}
```

## Feature Status

| Need | Status |
| --- | --- |
| OS entropy | Current: `entropy()` returns a `u64`; `entropy_result()` and `fill_result(values)` expose recoverable `Error` results; `fill(values)` fills byte slices directly from the host. |
| `/dev/urandom` | Current Linux runtime fallback when `getrandom` does not return eight bytes. |
| `getrandom` syscall | Current hosted Linux runtime uses the libc `getrandom` entry point before fallback. |
| CSPRNG seed | Current: use `entropy()`, `from_entropy()`, or `seed_from_os()` as seed material. |
| CSPRNG stream | Roadmap: not exposed yet; do not use `Prng` for cryptography. |
| non-crypto PRNG | Current: deterministic `Prng` with `seed`, `next`, `boolean`, `below`, `range`, and `float`. |
| shuffle | Current: `shuffle<T>(ref mut rng, values)` and `rng.shuffle<T>(values)`. |
| random bool | Current: `boolean()` and `rng.boolean()` from the deterministic PRNG stream. |
| random int | Current: unbiased `below`/`range` asserting helpers and `try_below`/`try_range` fallible helpers over `i64` bounds. |
| random float | Current: `float() -> f64` in `[0.0, 1.0)`. |

## Current Limits

- `Prng` is not cryptographically secure. It is for reproducible and ordinary
  randomized behavior only.
- Bounded integer helpers use rejection sampling over the raw `u64` stream.
  The exact sequence may still change if the PRNG core changes, so tests should
  assert range and repeatability properties rather than pin every value.
- Strict OS entropy helpers still hard-fail on host entropy failure. Use
  `entropy_result()` or `fill_result(values)` when the caller should decide how
  to recover.
- No distribution helpers exist yet. Normal/exponential/weighted sampling
  should probably live outside the first core stdlib slice.

## Tests

```text
tests/cases/standard-library/ok/random/std-random-basic.ari
tests/cases/standard-library/ok/random/std-random-result.ari
tests/cases/standard-library/ok/random/std-random-try-bounds.ari
```

The focused test covers runtime entropy hook reachability, deterministic
seeded PRNG behavior, bounded integer generation, unit float generation,
deterministic boolean generation, deterministic byte filling, OS byte filling,
and generic slice shuffling.
The bound test covers `Option`-returning invalid-bound handling for both module
functions and `Prng` methods.
The result test covers `Result`-returning OS entropy helpers and a recoverable
invalid-slice error path.

## Next Work

- Add larger distribution tests once Ari has a property-test or statistical
  test harness.
- Keep cryptographic PRNG streams and advanced distributions separate until
  Ari has a clear crypto/package policy.
