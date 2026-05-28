# Memory Direction Notes

## Core Philosophy

Ari should not copy the C/C++ model where most memory facts live in comments,
conventions, or programmer discipline.

It also should not hide memory policy behind a garbage collector.

The intended middle path is explicit ownership, explicit borrowing, explicit
allocation capabilities, and deterministic destruction.

## Ownership

An owning value should have one live owner. Moving the value consumes the old
binding. Dropping the value consumes it too.

This gives the compiler a clear state machine:

```text
uninitialized -> live -> moved
uninitialized -> live -> dropped
```

Assignments into owned storage are allowed only when the old value is already
gone.

## Borrowing

Borrowing should make aliasing visible:

- many `ref T`
- or one `ref mut T`
- never both at the same time

The current checker only supports temporary call borrows. Named borrow lifetimes
are the next hard step.

## Allocation

Allocation should be a value-level capability:

```ari
fn make_buffer(allocator: ref mut Allocator, len: i64) -> own Buffer
```

That keeps memory policy testable and local. Programs can choose arena, bump,
debug, fixed-buffer, or OS-backed allocation without the language requiring one
ambient heap.

Current direction: `Zone` is the first concrete region implementation, but the
public growth vocabulary should move toward `std::allocator::Allocator`.
`ZoneMetadata` exists as a compatibility bridge over today's allocation header;
ordinary library users should not need to name it.

## Regions

Regions are a good fit for short-lived allocation groups:

```ari
with region temp using allocator {
  let items = make_items(temp)
}
```

The spelling is not settled. The important rule is that a zone owns the large
area of storage and can release it all at once, while an `Allocator` capability
is what containers and strings use when they need more storage from the same
region. Borrow and ownership analysis can still warn about obvious escapes or
use-after-release cases, but Ari does not try to make raw memory fully safe.

Likely explicit memory operations:

- raw pointer dereference
- pointer casts
- byte-wise pointer offsets with `ptr_offset(pointer, bytes)`
- external calls with unchecked lifetime rules
- manual initialization state changes
- bypassing or manually driving drop checking
