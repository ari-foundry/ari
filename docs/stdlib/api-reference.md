# Standard Library API Reference

This is a compact guide to the current public `std` surface. The source of
truth is still `lib/std.arih`, `lib/std/*.arih`, and
`tests/std_api_manifest.txt`.

For an exhaustive generated spelling list, use
[generated/api-index.md](generated/api-index.md). This hand-written page keeps
the common APIs readable; the generated index keeps every public declaration
findable and tied to its manifest coverage note.

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
read_line(ref mut zone)
read_line_text()
read_line_owned(ref mut zone)
input(ref mut zone)
input_text()
input_owned(ref mut zone)
arg_count()
arg(ref mut zone, index)
arg_text(index)
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
test::Bench

test::report()
test::scratch(capacity)
test::temp_file(ref mut zone)
test::temp_dir(ref mut zone)
test::bench(iterations)
test::benchmark(iterations)
test::check(ref mut report, condition)
test::equal<T>(ref mut report, left, right)
test::not_equal<T>(ref mut report, left, right)
test::matches_snapshot(actual, expected)
test::golden_matches(actual, expected)
test::check_snapshot(ref mut report, actual, expected)
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

bench.elapsed()
bench.iterations()
bench.elapsed_nanos()
bench.nanos_per_iter()
```

`Report` stores pass/fail counts. `equal` and `not_equal` use generic
comparison, so the public names stay natural rather than type-suffixed.
`finish()` returns `0` when no failures were recorded and `1` otherwise.
`scratch(capacity)` creates an owned `Zone` for tests; destroy it explicitly
with `zone::destroy(zone)` when the test is done.
`temp_file` and `temp_dir` return `Result[..., Error]` wrappers around the
process temp-path helpers. `bench`/`benchmark` provide a minimal elapsed-time
handle. `matches_snapshot`, `golden_matches`, and `check_snapshot` compare
actual bytes with expected golden bytes supplied by the test.

The compiler test runner is available as either `ari --test file.ari` or the
friendlier `ari test file.ari` subcommand. `--test-filter name` and
`ari test ... --filter name` select `@test` functions whose names contain the
given substring. `void` tests pass when they return normally; `i64` tests may
return a non-zero status to stop the generated runner and become the process
exit code. The runner writes `test name ...`, `ok name`, and `failed name`
progress lines to `stderr`. Panic/assert failures still stop the process
through the current panic hook, but the last `test name ...` line identifies
the running test.

Debug printing can use `print`/`println` with `{:?}` for built-in printable
values, `format_in!(ref mut zone, "{:?}", value)` for values implementing
`fmt::Debug`, or the `std::fmt`/`std::io` writer surface.

Recoverable error values live in `std::error`:

```ari
error::Kind
error::ErrorKind
error::Error

error::new(kind)
error::with_code(kind, code)
error::try_with_code(kind, code)
error::from_errno(code)
error::from_os_code(code)
error::try_from_errno(code)
error::try_from_os_code(code)
error::from_raw(raw)
error::try_from_raw(raw)
error::map_raw[T](value)
error::map_errno[T](value)
error::map_os_code[T](value)
error::to_raw[T](value)
error::kind(ref error)
error::code(ref error)
error::raw(ref error)
error::is_kind(ref error, kind)
error::is_not_found(ref error)
error::is_interrupted(ref error)
error::is_connection_refused(ref error)
error::is_retryable(ref error)
error::name(ref mut zone, kind)
error::message(ref mut zone, ref error)
error::name_text(kind)
error::message_text(ref error)

reason.kind()
reason.code()
reason.raw()
reason.is_kind(kind)
reason.is_not_found()
reason.is_interrupted()
reason.is_connection_refused()
reason.is_retryable()
reason.name(ref mut zone)
reason.message(ref mut zone)
reason.name_text()
reason.message_text()
```

Use `Error` for OS/runtime/library failures, and use `ErrorKind` for the root
alias of `std::error::Kind`. Direct `Result[T, Error]` is the preferred shape
for recoverable OS/runtime/library failures. The raw scalar representation is
kept for runtime, FFI, and compatibility bridges through
`map_raw`/`to_raw`. The strict constructors are for trusted
values; use `try_with_code`, `try_from_errno`, and `try_from_raw` when
validating untrusted boundary data.
The fs/io/net/os/process modules re-export this shared payload as
`fs::Error`, `io::Error`, `net::Error`, `os::Error`, and `process::Error`
with matching `ErrorKind` aliases. `Error` and `error::Kind` implement
`Display`/`Debug` for stable logs and diagnostics.

Level-prefixed diagnostic logging lives in `std::log`:

```ari
log::Level
log::Trace
log::Debug
log::Info
log::Warn
log::Error

log::rank(level)
log::name(ref mut zone, level)
log::name_text(level)
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

c::from(bytes)
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
string literal with `c::from("name")`, assign a literal directly when
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
context::arg(ref mut zone, index)
context::arg_text(index)
context::thread_id()
context::cwd(ref mut zone)
context::cwd_text()
context::executable_path(ref mut zone)
context::executable_path_text()
context::has_args()
context::has_arg(index)
context::try_arg(ref mut zone, index)
context::try_arg_text(index)
context::user_arg_count()
context::has_user_args()
context::is_main_thread()
context::has_cwd()
context::try_cwd(ref mut zone)
context::try_cwd_text()
context::cwd_os()
context::try_cwd_os()
context::cwd_path()
context::has_executable_path()
context::try_executable_path(ref mut zone)
context::try_executable_path_text()
context::executable_path_os()
context::try_executable_path_os()
arg_count()
arg(ref mut zone, index)
arg_text(index)
has_arg(index)
```

`has_arg(index)` is true only for `0 <= index < context::argc()`. It is the
preferred guard before reading optional host arguments. Natural context APIs
copy host text into a zone-backed `std::string::String` so callers can keep the
value after later runtime calls. The `_text` variants expose the raw borrowed
runtime `string` buffer for compatibility and low-level bridge code.
Out-of-range raw argument access returns an empty string, while
`try_arg(ref mut zone, index)` returns `None`.

`user_arg_count()` excludes `argv[0]`, `has_user_args()` is its boolean form,
and `thread_id()` returns the Ari runtime thread id. The main thread is `0`, so
`is_main_thread()` is true for current executable builds.
`context::cwd(ref mut zone)` and `context::executable_path(ref mut zone)` are
startup snapshots captured by `@ari_entry`; use
`std::env::current_dir(ref mut zone)` when code needs the current process
directory after possible `chdir` calls.

Application code should usually use the user-facing `std::env` wrappers:

```ari
env::arg_count()
env::arg(ref mut zone, index)
env::args(ref mut zone)
env::args_os(ref mut zone)
env::arg_optional(ref mut zone, index)
env::arg_os(index)
env::arg_os_optional(index)
env::arg_os_unchecked(index)
env::has_arg(index)
env::try_arg(ref mut zone, index)
env::try_arg_os(index)
env::program_name(ref mut zone)
env::program_name_optional(ref mut zone)
env::program_name_os()
env::program_name_os_optional()
env::var(ref mut zone, name)
env::var_optional(ref mut zone, name)
env::var_or_default(ref mut zone, name)
env::var_os(name)
env::var_os_optional(name)
env::var_os_or_default(name)
env::get(ref mut zone, name)
env::get_os(name)
env::get_or_default(ref mut zone, name)
env::get_os_or_default(name)
env::has(name)
env::try_get(ref mut zone, name)
env::try_get_os(name)
env::set_var(name, value)
env::set(name, value)
env::set_unchecked(name, value)
env::remove_var(name)
env::remove(name)
env::remove_unchecked(name)
env::current_dir(ref mut zone)
env::current_dir_optional(ref mut zone)
env::current_dir_or_default(ref mut zone)
env::try_current_dir(ref mut zone)
env::current_dir_os()
env::current_dir_os_optional()
env::try_current_dir_os()
env::current_dir_path()
env::current_dir_path_optional()
env::try_current_dir_path()
env::set_current_dir(path)
env::set_current_dir_unchecked(path)
env::executable_path(ref mut zone)
env::executable_path_optional(ref mut zone)
env::executable_path_or_default(ref mut zone)
env::try_executable_path(ref mut zone)
env::executable_path_os()
env::executable_path_os_optional()
env::try_executable_path_os()
env::executable_path_path()
env::executable_path_path_or_default()
env::executable_path_path_optional()
env::try_executable_path_path()
env::home_dir()
```

`env::arg(ref mut zone, index)` returns `Result[String, Error]`, using
`NotFound` for an out-of-range argument index. `env::arg_optional(ref mut zone,
index)` and `env::try_arg(ref mut zone, index)` keep only the optional success
payload. Raw startup-context strings stay behind `std::context` and boundary
helpers. `env::program_name(ref mut zone)` follows the same Result policy for
`argv[0]`, and
`env::program_name_optional(ref mut zone)` is the optional form.
`env::args(ref mut zone)` collects all arguments into an owned
`Vec[std::string::String]`, while `env::args_os(ref mut zone)` collects
OS-string views for CLI code that wants byte-preserving argument handling.

`env::arg_os(index)`, `env::arg_os_optional(index)`,
`env::arg_os_unchecked(index)`, `env::try_arg_os(index)`,
`env::program_name_os()`, and `env::program_name_os_optional()` expose the same
values as `std::string::OsStr` when an argument should stay in OS-string form
until the caller chooses bytes or UTF-8.

`env::var(ref mut zone, name)` returns `Option[String]` for environment
variables because a missing variable is ordinary configuration absence.
`env::var_optional(ref mut zone, name)` and
`env::try_get(ref mut zone, name)`
keep the same optional shape, while `env::var_or_default(ref mut zone, name)`
and `env::get_or_default(ref mut zone, name)` copy the fallback into owned
`String` values. `env::get(ref mut zone, name)` is the Result-returning
lookup and uses `NotFound` for missing names. Public environment variable names
and values are borrowed byte text (`Slice[u8]`): string literals work directly,
immutable literal bindings keep their byte length, and owned `String` locals are
viewed as byte slices at the call site. Explicit `ref String` borrows also
coerce to the same read-only byte view.
`env::set_var(name, value)` overwrites a current-process variable and
`env::remove_var(name)` unsets it; both return `Result[(), Error]`.
`env::set(name, value)` and `env::remove(name)` are compatibility
aliases with the same Result behavior. `set_unchecked` and `remove_unchecked`
keep the older boolean compatibility shape. `env::current_dir(ref mut zone)`,
`env::executable_path(ref mut zone)`, and `env::set_current_dir(path)` return
`Result[..., Error]`; `_optional` and `try_*` wrappers keep only the success
payload, `_or_default` wrappers copy fallback values into owned `String`
handles, and `_unchecked` names are explicit information-discarding helpers.
Portable child-process spawn handles remain roadmap work; thread helpers live
in `std::thread`.

`env::var_os(name)` returns an `Option[OsStr]` environment view.
`env::var_os_optional(name)` and `env::try_get_os(name)` keep the same optional
shape, and `env::var_os_or_default(name)` /
`env::get_os_or_default(name)` are the compatibility fallbacks.
`env::get_os(name)` is the Result-returning
OS-string lookup.
`env::current_dir_os()` / `current_dir_os_optional()` and
`env::executable_path_os()` / `executable_path_os_optional()` expose path-like
host data as OS strings. `env::current_dir_path()` and
`env::current_dir_path_optional()` expose the current directory as
`std::path::PathBytes`; `env::executable_path_path()` does the same for the
running executable. `env::home_dir()` returns `Option[PathBytes]` from `HOME`,
which is enough for package-manager paths such as `~/.ari` on the current
POSIX-backed host slice.

Target and platform facts live in `std::target`:

```ari
target::triple(zone)
target::triple_text()
target::arch()
target::arch_name(zone)
target::arch_name_text()
target::os()
target::os_name(zone)
target::os_name_text()
target::env()
target::env_name(zone)
target::env_name_text()
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
process::exit_code(value)
process::success_code()
process::failure_code()
process::is_success(code)
process::is_failure(code)
process::is_root()
process::fork()
process::fork_raw()
process::wait_status(pid)
process::wait(pid)
process::wait_raw(pid)
process::is_child(pid)
process::is_parent(pid)
process::is_fork_error(pid)
process::is_wait_error(status)
process::signal(value)
process::sig_check()
process::sighup()
process::sigint()
process::sigquit()
process::sigkill()
process::sigterm()
process::arg(value)
process::arg_bytes(zone, value)
process::env_var(name, value)
process::env_var_bytes(zone, name, value)
process::command(program)
process::command_with_args(program, args)
process::spawn(command)
process::spawn_piped(command)
process::status(command)
process::status_code(command)
process::status_with_stdin(command, values)
process::status_with_stdin_string(command, text)
process::exit_status(command)
process::output_in(command, zone)
process::output(command, zone)
process::exec(command)
process::kill(pid, signal)
process::kill_signal(pid, signal)
process::terminate(pid)
process::current_dir(zone)
process::current_dir_optional(zone)
process::current_dir_or_default(zone)
process::try_current_dir(zone)
process::executable_path(zone)
process::executable_path_optional(zone)
process::executable_path_or_default(zone)
process::try_executable_path(zone)
process::temp_file(zone)
process::temp_file_in(zone, prefix)
process::temp_dir(zone)
process::temp_dir_in(zone, prefix)

process::ChildStdin
process::ChildStdout
process::ChildStderr
process::ChildPipes

process::Command::new(program)
process::Command::with_args(program, args)
Command::arg(zone, value)
Command::arg_bytes(zone, value)
Command::arg_value(zone, value)
Command::args(args)
Command::clear_env()
Command::env(zone, name, value)
Command::env_bytes(zone, name, value)
Command::env_values(env_values)
Command::env_var(zone, name, value)
Command::env_value(zone, value)
Command::inherit_env()
Command::current_dir(path)
Command::current_dir_bytes(zone, path)
Command::current_dir_path(zone, path)
Command::with_arg(zone, value)
Command::with_clear_env()
Command::with_env(zone, name, value)
Command::with_inherit_env()
Command::with_current_dir(path)
Command::spawn()
Command::spawn_piped()
Command::spawn_with_stdin_file(path)
Command::spawn_with_stdin_null()
Command::status()
Command::status_with_stdin(values)
Command::status_with_stdin_string(text)
Command::status_with_stdin_file(path)
Command::status_with_stdin_file_bytes(zone, path)
Command::status_with_stdin_file_path(zone, path)
Command::status_with_stdin_null()
Command::status_code()
Command::exit_status()
Command::output_in(zone)
Command::output(zone)
Command::exec()

ExitCode::raw()
ExitCode::code()
ExitCode::is_success()
ExitCode::is_failure()
ExitCode::exit()

ExitStatus::raw()
ExitStatus::exited()
ExitStatus::signaled()
ExitStatus::stopped()
ExitStatus::is_stopped()
ExitStatus::continued()
ExitStatus::is_continued()
ExitStatus::code()
ExitStatus::code_or(fallback)
ExitStatus::exit_code()
ExitStatus::signal()
ExitStatus::signal_or(fallback)
ExitStatus::stop_signal()
ExitStatus::stop_signal_or(fallback)
ExitStatus::core_dumped()
ExitStatus::is_core_dumped()
ExitStatus::is_success()
ExitStatus::success()
ExitStatus::is_failure()

Signal::raw()
Signal::is_check()

Output::status()
Output::exit_status()
Output::is_success()
Output::stdout()
Output::stdout_string(zone)
Output::stderr()
Output::stderr_string(zone)

TempFile::path()
TempFile::as_c_str()
TempFile::as_fd()
TempFile::is_open()
TempFile::close() -> Result[(), process::Error]
TempFile::close_bool() -> bool
TempFile::remove()
TempFile::close_and_remove()

TempDir::path()
TempDir::as_c_str()
TempDir::remove()

Child::pid()
Child::wait()
Child::wait_status()
Child::kill(signal)
Child::signal(signal)
Child::terminate()
Child::detach()

ChildPipes::child()
ChildPipes::child_mut()
ChildPipes::pid()
ChildPipes::stdin()
ChildPipes::stdout()
ChildPipes::stderr()
ChildPipes::close_stdin()
ChildPipes::close_stdout()
ChildPipes::close_stderr()
ChildPipes::close_streams()
ChildPipes::wait()
ChildPipes::wait_status()
ChildPipes::kill(signal)
ChildPipes::signal(signal)
ChildPipes::terminate()
ChildPipes::detach()
```

`id()` returns the host process id as `i64`. `uid()` and `gid()` return the
current user and group ids. `is_root()` is the source convenience check for
`uid() == 0`. `exit(code)` terminates the process and does not return.
`abort()` terminates through the host abnormal-termination path. The status
helpers are source functions for the common `0` success and `1` failure
convention. `ExitCode` is the typed wrapper for code values before returning
or calling `exit`.

`fork()` and `wait_status(pid)` are the preferred POSIX child-process helpers
when failure matters. `fork()` returns `Ok(0)` in
the child, `Ok(child_pid)` in the parent, or `Err(Error)` from the host `fork`
failure. `wait_status(pid)` returns a typed `ExitStatus`, maps host
`waitpid` failures through `std::c::error()`, and preserves signal termination
without flattening it into an `i64` sentinel. `wait(pid)` returns a normal exit
code as `Result[i64, Error]` and reports signal termination as `Error(Other)`.

`fork_raw()` and `wait_raw(pid)` are the raw compatibility slice on the
Linux/LLVM runtime path. `fork_raw()` returns `0` in the child, a positive child
pid in the parent, and a negative value on failure; use `is_child`,
`is_parent`, and `is_fork_error` to make that branch readable. `wait_raw(pid)`
returns a normal child exit status or `-1`; use `is_wait_error` for that
sentinel.

`Command` is the higher-level child-process builder. Use `process::arg` for
argv entries and `process::env_var` for child environment assignments, or use
the explicit-zone appenders `Command::arg(ref mut zone, value)` and
`Command::env(ref mut zone, name, value)` / `Command::env_var(ref mut zone,
name, value)` when the builder should grow one item at a time. Use
`arg_bytes`, `env_var_bytes`, `Command::arg_bytes`, `Command::env_bytes`,
`Command::current_dir_bytes`, and `Command::current_dir_path` for owned
`String`, `PathBuf`, `PathBytes`, or `Slice[u8]` values; those helpers return
`Result` and reject interior NUL before building C argv/env/cwd storage.

```ari
var zone = zone::temp(128);
var cmd = process::Command::new("sh")
  .with_arg(ref mut zone, "-c")
  .with_arg(ref mut zone, "exit 7");
let status = cmd.status();
```

The `with_arg`, `with_env`, `with_clear_env`, `with_inherit_env`, and
`with_current_dir` helpers are by-value chainable forms. The mutating
`arg`/`env` forms still return `void` because they take both `ref mut Command`
and `ref mut Zone`.

Child environments inherit the parent environment by default. Use
`clear_env()` or `with_clear_env()` before `env(...)`/`with_env(...)` when a
tool should start from an empty environment and add back only selected values.
Use `inherit_env()` or `with_inherit_env()` to return to the default inherited
policy while preserving the configured assignment list. On the current
Linux/POSIX path, clear setup uses `clearenv(3)` in the child immediately
before applying configured `setenv(3)` assignments and `execvp`.

`status()` spawns and waits, returning typed `ExitStatus` so natural fallible
process APIs preserve signal termination detail. `status_code()` is the
explicit normal-exit-code compatibility helper and reports signal termination
as `Error(Other)`. `exit_status()` remains as a readable typed-status alias.
`spawn()` returns a `Child` handle. `exec()` replaces the current process on
success and returns `Err(Error)` only if the host `execvp` path fails.
`kill(pid, signal)` and
`Child::kill(signal)` return `Result[(), Error]`; `kill_signal` and
`Child::signal` use the typed `Signal` wrapper. `sig_check()` is signal `0`,
and `terminate` sends `SIGTERM`. `Child::detach()` consumes the child handle as
an explicit marker that Ari will not wait through that handle; on the current
POSIX backend it does not daemonize or install a reaper, so ordinary child
processes should still use `wait` or `wait_status`.
Module-level `process::spawn(ref cmd)`, `process::status(ref cmd)`,
`process::spawn_piped(ref cmd)`, `process::status_code(ref cmd)`,
`process::exit_status(ref cmd)`, `process::output_in(ref cmd, ref mut zone)`,
`process::output(ref cmd, ref mut zone)`, and `process::exec(ref cmd)` are
direct wrappers over the matching `Command` methods for call sites that prefer
function-style process APIs.

`Command::status_with_stdin(values)` and
`process::status_with_stdin(ref cmd, values)` spawn the child with a pipe
connected to stdin, write the complete byte slice from the parent, close the
write end, and then wait for typed `ExitStatus`. Use
`status_with_stdin_string(text)` for string-literal or UTF-8 source text. These
helpers are bounded-input conveniences for CLI-style programs; if the parent
cannot write all bytes, the helper waits for the child and returns the writer
`Error`.

`ExitStatus::code()` returns `Some(code)` only for normal exits.
`ExitStatus::signal()` returns `Some(signal)` only for signal termination.
`ExitStatus::stopped()`/`is_stopped()` and `stop_signal()` expose stopped-child
wait-status detail, `continued()`/`is_continued()` recognizes a continued
status, and `core_dumped()`/`is_core_dumped()` reports the POSIX core-dump bit
for signaled children. `code_or`, `signal_or`, and `stop_signal_or` are
convenience fallbacks for compact control flow, `exit_code()` returns the typed
`ExitCode` form. `raw()` exposes the hosted wait-status bits for diagnostics.

`output_in(zone)` is the captured-output helper. It spawns the child with
stdout and stderr redirected to pipes, drains both streams with descriptor
readiness, waits for the child, and returns an `Output` whose byte buffers live
in the provided zone:

```ari
var zone = zone::temp(512);
var args = [process::arg("-c"), process::arg("printf 'ok'")];
var cmd = process::command_with_args("sh", args.as_slice());
let result = cmd.output(ref mut zone);
```

Use `Output::exit_status()` for typed status, `Output::status()` for the normal
exit code compatibility accessor, `Output::is_success()` for the standard
success check, and `stdout()` / `stderr()` for borrowed `Slice[u8]` views.
`stdout_string(zone)` and `stderr_string(zone)` validate the captured bytes as
UTF-8, copy them into a zone-owned `String`, and return `Error(InvalidData)` for
non-UTF-8 output. `output` drains stdout and stderr with descriptor readiness
so one captured stream cannot fill its pipe while the other is being read; it
still stores the complete capture in memory. File-backed and `/dev/null` stdin
redirection helpers plus bounded pipe-backed stdin status helpers exist on
`Command`. Fork-based `spawn`, `status`, stdin-redirection, and `output`
helpers use a close-on-exec setup-error pipe so `chdir`, stdin open/`dup2`,
stdout/stderr `dup2`, or `execvp` failures return `Err(Error)` to the parent
instead of being hidden as status `127`.

Use `Command::spawn_piped()` or `process::spawn_piped(ref cmd)` when the parent
needs interactive child IO. It returns `ChildPipes`, which owns a `Child` plus
the child stdin writer, stdout reader, and stderr reader. Write through
`std::io::write_all<process::ChildStdin>(child.stdin(), bytes)`, close stdin
with `close_stdin()` to signal EOF, read stdout/stderr with the normal
`PipeReader`/`Reader` helpers, and then call `wait_status()` or `wait()`.
`ChildPipes::detach()` closes the pipe endpoints and explicitly discards
Ari-side wait ownership. Portable Windows mapping and platform-specific status
detail are still future process-library work.

`ChildStdin`, `ChildStdout`, and `ChildStderr` name the current pipe endpoint
types used by streaming process IO. `current_dir`,
`try_current_dir`, `executable_path`, and `try_executable_path` are
process-oriented wrappers around the `std::env` path helpers; the natural names
return `Result`, and optional/default wrappers make discarded errors explicit.
`temp_file` and
`temp_dir` create unique hosted paths under `/tmp`; removal is explicit through
`TempFile::close_and_remove`, `TempFile::remove`, or `TempDir::remove`.

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
os::pipe_optional()

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
owned.clone()
owned.close_on_exec()
owned.close_on_exec_optional()
owned.set_close_on_exec(enabled)
owned.set_close_on_exec_bool(enabled)
owned.is_nonblocking()
owned.is_nonblocking_optional()
owned.set_nonblocking(enabled)
owned.set_nonblocking_bool(enabled)
owned.close()
owned.close_bool()

pipe.read_end()
pipe.write_end()
pipe.take_read_end()
pipe.take_write_end()
pipe.close_read_end()
pipe.close_read_end_bool()
pipe.close_write_end()
pipe.close_write_end_bool()
pipe.close()
pipe.close_bool()
```

`Fd` is non-owning. It identifies a descriptor but does not close, duplicate,
or mutate it. `std::fs::File.descriptor()` returns an `Fd` view over a file
handle without transferring ownership.

`OwnedFd` owns close responsibility for a raw descriptor. Construct it with
`OwnedFd::from_raw(raw)` only when the caller is taking ownership of exactly
one close. `as_fd()` borrows the descriptor as `Fd`, `take()` disarms the owner
without closing, and `close()` disarms before calling the runtime close hook so
the same handle cannot close twice. `try_clone()` duplicates the descriptor and
returns `Option[OwnedFd]`; `clone()` returns `Result[OwnedFd, Error]`. The
original and cloned owners close independently. `close_on_exec()` and
`is_nonblocking()` return `Result[bool, Error]`, and the setters return
`Result[(), Error]`. The `_optional` and `_bool` forms are compatibility
helpers for callers that intentionally discard the reason. Readiness APIs, raw
syscalls, signals, and memory mapping remain future `std::os` work after richer
error policy is stable.

`Pipe` owns the read and write descriptors returned by `os::pipe()`.
`read_end()` and `write_end()` borrow non-owning `Fd` views, `take_read_end()`
and `take_write_end()` move individual owned ends out of the pair, and
`close_read_end()`, `close_write_end()`, or `close()` explicitly release the
remaining descriptors. `os::pipe()` returns `Result[Pipe, Error]`;
`os::pipe_optional()` returns `None` if the hosted pipe call fails.

## Paths

`std::path` contains source-only lexical path helpers over path byte slices.
POSIX-style `/` behavior remains the default; explicit `windows_*` helpers
classify Windows-shaped bytes without changing normal join/component behavior:

```ari
path::Path
path::PathBytes
path::PathBuf
path::Component
path::ComponentKind
path::bytes(path)
path::from_os(os)
path::from_bytes(ref mut zone, path)
path::from(ref mut zone, text)
path::to_string(ref mut zone, path)
path::is_separator(value: char)
path::is_empty(path)
path::contains_nul(path)
path::as_bytes(path)
path::is_absolute(path)
path::is_relative(path)
path::is_windows_separator(value: char)
path::is_windows_absolute(path)
path::has_windows_drive_prefix(path)
path::windows_drive(path)
path::is_windows_drive_absolute(path)
path::is_windows_drive_relative(path)
path::is_windows_unc(path)
path::windows_unc_prefix(path)
path::trim_trailing_separators(path)
path::components(path)
path::components_with_kinds(path)
component.kind()
component.as_slice()
component.is_root()
component.is_current()
component.is_parent()
component.is_normal()
component.is_windows_drive_prefix()
component.is_windows_unc_prefix()
component.is_windows_prefix()
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
path::join(ref mut zone, base, child)
path::join_many(ref mut zone, parts)
path::current_dir_join(ref mut zone, child)
path::normalize_in(ref mut zone, path)

path.as_slice()
path.as_bytes()
path.to_string(ref mut zone)
path.len()
path.is_empty()
path.contains_nul()
path.is_absolute()
path.is_relative()
path.is_windows_absolute()
path.has_windows_drive_prefix()
path.windows_drive()
path.is_windows_drive_absolute()
path.is_windows_drive_relative()
path.is_windows_unc()
path.windows_unc_prefix()
path.trim_trailing_separators()
path.components()
path.components_with_kinds()
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
path.join(ref mut zone, child)
path.normalize_in(ref mut zone)
path.normalize(ref mut zone)

path_buf.as_bytes()
path_buf.as_path()
path_buf.to_string(ref mut zone)
path_buf.contains_nul()
path_buf.is_absolute()
path_buf.is_relative()
path_buf.is_windows_absolute()
path_buf.has_windows_drive_prefix()
path_buf.windows_drive()
path_buf.is_windows_drive_absolute()
path_buf.is_windows_drive_relative()
path_buf.is_windows_unc()
path_buf.windows_unc_prefix()
path_buf.components()
path_buf.components_with_kinds()
path_buf.file_name()
path_buf.parent()
path_buf.extension()
path_buf.stem()
path_buf.file_stem()
path_buf.join(ref mut zone, child)
path_buf.normalize(ref mut zone)
path_buf.with_file_name(ref mut zone, new_file_name)
path_buf.with_extension(ref mut zone, new_extension)
```

The default separator policy is hosted Linux/POSIX-style `/`. Paths are byte
strings, not validated UTF-8. `std::string::String` is Ari's zone-backed owned
byte string, `std::string::Utf8` is the validated UTF-8 view, and
`PathBytes`/`PathBuf` interpret bytes as lexical paths. Lexical helpers
preserve interior NUL bytes; reject them with `path::contains_nul` before
crossing into C-string or hosted filesystem APIs when the bytes are not known
to be clean.

Single-component helpers return `Option[Slice[u8]]` views into the original
path bytes.
`components(path)` returns a lazy iterator over non-empty borrowed components
and skips leading, repeated, and trailing separators.
`components_with_kinds(path)` yields `Component` values so callers can keep the
lexical meaning that plain components intentionally discard. Absolute paths
start with one `RootDir` component for `/`; `.` yields `CurrentDir`, `..` yields
`ParentDir`, and all other non-empty components are `Normal`. Windows drive
prefixes such as `C:` yield `WindowsDrivePrefix`, and UNC prefixes such as
`//server/share` or the byte-equivalent backslash form yield
`WindowsUncPrefix`. After a Windows prefix, the kinded iterator treats both `/`
and backslash as separators. `as_slice()` keeps returning borrowed bytes from
the original path, and the `is_root`, `is_current`, `is_parent`, `is_normal`,
`is_windows_drive_prefix`, `is_windows_unc_prefix`, and `is_windows_prefix`
helpers are the branch-friendly accessors for the enum kind.
`join_in` and `normalize_in` are the string-returning compatibility helpers;
the owned-path wrappers `join`, `join_many`, `PathBytes::join`,
`PathBytes::normalize`, and the matching `PathBuf` methods return `PathBuf`.
Normalization collapses repeated separators and removes `.` components, but
keeps `..` components because resolving them safely depends on stronger
filesystem and platform policy.
`with_file_name_in` and `with_extension_in` are zone-backed lexical editing
helpers for replacing the final component or final extension without touching
the filesystem. `with_file_name_in` preserves root paths, while
`with_extension_in` leaves paths without a final component unchanged.
`Path` is a readability alias for `PathBytes`. `PathBytes` is the typed
borrowed path-byte view. `PathBuf` is the current owned POSIX path buffer; it is
a distinct struct over an internal `std::string::String`, so APIs can request
an owned path without accepting every byte string. Its bytes live in the zone
used to create it and remain valid until that zone is reset or destroyed. Use
these types when a byte slice or `std::string::OsStr` should be treated as a
path rather than as generic bytes or validated text. When `PathBytes` is
expected, a string literal can be used directly as a borrowed path-byte view.
The `has_*` helpers are allocation-free predicates over `file_name`,
`extension`, `stem`, and `file_stem`; they return `false` when the
corresponding view is absent. `file_stem` is an explicit alias for `stem`.
`starts_with`/`ends_with` and `strip_prefix`/`strip_suffix` are
component-aware: trailing separators are ignored, and a match must end on a
path component boundary. The strip helpers return borrowed views into the
trimmed input path.
`current_dir_join` uses `std::env::current_dir_path()` and returns
`Result[PathBuf, Error]` because the current-directory lookup can fail; the
join step itself remains lexical.
Windows helpers are opt-in classifiers. `is_windows_separator` treats `/` and
byte value `92u8` (backslash) as separators. `has_windows_drive_prefix` and
`windows_drive` recognize ASCII letter drive prefixes such as `C:`.
Drive-absolute paths need a
separator after the drive, while drive-relative paths do not. UNC helpers
require two leading Windows separators plus non-empty server and share
components and return the borrowed `//server/share` or byte-equivalent prefix.
`is_windows_absolute` accepts drive-absolute, UNC, and single-rooted Windows
paths, but not drive-relative paths such as `C:tmp`.
`components_with_kinds` uses the same drive and UNC recognizers to expose
Windows prefix components when source-level tools need to preserve those
prefixes.

Thread helpers live in `std::thread`:

```ari
thread::spawn(entry)
thread::spawn_raw(entry, data)
thread::join(ref mut handle)
thread::join_value(ref mut handle)
thread::try_join(ref mut handle)
thread::try_join_value(ref mut handle)
thread::join_thread(thread)
thread::join_thread_value(thread)
thread::join_compat(thread)
thread::is_finished(thread)
thread::yield_now()
thread::sleep(duration)
thread::id()
thread::id_raw()
thread::current()
thread::is_main()
thread::available_parallelism()
thread::available_parallelism_or(default)
thread::available_parallelism_raw()
thread::is_join_error(status)
thread::builder()
thread::scope(ref mut zone, capacity)
thread::thread_local<T>(ref mut zone)
thread::thread_local_with_capacity<T>(ref mut zone, capacity)

ThreadId::current()
ThreadId::from_raw(raw)
thread_id.as_i64()
thread_id.is_main()
thread_id.is_valid()
thread_id.equals(other)

Builder::new()
builder.name(value)
builder.stack_size(bytes)
builder.configured_name()
builder.configured_stack_size()
builder.spawn(entry)
builder.spawn_raw(entry, data)
Thread::spawn(entry)
Thread::spawn_raw(entry, data)
Thread::current()
Thread::invalid()
thread.id()
thread.id_raw()
thread.is_valid()
thread.is_joinable()
thread.is_finished()
thread.join()

JoinHandle::invalid()
JoinHandle::from_thread(thread)
handle.thread()
handle.thread_id()
handle.id()
handle.is_valid()
handle.is_finished()
handle.detach()
handle.join()
handle.join_value()
handle.try_join()
handle.try_join_value()

ThreadGroupResult::empty()
group.spawned()
group.joined()
group.failed_statuses()
group.status_sum()
group.all_joined()
group.all_success()

ThreadScope::new(ref mut zone, capacity)
scope.capacity()
scope.len()
scope.remaining_capacity()
scope.is_closed()
scope.spawn(entry)
scope.spawn_raw(entry, data)
scope.join_all()
scope.close()

ThreadLocal::new<T>(ref mut zone)
ThreadLocal::with_capacity<T>(ref mut zone, capacity)
thread_local.capacity()
thread_local.is_initialized()
thread_local.set(value)
thread_local.get()
thread_local.get_mut()
thread_local.get_or_init(initializer)
thread_local.take()
thread_local.remove()
```

`spawn(entry)` starts a thread from a plain `fn() -> i64` entry function and
returns a `Result[JoinHandle, Error]`. `JoinHandle` owns the right to join or
detach the native thread; join once, or call `detach()` when no result will be
collected. `thread::join(ref mut handle)` and `handle.join()` return
`Result[i64, JoinError]` so lifecycle mistakes do not collapse into raw
sentinels. `try_join` and `handle.try_join()` return `Ok(None)` when the
thread still appears to be running, `Ok(Some(status))` after joining, or the
same `JoinError` lifecycle cases as `join`. `thread::join_value(ref mut
handle)`, `thread::join_thread_value`, `handle.join_value()`, and
`try_join_value` wrap the joined `i64` in `ThreadResult` for callers that
prefer named success/failure predicates around the process-style status value.
`Thread` is the raw thread-information value kept for inspection and
compatibility, with `join_thread`, `join_compat`, and `join_unchecked`
bridging older call sites.
`spawn_raw(entry, data)` starts a raw-data thread with entry type
`fn(ptr u8) -> i64`. The runtime owns only the start packet; the caller owns
the pointed-to data and must keep it valid until the handle is joined or
detached. `ThreadScope` is a fixed-capacity, zone-backed join owner that can
spawn function-pointer or raw-data workers, then `join_all` them into
`ThreadGroupResult`. Dropping a still-open scope performs best-effort joining,
but explicit `join_all` is the recoverable path.
`id()` returns a `ThreadId`, with main thread `0` and spawned threads positive;
use `id_raw()` for compatibility integer access. `yield_now()` is a host
scheduler hint, not synchronization. `sleep(duration)` forwards to
`std::time::sleep`. `available_parallelism()` now preserves host failure as
`Result[u64, Error]`; use `available_parallelism_or(default)` for a fallback
and `available_parallelism_raw()` only at runtime boundaries. `is_finished` is
advisory and does not replace `join`.
`Builder` records a requested thread name and stack size and delegates through
`spawn_configured`; the LLVM/Linux pthread backend applies stack size with
thread attributes and applies the name as a best-effort host hint.
`ThreadLocal[T]` is an explicit zone-backed handle for per-thread values. It is
fixed-capacity, uses a caller-owned zone, and returns `OptionRef[T]` from
`get_or_init` until Ari can expose a direct borrowed `ref T` from indexed heap
slots. Compiler-level `thread_local` declarations, TLS destructors, captured
thread entries, borrowed scoped threads, and generic `JoinHandle[T]` remain
roadmap work.

Synchronization helpers live in `std::sync`:

```ari
AtomicI64::new(value)
AtomicBool::new(value)
AtomicUsize::new(value)
AtomicPtr::new<T>(ptr)
Mutex::new<T>(value)
RwLock::new<T>(value)
RawMutex::new()
RawRwLock::new()
Once::new()
OnceLock::new<T>()
Condvar::new()
Barrier::new(parties)
Semaphore::new(permits)

sync::channel<T>(ref mut zone)
sync::mpsc_channel<T>(ref mut zone)
sync::bounded_channel<T>(ref mut zone, capacity)
sync::seq_cst()
sync::is_load_order(ordering)
sync::is_store_order(ordering)
sync::is_rmw_order(ordering)
sync::is_compare_exchange_order(success, failure)
sync::load(value)
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
atomic.load_order(ordering)
atomic.store(replacement)
atomic.store_order(replacement, ordering)
atomic.swap(replacement)
atomic.fetch_add(amount)
atomic.compare_exchange(expected, replacement)
atomic.compare_exchange_bool(expected, replacement)
atomic.compare_exchange_order(expected, replacement, success, failure)
atomic.compare_exchange_order_bool(expected, replacement, success, failure)

mutex.try_lock()
mutex.lock()
mutex.try_lock_bool()
mutex.lock_raw()
mutex.unlock()
mutex.is_locked()
mutex_guard.unlock()
mutex_guard.is_active()

rwlock.try_read()
rwlock.read()
rwlock.try_write()
rwlock.write()
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
rwlock_read_guard.unlock()
rwlock_read_guard.is_active()
rwlock_write_guard.unlock()
rwlock_write_guard.is_active()

once.call_once(action)
once.is_completed()

once_lock.set(value)
once_lock.set_bool(value)
once_lock.get()
once_lock.get_mut()
once_lock.get_or_init(initializer)
once_lock.get_or_try_init(initializer)
once_lock.take()

condvar.notify_one()
condvar.notify_all()
condvar.wait(ref mut mutex)
condvar.wait_timeout(ref mut mutex, duration)
condvar.wait_while(ref mut mutex, condition)
wait_timeout.timed_out()

barrier.wait()

semaphore.available_permits()
semaphore.try_acquire()
semaphore.acquire()
semaphore.release()
semaphore.add_permits(permits)
permit.release()
permit.is_active()

channel.sender()
channel.receiver()
channel.capacity()
sender.send(value)
sender.try_send(value)
sender.send_bool(value)
sender.clone()
sender.capacity()
sender.close()
sender.is_closed()
receiver.try_recv()
receiver.try_recv_optional()
receiver.recv()
receiver.recv_timeout(duration)
receiver.recv_optional()
receiver.capacity()
receiver.len()
receiver.close()
receiver.is_closed()
receiver.is_empty()
```

The atomic slice now has `AtomicI64`, `AtomicBool`, `AtomicUsize`, and
`AtomicPtr[T]`. `AtomicI64` lowers to LLVM atomic operations; the other wrappers
compose over it. Default methods are sequentially consistent; explicit-order
methods lower `Relaxed`, `Acquire`, `Release`, `AcqRel`, and `SeqCst` to the
matching LLVM atomic ordering where that operation allows it. Invalid ordering
arguments are programmer errors and assert/panic instead of returning
recoverable errors. `fetch_add` and `swap` return the previous value;
`compare_exchange` now returns `Result[old, current]`; `_bool` compare-exchange
helpers keep the older success/failure-only compatibility shape. For
`AtomicPtr[T]`, the Result payload is the old/current raw pointer value as
`u64`, with casts back to `ptr T` left explicit.

`Mutex[T]` is a source spin/yield value lock built on `RawMutex`. The natural
method API returns `MutexGuard[T]`: `lock` waits for an active guard and
`try_lock` returns `Option[MutexGuard[T]]`. The guard owns both the unlock
operation and access to the payload through `value`, `value_ref`, `value_mut`,
`set`, and `replace`. `RawMutex` keeps the older manual bool/void behavior for
low-level code through `try_lock_bool`, `lock_raw`, and top-level
`sync::try_lock`/`sync::lock`.

`RwLock[T]` follows the same value-protecting rule with `read`, `try_read`,
`write`, and `try_write` returning read/write payload guards. `RawRwLock`
keeps the manual `*_lock`/`*_unlock` helpers. Automatic scope/early-return RAII
cleanup is not promised yet; use `guard.unlock()` or explicit `drop guard`.
`Mutex[T]`, `RwLock[T]`, `RawMutex`, and `RawRwLock` do not poison after
panic/failure; shared-state consistency remains caller-owned unless a future
poison-aware type is introduced. `Once` runs a plain
`fn() -> void` at most once and reports whether the current caller ran it.
`OnceLock[T]` is the
sync-facing one-time value slot: `set` preserves the rejected value through
`Result[(), T]`, `set_bool` is the lossy compatibility form, and
`get_or_try_init` resets the slot to empty when the initializer returns
`Err`. `Condvar`, `Barrier`, `Semaphore`, and bounded MPSC channels provide the
standard shapes now, using spin/yield internals until Ari grows blocking
wait/wake runtime support. `Condvar::wait_timeout` is a monotonic deadline
spin/yield wait, not an OS sleeping condvar. `Semaphore::try_acquire` returns
`Option[SemaphorePermit]`, `acquire` yields until a permit is available, and an
active permit releases on explicit `release` or `drop permit`. Channels are
bounded MPSC queues: `send`/`try_send`, `recv`/`try_recv`, and `recv_timeout`
return Result errors, while `_bool` and `_optional` helpers intentionally
discard detail. `Sender[T]::clone` creates another handle to the same bounded
channel state and increments the active-sender count. Closing one sender keeps
the channel open for receivers while another cloned sender remains open; the
receiver observes the channel as closed after the last sender is closed or the
receiver closes the channel explicitly.

Shared-ownership handles live in `std::rc` as `Rc`, `Arc`, and `Weak`.
`LazyLock`, poison-aware lock variants, futex-backed blocking locks,
unbounded channel policy, send/share trait checks, compiler-owned
`thread_local` declarations, and target-native relaxed ordering remain future
concurrency work.

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

Filesystem path-like public arguments are borrowed byte slices. String
literals therefore work directly for paths, mode strings, temporary prefixes,
and `write_string` text, for example `fs::read_dir_entries(ref mut zone, ".")`
or `fs::write_string("out.txt", "ok")`. Owned `String` values can be passed as
`value`, `ref value`, or the explicit `value.as_slice()` view, and reusable
literal path variables keep their literal length when initialized immutably.

```ari
fs::exists(path)
fs::can_read(path)
fs::can_write(path)
fs::can_execute(path)
fs::permissions(path)
fs::metadata(path)
fs::metadata_raw(path)
fs::metadata(path)
fs::try_metadata(path)
fs::symlink_metadata(path)
fs::symlink_metadata_raw(path)
fs::symlink_metadata(path)
fs::try_symlink_metadata(path)
fs::file_type_raw(path)
fs::file_type(path)
fs::try_file_type(path)
fs::is_file(path)
fs::is_dir(path)
fs::is_symlink(path)
fs::is_other(path)
fs::mode(path)
fs::mode_raw(path)
fs::mode(path)
fs::try_mode(path)
fs::set_mode(path, mode)
fs::set_permissions(path, permissions)
fs::canonicalize(ref mut zone, path)
fs::canonicalize_optional(ref mut zone, path)
fs::canonicalize(ref mut zone, path)
fs::canonicalize_unchecked(ref mut zone, path)
fs::try_canonicalize(ref mut zone, path)
fs::remove(path)
fs::remove_file(path)
fs::remove_bool(path)
fs::remove_file_bool(path)
fs::remove_unchecked(path)
fs::rename(source, target)
fs::rename_bool(source, target)
fs::rename_unchecked(source, target)
fs::hard_link(existing, link_path)
fs::hard_link_bool(existing, link_path)
fs::hard_link_raw(existing, link_path)
fs::hard_link_unchecked(existing, link_path)
fs::symbolic_link(target, link_path)
fs::symbolic_link_bool(target, link_path)
fs::symbolic_link_raw(target, link_path)
fs::symbolic_link_unchecked(target, link_path)
fs::read_link(ref mut zone, path)
fs::read_link_optional(ref mut zone, path)
fs::read_link(ref mut zone, path)
fs::read_link_unchecked(ref mut zone, path)
fs::try_read_link(ref mut zone, path)
fs::ensure_file(path)
fs::create_dir(path)
fs::create_dir_bool(path)
fs::create_dir_raw(path)
fs::create_dir(path)
fs::create_dir_unchecked(path)
fs::ensure_dir(path)
fs::create_dir_all(path)
fs::create_dir_all_bool(path)
fs::create_dir_all_raw(path)
fs::create_dir_all_unchecked(path)
fs::ensure_dir_all(path)
fs::remove_dir(path)
fs::remove_dir_bool(path)
fs::remove_dir_raw(path)
fs::remove_dir(path)
fs::remove_dir_unchecked(path)
fs::remove_dir_all(path)
fs::remove_dir_all_bool(path)
fs::open_dir(path)
fs::open_dir_raw(path)
fs::open_dir(path)
fs::try_open_dir(path)
fs::read_dir(ref mut zone, path)
fs::read_dir_optional(ref mut zone, path)
fs::read_dir(ref mut zone, path)
fs::read_dir_unchecked(ref mut zone, path)
fs::try_read_dir(ref mut zone, path)
fs::read_dir_names(ref mut zone, path)
fs::read_dir_names_optional(ref mut zone, path)
fs::read_dir_names_unchecked(ref mut zone, path)
fs::try_read_dir_names(ref mut zone, path)
fs::read_dir_entries(ref mut zone, path)
fs::read_dir_entries_optional(ref mut zone, path)
fs::read_dir_entries(ref mut zone, path)
fs::read_dir_entries_unchecked(ref mut zone, path)
fs::try_read_dir_entries(ref mut zone, path)
fs::read_dir_next(ref mut zone, dir)
fs::close_dir(dir)
fs::open(path, mode)
fs::open_optional(path, mode)
fs::open_unchecked(path, mode)
fs::create(path)
fs::create_optional(path)
fs::create_unchecked(path)
fs::remove_raw(path)
fs::remove(path)
fs::rename_raw(source, target)
fs::rename(source, target)
fs::open_read(path)
fs::open_write(path)
fs::open_append(path)
fs::open_read_optional(path)
fs::open_write_optional(path)
fs::open_append_optional(path)
fs::open_read_unchecked(path)
fs::open_write_unchecked(path)
fs::open_append_unchecked(path)
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
fs::read_bytes(ref mut zone, path)
fs::read_optional(ref mut zone, path)
fs::read_or_default(ref mut zone, path)
fs::read(ref mut zone, path)
fs::read_unchecked(ref mut zone, path)
fs::try_read(ref mut zone, path)
fs::write(path, values)
fs::write_bool(path, values)
fs::write_raw(path, values)
fs::write(path, values)
fs::write_string(path, text)
fs::try_write(path, values)
fs::append(path, values)
fs::append_bool(path, values)
fs::append_raw(path, values)
fs::append(path, values)
fs::try_append(path, values)
fs::truncate(path)
fs::copy(source, target)
fs::copy_bool(source, target)
fs::copy_raw(source, target)
fs::copy(source, target)
fs::try_copy(source, target)
fs::read_to_string(ref mut zone, path)
fs::read_to_string_optional(ref mut zone, path)
fs::read_to_string_or_default(ref mut zone, path)
fs::read_to_string(ref mut zone, path)
fs::read_to_string_unchecked(ref mut zone, path)
fs::try_read_to_string(ref mut zone, path)

fs::open_raw(path, mode)
fs::open(path, mode)
fs::create_raw(path)
fs::create(path)
fs::open_options()
OpenOptions::new()
options.read(enabled)
options.write(enabled)
options.append(enabled)
options.truncate(enabled)
options.create(enabled)
options.create_new(enabled)
options.open(path)
options.open_optional(path)
options.open_unchecked(path)
options.open_raw(path)
options.open(path)
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

Use `open(path, mode)` for ordinary fallible open operations; it returns
`Result[File, Error]`. `open_optional(path, mode)` and `try_open(path, mode)`
discard failures into `None`, while `open_unchecked(path, mode)` keeps the old
invalid-handle sentinel convention. Supported modes are `"r"` for read, `"w"`
for create/truncate write, `"a"` for create/append write, `"rw"` for existing
read/write, `"r+"` as a familiar alias for `"rw"`, `"w+"` for create/truncate
read/write, and `"a+"` for read/append. `open_read`, `open_write`, and
`open_append` are Result-returning wrappers over those mode strings; their
`_optional`/`try_open_*` partners discard reasons, and their `_unchecked`
partners preserve the invalid-handle shape. Use
`open_raw(path, mode)` or `create_raw(path)` only
for compatibility callers that still need `Result[File, i64]`.
Use `read(ref mut zone, path)` or `read_to_string(ref mut zone, path)` when a
missing file should return `Error(NotFound)`.
`read_optional`/`read_to_string_optional` and the older `try_*` helpers discard
failure reasons, `_or_default` keeps the old empty-string fallback, and
`_unchecked` asserts on failure.
Use `OpenOptions::new()` or `fs::open_options()` when named policy is clearer:
`read`, `write`, `append`, `truncate`, `create`, and `create_new` each return a
new options value, `options.try_open(path)` returns `Option[File]`, and
`options.open(path)` returns `Result[File, Error]`. `options.open_optional`
and `options.try_open(path)` discard failures into `None`; `options.open_unchecked`
keeps the old invalid-handle sentinel.
`options.open_raw(path)` keeps the raw integer compatibility `Result`
shape.
`create_new(true)` is exclusive creation; `append(true).truncate(true)` and
create/truncate without write or append are rejected as invalid option sets.
`create(path)` is the natural Result-returning create/truncate helper over
`"w"` mode. `create_optional(path)` and `try_create(path)` discard the reason,
while `create_unchecked(path)` keeps the invalid-handle compatibility shape.
`File` implements `std::io::Reader`, `std::io::Writer`, and
`std::io::Seek`, so a handle from `try_open` can be passed to generic IO
helpers such as `io::read_to_string<std::fs::File>`, `io::copy`,
`io::try_copy`, `io::write_all`, `io::flush`, or a caller's own
`S: io::Seek` helper. `File` writes are direct descriptor writes; `flush`
returns `Ok(())` while the handle is open and `Err(InvalidInput)` after close,
rather than draining a separate file buffer. `position(file)`/`file.position()`
returns the current byte offset or `-1` for invalid or unseekable handles.
`seek(file, position)` and
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
unstatable paths. `metadata(path)` returns `Result[Metadata, Error]` and
preserves errno-derived failure kinds. These helpers follow symbolic links.
`metadata_raw(path)` keeps the raw compatibility bridge.
`try_symlink_metadata(path)` and `symlink_metadata(path)` use no-follow
metadata lookup, so a symbolic link reports `FileKind::Symlink` and its stored
target byte length instead of the target file's metadata. The `Permissions`
field is still the same access-style snapshot as `permissions(path)`; portable
symlink permission-bit policy is not part of this slice.
`symlink_metadata(path)` and `symlink_metadata_raw(path)` are the
direct `Error` and raw compatibility versions of that no-follow lookup.
`Metadata::len` reports host byte length, `Metadata::file_type` returns
`FileKind` (`Regular`, `Directory`, `Symlink`, or `Other`), and
`Metadata::permissions` carries the access-style permission snapshot.
`Metadata::accessed`, `Metadata::modified`, and `Metadata::changed` return
`std::time::SystemTime` values for access time, modification time, and POSIX
status-change time respectively. `changed` is not a portable creation time.
`try_file_type(path)` returns just `Option[FileKind]` without building the full
metadata/permission snapshot.
`file_type(path)` returns `Result[FileKind, Error]` when the caller
needs a precise failure reason, and `file_type_raw(path)` keeps raw
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
permission bits, and `mode(path)` returns `Result[i64, Error]`. Use
`set_mode(path, mode)` for direct chmod-style updates, or
`set_permissions(path, permissions)` when a `Permissions` value is clearer at
the call site. `Permissions::to_mode`
maps the three booleans to user/group/other bits, so `read_only()` maps to
`0444` and `all()` maps to `0777`.
`mode_raw(path)` is the raw compatibility form.
`canonicalize(ref mut zone, path)` returns `Result[String, Error]` for host
`realpath` resolution. The returned string is absolute, owned by the provided
zone, and follows the host symlink policy. `canonicalize_optional` and
`try_canonicalize` discard the reason, and `canonicalize_unchecked` asserts on
failure.
`read_link(ref mut zone, path)` returns `Result[String, Error]` containing the
stored target bytes of a symbolic link. Use `read_link` when code needs the
link text itself; use `canonicalize` when code wants the host-resolved absolute
path. `read_link_optional` and `try_read_link` discard the reason, and
`read_link_unchecked` asserts on failure.
`read_byte` returns an `i64` byte value, `-1` at EOF, or a value below `-1`
when the host read call fails. The `try_read_byte` compatibility wrappers
collapse both non-byte states to `None`. Result-returning whole-file reads and
copies preserve open, read, write, and close failures as `Error`.
`write_byte` returns `Result[(), Error]`. `write_bytes` writes a `Slice[u8]`
and returns `Result[i64, Error]` with the byte count.
`write(path, values)` truncates or creates a small byte file, writes the whole
`Slice[u8]`, and returns `Ok(byte_count)` when the write and close succeed.
`write_string(path, text)` writes borrowed byte text such as a literal or an
owned `String`'s `as_slice()` view and returns `Result[(), Error]` after
discarding the byte count.
`append(path, values)` creates if needed and appends the whole slice with the
same `Result[i64, Error]` policy. `write_raw` and
`append_raw` preserve the raw
`Result[i64, i64]` compatibility shape. `try_write(path, values)` and
`try_append(path, values)` are `Option[i64]` wrappers, while
`write_bool(path, values)` and `append_bool(path, values)` are boolean
compatibility wrappers.
`read_to_string(ref mut zone, path)` and its short alias `read(ref mut zone,
path)` return `Result[String, Error]`, using the caller's zone for the
byte-oriented `std::string::String`. `read_bytes(ref mut zone, path)` copies
those bytes into `Vec[u8]`. `try_read_to_string`/`try_read` and
`read_to_string_optional`/`read_optional` collapse failures to `None`; the
`_or_default` helpers keep the old empty-string fallback. `truncate(path)`
creates or empties a file. `try_copy(source, target)` streams bytes from the
source handle into the target opened with truncating semantics and returns
`Some(byte_count)` on success or `None` on open/write/close failure.
`copy(source, target)` is the natural `Result[i64, Error]` copy helper,
`copy_raw(source, target)` keeps the raw `Result[i64, i64]` bridge, and
`copy_bool(source, target)` is the boolean compatibility wrapper over
`try_copy`. `rename(source, target)` moves or renames one path according to the
host runtime's current behavior and returns `Result[(), Error]`;
`rename_bool` is the compatibility
boolean shape, and `rename_unchecked` is the direct runtime hook.
`remove(path)`/`remove_file(path)`, `create_dir(path)`, `remove_dir(path)`,
`create_dir_all(path)`, and `remove_dir_all(path)` are natural `Result[(),
Error]` helpers. Their `*_bool` names keep old boolean compatibility, and their `*_raw`
or `*_unchecked` forms are only for compatibility/runtime-hook code.
`ensure_dir(path)` and `ensure_dir_all(path)` stay boolean setup helpers.
`hard_link(existing, link_path)` and `symbolic_link(target, link_path)` are
also Result-first; their `_bool`, `_unchecked`, and `_raw` variants are
the non-default compatibility forms.
`open_dir(path)` returns `Result[Dir, Error]` and
`open_dir_raw(path)` keeps raw compatibility. `try_open_dir(path)`
returns `Option[Dir]`, `dir.next(ref mut zone)` returns the next entry name
while skipping `"."` and `".."`, and `dir.close()` closes the handle.
`read_dir(ref mut zone, path)` opens, collects `DirEntry` values, closes, and
returns `Result[Vec[DirEntry], Error]`. `read_dir_optional` discards the reason,
and `read_dir_unchecked` asserts on failure. Use `read_dir_names(ref mut zone,
path)` for the old `Vec[String]` name-list shape; `try_read_dir`,
`try_read_dir_names`, `read_dir_names_optional`, and `read_dir_names_unchecked`
are the absence-only and unchecked name-list compatibility helpers. Use
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
net::UdpRecvFrom
net::UnixListener
net::UnixStream
net::Shutdown

net::ipv4(a, b, c, d)
net::ipv6(s0, s1, s2, s3, s4, s5, s6, s7)
net::socket_addr(ip, port)
net::localhost(port)
net::lookup_v4(host, port)
net::lookup_v4_optional(host, port)
net::try_lookup_v4(host, port)
net::lookup_v4_raw(host, port)
net::lookup_v6(host, port)
net::lookup_v6_optional(host, port)
net::try_lookup_v6(host, port)
net::lookup_v6_raw(host, port)
net::service_port(name)
net::service_port_optional(name)
net::service_port_bytes(bytes)
net::service_port_bytes_optional(bytes)
net::is_timeout(ref error)
net::is_timed_out(ref error)
net::is_would_block(ref error)
net::is_interrupted(ref error)
net::is_connection_refused(ref error)
net::is_retryable(ref error)
net::resolve(endpoint)
net::resolve_optional(endpoint)
net::try_resolve(endpoint)
net::resolve_raw(endpoint)
net::resolve_all(zone, host, port)
net::resolve_service(zone, host, service)
net::to_socket_addrs(zone, endpoint)
net::to_socket_addrs_service(zone, host, service)
net::listen(addr)
net::tcp_listen(addr)
net::tcp_listen_v6(addr)
net::connect(addr)
net::tcp_connect(addr)
net::tcp_connect_v6(addr)
net::connect_host(endpoint)
net::tcp_connect_host(endpoint)
net::udp_bind(addr)
net::udp_bind_v6(addr)
net::unix_listen(path)
net::unix_connect(path)

ToSocketAddrs::to_socket_addrs(zone)

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
TcpListener::bind_optional(addr)
TcpListener::try_bind(addr)
TcpListener::bind_raw(addr)
listener.descriptor()
listener.is_open()
listener.local_port()
listener.local_addr()
listener.local_addr_v6()
listener.is_nonblocking()
listener.set_nonblocking(enabled)
listener.close_on_exec()
listener.set_close_on_exec(enabled)
listener.reuse_addr()
listener.set_reuse_addr(enabled)
listener.reuse_port()
listener.set_reuse_port(enabled)
listener.set_accept_timeout(timeout)
listener.set_accept_timeout_millis(millis)
listener.accept()
listener.accept_optional()
listener.try_accept()
listener.accept_raw()
listener.close()

TcpStream::connect(addr)
TcpStream::connect_optional(addr)
TcpStream::try_connect(addr)
TcpStream::connect_raw(addr)
stream.descriptor()
stream.is_open()
stream.local_addr()
stream.local_addr_v6()
stream.peer_addr()
stream.peer_addr_v6()
stream.is_nonblocking()
stream.set_nonblocking(enabled)
stream.close_on_exec()
stream.set_close_on_exec(enabled)
stream.nodelay()
stream.set_nodelay(enabled)
stream.keepalive()
stream.set_keepalive(enabled)
stream.linger_seconds()
stream.set_linger_seconds(value)
stream.disable_linger()
stream.send_buffer_size()
stream.set_send_buffer_size(value)
stream.recv_buffer_size()
stream.set_recv_buffer_size(value)
stream.ttl()
stream.set_ttl(value)
stream.hop_limit()
stream.set_hop_limit(value)
stream.set_read_timeout(timeout)
stream.set_read_timeout_millis(millis)
stream.set_write_timeout(timeout)
stream.set_write_timeout_millis(millis)
stream.shutdown(mode)
stream.try_read_byte()
stream.read(output)
stream.read_exact(output, len)
stream.read_exact_slice(output)
stream.read_to_end(zone)
stream.read_to_string(zone)
stream.write(values)
stream.write_all(values)
stream.flush()
stream.close()

UdpSocket::bind(addr)
UdpSocket::bind_optional(addr)
UdpSocket::try_bind(addr)
UdpSocket::bind_raw(addr)
socket.descriptor()
socket.is_open()
socket.local_port()
socket.local_addr()
socket.local_addr_v6()
socket.is_nonblocking()
socket.set_nonblocking(enabled)
socket.close_on_exec()
socket.set_close_on_exec(enabled)
socket.reuse_addr()
socket.set_reuse_addr(enabled)
socket.reuse_port()
socket.set_reuse_port(enabled)
socket.broadcast()
socket.set_broadcast(enabled)
socket.send_buffer_size()
socket.set_send_buffer_size(value)
socket.recv_buffer_size()
socket.set_recv_buffer_size(value)
socket.ttl()
socket.set_ttl(value)
socket.hop_limit()
socket.set_hop_limit(value)
socket.set_read_timeout(timeout)
socket.set_read_timeout_millis(millis)
socket.set_write_timeout(timeout)
socket.set_write_timeout_millis(millis)
socket.connect(addr)
socket.send_to(values, addr)
socket.send(values)
socket.recv(output)
socket.recv_from(output)
socket.peek_from(output)
socket.send_byte_to(value, addr)
socket.recv_byte()
socket.try_recv_byte()
socket.close()

recv.len()
recv.addr()
recv.source()

UnixListener::bind(path)
UnixListener::bind_optional(path)
UnixListener::try_bind(path)
UnixListener::bind_raw(path)
listener.descriptor()
listener.is_open()
listener.is_nonblocking()
listener.set_nonblocking(enabled)
listener.close_on_exec()
listener.set_close_on_exec(enabled)
listener.accept()
listener.accept_optional()
listener.try_accept()
listener.accept_raw()
listener.close()

UnixStream::connect(path)
UnixStream::connect_optional(path)
UnixStream::try_connect(path)
UnixStream::connect_raw(path)
stream.descriptor()
stream.peer_credentials()
stream.peer_credentials_optional()
stream.is_open()
stream.is_nonblocking()
stream.set_nonblocking(enabled)
stream.close_on_exec()
stream.set_close_on_exec(enabled)
stream.set_read_timeout(timeout)
stream.set_read_timeout_millis(millis)
stream.set_write_timeout(timeout)
stream.set_write_timeout_millis(millis)
stream.shutdown(mode)
stream.try_read_byte()
stream.read(output)
stream.read_exact(output, len)
stream.read_exact_slice(output)
stream.read_to_end(zone)
stream.read_to_string(zone)
stream.write(values)
stream.write_all(values)
stream.flush()
stream.close()

credentials.pid()
credentials.uid()
credentials.gid()
```

Address values are deterministic source structs. Use `octet`/`segment` for
known-good indexes and `try_octet`/`try_segment` when validating parsed input.
`lookup_v4` resolves one IPv4 address through the hosted `getaddrinfo` path and
returns `Result[SocketAddr, Error]`. `lookup_v4_optional` and `try_lookup_v4`
discard the reason intentionally. `lookup_v6` is the IPv6 sibling and follows
the same Result/optional/raw naming policy. `resolve("host:port")`,
`resolve("[::1]:port")`, and `resolve_raw` parse common IPv4/host-name
and bracketed IPv6 endpoint spellings, rejecting malformed endpoints as
`InvalidInput` before calling the resolver.
`resolve_optional` and `try_resolve` keep the old absence-only shape.
`service_port(name)` and `service_port_bytes(bytes)` map the documented
well-known service table to numeric ports, returning `Error(InvalidInput)` for
unknown names. `_optional` forms intentionally collapse unknown service names
to absence.
`resolve_all(zone, host, port)`, `resolve_service(zone, host, service)`,
`to_socket_addrs(zone, endpoint)`, and
`to_socket_addrs_service(zone, host, service)` return zone-backed
`Vec[SocketAddr]` values; the hosted implementation currently collects the
first IPv4 and first IPv6 address exposed by the resolver.
`string` implements the matching `ToSocketAddrs` trait shape.
Matching `*_raw` helpers are compatibility-only bridges for
low-level callers that still need raw integer errors.
`net::listen`/`net::connect` are TCP-focused module-level `Result` helpers;
use `tcp_listen`/`tcp_connect`, explicit IPv6 `tcp_listen_v6`/
`tcp_connect_v6`, `connect_host`/`tcp_connect_host`, `udp_bind`,
`udp_bind_v6`, `unix_listen`, `unix_connect`, `unix_datagram`, and
`unix_datagram_unbound` when the socket family should be explicit at the call
site.
`TcpListener`, `TcpStream`, `UdpSocket`, `UnixListener`, `UnixStream`, and `UnixDatagram` are
owned descriptor-backed handles. They support hosted IPv4/IPv6 TCP
bind/connect/accept, IPv4/IPv6 UDP bind/buffer datagrams/source-address receive,
connected UDP send/receive, Unix stream
bind/connect/accept, pathname Unix datagram bind/unbound/connect/send/receive,
local bound-port and local socket-address lookup,
borrowed descriptor views, explicit close, nonblocking flags,
reuse-address/reuse-port helpers, close-on-exec helpers, TCP nodelay,
keepalive, and linger helpers, UDP broadcast, IPv4 multicast loop/TTL and
membership helpers, send/receive buffer-size helpers, IPv4 TTL and IPv6 hop-limit helpers,
`std::time::Duration` timeout setters with raw millisecond compatibility
helpers, and stream shutdown. TCP and
Unix streams adapt to `std::io::Reader`/`Writer` and provide inherent
`read`, `write`, `read_exact(output, len)`, `read_exact_slice(output)`,
`read_to_end(zone)`, `read_to_string(zone)`, and `write_all(values)` helpers for natural stream
method syntax. Connected Unix streams expose `peer_credentials()` as
`Result[UnixPeerCredentials, Error]`, with `pid()`, `uid()`, and `gid()`
accessors for the snapshot returned by the hosted Unix socket. Natural
bind/connect/accept/resolve names return
`Result[..., Error]`; matching `_optional` and `try_*` helpers keep
compatibility call sites concise when they intentionally discard the reason.
Host-port `connect_host` first resolves through `resolve`, then delegates to
`TcpStream::connect`. `*_ready` helpers expose single-descriptor readiness
over `std::os::poll_read`/`poll_write`; they are advisory and callers must
still handle the actual operation result. `linger_seconds` reads TCP
`SO_LINGER` as `Option[i64]`, where `None` means disabled; `set_linger_seconds`
enables it and `disable_linger` clears it. `ttl` controls IPv4 `IP_TTL` and
`hop_limit` controls IPv6 `IPV6_UNICAST_HOPS`; setters accept `1..=255` and
return `InvalidInput` outside that range. UDP IPv4 multicast exposes
loopback, TTL, join, and leave helpers; multicast TTL accepts `0..=255`.
Socket error predicates mirror the shared error vocabulary in names that are
local to the networking module: `is_timeout`/`is_timed_out`, `is_would_block`,
`is_interrupted`, `is_connection_refused`, and `is_retryable`. Timeout,
would-block, interrupted, and in-progress errors are retryable by stdlib
policy; connection refused is not retryable by default.
Full `getaddrinfo` iteration, host service-database lookup, IPv6 multicast
policy, multi-descriptor poll/event loops, and TLS packaging decisions remain
roadmap work.

## IO And Input

`std::io` exposes low-level process IO hooks and a small trait surface for
byte readers/writers, while `std::input` keeps stdin-oriented helper names:

```ari
io::Reader
io::Writer
io::Seek
io::ReadByte
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
io::pipe() -> Result[io::Pipe, io::Error]
io::pipe_optional() -> Option[io::Pipe]
pipe.read_end()
pipe.write_end()
pipe.take_reader()
pipe.take_writer()
pipe.close() -> Result[(), io::Error]
pipe.close_bool() -> bool
pipe_reader.as_fd()
pipe_reader.is_open()
pipe_reader.close() -> Result[(), io::Error]
pipe_reader.close_bool() -> bool
pipe_writer.as_fd()
pipe_writer.is_open()
pipe_writer.close_on_exec()
pipe_writer.close_on_exec_optional()
pipe_writer.set_close_on_exec(enabled)
pipe_writer.set_close_on_exec_bool(enabled)
pipe_writer.close() -> Result[(), io::Error]
pipe_writer.close_bool() -> bool
io::cursor(values)
io::buf_reader[R: Reader](inner, buffer)
io::buf_writer[W: Writer](inner, buffer)
reader.read_one()
reader.read(output)
reader.read_line(zone)
reader.read_to_string(zone)
writer.write(values)
writer.write_all(values)
io::read_one[R: Reader](reader: ref mut R)
io::read[R: Reader](reader: ref mut R, output)
io::read_exact[R: Reader](reader: ref mut R, output, len)
io::read_exact_unchecked[R: Reader](reader: ref mut R, output, len)
io::read_all[R: Reader](zone: ref mut Zone, reader: ref mut R)
io::read_line_from[R: Reader](zone: ref mut Zone, reader: ref mut R)
io::read_to_string[R: Reader](zone: ref mut Zone, reader: ref mut R)
io::read_to_string_unchecked[R: Reader](zone: ref mut Zone, reader: ref mut R)
io::copy[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W)
io::try_copy[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W)
io::copy_unchecked[R: Reader, W: Writer](reader: ref mut R, writer: ref mut W)
io::write[W: Writer](writer: ref mut W, values)
io::write_all[W: Writer](writer: ref mut W, values)
io::write_all_unchecked[W: Writer](writer: ref mut W, values)
io::flush[W: Writer](writer: ref mut W)
io::flush_unchecked[W: Writer](writer: ref mut W)
io::print(text: Slice[u8])
io::print_text(text: Slice[u8])
io::print_string(text: ref String)
io::println(text: Slice[u8])
io::println_text(text: Slice[u8])
io::println_string(text: ref String)
io::eprint(text: Slice[u8])
io::eprint_text(text: Slice[u8])
io::eprint_string(text: ref String)
io::eprintln(text: Slice[u8])
io::eprintln_text(text: Slice[u8])
io::eprintln_string(text: ref String)
io::write_i64(value)
io::write_u64(value)
io::write_bool(value)
io::write_byte(value)
io::write_bytes(values)
io::newline()
io::read_byte()
io::read_line(ref mut zone)
io::read_line_text()
io::read_line_owned(ref mut zone)

input::read_byte()
input::try_read_byte()
input::line(ref mut zone)
input::line_text()
input::owned_line(ref mut zone)
```

`read_byte` returns an `i64` byte value or `-1` at EOF; OS-backed adapters use
values below `-1` for host read failures. `input::try_read_byte()` wraps the
stdin shape as `Option[u8]`. `write_bytes` writes every byte in a `Slice[u8]`
and returns the byte count attempted. Natural line input APIs require a zone
and return `std::string::String`. `io::read_line_text()` and
`input::line_text()` expose the reusable borrowed runtime buffer for raw
compatibility code.

`io::Cursor` implements `Reader` and `Seek` over a borrowed `Slice[u8]`.
`io::read_one(ref mut reader)` converts one low-level byte read into
`ReadByteValue(byte)`, `ReadByteEof`, or `ReadByteError(error)` when an adapter
can detect a real failure before reaching the raw hook. The `ReadByte` methods
`is_byte`, `is_eof`, `is_error`, `byte`, and `error` give compact branching
without inspecting the raw `-1` sentinel. `io::read(ref mut reader, output)`
fills up to `output.len` bytes and returns `Ok(count)`, with EOF reported as
`Ok(0)` or a short positive count rather than an error.
`io::read_exact(ref mut reader, output, len)` copies exactly `len` bytes or
returns `Err(Error(UnexpectedEof))`; `io::read_exact_unchecked` is the bool
compatibility wrapper. A negative `len` returns `Err(Error(InvalidInput))`.
`io::read_all(ref mut zone, ref mut reader)` collects the remaining bytes from
any `Reader` into a zone-backed `Vec[u8]`, stopping at the same EOF sentinel as
`read_exact`.
`io::read_line_from(ref mut zone, ref mut reader)` reads through the first
newline or EOF and returns a zone-backed `String`. EOF before any bytes is
`Ok(empty)`. `Stdin`, `Cursor`, `BufReader`, and `PipeReader` expose matching
`read_line(zone)` and `read_to_string(zone)` method forms.
`io::read_to_string(ref mut zone, ref mut reader)` collects the remaining bytes
directly into an owned `std::string::String` inside `Result`.
`io::read_to_string_unchecked` is the compatibility helper for old call sites
that intentionally discard that wrapper. The read is byte-oriented like the
rest of `std::io`; use `String::try_utf8()` when a validated UTF-8 view is
needed.
`io::copy(ref mut reader, ref mut writer)` streams bytes from any `Reader` to
any `Writer`, flushes at EOF, and returns `Ok(byte_count)` on
complete success. Failed byte writes become `Err(Error(BrokenPipe))`; failed
final flushes return the writer's flush error. `io::try_copy` is the `Option[i64]`
compatibility wrapper and `io::copy_unchecked` is the bool wrapper when the
byte count is not needed.
`Writer::write(ref mut writer, values)` and
`io::write(ref mut writer, values)` return the accepted byte count.
`Writer::write_all(ref mut writer, values)` and
`io::write_all(ref mut writer, values)` return `Ok(())` after every byte is
accepted. These APIs return `Err(Error(BrokenPipe))` on the first failed byte
write. `Stdout`, `Stderr`, `PipeWriter`, `BufWriter`, `std::fs::File`,
`TcpStream`, and `UnixStream` implement the natural trait methods, so generic
`W: io::Writer` code can call `writer.write(values)` directly.
`io::write_all_unchecked` is the bool wrapper.
`io::flush(ref mut writer)` returns `Ok(())` for a successful flush and
the writer's `Error` for a failed flush; `io::flush_unchecked` is the bool
wrapper.
`io::print`, `io::println`, `io::eprint`, and `io::eprintln` are
Result-returning plain-text helpers for CLI messages. They accept borrowed
bytes, so string literals work directly. They do not parse format placeholders.
Use the `_string` variants for owned `String` values. The older `_text`
spellings remain compatibility aliases for borrowed bytes. Root `print`/
`println` and `std::print`/`std::println` are still compiler formatting calls;
the `io::` names are ordinary stdlib functions for direct byte output.
`io::Stdout` and `io::Stderr` implement `Writer` over the current process
stream hooks, with `flush` currently succeeding as a no-op. `io::BufReader`
and `io::BufWriter` wrap any `Reader` or `Writer` with either an explicit
caller-provided `Slice[u8]` buffer or a zone-backed buffer allocated by
`io::buf_reader_in`, `io::buf_writer_in`, `BufReader::with_capacity`, and
`BufWriter::with_capacity`. Borrowed buffers must stay alive while the wrapper
is used; zone-backed buffers stay valid until that zone is reset or destroyed.
The zone-backed constructors return `Err(InvalidInput)` when `capacity <= 0`.
`io::BufWriter` flushes when the buffer fills, on explicit `flush()`,
and as a best-effort cleanup when a still-buffered writer is dropped. Drop
cleanup cannot report errors, so callers that need to observe write failures
should still call `flush()` explicitly before dropping the writer. `io::pipe()`
returns `Result[io::Pipe, Error]` and wraps
`std::os::Pipe` in `PipeReader` and `PipeWriter` adapters; use
`io::pipe_optional()` only when the caller intentionally discards creation
errors. The reader implements `Reader`, the writer implements `Writer`, and
both expose explicit Result-returning close helpers plus `_bool`
compatibility wrappers. `PipeWriter` also exposes close-on-exec query/set
helpers over the underlying owned descriptor; process setup-error pipes use
that flag so a successful `exec` closes the reporting channel automatically.
`std::fs::File` implements `Reader`, `Writer`, and `Seek`.

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

`std::region` is the preferred user-facing bulk allocation lifetime facade:

```ari
region::create(capacity)
region::default_capacity()
region::allocator(ref mut region)
region::alloc(ref mut region, bytes, align)
region::alloc_array<T>(ref mut region, count)
region::new<T>(ref mut region, value)
region::promote<T>(ref mut target, source)
region::capacity(ref mut region)
region::used(ref mut region)
region::remaining(ref mut region)
region::can_alloc(ref mut region, bytes)
region::can_alloc_array<T>(ref mut region, count)
region::reset(ref mut region)
region::destroy(region)
```

Use `Region` when code chooses the owner of a bulk lifetime. The current
implementation aliases `Region` to the existing `Zone` runtime, so older zone
APIs remain source compatible. New user-facing docs should prefer `Region`
for lifetime ownership and `Allocator` for follow-up growth from an existing
region-backed handle.

`std::allocator` is the public allocation-capability view used by handles that
need to grow from existing region-backed storage:

```ari
allocator::from_zone(ref mut zone)
allocator::from_data(data)
allocator::from_zone_metadata(metadata)
allocator::of(ref value)
allocator::alloc(ref allocator, bytes, align)
allocator::alloc_array<T>(ref allocator, count)
allocator::capacity(ref allocator)
allocator::used(ref allocator)
allocator::remaining(ref allocator)
allocator::can_alloc(ref allocator, bytes)
allocator::can_alloc_array<T>(ref allocator, count)

allocator.alloc(bytes, align)
allocator.alloc_array<T>(count)
allocator.capacity()
allocator.used()
allocator.remaining()
allocator.can_alloc(bytes)
allocator.can_alloc_array<T>(count)
allocator.equals(ref other)
```

Use `Region` for region creation and lifecycle. Use `Allocator` when a value
already owns region-backed storage and follow-up work needs the same allocation
capability. `from_zone_metadata` and `allocator.metadata()` are compatibility
bridges for existing low-level code; new public APIs should prefer
`from_zone`, `from_data`, or `of`.

`std::zone` exposes the low-level region runtime and compatibility capability:

```ari
zone::create(capacity)
zone::default_capacity()
zone::alloc(ref mut zone, bytes, align)
zone::capacity(ref mut zone)
zone::used(ref mut zone)
zone::remaining(ref mut zone)
zone::can_alloc(ref mut zone, bytes)
zone::can_alloc_array<T>(ref mut zone, count)
zone::alloc<T>(ref mut zone)
zone::alloc_array<T>(ref mut zone, count)
zone::new<T>(ref mut zone, value)
zone::promote<T>(ref mut target, source)
zone::allocation_zone(data)
zone::metadata(data)
zone::from_zone(ref mut zone)
zone::of<T: zone::ZoneBacked>(value)
value.zone()
metadata.as_ptr()
metadata.as_zone_ptr()
metadata.alloc(bytes, align)
metadata.alloc_array<T>(count)
metadata.capacity()
metadata.used()
metadata.remaining()
metadata.can_alloc(bytes)
metadata.can_alloc_array<T>(count)
metadata.equals(ref other)
zone::reset(ref mut zone)
zone::destroy(zone)
```

`alloc_array<T>` returns uninitialized storage for `count` consecutive `T`
values. It returns null for `0`, asserts for negative counts, and does not run
destructors for the slots; initialize before reading and prefer higher-level
handles when ownership matters.

`default_capacity()` returns the capacity used by `zone { ... }` current-zone
blocks when no explicit `zone(capacity)` value is written. It is 4096 bytes in
the current hosted compiler.

Zone capacity failures are runtime traps, not recoverable allocation results.
The hosted runtime writes a short stderr diagnostic before exiting: invalid
zone capacities name the valid range, allocation exhaustion suggests
`zone(capacity)` or a larger explicit zone, and invalid raw zone handles or raw
allocation arguments are named separately. Use `zone(capacity) { ... }` for
bulk scratch work when the 4096-byte default is too small.

`capacity(ref mut zone)`, `used(ref mut zone)`, and `remaining(ref mut zone)`
read logical payload counters from a zone. Inside a current-zone block they can
omit the argument, so `zone::remaining()` reports the current block's scratch
space. The counters are planning/debugging helpers; they do not include
allocation headers or alignment padding.

`can_alloc(ref mut zone, bytes)` and `can_alloc_array<T>(ref mut zone, count)`
are preflight helpers over those logical counters. They return `false` for
negative inputs, `true` for zero array counts, and otherwise report whether the
requested payload bytes fit in the zone's current remaining capacity. Inside a
current-zone block the zone argument can be omitted, so `zone::can_alloc(512)`
checks the block's hidden scratch zone.

`allocation_zone(data)` reads Ari's allocation header for a non-null zone
allocation and returns the raw opaque handle. Prefer `metadata(data)`, which
wraps that handle as `ZoneMetadata`. `from_zone(ref mut zone)` captures
metadata directly from an explicit zone capability. `zone::of(value)` and
`value.zone()` use the `ZoneBacked` trait to expose `ZoneMetadata` from
supported heap-backed stdlib values such as `Box[T]`, `String`, `Vec[T]`,
zone-backed `std::collections` handles, and map update-entry handles.
`metadata.as_ptr()` is the raw escape hatch, `metadata.as_zone_ptr()` exposes
the same address as `ptr Zone`, and `metadata.equals(ref other)` checks handle
identity. `metadata.alloc(bytes, align)` and `metadata.alloc_array<T>(count)`
allocate through the recovered runtime zone handle. Zero-capacity handles may
carry metadata from construction even when they have no backing data pointer;
raw `metadata(data)` still requires a non-null allocation pointer.
`metadata.capacity()`, `metadata.used()`, and `metadata.remaining()` expose the
same counters through the recovered handle. `metadata.can_alloc(bytes)` and
`metadata.can_alloc_array<T>(count)` expose the same preflight checks for
zone-backed handles that grow from recovered metadata instead of an explicit
`ref mut Zone` field. The metadata handle follows normal zone provenance rules
and cannot be used after the source zone is reset or destroyed.

## Option And Result

`Option[T]` models a present or missing value:

```ari
Some(value)
None<T>()
value.is_some()
value.is_none()
value.as_ref()
value.as_mut()
value.take()
value.replace(next)
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
value.as_ref()
value.as_mut()
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
control-flow value. Use `as_ref` and `as_mut` when the payload must be borrowed
without consuming the enum; they return `OptionRef`/`OptionMut` and
`ResultRef`/`ResultMut` tagged-union view handles with branch predicates and borrowed
`unwrap` helpers. `Option::take` moves the payload out and leaves `None<T>()`;
`Option::replace(next)` stores the new payload and returns the previous
option. Use
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
`ZoneMetadata` recovered from the backing allocation header; the `_in` forms
remain available for explicit capability plumbing. `resize_with(length, make_value)` calls the
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
set.intersection(ref other)
set.difference(ref other)
set.union(ref other)
set.symmetric_difference(ref other)
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
the standard iterator path. `intersection`, `difference`, `union`, and
`symmetric_difference` are lazy `Iterator[T]` cursors that borrow both sets and
yield copied values; linear set algebra preserves insertion-order behavior for
the side being walked. Do not mutate either set while one of these cursors is
live. `drain()` yields insertion-order values and leaves the source set empty.

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
BinaryHeap::new<T: std::cmp::Ord[T]>(ref mut zone)
BinaryHeap::with_capacity<T: std::cmp::Ord[T]>(ref mut zone, capacity)
BinaryHeap::with_less<T>(ref mut zone, capacity, less)
collections::priority_queue<T>(ref mut zone, capacity, less)
PriorityQueue::new<T: std::cmp::Ord[T]>(ref mut zone)
PriorityQueue::with_capacity<T: std::cmp::Ord[T]>(ref mut zone, capacity)
PriorityQueue::with_less<T>(ref mut zone, capacity, less)
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
collections::hash_string(value)
collections::hash_map<K, V>(ref mut zone, capacity, hash)
collections::string_hash_map<V>(ref mut zone, capacity)
HashMap::new<K: std::hash::Hash[K], V>(ref mut zone)
HashMap::with_capacity<K: std::hash::Hash[K], V>(ref mut zone, capacity)
HashMap::with_hash<K, V>(ref mut zone, capacity, hash)
map.len()
map.capacity()
map.is_empty()
map.contains(key)
map.contains_key(key)
map.contains_value(value)
map.contains_key_bytes(bytes) // HashMap[String, V]
map.get(key)
map.get_bytes(bytes) // HashMap[String, V]
map.get_or(key, fallback)
map.get_or_bytes(bytes, fallback) // HashMap[String, V]
map.try_get(key)
map.try_get_bytes(bytes) // HashMap[String, V]
map.get_mut(key)
map.get_mut_bytes(bytes) // HashMap[String, V]
map.try_get_mut(key)
map.try_get_mut_bytes(bytes) // HashMap[String, V]
map.insert(ref mut zone, key, value)
map.replace(ref mut zone, key, value)
map.entry(ref mut zone, key)
map.entry(key)
map.remove(key)
map.remove_bytes(bytes) // HashMap[String, V]
map.remove_entry(key)
map.remove_entry_bytes(bytes) // HashMap[String, V]
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
collections::string_hash_set(ref mut zone, capacity)
HashSet::new<T: std::hash::Hash[T]>(ref mut zone)
HashSet::with_capacity<T: std::hash::Hash[T]>(ref mut zone, capacity)
HashSet::with_hash<T>(ref mut zone, capacity, hash)
set.len()
set.capacity()
set.is_empty()
set.contains(value)
set.contains_bytes(bytes) // HashSet[String]
set.get(value)
set.get_bytes(bytes) // HashSet[String]
set.try_get(value)
set.try_get_bytes(bytes) // HashSet[String]
set.equals(ref other)
set.is_subset(ref other)
set.is_superset(ref other)
set.is_disjoint(ref other)
set.intersection(ref other)
set.difference(ref other)
set.union(ref other)
set.symmetric_difference(ref other)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.take(value)
set.take_bytes(bytes) // HashSet[String]
set.remove(value)
set.remove_bytes(bytes) // HashSet[String]
set.clear()
set.retain(keep)
set.reserve(ref mut zone, capacity)
set.reserve_extra(ref mut zone, additional)
set.copy_to(ref mut target)
set.iter()
set.drain()
```

`collections::hash_i64` is a compatibility helper over `std::hash::value<i64>`.
`collections::hash_string` is the matching content hash for owned `String`
keys. `collections::string_hash_map` and `collections::string_hash_set` wire
that hash policy in for the common `HashMap[String,V]` and `HashSet[String]`
cases while explicit custom hash constructors remain available.
For parser-style code where the lookup key is already borrowed bytes,
`HashMap[String,V]` also exposes `contains_key_bytes`, `try_get_bytes`,
`get_bytes`, `get_or_bytes`, `try_get_mut_bytes`, `get_mut_bytes`,
`remove_bytes`, and `remove_entry_bytes`. `HashSet[String]` mirrors the same
idea with `contains_bytes`, `try_get_bytes`, `get_bytes`, `take_bytes`, and
`remove_bytes`. These helpers hash the borrowed `Slice[u8]` directly and
compare against stored `String` keys/values by byte content, avoiding a
temporary owned `String`.

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
natural `entry(key)` spelling. The entry handle stores only the map pointer and
key; allocation uses `map.zone()`/allocation-header metadata. `or_default`
requires `V: Default`, while
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
`HashSet.intersection`, `difference`, `union`, and `symmetric_difference` are
lazy live-bucket cursors. They borrow both sets, skip tombstones, yield copied
values, and use the current hash-bucket order rather than insertion or sorted
order, so callers should consume the cursor before mutating either set.

Tree collections use explicit strict less-than comparators:

```ari
collections::less_i64(left, right)
collections::tree_map<K, V>(ref mut zone, capacity, less)
TreeMap::new<K: std::cmp::Ord[K], V>(ref mut zone)
TreeMap::with_capacity<K: std::cmp::Ord[K], V>(ref mut zone, capacity)
TreeMap::with_less<K, V>(ref mut zone, capacity, less)
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
map.retain(keep)
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
TreeSet::new<T: std::cmp::Ord[T]>(ref mut zone)
TreeSet::with_capacity<T: std::cmp::Ord[T]>(ref mut zone, capacity)
TreeSet::with_less<T>(ref mut zone, capacity, less)
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
set.intersection(ref other)
set.difference(ref other)
set.union(ref other)
set.symmetric_difference(ref other)
set.insert(ref mut zone, value)
set.replace(ref mut zone, value)
set.take(value)
set.remove(value)
set.retain(keep)
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
Like hash entries, tree entries recover allocation from `map.zone()` instead of
carrying a separate zone pointer.
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
`TreeMap.retain(fn(ref K, ref mut V) -> bool)` filters live entries in place;
the predicate may mutate retained values, and rejected entries are dropped via
the same direct red-black deletion and compacting storage path as `remove`.
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
`TreeSet.retain(fn(ref T) -> bool)` filters values in place and drops rejected
values through the direct tree deletion path.
`TreeSet.intersection`, `difference`, `union`, and `symmetric_difference` are
lazy `Iterator[T]` cursors in comparator order. The merge-style operations
borrow both trees, yield copied values, and should be consumed before either
tree is mutated.
`TreeMap.values_mut()` exposes a mutable sorted value cursor,
`TreeMap.iter_mut()` walks sorted `MapEntryMut[K,V]` handles, direct
`for entry in tree_map` walks `MapEntry[K,V]`, and `TreeMap.drain()`/
`TreeSet.drain()` leave the source container empty while yielding sorted
drained entries or values. Tree
`copy_to` methods rebuild the map or set in the target zone with the same
comparator.

`std::string::String` is an owned byte string:

```ari
std::string::String
std::string::Utf8
std::string::Utf8String
std::string::Codepoints
std::string::OsStr
std::string::OsString
std::string::utf8(bytes)
std::string::utf8_string(ref mut zone, bytes)
std::string::utf8_string_optional(ref mut zone, bytes)
std::string::utf8_string_unchecked(ref mut zone, bytes)
std::string::codepoints(bytes)
std::string::os_str(bytes)
std::string::os_string(ref mut zone, bytes)
std::string::os_string_from_slice(ref mut zone, bytes)
std::string::c_str(cstr)
std::string::c_len(cstr)
std::string::c_bytes(cstr)
std::string::bytes(bytes)
std::string::new(ref mut zone, capacity)
std::string::empty(ref mut zone)
std::string::from(ref mut zone, "text")
std::string::copy(ref mut zone, bytes)
std::string::from_slice_in(ref mut zone, bytes)
std::string::join_in(ref mut zone, parts, separator)
std::string::lines(bytes)
std::string::trim_start(bytes)
std::string::trim_end(bytes)
std::string::trim(bytes)
std::string::split(bytes, delimiter)
std::string::split_once(bytes, delimiter)
std::string::find(bytes, needle)
std::string::contains(bytes, needle)
std::string::starts_with(bytes, prefix)
std::string::ends_with(bytes, suffix)
std::string::strip_prefix(bytes, prefix)
std::string::strip_suffix(bytes, suffix)
std::string::substring(bytes, start, end)
std::string::replace(ref mut zone, bytes, needle, replacement)
split_once.left()
split_once.right()
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
text.push_str(ref mut zone, bytes)
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
text.eq(other_owned_string)
text == other_owned_string
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
text.codepoint_next_index(byte_index)
text.codepoints()
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
utf8.next_index(byte_index)
utf8.codepoints()

utf8_string.as_string()
utf8_string.as_slice()
utf8_string.as_utf8()
utf8_string.len()
utf8_string.is_empty()
utf8_string.codepoint_count()
utf8_string.codepoint_at(byte_index)
utf8_string.next_index(byte_index)
utf8_string.codepoints()
utf8_string.to_string(ref mut zone)

os.as_slice()
os.len()
os.is_empty()
os.is_utf8()
os.try_utf8()

os_string.as_string()
os_string.as_slice()
os_string.as_os_str()
os_string.len()
os_string.is_empty()
os_string.is_utf8()
os_string.try_utf8()
os_string.try_utf8_string(ref mut zone)
os_string.to_string(ref mut zone)

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
`text.find("needle")` are valid. Immutable bindings initialized from a string
literal keep the same length information, and owned `std::string::String`
locals can be passed to byte-slice APIs without spelling `ref`; the compiler
borrows their byte view for the call. Literals can also act as read-only
`Slice[u8]` receivers, so `"hello".starts_with("he")`,
`"hello".find("ll")`, and `"hello".slice(1, 4).equals("ell")` use the same
borrowed slice vocabulary as named views. They can also initialize local byte
storage: `var bytes: Vec[u8] = "true";` and
`let fixed: [u8, 4] = "true";`.
`std::string::bytes(bytes)` returns the same borrowed byte view when code wants
to name the boundary explicitly. Single-quoted byte
character literals such as `'t'`, `'\n'`, and `'\x74'` are `u8`, so local byte
vectors can still be written as `['t', 'r', 'u', 'e']` when per-byte spelling
is clearer.

`std::string::from(ref mut zone, "text")`, `std::string::copy(ref mut zone,
bytes)`, and `std::string::empty(ref mut zone)` are the natural constructors
for everyday code. The older `from` and `from_slice_in` names remain
available when the source kind should be explicit. `text.append`, `append_byte`,
`append_bytes`, and `push_str` grow with the owning zone while hiding the
lower-level `append_string_in`, `append_value_in`, `append_debug_in`, and
`extend_from_slice_in` names from normal call sites. `String` is the current
appendable byte buffer for CLI output and parser diagnostics.
Tracked local strings can also call `text.append_value(value)` for `Display`
values and `text.append_debug(value)` for `Debug` values; the compiler lowers
those convenience calls to same-zone explicit forms.

`String` stores bytes, so module helpers such as `lines`, `trim`,
`split_once`, `starts_with`, `ends_with`, `contains`, `find`, `strip_prefix`,
`strip_suffix`, and `substring` operate on borrowed `Slice[u8]` views.
String literals can be passed directly to these helpers when the parameter type
is a byte slice. `split_once` returns `Option[std::string::SplitOnce]` with
borrowed `.left()` and `.right()` views around the first delimiter match.
`lines` splits on `'\n'` and leaves `'\r'` for a later ASCII trim step.
`replace(ref mut zone, bytes, needle, replacement)` is the allocating helper
for non-overlapping byte replacement; an empty needle is a no-op copy.
Owned `String` helpers such as `find`, `contains_slice`, `slice`, `split_at`,
`chunks`, `windows`, and delimiter `split` use the same byte offsets and
borrowed views. `find_text`, `contains_text`, `starts_with_text`,
`ends_with_text`, and `equals_text` accept Ari `string` values directly by
using `std::string::bytes` internally. `eq` and the `==` / `!=` operators
compare owned `String` values by byte contents, so independently allocated
strings can act as hash-map keys. `join_in` joins `Slice[Slice[u8]]` parts with
a byte separator into the caller's zone.
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
returns an `Option[i64]` scalar count, `codepoint_at` decodes one scalar at a
byte offset into `Option[std::encoding::Utf8Char]`, and
`codepoint_next_index` returns the next scalar byte offset. `codepoints`
validates the whole `String` and returns `Option[std::string::Codepoints]`;
`Utf8::codepoints()` skips that revalidation because `Utf8` already proves the
borrowed bytes valid. `std::string::codepoints(bytes)` is the borrowed-slice
constructor. The iterator yields `Utf8Char` values and implements
`std::Iterator[std::encoding::Utf8Char]`. `push_codepoint_in` appends one
Unicode scalar encoded as UTF-8 and panics for invalid scalar values. These
helpers work with Unicode scalar values, not grapheme clusters or
normalization. Use `std::string::utf8(bytes)` to construct a validated borrowed
`Utf8` view when a function requires UTF-8. Use
`std::string::utf8_string(ref mut zone, bytes)` when validated UTF-8 must own
its bytes; it returns `Result[Utf8String, std::encoding::Utf8Error]`, while
`utf8_string_optional` intentionally discards the diagnostic and
`utf8_string_unchecked` asserts trusted input. `Utf8String` exposes borrowed
`Utf8` and byte views, scalar helpers, and a copy-out `to_string` method, but
does not expose mutable byte access because mutation could break the invariant.
Use `OsStr` for borrowed operating-system bytes that may not be UTF-8 and
`OsString` for the owned POSIX-byte counterpart. `OsString::try_utf8` and
`try_utf8_string` validate before crossing into UTF-8 APIs. Use `PathBytes` for
path interpretation, and `std::c::CStr` for NUL-terminated C ABI text.
`std::string::c_str(cstr)` returns that same `std::c::CStr` borrowed view.
String literals can flow
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

`std::cell` provides local interior mutability and one-time initialization:

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

once = OnceCell::new<T>(ref mut zone)
once.set(value)
once.get()
once.get_mut()
once.get_or_init(op)
once.take()
once.replace(value)
once.is_initialized()
once.is_empty()

lazy = Lazy::new<T>(ref mut zone, op)
lazy.force()
lazy.get()
lazy.is_initialized()
```

`std::rc` provides zone-backed shared ownership handles:

```ari
std::rc::rc<T>(ref mut zone, value)
std::rc::arc<T>(ref mut zone, value)

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

`Rc`, `Arc`, and `Weak` store only a shared control pointer. The zone is
recovered from that allocation header when the handle reports zone metadata;
there is no per-handle allocator field.

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
fmt::format_value<T: Display>(ref mut zone, value)
fmt::debug_value<T: Debug>(ref mut zone, value)
fmt::format<T: Display>(ref mut zone, template, value)
fmt::format2<A: Display, B: Display>(ref mut zone, template, first, second)
fmt::format3<A: Display, B: Display, C: Display>(ref mut zone, template, first, second, third)
fmt::format4<A: Display, B: Display, C: Display, D: Display>(ref mut zone, template, first, second, third, fourth)
fmt::concat2<A: Display, B: Display>(ref mut zone, first, second)
fmt::concat3<A: Display, B: Display, C: Display>(ref mut zone, first, second, third)
fmt::write_concat2<W: io::Writer, A: Display, B: Display>(ref mut writer, ref mut zone, first, second)
fmt::write_concat2_bool<W: io::Writer, A: Display, B: Display>(ref mut writer, ref mut zone, first, second)
fmt::write_concat3<W: io::Writer, A: Display, B: Display, C: Display>(ref mut writer, ref mut zone, first, second, third)
fmt::write_concat3_bool<W: io::Writer, A: Display, B: Display, C: Display>(ref mut writer, ref mut zone, first, second, third)
fmt::write_format<W: io::Writer, T: Display>(ref mut writer, ref mut zone, template, value)
fmt::write_format2<W: io::Writer, A: Display, B: Display>(ref mut writer, ref mut zone, template, first, second)
fmt::write_format3<W: io::Writer, A: Display, B: Display, C: Display>(ref mut writer, ref mut zone, template, first, second, third)
fmt::write_format4<W: io::Writer, A: Display, B: Display, C: Display, D: Display>(ref mut writer, ref mut zone, template, first, second, third, fourth)
fmt::write_format_stream<W: io::Writer, T: Display>(ref mut writer, ref mut zone, template, value)
fmt::write_format_stream2<W: io::Writer, A: Display, B: Display>(ref mut writer, ref mut zone, template, first, second)
fmt::write_format_stream3<W: io::Writer, A: Display, B: Display, C: Display>(ref mut writer, ref mut zone, template, first, second, third)
fmt::write_format_stream4<W: io::Writer, A: Display, B: Display, C: Display, D: Display>(ref mut writer, ref mut zone, template, first, second, third, fourth)
fmt::write_unsigned<W: io::Writer>(ref mut writer, ref mut zone, value, spec)
fmt::write_unsigned_bool<W: io::Writer>(ref mut writer, ref mut zone, value, spec)
fmt::write_unsigned_stream<W: io::Writer>(ref mut writer, value, spec)
fmt::write_integer<W: io::Writer>(ref mut writer, ref mut zone, value)
fmt::write_integer_bool<W: io::Writer>(ref mut writer, ref mut zone, value)
fmt::write_integer_stream<W: io::Writer>(ref mut writer, value)
fmt::write_boolean<W: io::Writer>(ref mut writer, ref mut zone, value)
fmt::write_boolean_bool<W: io::Writer>(ref mut writer, ref mut zone, value)
fmt::write_boolean_stream<W: io::Writer>(ref mut writer, value)
fmt::write_text<W: io::Writer>(ref mut writer, ref mut zone, value)
fmt::write_text_bool<W: io::Writer>(ref mut writer, ref mut zone, value)
fmt::write_text_stream<W: io::Writer>(ref mut writer, value)
fmt::write_char_stream<W: io::Writer>(ref mut writer, value)
fmt::write_value<W: io::Writer, T: Display>(ref mut writer, ref mut zone, value)
fmt::write_value_bool<W: io::Writer, T: Display>(ref mut writer, ref mut zone, value)
fmt::write_debug<W: io::Writer, T: Debug>(ref mut writer, ref mut zone, value)
fmt::write_debug_bool<W: io::Writer, T: Debug>(ref mut writer, ref mut zone, value)
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
`write_*` helpers. The writer helpers return `Result[(), Error]` from the
underlying `std::io::Writer`; `_bool` wrappers keep compatibility for call sites
that intentionally discard the failure reason. `char` is a byte-character alias,
so `fmt::char_in`,
`String.append_value('A')`, and `format_in!(..., "{}", 'A')` write the byte as
text rather than as the number `65`.
Built-in `Debug` impls cover the same initial scalar/text set. `string` and
`String` debug output are quoted; `char` debug output is single-quoted. Use
`fmt::debug_value`, `fmt::write_debug`, or `fmt::println_debug` when diagnostic
output should use that policy.
`fmt::format_value` is the named source helper for the common one-value
`Display` case. `fmt::format`, `fmt::format2`, `fmt::format3`, and
`fmt::format4` are fixed-arity runtime-template helpers. They support `{}`
placeholders, escaped braces with `{{` and `}}`, allocate into the explicit zone, and return
`Err(InvalidInput)` when the template is malformed or the placeholder count
does not match the chosen arity. `fmt::concat2` and `fmt::concat3` build small
explicit-zone strings from `Display` values, which is handy for CLI messages
such as `"Compiling " + name` while Ari keeps general string interpolation and
hidden allocation out of the language. Use `concat_all` or `concat_strings` for
longer or dynamic piece counts instead of another fixed arity.
`fmt::write_concat2` and `fmt::write_concat3` write the same short
`Display`-driven message shape directly into an `io::Writer`, returning the
first writer error instead of allocating one combined output string. Their
`_bool` variants are compatibility wrappers for call sites that deliberately
discard the failure reason.
`fmt::write_format`, `fmt::write_format2`, `fmt::write_format3`, and
`fmt::write_format4` apply the same runtime-template rules for writer
destinations. They stream literal bytes and placeholder values directly into the writer instead of building one
combined output string first. Placeholder values still use
`Display::format_in`, so the supplied zone remains the explicit temporary
allocation capability. Invalid-template errors and writer failures stay
recoverable; bytes written before a later template or writer error are not
rolled back.
`fmt::write_format_stream`, `fmt::write_format_stream2`,
`fmt::write_format_stream3`, and `fmt::write_format_stream4` are explicit
streaming-name aliases for the same writer-backed runtime-template path. Use
them when call sites want to make the writer destination obvious. For known
scalar and text values, `write_unsigned_stream`, `write_integer_stream`,
`write_boolean_stream`, `write_text_stream`, and `write_char_stream` write
directly to the writer without allocating an owned `String`. They return the
first writer error as `Result[(), Error]`, and they are the preferred building
blocks for fully streaming helpers until compiler support for generic
writer-facing display dispatch lands.

The executable formatting path is still macro-based: `print!`, `println!`,
`eprintln!`, and `format_in!(ref mut zone, "...", values...)`. `{}` uses
display formatting, `{:.N}` gives float precision, and `{:?}` uses debug
formatting. `eprintln!` uses the same placeholder rules and writes to stderr.
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

`integer_in`, `boolean_in`, `text_in`, `char_in`, `format_value`,
`debug_text_in`, and `debug_char_in` are small
allocator-backed helpers for common scalar/text values. `write_*` helpers
format through an explicit temporary zone and then write to any `io::Writer`.
Full custom formatter objects and direct streaming formatters remain roadmap
work; `Debug` dispatch itself is source-level today. Variadic/default-zone
formatting is also still future work.

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
hash::string(value)
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
hashes, `hash::bytes` for a borrowed `Slice[u8]`, `hash::string` for owned
`String`, and `Hasher` plus `write` calls for incremental hashing. Current
built-in impls cover fixed-width signed and unsigned integer types, `bool`,
`Slice[u8]`, and `String`.

## Random

`std::random` contains OS entropy and deterministic non-cryptographic PRNG
helpers:

```ari
random::Prng
random::entropy()
random::entropy_raw()
random::entropy_unchecked()
random::fill(values)
random::fill_raw(values)
random::fill_unchecked(values)
random::seed(value)
random::from_entropy()
random::from_entropy_unchecked()
random::seed_from_os()
random::seed_from_os_unchecked()
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
Prng::from_entropy_unchecked()
Prng::seed_from_os()
Prng::seed_from_os_unchecked()
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
OS and failures should be returned as `std::error::Error`. On hosted Linux,
both use `getrandom` first and fall back to `/dev/urandom`; `fill(values)`
writes the caller's byte slice directly instead of looping through `entropy()`
words. The `_unchecked` helpers terminate on host entropy failure. The
`*_raw` forms keep the
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
parse::Parse
parse::ParseError
parse::ParseErrorKind
parse_error.kind()
parse_error.offset()
parse_error.name(ref mut zone)
parse_error.message(ref mut zone)
parse_error.name_text()
parse_error.message_text()
parse_error.is_empty_input()
parse_error.is_expected_digit()
parse_error.is_invalid_radix()
parse_error.is_invalid_digit()
parse_error.is_invalid_sign()
parse_error.is_invalid_separator()
parse_error.is_overflow()
parse_error.is_underflow()
parse::parse[T: Parse](bytes)
parse::parse_or[T: Parse](bytes, fallback)
parse::is_parse[T: Parse](bytes)
parse::integer(bytes)
parse::integer_error(bytes)
parse::integer_optional(bytes)
parse::is_integer(bytes)
parse::integer_or(bytes, fallback)
parse::integer_with_underscores(bytes)
parse::integer_with_underscores_error(bytes)
parse::integer_with_underscores_optional(bytes)
parse::is_integer_with_underscores(bytes)
parse::integer_with_underscores_or(bytes, fallback)
parse::integer_radix(bytes, radix)
parse::integer_radix_error(bytes, radix)
parse::integer_radix_optional(bytes, radix)
parse::is_integer_radix(bytes, radix)
parse::integer_radix_or(bytes, radix, fallback)
parse::integer_radix_with_underscores(bytes, radix)
parse::integer_radix_with_underscores_error(bytes, radix)
parse::integer_radix_with_underscores_optional(bytes, radix)
parse::is_integer_radix_with_underscores(bytes, radix)
parse::integer_radix_with_underscores_or(bytes, radix, fallback)
parse::unsigned(bytes)
parse::unsigned_error(bytes)
parse::unsigned_optional(bytes)
parse::is_unsigned(bytes)
parse::unsigned_or(bytes, fallback)
parse::unsigned_with_underscores(bytes)
parse::unsigned_with_underscores_error(bytes)
parse::unsigned_with_underscores_optional(bytes)
parse::is_unsigned_with_underscores(bytes)
parse::unsigned_with_underscores_or(bytes, fallback)
parse::unsigned_radix(bytes, radix)
parse::unsigned_radix_error(bytes, radix)
parse::unsigned_radix_optional(bytes, radix)
parse::is_unsigned_radix(bytes, radix)
parse::unsigned_radix_or(bytes, radix, fallback)
parse::unsigned_radix_with_underscores(bytes, radix)
parse::unsigned_radix_with_underscores_error(bytes, radix)
parse::unsigned_radix_with_underscores_optional(bytes, radix)
parse::is_unsigned_radix_with_underscores(bytes, radix)
parse::unsigned_radix_with_underscores_or(bytes, radix, fallback)
parse::hex_integer(bytes)
parse::hex_integer_optional(bytes)
parse::is_hex_integer(bytes)
parse::hex_integer_or(bytes, fallback)
parse::hex_integer_with_underscores(bytes)
parse::hex_integer_with_underscores_optional(bytes)
parse::is_hex_integer_with_underscores(bytes)
parse::hex_integer_with_underscores_or(bytes, fallback)
parse::binary_integer(bytes)
parse::binary_integer_optional(bytes)
parse::is_binary_integer(bytes)
parse::binary_integer_or(bytes, fallback)
parse::binary_integer_with_underscores(bytes)
parse::binary_integer_with_underscores_optional(bytes)
parse::is_binary_integer_with_underscores(bytes)
parse::binary_integer_with_underscores_or(bytes, fallback)
parse::octal_integer(bytes)
parse::octal_integer_optional(bytes)
parse::is_octal_integer(bytes)
parse::octal_integer_or(bytes, fallback)
parse::octal_integer_with_underscores(bytes)
parse::octal_integer_with_underscores_optional(bytes)
parse::is_octal_integer_with_underscores(bytes)
parse::octal_integer_with_underscores_or(bytes, fallback)
parse::boolean(bytes)
parse::boolean_optional(bytes)
parse::is_boolean(bytes)
parse::boolean_or(bytes, fallback)
parse::is_float(bytes)
parse::float(bytes)
parse::float_error(bytes)
parse::float_optional(bytes)
parse::float_or(bytes, fallback)
parse::is_float_with_underscores(bytes)
parse::float_with_underscores(bytes)
parse::float_with_underscores_error(bytes)
parse::float_with_underscores_optional(bytes)
parse::float_with_underscores_or(bytes, fallback)
parse::float_unchecked(bytes)
```

`integer` returns `Result[i64, Error]` and accepts optional `+` or `-` signs.
`integer_optional` keeps the compact compatibility `Option[i64]` shape,
`is_integer` validates the same shape, and `integer_or` returns a caller
fallback on invalid input. Decimal and radix integer parsers reject values
outside the `i64` range instead of wrapping. `integer_radix` accepts bases `2`
through `36` with ASCII alphanumeric digits, returns `InvalidInput` for invalid
radices, and returns `InvalidData` for invalid or out-of-range input.
`hex_integer` / `binary_integer` / `octal_integer` are readable wrappers for
common bases. Each has a matching `_optional`, `is_*`, and `*_or` helper.
When a caller needs a precise diagnostic, `integer_error`,
`integer_radix_error`, and the underscore-aware `*_error` variants return
`Option[ParseError]`: `None` means the input parses successfully, while
`Some(error)` carries a `ParseErrorKind` and the byte offset in the
ASCII-trimmed input where parsing failed. Integer diagnostics distinguish
`EmptyInput`, `ExpectedDigit`, `InvalidRadix`, `InvalidDigit`,
`InvalidSeparator`, and `Overflow`.
`ParseError::name()` returns a stable lowercase diagnostic label,
`ParseError::message()` returns a short explanatory sentence, and the
`is_*` predicates mirror each `ParseErrorKind` for direct branching in CLI and
configuration parser code.
`unsigned` and `unsigned_radix` are the matching `u64` `Result` parsers: they
accept an optional leading `+`, reject `-`, check overflow against `u64::MAX`,
and have matching `_optional`, `is_*`, and `*_or` helpers.
Their `*_error` helpers use the same `ParseError` shape and additionally report
`InvalidSign` for a leading `-`.
These radix parsers trim whitespace but do not recognize prefixes such as
`0x`, `0b`, or leading-zero octal policy. `boolean` returns
`Result[bool, Error]` and accepts only lowercase `true` and `false`;
`boolean_optional`, `is_boolean`, and `boolean_or` provide the same
compatibility validator/fallback pattern.
Default numeric parsers reject `_`. The explicit `*_with_underscores` integer,
radix, unsigned, base-wrapper, and float helpers accept `_` only between two
valid digits in that digit run, preserving the same `Result`, `_optional`,
`is_*`, and `_or` naming policy. `1_000`, `ff_ff`, and `1_000.5_0e1_2` are
valid in the underscore-aware helpers; `_1`, `1_`, `1__0`, `1_.0`, `1._0`, and
`1e_2` are invalid.
`is_float` validates a decimal float shape with optional sign, fraction, and
exponent. `float` returns `Result[f64, Error]` and reports `InvalidData` for
empty, invalid, trailing-garbage, or range-invalid input. `float_optional`
discards that error detail, `float_or` returns a fallback for invalid input, and
`float_unchecked` preserves the old asserting behavior by panicking on invalid
input.
`float_error` and `float_with_underscores_error` return `Option[ParseError]`
for strict and separator-aware float spelling diagnostics. They use
`ExpectedDigit` for missing mantissa or exponent digits, `InvalidSeparator` for
bad `_` placement, `InvalidDigit` for unsupported trailing bytes, `Overflow`
for decimal exponents too large for finite `f64`, and `Underflow` for decimal
exponents below Ari's accepted subnormal range. When an exponent is present,
the range diagnostic offset points at the exponent digit that crossed the
accepted range; otherwise it points at the significant mantissa digit that made
the effective decimal exponent too large or too small.

Use `parse::parse[T]` when the target type should choose the parser. It is the
asserting typed entry point, `parse::parse_or[T]` is the fallback form, and
`parse::is_parse[T]` validates without returning a value. The built-in `Parse`
impls cover `i64`, `u64`, `bool`, and `f64`:

```ari
let count = parse::parse[i64]("42");
let size = parse::parse[u64]("18446744073709551615");
let enabled = parse::parse[bool]("true");
let ratio = parse::parse[f64]("1.25e2");
```
Typed `parse[f64]` remains the asserting trait entry point; call
`parse::float` when ordinary float parse failures should return `Result`.

## Encoding

`std::encoding` contains text validation and byte codecs:

```ari
encoding::is_ascii(bytes)
encoding::is_unicode_scalar(scalar)
encoding::CodecErrorKind
encoding::CodecError
CodecError::name()
CodecError::message()
encoding::Utf8ErrorKind
encoding::Utf8Error
Utf8Error::name()
Utf8Error::message()
encoding::utf8_error(bytes)
encoding::validate_utf8(bytes)
encoding::validate_utf8_optional(bytes)
encoding::decode_utf8(ref mut zone, bytes)
encoding::decode_utf8_in(ref mut zone, bytes)
encoding::decode_utf8_optional_in(ref mut zone, bytes)
encoding::decode_utf8_unchecked_in(ref mut zone, bytes)
encoding::utf8_count(bytes)
encoding::utf8_count_optional(bytes)
encoding::is_utf8(bytes)
encoding::utf8_width(first_byte)
encoding::utf8_encoded_len(scalar)
encoding::utf8_at(bytes, byte_index)
encoding::utf8_next_index(bytes, byte_index)
encoding::encode_utf8(ref mut zone, scalar)
encoding::encode_utf8_in(ref mut zone, scalar)
encoding::encode_utf8_optional_in(ref mut zone, scalar)
encoding::try_encode_utf8_in(ref mut zone, scalar)
encoding::encode_utf8_unchecked_in(ref mut zone, scalar)
encoding::utf16_count(words)
encoding::utf16_count_optional(words)
encoding::is_utf16(words)
encoding::hex_encoded_len(bytes)
encoding::encode_hex_in(ref mut zone, bytes)
encoding::hex_error(bytes)
encoding::hex_decoded_len(bytes)
encoding::hex_decoded_len_optional(bytes)
encoding::can_decode_hex(bytes)
encoding::decode_hex(ref mut zone, bytes)
encoding::decode_hex_in(ref mut zone, bytes)
encoding::decode_hex_optional_in(ref mut zone, bytes)
encoding::try_decode_hex_in(ref mut zone, bytes)
encoding::decode_hex_unchecked_in(ref mut zone, bytes)
encoding::base64_encoded_len(bytes)
encoding::encode_base64_in(ref mut zone, bytes)
encoding::base64_error(bytes)
encoding::base64_decoded_len(bytes)
encoding::base64_decoded_len_optional(bytes)
encoding::can_decode_base64(bytes)
encoding::decode_base64(ref mut zone, bytes)
encoding::decode_base64_in(ref mut zone, bytes)
encoding::decode_base64_optional_in(ref mut zone, bytes)
encoding::try_decode_base64_in(ref mut zone, bytes)
encoding::decode_base64_unchecked_in(ref mut zone, bytes)
encoding::base64_mime_encoded_len(bytes)
encoding::encode_base64_mime_in(ref mut zone, bytes)
encoding::base64_mime_error(bytes)
encoding::base64_mime_decoded_len(bytes)
encoding::base64_mime_decoded_len_optional(bytes)
encoding::can_decode_base64_mime(bytes)
encoding::decode_base64_mime(ref mut zone, bytes)
encoding::decode_base64_mime_in(ref mut zone, bytes)
encoding::decode_base64_mime_optional_in(ref mut zone, bytes)
encoding::try_decode_base64_mime_in(ref mut zone, bytes)
encoding::decode_base64_mime_unchecked_in(ref mut zone, bytes)
encoding::base64_url_encoded_len(bytes)
encoding::base64_url_unpadded_encoded_len(bytes)
encoding::encode_base64_url_in(ref mut zone, bytes)
encoding::encode_base64_url_unpadded_in(ref mut zone, bytes)
encoding::base64_url_error(bytes)
encoding::base64_url_decoded_len(bytes)
encoding::base64_url_decoded_len_optional(bytes)
encoding::can_decode_base64_url(bytes)
encoding::decode_base64_url(ref mut zone, bytes)
encoding::decode_base64_url_in(ref mut zone, bytes)
encoding::decode_base64_url_optional_in(ref mut zone, bytes)
encoding::try_decode_base64_url_in(ref mut zone, bytes)
encoding::decode_base64_url_unchecked_in(ref mut zone, bytes)
```

`utf8_count` and `utf16_count` validate and return code-point counts through
`Result[i64, Error]`; the `*_optional` forms discard invalid-input detail into
`Option[i64]`, and the `is_*` forms return only a bool. `validate_utf8`
returns `Result[(), Utf8Error]`. `utf8_error` and `validate_utf8_optional`
return `None` for valid UTF-8 or `Some(Utf8Error)` with the failing byte index,
byte value, and a `Utf8ErrorKind` such as `InvalidContinuation`,
`OverlongEncoding`, or `SurrogateCodePoint`. `Utf8Error::name()` returns a
stable short label and `Utf8Error::message()` returns a longer human-readable
diagnostic for CLIs and logs. `decode_utf8(ref mut zone, bytes)` validates and
copies UTF-8 bytes into a zone-backed `String`;
`decode_utf8_in` is the compatibility spelling, while `_optional_in` and
`_unchecked_in` discard details or assert trusted input.
`Utf8Char` is the decoded UTF-8 scalar wrapper with `scalar()`, `len()`, and
`next_index(byte_index)`. `utf8_at` validates at one byte offset, while
`utf8_width` only classifies a lead byte. `encode_utf8(ref mut zone, scalar)`
returns `Result[String, Utf8Error]` for one Unicode scalar, and
`encode_utf8_in` is its compatibility spelling. Use `encode_utf8_optional_in`
or the compatibility `try_encode_utf8_in` alias when invalid scalars should be
`None`; use `encode_utf8_unchecked_in` only for trusted scalars.
Hex encoding emits lowercase digits and decoding accepts ASCII hex digits.
`hex_error`, `base64_error`, `base64_mime_error`, and `base64_url_error` return
`None` for valid input or `Some(CodecError)` with byte index, byte value, and a
`CodecErrorKind` of `InvalidLength`, `InvalidByte`, or `InvalidPadding`.
`CodecError::name()` and `CodecError::message()` provide stable user-facing
diagnostic text without forcing callers to duplicate the category mapping.
Base64 uses the standard `+`/`/` alphabet with `=` padding. MIME base64 uses
the standard alphabet, inserts CRLF after each 76 encoded characters, and
ignores ASCII space, tab, CR, and LF while decoding. URL-safe base64 uses
`-`/`_`; the padded encoder keeps `=`, the unpadded encoder omits it, and the
URL-safe decoder accepts both padded and unpadded URL-safe input while
rejecting the standard alphabet. Decoders use natural names such as
`decode_hex`, `decode_base64`, `decode_base64_mime`, and `decode_base64_url` for
`Result[String, Error]`. The older `decode_*_in` names are compatibility
spellings for the same explicit-zone operations, `_optional_in` or older
`try_decode_*_in` aliases return `Option[String]`, and `_unchecked_in` keeps
the asserting compatibility behavior.

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
