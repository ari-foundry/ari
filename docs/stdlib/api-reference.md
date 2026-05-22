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
Error
ErrorKind
CStr
CString
Library
Symbol
AtomicI64
Mutex
Once
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
DoubleEndedIterator[T]
ExactSizeIterator[T]
std::Vec[T]
move(value)
take(place)
assert(condition)
debug_assert(condition)
assert_equal(left, right)
assert_not_equal(left, right)
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
min_by<T>(left, right, less)
max_by<T>(left, right, less)
clamp_by<T>(value, low, high, less)
is_between_by<T>(value, low, high, less)
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

## Test Support, Logging, And Errors

Immediate assertion and panic helpers are available from the root prelude:

```ari
assert(condition)
debug_assert(condition)
assert_equal(left, right)
assert_not_equal(left, right)
assert_eq_i64(left, right)
assert_ne_i64(left, right)
assert_eq_bool(left, right)
assert_ne_bool(left, right)
panic()
todo()
unreachable()
```

Use these when a failed check should stop the program immediately. Current
`panic`, `todo`, and `unreachable` share the same panic hook; Ari does not yet
attach source location, stack trace, or backtrace data to that hook.

Aggregated executable test helpers live in `std::test`:

```ari
test::Report

test::report()
test::scratch(capacity)
test::check(ref mut report, condition)
test::equal<T>(ref mut report, left, right)
test::not_equal<T>(ref mut report, left, right)
test::passed(ref report)
test::failed(ref report)
test::ok(ref report)
test::finish(ref report)
test::require(ref report)

report.check(condition)
report.equal<T>(left, right)
report.not_equal<T>(left, right)
report.passed()
report.failed()
report.ok()
report.finish()
report.require()
```

`Report` stores pass/fail counts. `equal` and `not_equal` use generic
comparison, so the public names stay natural rather than type-suffixed.
`finish()` returns `0` when no failures were recorded and `1` otherwise.
`scratch(capacity)` creates an owned `Zone` for tests; destroy it explicitly
with `zone::destroy(zone)` when the test is done.

Debug printing can use `print`/`println` with `{:?}` for built-in printable
values, `format_in!(ref mut zone, "{:?}", value)` for values implementing
`fmt::Debug`, or the `std::fmt`/`std::io` writer surface.

Recoverable error values live in `std::error`:

```ari
error::Kind
error::Error

error::new(kind)
error::with_code(kind, code)
error::try_with_code(kind, code)
error::from_errno(code)
error::try_from_errno(code)
error::from_raw(raw)
error::try_from_raw(raw)
error::from_raw_result[T](value)
error::to_raw_result[T](value)
error::kind(ref error)
error::code(ref error)
error::raw(ref error)
error::is_kind(ref error, kind)
error::is_not_found(ref error)
error::is_interrupted(ref error)
error::is_retryable(ref error)
error::name(kind)
error::message(ref error)

reason.kind()
reason.code()
reason.raw()
reason.is_kind(kind)
reason.is_not_found()
reason.is_interrupted()
reason.is_retryable()
reason.name()
reason.message()
```

Use `Error` for OS/runtime/library failures, and use `ErrorKind` for the root
alias of `std::error::Kind`. Direct `Result[T, Error]` is the preferred shape
for recoverable OS/runtime/library failures. The raw scalar representation is
kept for runtime, FFI, and compatibility bridges through
`from_raw_result`/`to_raw_result`. The strict constructors are for trusted
values; use `try_with_code`, `try_from_errno`, and `try_from_raw` when
validating untrusted boundary data.

Level-prefixed diagnostic logging lives in `std::log`:

```ari
log::Level
log::Trace
log::Debug
log::Info
log::Warn
log::Error

log::rank(level)
log::name(level)
log::enabled(level, minimum)
log::write(level, bytes)
log::message(level, text)
log::trace(text)
log::debug(text)
log::info(text)
log::warn(text)
log::error(text)
```

`log::write(level, bytes)` writes a `Slice[u8]` to `stderr` as
`[level] bytes\n`. `log::message(level, text)` writes a null-terminated Ari
`string` the same way, and the level convenience functions call `message`.
`enabled(level, minimum)` is an explicit threshold predicate; there is no
global logging filter today.

Compiler error reports, source spans, labels, JSON renderers, and golden
output belong in compiler/tooling packages rather than the
production standard library.

## C Interop

`std::c` contains C ABI boundary helpers around compiler-known C alias types:

```ari
c::CStr
c::CString
c::Library
c::Symbol

c::from_string(text)
c::from_ptr(data)
c::from_slice_in(ref mut zone, bytes)
c::from_cstr_in(ref mut zone, value)
c::is_null(value)

c::errno()
c::error()

c::lazy()
c::now()
c::local()
c::global()
c::open(path, flags)
c::main_program(flags)
c::symbol(ref library, name)
c::function[T](ref symbol)
c::close(ref mut library)
c::last_error()

cstr.as_ptr()
cstr.len()
cstr.is_empty()
cstr.as_slice()

cstring.len()
cstring.is_empty()
cstring.as_ptr()
cstring.as_c_str()
cstring.as_slice()
cstring.as_bytes_with_nul()

library.is_open()
library.symbol(name)
library.close()
Library::invalid()

symbol.is_valid()
symbol.as_ptr()
symbol.function[T]()
Symbol::invalid()
```

C ABI type aliases such as `c_int`, `c_char`, `c_void`, `size_t`, `c_long`,
and `c_ulong` are compiler-owned and follow the selected target ABI. Use
`ptr c_void` for `void*`; by-value `c_void` parameters are rejected.

`CStr` is a borrowed non-owning NUL-terminated string view. Construct it from a
string literal with `c::from_string("name")`, assign a literal directly when
`CStr` is expected, or wrap a non-null `ptr c_char` with `c::from_ptr(ptr)`.
`CStr.as_slice()` excludes the trailing NUL.

`CString` is a zone-backed owned C string buffer. `from_slice_in` asserts that
the input bytes contain no interior NUL, copies them into the given zone, and
adds one trailing NUL. `as_slice()` excludes the terminator, while
`as_bytes_with_nul()` includes it.

The current zone checker is conservative around arbitrary C calls: borrowed
literal-backed `CStr` values can be passed to `extern "C"` calls today, while
passing pointers from zone-backed `CString` storage directly to arbitrary C
imports remains roadmap work until Ari has an explicit FFI escape policy.

`errno()` reads the current POSIX thread-local errno on the hosted Linux/glibc
path, and `error()` maps it through `std::error::from_errno`. Dynamic loading
wraps `dlopen`, `dlsym`, `dlclose`, and `dlerror` with `Library` and `Symbol`
sentinels. `Symbol.function<fn(...) -> ...>()` casts a valid dynamic symbol to
a callable function pointer; keep that signature at the boundary because Ari
does not infer generic return-only function signatures yet. `Symbol.as_ptr()`
returns the raw address for lower-level bindings.

## Process Context And Environment

Runtime-backed context access lives in `std::context`:

```ari
context::argc()
context::arg(index)
context::thread_id()
context::cwd()
context::executable_path()
context::has_args()
context::has_arg(index)
context::user_arg_count()
context::has_user_args()
context::is_main_thread()
context::has_cwd()
context::try_cwd()
context::cwd_os()
context::try_cwd_os()
context::cwd_path()
context::has_executable_path()
context::try_executable_path()
context::executable_path_os()
context::try_executable_path_os()
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
`context::cwd()` and `context::executable_path()` are startup snapshots captured
by `@ari_entry`; use `std::env::current_dir()` when code needs the current
process directory after possible `chdir` calls.

Application code should usually use the user-facing `std::env` wrappers:

```ari
env::arg_count()
env::arg(index)
env::arg_os(index)
env::has_arg(index)
env::try_arg(index)
env::try_arg_os(index)
env::program_name()
env::program_name_os()
env::get(name)
env::get_os(name)
env::has(name)
env::try_get(name)
env::try_get_os(name)
env::set(name, value)
env::remove(name)
env::current_dir()
env::try_current_dir()
env::current_dir_os()
env::try_current_dir_os()
env::current_dir_path()
env::try_current_dir_path()
env::set_current_dir(path)
env::executable_path()
env::try_executable_path()
env::executable_path_os()
env::try_executable_path_os()
```

`env::try_arg(index)` returns `Option[string]`, and `env::program_name()` is
the optional `argv[0]` value.

`env::arg_os(index)`, `env::try_arg_os(index)`, and
`env::program_name_os()` return `std::string::OsStr` views when the argument
should stay in OS-string form until the caller chooses bytes or UTF-8.

`env::try_get(name)` returns `Option[string]` for environment variables.
`env::get(name)` returns an empty string when the variable is missing, so prefer
`try_get` or `has` when absence matters. `env::set(name, value)` overwrites a
current-process variable and `env::remove(name)` unsets it; both return whether
the host accepted the request. `env::current_dir()` and
`env::executable_path()` return borrowed runtime strings, with `try_*` wrappers
for ordinary failure; `env::set_current_dir(path)` mutates the current process
working directory. Portable child-process spawn handles remain roadmap work;
thread helpers live in `std::thread`.

`env::get_os(name)` and `env::try_get_os(name)` return `OsStr` views for
environment values. `env::current_dir_os()` / `try_current_dir_os()` and
`env::executable_path_os()` / `try_executable_path_os()` expose path-like host
data as OS strings. `env::current_dir_path()` and
`env::try_current_dir_path()` expose the current directory as
`std::path::PathBytes`; convert executable OS strings with `std::path::from_os`
when the next step is path manipulation.

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
process::fork_result()
process::fork()
process::wait_status_result(pid)
process::wait_result(pid)
process::wait(pid)
process::is_child(pid)
process::is_parent(pid)
process::is_fork_error(pid)
process::is_wait_error(status)
process::arg(value)
process::env_var(name, value)
process::command(program)
process::command_with_args(program, args)
process::spawn(command)
process::status(command)
process::exit_status(command)
process::output_in(command, zone)
process::exec(command)
process::kill(pid, signal)
process::terminate(pid)

process::Command::new(program)
process::Command::with_args(program, args)
Command::args(args)
Command::env(env_values)
Command::current_dir(path)
Command::spawn()
Command::status()
Command::exit_status()
Command::output_in(zone)
Command::exec()

ExitStatus::raw()
ExitStatus::exited()
ExitStatus::signaled()
ExitStatus::code()
ExitStatus::code_or(fallback)
ExitStatus::signal()
ExitStatus::signal_or(fallback)
ExitStatus::is_success()
ExitStatus::is_failure()

Output::status()
Output::exit_status()
Output::is_success()
Output::stdout()
Output::stderr()

Child::pid()
Child::wait()
Child::wait_status()
Child::kill(signal)
Child::terminate()
```

`id()` returns the host process id as `i64`. `uid()` and `gid()` return the
current user and group ids. `is_root()` is the source convenience check for
`uid() == 0`. `exit(code)` terminates the process and does not return.
`abort()` terminates through the host abnormal-termination path. The status
helpers are source functions for the common `0` success and `1` failure
convention.

`fork_result()` and `wait_status_result(pid)` are the preferred POSIX
child-process helpers when failure matters. `fork_result()` returns `Ok(0)` in
the child, `Ok(child_pid)` in the parent, or `Err(Error)` from the host `fork`
failure. `wait_status_result(pid)` returns a typed `ExitStatus`, maps host
`waitpid` failures through `std::c::error()`, and preserves signal termination
without flattening it into an `i64` sentinel. `wait_result(pid)` remains the
convenient compatibility form for normal exit codes only.

`fork()` and `wait(pid)` are the raw compatibility slice on the Linux/LLVM
runtime path. `fork()` returns `0` in the child, a positive child pid in the
parent, and a negative value on failure; use `is_child`, `is_parent`, and
`is_fork_error` to make that branch readable. `wait(pid)` returns a normal
child exit status or `-1`; use `is_wait_error` for that sentinel.

`Command` is the higher-level child-process builder. Use `process::arg` for
argv entries and `process::env_var` for child environment assignments:

```ari
var args = [process::arg("-c"), process::arg("exit 7")];
var cmd = process::command_with_args("sh", args.as_slice());
let status = cmd.status();
```

`status()` spawns and waits, returning a normal exit code for compatibility.
`exit_status()` spawns and waits, returning the typed `ExitStatus` that can
distinguish normal exit from signal termination. `spawn()` returns a `Child`
handle. `exec()` replaces the current process on success and returns
`Err(Error)` only if the host `execvp` path fails. `kill(pid, signal)` and
`Child::kill(signal)` return `Result[(), Error]`; `terminate` sends `SIGTERM`.
Module-level `process::spawn(ref cmd)`, `process::status(ref cmd)`,
`process::exit_status(ref cmd)`, `process::output_in(ref cmd, ref mut zone)`,
and `process::exec(ref cmd)` are direct wrappers over the matching `Command`
methods for call sites that prefer function-style process APIs.

`ExitStatus::code()` returns `Some(code)` only for normal exits.
`ExitStatus::signal()` returns `Some(signal)` only for signal termination.
`code_or` and `signal_or` are convenience fallbacks for compact control flow,
and `raw()` exposes the hosted wait-status bits for diagnostics.

`output_in(zone)` is the first captured-output helper. It spawns the child with
stdout and stderr redirected to pipes, waits for it, and returns an `Output`
whose byte buffers live in the provided zone:

```ari
var zone = zone::temp(512);
var args = [process::arg("-c"), process::arg("printf 'ok'")];
var cmd = process::command_with_args("sh", args.as_slice());
let result = cmd.output_in(ref mut zone);
```

Use `Output::exit_status()` for typed status, `Output::status()` for the normal
exit code compatibility accessor, `Output::is_success()` for the standard
success check, and `stdout()` / `stderr()` for borrowed `Slice[u8]` views. This
slice is meant for small outputs today; large concurrent streams, stdin
redirection, environment inheritance policy, portable Windows mapping, and
platform-specific status detail are still future process-library work.

## OS Descriptor Views

`std::os` contains low-level OS values that should not remain loose integers in
portable code:

```ari
os::Fd
os::OwnedFd
os::Pipe
os::fd(raw)
os::invalid()
os::stdin()
os::stdout()
os::stderr()
os::pipe()

fd.raw()
fd.is_valid()
fd.is_invalid()
fd.is_stdin()
fd.is_stdout()
fd.is_stderr()
fd.is_standard()
fd.equals(other)

OwnedFd::from_raw(raw)
OwnedFd::invalid()
owned.raw()
owned.as_fd()
owned.is_open()
owned.is_closed()
owned.take()
owned.try_clone()
owned.close_on_exec()
owned.set_close_on_exec(enabled)
owned.is_nonblocking()
owned.set_nonblocking(enabled)
owned.close()

pipe.read_end()
pipe.write_end()
pipe.take_read_end()
pipe.take_write_end()
pipe.close_read_end()
pipe.close_write_end()
pipe.close()
```

`Fd` is non-owning. It identifies a descriptor but does not close, duplicate,
or mutate it. `std::fs::File.descriptor()` returns an `Fd` view over a file
handle without transferring ownership.

`OwnedFd` owns close responsibility for a raw descriptor. Construct it with
`OwnedFd::from_raw(raw)` only when the caller is taking ownership of exactly
one close. `as_fd()` borrows the descriptor as `Fd`, `take()` disarms the owner
without closing, and `close()` disarms before calling the runtime close hook so
the same handle cannot close twice. `try_clone()` duplicates the descriptor and
returns `Option[OwnedFd]`; the original and cloned owners close independently.
`close_on_exec()` returns `Option[bool]`, and `set_close_on_exec(enabled)`
updates descriptor inheritance policy without exposing raw `fcntl` constants.
`is_nonblocking()` returns `Option[bool]`, and `set_nonblocking(enabled)`
updates blocking behavior with the same owned-descriptor policy. Readiness APIs,
raw syscalls, signals, and memory mapping remain future `std::os` work after
richer error policy is stable.

`Pipe` owns the read and write descriptors returned by `os::pipe()`.
`read_end()` and `write_end()` borrow non-owning `Fd` views, `take_read_end()`
and `take_write_end()` move individual owned ends out of the pair, and
`close_read_end()`, `close_write_end()`, or `close()` explicitly release the
remaining descriptors. `os::pipe()` returns `None` if the hosted pipe call
fails.

## Paths

`std::path` contains source-only lexical path helpers over `Slice[u8]`:

```ari
path::PathBytes
path::bytes(path)
path::from_os(os)
path::is_separator(value: char)
path::is_absolute(path)
path::is_relative(path)
path::trim_trailing_separators(path)
path::components(path)
path::file_name(path)
path::parent(path)
path::extension(path)
path::stem(path)
path::file_stem(path)
path::has_file_name(path, expected)
path::has_extension(path, expected)
path::has_stem(path, expected)
path::has_file_stem(path, expected)
path::starts_with(path, prefix)
path::strip_prefix(path, prefix)
path::ends_with(path, suffix)
path::strip_suffix(path, suffix)
path::with_file_name_in(ref mut zone, path, new_file_name)
path::with_extension_in(ref mut zone, path, new_extension)
path::join_in(ref mut zone, base, child)
path::normalize_in(ref mut zone, path)

path.as_slice()
path.len()
path.is_empty()
path.is_absolute()
path.is_relative()
path.trim_trailing_separators()
path.components()
path.file_name()
path.parent()
path.extension()
path.stem()
path.file_stem()
path.has_file_name(expected)
path.has_extension(expected)
path.has_stem(expected)
path.has_file_stem(expected)
path.starts_with(prefix)
path.strip_prefix(prefix)
path.ends_with(suffix)
path.strip_suffix(suffix)
path.with_file_name_in(ref mut zone, new_file_name)
path.with_extension_in(ref mut zone, new_extension)
path.join_in(ref mut zone, child)
path.normalize_in(ref mut zone)
```

The current separator policy is POSIX-style `/` only. Single-component helpers
return `Option[Slice[u8]]` views into the original path bytes.
`components(path)` returns a lazy iterator over non-empty borrowed components
and skips leading, repeated, and trailing separators.
`join_in` and `normalize_in` allocate byte strings in the caller-provided
zone. Normalization collapses repeated separators and removes `.` components,
but keeps `..` components because resolving them safely depends on stronger
filesystem and platform policy.
`with_file_name_in` and `with_extension_in` are zone-backed lexical editing
helpers for replacing the final component or final extension without touching
the filesystem. `with_file_name_in` preserves root paths, while
`with_extension_in` leaves paths without a final component unchanged.
`PathBytes` is the typed borrowed path-byte view. Use it when a byte slice or
`std::string::OsStr` should be treated as a path rather than as generic bytes
or validated text. When `PathBytes` is expected, a string literal can be used
directly as a borrowed path-byte view.
The `has_*` helpers are allocation-free predicates over `file_name`,
`extension`, `stem`, and `file_stem`; they return `false` when the
corresponding view is absent. `file_stem` is an explicit alias for `stem`.
`starts_with`/`ends_with` and `strip_prefix`/`strip_suffix` are
component-aware: trailing separators are ignored, and a match must end on a
path component boundary. The strip helpers return borrowed views into the
trimmed input path.

Thread helpers live in `std::thread`:

```ari
thread::spawn(entry)
thread::join(thread)
thread::yield_now()
thread::sleep(duration)
thread::id()
thread::is_main()
thread::available_parallelism()
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
`sleep(duration)` forwards to `std::time::sleep`. `available_parallelism()`
returns the hosted runtime's online processor count and falls back to `1` when
the platform call fails. User-facing thread-local storage and stack-size
configuration remain roadmap work.

Synchronization helpers live in `std::sync`:

```ari
AtomicI64::new(value)
Mutex::new()
RwLock::new()
Once::new()

sync::load(ref value)
sync::store(ref mut value, replacement)
sync::swap(ref mut value, replacement)
sync::fetch_add(ref mut value, amount)
sync::compare_exchange(ref mut value, expected, replacement)
sync::try_lock(ref mut mutex)
sync::lock(ref mut mutex)
sync::unlock(ref mut mutex)
sync::is_locked(ref mutex)
sync::try_read_lock(ref mut rwlock)
sync::read_lock(ref mut rwlock)
sync::read_unlock(ref mut rwlock)
sync::try_write_lock(ref mut rwlock)
sync::write_lock(ref mut rwlock)
sync::write_unlock(ref mut rwlock)
sync::reader_count(ref rwlock)
sync::is_read_locked(ref rwlock)
sync::is_write_locked(ref rwlock)
sync::call_once(ref mut once, action)
sync::is_completed(ref once)

atomic.load()
atomic.store(replacement)
atomic.swap(replacement)
atomic.fetch_add(amount)
atomic.compare_exchange(expected, replacement)

mutex.try_lock()
mutex.lock()
mutex.unlock()
mutex.is_locked()

rwlock.try_read_lock()
rwlock.read_lock()
rwlock.read_unlock()
rwlock.try_write_lock()
rwlock.write_lock()
rwlock.write_unlock()
rwlock.reader_count()
rwlock.is_read_locked()
rwlock.is_write_locked()
rwlock.is_locked()

once.call_once(action)
once.is_completed()
```

The atomic slice is concrete: `AtomicI64` only. Operations use sequentially
consistent ordering and keep the API name natural by putting the type in the
value, not in every method name. `fetch_add` and `swap` return the previous
value; `compare_exchange` returns whether the replacement happened.

`Mutex` is a source spin/yield lock built on `AtomicI64`. It is not a
value-protecting `Mutex[T]` and has no guard type yet, so keep lock/unlock
scopes explicit and small. `RwLock` is a source reader/writer primitive built
on the same atomic. It allows multiple readers or one writer, but it is not a
value-protecting `RwLock[T]` and has no read/write guards yet. `Once` runs a
plain `fn() -> void` at most once and reports whether the current caller ran
it.

`Shared`, `Weak`, `Condvar`, `OnceLock`, `LazyLock`, barriers, semaphores,
MPSC channels, futex-backed blocking locks, and weaker memory-order options
remain future concurrency work.

Runtime-backed time helpers live in `std::time`:

```ari
time::monotonic_nanos()
time::unix_nanos()
time::sleep_nanos(nanos)

time::nanoseconds(value)
time::try_nanoseconds(value)
time::microseconds(value)
time::try_microseconds(value)
time::milliseconds(value)
time::try_milliseconds(value)
time::seconds(value)
time::try_seconds(value)
time::now()
time::system_now()
time::system_from_unix(seconds, nanosecond)
time::try_system_from_unix(seconds, nanosecond)
time::is_leap_year(year)
time::days_in_month(year, month)
time::try_days_in_month(year, month)
time::utc_from_unix(seconds, nanosecond)
time::try_utc_from_unix(seconds, nanosecond)
time::elapsed(start)
time::sleep(duration)
time::deadline_at(instant)
time::timeout_after(duration)
time::timeout(duration)

Duration::zero()
Duration::try_nanoseconds(value)
Duration::try_microseconds(value)
Duration::try_milliseconds(value)
Duration::try_seconds(value)
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
instant.saturating_duration_since(earlier)
instant.try_duration_since(earlier)
instant.elapsed()
instant.add(duration)

SystemTime::now()
SystemTime::from_unix(seconds, nanosecond)
SystemTime::try_from_unix(seconds, nanosecond)
system_time.as_unix_nanos()
system_time.as_unix_seconds()
system_time.subsec_nanos()
system_time.duration_since_unix_epoch()
system_time.to_utc()

Deadline::at(instant)
Deadline::after(duration)
deadline.instant()
deadline.has_expired()
deadline.remaining()
deadline.sleep()

UtcDateTime::from_unix(seconds, nanosecond)
UtcDateTime::try_from_unix(seconds, nanosecond)
utc.year()
utc.month()
utc.day()
utc.hour()
utc.minute()
utc.second()
utc.nanosecond()
utc.is_leap_year()
```

Use `Instant` for elapsed time and `SystemTime` for Unix wall-clock
timestamps. Direct `Duration` constructors assert on negative values; use
`try_nanoseconds`, `try_microseconds`, `try_milliseconds`, or `try_seconds`
when input should be validated as `Option[Duration]`. The raw `*_nanos`
functions are exposed for low-level code, but ordinary code should prefer
`now`, `system_now`, `elapsed`, and `sleep`. Use `Deadline` plus
`timeout(duration)` or `deadline_at(instant)` when an API needs timeout policy
without depending on wall-clock time. `utc_from_unix` and
`SystemTime::to_utc` provide deterministic UTC calendar conversion for
non-negative Unix timestamps; `try_system_from_unix` and `try_utc_from_unix`
validate timestamp parts as `Option` before constructing values.
`try_days_in_month` validates user-provided month numbers before returning a
month length. Timezone databases are outside the first standard-library slice.

Filesystem helpers live in `std::fs`:

```ari
fs::exists(path)
fs::can_read(path)
fs::can_write(path)
fs::can_execute(path)
fs::permissions(path)
fs::metadata(path)
fs::metadata_raw_result(path)
fs::metadata_result(path)
fs::try_metadata(path)
fs::symlink_metadata(path)
fs::symlink_metadata_raw_result(path)
fs::symlink_metadata_result(path)
fs::try_symlink_metadata(path)
fs::file_type_raw_result(path)
fs::file_type_result(path)
fs::try_file_type(path)
fs::is_file(path)
fs::is_dir(path)
fs::is_symlink(path)
fs::is_other(path)
fs::mode(path)
fs::mode_raw_result(path)
fs::mode_result(path)
fs::try_mode(path)
fs::set_mode(path, mode)
fs::set_permissions(path, permissions)
fs::canonicalize(ref mut zone, path)
fs::canonicalize_result(ref mut zone, path)
fs::try_canonicalize(ref mut zone, path)
fs::remove(path)
fs::rename(source, target)
fs::hard_link(existing, link_path)
fs::symbolic_link(target, link_path)
fs::read_link(ref mut zone, path)
fs::read_link_result(ref mut zone, path)
fs::try_read_link(ref mut zone, path)
fs::ensure_file(path)
fs::create_dir(path)
fs::create_dir_raw_result(path)
fs::create_dir_result(path)
fs::ensure_dir(path)
fs::create_dir_all(path)
fs::ensure_dir_all(path)
fs::remove_dir(path)
fs::remove_dir_raw_result(path)
fs::remove_dir_result(path)
fs::remove_dir_all(path)
fs::open_dir(path)
fs::open_dir_raw_result(path)
fs::open_dir_result(path)
fs::try_open_dir(path)
fs::read_dir(ref mut zone, path)
fs::read_dir_result(ref mut zone, path)
fs::try_read_dir(ref mut zone, path)
fs::read_dir_entries(ref mut zone, path)
fs::read_dir_entries_result(ref mut zone, path)
fs::try_read_dir_entries(ref mut zone, path)
fs::read_dir_next(ref mut zone, dir)
fs::close_dir(dir)
fs::open(path, mode)
fs::create(path)
fs::remove_raw_result(path)
fs::remove_result(path)
fs::rename_raw_result(source, target)
fs::rename_result(source, target)
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
fs::try_read_byte(file)
fs::write_byte(file, value)
fs::write_bytes(file, values)
fs::position(file)
fs::seek(file, position)
fs::read(ref mut zone, path)
fs::read_result(ref mut zone, path)
fs::try_read(ref mut zone, path)
fs::write(path, values)
fs::write_raw_result(path, values)
fs::write_result(path, values)
fs::try_write(path, values)
fs::append(path, values)
fs::append_raw_result(path, values)
fs::append_result(path, values)
fs::try_append(path, values)
fs::truncate(path)
fs::copy(source, target)
fs::copy_raw_result(source, target)
fs::copy_result(source, target)
fs::try_copy(source, target)
fs::read_to_string(ref mut zone, path)
fs::read_to_string_result(ref mut zone, path)
fs::try_read_to_string(ref mut zone, path)

fs::open_raw_result(path, mode)
fs::open_result(path, mode)
fs::create_raw_result(path)
fs::create_result(path)
fs::open_options()
OpenOptions::new()
options.read(enabled)
options.write(enabled)
options.append(enabled)
options.truncate(enabled)
options.create(enabled)
options.create_new(enabled)
options.open(path)
options.open_raw_result(path)
options.open_result(path)
options.try_open(path)

File::invalid()
file.is_open()
file.close()
file.read_byte()
file.try_read_byte()
file.write_byte(value)
file.write_bytes(values)
file.position()
file.seek(position)

impl std::io::Reader for File
impl std::io::Writer for File
impl std::io::Seek for File

Dir::invalid()
dir.is_open()
dir.next(ref mut zone)
dir.close()

entry.name()
entry.path()
entry.name_equals(value)
entry.path_equals(value)
entry.try_metadata()
entry.metadata()
entry.try_symlink_metadata()
entry.symlink_metadata()
entry.try_file_type()
entry.is_file()
entry.is_dir()
entry.is_symlink()
entry.is_other()

Permissions::none()
Permissions::read_only()
Permissions::all()
permissions.can_read()
permissions.can_write()
permissions.can_execute()
permissions.any()
permissions.to_mode()

metadata.len()
metadata.file_type()
metadata.permissions()
metadata.accessed()
metadata.modified()
metadata.changed()
metadata.is_file()
metadata.is_dir()
metadata.is_symlink()
metadata.is_other()
```

Use `try_open(path, mode)` for ordinary fallible open operations; it returns
`Option[File]`. The raw `open(path, mode)` call returns a `File` directly,
with `File::invalid()` and `file.is_open()` exposing the invalid-handle
convention. Supported modes are `"r"` for read, `"w"` for create/truncate
write, `"a"` for create/append write, `"rw"` for existing read/write, `"r+"`
as a familiar alias for `"rw"`, `"w+"` for create/truncate read/write, and
`"a+"` for read/append. `open_read`, `open_write`, `open_append`, and their
`try_open_*` variants are compatibility wrappers over those mode strings.
Use `open_result(path, mode)` or `create_result(path)` when callers need more
than presence/absence. They return `Result[File, Error]`. Use
`open_raw_result(path, mode)` or `create_raw_result(path)` only for
compatibility callers that still need `Result[File, i64]`.
Use `read_result(ref mut zone, path)` or
`read_to_string_result(ref mut zone, path)` when a missing file should return
`Error(NotFound)` instead of the compatibility empty string/`None` behavior.
Use `OpenOptions::new()` or `fs::open_options()` when named policy is clearer:
`read`, `write`, `append`, `truncate`, `create`, and `create_new` each return a
new options value, `options.try_open(path)` returns `Option[File]`, and
`options.open_result(path)` returns `Result[File, Error]`.
`options.open_raw_result(path)` keeps the raw integer compatibility `Result`
shape.
`create_new(true)` is exclusive creation; `append(true).truncate(true)` and
create/truncate without write or append are rejected as invalid option sets.
`create(path)` and `try_create(path)` are the natural create/truncate helpers
over `"w"` mode. `File` implements `std::io::Reader`, `std::io::Writer`, and
`std::io::Seek`, so a handle from `try_open` can be passed to generic IO
helpers such as `io::read_to_string<std::fs::File>`, `io::copy`,
`io::try_copy`, `io::write_all`, `io::flush`, or a caller's own
`S: io::Seek` helper. `File` writes are direct descriptor writes; `flush`
currently reports whether the handle is open rather than draining a separate
file buffer. `position(file)`/`file.position()` returns the current byte
offset or `-1` for invalid or unseekable handles. `seek(file, position)` and
`file.seek(position)` move to an absolute offset from the start of the file and
return `false` for negative positions, invalid handles, or host seek failures.
Append-mode writes still follow host append semantics.
`ensure_file(path)` creates an empty file only when the path
is missing, treats an existing regular file as success without truncating it,
and returns `false` for directories, other existing path kinds, or missing
parents. `can_read`, `can_write`, and `can_execute` are access-style
preflight checks for the current process. `permissions(path)` snapshots those
three checks into a `Permissions` value; still handle later open/read/write
failures because filesystem access can change after the check.
`try_metadata(path)` returns `Option[Metadata]`, using `None` for missing or
unstatable paths. `metadata(path)` asserts that metadata is available. These
helpers follow symbolic links.
`metadata_result(path)` returns `Result[Metadata, Error]`, preserving
errno-derived failure kinds, and `metadata_raw_result(path)` keeps the raw
compatibility bridge.
`try_symlink_metadata(path)` and `symlink_metadata(path)` use no-follow
metadata lookup, so a symbolic link reports `FileKind::Symlink` and its stored
target byte length instead of the target file's metadata. The `Permissions`
field is still the same access-style snapshot as `permissions(path)`; portable
symlink permission-bit policy is not part of this slice.
`symlink_metadata_result(path)` and `symlink_metadata_raw_result(path)` are the
direct `Error` and raw compatibility versions of that no-follow lookup.
`Metadata::len` reports host byte length, `Metadata::file_type` returns
`FileKind` (`Regular`, `Directory`, `Symlink`, or `Other`), and
`Metadata::permissions` carries the access-style permission snapshot.
`Metadata::accessed`, `Metadata::modified`, and `Metadata::changed` return
`std::time::SystemTime` values for access time, modification time, and POSIX
status-change time respectively. `changed` is not a portable creation time.
`try_file_type(path)` returns just `Option[FileKind]` without building the full
metadata/permission snapshot.
`file_type_result(path)` returns `Result[FileKind, Error]` when the caller
needs a precise failure reason, and `file_type_raw_result(path)` keeps raw
errno compatibility.
`fs::is_file(path)`, `fs::is_dir(path)`, `fs::is_symlink(path)`, and
`fs::is_other(path)` are direct path predicates that return `false` for
missing or unstatable paths. `is_symlink` uses the no-follow policy; the other
direct predicates follow symbolic links through ordinary metadata. The
matching `metadata.is_*()` methods are the right choice when code already has a
`Metadata` value. The current runtime implementation uses Linux/glibc `stat`
for ordinary metadata and `lstat` for no-follow metadata; portable creation or
birth time is future platform-policy work.
`try_mode(path)` returns `Option[i64]` containing the low POSIX `0777`
permission bits, and `mode(path)` is the asserting wrapper. Use `set_mode(path,
mode)` for direct chmod-style updates, or `set_permissions(path, permissions)`
when a `Permissions` value is clearer at the call site. `Permissions::to_mode`
maps the three booleans to user/group/other bits, so `read_only()` maps to
`0444` and `all()` maps to `0777`.
`mode_result(path)` returns `Result[i64, Error]`; `mode_raw_result(path)` is
the raw compatibility form.
`try_canonicalize(ref mut zone, path)` returns `Option[String]`, using `None`
when the host cannot resolve the path. The returned string is absolute, owned
by the provided zone, and follows the host `realpath` policy. `canonicalize(ref
mut zone, path)` is the asserting wrapper for code that treats failed
resolution as a programmer error.
`canonicalize_result(ref mut zone, path)` returns `Result[String, Error]` for
tools and libraries that should surface why path resolution failed.
`try_read_link(ref mut zone, path)` returns `Option[String]` containing the
stored target bytes of a symbolic link. It returns `None` for missing paths,
regular files, unreadable links, or runtime-buffer overflow. `read_link(ref
mut zone, path)` is the asserting wrapper. Use `read_link` when code needs the
link text itself; use `canonicalize` when code wants the host-resolved
absolute path.
`read_link_result(ref mut zone, path)` is the direct `Error` form.
`read_byte` returns an `i64` byte value or `-1` at EOF/failure, and
`write_byte` returns whether one byte was written. `write_bytes` writes a
`Slice[u8]` and returns the count written before the first failed byte write.
`write_result(path, values)` truncates or creates a small byte file, writes the
whole `Slice[u8]`, and returns `Ok(byte_count)` when the write and close
succeed. `append_result(path, values)` creates if needed and appends the whole
slice with the same `Result[i64, Error]` policy. `write_raw_result` and
`append_raw_result` preserve the raw `Result[i64, i64]` compatibility shape.
`try_write(path, values)` and `try_append(path, values)` are `Option[i64]`
wrappers over those helpers, and `write(path, values)` and
`append(path, values)` are boolean compatibility wrappers.
`try_read_to_string(ref mut zone, path)` and its short alias
`try_read(ref mut zone, path)` return `Option[String]`, using `None` for a
missing or unopenable file and `Some(empty)` for an empty file. `read(ref mut
zone, path)` is the short compatibility alias for `read_to_string(ref mut zone,
path)`, returning a zone-backed byte-oriented `std::string::String` and using
an empty `String` when the file cannot be opened. Prefer `try_read` when
absence matters. `truncate(path)` creates or empties a file. `try_copy(source,
target)` streams bytes from the source handle into the target opened with
truncating semantics and returns `Some(byte_count)` on success or `None` on
open/write/close failure. `copy_result(source, target)` keeps those
open/write/close failures as `Err(Error)`, `copy_raw_result(source, target)`
keeps the raw `Result[i64, i64]` bridge, and `copy(source, target)` is the
boolean compatibility wrapper over `try_copy`. `rename(source, target)` moves or renames one path
according to the host runtime's current behavior, and
`rename_result(source, target)` preserves a `std::error::Error` value on failure.
`remove_result(path)`, `create_dir_result(path)`, and
`remove_dir_result(path)` are the same `Result[(), Error]` shape for file
removal and single-directory creation/removal; `*_raw_result` variants remain
for compatibility. `create_dir(path)` creates
one directory, `ensure_dir(path)` treats an existing directory as success or
creates a missing one, `create_dir_all(path)` recursively creates missing
parent directories, `ensure_dir_all(path)` is the idempotent recursive alias,
`remove_dir(path)` removes one empty directory, and `remove_dir_all(path)`
recursively removes a directory tree without following symlink entries outside
that tree.
`open_dir_result(path)` returns `Result[Dir, Error]` and
`open_dir_raw_result(path)` keeps raw compatibility. `try_open_dir(path)` returns `Option[Dir]`, `dir.next(ref mut zone)` returns
the next entry name while skipping `"."` and `".."`, and `dir.close()` closes
the handle. `try_read_dir(ref mut zone, path)` opens, collects names into
`std::vec::Vec[String]`, closes, and returns `None` on open/close failure.
`read_dir_result(ref mut zone, path)` returns `Result[Vec[String], Error]`,
and `read_dir(ref mut zone, path)` is the asserting wrapper. Use
`try_read_dir_entries(ref mut zone, path)` or
`read_dir_entries_result(ref mut zone, path)` or
`read_dir_entries(ref mut zone, path)` when callers need `DirEntry` values with
`entry.name()`, `entry.path()`, and lazy metadata/file-kind helpers. Entry
`metadata`, `try_metadata`, `try_file_type`, `is_file`, `is_dir`, and
`is_other` follow symbolic links. Entry `symlink_metadata`,
`try_symlink_metadata`, and `is_symlink` use no-follow link metadata. Recursive
directory removal, richer per-entry errors, and owned resource policy are
future work.
The current `File` and `Dir` values are not owned resources yet, so close each
successful handle once and do not reuse copied handles after closing.

Network address helpers live in `std::net`:

```ari
net::Ipv4Addr
net::Ipv6Addr
net::IpAddr
net::SocketAddr
net::TcpListener
net::TcpStream
net::UdpSocket
net::UnixListener
net::UnixStream
net::Shutdown

net::ipv4(a, b, c, d)
net::ipv6(s0, s1, s2, s3, s4, s5, s6, s7)
net::socket_addr(ip, port)
net::localhost(port)
net::lookup_v4(host, port)
net::lookup_v4_raw_result(host, port)
net::lookup_v4_result(host, port)
net::listen(addr)
net::tcp_listen(addr)
net::connect(addr)
net::tcp_connect(addr)
net::udp_bind(addr)
net::unix_listen(path)
net::unix_connect(path)

Ipv4Addr::new(a, b, c, d)
Ipv4Addr::any()
Ipv4Addr::localhost()
ipv4.octet(index)
ipv4.try_octet(index)
ipv4.is_unspecified()
ipv4.is_loopback()
ipv4.as_ip()

Ipv6Addr::new(s0, s1, s2, s3, s4, s5, s6, s7)
Ipv6Addr::any()
Ipv6Addr::localhost()
ipv6.segment(index)
ipv6.try_segment(index)
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

TcpListener::bind(addr)
TcpListener::try_bind(addr)
TcpListener::bind_raw_result(addr)
TcpListener::bind_result(addr)
listener.descriptor()
listener.is_open()
listener.local_port()
listener.local_addr()
listener.is_nonblocking()
listener.set_nonblocking(enabled)
listener.set_accept_timeout(timeout)
listener.set_accept_timeout_millis(millis)
listener.accept()
listener.try_accept()
listener.accept_raw_result()
listener.accept_result()
listener.close()

TcpStream::connect(addr)
TcpStream::try_connect(addr)
TcpStream::connect_raw_result(addr)
TcpStream::connect_result(addr)
stream.descriptor()
stream.is_open()
stream.local_addr()
stream.peer_addr()
stream.is_nonblocking()
stream.set_nonblocking(enabled)
stream.set_read_timeout(timeout)
stream.set_read_timeout_millis(millis)
stream.set_write_timeout(timeout)
stream.set_write_timeout_millis(millis)
stream.shutdown(mode)
stream.try_read_byte()
stream.read_exact(output, len)
stream.write_all(values)
stream.close()

UdpSocket::bind(addr)
UdpSocket::try_bind(addr)
UdpSocket::bind_raw_result(addr)
UdpSocket::bind_result(addr)
socket.descriptor()
socket.is_open()
socket.local_port()
socket.local_addr()
socket.is_nonblocking()
socket.set_nonblocking(enabled)
socket.set_read_timeout(timeout)
socket.set_read_timeout_millis(millis)
socket.set_write_timeout(timeout)
socket.set_write_timeout_millis(millis)
socket.send_byte_to(value, addr)
socket.recv_byte()
socket.try_recv_byte()
socket.close()

UnixListener::bind(path)
UnixListener::try_bind(path)
UnixListener::bind_raw_result(path)
UnixListener::bind_result(path)
listener.descriptor()
listener.is_open()
listener.is_nonblocking()
listener.set_nonblocking(enabled)
listener.accept()
listener.try_accept()
listener.accept_raw_result()
listener.accept_result()
listener.close()

UnixStream::connect(path)
UnixStream::try_connect(path)
UnixStream::connect_raw_result(path)
UnixStream::connect_result(path)
stream.descriptor()
stream.is_open()
stream.is_nonblocking()
stream.set_nonblocking(enabled)
stream.set_read_timeout(timeout)
stream.set_read_timeout_millis(millis)
stream.set_write_timeout(timeout)
stream.set_write_timeout_millis(millis)
stream.shutdown(mode)
stream.try_read_byte()
stream.read_exact(output, len)
stream.write_all(values)
stream.close()
```

Address values are deterministic source structs. Use `octet`/`segment` for
known-good indexes and `try_octet`/`try_segment` when validating parsed input.
`lookup_v4` resolves one IPv4 address through the hosted `getaddrinfo` path.
Socket and lookup `*_result` helpers return `Result[..., Error]`; matching
`*_raw_result` helpers are compatibility-only bridges for low-level callers that
still need raw integer errors.
`net::listen`/`net::connect` are TCP-focused module-level `Result` helpers;
use `tcp_listen`/`tcp_connect`, `udp_bind`, `unix_listen`, and `unix_connect`
when the socket family should be explicit at the call site.
`TcpListener`, `TcpStream`, `UdpSocket`, `UnixListener`, and `UnixStream` are
owned descriptor-backed handles. They support hosted IPv4 TCP bind/connect/
accept, IPv4 UDP bind/send-byte/receive-byte, Unix stream bind/connect/accept,
local bound-port and local IPv4 socket-address lookup where it applies,
borrowed descriptor views, explicit close, nonblocking flags,
`std::time::Duration` timeout setters with raw millisecond compatibility
helpers, and stream shutdown. TCP and
Unix streams adapt to `std::io::Reader`/`Writer` and provide inherent
`read_exact(output, len)` / `write_all(values)` helpers for natural stream
method syntax. IPv6 socket handles, buffered datagram APIs, richer socket
options, UDP source address helpers, and timeout-specific error results remain
roadmap work.

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
io::Pipe
io::PipeReader
io::PipeWriter
io::Cursor
io::BufReader[R]
io::BufWriter[W]
io::stdin()
io::stdout()
io::stderr()
io::pipe()
pipe.read_end()
pipe.write_end()
pipe.take_reader()
pipe.take_writer()
pipe.close()
pipe_reader.as_fd()
pipe_reader.is_open()
pipe_reader.close()
pipe_writer.as_fd()
pipe_writer.is_open()
pipe_writer.close()
io::cursor(values)
io::buf_reader[R: Reader](inner, buffer)
io::buf_writer[W: Writer](inner, buffer)
io::read_exact_result[R: Reader](reader: ref mut R, output, len)
io::read_exact[R: Reader](reader: ref mut R, output, len)
io::read_all[R: Reader](zone: ref mut Zone, reader: ref mut R)
io::read_to_string[R: Reader](zone: ref mut Zone, reader: ref mut R)
io::copy_result[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W)
io::try_copy[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W)
io::copy[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W)
io::write_all_result[W: Writer](writer: ref mut W, values)
io::write_all[W: Writer](writer: ref mut W, values)
io::flush_result[W: Writer](writer: ref mut W)
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
`io::read_exact_result(ref mut reader, output, len)` copies exactly `len`
bytes or returns `Err(Error(UnexpectedEof))`; `io::read_exact` is the bool
compatibility wrapper. A negative `len` returns `Err(Error(InvalidInput))`.
`io::read_all(ref mut zone, ref mut reader)` collects the remaining bytes from
any `Reader` into a zone-backed `Vec[u8]`, stopping at the same EOF sentinel as
`read_exact`.
`io::read_to_string(ref mut zone, ref mut reader)` collects the remaining bytes
directly into an owned `std::string::String`. It is byte-oriented like the rest
of `std::io`; use `String::try_utf8()` when a validated UTF-8 view is needed.
`io::copy_result(ref mut reader, ref mut writer)` streams bytes from any
`Reader` to any `Writer`, flushes at EOF, and returns `Ok(byte_count)` on
complete success. Failed byte writes become `Err(Error(BrokenPipe))`; failed
final flushes become `Err(Error(Other))`. `io::try_copy` is the `Option[i64]`
compatibility wrapper and `io::copy` is the bool wrapper when the byte count is
not needed.
`io::write_all_result(ref mut writer, values)` returns `Ok(())` after every
byte is accepted and `Err(Error(BrokenPipe))` on the first failed write.
`io::write_all` is the bool wrapper. `io::flush_result(ref mut writer)` returns
`Ok(())` for a successful flush and `Err(Error(Other))` for a failed flush;
`io::flush` is the bool wrapper.
`io::Stdout` and `io::Stderr` implement `Writer` over the current process
stream hooks, with `flush` currently succeeding as a no-op. `io::BufReader`
and `io::BufWriter` wrap any `Reader` or `Writer` with an explicit
caller-provided `Slice[u8]` buffer, so allocation and buffer lifetime stay
visible. `io::pipe()` wraps `std::os::Pipe` in `PipeReader` and `PipeWriter`
adapters. The reader implements `Reader`, the writer implements `Writer`, and
both expose explicit close helpers. `std::fs::File` implements `Reader`,
`Writer`, and `Seek`; zone-owning buffered constructors and drop-time writer
flush remain roadmap items until generic resource policy is specified.

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
mem::page_size()
mem::replace<T>(ref mut place, value)
mem::swap<T>(ref mut left, ref mut right)
```

For everyday raw-pointer code, prefer the operator spelling when it keeps the
site readable: `pointer + count` and `pointer - count` are typed element
offsets equivalent to `ptr_add`, while `*pointer` and `*pointer = value` are the
load/store forms equivalent to `ptr_load` and `ptr_store`. Keep
`ptr_offset(pointer, bytes)` for byte-wise address arithmetic.

`copy_bytes`, `move_bytes`, and `set_bytes` operate on `ptr u8` and byte
lengths. `copy_bytes` is for non-overlapping regions, `move_bytes` permits
overlap, and `set_bytes` fills a region with one byte. They lower through LLVM
memory intrinsics and trap on negative lengths.

`page_size()` returns the hosted runtime page size in bytes. Use it to check
alignment or prepare for future mapping APIs; it does not allocate or map
memory.

`std::zone` exposes the explicit allocation capability:

```ari
zone::create(capacity)
zone::alloc(ref mut zone, bytes, align)
zone::alloc<T>(ref mut zone)
zone::alloc_array<T>(ref mut zone, count)
zone::new<T>(ref mut zone, value)
zone::promote<T>(ref mut target, source)
zone::allocation_zone(data)
zone::metadata(data)
zone::from_zone(ref mut zone)
zone::of<T: zone::ZoneBacked>(ref value)
value.zone()
metadata.as_ptr()
metadata.as_zone_ptr()
metadata.alloc(bytes, align)
metadata.alloc_array<T>(count)
metadata.equals(ref other)
zone::reset(ref mut zone)
zone::destroy(zone)
```

`alloc_array<T>` returns uninitialized storage for `count` consecutive `T`
values. It returns null for `0`, asserts for negative counts, and does not run
destructors for the slots; initialize before reading and prefer higher-level
handles when ownership matters.

`allocation_zone(data)` reads Ari's allocation header for a non-null zone
allocation and returns the raw opaque handle. Prefer `metadata(data)`, which
wraps that handle as `ZoneMetadata`. `from_zone(ref mut zone)` captures
metadata directly from an explicit zone capability. `zone::of(ref value)` and
`value.zone()` use the `ZoneBacked` trait to expose `ZoneMetadata` from
supported heap-backed stdlib values such as `Box[T]`, `String`, `Vec[T]`, and
zone-backed `std::collections` handles.
`metadata.as_ptr()` is the raw escape hatch, `metadata.as_zone_ptr()` exposes
the same address as `ptr Zone`, and `metadata.equals(ref other)` checks handle
identity. `metadata.alloc(bytes, align)` and `metadata.alloc_array<T>(count)`
allocate through the recovered runtime zone handle. Zero-capacity handles may
carry metadata from construction even when they have no backing data pointer;
raw `metadata(data)` still requires a non-null allocation pointer.

## Option And Result

`Option[T]` models a present or missing value:

```ari
Some(value)
None<T>()
value.is_some()
value.is_none()
value.is_some_and(op)
value.is_none_or(op)
value.contains(value)
value.unwrap_or(fallback)
value.unwrap_or_else(op)
value.unwrap()
value.expect("message")
value.map<U>(op)
value.map_or<U>(fallback, op)
value.map_or_else<U>(fallback, op)
value.inspect(op)
value.and<U>(other)
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
value.contains(value)
value.contains_err(error)
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
value.map_or<U>(fallback, op)
value.map_or_else<U>(fallback, op)
value.inspect(op)
value.inspect_err(op)
value.and<U>(other)
value.and_then<U>(op)
value.or<F>(fallback)
value.or_else<F>(op)
value.transpose()
```

Use `is_some_and`, `is_none_or`, `is_ok_and`, and `is_err_and` when a branch
depends on both the enum case and a payload predicate. These helpers consume
the enum value and pass the payload to the predicate. Use `contains` and
`contains_err` for exact `==` comparisons against present success/error
payloads. `Option::flatten` is available on `Option[Option[T]]` and removes
one optional layer. `filter` keeps a `Some(T)` only when a borrowed
`fn(ref T) -> bool` predicate accepts the payload.
`Option::transpose` is available on `Option[Result[T, E]]` and turns optional
fallible work into fallible optional work. Use the lazy `*_else` forms when the
fallback is expensive or should only run on the missing/error branch. Use
`map_or` and `map_or_else` when the desired result is not another
`Option`/`Result`, but a direct fallback-or-mapped value. Use `inspect` and
`inspect_err` to observe borrowed payloads while preserving the original
control-flow value. Use
`Option::and` and `Result::and` when the next value is already available and
should be selected only from the present/success branch. Use
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
view.first_mut()
view.last()
view.try_last()
view.last_mut()
view.get(index)
view.try_get(index)
view.get_mut(index)
view[index]
view.as_ptr()
view.iter()
view.iter_mut()
view.contains(value)
view.index_of(value)
view.count(value)
view.find_if(predicate)
view.position(predicate)
view.rposition(predicate)
view.any(predicate)
view.all(predicate)
view.count_if(predicate)
view.find(needle)
view.contains_slice(needle)
view.starts_with(other)
view.ends_with(other)
view.strip_prefix(other)
view.strip_suffix(other)
view.equals(other)
view.compare(other)
view.ordering(other)
view.slice(start, end)
view.split_at(index)
view.split_first()
view.split_last()
view.chunks(size)
view.windows(size)
view.split(delimiter)
view.reverse()
view.reverse_range(start, end)
view.rotate_left(count)
view.rotate_right(count)
view.rotate_range(start, end, count)
view.fill(value)
view.fill_range(start, end, value)
view.copy_from(source)
view.copy_within(start, end, target)
view.partition(keep)
view.stable_partition(keep)
view.dedup()
view.dedup_by(same)
view.dedup_by_key(key)
view.sort()
view.sort_by(less)
view.stable_sort()
view.stable_sort_by(less)
view.stable_sort_in(ref mut zone)
view.stable_sort_by_in(less, ref mut zone)
view.try_stable_sort()
view.try_stable_sort_by(less)
view.is_sorted()
view.is_sorted_by(less)
view.binary_search(value)
view.binary_search_by(value, less)
view.lower_bound(value)
view.lower_bound_by(value, less)
view.upper_bound(value)
view.upper_bound_by(value, less)
view.equal_range(value)
view.equal_range_by(value, less)
view.partition_point(predicate)
view.min()
view.min_by(less)
view.max()
view.max_by(less)
view.copy_to(ref mut zone)
```

`first`, `last`, and `get` assert when the requested element does not exist.
Use `try_first`, `try_last`, and `try_get` when absence is an ordinary branch;
they return `Option[T]`. `first_mut`, `last_mut`, and `get_mut` assert on
absence and return mutable element borrows into the same backing storage.
`iter()` returns a `SliceIter[T]` value cursor. `iter_mut()` returns a
`SliceIterMut[T]` cursor whose items are `SliceValueMut[T]` handles with
`value()` and `value_mut()` accessors for in-place updates.
`is_empty` is a source method that borrows the view and checks whether the
stored length is zero. `find` searches for a borrowed subslice and returns an
index or `-1`; an empty needle matches at `0`. `contains_slice` is the boolean
wrapper. `find_if`, `position`, `rposition`, `any`, `all`, and `count_if` take
`fn(ref T) -> bool` predicates. `strip_prefix` and `strip_suffix` return
`Option[Slice[T]]` borrowed remainders. `compare` is lexicographic and returns
`-1`, `0`, or `1` for compatibility; prefer `ordering` in new code because it
returns `cmp::Ordering`. `slice`, `split_at`, `split_first`, and `split_last`
return borrowed views into the same storage; endpoint splitting returns
`Option` so empty slices can be handled directly. `chunks`, `windows`, and
delimiter `split` are lazy
iterators that yield borrowed `Slice[T]` views. The reordering, fill/copy,
stable/unstable partition, dedup variants, sort/search, and min/max receiver
methods forward to `std::algo`; ordered methods require `T: std::cmp::Ord[T]`,
and `*_by` methods take explicit comparators for call-site ordering.
The half-open range methods `copy_within`, `fill_range`, `reverse_range`, and
`rotate_range` mutate only `[start, end)` or copy that range to `target`.
`lower_bound` and `upper_bound` return sorted insertion indexes. `equal_range`
returns the matching `(lower, upper)` range, and `partition_point` returns the first false
predicate index in a predicate-partitioned view.
These helpers share the current copy-oriented
[value movement contract](value-contracts.md): they are for copyable/plain
elements today, and move-aware resource elements remain future work.

`std::vec::Vec[T]` is the source growable sequence:

```ari
std::vec::new<T>(ref mut zone, capacity)
Vec::with_capacity<T>(ref mut zone, capacity)
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
vec.try_remove(index)
vec.swap_remove(index)
vec.truncate(length)
vec.retain(keep)
vec.dedup()
vec.dedup_by(same)
vec.dedup_by_key(key)
vec.fill(value)
vec.fill_range(start, end, value)
vec.copy_from(source)
vec.copy_within(start, end, target)
vec.partition(keep)
vec.stable_partition(keep)
vec.clear()
vec.reserve(capacity)
vec.try_reserve(capacity)
vec.reserve_in(ref mut zone, capacity)
vec.reserve_extra(additional)
vec.reserve_extra_in(ref mut zone, additional)
vec.shrink_to_fit()
vec.extend(values)
vec.extend_iter(iter)
vec.extend_from_slice(values)
vec.extend_from_slice_in(ref mut zone, values)
vec.append(ref mut other)
vec.resize(length, value)
vec.resize_with(length, make_value)
vec.resize_in(ref mut zone, length, value)
vec.drain()
vec.drain_range(start, end)
vec.insert_many(index, values)
vec.remove_range(start, end)
vec.splice(start, end, replacement)
vec.split_off(index)
vec.index_of(value)
vec.contains(value)
vec.count(value)
vec.find(needle)
vec.contains_slice(needle)
vec.starts_with(values)
vec.ends_with(values)
vec.equals(values)
vec.compare(values)
vec.ordering(values)
vec.slice(start, end)
vec.split_at(index)
vec.chunks(size)
vec.windows(size)
vec.split(delimiter)
vec.reverse()
vec.reverse_range(start, end)
vec.rotate_left(count)
vec.rotate_right(count)
vec.rotate_range(start, end, count)
vec.sort()
vec.sort_by(less)
vec.stable_sort()
vec.stable_sort_by(less)
vec.stable_sort_in(ref mut zone)
vec.stable_sort_by_in(less, ref mut zone)
vec.try_stable_sort()
vec.try_stable_sort_by(less)
vec.is_sorted()
vec.is_sorted_by(less)
vec.binary_search(value)
vec.binary_search_by(value, less)
vec.lower_bound(value)
vec.lower_bound_by(value, less)
vec.upper_bound(value)
vec.upper_bound_by(value, less)
vec.equal_range(value)
vec.equal_range_by(value, less)
vec.partition_point(predicate)
vec.min()
vec.min_by(less)
vec.max()
vec.max_by(less)
vec.as_slice()
vec.as_ptr()
vec.as_mut_ptr()
vec.copy_to(ref mut zone)
vec.iter()
vec.iter_mut()
```

The `try_*` accessors return `Option[T]` for empty or out-of-range reads.
`try_pop` and `try_remove` keep empty or missing-index removal in `Option[T]`.
Use the non-`try` forms when absence is a programmer error and an assertion is
the desired behavior. `swap_remove(index)` is the unordered O(1) removal form:
it returns the removed value and moves the old tail value into the hole.
`retain(keep)` compacts accepted values in place, preserves their order, and
drops rejected values. `dedup()`, `dedup_by(same)`, and `dedup_by_key(key)`
remove consecutive duplicate values from the owned vector and return the new
length. `fill`, `copy_from`, `partition`, and
`stable_partition` are owned-vector wrappers over the same live-prefix
policies as `Slice[T]`. `copy_within`, `fill_range`, `reverse_range`, and
`rotate_range` are half-open range mutation helpers. `extend(values)` is the
natural alias for `extend_from_slice(values)`, while `extend_iter(iter)`
consumes any `Iterator[T]` and pushes yielded values. `append(ref mut other)`
moves another vector's live values into the receiver and empties the source
vector. `insert_many`, `remove_range`, `drain_range`, and `splice` cover common
half-open range edits. `drain()` empties the vector and returns a
`std::vec::Drain[T]` cursor; `drain_range(start, end)` returns the same cursor
shape for only that removed range. Unconsumed drained values are dropped with
the cursor. `split_off(index)` moves the tail into a new same-zone `Vec[T]`
and leaves the receiver with the prefix. Its source declaration carries the
same-zone parameter that the compiler injects for tracked local receivers, so
ordinary code uses the natural one-argument call. `shrink_to_fit()` shrinks the
handle's logical capacity to `len()` by moving live values into a new zone
allocation, while old bytes remain owned by the zone until reset/destroy.
`try_reserve(capacity)` returns `false` for negative capacities and otherwise
uses the same runtime allocation policy as `reserve`. `push`, `insert`,
`reserve`, `try_reserve`, `reserve_extra`, `extend`, `extend_from_slice`,
`append`, `insert_many`, `splice`, growing `resize`, and `resize_with` use
`ZoneMetadata` stored inside the handle; the `_in` forms remain available for
explicit capability plumbing. `resize_with(length, make_value)` calls the
zero-argument maker once per new slot, which is the natural growth spelling
when one repeated value is not the right contract.
The borrowed sequence helpers
mirror the root `Slice[T]` vocabulary: `slice` and `split_at` create views over live vector storage,
`find` and `contains_slice` search for borrowed subsequences, `compare` is
lexicographic, `ordering` returns typed `cmp::Ordering`, and `chunks`,
`windows`, and delimiter `split` are lazy allocation-free view iterators.
The sort/search wrappers share the `std::algo` policy, including
lower/upper/equal-range bounds and partition-point lookup.
`iter_mut()` returns a `SliceIterMut[T]` mutable value cursor; each yielded
handle supports `value()` and `value_mut()`.
Vector growth and reordering also follow the shared
[value movement contract](value-contracts.md). Shrink and removal paths drop
removed live values, while growing `resize(length, value)` repeats one value and
therefore is not a clone/generator API for resource owners. Use
`resize_with(length, make_value)` when each new slot should be built by a fresh
function call; final placement still follows today's raw storage model.

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
set.equals(ref other)
set.is_subset(ref other)
set.is_superset(ref other)
set.is_disjoint(ref other)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.remove(value)
set.take(value)
set.pop()
set.try_pop()
set.clear()
set.retain(keep)
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
set.as_slice()
set.iter()
set.drain()
set.copy_to(ref mut zone)
```

For a local collection handle created from a tracked zone allocation, common
growth methods may omit the repeated zone argument. Examples include
`set.insert(value)`, `set.replace(value)`, `map.insert(key, value)`,
`map.reserve(capacity)`, `map.reserve_extra(additional)`,
`deque.push_back(value)`, `list.push_front(value)`, and `heap.push(value)`.
Keep the explicit `ref mut zone` form for manually
assembled or otherwise untracked handles.

`insert` returns `true` only for newly inserted values. `replace` returns
`Some(previous)` for an equal existing value, or inserts the missing value and
returns `None`. `remove` drops the removed value and reports whether it was
present; `take` returns `Option[T]`. Relationship methods compare membership
instead of insertion order and borrow the other set explicitly. `first`,
`last`, and `get` assert that the requested element exists, while
`try_first`/`try_last`/`try_get` return `Option[T]`. `pop` removes the last
insertion-order value, and `try_pop` returns `None` on an empty set.
`retain(fn(ref T) -> bool)` filters in place, keeps insertion order for
retained values, and drops removed values. `reserve` and `reserve_extra` grow
through the same source zone. The set preserves insertion order in accessors,
`index_of`, `as_slice`,
`iter`, and `copy_to`. `std::collections::Iter[T]` implements `Iterator[T]`,
and `Set[T]` implements `IntoIterator[T]`, so `for value in set` works through
the standard iterator path. `drain()` yields insertion-order values and leaves
the source set empty.

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
deque.copy_to(ref mut target)
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
buffer.copy_to(ref mut target)
buffer.iter()
```

`Deque[T]` grows with the same zone when either end needs more room.
`RingBuffer[T]` is fixed-capacity: `push` returns `false` when full, and
`push_overwrite` returns the overwritten oldest value when it has to make room.
Both collections support `copy_to(ref mut target)` for moving logical contents
into another zone without depending on the source zone lifetime.

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
list.copy_to(ref mut target)
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
heap.copy_to(ref mut target)
queue.copy_to(ref mut target)
```

`LinkedList[T]` uses zone-backed index nodes and reuses removed node slots.
`BinaryHeap[T]` and `PriorityQueue[T]` use `less(a, b)` as a lower-priority
predicate, so `collections::less_i64` makes larger integers pop first.
`copy_to(ref mut target)` rebuilds linked-list live order or copies heap
storage into the target zone while preserving observable pop order.

Hash-table collections use explicit hash functions:

```ari
collections::hash_i64(value)
collections::hash_map<K, V>(ref mut zone, capacity, hash)
HashMap::new<K, V>(ref mut zone, capacity, hash)
map.len()
map.capacity()
map.is_empty()
map.contains(key)
map.contains_key(key)
map.contains_value(value)
map.get(key)
map.get_or(key, fallback)
map.try_get(key)
map.get_mut(key)
map.try_get_mut(key)
map.insert(ref mut zone, key, value)
map.replace(ref mut zone, key, value)
map.entry(ref mut zone, key)
map.entry(key)
map.remove(key)
map.remove_entry(key)
map.clear()
map.retain(keep)
map.reserve(ref mut zone, capacity)
map.reserve_extra(ref mut zone, additional)
map.copy_to(ref mut target)
map.keys()
map.values()
map.values_mut()
map.entries()
map.iter()
map.iter_mut()
map.drain()

collections::hash_set<T>(ref mut zone, capacity, hash)
HashSet::new<T>(ref mut zone, capacity, hash)
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.get(value)
set.try_get(value)
set.equals(ref other)
set.is_subset(ref other)
set.is_superset(ref other)
set.is_disjoint(ref other)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.take(value)
set.remove(value)
set.clear()
set.retain(keep)
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
set.copy_to(ref mut target)
set.iter()
set.drain()
```

`collections::hash_i64` is a compatibility helper over `std::hash::value<i64>`.

`HashMap.contains_key(key)` is the preferred key-membership spelling;
`HashMap.contains(key)` remains available for compatibility.
`HashMap.contains_value(value)` scans live bucket values and ignores
tombstones. `HashMap.get_or(key, fallback)` returns the stored value or the
fallback when the key is absent. `HashMap.get_mut(key)` asserts that the key
exists and returns `ref mut V`; `HashMap.try_get_mut(key)` returns
`Option[MapValueMut[V]]` with `value()` and `value_mut()` helpers.
`HashMap.insert` and `HashMap.replace` return `Option[V]` with the previous
value on replacement. `HashMap.entry(key)` returns a
`HashMapEntry[K,V]` update handle with `or_insert`, `or_insert_with`,
`or_default`, `and_modify`, `insert`, `insert_entry`, `remove`, `key`,
`value`, and `value_mut`; tracked local maps infer the allocation zone for the
natural `entry(key)` spelling. `or_default` requires `V: Default`, while
`insert_entry(value)` stores `value` and returns the entry handle for continued
chaining.
`HashMap.remove` returns `Option[V]` and leaves
a tombstone so later probes still find collided keys. `HashMap.retain(keep)`
visits live buckets in place with `fn(ref K, ref mut V) -> bool`; retained
values may be mutated, and rejected key/value pairs are dropped and replaced by
tombstones. `HashMap.keys()` and
`HashMap.values()` iterate live buckets; this is deterministic for the table
state, but it is not insertion order. `HashMap.values_mut()` is a mutable
value cursor with `has_next()` and `next() -> ref mut V`. `HashMap.iter()` is
an alias for `entries()`, `HashMap.iter_mut()` walks `MapEntryMut[K,V]`
handles with copied keys and mutable values, direct `for entry in map` walks
`MapEntry[K,V]` values, and `HashMap.drain()` leaves the map empty while
yielding drained entries. `HashSet.get(value)` and
`HashSet.try_get(value)` read the stored equal representative. `HashSet`
relationship methods compare live membership and ignore tombstones;
`HashMap.entries()` yields `MapEntry[K,V]` values with `.key`/`.value` fields
and `key()`/`value()` accessors over the same live buckets.
`HashMap.remove_entry(key)` returns
`Option[MapEntry[K,V]]` so callers can keep both removed key and value.
`HashSet.iter()` and direct
`for value in set` use the same live-bucket cursor. `HashSet.drain()` leaves
the set empty while yielding drained values. `HashSet.retain(keep)` filters
live buckets with `fn(ref T) -> bool`, drops rejected values, and leaves
tombstones for probing correctness. Hash
`reserve_extra(additional)` grows enough buckets for `len + additional` live
items without immediately violating the load-factor rule. Hash `copy_to`
methods copy only live entries into the target zone, leaving tombstones behind.

Tree collections use explicit strict less-than comparators:

```ari
collections::less_i64(left, right)
collections::tree_map<K, V>(ref mut zone, capacity, less)
TreeMap::new<K, V>(ref mut zone, capacity, less)
map.len()
map.capacity()
map.is_empty()
map.contains(key)
map.contains_key(key)
map.contains_value(value)
map.first_key()
map.try_first_key()
map.last_key()
map.try_last_key()
map.first_value()
map.try_first_value()
map.last_value()
map.try_last_value()
map.first_entry()
map.try_first_entry()
map.last_entry()
map.try_last_entry()
map.get(key)
map.get_or(key, fallback)
map.try_get(key)
map.get_mut(key)
map.try_get_mut(key)
map.lower_bound(key)
map.upper_bound(key)
map.insert(ref mut zone, key, value)
map.replace(ref mut zone, key, value)
map.entry(ref mut zone, key)
map.entry(key)
map.remove(key)
map.remove_entry(key)
map.clear()
map.reserve(ref mut zone, capacity)
map.reserve_extra(ref mut zone, additional)
map.copy_to(ref mut target)
map.keys()
map.values()
map.values_mut()
map.entries()
map.iter()
map.iter_mut()
map.drain()

collections::tree_set<T>(ref mut zone, capacity, less)
TreeSet::new<T>(ref mut zone, capacity, less)
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.get(value)
set.try_get(value)
set.first()
set.try_first()
set.last()
set.try_last()
set.lower_bound(value)
set.upper_bound(value)
set.equals(ref other)
set.is_subset(ref other)
set.is_superset(ref other)
set.is_disjoint(ref other)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.take(value)
set.remove(value)
set.clear()
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
set.copy_to(ref mut target)
set.iter()
set.drain()
```

`TreeMap.contains_key(key)` is the preferred key-membership spelling;
`TreeMap.contains(key)` remains available for compatibility.
`TreeMap.contains_value(value)` scans stored values without using key order.
`TreeMap.get_or(key, fallback)` returns the stored value or the fallback when
the key is absent. `TreeMap.get_mut(key)` asserts that the key exists and
returns `ref mut V`; `TreeMap.try_get_mut(key)` returns
`Option[MapValueMut[V]]` with `value()` and `value_mut()` helpers.
`TreeMap.replace` is the named insert-or-replace form and returns the previous
value like `insert`. `TreeMap.entry(key)` returns a `TreeMapEntry[K,V]` update
handle with `or_insert`, `or_insert_with`, `or_default`, `and_modify`,
`insert`, `insert_entry`, `remove`, `key`, `value`, and `value_mut`; tracked
local maps infer the allocation zone for the natural `entry(key)` spelling.
`or_default` requires `V: Default`, while `insert_entry(value)` stores `value`
and returns the entry handle for continued chaining.
`TreeMap.remove(key)` returns `Option[V]` and rebuilds links
in place after compacting live storage. `TreeMap` boundary methods read the
smallest or largest key and the value attached to that key in comparator order;
use the `try_*` forms when an empty tree is a normal case. `first_entry` and
`last_entry` return `MapEntry[K,V]` values when both boundary key and value are
needed together. `TreeMap.lower_bound(key)` returns the first entry whose key
is not less than `key`; `TreeMap.upper_bound(key)` returns the first entry
whose key is greater than `key`. `TreeMap.remove_entry(key)` returns
`Option[MapEntry[K,V]]`, keeping the removed key and value together.
`TreeSet` relationship methods compare
ordered-set membership, not internal tree shape.
`TreeSet.first()` and `TreeSet.last()` read the smallest and largest values,
with `try_first` and `try_last` for empty-safe access. `TreeSet.lower_bound`
and `TreeSet.upper_bound` return optional nearest values in comparator order.
`TreeSet.get(value)` and `TreeSet.try_get(value)` read the stored equal
representative. `TreeSet.take(value)` returns the removed value as `Option[T]`;
`TreeSet.remove(value)` drops it and returns a boolean. `TreeMap.keys()`,
`TreeMap.values()`, `TreeMap.entries()`, `TreeMap.iter()`, `TreeSet.iter()`,
and direct `for value in tree_set` walk values in ascending comparator order.
`TreeMap.values_mut()` exposes a mutable sorted value cursor,
`TreeMap.iter_mut()` walks sorted `MapEntryMut[K,V]` handles, direct
`for entry in tree_map` walks `MapEntry[K,V]`, and `TreeMap.drain()`/
`TreeSet.drain()` leave the source container empty while yielding sorted
drained entries or values. Tree
`copy_to` methods rebuild the map or set in the target zone with the same
comparator.

`std::string::String` is an owned byte string:

```ari
std::string::Utf8
std::string::OsStr
std::string::utf8(bytes)
std::string::os_str(bytes)
std::string::c_str(text)
std::string::c_len(text)
std::string::c_bytes(text)
std::string::bytes(text)
std::string::new(ref mut zone, capacity)
std::string::empty(ref mut zone)
std::string::from(ref mut zone, "text")
std::string::from_string(ref mut zone, "text")
std::string::copy(ref mut zone, bytes)
std::string::from_slice_in(ref mut zone, bytes)
std::string::join_in(ref mut zone, parts, separator)
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
text.push(char)
text.push_in(ref mut zone, char)
text.pop()
text.try_pop()
text.remove(index)
text.try_remove(index)
text.insert(index, char)
text.insert_in(ref mut zone, index, char)
text.clear()
text.truncate(length)
text.retain(keep)
text.reserve(ref mut zone, capacity)
text.reserve_extra(ref mut zone, additional)
text.append(ref mut zone, "text")
text.append_byte(ref mut zone, char)
text.append_bytes(ref mut zone, bytes)
text.extend_from_slice_in(ref mut zone, bytes)
text.resize_in(ref mut zone, length, char)
text.index_of(char)
text.contains(char)
text.count(char)
text.find(bytes)
text.find_text("text")
text.contains_slice(bytes)
text.contains_text("text")
text.slice(start, end)
text.split_at(index)
text.chunks(size)
text.windows(size)
text.split(delimiter: char)
text.starts_with(bytes)
text.starts_with_text("text")
text.ends_with(bytes)
text.ends_with_text("text")
text.equals(bytes)
text.equals_text("text")
text.equals_ignore_case(bytes)
text.equals_text_ignore_case("text")
text.starts_with_ignore_case(bytes)
text.starts_with_text_ignore_case("text")
text.ends_with_ignore_case(bytes)
text.ends_with_text_ignore_case("text")
text.index_of_ignore_case(bytes)
text.index_of_text_ignore_case("text")
text.contains_ignore_case(bytes)
text.contains_text_ignore_case("text")
text.append_string_in(ref mut zone, "text")
text.append_i64_in(ref mut zone, value)
text.append_u64_in(ref mut zone, value)
text.append_bool_in(ref mut zone, value)
text.append_value_in(ref mut zone, display_value)
text.append_debug_in(ref mut zone, debug_value)
text.append_f32_in(ref mut zone, value, precision)
text.append_f64_in(ref mut zone, value, precision)
text.push_codepoint_in(ref mut zone, scalar)
text.is_utf8()
text.try_utf8()
text.codepoint_count()
text.codepoint_at(byte_index)
text.trim_start()
text.trim_start_to(ref mut zone)
text.trimmed_start(ref mut zone)
text.trim_end()
text.trim_end_to(ref mut zone)
text.trimmed_end(ref mut zone)
text.trim()
text.trim_to(ref mut zone)
text.trimmed(ref mut zone)
text.parse_decimal()
text.parse_decimal_prefix()
text.parse_signed_decimal()
text.parse_signed_decimal_prefix()
text.parse_hex()
text.parse_hex_prefix()
text.bytes()
text.as_slice()
text.as_ptr()
text.copy_to(ref mut zone)

utf8.as_slice()
utf8.len()
utf8.is_empty()
utf8.codepoint_count()
utf8.codepoint_at(byte_index)

os.as_slice()
os.len()
os.is_empty()
os.is_utf8()
os.try_utf8()

c.as_ptr()
c.as_slice()
c.len()
c.is_empty()
```

`remove` and `try_remove` remove one byte and shift the following bytes left.
Use the `try_` form when a missing index is ordinary input. `retain(keep)`
keeps bytes accepted by `keep: fn(ref u8) -> bool` and preserves their order.

String literals coerce to borrowed `Slice[u8]` values when a byte-slice API
expects one, so calls such as `ascii::parse_decimal("123")` and
`text.find("needle")` are valid. They can also act as read-only
`Slice[u8]` receivers, so `"hello".starts_with("he")`,
`"hello".find("ll")`, and `"hello".slice(1, 4).equals("ell")` use the same
borrowed slice vocabulary as named views. They can also initialize local byte
storage: `var bytes: Vec[u8] = "true";` and
`let fixed: [u8, 4] = "true";`.
`std::string::bytes(text)` returns the same kind of view without the trailing
NUL when code wants to name the boundary explicitly. Single-quoted byte
character literals such as `'t'`, `'\n'`, and `'\x74'` are `u8`, so local byte
vectors can still be written as `['t', 'r', 'u', 'e']` when per-byte spelling
is clearer.

`std::string::from(ref mut zone, "text")`, `std::string::copy(ref mut zone,
bytes)`, and `std::string::empty(ref mut zone)` are the natural constructors
for everyday code. The older `from_string` and `from_slice_in` names remain
available when the source kind should be explicit. `text.append`, `append_byte`,
and `append_bytes` grow with the owning zone while hiding the lower-level
`append_string_in`, `append_value_in`, `append_debug_in`, and
`extend_from_slice_in` names from normal call sites.
Tracked local strings can also call `text.append_value(value)` for `Display`
values and `text.append_debug(value)` for `Debug` values; the compiler lowers
those convenience calls to same-zone explicit forms.

`String` stores bytes, so `join_in`, `find`, `contains_slice`, `slice`,
`split_at`, `chunks`, `windows`, and delimiter `split` operate on byte offsets
and borrowed `Slice[u8]` views. `find_text`, `contains_text`,
`starts_with_text`, `ends_with_text`, and `equals_text` accept Ari `string`
values directly by using `std::string::bytes` internally. `join_in` is the
allocating helper: it joins `Slice[Slice[u8]]` parts with a byte separator into
the caller's zone.
`equals_ignore_case`, `starts_with_ignore_case`,
`ends_with_ignore_case`, `index_of_ignore_case`, `contains_ignore_case`,
`trim_start`, `trim_end`, `trim`, `parse_decimal`, `parse_decimal_prefix`,
`parse_signed_decimal`, `parse_signed_decimal_prefix`, `parse_hex`, and
`parse_hex_prefix` intentionally reuse `std::ascii` behavior. The
`_text_ignore_case` forms are literal-friendly wrappers over the same ASCII
policy.
The `try_*` byte accessors return `Option[u8]` for empty or out-of-range
access. The plain trim methods return borrowed `Slice[u8]` views, while
`trim_start_to`, `trim_end_to`, and `trim_to` copy the trimmed bytes into a
target zone and return owned `String` handles. `trimmed_start`, `trimmed_end`,
and `trimmed` are friendlier owned-copy aliases. The whole parse methods require
the whole string to be valid and return `Option[i64]`; the signed decimal form
accepts one optional leading `+` or `-` and still rejects bare signs or trailing
bytes. Prefix parsers return `Option[std::ascii::ParsedInt]` and stop before
the first invalid byte; the signed prefix form counts an accepted sign in
`len`. Trim first when leading or trailing ASCII whitespace should be ignored.

The UTF-8 helpers reuse `std::encoding`. `is_utf8` validates the whole byte
string, `try_utf8` returns `Option[std::string::Utf8]`, `codepoint_count`
returns an `Option[i64]` scalar count, and `codepoint_at` decodes one scalar at
a byte offset into
`Option[std::encoding::Utf8Char]`. `push_codepoint_in` appends one Unicode
scalar encoded as UTF-8 and panics for invalid scalar values. These helpers
work with Unicode scalar values, not grapheme clusters or normalization.
Use `std::string::utf8(bytes)` to construct a validated borrowed `Utf8` view
when a function requires UTF-8. Use `OsStr` for operating-system bytes that may
not be UTF-8, `PathBytes` for path interpretation, and `std::c::CStr` or the
builtin `string` type for NUL-terminated C ABI text. `std::string::c_str(text)`
returns that same `std::c::CStr` borrowed view. String literals can flow
directly into expected `Utf8`, `OsStr`, `PathBytes`, and `CStr` boundary views;
direct `Utf8` literals are validated at compile time. Non-overlapping boundary
methods can also use literal receivers: `"\xC3\xA9".codepoint_count()` uses a
validated `Utf8` view, `"name".is_utf8()` uses an `OsStr` view, and
`"/tmp/bin".file_name()` uses a `PathBytes` view. Generic byte-slice methods
such as `len`, `is_empty`, `find`, and `starts_with` still use the direct
`Slice[u8]` receiver path.

`std::boxed::Box[T]` is a zone-backed single-value owner:

```ari
std::boxed::new<T>(ref mut zone, value)
Box!(T, ref mut zone, value)
box.get()
box.try_get()
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
iter::empty<T>()
iter::once<T>(value)
iter::repeat_with<T>(make_value)
iter::map<T, U, I: std::Iterator[T]>(iter, op)
iter::filter<T, I: std::Iterator[T]>(iter, keep)
iter::take<T, I: std::Iterator[T]>(iter, count)
iter::skip<T, I: std::Iterator[T]>(iter, count)
iter::enumerate<T, I: std::Iterator[T]>(iter)
iter::zip<T, U, I: std::Iterator[T], J: std::Iterator[U]>(left, right)
iter::count<T, I: std::Iterator[T]>(iter)
iter::count_if<T, I: std::Iterator[T]>(iter, predicate)
iter::nth<T, I: std::Iterator[T]>(iter, index)
iter::last<T, I: std::Iterator[T]>(iter)
iter::find_if<T, I: std::Iterator[T]>(iter, predicate)
iter::position<T, I: std::Iterator[T]>(iter, predicate)
iter::any<T, I: std::Iterator[T]>(iter, predicate)
iter::all<T, I: std::Iterator[T]>(iter, predicate)
iter::fold<T, U, I: std::Iterator[T]>(iter, initial, op)
iter::reduce<T, I: std::Iterator[T]>(iter, op)
iter::collect<T, I: std::Iterator[T]>(ref mut zone, iter)
iter::DoubleEndedIterator[T]
iter::ExactSizeIterator[T]
iter::Iterator[T]
iter::IntoIterator[T]
iter::Iterable[T]
```

Root aliases expose `range(start, end)` and `range_inclusive(start, end)`.
Source cursors implement `Iterator[T]::next(self: ref mut Self) -> Option[T]`.
`DoubleEndedIterator[T]` is a child trait of `Iterator[T]` for cursors that
can also yield from the back with `next_back()`.
`ExactSizeIterator[T]` is a child trait of `Iterator[T]` for cursors that can
report an exact remaining `len()`; child bounds can still call parent
`next()`.
Collections that implement `IntoIterator[T]` can be used directly in `for`
loops; `enumerate` yields `(index, value)` tuples and `zip` yields `(left,
right)` tuples. Map-like collections still use explicit `keys()` and
`values()` cursors while entry borrowing and collection-view policy stay
explicit.

The adapter constructors are lazy except for the consumer helpers.
`empty<T>()` yields no values, while `once(value)` yields one value and then
stops. `repeat_with(make_value)` is an infinite source adapter that calls the
zero-argument maker on each `next()`, so bound it with `take`, `zip`, or
another terminating consumer before collecting or extending a vector. Use
`skip` for the usual drop-count adapter because `drop` is a language operation.
`count`, `count_if`, `nth`, `last`, `find_if`, `position`, `any`, `all`,
`fold`, `reduce`, and `collect` advance the source iterator to compute a
single value or owned collection. `collect` builds a `std::vec::Vec[T]` in the
caller-provided zone.

`std::fmt` contains the formatting trait surface:

```ari
fmt::Debug
fmt::Display
Display::format_in(self: ref Self, zone: ref mut Zone)
fmt::FormatSpec
fmt::decimal()
fmt::hex()
fmt::binary()
fmt::octal()
fmt::with_width(spec, width)
fmt::try_with_width(spec, width)
fmt::with_precision(spec, precision)
fmt::try_with_precision(spec, precision)
fmt::left(spec)
fmt::right(spec)
fmt::center(spec)
fmt::uppercase(spec)
fmt::alternate(spec)
fmt::unsigned_in(ref mut zone, value, spec)
fmt::integer_in(ref mut zone, value)
fmt::boolean_in(ref mut zone, value)
fmt::float_in(ref mut zone, value, precision)
fmt::text_in(ref mut zone, value)
fmt::char_in(ref mut zone, value)
fmt::debug_text_in(ref mut zone, value)
fmt::debug_char_in(ref mut zone, value)
fmt::debug_value<T: Debug>(ref mut zone, value)
fmt::write_unsigned<W: io::Writer>(ref mut writer, ref mut zone, value, spec)
fmt::write_integer<W: io::Writer>(ref mut writer, ref mut zone, value)
fmt::write_boolean<W: io::Writer>(ref mut writer, ref mut zone, value)
fmt::write_text<W: io::Writer>(ref mut writer, ref mut zone, value)
fmt::write_value<W: io::Writer, T: Display>(ref mut writer, ref mut zone, value)
fmt::write_debug<W: io::Writer, T: Debug>(ref mut writer, ref mut zone, value)
fmt::print_value<T: Display>(ref mut zone, value)
fmt::println_value<T: Display>(ref mut zone, value)
fmt::print_debug<T: Debug>(ref mut zone, value)
fmt::println_debug<T: Debug>(ref mut zone, value)
```

Built-in `Display` impls cover `char`, `i64`, `u64`, `bool`, `f32`, `f64`,
lowercase `string`, and `std::string::String`. Use explicit impls for domain
structs and enums. Float `Display` uses six fractional digits; call
`fmt::float_in` to pick a precision explicitly. Prefer `fmt::write_value` for
generic Writer-backed display output and `fmt::print_value`/`fmt::println_value`
for generic stdout display output instead of adding new type-suffixed
`write_*` helpers. `char` is a byte-character alias, so `fmt::char_in`,
`String.append_value('A')`, and `format_in!(..., "{}", 'A')` write the byte as
text rather than as the number `65`.
Built-in `Debug` impls cover the same initial scalar/text set. `string` and
`String` debug output are quoted; `char` debug output is single-quoted. Use
`fmt::debug_value`, `fmt::write_debug`, or `fmt::println_debug` when diagnostic
output should use that policy.

The executable formatting path is still macro-based: `print!`, `println!`,
and `format_in!(ref mut zone, "...", values...)`. `{}` uses display
formatting, `{:.N}` gives float precision, and `{:?}` uses debug formatting.
`{name}`, `{name.field}`, and `{name.0}` capture a local binding, named field,
or tuple field without passing it again as a separate argument; `:?` and `:.N`
can be attached to those named captures too. Use ordinary `{}` placeholders
for module paths, indexing, method calls, or computed expressions. Use
`format_in!` for owned formatted strings because Ari does not hide a default
allocation zone; this is also the macro path that can call custom
`Debug::debug_in` impls.

`FormatSpec` is the source-controlled formatting path for unsigned integer
bases, width, integer precision, alignment, uppercase digits, and alternate
prefixes. Build a spec from a base helper and modifiers:

```ari
let spec = fmt::alternate(fmt::uppercase(fmt::with_width(fmt::hex(), 6)));
let text = fmt::unsigned_in(ref mut zone, 255u64, spec);
```

`with_width` and `with_precision` are strict builders for already-validated
values. `try_with_width` and `try_with_precision` return `Option[FormatSpec]`
for runtime input so callers can reject invalid negative width or precision
without panicking.

`integer_in`, `boolean_in`, `text_in`, `char_in`, `debug_text_in`, and
`debug_char_in` are small
allocator-backed helpers for common scalar/text values. `write_*` helpers
format through an explicit temporary zone and then write to any `io::Writer`.
Full custom formatter objects and direct streaming formatters remain roadmap
work; `Debug` dispatch itself is source-level today.

## Comparison

`std::cmp` contains source comparison traits and generic value helpers:

```ari
cmp::Eq[T]
cmp::PartialEq[T]
cmp::Ord[T]
cmp::PartialOrd[T]
cmp::Ordering
cmp::compare<T>(left, right)
cmp::compare_by<T>(left, right, less)
cmp::reverse(ordering)
cmp::then(first, second)
cmp::then_compare<T>(ordering, left, right)
cmp::then_compare_by<T>(ordering, left, right, less)
cmp::is_less(ordering)
cmp::is_equal(ordering)
cmp::is_greater(ordering)
cmp::is_less_or_equal(ordering)
cmp::is_greater_or_equal(ordering)
Ordering::reverse()
Ordering::then(ordering)
Ordering::then_compare<T>(left, right)
Ordering::then_compare_by<T>(left, right, less)
Ordering::is_less()
Ordering::is_equal()
Ordering::is_greater()
Ordering::is_less_or_equal()
Ordering::is_greater_or_equal()
cmp::min<T>(left, right)
cmp::max<T>(left, right)
cmp::clamp<T>(value, low, high)
cmp::is_between<T>(value, low, high)
cmp::min_by<T>(left, right, less)
cmp::max_by<T>(left, right, less)
cmp::clamp_by<T>(value, low, high, less)
cmp::is_between_by<T>(value, low, high, less)
```

`Eq[T]` currently requires `eq(self, other: T) -> bool`. For types without a
builtin equality comparison, `==` dispatches to `eq` and `!=` dispatches to
`!eq(...)`. `Ord[T]` currently requires `lt(self, other: T) -> bool`. For
types without builtin numeric ordering, `<` dispatches to `lt`, `>` swaps the
operands, and `<=`/`>=` negate the opposite `lt` call. `compare`, `min`,
`max`, `clamp`, and `is_between` use that trait bound, so custom ordered values
need an `impl cmp::Ord[T] for T`. The `*_by` forms take a `fn(T, T) -> bool`
less-than callback instead, which is useful for a one-off call-site ordering
without making it the type-wide `Ord` meaning. `std::cmp` already provides
`Eq` and `PartialEq` impls for `bool` and fixed-width integers, plus `Ord` and
`PartialOrd` impls for fixed-width integers.
`Ordering` has `Less`, `Equal`, and `Greater` cases. Use `then`/`then_compare`
or the matching `Ordering` methods to build lexicographic comparisons without
inventing numeric sentinel values.
The method style is usually clearer after a value has already been compared.
`clamp` and `is_between` assert that `low <= high`; `is_between` is inclusive
at both ends. `clamp_by` and `is_between_by` assert `!less(high, low)`.

The root prelude re-exports the value helpers as `min<T>`, `max<T>`,
`clamp<T>`, `is_between<T>`, `min_by<T>`, `max_by<T>`, `clamp_by<T>`, and
`is_between_by<T>`.

## Algorithms

`std::algo` contains source algorithms over borrowed `Slice[T]` views:

```ari
algo::sort<T>(values)
algo::sort_by<T>(values, less)
algo::stable_sort<T>(values)
algo::stable_sort_by<T>(values, less)
algo::stable_sort_in<T>(values, ref mut zone)
algo::stable_sort_by_in<T>(values, less, ref mut zone)
algo::try_stable_sort<T>(values)
algo::try_stable_sort_by<T>(values, less)
algo::binary_search<T>(values, target)
algo::binary_search_by<T>(values, target, less)
algo::lower_bound<T>(values, target)
algo::lower_bound_by<T>(values, target, less)
algo::upper_bound<T>(values, target)
algo::upper_bound_by<T>(values, target, less)
algo::equal_range<T>(values, target)
algo::equal_range_by<T>(values, target, less)
algo::partition_point<T>(values, predicate)
algo::is_sorted<T>(values)
algo::is_sorted_by<T>(values, less)
algo::reverse<T>(values)
algo::reverse_range<T>(values, start, end)
algo::rotate_left<T>(values, count)
algo::rotate_right<T>(values, count)
algo::rotate_range<T>(values, start, end, count)
algo::partition<T>(values, keep)
algo::stable_partition<T>(values, keep)
algo::min<T>(values)
algo::min_by<T>(values, less)
algo::max<T>(values)
algo::max_by<T>(values, less)
algo::clamp<T>(value, low, high)
algo::clamp_by<T>(value, low, high, less)
algo::swap<T>(values, left, right)
algo::fill<T>(values, value)
algo::fill_range<T>(values, start, end, value)
algo::copy<T>(target, source)
algo::copy_within<T>(values, start, end, target)
algo::dedup<T>(values)
algo::dedup_by<T>(values, same)
algo::dedup_by_key<T, K>(values, key)
```

The ordered helpers use `cmp::Ord[T]`; the `*_by` helpers take an explicit
`fn(T, T) -> bool` comparator. `binary_search` and `binary_search_by` return
`Option[i64]`, while bound helpers return sorted insertion indexes.
`sort`/`sort_by` use introsort with duplicate-aware 3-way partitioning and a
heapsort fallback. `stable_sort`/`stable_sort_by` use a merge-sort engine with
a temporary buffer; the `_in` forms take that temporary zone from the caller,
and the `try_` forms return `Result[(), Error]` for layout preflight failures.
`equal_range` returns the matching `(lower, upper)` duplicate range, and
`partition_point` returns the first false predicate index in a
predicate-partitioned slice.
`partition` accepts `fn(ref T) -> bool` and returns the split index;
`stable_partition` preserves relative order while producing the same split
shape. `copy` returns the number of copied elements. `dedup`, `dedup_by`, and
`dedup_by_key` compact consecutive duplicates and return the logical prefix
length.
The current implementation contract is copy-oriented: element materialization
uses raw place loads/stores and is intended for copyable scalar values and plain
Ari-layout aggregates. `copy` is forward element copy, not guaranteed typed
overlap handling. Borrowed-slice `dedup*` returns a prefix length and does not
drop the suffix; `Vec::dedup*` truncates and drops removed values. See
[Value Movement Contracts](value-contracts.md) before using these helpers with
types that own files, sockets, boxes, or other resources.

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

Use `hash::value<T>` for a single value with a `Hash[T]` impl, `hash::pair`
for two hashable values in order, `hash::combine` for two precomputed `u64`
hashes, `hash::bytes` for a borrowed `Slice[u8]`, and `Hasher` plus `write`
calls for incremental hashing. Current built-in impls cover fixed-width signed
and unsigned integer types, `bool`, and `Slice[u8]`.

## Random

`std::random` contains OS entropy and deterministic non-cryptographic PRNG
helpers:

```ari
random::Prng
random::entropy()
random::entropy_raw_result()
random::entropy_result()
random::fill(values)
random::fill_raw_result(values)
random::fill_result(values)
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

Use `entropy()` or `fill(values)` when seed material must come from the host
OS. On hosted Linux, both use `getrandom` first and fall back to
`/dev/urandom`; `fill(values)` writes the caller's byte slice directly instead
of looping through `entropy()` words. The strict helpers terminate on host
entropy failure. Use `entropy_result()` and `fill_result(values)` when failures
should be returned as `std::error::Error`; the `*_raw_result` forms keep the
compatibility `i64` error payload. Use `Prng` for reproducible booleans,
integers, floats, tests, games, randomized algorithms, and shuffling.
`below` and `range` use rejection sampling instead of raw modulo so bounded
integer results are not biased. `Prng` is not cryptographic.

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
convert::try_from<T, U>(value)
convert::try_into<T, U>(value)
```

`identity` returns its input unchanged. `from<T, U>` builds destination `U`
through `convert::From[T]`, and `into<T, U>` turns source `U` into destination
`T` through `convert::Into[T]`. `try_from<T, U>` and `try_into<T, U>` mirror
those helpers through `convert::TryFrom[T]` and `convert::TryInto[T]` and
return `Option` so invalid input stays an ordinary value.

## Math

`std::math` currently contains conservative source-only helpers with `i64`
signatures. The names intentionally avoid type suffixes so they can grow into
generic numeric APIs later without changing call sites:

```ari
math::min_value()
math::max_value()
math::abs(value)
math::sign(value)
math::is_positive(value)
math::is_negative(value)
math::is_zero(value)
math::is_even(value)
math::is_odd(value)
math::checked_add(left, right)
math::checked_sub(left, right)
math::checked_mul(left, right)
math::checked_div(left, right)
math::checked_rem(left, right)
math::checked_neg(value)
math::checked_abs(value)
math::checked_pow(base, exponent)
math::wrapping_add(left, right)
math::wrapping_sub(left, right)
math::wrapping_mul(left, right)
math::wrapping_pow(base, exponent)
math::overflowing_add(left, right)
math::overflowing_sub(left, right)
math::overflowing_mul(left, right)
math::overflowing_pow(base, exponent)
math::saturating_add(left, right)
math::saturating_sub(left, right)
math::saturating_mul(left, right)
math::saturating_div(left, right)
math::saturating_neg(value)
math::saturating_abs(value)
math::saturating_pow(base, exponent)
math::pow(base, exponent)
math::div_floor(numerator, denominator)
math::div_ceil(numerator, denominator)
math::mod_floor(numerator, denominator)
math::gcd(left, right)
math::lcm(left, right)
```

`min_value` and `max_value` return the current signed `i64` bounds without
making callers repeat long sentinel literals. `is_positive`, `is_negative`, and
`is_zero` are predicate forms for the same sign policy as `sign`. `pow` is the
strict convenience spelling: it panics when the exponent is negative or the
result cannot fit in `i64`.
`div_floor` rounds signed division toward negative infinity, `div_ceil` rounds
toward positive infinity, and `mod_floor` returns the matching floor remainder.
The division helpers assert that
`denominator != 0` and that the quotient is representable. `gcd` and `lcm`
normalize negative inputs through absolute values. `lcm` returns `0` when
either input is `0`.

`checked_add`, `checked_sub`, `checked_mul`, `checked_div`, `checked_rem`,
`checked_neg`, `checked_abs`, and `checked_pow` return `Option[i64]`, using
`None<i64>()` for overflow, underflow, division by zero, a negative exponent,
or the unrepresentable signed-minimum division edge. Their `saturating_*`
counterparts clamp to the nearest `i64` bound where that policy is meaningful.
`checked_mul` guards with division before multiplying so the successful branch
is defined. `saturating_div` asserts a non-zero divisor and saturates only
`i64_min / -1`; `saturating_pow` asserts a non-negative exponent and clamps
overflow according to the final sign. `wrapping_add`, `wrapping_sub`,
`wrapping_mul`, and `wrapping_pow` return the two's-complement wrapped result.
`wrapping_mul` routes through `u64` so the modulo behavior is explicit, and
`wrapping_pow` repeats that same multiplication policy. `overflowing_add`,
`overflowing_sub`, `overflowing_mul`, and `overflowing_pow` return an
`(i64, bool)` tuple whose first slot is the wrapped result and whose second
slot is the overflow flag. This keeps `Option` reserved for absent values and
uses tuples for always-present paired values. Other math helpers still use
ordinary `i64` arithmetic internally, so generic cross-width helpers remain
future numeric-policy work.

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
bits::checked_align_down(value, alignment)
bits::checked_align_up(value, alignment)
bits::wrapping_align_up(value, alignment)
bits::count_ones(value)
bits::population_count(value)
bits::count_zeros(value)
bits::byte_swap(value)
bits::leading_zeros(value)
bits::trailing_zeros(value)
bits::leading_ones(value)
bits::trailing_ones(value)
```

`is_set` requires all bits from `mask`; `any_set` requires at least one
overlap. `rotate_left` and `rotate_right` assert that `count` is non-negative
and then rotate modulo 64. `align_down` and `align_up` assert that `alignment`
is a non-zero power of two. `checked_align_down` and `checked_align_up` return
`Option[u64]` instead of asserting on bad alignment; `checked_align_up` also
returns `None` on `u64` overflow. `wrapping_align_up` keeps the alignment
assertion but wraps the addition before masking. These helpers currently have
`u64` signatures and intentionally avoid type suffixes so future generic
integer APIs can keep the same names. The zero value has 64 leading zeros,
64 trailing zeros, zero
leading ones, and zero trailing ones; `~0u64` has 64 leading ones and 64
trailing ones.
`bit_width` returns the number of bits needed to represent a value,
`floor_power_of_two` and `ceil_power_of_two` round to nearby powers of two, and
`low_mask(width)` returns a mask with the lowest `width` bits set. `low_mask`
accepts widths from 0 through 64. `population_count` is an explicit alias for
`count_ones`, and `byte_swap` reverses the eight bytes in a `u64`.

## ASCII

`std::ascii` contains byte-oriented helpers for ASCII-only text and parser
code. Public names stay natural because the module path already says the
policy:

```ari
ascii::ParsedInt
ascii::is_digit(ch)
ascii::is_lower(ch)
ascii::is_upper(ch)
ascii::is_alpha(ch)
ascii::is_alphanumeric(ch)
ascii::is_blank(ch)
ascii::is_whitespace(ch)
ascii::is_control(ch)
ascii::is_printable(ch)
ascii::is_graphic(ch)
ascii::is_punctuation(ch)
ascii::is_hex_digit(ch)
ascii::to_lower(ch)
ascii::to_upper(ch)
ascii::digit_value(ch)
ascii::hex_value(ch)
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
ascii::parse_signed_decimal(bytes)
ascii::parse_signed_decimal_prefix(bytes)
ascii::parse_hex(bytes)
ascii::parse_hex_prefix(bytes)
```

`is_blank` covers `' '` and `'\t'`. `is_whitespace` covers `' '`, `'\t'`,
`'\n'`, and `'\r'`. `is_printable` includes space; `is_graphic` excludes
space. `is_punctuation` is true for graphic ASCII bytes that are not letters or
digits. Scalar helpers take `char`, the standard alias for an ASCII `u8`.
Prefer character literals such as `'0'` and `'A'` over decimal byte casts for
ASCII call sites.

Slice helpers take `Slice[u8]` and accept string literals directly. For
example, `ascii::parse_decimal("123")` and
`ascii::starts_with_ignore_case("AriLang", "ari")` lower to borrowed literal
byte slices without the trailing NUL. Use `var bytes: Vec[u8] = "text";` when
the bytes should become mutable local storage.

`digit_value` and `hex_value` return `Option[i64]`. Non-digit input returns
`None<i64>()` where appropriate.

`equals_ignore_case`, `starts_with_ignore_case`, `ends_with_ignore_case`,
`index_of_ignore_case`, and `contains_ignore_case` operate on `Slice[u8]` and
fold only ASCII letter case. Empty prefixes, suffixes, and search needles
match. `index_of_ignore_case` returns the first matching byte offset or `-1`;
`contains_ignore_case` returns the same search as a bool. `skip_whitespace`,
`trim_start`, `trim_end`, and `trim` also operate on `Slice[u8]` and return
either a byte offset or a borrowed sub-slice. `parse_decimal`,
`parse_signed_decimal`, and `parse_hex` parse the entire slice and return
`Option[i64]`; empty input or invalid bytes return `None<i64>()`.
`parse_signed_decimal` accepts one optional leading `+` or `-` and requires at
least one digit after it. These parser helpers do not trim and do not define
wrapping overflow behavior: decimal, signed decimal, and hexadecimal digit runs
that exceed `i64` return `None`. Signed decimal parsing accepts
`-9223372036854775808` as the minimum `i64` value.

`ParsedInt` carries `value: i64` and `len: i64` for prefix parser results.
`parse_decimal_prefix`, `parse_signed_decimal_prefix`, and `parse_hex_prefix`
parse only the leading digit run, stop before the first invalid byte, and
return `None<ParsedInt>()` when the first byte is empty or invalid.
`parse_signed_decimal_prefix` accepts one optional leading sign and counts it
in `len`. The prefix parsers do not trim or recognize hexadecimal prefixes
such as `0x`; a digit run that overflows `i64` returns `None<ParsedInt>()`
instead of a partial wrapped value.

## Parsing

`std::parse` contains whole-input parsers over ASCII-trimmed `Slice[u8]`
values:

```ari
parse::integer(bytes)
parse::is_integer(bytes)
parse::integer_or(bytes, fallback)
parse::integer_radix(bytes, radix)
parse::is_integer_radix(bytes, radix)
parse::integer_radix_or(bytes, radix, fallback)
parse::hex_integer(bytes)
parse::is_hex_integer(bytes)
parse::hex_integer_or(bytes, fallback)
parse::binary_integer(bytes)
parse::is_binary_integer(bytes)
parse::binary_integer_or(bytes, fallback)
parse::octal_integer(bytes)
parse::is_octal_integer(bytes)
parse::octal_integer_or(bytes, fallback)
parse::boolean(bytes)
parse::is_boolean(bytes)
parse::boolean_or(bytes, fallback)
parse::is_float(bytes)
parse::float_or(bytes, fallback)
parse::float(bytes)
```

`integer` returns `Option[i64]` and accepts optional `+` or `-` signs.
`is_integer` validates the same shape, and `integer_or` returns a caller
fallback on invalid input. Decimal and radix integer parsers reject values
outside the `i64` range instead of wrapping. `integer_radix` accepts bases `2`
through `36` with ASCII alphanumeric digits, and `hex_integer` /
`binary_integer` / `octal_integer` are readable wrappers for common bases.
These radix parsers trim whitespace but do not recognize prefixes such as
`0x`, `0b`, or leading-zero octal policy. `boolean`
returns `Option[bool]` and accepts only lowercase `true` and `false`;
`is_boolean` and `boolean_or` provide the same validator/fallback pattern.
`is_float` validates a decimal float shape with optional sign, fraction, and
exponent. `float_or` returns a fallback for invalid input, while `float` panics
on invalid input. `Option[f64]` is future work because the compiler does not
lower float enum payloads yet.

## Encoding

`std::encoding` contains text validation and byte codecs:

```ari
encoding::is_ascii(bytes)
encoding::is_unicode_scalar(scalar)
encoding::utf8_count(bytes)
encoding::is_utf8(bytes)
encoding::utf8_width(first_byte)
encoding::utf8_encoded_len(scalar)
encoding::utf8_at(bytes, byte_index)
encoding::utf8_next_index(bytes, byte_index)
encoding::encode_utf8_in(ref mut zone, scalar)
encoding::try_encode_utf8_in(ref mut zone, scalar)
encoding::utf16_count(words)
encoding::is_utf16(words)
encoding::hex_encoded_len(bytes)
encoding::encode_hex_in(ref mut zone, bytes)
encoding::hex_decoded_len(bytes)
encoding::can_decode_hex(bytes)
encoding::decode_hex_in(ref mut zone, bytes)
encoding::try_decode_hex_in(ref mut zone, bytes)
encoding::base64_encoded_len(bytes)
encoding::encode_base64_in(ref mut zone, bytes)
encoding::base64_decoded_len(bytes)
encoding::can_decode_base64(bytes)
encoding::decode_base64_in(ref mut zone, bytes)
encoding::try_decode_base64_in(ref mut zone, bytes)
```

`utf8_count` and `utf16_count` validate and return code-point counts through
`Option[i64]`; the `is_*` forms return only a bool. `Utf8Char` is the decoded
UTF-8 scalar wrapper with `scalar()`, `len()`, and `next_index(byte_index)`.
`utf8_at` validates at one byte offset, while `utf8_width` only classifies a
lead byte. `encode_utf8_in` returns an owned byte `String` for one Unicode
scalar and panics for invalid scalar values. Use `try_encode_utf8_in` for
untrusted scalar input. Hex encoding emits lowercase digits and decoding
accepts ASCII hex digits. Base64 uses the standard `+`/`/` alphabet with `=`
padding. Decoders have `try_decode_*_in` forms that return `Option[String]`
for untrusted input. The plain `decode_*_in` helpers are asserting forms and
panic on invalid input.

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
