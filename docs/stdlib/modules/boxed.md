# std::boxed

`std::boxed` provides `Box[T]`, a zone-backed owner for exactly one value. Use
it when a value needs a stable address, must be moved in and out explicitly, or
should live in a caller-chosen `Zone` without hiding allocation behind a global
heap.

## Purpose

`Box[T]` is intentionally smaller than a general shared pointer:

- allocation is explicit through `ref mut Zone`
- region-first allocation is available through `Region::boxed` and
  allocator-based helpers
- the handle owns at most one value
- `try_get` reports an empty handle with `Option[T]`
- `take` moves the value out and leaves an empty handle
- `try_take` reports an empty handle with `Option[T]`
- `clear` drops the contained value and leaves the handle empty
- `put_in` refills an empty handle using the same tracked zone policy

Use `std::boxed` for single owned values. Use `std::vec` or
`std::collections` for many values, and use raw pointers only when FFI or
low-level memory work requires them.

## API

Construct a boxed value with a module function, an associated function, or the
root `Box!` macro:

```ari
var zone = zone::create(64);
var value = std::boxed::new<i64>(ref mut zone, 21);
var other = Box::new<i64>(ref mut zone, 34);
var macro_value = Box!(i64, ref mut zone, 55);
```

New code can keep the public lifetime at the `Region` layer:

```ari
var region = region::create(64);
var value = region.boxed<i64>(21);
let allocator = region.allocator();
var other = std::boxed::new_with_allocator<i64>(ref allocator, 34);
region::destroy(region);
```

Read and update the value with natural method names:

```ari
let before = value.get();
let optional_before = value.try_get();
value.set(9);
let previous = value.replace(13);
```

Move out of the handle when ownership should leave the box:

```ari
let moved = value.take();
if value.is_empty() {
  value.put_in(ref mut zone, moved + 1);
}
```

Use `try_take` when an empty handle is an ordinary case:

```ari
let maybe_value = value.try_take();
let fallback = maybe_value.unwrap_or(0);
```

Use `try_get` when reading from an empty handle is an ordinary branch. It
returns `Some(value)` while the handle contains a value and `None` after
`take` or `clear`:

```ari
let maybe_current = value.try_get();
let current_or_zero = maybe_current.unwrap_or(0);
```

Borrow or expose raw pointers only when the caller needs that shape:

```ari
let shared = value.as_ref();
let raw = value.as_ptr();
```

## Ownership And Zones

`Box[T]` remembers the zone provenance of its allocation. Values copied with
`copy_to(ref mut target)` belong to the target zone. Methods that can allocate,
such as `put_in`, take an explicit `ref mut Zone` so the allocation site is
visible at the call site. The allocator variants, such as
`put_with_allocator` and `copy_with_allocator`, are the migration path for
helper APIs that need allocation capability without owning region lifecycle.

After `zone::reset(zone)` or `zone::destroy(zone)`, using a box allocated from
that zone is invalid and sema should reject it. This is why `std::boxed` stays
explicit instead of pretending to be a garbage-collected reference.

The `Drop` implementation drops the contained value if the handle is not empty.
An empty handle can still be dropped safely.

## Current Limits

- `Box[T]` is not shared ownership. Use `std::rc::Rc`, `std::rc::Arc`, or
  `std::rc::Weak` when reference-counted ownership is the right model.
- Empty boxes exist only through operations such as `take` or `clear`; users
  should not construct invalid raw `data` fields directly.
- Raw pointer methods are sharp edges for low-level code. Prefer `as_ref`,
  `as_mut`, `get`, `try_get`, `set`, `replace`, `take`, and `try_take` in
  ordinary code.
- The current handle does not free zone memory early; zones reclaim memory on
  reset or destroy.

## Tests

Focused tests live under `tests/cases/standard-library/ok/boxed/` and
`tests/cases/standard-library/errors/boxed/`.

Representative ok tests:

- `std-boxed-box.ari`: construction, `get`, and `set`
- `std-boxed-take.ari`: move-out and empty-handle state
- `std-boxed-try-get.ari`: `Option`-returning empty-handle read
- `std-boxed-try-take.ari`: `Option`-returning empty-handle flow
- `std-boxed-put-in.ari`: refill after `take`
- `std-boxed-copy-to.ari`: target-zone copy behavior
- `std-boxed-as-ref-mut.ari`: borrowed value access
- `prelude-box-root.ari`: root alias and associated constructor behavior

Representative error tests:

- `std-boxed-box-after-reset.ari`
- `std-boxed-put-in-different-zone.ari`
- `std-boxed-put-in-untracked.ari`
- `std-boxed-as-ref-mut-conflict.ari`
- `std-boxed-drop-use-after.ari`

These tests make `std::boxed` an allocation/ownership module, not just a small
pointer wrapper.
