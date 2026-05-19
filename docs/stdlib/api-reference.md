# Standard Library API Reference

This is a compact guide to the current public `std` surface. The source of
truth is still `lib/std.arih`, `lib/std/*.arih`, and
`tests/std_api_manifest.txt`.

## Prelude Root

Common programs can use these names through implicit `std` loading:

```ari
Option[T]
Result[T, E]
Slice[T]
Range[T]
RangeInclusive[T]
Box[T]
String
Thread
AtomicI64
Set[T]
Deque[T]
RingBuffer[T]
LinkedList[T]
BinaryHeap[T]
PriorityQueue[T]
HashMap[K, V]
HashSet[T]
TreeMap[K, V]
TreeSet[T]
std::Vec[T]
move(value)
take(place)
assert(condition)
debug_assert(condition)
panic()
todo()
unreachable()
```

Root aliases include IO, context, memory, comparison, zone, and range helpers:

```ari
write_i64(value)
write_u64(value)
write_bool(value)
write_byte(value)
write_bytes(values)
newline()
read_byte()
read_line()
read_line_owned(ref mut zone)
input()
input_owned(ref mut zone)
arg_count()
arg(index)
has_arg(index)
size_of<T>()
align_of<T>()
ptr_add(pointer, count)
ptr_load(pointer)
ptr_store(pointer, value)
replace(ref mut place, value)
swap(ref mut left, ref mut right)
min<T>(left, right)
max<T>(left, right)
clamp<T>(value, low, high)
is_between<T>(value, low, high)
create(capacity)
alloc(ref mut zone, bytes, align)
alloc_array<T>(ref mut zone, count)
new<T>(ref mut zone, value)
promote<T>(ref mut target, source)
reset(ref mut zone)
destroy(zone)
range(start, end)
range_inclusive(start, end)
```

## Process Context And Environment

Runtime-backed context access lives in `std::context`:

```ari
context::argc()
context::arg(index)
context::thread_id()
context::has_args()
context::has_arg(index)
context::user_arg_count()
context::has_user_args()
context::is_main_thread()
arg_count()
arg(index)
has_arg(index)
```

`has_arg(index)` is true only for `0 <= index < context::argc()`. It is the
preferred low-level guard before reading optional host arguments. `arg(index)`
returns a lowercase `string`; out-of-range access returns an empty string.
`user_arg_count()` excludes `argv[0]`, `has_user_args()` is its boolean form,
and `thread_id()` returns the Ari runtime thread id. The main thread is `0`, so
`is_main_thread()` is true for current executable builds.

Application code should usually use the user-facing `std::env` wrappers:

```ari
env::arg_count()
env::arg(index)
env::has_arg(index)
env::try_arg(index)
env::program_name()
env::get(name)
env::has(name)
env::try_get(name)
env::set(name, value)
env::remove(name)
env::current_dir()
env::try_current_dir()
env::set_current_dir(path)
env::executable_path()
env::try_executable_path()
```

`env::try_arg(index)` returns `Option[string]`, and `env::program_name()` is
the optional `argv[0]` value.

`env::try_get(name)` returns `Option[string]` for environment variables.
`env::get(name)` returns an empty string when the variable is missing, so prefer
`try_get` or `has` when absence matters. `env::set(name, value)` overwrites a
current-process variable and `env::remove(name)` unsets it; both return whether
the host accepted the request. `env::current_dir()` and
`env::executable_path()` return borrowed runtime strings, with `try_*` wrappers
for ordinary failure; `env::set_current_dir(path)` mutates the current process
working directory. Portable child-process spawn handles remain roadmap work;
thread helpers live in `std::thread`.

Target and platform facts live in `std::target`:

```ari
target::triple()
target::arch()
target::arch_name()
target::os()
target::os_name()
target::env()
target::env_name()
target::object_format()
target::debug_format()
target::errno_abi()
target::pointer_bits()
target::long_bits()
target::syscall_abi()
target::is_x86_64()
target::is_aarch64()
target::is_riscv64()
target::is_linux()
target::is_macos()
target::is_windows()
target::is_unix()
target::uses_glibc()
target::uses_musl()
target::uses_elf()
target::uses_dwarf()
target::uses_posix_errno()
target::has_tls()
target::has_vdso()
target::has_procfs()
target::has_sysfs()
target::has_epoll()
target::has_inotify()
target::has_eventfd()
target::has_timerfd()
target::has_signalfd()
target::has_memfd()
target::has_pidfd_api()
target::has_fanotify_api()
target::has_io_uring_api()
target::has_cgroups_api()
target::has_namespaces_api()
target::has_seccomp_api()
target::has_capabilities_api()
```

`std::target` describes the selected compile target. It does not promise that
a mounted filesystem, kernel version, process capability, or linker hardening
mode is active at runtime.

Process helpers live in `std::process`:

```ari
process::id()
process::uid()
process::gid()
process::exit(code)
process::abort()
process::success()
process::failure()
process::is_success(code)
process::is_failure(code)
process::is_root()
process::fork()
process::wait(pid)
process::is_child(pid)
process::is_parent(pid)
process::is_fork_error(pid)
process::is_wait_error(status)
```

`id()` returns the host process id as `i64`. `uid()` and `gid()` return the
current user and group ids. `is_root()` is the source convenience check for
`uid() == 0`. `exit(code)` terminates the process and does not return.
`abort()` terminates through the host abnormal-termination path. The status
helpers are source functions for the common `0` success and `1` failure
convention.

`fork()` and `wait(pid)` are the first POSIX child-process slice on the
Linux/LLVM runtime path. `fork()` returns `0` in the child, a positive child pid
in the parent, and a negative value on failure; use `is_child`, `is_parent`,
and `is_fork_error` to make that branch readable. `wait(pid)` returns a normal
child exit status or `-1`; use `is_wait_error` for that sentinel. Rich process
handles, portable spawn, and detailed status values remain roadmap work.

## Paths

`std::path` contains source-only lexical path helpers over `Slice[u8]`:

```ari
path::is_separator(byte)
path::is_absolute(path)
path::is_relative(path)
path::trim_trailing_separators(path)
path::components(path)
path::file_name(path)
path::parent(path)
path::extension(path)
path::stem(path)
path::join_in(ref mut zone, base, child)
path::normalize_in(ref mut zone, path)
```

The current separator policy is POSIX-style `/` only. Single-component helpers
return `Option[Slice[u8]]` views into the original path bytes.
`components(path)` returns a lazy iterator over non-empty borrowed components
and skips leading, repeated, and trailing separators.
`join_in` and `normalize_in` allocate byte strings in the caller-provided
zone. Normalization collapses repeated separators and removes `.` components,
but keeps `..` components because resolving them safely depends on stronger
filesystem and platform policy.

Thread helpers live in `std::thread`:

```ari
thread::spawn(entry)
thread::join(thread)
thread::yield_now()
thread::id()
thread::is_main()
thread::is_join_error(status)

Thread::spawn(entry)
Thread::invalid()
thread.id()
thread.is_valid()
thread.join()
```

`spawn(entry)` starts a thread from a plain `fn() -> i64` entry function and
returns a `Thread` value handle. `join(thread)` waits for the handle and
returns the entry function's `i64` result, or `-1` for the current failure
sentinel; use `is_join_error(status)` for that check. `id()` returns the
current Ari runtime thread id, with main thread `0` and spawned threads
positive. `yield_now()` is a host scheduler hint, not synchronization.

Synchronization helpers live in `std::sync`:

```ari
AtomicI64::new(value)

sync::load(ref value)
sync::store(ref mut value, replacement)
sync::swap(ref mut value, replacement)
sync::fetch_add(ref mut value, amount)
sync::compare_exchange(ref mut value, expected, replacement)

atomic.load()
atomic.store(replacement)
atomic.swap(replacement)
atomic.fetch_add(amount)
atomic.compare_exchange(expected, replacement)
```

The first sync slice is concrete: `AtomicI64` only. Operations use
sequentially consistent ordering and keep the API name natural by putting the
type in the value, not in every method name. `fetch_add` and `swap` return the
previous value; `compare_exchange` returns whether the replacement happened.
`Shared`, `Weak`, `Mutex`, channels, and weaker memory-order options remain
future concurrency work.

Runtime-backed time helpers live in `std::time`:

```ari
time::monotonic_nanos()
time::unix_nanos()
time::sleep_nanos(nanos)

time::nanoseconds(value)
time::microseconds(value)
time::milliseconds(value)
time::seconds(value)
time::now()
time::system_now()
time::elapsed(start)
time::sleep(duration)

Duration::zero()
duration.as_nanos()
duration.as_micros()
duration.as_millis()
duration.as_seconds()
duration.is_zero()
duration.add(other)
duration.saturating_sub(other)

Instant::now()
instant.as_nanos()
instant.duration_since(earlier)
instant.try_duration_since(earlier)
instant.elapsed()

SystemTime::now()
system_time.as_unix_nanos()
system_time.duration_since_unix_epoch()
```

Use `Instant` for elapsed time and `SystemTime` for Unix wall-clock
timestamps. `Duration` constructors assert on negative values. The raw
`*_nanos` functions are exposed for low-level code, but ordinary code should
prefer `now`, `system_now`, `elapsed`, and `sleep`.

Filesystem helpers live in `std::fs`:

```ari
fs::exists(path)
fs::can_read(path)
fs::can_write(path)
fs::can_execute(path)
fs::permissions(path)
fs::remove(path)
fs::rename(source, target)
fs::hard_link(existing, link_path)
fs::symbolic_link(target, link_path)
fs::create_dir(path)
fs::remove_dir(path)
fs::open(path, mode)
fs::create(path)
fs::open_read(path)
fs::open_write(path)
fs::open_append(path)
fs::try_open(path, mode)
fs::try_create(path)
fs::try_open_read(path)
fs::try_open_write(path)
fs::try_open_append(path)
fs::close(file)
fs::read_byte(file)
fs::write_byte(file, value)
fs::write_bytes(file, values)
fs::read(ref mut zone, path)
fs::write(path, values)
fs::append(path, values)
fs::truncate(path)
fs::copy(source, target)
fs::read_to_string(ref mut zone, path)

File::invalid()
file.is_open()
file.close()
file.read_byte()
file.write_byte(value)
file.write_bytes(values)

Permissions::none()
permissions.can_read()
permissions.can_write()
permissions.can_execute()
permissions.any()
```

Use `try_open(path, mode)` for ordinary fallible open operations; it returns
`Option[File]`. The raw `open(path, mode)` call returns a `File` directly,
with `File::invalid()` and `file.is_open()` exposing the invalid-handle
convention. Supported modes are `"r"` for read, `"w"` for create/truncate
write, `"a"` for create/append write, `"rw"` for existing read/write, `"r+"`
as a familiar alias for `"rw"`, `"w+"` for create/truncate read/write, and
`"a+"` for read/append. `open_read`, `open_write`, `open_append`, and their
`try_open_*` variants are compatibility wrappers over those mode strings.
`create(path)` and `try_create(path)` are the natural create/truncate helpers
over `"w"` mode. `can_read`, `can_write`, and `can_execute` are access-style
preflight checks for the current process. `permissions(path)` snapshots those
three checks into a `Permissions` value; still handle later open/read/write
failures because filesystem access can change after the check.
`read_byte` returns an `i64` byte value or `-1` at EOF/failure, and
`write_byte` returns whether one byte was written. `write_bytes` writes a
`Slice[u8]` and returns the count written before the first failed byte write.
`write(path, values)` truncates or creates a small byte file and writes the
whole `Slice[u8]`; `append(path, values)` creates if needed and appends the
whole slice. `read(ref mut zone, path)` is the short alias for
`read_to_string(ref mut zone, path)`, returning a zone-backed byte-oriented
`std::string::String` and using an empty `String` when the file cannot be
opened. `truncate(path)` creates or empties a file. `copy(source, target)`
streams bytes from the source handle into the target opened with truncating
semantics. `rename(source, target)` moves or renames one path according to the
host runtime's current behavior. `create_dir(path)` creates one directory and
`remove_dir(path)` removes one empty directory; recursive directory helpers and
directory iteration are future work.
The current `File` value is not an owned resource yet, so close each successful
handle once and do not reuse copied handles after closing.

Network address helpers live in `std::net`:

```ari
net::Ipv4Addr
net::Ipv6Addr
net::IpAddr
net::SocketAddr

net::ipv4(a, b, c, d)
net::ipv6(s0, s1, s2, s3, s4, s5, s6, s7)
net::socket_addr(ip, port)
net::localhost(port)

Ipv4Addr::new(a, b, c, d)
Ipv4Addr::any()
Ipv4Addr::localhost()
ipv4.octet(index)
ipv4.is_unspecified()
ipv4.is_loopback()
ipv4.as_ip()

Ipv6Addr::new(s0, s1, s2, s3, s4, s5, s6, s7)
Ipv6Addr::any()
Ipv6Addr::localhost()
ipv6.segment(index)
ipv6.is_unspecified()
ipv6.is_loopback()
ipv6.as_ip()

ip.is_v4()
ip.is_v6()
ip.is_unspecified()
ip.is_loopback()

SocketAddr::new(ip, port)
SocketAddr::localhost(port)
addr.ip()
addr.port()
addr.with_port(port)
addr.is_unspecified()
addr.is_loopback()
```

The current network slice is source-only and deterministic. It does not do DNS
lookup, open sockets, or touch the host network. TCP listeners/streams, UDP
sockets, Unix domain sockets, socket options, nonblocking mode, timeouts, and
shutdown are roadmap work for the runtime-backed `std::net` handle layer.

## IO And Input

`std::io` exposes low-level process IO hooks and a small trait surface for
byte readers/writers, while `std::input` keeps stdin-oriented helper names:

```ari
io::Reader
io::Writer
io::Seek
io::Stdin
io::Stdout
io::Stderr
io::Cursor
io::BufReader[R]
io::BufWriter[W]
io::stdin()
io::stdout()
io::stderr()
io::cursor(values)
io::buf_reader[R: Reader](inner, buffer)
io::buf_writer[W: Writer](inner, buffer)
io::read_exact[R: Reader](reader: ref mut R, output, len)
io::write_all[W: Writer](writer: ref mut W, values)
io::flush[W: Writer](writer: ref mut W)
io::write_i64(value)
io::write_u64(value)
io::write_bool(value)
io::write_byte(value)
io::write_bytes(values)
io::newline()
io::read_byte()
io::read_line()
io::read_line_owned(ref mut zone)

input::read_byte()
input::try_read_byte()
input::line()
input::owned_line(ref mut zone)
```

`read_byte` returns an `i64` byte value or `-1` at EOF.
`input::try_read_byte()` wraps that shape as `Option[u8]`. `write_bytes`
writes every byte in a `Slice[u8]` and returns the byte count attempted.
Borrowed line input uses a reusable runtime buffer; use the owned forms when
the line must survive later input reads.

`io::Cursor` implements `Reader` and `Seek` over a borrowed `Slice[u8]`.
`io::Stdout` and `io::Stderr` implement `Writer` over the current process
stream hooks, with `flush` currently succeeding as a no-op. `io::BufReader`
and `io::BufWriter` wrap any `Reader` or `Writer` with an explicit
caller-provided `Slice[u8]` buffer, so allocation and buffer lifetime stay
visible. `pipe`, file adapters, zone-owning buffered constructors, and
drop-time writer flush remain roadmap items until owned OS handles and generic
resource policy are specified.

## Memory And Zones

`std::mem` exposes layout, raw pointer, and byte memory helpers:

```ari
mem::size_of<T>()
mem::align_of<T>()
mem::ptr_offset<T>(pointer, bytes)
mem::ptr_add<T>(pointer, count)
mem::ptr_load<T>(pointer)
mem::ptr_store<T>(pointer, value)
mem::copy_bytes(target, source, len)
mem::move_bytes(target, source, len)
mem::set_bytes(target, value, len)
mem::replace<T>(ref mut place, value)
mem::swap<T>(ref mut left, ref mut right)
```

`copy_bytes`, `move_bytes`, and `set_bytes` operate on `ptr u8` and byte
lengths. `copy_bytes` is for non-overlapping regions, `move_bytes` permits
overlap, and `set_bytes` fills a region with one byte. They lower through LLVM
memory intrinsics and trap on negative lengths.

`std::zone` exposes the explicit allocation capability:

```ari
zone::create(capacity)
zone::alloc(ref mut zone, bytes, align)
zone::alloc<T>(ref mut zone)
zone::alloc_array<T>(ref mut zone, count)
zone::new<T>(ref mut zone, value)
zone::promote<T>(ref mut target, source)
zone::allocation_zone(data)
zone::reset(ref mut zone)
zone::destroy(zone)
```

`alloc_array<T>` returns uninitialized storage for `count` consecutive `T`
values. It returns null for `0`, asserts for negative counts, and does not run
destructors for the slots; initialize before reading and prefer higher-level
handles when ownership matters.

## Option And Result

`Option[T]` models a present or missing value:

```ari
Some(value)
None<T>()
value.is_some()
value.is_none()
value.is_some_and(op)
value.is_none_or(op)
value.unwrap_or(fallback)
value.unwrap_or_else(op)
value.unwrap()
value.expect("message")
value.map<U>(op)
value.and_then<U>(op)
value.filter(op)
value.flatten()
value.transpose()
value.or(fallback)
value.or_else(op)
value.xor(other)
value.ok_or<E>(error)
value.ok_or_else<E>(op)
```

`Result[T, E]` models success or failure:

```ari
Ok<T, E>(value)
Err<T, E>(error)
value.is_ok()
value.is_err()
value.is_ok_and(op)
value.is_err_and(op)
value.unwrap_or(fallback)
value.unwrap_or_else(op)
value.unwrap()
value.expect("message")
value.unwrap_err()
value.expect_err("message")
value.ok()
value.err()
value.map<U>(op)
value.map_err<F>(op)
value.and_then<U>(op)
value.or<F>(fallback)
value.or_else<F>(op)
value.transpose()
```

Use `is_some_and`, `is_none_or`, `is_ok_and`, and `is_err_and` when a branch
depends on both the enum case and a payload predicate. These helpers consume
the enum value and pass the payload to the predicate. `Option::flatten` is
available on `Option[Option[T]]` and removes one optional layer. `filter`
keeps a `Some(T)` only when a borrowed `fn(ref T) -> bool` predicate accepts
the payload.
`Option::transpose` is available on `Option[Result[T, E]]` and turns optional
fallible work into fallible optional work. Use the lazy `*_else` forms when the
fallback is expensive or should only run on the missing/error branch. Use
`ok_or` and `ok_or_else` when an optional value needs to enter a
`Result`-returning flow. Use `ok` and `err` when a `Result` should be projected
back into an `Option`. Use `Result::or` when a fallback `Result` is already
available; use `Result::or_else` when the fallback should be built from the
error only on the `Err` branch. `Result::transpose` is available on
`Result[Option[T], E]` and turns fallible optional work back into optional
fallible work.

## Slice, Vec, Set, String, And Box

`Slice[T]` is a borrowed contiguous view:

```ari
slice(pointer, len)
view.len()
view.is_empty()
view.first()
view.try_first()
view.last()
view.try_last()
view.get(index)
view.try_get(index)
view[index]
view.as_ptr()
view.contains(value)
view.index_of(value)
view.count(value)
view.starts_with(other)
view.ends_with(other)
view.equals(other)
view.copy_to(ref mut zone)
```

`first`, `last`, and `get` assert when the requested element does not exist.
Use `try_first`, `try_last`, and `try_get` when absence is an ordinary branch;
they return `Option[T]`. `is_empty` is a source method that borrows the view
and checks whether the stored length is zero.

`std::vec::Vec[T]` is the source growable sequence:

```ari
std::vec::new<T>(ref mut zone, capacity)
Vec!(T, ref mut zone, capacity)
vec.len()
vec.capacity()
vec.is_empty()
vec.first()
vec.try_first()
vec.last()
vec.try_last()
vec.get(index)
vec.try_get(index)
vec.get_ref(index)
vec.get_mut(index)
vec.push(value)
vec.push_in(ref mut zone, value)
vec.pop()
vec.try_pop()
vec.set(index, value)
vec.replace(index, value)
vec.swap(left, right)
vec.insert(index, value)
vec.insert_in(ref mut zone, index, value)
vec.remove(index)
vec.truncate(length)
vec.clear()
vec.reserve(ref mut zone, capacity)
vec.reserve_extra(ref mut zone, additional)
vec.extend_from_slice_in(ref mut zone, values)
vec.resize_in(ref mut zone, length, value)
vec.as_slice()
vec.as_ptr()
vec.as_mut_ptr()
vec.copy_to(ref mut zone)
vec.iter()
```

The `try_*` accessors return `Option[T]` for empty or out-of-range reads.
Use the non-`try` forms when absence is a programmer error and an assertion is
the desired behavior.

`std::collections::Set[T]` is a zone-backed linear set:

```ari
collections::new<T>(ref mut zone, capacity)
Set::new<T>(ref mut zone, capacity)
collections::from_slice_in<T>(ref mut zone, values)
set.len()
set.capacity()
set.is_empty()
set.first()
set.try_first()
set.last()
set.try_last()
set.get(index)
set.try_get(index)
set.index_of(value)
set.contains(value)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.remove(value)
set.take(value)
set.pop()
set.try_pop()
set.clear()
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
set.as_slice()
set.iter()
set.copy_to(ref mut zone)
```

`insert` returns `true` only for newly inserted values. `replace` returns
`Some(previous)` for an equal existing value, or inserts the missing value and
returns `None`. `remove` drops the removed value and reports whether it was
present; `take` returns `Option[T]`. `first`/`last`/`get` assert that the
requested element exists,
while `try_first`/`try_last`/`try_get` return `Option[T]`. `pop` removes the
last insertion-order value, and `try_pop` returns `None` on an empty set.
`reserve` and `reserve_extra` keep growth explicit through the same source
zone. The set preserves insertion order in accessors, `index_of`, `as_slice`,
`iter`, and `copy_to`. `std::collections::Iter[T]` implements `Iterator[T]`,
and `Set[T]` implements `IntoIterator[T]`, so `for value in set` works through
the standard iterator path.

Double-ended and bounded sequence collections:

```ari
collections::deque<T>(ref mut zone, capacity)
Deque::new<T>(ref mut zone, capacity)
deque.push_front(ref mut zone, value)
deque.push_back(ref mut zone, value)
deque.pop_front()
deque.try_pop_front()
deque.pop_back()
deque.try_pop_back()
deque.front()
deque.back()
deque.get(index)
deque.try_get(index)
deque.iter()

collections::ring_buffer<T>(ref mut zone, capacity)
RingBuffer::new<T>(ref mut zone, capacity)
buffer.push(value)
buffer.push_overwrite(value)
buffer.pop()
buffer.try_pop()
buffer.peek()
buffer.try_peek()
buffer.get(index)
buffer.try_get(index)
buffer.is_full()
buffer.iter()
```

`Deque[T]` grows with the same zone when either end needs more room.
`RingBuffer[T]` is fixed-capacity: `push` returns `false` when full, and
`push_overwrite` returns the overwritten oldest value when it has to make room.

Linked lists and priority queues:

```ari
collections::linked_list<T>(ref mut zone, capacity)
LinkedList::new<T>(ref mut zone, capacity)
list.push_front(ref mut zone, value)
list.push_back(ref mut zone, value)
list.pop_front()
list.pop_back()
list.remove_at(index)
list.try_remove_at(index)
list.iter()

collections::binary_heap<T>(ref mut zone, capacity, less)
BinaryHeap::new<T>(ref mut zone, capacity, less)
collections::priority_queue<T>(ref mut zone, capacity, less)
PriorityQueue::new<T>(ref mut zone, capacity, less)
heap.push(ref mut zone, value)
heap.peek()
heap.try_peek()
heap.pop()
heap.try_pop()
```

`LinkedList[T]` uses zone-backed index nodes and reuses removed node slots.
`BinaryHeap[T]` and `PriorityQueue[T]` use `less(a, b)` as a lower-priority
predicate, so `collections::less_i64` makes larger integers pop first.

Hash-table collections use explicit hash functions:

```ari
collections::hash_i64(value)
collections::hash_map<K, V>(ref mut zone, capacity, hash)
HashMap::new<K, V>(ref mut zone, capacity, hash)
map.len()
map.capacity()
map.is_empty()
map.contains(key)
map.get(key)
map.try_get(key)
map.insert(ref mut zone, key, value)
map.remove(key)
map.clear()
map.reserve(ref mut zone, capacity)
map.keys()
map.values()

collections::hash_set<T>(ref mut zone, capacity, hash)
HashSet::new<T>(ref mut zone, capacity, hash)
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.take(value)
set.remove(value)
set.clear()
set.reserve(ref mut zone, capacity)
set.iter()
```

`collections::hash_i64` is a compatibility helper over `std::hash::value<i64>`.

`HashMap.insert` returns `Option[V]` with the previous value on replacement.
`HashMap.remove` returns `Option[V]` and leaves a tombstone so later probes
still find collided keys. `HashMap.keys()` and `HashMap.values()` iterate live
buckets; this is deterministic for the table state, but it is not insertion
order. `HashSet.iter()` and direct `for value in set` use the same live-bucket
cursor.

Tree collections use explicit strict less-than comparators:

```ari
collections::less_i64(left, right)
collections::tree_map<K, V>(ref mut zone, capacity, less)
TreeMap::new<K, V>(ref mut zone, capacity, less)
map.len()
map.capacity()
map.is_empty()
map.contains(key)
map.get(key)
map.try_get(key)
map.insert(ref mut zone, key, value)
map.clear()
map.reserve(ref mut zone, capacity)
map.keys()
map.values()

collections::tree_set<T>(ref mut zone, capacity, less)
TreeSet::new<T>(ref mut zone, capacity, less)
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.clear()
set.reserve(ref mut zone, capacity)
set.iter()
```

`TreeMap.keys()`, `TreeMap.values()`, `TreeSet.iter()`, and direct
`for value in tree_set` walk values in ascending comparator order.

`std::string::String` is an owned byte string:

```ari
std::string::new(ref mut zone, capacity)
std::string::from_string(ref mut zone, "text")
std::string::from_slice_in(ref mut zone, bytes)
text.len()
text.capacity()
text.is_empty()
text.first()
text.try_first()
text.last()
text.try_last()
text.get(index)
text.try_get(index)
text.set(index, byte)
text.replace(index, byte)
text.push(byte)
text.push_in(ref mut zone, byte)
text.pop()
text.try_pop()
text.insert(index, byte)
text.insert_in(ref mut zone, index, byte)
text.clear()
text.truncate(length)
text.reserve(ref mut zone, capacity)
text.reserve_extra(ref mut zone, additional)
text.extend_from_slice_in(ref mut zone, bytes)
text.resize_in(ref mut zone, length, byte)
text.index_of(byte)
text.contains(byte)
text.count(byte)
text.starts_with(bytes)
text.ends_with(bytes)
text.equals(bytes)
text.equals_ignore_case(bytes)
text.starts_with_ignore_case(bytes)
text.ends_with_ignore_case(bytes)
text.index_of_ignore_case(bytes)
text.contains_ignore_case(bytes)
text.append_string_in(ref mut zone, "text")
text.append_i64_in(ref mut zone, value)
text.append_u64_in(ref mut zone, value)
text.append_bool_in(ref mut zone, value)
text.append_f32_in(ref mut zone, value, precision)
text.append_f64_in(ref mut zone, value, precision)
text.trim_start()
text.trim_start_to(ref mut zone)
text.trim_end()
text.trim_end_to(ref mut zone)
text.trim()
text.trim_to(ref mut zone)
text.parse_decimal()
text.parse_decimal_prefix()
text.parse_hex()
text.parse_hex_prefix()
text.as_slice()
text.as_ptr()
text.copy_to(ref mut zone)
```

`String` stores bytes, so `equals_ignore_case`, `starts_with_ignore_case`,
`ends_with_ignore_case`, `index_of_ignore_case`, `contains_ignore_case`,
`trim_start`, `trim_end`, `trim`, `parse_decimal`, `parse_decimal_prefix`,
`parse_hex`, and `parse_hex_prefix` intentionally reuse `std::ascii` behavior.
The `try_*` byte accessors return `Option[u8]` for empty or out-of-range
access. The plain trim methods return borrowed `Slice[u8]` views, while
`trim_start_to`, `trim_end_to`, and `trim_to` copy the trimmed bytes into a
target zone and return owned `String` handles. The whole parse methods require
the whole string to be valid and return `Option[i64]`; prefix parsers return
`Option[std::ascii::ParsedInt]` and stop before the first invalid byte. Trim
first when leading or trailing ASCII whitespace should be ignored.

`std::boxed::Box[T]` is a zone-backed single-value owner:

```ari
std::boxed::new<T>(ref mut zone, value)
Box!(T, ref mut zone, value)
box.get()
box.set(value)
box.replace(value)
box.take()
box.try_take()
box.clear()
box.put_in(ref mut zone, value)
box.copy_to(ref mut zone)
box.as_ref()
box.as_mut()
box.as_ptr()
box.as_mut_ptr()
box.swap(ref mut other)
box.is_empty()
```

## Iteration And Formatting

`std::iter` contains the shared loop traits and range constructors:

```ari
iter::range<T>(start, end)
iter::range_inclusive<T>(start, end)
iter::map<T, U, I: std::Iterator[T]>(iter, op)
iter::filter<T, I: std::Iterator[T]>(iter, keep)
iter::take<T, I: std::Iterator[T]>(iter, count)
iter::skip<T, I: std::Iterator[T]>(iter, count)
iter::enumerate<T, I: std::Iterator[T]>(iter)
iter::zip<T, U, I: std::Iterator[T], J: std::Iterator[U]>(left, right)
iter::fold<T, U, I: std::Iterator[T]>(iter, initial, op)
iter::reduce<T, I: std::Iterator[T]>(iter, op)
iter::collect<T, I: std::Iterator[T]>(ref mut zone, iter)
iter::Iterator[T]
iter::IntoIterator[T]
iter::Iterable[T]
```

Root aliases expose `range(start, end)` and `range_inclusive(start, end)`.
Source cursors implement `Iterator[T]::next(self: ref mut Self) -> Option[T]`.
Collections that implement `IntoIterator[T]` can be used directly in `for`
loops; map-like collections use explicit `keys()` and `values()` cursors until
pair or tuple iteration policy is stable.

The adapter constructors are lazy except for `fold`, `reduce`, and `collect`.
Use `skip` for the usual drop-count adapter because `drop` is a language
operation. `collect` builds a `std::vec::Vec[T]` in the caller-provided zone.

`std::fmt` contains the formatting trait surface:

```ari
fmt::Debug
fmt::Display
Display::format_in(self: ref Self, zone: ref mut Zone)
```

The executable formatting path is still macro-based: `print!`, `println!`,
and `format_in!(ref mut zone, "...", values...)`. Use `format_in!` for owned
formatted strings because Ari does not hide a default allocation zone.

## Comparison

`std::cmp` contains source comparison traits and generic value helpers:

```ari
cmp::Eq[T]
cmp::PartialEq[T]
cmp::Ord[T]
cmp::PartialOrd[T]
cmp::min<T>(left, right)
cmp::max<T>(left, right)
cmp::clamp<T>(value, low, high)
cmp::is_between<T>(value, low, high)
```

`Ord[T]` currently requires `lt(self, other: T) -> bool`. `min`, `max`,
`clamp`, and `is_between` use that trait bound, so custom ordered values need
an `impl cmp::Ord[T] for T`. `clamp` and `is_between` assert that
`low <= high`; `is_between` is inclusive at both ends.

The root prelude re-exports the value helpers as `min<T>`, `max<T>`,
`clamp<T>`, and `is_between<T>`.

## Algorithms

`std::algo` contains source algorithms over borrowed `Slice[T]` views:

```ari
algo::sort<T>(values)
algo::sort_by<T>(values, less)
algo::stable_sort<T>(values)
algo::stable_sort_by<T>(values, less)
algo::binary_search<T>(values, target)
algo::is_sorted<T>(values)
algo::reverse<T>(values)
algo::rotate_left<T>(values, count)
algo::rotate_right<T>(values, count)
algo::partition<T>(values, keep)
algo::min<T>(values)
algo::max<T>(values)
algo::clamp<T>(value, low, high)
algo::swap<T>(values, left, right)
algo::fill<T>(values, value)
algo::copy<T>(target, source)
algo::dedup<T>(values)
```

The ordered helpers use `cmp::Ord[T]`; the `*_by` helpers take an explicit
`fn(T, T) -> bool` comparator. `binary_search` returns `Option[i64]`.
`partition` accepts `fn(ref T) -> bool` and returns the split index. `copy`
returns the number of copied elements. `dedup` compacts consecutive duplicates
and returns the logical prefix length.

## Hashing

`std::hash` contains deterministic, non-cryptographic hashing helpers:

```ari
hash::Hasher
hash::Hash[T]
hash::new()
hash::reset(ref mut state)
hash::finish(ref state)
hash::write<T>(ref mut state, value)
hash::value<T>(value)
hash::bytes(values)
hash::write_byte(ref mut state, value)
hash::write_bytes(ref mut state, values)
hash::write_u64(ref mut state, value)
hash::write_i64(ref mut state, value)
hash::write_bool(ref mut state, value)
```

Use `hash::value<T>` for a single value with a `Hash[T]` impl, `hash::bytes`
for a borrowed `Slice[u8]`, and `Hasher` plus `write` calls for incremental
hashing. Current built-in impls cover `i64`, `u64`, `u8`, and `bool`.

## Conversion

`std::convert` contains explicit conversion trait names and source helper
functions:

```ari
convert::From[T]
convert::Into[T]
convert::TryFrom[T]
convert::TryInto[T]
convert::identity<T>(value)
convert::from<T, U>(value)
convert::into<T, U>(value)
```

`identity` returns its input unchanged. `from<T, U>` builds destination `U`
through `convert::From[T]`, and `into<T, U>` turns source `U` into destination
`T` through `convert::Into[T]`. `TryFrom` and `TryInto` are reserved trait
names only for now; fallible conversion methods are future library work.

## Math

`std::math` currently contains conservative source-only helpers with `i64`
signatures. The names intentionally avoid type suffixes so they can grow into
generic numeric APIs later without changing call sites:

```ari
math::abs(value)
math::sign(value)
math::is_positive(value)
math::is_negative(value)
math::is_zero(value)
math::is_even(value)
math::is_odd(value)
math::pow(base, exponent)
math::div_floor(numerator, denominator)
math::div_ceil(numerator, denominator)
math::mod_floor(numerator, denominator)
math::gcd(left, right)
math::lcm(left, right)
```

`is_positive`, `is_negative`, and `is_zero` are predicate forms for the same
sign policy as `sign`. `pow` requires a non-negative exponent and asserts that
precondition at runtime. `div_floor` rounds signed division toward negative
infinity, `div_ceil` rounds toward positive infinity, and `mod_floor` returns
the matching floor remainder. The division helpers assert that
`denominator != 0`. `gcd` and `lcm` normalize negative inputs through absolute
values. `lcm` returns `0` when either input is `0`. These helpers
intentionally do not define overflow semantics yet.

## Bits

`std::bits` contains source-only `u64` helpers for bit masks, rotations,
power-of-two rounding, low-bit masks, alignment, and zero/one-run bit scans:

```ari
bits::is_set(value, mask)
bits::any_set(value, mask)
bits::set(value, mask)
bits::clear(value, mask)
bits::toggle(value, mask)
bits::rotate_left(value, count)
bits::rotate_right(value, count)
bits::is_power_of_two(value)
bits::bit_width(value)
bits::floor_power_of_two(value)
bits::ceil_power_of_two(value)
bits::low_mask(width)
bits::align_down(value, alignment)
bits::align_up(value, alignment)
bits::count_ones(value)
bits::count_zeros(value)
bits::leading_zeros(value)
bits::trailing_zeros(value)
bits::leading_ones(value)
bits::trailing_ones(value)
```

`is_set` requires all bits from `mask`; `any_set` requires at least one
overlap. `rotate_left` and `rotate_right` assert that `count` is non-negative
and then rotate modulo 64. `align_down` and `align_up` assert that `alignment`
is a non-zero power of two. These helpers currently have `u64` signatures and
intentionally avoid type suffixes so future generic integer APIs can keep the
same names. The zero value has 64 leading zeros, 64 trailing zeros, zero
leading ones, and zero trailing ones; `~0u64` has 64 leading ones and 64
trailing ones.
`bit_width` returns the number of bits needed to represent a value,
`floor_power_of_two` and `ceil_power_of_two` round to nearby powers of two, and
`low_mask(width)` returns a mask with the lowest `width` bits set. `low_mask`
accepts widths from 0 through 64.

## ASCII

`std::ascii` contains byte-oriented helpers for ASCII-only text and parser
code. Public names stay natural because the module path already says the
policy:

```ari
ascii::ParsedInt
ascii::is_digit(byte)
ascii::is_lower(byte)
ascii::is_upper(byte)
ascii::is_alpha(byte)
ascii::is_alphanumeric(byte)
ascii::is_blank(byte)
ascii::is_whitespace(byte)
ascii::is_control(byte)
ascii::is_printable(byte)
ascii::is_graphic(byte)
ascii::is_punctuation(byte)
ascii::is_hex_digit(byte)
ascii::to_lower(byte)
ascii::to_upper(byte)
ascii::digit_value(byte)
ascii::hex_value(byte)
ascii::equals_ignore_case(left, right)
ascii::starts_with_ignore_case(bytes, prefix)
ascii::ends_with_ignore_case(bytes, suffix)
ascii::index_of_ignore_case(bytes, needle)
ascii::contains_ignore_case(bytes, needle)
ascii::skip_whitespace(bytes)
ascii::trim_start(bytes)
ascii::trim_end(bytes)
ascii::trim(bytes)
ascii::parse_decimal(bytes)
ascii::parse_decimal_prefix(bytes)
ascii::parse_hex(bytes)
ascii::parse_hex_prefix(bytes)
```

`is_blank` covers space and tab. `is_whitespace` covers space, tab, line feed,
and carriage return. `is_printable` includes space; `is_graphic` excludes
space. `is_punctuation` is true for graphic ASCII bytes that are not letters or
digits.

`digit_value` and `hex_value` return `Option[i64]`. Non-digit input returns
`None<i64>()` where appropriate.

`equals_ignore_case`, `starts_with_ignore_case`, `ends_with_ignore_case`,
`index_of_ignore_case`, and `contains_ignore_case` operate on `Slice[u8]` and
fold only ASCII letter case. Empty prefixes, suffixes, and search needles
match. `index_of_ignore_case` returns the first matching byte offset or `-1`;
`contains_ignore_case` returns the same search as a bool. `skip_whitespace`,
`trim_start`, `trim_end`, and `trim` also operate on `Slice[u8]` and return
either a byte offset or a borrowed sub-slice. `parse_decimal` and `parse_hex`
parse the entire slice and return `Option[i64]`; empty input or invalid bytes
return `None<i64>()`. These parser helpers do not define overflow behavior
yet.

`ParsedInt` carries `value: i64` and `len: i64` for prefix parser results.
`parse_decimal_prefix` and `parse_hex_prefix` parse only the leading digit run,
stop before the first invalid byte, and return `None<ParsedInt>()` when the
first byte is empty or invalid. They do not trim, parse signs, or recognize
hexadecimal prefixes such as `0x`.

## Parsing

`std::parse` contains whole-input parsers over ASCII-trimmed `Slice[u8]`
values:

```ari
parse::integer(bytes)
parse::boolean(bytes)
parse::is_float(bytes)
parse::float_or(bytes, fallback)
parse::float(bytes)
```

`integer` returns `Option[i64]` and accepts optional `+` or `-` signs.
`boolean` returns `Option[bool]` and accepts only lowercase `true` and
`false`. `is_float` validates a decimal float shape with optional sign,
fraction, and exponent. `float_or` returns a fallback for invalid input, while
`float` panics on invalid input. `Option[f64]` is future work because the
compiler does not lower float enum payloads yet.

## Encoding

`std::encoding` contains text validation and byte codecs:

```ari
encoding::is_ascii(bytes)
encoding::utf8_count(bytes)
encoding::is_utf8(bytes)
encoding::utf16_count(words)
encoding::is_utf16(words)
encoding::hex_encoded_len(bytes)
encoding::encode_hex_in(ref mut zone, bytes)
encoding::hex_decoded_len(bytes)
encoding::can_decode_hex(bytes)
encoding::decode_hex_in(ref mut zone, bytes)
encoding::base64_encoded_len(bytes)
encoding::encode_base64_in(ref mut zone, bytes)
encoding::base64_decoded_len(bytes)
encoding::can_decode_base64(bytes)
encoding::decode_base64_in(ref mut zone, bytes)
```

`utf8_count` and `utf16_count` validate and return code-point counts through
`Option[i64]`; the `is_*` forms return only a bool. Hex encoding emits
lowercase digits and decoding accepts ASCII hex digits. Base64 uses the
standard `+`/`/` alphabet with `=` padding. Decoders panic on invalid input, so
call `can_decode_hex`, `hex_decoded_len`, `can_decode_base64`, or
`base64_decoded_len` before decoding untrusted input. `Option[String]` and
`Result[String, E]` decoder wrappers are future work because zone-backed enum
payloads are not supported yet.

## Choosing The Right Collection

Use bare `Vec[T]` literals like `[1, 2, 3]` for small local compiler-known
sequence storage. Use `std::vec::Vec[T]` when you need a growable source
library collection tied to an explicit allocation zone.

Use `Slice[T]` when you only need a borrowed view. Use `String` when bytes must
be owned and copied into a zone. Use `Box[T]` for one zone-backed owned value.

Use `Set[T]` for tiny insertion-order unique lists, `Deque[T]` for both-end
queues, `RingBuffer[T]` for bounded FIFO storage, `LinkedList[T]` for
front/back linked node operations, hash containers for average-case lookup,
tree containers for sorted lookup, and heaps/priority queues for repeated
highest-priority removal.
