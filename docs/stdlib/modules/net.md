# std::net

`std::net` is Ari's portable networking module. It exists so programs can
name network addresses, resolve IPv4 host names, and use explicit socket
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
older call sites. Existing `bind_result`, `connect_result`, `accept_result`,
and `*_raw_result` names are transitional compatibility forms until the older
Option-returning bind/connect surface is migrated.

## API

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
net::Error
net::ErrorKind

net::ipv4(a, b, c, d)
net::ipv6(s0, s1, s2, s3, s4, s5, s6, s7)
net::socket_addr(ip, port)
net::localhost(port)
net::lookup_v4(host, port)
net::lookup_v4_raw_result(host, port)
net::lookup_v4_result(host, port)
net::resolve(endpoint)
net::resolve_raw_result(endpoint)
net::resolve_result(endpoint)
net::to_socket_addrs(endpoint)
net::listen(addr)
net::tcp_listen(addr)
net::connect(addr)
net::tcp_connect(addr)
net::connect_host(endpoint)
net::tcp_connect_host(endpoint)
net::udp_bind(addr)
net::unix_listen(path)
net::unix_connect(path)

ToSocketAddrs::to_socket_addrs()

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
TcpListener::try_bind(addr)
TcpListener::bind_raw_result(addr)
TcpListener::bind_result(addr)
listener.descriptor()
listener.is_open()
listener.local_port()
listener.local_port_optional()
listener.local_addr()
listener.local_addr_optional()
listener.is_nonblocking()
listener.is_nonblocking_optional()
listener.set_nonblocking(enabled)
listener.set_nonblocking_unchecked(enabled)
listener.reuse_addr()
listener.reuse_addr_optional()
listener.set_reuse_addr(enabled)
listener.set_reuse_addr_unchecked(enabled)
listener.set_accept_timeout(timeout)
listener.set_accept_timeout_millis(millis)
listener.set_accept_timeout_millis_unchecked(millis)
listener.accept()
listener.try_accept()
listener.accept_raw_result()
listener.accept_result()
listener.close()
listener.close_unchecked()

TcpStream::connect(addr)
TcpStream::try_connect(addr)
TcpStream::connect_raw_result(addr)
TcpStream::connect_result(addr)
stream.descriptor()
stream.is_open()
stream.local_addr()
stream.local_addr_optional()
stream.peer_addr()
stream.peer_addr_optional()
stream.is_nonblocking()
stream.is_nonblocking_optional()
stream.set_nonblocking(enabled)
stream.set_nonblocking_unchecked(enabled)
stream.nodelay()
stream.nodelay_optional()
stream.set_nodelay(enabled)
stream.set_nodelay_unchecked(enabled)
stream.set_read_timeout(timeout)
stream.set_read_timeout_millis(millis)
stream.set_read_timeout_millis_unchecked(millis)
stream.set_write_timeout(timeout)
stream.set_write_timeout_millis(millis)
stream.set_write_timeout_millis_unchecked(millis)
stream.shutdown(mode)
stream.try_read_byte()
stream.read_exact(output, len)
stream.read_exact_unchecked(output, len)
stream.write_all(values)
stream.write_all_unchecked(values)
stream.close()
stream.close_unchecked()

UdpSocket::bind(addr)
UdpSocket::try_bind(addr)
UdpSocket::bind_raw_result(addr)
UdpSocket::bind_result(addr)
socket.descriptor()
socket.is_open()
socket.local_port()
socket.local_port_optional()
socket.local_addr()
socket.local_addr_optional()
socket.is_nonblocking()
socket.is_nonblocking_optional()
socket.set_nonblocking(enabled)
socket.set_nonblocking_unchecked(enabled)
socket.reuse_addr()
socket.reuse_addr_optional()
socket.set_reuse_addr(enabled)
socket.set_reuse_addr_unchecked(enabled)
socket.set_read_timeout(timeout)
socket.set_read_timeout_millis(millis)
socket.set_read_timeout_millis_unchecked(millis)
socket.set_write_timeout(timeout)
socket.set_write_timeout_millis(millis)
socket.set_write_timeout_millis_unchecked(millis)
socket.send_byte_to(value, addr)
socket.send_byte_to_unchecked(value, addr)
socket.recv_byte()
socket.try_recv_byte()
socket.recv_byte_unchecked()
socket.close()
socket.close_unchecked()

UnixListener::bind(path)
UnixListener::try_bind(path)
UnixListener::bind_raw_result(path)
UnixListener::bind_result(path)
listener.descriptor()
listener.is_open()
listener.is_nonblocking()
listener.is_nonblocking_optional()
listener.set_nonblocking(enabled)
listener.set_nonblocking_unchecked(enabled)
listener.accept()
listener.try_accept()
listener.accept_raw_result()
listener.accept_result()
listener.close()
listener.close_unchecked()

UnixStream::connect(path)
UnixStream::try_connect(path)
UnixStream::connect_raw_result(path)
UnixStream::connect_result(path)
stream.descriptor()
stream.is_open()
stream.is_nonblocking()
stream.is_nonblocking_optional()
stream.set_nonblocking(enabled)
stream.set_nonblocking_unchecked(enabled)
stream.set_read_timeout(timeout)
stream.set_read_timeout_millis(millis)
stream.set_read_timeout_millis_unchecked(millis)
stream.set_write_timeout(timeout)
stream.set_write_timeout_millis(millis)
stream.set_write_timeout_millis_unchecked(millis)
stream.shutdown(mode)
stream.try_read_byte()
stream.read_exact(output, len)
stream.read_exact_unchecked(output, len)
stream.write_all(values)
stream.write_all_unchecked(values)
stream.close()
```

`TcpStream` and `UnixStream` implement `std::io::Reader` and
`std::io::Writer`, so byte-oriented helpers such as `stream.write_byte(value)`,
`stream.read_byte()`, `std::io::write_all`, and `std::io::read_exact` work
through the common IO trait surface. They also expose inherent
`stream.write_all(values)` and `stream.read_exact(output, len)` methods for
the common case where callers want natural socket method syntax without
spelling the generic IO trait adapter.

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
`SocketAddr::localhost(port)` or `localhost(port)` for `127.0.0.1:port`.

## DNS Lookup

`lookup_v4(host, port)` resolves one IPv4 address through the hosted
`getaddrinfo` path and returns `Option[SocketAddr]`. It is intentionally a
small first slice: the returned address uses the resolved IPv4 octets and the
caller-provided port.

`lookup_v4_result(host, port)` returns `Result[SocketAddr, Error]` for callers
that want a failure branch. `lookup_v4_raw_result(host, port)` keeps the
compatibility `Result[SocketAddr, i64]` shape.

`resolve(endpoint)` accepts the common `"host:port"` spelling and returns
`Option[SocketAddr]`. `resolve_result(endpoint)` returns `Result[SocketAddr,
Error]`, while `resolve_raw_result(endpoint)` preserves the compatibility
`Result[SocketAddr, i64]` bridge. The endpoint form rejects missing hosts,
missing ports, non-decimal ports, and ports outside `0..65535` as
`InvalidInput` before it calls the hosted resolver.

`to_socket_addrs(endpoint)` is the module-level convenience wrapper for code
that wants the same shape as the `ToSocketAddrs` trait. The current trait is a
single-address seed rather than an iterator because the runtime resolver still
returns one IPv4 address. `string` implements `ToSocketAddrs`, so generic code
can ask a host-port string for a socket address without spelling the resolver
function directly.

Detailed `getaddrinfo` error categories, service names, canonical names, and
owned multi-address result lists are future work.

## TCP Sockets

At module level, `listen(addr)` and `tcp_listen(addr)` are natural aliases for
`TcpListener::bind_result(addr)`, and `connect(addr)`/`tcp_connect(addr)` are
aliases for `TcpStream::connect_result(addr)`. `connect_host(endpoint)` and
`tcp_connect_host(endpoint)` combine `"host:port"` resolution with
`TcpStream::connect_result`. Use the short names for ordinary TCP code and the
explicit `tcp_*` names where UDP or Unix socket code appears nearby.

`TcpListener` owns a listening TCP descriptor. `bind` and `try_bind` return
`Option[TcpListener]` for simple code. `bind_result` returns
`Result[TcpListener, Error]`, while `bind_raw_result` keeps the old compact
integer shape. Use `local_port()` after binding to port `0` to learn the
ephemeral port chosen by the OS, or `local_addr()` when the caller needs the
complete IPv4 `SocketAddr`. `accept`/`try_accept` return `Option[TcpStream]`;
`accept_result` exposes `Error`, and `accept_raw_result` is the low-level
compatibility form.
`local_port`, `local_addr`, option setter/query methods, and `close` preserve
invalid-handle and host failures as `std::error::Error`; `_optional` and
`_unchecked` methods remain compatibility helpers for callers that only need
success or absence.

`TcpStream` owns a connected TCP descriptor. `connect` and `try_connect` return
`Option[TcpStream]`; `connect_result` preserves OS error detail as `Error`, and
`connect_raw_result` keeps raw compatibility. Use
`shutdown(Shutdown::Write)`, `shutdown(Shutdown::Read)`, or
`shutdown(Shutdown::Both)` to half-close or fully shut down the stream without
closing the descriptor owner. Use `write_all(values)` to send every byte in a
`Slice[u8]`, and `read_exact(output, len)` to fill a caller-owned byte buffer
or return `Error` if the stream closes or errors first. `local_addr()` reports
the bound local IPv4 socket address after connect or accept. `peer_addr()`
reports the connected remote IPv4 `SocketAddr`: the listener address on the
client side and the accepted client address on the server side.
The natural stream methods expose `Error` payloads: closed handles become
`InvalidInput`, shutdown/option/address failures preserve host errors, write
failures use `BrokenPipe`, and short reads or receive sentinels use
`UnexpectedEof`. `try_read_byte` remains the single-byte compatibility shape
because the current `std::io::Reader::read_byte` trait method still occupies
that natural name with an `i64` EOF sentinel.

## UDP Sockets

`UdpSocket` owns an IPv4 UDP descriptor. `bind`, `try_bind`, `bind_result`, and
`bind_raw_result` match the TCP listener return shapes. Use `local_port()` after binding to port
`0` to discover the OS-selected port, or `local_addr()` to retrieve the full
local IPv4 `SocketAddr`.

The current datagram payload surface is intentionally tiny:
`send_byte_to(value, addr)` sends one byte to an IPv4 `SocketAddr`,
`recv_byte()` returns a received byte as `i64` or `-1`, and
`try_recv_byte()` converts that shape to `Option[u8]`. Larger buffers,
source-address reporting, connected UDP, multicast, and IPv6 UDP are future
slices.
Use `send_byte_to`, `recv_byte`, `local_port`, `local_addr`, option
setter/query methods, and `close` when a caller needs to keep invalid handles,
unsupported address families, and host socket errors visible. Use
`send_byte_to_unchecked`, `recv_byte_unchecked`, and `_optional` helpers only
when discarding those errors is intentional.

## Unix Domain Sockets

`UnixListener` and `UnixStream` are Linux/Unix hosted stream socket wrappers.
`UnixListener::bind(path)` listens on a filesystem path, and
`UnixStream::connect(path)` connects to it. Remove stale socket files with
`std::fs::remove(path)` before binding when tests or tools reuse a path.

`UnixListener::accept()` returns a `UnixStream`. `UnixStream` implements the
same `std::io::Reader` and `std::io::Writer` traits as `TcpStream`, so local
IPC code can reuse byte-oriented IO helpers. It also has the same
`write_all(values)` and `read_exact(output, len)` methods as `TcpStream` for
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
servers, tests, and restartable tools. `TcpStream` exposes `nodelay()` and
`set_nodelay(enabled)` for `TCP_NODELAY`, so latency-sensitive protocols can
disable Nagle buffering without dropping to raw C socket APIs. Query methods
return `Result[bool, Error]`; setters return `Result[(), Error]`. The
corresponding `_optional` and `_unchecked` helpers discard those failures.

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
  std::Some(addr) => {
    return addr.port() as i64;
  }
  std::None => {}
}
return 1;
```

Resolve a host-port endpoint:

```ari
match net::resolve_result("127.0.0.1:8080") {
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
match net::TcpListener::bind_result(bind_addr) {
  std::Ok(listener) => {
    var server = listener;
    let timeout = time::milliseconds(1000);
    server.set_accept_timeout(timeout).unwrap();
    let port = server.local_port().unwrap();
    var client = net::TcpStream::connect_result(net::SocketAddr::localhost(port)).unwrap();
    var accepted = server.accept_result().unwrap();
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

Send one UDP byte to a loopback socket:

```ari
var server = net::UdpSocket::bind(net::SocketAddr::localhost(0 as u16)).unwrap();
let port = server.local_port().unwrap();
var client = net::UdpSocket::bind(net::SocketAddr::localhost(0 as u16)).unwrap();
client.send_byte_to(42u8, net::SocketAddr::localhost(port)).unwrap();
let value = server.recv_byte().unwrap();
client.close().unwrap();
server.close().unwrap();
return value as i64;
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
| DNS lookup | Current hosted IPv4 slice: `lookup_v4`, `lookup_v4_result` with `Error`, `lookup_v4_raw_result` compatibility, `"host:port"` `resolve`/`resolve_result`/`resolve_raw_result`, module-level `to_socket_addrs`, and the `ToSocketAddrs` trait seed over `getaddrinfo`. |
| TCP listener | Current hosted IPv4 slice: module-level `listen`/`tcp_listen`, `TcpListener::bind`, `try_bind`, transitional `bind_result` with `Error`, `bind_raw_result` compatibility, Result-returning `local_port`/`local_addr`, `_optional` lookup helpers, accept helpers, descriptor/open helpers, nonblocking and reuse-address setter/query with Result defaults, `Duration` and raw-millisecond accept timeout setters, unchecked timeout compatibility, and explicit close/close_unchecked. |
| TCP stream | Current hosted IPv4 slice: module-level `connect`/`tcp_connect` plus host-port `connect_host`/`tcp_connect_host`, `TcpStream::connect`, `try_connect`, transitional `connect_result` with `Error`, `connect_raw_result` compatibility, Result-returning local/peer address helpers, descriptor/open helpers, nonblocking and TCP nodelay setter/query with Result defaults, `Duration` and raw-millisecond read/write timeout setters, shutdown, `try_read_byte`, Result-returning `read_exact`/`write_all`, unchecked buffer IO compatibility, explicit close/close_unchecked, and `std::io::Reader`/`Writer` adapters. |
| UDP socket | Current hosted IPv4 slice: module-level `udp_bind`, bind helpers with `Error` and raw compatibility forms, Result-returning local-port and local-address lookup, descriptor/open helpers, nonblocking and reuse-address setter/query with Result defaults, `Duration` and raw-millisecond read/write timeout setters, single-byte Result `send_byte_to`/`recv_byte`, `_unchecked` and `try_recv_byte` compatibility, and close/close_unchecked. |
| Unix domain socket | Current hosted stream slice: module-level `unix_listen`/`unix_connect`, `UnixListener` bind/accept and `UnixStream` connect helpers with `Error` and raw compatibility forms, IO/shutdown Result methods, `Duration` and raw-millisecond timeout setters, and `read_exact`/`write_all` buffer helpers. |
| socket options | Current: nonblocking, read/write timeout, TCP listener/UDP reuse-address, and TCP nodelay helpers with Result defaults plus `_optional`/`_unchecked` compatibility; future buffer size, linger, multicast, and close-on-exec-at-creation options. |
| timeout | Current: preferred `std::time::Duration` read/write/accept timeout setters plus raw millisecond setters, all Result-returning, with `_unchecked` raw-millisecond compatibility helpers. |
| shutdown | Current: `Shutdown::{Read, Write, Both}` and stream `shutdown(mode)` for TCP and Unix streams. |

## Current Limits

- Runtime-backed Internet sockets currently support IPv4 hosted targets only.
  IPv6 address values exist, but IPv6 TCP/UDP socket handles are still future
  work.
- DNS lookup returns one IPv4 address and does not expose canonical names,
  multiple addresses, service names, or detailed `getaddrinfo` status yet.
  The endpoint parser accepts only the simple `"host:port"` form; bracketed
  IPv6 endpoints and service-name ports are future work.
- UDP supports single-byte datagrams only. Buffer-oriented send/receive,
  source-address return values, connected UDP, multicast, and IPv6 UDP are
  future slices.
- Unix sockets are stream-only and path-based. Abstract namespace sockets,
  Unix datagram sockets, peer credentials, and platform guards need later
  design.
- Tests may run on hosts that forbid socket creation. In that case
  `*_result` helpers should report `PermissionDenied` or `Unsupported`
  through `std::error::Error`; socket tests treat that as host policy, not a
  language failure.
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
tests/cases/standard-library/ok/net/std-net-unix-socket.ari
tests/cases/standard-library/ok/net/std-net-dns-lookup.ari
```

`std-net-addresses.ari` covers IPv4/IPv6 constructors, generic `IpAddr`
family predicates, loopback/unspecified checks, socket-address construction,
port replacement, and associated/module constructor forms.
`std-net-address-validation.ari` covers strict and fallible IPv4 octet and
IPv6 segment accessors.
`std-net-tcp-loopback.ari` covers IPv6 unsupported errors, module-level
`listen`/`connect` and explicit `tcp_*` aliases, IPv4 listener bind, ephemeral
local-port/local-address lookup, stream connect, accept, stream local-address
lookup, timeout/nonblocking helpers, stream shutdown, byte transfer through
both stream methods and `std::io::Reader`/`Writer`, and explicit close. On
restricted hosts it verifies that socket creation reports `PermissionDenied`
through the shared error bridge.
`std-net-udp-socket.ari` covers module-level `udp_bind`, IPv4 UDP bind,
local-port/local-address lookup, timeout/nonblocking helpers, single-byte
datagram send/receive, unsupported IPv6 bind errors, restricted-host fallback,
and explicit close.
`std-net-unix-socket.ari` covers module-level Unix listener/connect wrappers,
Unix stream listener bind, stream connect,
accept, timeout/nonblocking helpers, bidirectional byte and buffer IO,
shutdown, close, and test socket-file cleanup.
`std-net-dns-lookup.ari` covers numeric IPv4 lookup, `Option` and `Result`
lookup shapes, `"host:port"` endpoint resolution, `ToSocketAddrs`,
host-connect input validation, unsupported IPv6 text input, and edge IPv4
addresses.

## Next Work

- Add address parsing and formatting once `std::string` formatting and parse
  policy can express dotted IPv4 and compressed IPv6 cleanly.
- Add IPv6 TCP and UDP socket handles, UDP source address helpers, and richer
  socket-address reporting.
- Add timeout-specific error categories once the runtime can distinguish
  deadline expiry from ordinary read, write, accept, and connect failures.
- Add UDP buffer-oriented send and receive helpers, including `recv_from`
  source-address reporting; TCP/Unix stream buffer helpers are available as
  `read_exact` and `write_all`.
- Add remaining socket options such as buffer sizes, linger, multicast options,
  and close-on-exec-at-creation only with focused platform docs and tests.
- Add Unix datagram sockets, abstract namespace policy, and peer credential
  helpers behind explicit Linux/Unix platform documentation.
