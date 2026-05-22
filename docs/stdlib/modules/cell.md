# std::cell

`std::cell` provides interior-mutability and one-time initialization helpers.
Use it when the outer value should stay shared while a small piece of internal
state changes through a checked API.

## Purpose

The current module has four source-level building blocks:

- `Cell[T]`: simple replace/get/set storage for plain copy-like values.
- `RefCell[T]`: runtime borrow checking for one mutable borrow or many shared
  borrows.
- `OnceCell[T]`: a zone-backed slot that can be initialized once, then read
  through borrowed pointers.
- `Lazy[T]`: a `OnceCell[T]` plus a plain `fn() -> T` initializer, evaluated
  by `force()`.

`OnceCell[T]` and `Lazy[T]` allocate an internal slot in the caller's `Zone`.
They do not carry a redundant zone field; zone recovery comes from the backing
allocation header when later initialization needs the zone again.

## API

```ari
Cell::new<T>(value)
cell.get()
cell.set(value)
cell.replace(value)
cell.take()
cell.into_inner()

RefCell::new<T>(value)
cell.borrow()
cell.try_borrow()
cell.borrow_mut()
cell.try_borrow_mut()
cell.get_mut()
cell.replace(value)
cell.take()
cell.borrow_count()
cell.is_borrowed()

OnceCell::new<T>(ref mut zone)
once.set(value)
once.get()
once.get_mut()
once.get_or_init(op)
once.take()
once.replace(value)
once.is_initialized()
once.is_empty()

Lazy::new<T>(ref mut zone, op)
lazy.force()
lazy.get()
lazy.is_initialized()
```

`Ref[T]` and `RefMut[T]` are guard values returned by `RefCell`. Dropping a
guard updates the runtime borrow state. `try_borrow` and `try_borrow_mut`
return `Option` instead of panicking when the borrow state is incompatible.

## Example

```ari
fn make_value() -> i64 {
  return 42;
}

fn main() -> i64 {
  var zone = zone::create(64);

  let counter = Cell::new<i64>(1);
  assert(counter.replace(2) == 1);

  var state = RefCell::new<i64>(10);
  {
    var value = state.borrow_mut();
    value.set(20);
    drop value;
  }

  var once = OnceCell::new<i64>(ref mut zone);
  assert(read_i64(once.get_or_init(make_value)) == 42);

  let lazy = Lazy::new<i64>(ref mut zone, make_value);
  assert(read_i64(lazy.force()) == 42);

  zone::destroy(zone);
  return 0;
}
```

## Current Limits

- `Cell[T]` is intended for small copy-like values. Move-aware replacement
  contracts need the same future type-trait work as collection algorithms.
- `RefCell[T]` runtime borrow checking is local and source-level. It is not a
  thread-safe synchronization primitive.
- `OnceCell[T]` and `Lazy[T]` use plain function pointers for initialization.
  Capturing closures and initializer error policies are future work.
- The current handles do not free zone memory early. Zones reclaim backing
  storage on reset or destroy.

## Tests

Focused coverage lives in
`tests/cases/standard-library/ok/cell/std-cell-basic.ari`. It checks
`Cell`, `RefCell`, `OnceCell`, `Lazy`, guard drops, fallible borrows, and
lazy initialization.
