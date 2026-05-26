# std::net

`std::net` is Ari's portable networking module. It exists so programs can
name network addresses, resolve IPv4 and IPv6 host names, and use explicit socket
handles without declaring raw C networking ABIs at every call site.

The module has two layers today. Address values are plain Ari source structs:
`Ipv4Addr`, `Ipv6Addr`, `IpAddr`, and `SocketAddr`. Runtime-backed handles use
`std::os::OwnedFd` internally and expose owned TCP, UDP, and Unix stream
socket shapes through `TcpListener`, `TcpStream`, `UdpSocket`,
`UnixListener`, and `UnixStream`. Each successful handle should be closed once
with `close()` until Ari grows drop-time resource cleanup.

The API deliberately keeps compatibility helpers beside `Result[..., Error]`
methods. Natural method names are the error-preserving forms where this module
owns the name; `_optional` and `_unchecked` methods discard error details for
older call sites. `try_*` helpers are compatibility aliases for the optional
forms. `*_raw` names are low-level compatibility bridges for callers
that need compact raw host errors instead of `std::error::Error`.

## API

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
net::Error
net::ErrorKind

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
net::resolve(endpoint)
net::resolve_optional(endpoint)
net::try_resolve(endpoint)
net::resolve_raw(endpoint)
net::resolve_all(zone, host, port)
net::to_socket_addrs(zone, endpoint)
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
addr.octet(index)
addr.try_octet(index)
addr.is_unspecified()
addr.is_loopback()
addr.as_ip()

Ipv6Addr::new(s0, s1, s2, s3, s4, s5, s6, s7)
Ipv6Addr::any()
Ipv6Addr::localhost()
addr.segment(index)
addr.try_segment(index)
addr.is_unspecified()
addr.is_loopback()
addr.as_ip()

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
listener.local_port_optional()
listener.local_addr()
listener.local_addr_optional()
listener.local_addr_v6()
listener.local_addr_v6_optional()
listener.is_nonblocking()
listener.is_nonblocking_optional()
listener.set_nonblocking(enabled)
listener.set_nonblocking_unchecked(enabled)
listener.close_on_exec()
listener.close_on_exec_optional()
listener.set_close_on_exec(enabled)
listener.set_close_on_exec_unchecked(enabled)
listener.reuse_addr()
listener.reuse_addr_optional()
listener.set_reuse_addr(enabled)
listener.set_reuse_addr_unchecked(enabled)
listener.reuse_port()
listener.reuse_port_optional()
listener.set_reuse_port(enabled)
listener.set_reuse_port_unchecked(enabled)
listener.set_accept_timeout(timeout)
listener.set_accept_timeout_millis(millis)
listener.set_accept_timeout_millis_unchecked(millis)
listener.accept()
listener.accept_optional()
listener.try_accept()
listener.accept_raw()
listener.close()
listener.close_unchecked()

TcpStream::connect(addr)
TcpStream::connect_optional(addr)
TcpStream::try_connect(addr)
TcpStream::connect_raw(addr)
stream.descriptor()
stream.is_open()
stream.local_addr()
stream.local_addr_optional()
stream.local_addr_v6()
stream.local_addr_v6_optional()
stream.peer_addr()
stream.peer_addr_optional()
stream.peer_addr_v6()
stream.peer_addr_v6_optional()
stream.is_nonblocking()
stream.is_nonblocking_optional()
stream.set_nonblocking(enabled)
stream.set_nonblocking_unchecked(enabled)
stream.close_on_exec()
stream.close_on_exec_optional()
stream.set_close_on_exec(enabled)
stream.set_close_on_exec_unchecked(enabled)
stream.nodelay()
stream.nodelay_optional()
stream.set_nodelay(enabled)
stream.set_nodelay_unchecked(enabled)
stream.keepalive()
stream.keepalive_optional()
stream.set_keepalive(enabled)
stream.set_keepalive_unchecked(enabled)
stream.send_buffer_size()
stream.set_send_buffer_size(value)
stream.recv_buffer_size()
stream.set_recv_buffer_size(value)
stream.set_read_timeout(timeout)
stream.set_read_timeout_millis(millis)
stream.set_read_timeout_millis_unchecked(millis)
stream.set_write_timeout(timeout)
stream.set_write_timeout_millis(millis)
stream.set_write_timeout_millis_unchecked(millis)
stream.shutdown(mode)
stream.try_read_byte()
stream.read(output)
stream.read_exact(output, len)
stream.read_exact_slice(output)
stream.read_to_end(zone)
stream.read_to_string(zone)
stream.read_exact_unchecked(output, len)
stream.write(values)
stream.write_all(values)
stream.write_all_unchecked(values)
stream.flush()
stream.close()
stream.close_unchecked()

UdpSocket::bind(addr)
UdpSocket::bind_optional(addr)
UdpSocket::try_bind(addr)
UdpSocket::bind_raw(addr)
socket.descriptor()
socket.is_open()
socket.local_port()
socket.local_port_optional()
socket.local_addr()
socket.local_addr_optional()
socket.local_addr_v6()
socket.local_addr_v6_optional()
socket.is_nonblocking()
socket.is_nonblocking_optional()
socket.set_nonblocking(enabled)
socket.set_nonblocking_unchecked(enabled)
socket.close_on_exec()
socket.close_on_exec_optional()
socket.set_close_on_exec(enabled)
socket.set_close_on_exec_unchecked(enabled)
socket.reuse_addr()
socket.reuse_addr_optional()
socket.set_reuse_addr(enabled)
socket.set_reuse_addr_unchecked(enabled)
socket.reuse_port()
socket.reuse_port_optional()
socket.set_reuse_port(enabled)
socket.set_reuse_port_unchecked(enabled)
socket.broadcast()
socket.broadcast_optional()
socket.set_broadcast(enabled)
socket.set_broadcast_unchecked(enabled)
socket.send_buffer_size()
socket.set_send_buffer_size(value)
socket.recv_buffer_size()
socket.set_recv_buffer_size(value)
socket.set_read_timeout(timeout)
socket.set_read_timeout_millis(millis)
socket.set_read_timeout_millis_unchecked(millis)
socket.set_write_timeout(timeout)
socket.set_write_timeout_millis(millis)
socket.set_write_timeout_millis_unchecked(millis)
socket.connect(addr)
socket.send_to(values, addr)
socket.send(values)
socket.recv(output)
socket.recv_from(output)
socket.peek_from(output)
socket.send_byte_to(value, addr)
socket.send_byte_to_unchecked(value, addr)
socket.recv_byte()
socket.try_recv_byte()
socket.recv_byte_unchecked()
socket.close()
socket.close_unchecked()

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
listener.is_nonblocking_optional()
listener.set_nonblocking(enabled)
listener.set_nonblocking_unchecked(enabled)
listener.close_on_exec()
listener.close_on_exec_optional()
listener.set_close_on_exec(enabled)
listener.set_close_on_exec_unchecked(enabled)
listener.accept()
listener.accept_optional()
listener.try_accept()
listener.accept_raw()
listener.close()
listener.close_unchecked()

UnixStream::connect(path)
UnixStream::connect_optional(path)
UnixStream::try_connect(path)
UnixStream::connect_raw(path)
stream.descriptor()
stream.is_open()
stream.is_nonblocking()
stream.is_nonblocking_optional()
stream.set_nonblocking(enabled)
stream.set_nonblocking_unchecked(enabled)
stream.close_on_exec()
stream.close_on_exec_optional()
stream.set_close_on_exec(enabled)
stream.set_close_on_exec_unchecked(enabled)
stream.set_read_timeout(timeout)
stream.set_read_timeout_millis(millis)
stream.set_read_timeout_millis_unchecked(millis)
stream.set_write_timeout(timeout)
stream.set_write_timeout_millis(millis)
stream.set_write_timeout_millis_unchecked(millis)
stream.shutdown(mode)
stream.try_read_byte()
stream.read(output)
stream.read_exact(output, len)
stream.read_exact_slice(output)
stream.read_to_end(zone)
stream.read_to_string(zone)
stream.read_exact_unchecked(output, len)
stream.write(values)
stream.write_all(values)
stream.write_all_unchecked(values)
stream.flush()
stream.close()
```

`TcpStream` and `UnixStream` implement `std::io::Reader` and
`std::io::Writer`, so byte-oriented helpers such as `stream.write_byte(value)`,
`stream.read_byte()`, `stream.write(values)`, `stream.write_all(values)`,
`std::io::write_all`, and `std::io::read_exact` work through the common IO
trait surface. They also expose inherent read helpers such as
`stream.read_exact(output, len)` for the common case where callers want natural
socket method syntax without spelling the generic IO trait adapter.

## Address Values

`Ipv4Addr` stores four `u8` octets. `Ipv4Addr::any()` returns `0.0.0.0`,
`Ipv4Addr::localhost()` returns `127.0.0.1`, and `octet(index)` reads octets
`0..3`. Use `octet(index)` when the index is a known constant or already
validated; use `try_octet(index)` when the index came from parsed input and
out-of-range values should become `None`.

`Ipv6Addr` stores eight `u16` segments. `Ipv6Addr::any()` returns `::`,
`Ipv6Addr::localhost()` returns `::1`, and `segment(index)` reads segments
`0..7`. Use `segment(index)` for known-good indexes and
`try_segment(index)` for fallible input validation.

`IpAddr` is the generic address value. It is represented as a small tagged
struct instead of an enum today because Ari's current aggregate-enum storage
cannot yet mix differently-shaped `Ipv4Addr` and `Ipv6Addr` payloads. The
public API still reads naturally: convert concrete addresses with `as_ip()`,
then call `is_v4`, `is_v6`, `is_unspecified`, or `is_loopback`.

`SocketAddr` pairs an `IpAddr` with a `u16` port. Use `socket_addr(ip, port)`
or `SocketAddr::new(ip, port)` when the IP is already known. Use
`SocketAddr::localhost(port)` or `localhost(port)` for `127.0.0.1:port`; use
`socket_addr(Ipv6Addr::localhost().as_ip(), port)` for `::1:port`.

## DNS Lookup

`lookup_v4(host, port)` resolves one IPv4 address through the hosted
`getaddrinfo` path and returns `Result[SocketAddr, Error]`. It is
intentionally a small first slice: the returned address uses the resolved IPv4
octets and the caller-provided port. Use `lookup_v4_optional(host, port)` or
`try_lookup_v4(host, port)` only when absence is enough and the caller is
intentionally discarding the reason. `lookup_v4_raw(host, port)` keeps
the compatibility `Result[SocketAddr, i64]` shape for low-level runtime
adapter tests.

`lookup_v6(host, port)` is the IPv6 sibling. It resolves one IPv6 address and
returns `Result[SocketAddr, Error]`; `lookup_v6_optional`/`try_lookup_v6`
discard the reason intentionally, and `lookup_v6_raw` keeps the compact
raw bridge for runtime tests.

`resolve(endpoint)` accepts the common `"host:port"` IPv4 spelling and the
bracketed IPv6 spelling `"[host]:port"`, returning
`Result[SocketAddr, Error]`. `resolve_optional(endpoint)` and
`try_resolve(endpoint)` are the compatibility forms that discard error detail,
while `resolve_raw(endpoint)` preserves the compatibility
`Result[SocketAddr, i64]` bridge. The endpoint form rejects missing hosts,
missing ports, non-decimal ports, and ports outside `0..65535` as
`InvalidInput` before it calls the hosted resolver.

`resolve_all(zone, host, port)` is the owned-list resolver shape. It returns a
zone-backed `Vec[SocketAddr]` and currently collects the first IPv4 address and
the first IPv6 address exposed by the hosted resolver, preserving `Error` when
neither family resolves. `to_socket_addrs(zone, endpoint)` is the module-level
host-port convenience wrapper for code that wants the same `Vec[SocketAddr]`
shape as the `ToSocketAddrs` trait. `string` implements `ToSocketAddrs`, so
generic code can ask a host-port string for addresses without spelling the
resolver function directly. Use brackets for IPv6 endpoints:

```ari
var zone = zone::create(1024);
let loopback = net::lookup_v6("::1", 8080 as u16).unwrap();
let endpoint = net::resolve("[::1]:8080").unwrap();
let addrs = net::to_socket_addrs(ref mut zone, "[::1]:8080").unwrap();
assert(addrs.len() > 0);
zone::destroy(zone);
```

Detailed `getaddrinfo` error categories, service names, canonical names, and
full `getaddrinfo` linked-list iteration are future work.

## TCP Sockets

At module level, `listen(addr)` and `tcp_listen(addr)` are natural aliases for
`TcpListener::bind(addr)`, and `connect(addr)`/`tcp_connect(addr)` are aliases
for `TcpStream::connect(addr)`. These natural APIs now dispatch on IPv4 or
IPv6 `SocketAddr` values. `tcp_listen_v6(addr)` and `tcp_connect_v6(addr)` are
explicit IPv6-only helpers that return `Unsupported` if the address is not
IPv6. `connect_host(endpoint)` and
`tcp_connect_host(endpoint)` combine `"host:port"` resolution with
`TcpStream::connect`. Use the short names for ordinary TCP code and the
explicit `tcp_*` names where UDP or Unix socket code appears nearby.

`TcpListener` owns a listening TCP descriptor. `bind` returns
`Result[TcpListener, Error]`; `bind_optional` and `try_bind` keep the old
information-discarding shape for simple compatibility code. `bind_raw`
keeps the old compact integer shape. Use `local_port()` after binding to port
`0` to learn the ephemeral port chosen by the OS, or `local_addr()` when the
caller needs the complete `SocketAddr`. `local_addr_v6()` is available when a
caller specifically wants to assert or require an IPv6 listener address.
`accept` returns
`Result[TcpStream, Error]`; `accept_optional` and `try_accept` discard the
reason, and `accept_raw` is the low-level compatibility form.
`local_port`, `local_addr`, option setter/query methods, and `close` preserve
invalid-handle and host failures as `std::error::Error`; `_optional` and
`_unchecked` methods remain compatibility helpers for callers that only need
success or absence.

`TcpStream` owns a connected TCP descriptor. `connect` returns
`Result[TcpStream, Error]`; `connect_optional` and `try_connect` discard OS
error detail for compatibility, and `connect_raw` keeps raw
compatibility. Use
`shutdown(Shutdown::Write)`, `shutdown(Shutdown::Read)`, or
`shutdown(Shutdown::Both)` to half-close or fully shut down the stream without
closing the descriptor owner. Use `write(values)` and `read(output)` for
partial-count buffer IO, `write_all(values)` to send every byte in a
`Slice[u8]`, and `read_exact(output, len)` or `read_exact_slice(output)` to
fill a caller-owned byte buffer or return `Error` if the stream closes or
errors first. `read_to_end(zone)` and `read_to_string(zone)` collect until the
peer closes; callers should reserve them for protocols where EOF is the
message boundary. `local_addr()` reports
the bound local socket address after connect or accept. `peer_addr()` reports
the connected remote `SocketAddr`: the listener address on the client side and
the accepted client address on the server side.
The natural stream methods expose `Error` payloads: closed handles become
`InvalidInput`, shutdown/option/address failures preserve host errors, write
failures use `BrokenPipe`, and short reads or receive sentinels use
`UnexpectedEof`. `try_read_byte` remains the single-byte compatibility shape
because the current `std::io::Reader::read_byte` trait method still occupies
that natural name with an `i64` EOF sentinel.
Use `local_addr_v6()` and `peer_addr_v6()` when an accepted or connected
stream is expected to be IPv6 and the caller wants family-specific failure
rather than generic address dispatch.
`close_on_exec`/`set_close_on_exec`, send/receive buffer-size setters, TCP
`keepalive`, TCP `nodelay`, and timeout helpers all return `Result` on their
natural names. `_optional` getters and `_unchecked` setters remain
compatibility conveniences where present.

## UDP Sockets

`UdpSocket` owns a UDP descriptor. `bind` returns
`Result[UdpSocket, Error]`; `bind_optional` and `try_bind` are compatibility
helpers that discard the reason; and `bind_raw` keeps the compact raw
host-error bridge. Use `local_port()` after binding to port `0` to discover
the OS-selected port, or `local_addr()` to retrieve the full local
`SocketAddr`. `udp_bind_v6(addr)` requires an IPv6 address and
`local_addr_v6()` reports an IPv6 local address explicitly.

Datagram payload APIs now include buffer-sized natural Result methods.
`send_to(values, addr)` sends one datagram and returns the byte count reported
by the host. `recv_from(output)` fills a caller-owned buffer and returns
`UdpRecvFrom`, which carries the received byte count and source `SocketAddr`;
`peek_from(output)` has the same return shape without consuming the datagram.
`connect(addr)` records a default peer for `send(values)` and `recv(output)`.
The older `send_byte_to(value, addr)`, `recv_byte()`, `try_recv_byte()`, and
`recv_byte_unchecked()` helpers remain compatibility conveniences for tests and
very small demos.
Use `send_to`, `recv_from`, `connect`, `send`, `recv`, `local_port`,
`local_addr`, option setter/query methods, and `close` when a caller needs to
keep invalid handles, unsupported address families, and host socket errors
visible. Use `send_byte_to_unchecked`, `recv_byte_unchecked`, and `_optional`
helpers only when discarding those errors is intentional. UDP exposes
`reuse_addr`, `reuse_port`, `broadcast`, close-on-exec, send-buffer, and
receive-buffer helpers as Result-returning natural methods; multicast and
TTL/hop-limit helpers are future slices.

## Unix Domain Sockets

`UnixListener` and `UnixStream` are Linux/Unix hosted stream socket wrappers.
`UnixListener::bind(path)` listens on a filesystem path and returns
`Result[UnixListener, Error]`; `UnixStream::connect(path)` connects to it and
returns `Result[UnixStream, Error]`. `bind_optional`, `connect_optional`, and
the matching `try_*` aliases keep compatibility call sites concise when they
intentionally discard the reason. Remove stale socket files with
`std::fs::remove(path)` before binding when tests or tools reuse a path.

`UnixListener::accept()` returns `Result[UnixStream, Error]`; `accept_optional`
and `try_accept` discard the reason. `UnixStream` implements the same
`std::io::Reader` and `std::io::Writer` traits as `TcpStream`, so local IPC
code can reuse byte-oriented IO helpers. It also has the same
`write`, `write_all`, `read`, `read_exact`, `read_exact_slice`,
`read_to_end`, `read_to_string`, and `flush` methods as `TcpStream` for
buffer-style local message tests and tools.
Unix stream/listener Result methods follow the same lifecycle policy as TCP:
owned handles close exactly once, closed-handle operations return
`InvalidInput`, and stream read/write/shutdown methods preserve ordinary
recoverable failures as `Error`.

## Options

All socket handles expose `descriptor()` and `is_open()` because they are
owned descriptor wrappers internally. TCP streams, UDP sockets, and Unix
streams expose read/write timeout setters that accept `std::time::Duration`;
TCP listeners expose `set_accept_timeout(timeout)`, which maps to the listener
read timeout used by `accept`.

`is_nonblocking()` and `set_nonblocking(enabled)` delegate to
`std::os::OwnedFd` descriptor flags and return `Result` values. The
`is_nonblocking_optional()` and `set_nonblocking_unchecked(enabled)` helpers
keep the older information-discarding shapes for ordinary control flow that
does not care about the failure reason.

`TcpListener` and `UdpSocket` expose `reuse_addr()` and
`set_reuse_addr(enabled)` for the common `SO_REUSEADDR` policy used by
servers, tests, and restartable tools. They also expose `reuse_port()` and
`set_reuse_port(enabled)` on hosted Linux/POSIX targets. `TcpStream` exposes
`nodelay()` and `set_nodelay(enabled)` for `TCP_NODELAY`, plus
`keepalive()`/`set_keepalive(enabled)` for the common `SO_KEEPALIVE` flag.
UDP sockets expose `broadcast()`/`set_broadcast(enabled)`. TCP streams and UDP
sockets expose `send_buffer_size`, `set_send_buffer_size`,
`recv_buffer_size`, and `set_recv_buffer_size` as Result-returning integer
option helpers. Query methods return `Result[..., Error]`; setters return
`Result[(), Error]`. The corresponding `_optional` and `_unchecked` helpers
discard those failures where they exist.

`close_on_exec()` and `set_close_on_exec(enabled)` are descriptor lifecycle
helpers on socket handles. Newly-created socket handles set close-on-exec by
default, and the explicit APIs let process-spawning code assert or adjust that
policy without reaching into `std::os::OwnedFd`.

Prefer the `Duration` setters in application and library code. The
`*_timeout_millis(millis)` forms return `Result` too. Their
`*_timeout_millis_unchecked(millis)` compatibility helpers are available for
FFI-style callers and tests that need to assert the runtime hook boundary
directly while discarding the error.

## Examples

Create loopback socket addresses:

```ari
let host = net::Ipv4Addr::localhost();
let ip = host.as_ip();
let addr = net::socket_addr(ip, 8080 as u16);
if addr.is_loopback() {
  return addr.port() as i64;
}
```

Resolve a numeric IPv4 host:

```ari
match net::lookup_v4("127.0.0.1", 8080 as u16) {
  std::Ok(addr) => {
    return addr.port() as i64;
  }
  std::Err(reason) => {
    return reason.code();
  }
}
```

Resolve a host-port endpoint:

```ari
match net::resolve("127.0.0.1:8080") {
  std::Ok(addr) => {
    return addr.port() as i64;
  }
  std::Err(reason) => {
    return reason.code();
  }
}
```

Bind a listener and connect a stream:

```ari
let bind_addr = net::SocketAddr::localhost(0 as u16);
match net::TcpListener::bind(bind_addr) {
  std::Ok(listener) => {
    var server = listener;
    let timeout = time::milliseconds(1000);
    server.set_accept_timeout(timeout).unwrap();
    let port = server.local_port().unwrap();
    var client = net::TcpStream::connect(net::SocketAddr::localhost(port)).unwrap();
    var accepted = server.accept().unwrap();
    client.set_write_timeout(timeout).unwrap();
    accepted.set_read_timeout(timeout).unwrap();
    var payload = [65u8, 66u8];
    client.write_all(payload.as_slice()).unwrap();
    var output = [0u8, 0u8];
    accepted.read_exact(output.as_slice().as_ptr(), 2).unwrap();
    client.close().unwrap();
    accepted.close().unwrap();
    server.close().unwrap();
    return ptr_load(output.as_slice().as_ptr()) as i64;
  }
  std::Err(reason) => {
    return reason.code();
  }
}
```

Send a UDP datagram to a loopback socket:

```ari
var server = net::UdpSocket::bind(net::SocketAddr::localhost(0 as u16)).unwrap();
let port = server.local_port().unwrap();
var client = net::UdpSocket::bind(net::SocketAddr::localhost(0 as u16)).unwrap();
var payload = [42u8, 43u8];
client.send_to(payload.as_slice(), net::SocketAddr::localhost(port)).unwrap();
var output = [0u8, 0u8];
let received = server.recv_from(output.as_slice()).unwrap();
assert(received.len() == 2);
client.close().unwrap();
server.close().unwrap();
return output[0] as i64;
```

Use a Unix stream socket:

```ari
let path = "build/prelude/example.sock";
std::fs::remove(path);
var listener = net::UnixListener::bind(path).unwrap();
var client = net::UnixStream::connect(path).unwrap();
var server = listener.accept().unwrap();
var payload = [7u8, 8u8];
client.write_all(payload.as_slice()).unwrap();
var output = [0u8, 0u8];
server.read_exact(output.as_slice().as_ptr(), 2).unwrap();
client.close().unwrap();
server.close().unwrap();
listener.close().unwrap();
return ptr_load(output.as_slice().as_ptr()) as i64;
```

## Feature Status

| Need | Status |
| --- | --- |
| IP address | Current: `Ipv4Addr`, `Ipv6Addr`, `IpAddr`, constructors, strict and fallible indexed accessors, family predicates, loopback/unspecified checks. |
| Socket address | Current: `SocketAddr`, `socket_addr`, `localhost`, `ip`, `port`, `with_port`. |
| DNS lookup | Current hosted IPv4/IPv6 slice: `lookup_v4`, `lookup_v6`, `"host:port"`, and `"[host]:port"` `resolve` return `Error`; `resolve_all(zone, host, port)` and `to_socket_addrs(zone, endpoint)` return zone-backed `Vec[SocketAddr]`; `_optional`/`try_*` helpers discard error detail intentionally; `_raw` helpers are raw compatibility bridges. |
| TCP listener | Current hosted IPv4/IPv6 slice: module-level `listen`/`tcp_listen`, explicit IPv6 `tcp_listen_v6`, `TcpListener::bind`, and `accept` return `Error`; optional/try compatibility and raw compatibility forms remain; Result-returning `local_port`/`local_addr` plus IPv6-specific `local_addr_v6`, descriptor/open/close-on-exec helpers, nonblocking, reuse-address, and reuse-port setter/query with Result defaults, `Duration` and raw-millisecond accept timeout setters, unchecked timeout compatibility, and explicit close/close_unchecked. |
| TCP stream | Current hosted IPv4/IPv6 slice: module-level `connect`/`tcp_connect`, explicit IPv6 `tcp_connect_v6`, and host-port `connect_host`/`tcp_connect_host`; `TcpStream::connect` returns `Error`, optional/try and raw compatibility forms remain, local/peer address helpers dispatch across IPv4/IPv6 with IPv6-specific `local_addr_v6`/`peer_addr_v6`, descriptor/open/close-on-exec helpers, nonblocking, TCP nodelay, keepalive, send/receive buffer-size helpers with Result defaults, `Duration` timeouts, shutdown, byte and buffer IO, close/close_unchecked, and `std::io::Reader`/`Writer` adapters. |
| UDP socket | Current hosted IPv4/IPv6 slice: module-level `udp_bind` plus explicit IPv6 `udp_bind_v6`, `UdpSocket::bind` returns `Error`, optional/try and raw compatibility forms remain, Result-returning local-port/local-address helpers including `local_addr_v6`, descriptor/open/close-on-exec helpers, nonblocking, reuse-address, reuse-port, broadcast, send/receive buffer-size helpers with Result defaults, `Duration` timeouts, datagram `send_to`/`recv_from`/`peek_from`, connected `send`/`recv`, single-byte compatibility helpers, and close/close_unchecked. |
| Unix domain socket | Current hosted stream slice: module-level `unix_listen`/`unix_connect`, `UnixListener` bind/accept and `UnixStream` connect return `Error`, optional/try compatibility helpers and raw compatibility bridges remain, IO/shutdown Result methods, close-on-exec helpers, `Duration` and raw-millisecond timeout setters, and `read`/`write`/`read_exact`/`write_all` buffer helpers. |
| socket options | Current: nonblocking, close-on-exec, read/write timeout, TCP listener/UDP reuse-address and reuse-port, TCP nodelay/keepalive, UDP broadcast, and TCP/UDP send/receive buffer-size helpers with Result defaults plus `_optional`/`_unchecked` compatibility where present; future linger, multicast, TTL/hop-limit, and readiness/poll abstractions. |
| timeout | Current: preferred `std::time::Duration` read/write/accept timeout setters plus raw millisecond setters, all Result-returning, with `_unchecked` raw-millisecond compatibility helpers. |
| shutdown | Current: `Shutdown::{Read, Write, Both}` and stream `shutdown(mode)` for TCP and Unix streams. |

## Current Limits

- Runtime-backed Internet sockets currently support hosted IPv4 and IPv6 TCP
  plus hosted IPv4 and IPv6 UDP on Linux/POSIX-like targets.
- DNS lookup exposes owned address lists, but the hosted implementation only
  collects the first IPv4 and first IPv6 address today. Full `getaddrinfo`
  iteration, canonical names, service names, and detailed resolver status are
  future work. The endpoint parser accepts `"host:port"` for IPv4/host names
  and `"[host]:port"` for IPv6; service-name ports are future work.
- UDP supports buffer datagrams, source-address return values, connected UDP,
  and peek. Multicast and TTL/hop-limit controls are future slices.
- Unix sockets are stream-only and path-based. Abstract namespace sockets,
  Unix datagram sockets, peer credentials, and platform guards need later
  design.
- Tests may run on hosts that forbid socket creation. In that case
  natural Result-returning helpers should report `PermissionDenied` or
  `Unsupported` through `std::error::Error`; socket tests treat that as host
  policy, not a language failure.
- Socket handles are owned descriptor wrappers. Close them once with `close()`.
  A second close through the same handle returns an
  invalid-handle failure through the Result form; descriptor duplication and
  richer drop policy should stay aligned with `std::os::OwnedFd`.
- Text parsing and formatting of addresses are not implemented yet. The
  current constructors use numeric octets/segments so behavior is precise.

## Tests

```text
tests/cases/standard-library/ok/net/std-net-addresses.ari
tests/cases/standard-library/ok/net/std-net-address-validation.ari
tests/cases/standard-library/ok/net/std-net-tcp-loopback.ari
tests/cases/standard-library/ok/net/std-net-udp-socket.ari
tests/cases/standard-library/ok/net/std-net-ipv6-socket.ari
tests/cases/standard-library/ok/net/std-net-unix-socket.ari
tests/cases/standard-library/ok/net/std-net-dns-lookup.ari
```

`std-net-addresses.ari` covers IPv4/IPv6 constructors, generic `IpAddr`
family predicates, loopback/unspecified checks, socket-address construction,
port replacement, and associated/module constructor forms.
`std-net-address-validation.ari` covers strict and fallible IPv4 octet and
IPv6 segment accessors.
`std-net-tcp-loopback.ari` covers module-level `listen`/`connect` and
explicit `tcp_*` aliases, IPv4 listener bind, ephemeral
local-port/local-address lookup, stream connect, accept, stream local-address
lookup, timeout/nonblocking helpers, close-on-exec, reuse-port, keepalive,
send/receive buffer-size options, stream shutdown, byte transfer through
both stream methods and `std::io::Reader`/`Writer`, and explicit close. On
restricted hosts it verifies that socket creation reports `PermissionDenied`
through the shared error bridge.
`std-net-udp-socket.ari` covers module-level `udp_bind`, IPv4 UDP bind,
local-port/local-address lookup, timeout/nonblocking helpers, close-on-exec,
reuse/broadcast/buffer-size options, datagram `send_to`/`recv_from`/`peek_from`,
connected `send`/`recv`, single-byte datagram compatibility helpers,
restricted-host fallback, and explicit close.
`std-net-ipv6-socket.ari` covers IPv6 lookup, bracketed endpoint resolution,
explicit `tcp_listen_v6`/`tcp_connect_v6`/`udp_bind_v6`, IPv6 local/peer
address helpers, TCP loopback transfer, UDP loopback datagrams, and
restricted-host fallback.
`std-net-unix-socket.ari` covers module-level Unix listener/connect wrappers,
Unix stream listener bind, stream connect,
accept, timeout/nonblocking helpers, bidirectional byte and buffer IO,
shutdown, close, and test socket-file cleanup.
`std-net-dns-lookup.ari` covers numeric IPv4 and IPv6 lookup, `Option` and
`Result` lookup shapes, `"host:port"` and bracketed `"[host]:port"` endpoint
resolution, `ToSocketAddrs`, host-connect input validation, rejected unbracketed
IPv6 input for the IPv4 resolver, and edge IPv4 addresses.

## Next Work

- Add address parsing and formatting once `std::string` formatting and parse
  policy can express dotted IPv4 and compressed IPv6 cleanly.
- Expand DNS from the current first-IPv4 plus first-IPv6 list into fuller
  `getaddrinfo` result iteration, including service-name ports, canonical
  names, and detailed resolver status.
- Add timeout-specific error categories once the runtime can distinguish
  deadline expiry from ordinary read, write, accept, and connect failures.
- Add remaining socket options such as linger, TTL/hop-limit, multicast
  join/leave, and close-on-exec-at-creation only with focused platform docs and
  tests.
- Add readiness/poll abstractions (`net::Poll`/`Events` or shared
  `io::poll_read`/`poll_write`) after the event-loop and nonblocking policy is
  clearer; arix MVP does not need this layer.
- Keep `std::net` focused on raw TCP/UDP/Unix sockets. TLS should live in a
  future arix package-layer library until the package ecosystem and certificate
  policy are deliberate.
- Add Unix datagram sockets, abstract namespace policy, and peer credential
  helpers behind explicit Linux/Unix platform documentation.
