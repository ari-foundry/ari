# std::rc

`std::rc` provides reference-counted shared ownership handles. Use it when a
value needs multiple owners and a non-owning handle should be able to test
whether the value is still alive.

## Purpose

The module exposes three handles:

- `Rc[T]`: single-threaded shared ownership vocabulary.
- `Arc[T]`: atomic shared ownership vocabulary for thread-facing APIs.
- `Weak[T]`: non-owning handle that does not keep the value alive and can
  upgrade back to `Option[Rc[T]]` or `Option[Arc[T]]`.

All three handles point at a zone-backed shared control block. The handle stores
only the control pointer. It does not store a redundant zone field; zone
metadata is recovered from the allocation header on the control block pointer.

## API

```ari
std::rc::rc<T>(ref mut zone, value) -> Rc[T]
std::rc::arc<T>(ref mut zone, value) -> Arc[T]

Rc::new<T>(ref mut zone, value)
rc.clone()
rc.downgrade()
rc.strong_count()
rc.weak_count()
rc.is_unique()
rc.get()
rc.as_ref()
rc.ptr_eq(ref other)

Arc::new<T>(ref mut zone, value)
arc.clone()
arc.downgrade()
arc.strong_count()
arc.weak_count()
arc.is_unique()
arc.get()
arc.as_ref()
arc.ptr_eq(ref other)

Weak::new<T>()
weak.clone()
weak.upgrade()
weak.upgrade_arc()
weak.is_empty()
weak.is_alive()
weak.strong_count()
weak.weak_count()
```

Dropping the last strong `Rc` or `Arc` drops the contained value exactly once.
`Weak` keeps only the control block alive as zone-owned storage; it cannot
recover a value after the strong count reaches zero.

## Example

```ari
fn main() -> i64 {
  var zone = zone::create(64);

  var owner = Rc::new<i64>(ref mut zone, 7);
  let other = owner.clone();
  let weak = owner.downgrade();

  assert(owner.strong_count() == 2);
  assert(weak.upgrade().is_some());

  drop other;
  drop owner;
  assert(weak.upgrade().is_none());

  zone::destroy(zone);
  return 0;
}
```

## Current Limits

- `Rc[T]` and `Arc[T]` currently share the same atomic control block. `Arc[T]`
  gives thread-facing API shape, but Ari still needs send/share trait policy
  before arbitrary values can safely cross threads.
- There is no `get_mut`/copy-on-write API yet. Use explicit uniqueness checks
  until the aliasing policy is nailed down.
- Control blocks remain zone-owned and are reclaimed by zone reset/destroy
  rather than individually freed when weak count reaches zero.
- `Shared[T]` remains a reserved root spelling. Use `Rc[T]` or `Arc[T]`
  explicitly so the intended ownership mode is visible.

## Tests

Focused coverage lives in
`tests/cases/standard-library/ok/rc/std-rc-arc-weak.ari`. It checks
construction, cloning, weak downgrade/upgrade, value reads, pointer equality,
last-strong drop behavior, and the empty weak sentinel.
