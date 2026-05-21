# std::hash

`std::hash` provides deterministic source-level hashing for tables, caches,
and small data structures. It is deliberately not a cryptographic module. Use
it when you need stable `u64` hash values inside Ari code; do not use it for
passwords, signatures, or adversarial security boundaries.

The module owns the domain, so public names stay short: `hash::new`,
`hash::write`, `hash::value`, `hash::pair`, `hash::combine`, and
`hash::bytes`.

## API

```ari
hash::Hasher
hash::Hash[T]
hash::new()
hash::reset(ref mut state)
hash::finish(ref state)
hash::write<T>(ref mut state, value)
hash::value<T>(value)
hash::pair<T, U>(left, right)
hash::combine(left_hash, right_hash)
hash::bytes(values)
hash::write_byte(ref mut state, value)
hash::write_bytes(ref mut state, values)
hash::write_u8(ref mut state, value)
hash::write_i8(ref mut state, value)
hash::write_u16(ref mut state, value)
hash::write_i16(ref mut state, value)
hash::write_u32(ref mut state, value)
hash::write_i32(ref mut state, value)
hash::write_u64(ref mut state, value)
hash::write_i64(ref mut state, value)
hash::write_bool(ref mut state, value)
```

`Hasher` is an incremental state object. Feed values with `write<T>` when a
`Hash[T]` impl exists, or with the low-level byte/integer helpers when building
custom hash implementations. `finish` returns the final `u64`.

`Hash[T]` is the trait used by `write<T>` and `value<T>`:

```ari
pub trait Hash[T] {
  fn hash(self, state: ref mut Hasher) -> void;
}
```

The current stdlib includes `Hash` impls for `i8`, `i16`, `i32`, `i64`,
`u8`, `u16`, `u32`, `u64`, and `bool`. Integer helpers feed the natural
little-endian byte width of the type, so `u8(1)`, `u16(1)`, and `u32(1)` stay
distinct hash inputs. User types can implement the trait by writing their
fields into the supplied hasher.

`pair<T, U>(left, right)` hashes two `Hash` values in order. It is a concise
helper for small compound keys and examples where a full custom `Hasher` block
would obscure the intent.

`combine(left_hash, right_hash)` hashes two already-computed `u64` hash values
in order. Use it when a caller already has stable component hashes and wants a
single composed hash.

`bytes(values)` hashes a `Slice[u8]` directly. It is the preferred helper for
byte buffers and byte strings when you already have a slice view. `Slice[u8]`
also implements `Hash`, so generic code can call `hash::value<Slice[u8]>` when
it should treat a byte view like any other hashable value.

`collections::hash_i64` remains as a compatibility helper for current
`HashMap`/`HashSet` constructors, and now delegates to `hash::value<i64>`.
New code that is not tied to collection constructor shape should prefer
`std::hash`.

## Examples

Hash one value:

```ari
let id_hash = hash::value<i64>(42);
```

Hash bytes:

```ari
var data = ['A', 'B', 'C'];
let digest = hash::bytes(data.as_slice());
```

Hash several values incrementally:

```ari
var state = hash::new();
hash::write<i64>(ref mut state, 7);
hash::write<bool>(ref mut state, true);
hash::write_byte(ref mut state, 9u8);
let digest = hash::finish(ref state);
```

Hash two values in order:

```ari
let digest = hash::pair<i64, bool>(7, true);
```

Compose two component hashes:

```ari
let left = hash::value<i64>(11);
let right = hash::value<i64>(13);
let digest = hash::combine(left, right);
```

Implement hashing for a small type:

```ari
struct Pair {
  left: i64,
  right: i64,
}

impl hash::Hash[Pair] for Pair {
  fn hash(self, state: ref mut hash::Hasher) -> void {
    hash::write<i64>(state, self.left);
    hash::write<i64>(state, self.right);
  }
}
```

## Current Limits

- The hasher is deterministic and non-cryptographic.
- The algorithm is a simple FNV-1a style byte mixer with a final avalanche.
  It is good enough as a stdlib seed, not a promise of long-term ABI-stable
  hash values.
- `HashMap` and `HashSet` still take explicit hash functions. Future
  trait-driven constructors can use `Hash[T]` once trait dispatch and equality
  policy are stronger.
- Hex/base64 encoding lives in `std::encoding`; keep `std::hash` focused on
  deterministic non-cryptographic hash values.

## Tests

```text
tests/cases/standard-library/ok/hash/std-hash-basic.ari
tests/cases/standard-library/ok/hash/std-hash-integer-widths.ari
tests/cases/standard-library/ok/hash/std-hash-combine-helpers.ari
```

The focused test covers hasher construction, reset, byte writes, byte-slice
hashing, primitive `Hash[T]` dispatch, and `collections::hash_i64`
compatibility. `std-hash-integer-widths.ari` checks fixed-width signed and
unsigned integer writers, generic `Hash[T]` dispatch, and width-distinct byte
feeds. `std-hash-combine-helpers.ari` checks `pair`, `combine`, ordered
composition, and the `Slice[u8]` `Hash` impl.

## Next Work

- Add derived `Hash` impl patterns for common aggregate/value types after
  derive and trait policy are settled.
- Add `HashMap`/`HashSet` constructors that use `Hash[T]` and `Eq[T]` instead
  of explicit hash functions.
- Add more non-integer `Hash` impls after aggregate/derive policy is settled.
