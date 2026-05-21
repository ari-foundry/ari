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

The API deliberately keeps simple `Option` helpers beside raw-error
`Result[..., i64]` helpers. Use `Option` when absence is enough; use
`*_result` when a caller needs the compact `std::error::Error.raw()` bridge.

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

net::ipv4(a, b, c, d)
net::ipv6(s0, s1, s2, s3, s4, s5, s6, s7)
net::socket_addr(ip, port)
net::localhost(port)
net::lookup_v4(host, port)
net::lookup_v4_result(host, port)

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
TcpListener::bind_result(addr)
listener.descriptor()
listener.is_open()
listener.local_port()
listener.local_addr()
listener.is_nonblocking()
listener.set_nonblocking(enabled)
listener.set_accept_timeout_millis(millis)
listener.accept()
listener.try_accept()
listener.accept_result()
listener.close()

TcpStream::connect(addr)
TcpStream::try_connect(addr)
TcpStream::connect_result(addr)
stream.descriptor()
stream.is_open()
stream.local_addr()
stream.peer_addr()
stream.is_nonblocking()
stream.set_nonblocking(enabled)
stream.set_read_timeout_millis(millis)
stream.set_write_timeout_millis(millis)
stream.shutdown(mode)
stream.try_read_byte()
stream.read_exact(output, len)
stream.write_all(values)
stream.close()

UdpSocket::bind(addr)
UdpSocket::try_bind(addr)
UdpSocket::bind_result(addr)
socket.descriptor()
socket.is_open()
socket.local_port()
socket.local_addr()
socket.is_nonblocking()
socket.set_nonblocking(enabled)
socket.set_read_timeout_millis(millis)
socket.set_write_timeout_millis(millis)
socket.send_byte_to(value, addr)
socket.recv_byte()
socket.try_recv_byte()
socket.close()

UnixListener::bind(path)
UnixListener::try_bind(path)
UnixListener::bind_result(path)
listener.descriptor()
listener.is_open()
listener.is_nonblocking()
listener.set_nonblocking(enabled)
listener.accept()
listener.try_accept()
listener.accept_result()
listener.close()

UnixStream::connect(path)
UnixStream::try_connect(path)
UnixStream::connect_result(path)
stream.descriptor()
stream.is_open()
stream.is_nonblocking()
stream.set_nonblocking(enabled)
stream.set_read_timeout_millis(millis)
stream.set_write_timeout_millis(millis)
stream.shutdown(mode)
stream.try_read_byte()
stream.read_exact(output, len)
stream.write_all(values)
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

`lookup_v4_result(host, port)` returns `Result[SocketAddr, i64]` for callers
that want a failure branch. The current error payload is the compact raw
`std::error::Error` bridge; detailed `getaddrinfo` error categories and owned
multi-address result lists are future work.

## TCP Sockets

`TcpListener` owns a listening TCP descriptor. `bind` and `try_bind` return
`Option[TcpListener]` for simple code. `bind_result` returns
`Result[TcpListener, i64]`, where the `i64` is the compact raw
`std::error::Error` bridge. Use `local_port()` after binding to port `0` to
learn the ephemeral port chosen by the OS, or `local_addr()` when the caller
needs the complete IPv4 `SocketAddr`. `accept`/`try_accept` return
`Option[TcpStream]`; `accept_result` exposes the raw error bridge.

`TcpStream` owns a connected TCP descriptor. `connect` and `try_connect` return
`Option[TcpStream]`; `connect_result` preserves OS error detail. Use
`shutdown(Shutdown::Write)`, `shutdown(Shutdown::Read)`, or
`shutdown(Shutdown::Both)` to half-close or fully shut down the stream without
closing the descriptor owner. Use `write_all(values)` to send every byte in a
`Slice[u8]`, and `read_exact(output, len)` to fill a caller-owned byte buffer
or return `false` if the stream closes or errors first. `local_addr()` reports
the bound local IPv4 socket address after connect or accept. `peer_addr()`
reports the connected remote IPv4 `SocketAddr`: the listener address on the
client side and the accepted client address on the server side.

## UDP Sockets

`UdpSocket` owns an IPv4 UDP descriptor. `bind`, `try_bind`, and `bind_result`
match the TCP listener return shapes. Use `local_port()` after binding to port
`0` to discover the OS-selected port, or `local_addr()` to retrieve the full
local IPv4 `SocketAddr`.

The current datagram payload surface is intentionally tiny:
`send_byte_to(value, addr)` sends one byte to an IPv4 `SocketAddr`,
`recv_byte()` returns a received byte as `i64` or `-1`, and
`try_recv_byte()` converts that shape to `Option[u8]`. Larger buffers,
source-address reporting, connected UDP, multicast, and IPv6 UDP are future
slices.

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

## Options

All socket handles expose `descriptor()` and `is_open()` because they are
owned descriptor wrappers internally. TCP streams, UDP sockets, and Unix
streams also expose read/write timeout setters in milliseconds. TCP listeners
expose `set_accept_timeout_millis(millis)`, which maps to the listener read
timeout used by `accept`.

`is_nonblocking()` and `set_nonblocking(enabled)` delegate to
`std::os::OwnedFd` descriptor flags. They return `Option[bool]` or `bool`
instead of panicking so invalid or already-closed handles can be handled by
ordinary control flow.

Timeout setters take raw milliseconds in this first slice. A later API should
also accept `std::time::Duration` after richer `Result` payloads and timeout
error categories are available across IO, fs, and net.

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

Bind a listener and connect a stream:

```ari
let bind_addr = net::SocketAddr::localhost(0 as u16);
match net::TcpListener::bind_result(bind_addr) {
  std::Ok(listener) => {
    var server = listener;
    server.set_accept_timeout_millis(1000);
    let port = server.local_port().unwrap();
    var client = net::TcpStream::connect(net::SocketAddr::localhost(port)).unwrap();
    var accepted = server.accept().unwrap();
    client.set_write_timeout_millis(1000);
    accepted.set_read_timeout_millis(1000);
    var payload = [65u8, 66u8];
    client.write_all(payload.as_slice());
    var output = [0u8, 0u8];
    accepted.read_exact(output.as_slice().as_ptr(), 2);
    return ptr_load(output.as_slice().as_ptr()) as i64;
  }
  std::Err(raw) => {
    let error = std::error::from_raw(raw);
    return error.code();
  }
}
```

Send one UDP byte to a loopback socket:

```ari
var server = net::UdpSocket::bind(net::SocketAddr::localhost(0 as u16)).unwrap();
let port = server.local_port().unwrap();
var client = net::UdpSocket::bind(net::SocketAddr::localhost(0 as u16)).unwrap();
client.send_byte_to(42u8, net::SocketAddr::localhost(port));
return server.recv_byte();
```

Use a Unix stream socket:

```ari
let path = "build/prelude/example.sock";
std::fs::remove(path);
var listener = net::UnixListener::bind(path).unwrap();
var client = net::UnixStream::connect(path).unwrap();
var server = listener.accept().unwrap();
var payload = [7u8, 8u8];
client.write_all(payload.as_slice());
var output = [0u8, 0u8];
server.read_exact(output.as_slice().as_ptr(), 2);
return ptr_load(output.as_slice().as_ptr()) as i64;
```

## Feature Status

| Need | Status |
| --- | --- |
| IP address | Current: `Ipv4Addr`, `Ipv6Addr`, `IpAddr`, constructors, strict and fallible indexed accessors, family predicates, loopback/unspecified checks. |
| Socket address | Current: `SocketAddr`, `socket_addr`, `localhost`, `ip`, `port`, `with_port`. |
| DNS lookup | Current hosted IPv4 slice: `lookup_v4`, `lookup_v4_result` over `getaddrinfo`. |
| TCP listener | Current hosted IPv4 slice: `TcpListener::bind`, `try_bind`, `bind_result`, `local_port`, `local_addr`, accept helpers, descriptor/open helpers, nonblocking setter/query, accept timeout, and explicit close. |
| TCP stream | Current hosted IPv4 slice: `TcpStream::connect`, `try_connect`, `connect_result`, `local_addr`, `peer_addr`, descriptor/open helpers, nonblocking setter/query, read/write timeout setters, shutdown, `try_read_byte`, `read_exact`, `write_all`, explicit close, and `std::io::Reader`/`Writer` adapters. |
| UDP socket | Current hosted IPv4 slice: bind helpers, local-port and local-address lookup, descriptor/open helpers, nonblocking setter/query, read/write timeout setters, single-byte `send_byte_to`, `recv_byte`, and `try_recv_byte`. |
| Unix domain socket | Current hosted stream slice: `UnixListener` bind/accept helpers and `UnixStream` connect/IO/shutdown plus `read_exact`/`write_all` buffer helpers. |
| socket options | Current: nonblocking and read/write timeout helpers; future reuse-address, nodelay, buffer size, linger, multicast, and close-on-exec-at-creation options. |
| timeout | Current: millisecond read/write/accept timeout setters; future `std::time::Duration` overloads and timeout-specific error results. |
| shutdown | Current: `Shutdown::{Read, Write, Both}` and stream `shutdown(mode)` for TCP and Unix streams. |

## Current Limits

- Runtime-backed Internet sockets currently support IPv4 hosted targets only.
  IPv6 address values exist, but IPv6 TCP/UDP socket handles are still future
  work.
- DNS lookup returns one IPv4 address and does not expose canonical names,
  multiple addresses, service names, or detailed `getaddrinfo` status yet.
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
- Socket handles are owned descriptor wrappers. Close them once with `close()`;
  descriptor duplication and richer drop policy should stay aligned with
  `std::os::OwnedFd`.
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
`std-net-tcp-loopback.ari` covers IPv6 unsupported errors, IPv4 listener bind,
ephemeral local-port/local-address lookup, stream connect, accept, stream
local-address lookup, timeout/nonblocking helpers, stream shutdown, byte
transfer through both stream methods and `std::io::Reader`/`Writer`, and
explicit close. On restricted hosts it verifies that socket creation reports
`PermissionDenied` through the shared error bridge.
`std-net-udp-socket.ari` covers IPv4 UDP bind, local-port/local-address lookup,
timeout/nonblocking helpers, single-byte datagram send/receive, unsupported
IPv6 bind errors, restricted-host fallback, and explicit close.
`std-net-unix-socket.ari` covers Unix stream listener bind, stream connect,
accept, timeout/nonblocking helpers, bidirectional byte and buffer IO,
shutdown, close, and test socket-file cleanup.
`std-net-dns-lookup.ari` covers numeric IPv4 lookup, `Option` and `Result`
lookup shapes, unsupported IPv6 text input, and edge IPv4 addresses.

## Next Work

- Add address parsing and formatting once `std::string` formatting and parse
  policy can express dotted IPv4 and compressed IPv6 cleanly.
- Add IPv6 TCP and UDP socket handles, UDP source address helpers, and richer
  socket-address reporting.
- Replace raw millisecond timeout setters with `std::time::Duration`-friendly
  helpers once direct `Result[..., Error]` payloads are available.
- Add UDP buffer-oriented send and receive helpers, including `recv_from`
  source-address reporting; TCP/Unix stream buffer helpers are available as
  `read_exact` and `write_all`.
- Add socket options such as reuse-address, nodelay, buffer sizes, linger, and
  multicast options only with focused platform docs and tests.
- Add Unix datagram sockets, abstract namespace policy, and peer credential
  helpers behind explicit Linux/Unix platform documentation.
