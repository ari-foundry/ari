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

Current direction: `Region` is the public lifetime owner, `Allocator` is the
growth capability, and `Zone` is the first concrete runtime implementation.
`ZoneMetadata` exists as a compatibility bridge over today's allocation
header; ordinary library users should not need to name it.

## Memory Model Review

The current zone implementation solved an important early problem: it gave the
compiler one concrete lifetime shape to track. The rough part is the public
surface. When docs and library code talk directly about `ZoneMetadata`, the
model starts to feel like "recover magic allocator state from a pointer header"
instead of "pass an allocation capability to code that allocates."

Rejected shapes:

- A magical process-global heap would make examples short, but it would hide
  memory policy, weaken reset/destroy diagnostics, and make embedded or
  freestanding profiles harder.
- A runtime-global `region::current()` function would look convenient, but it
  would make allocation depend on dynamic ambient state. Lexical current-zone
  insertion is easier to read, easier to diagnose, and easier for the checker
  to constrain.
- Pure `ZoneMetadata` everywhere avoids storing duplicate zone pointers, but it
  leaks allocation-header layout into normal API design. It is useful as a
  compatibility mechanism, not as the model users should learn first.
- Reference counting or garbage collection would reduce explicit region
  plumbing, but it would also change Ari's deterministic ownership story and
  require a much bigger runtime policy decision.

Chosen shape:

- `Region` names the owned bulk lifetime in user docs and new APIs.
- `Allocator` names the smaller capability that can allocate from a region or
  an existing region-backed handle.
- `Zone` remains the compiler/runtime primitive and compatibility spelling.
- `ZoneMetadata` remains available under `std::zone` for low-level tests and
  migration, but new user-facing examples should not require it.

This keeps the useful parts of zones, avoids a hidden global heap, and gives
the stdlib a more natural vocabulary for APIs that only need to allocate.

## Regions

Regions are a good fit for short-lived allocation groups:

```ari
region(8192) {
  let items = make_items()
}
```

The current source spelling is still `zone { ... }` because the compiler
feature predates the `Region` facade. The better user model is "a region owns
the large area of storage and can release it all at once, while an
`Allocator` capability is what containers and strings use when they need more
storage from the same region." Borrow and ownership analysis can still warn
about obvious escapes or use-after-release cases, but Ari does not try to make
raw memory fully safe.

The next ergonomic step should be a spelling that reads like region creation
rather than allocator metadata plumbing. Good candidates are:

- keep `zone { ... }` as compatibility syntax and document it as temporary
  region syntax
- add a future `region { ... }` alias once parser compatibility is clear
- allow constructors inside a current region to omit the region argument only
  when exactly one allocation lifetime is missing

Avoid a runtime-global `region::current()` API for ordinary library code. A
lexical current region is readable and checkable; an ambient global allocator
would make lifetime and reset behavior much harder to reason about.

Likely explicit memory operations:

- raw pointer dereference
- pointer casts
- byte-wise pointer offsets with `ptr_offset(pointer, bytes)`
- external calls with unchecked lifetime rules
- manual initialization state changes
- bypassing or manually driving drop checking
