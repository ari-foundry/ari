# std::net

`std::net` is Ari's portable networking module. It exists so programs can name
network addresses and open the first TCP sockets without declaring raw C
networking ABIs at every call site.

The module has two layers today. Address values are plain Ari source structs:
`Ipv4Addr`, `Ipv6Addr`, `IpAddr`, and `SocketAddr`. The first runtime-backed
socket slice adds owned IPv4 TCP listener/stream handles through
`TcpListener` and `TcpStream`. The socket API is intentionally small so error
policy, descriptor ownership, nonblocking behavior, and timeout support can
grow without forcing awkward compatibility wrappers later.

## API

```ari
net::Ipv4Addr
net::Ipv6Addr
net::IpAddr
net::SocketAddr
net::TcpListener
net::TcpStream

net::ipv4(a, b, c, d)
net::ipv6(s0, s1, s2, s3, s4, s5, s6, s7)
net::socket_addr(ip, port)
net::localhost(port)

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
listener.accept()
listener.try_accept()
listener.accept_result()
listener.close()

TcpStream::connect(addr)
TcpStream::try_connect(addr)
TcpStream::connect_result(addr)
stream.descriptor()
stream.is_open()
stream.try_read_byte()
stream.close()
```

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

`TcpListener` owns a listening TCP descriptor. `bind` and `try_bind` return
`Option[TcpListener]` for simple code. `bind_result` returns
`Result[TcpListener, i64]`, where the `i64` is the compact raw
`std::error::Error` bridge. Use `local_port()` after binding to port `0` to
learn the ephemeral port chosen by the OS. `accept`/`try_accept` return
`Option[TcpStream]`; `accept_result` exposes the raw error bridge.

`TcpStream` owns a connected TCP descriptor. `connect` and `try_connect` return
`Option[TcpStream]`; `connect_result` preserves OS error detail. `TcpStream`
implements `std::io::Reader` and `std::io::Writer`, so byte-oriented helpers
such as `stream.write_byte(value)`, `stream.read_byte()`,
`std::io::write_all`, and `std::io::read_exact` work without separate raw
socket functions. Use `try_read_byte()` when EOF or read failure should become
`None`.

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

Work with IPv6 segments:

```ari
let any = net::Ipv6Addr::any();
if any.is_unspecified() {
  return any.segment(0) as i64;
}
```

Validate parsed address indexes:

```ari
let addr = net::Ipv4Addr::localhost();
match addr.try_octet(3) {
  std::Some(value) => {
    return value as i64;
  }
  std::None => {}
}
return 1;
```

Change only the port:

```ari
let local = net::SocketAddr::localhost(3000 as u16);
let https = local.with_port(443 as u16);
```

Bind a listener and connect a stream:

```ari
let bind_addr = net::SocketAddr::localhost(0 as u16);
match net::TcpListener::bind_result(bind_addr) {
  std::Ok(listener) => {
    var server = listener;
    let port = server.local_port().unwrap();
    var client = net::TcpStream::connect(net::SocketAddr::localhost(port)).unwrap();
    var accepted = server.accept().unwrap();
    client.write_byte(65u8);
    return accepted.read_byte();
  }
  std::Err(raw) => {
    let error = std::error::from_raw(raw);
    return error.code();
  }
}
```

## Feature Status

| Need | Status |
| --- | --- |
| IP address | Current: `Ipv4Addr`, `Ipv6Addr`, `IpAddr`, constructors, strict and fallible indexed accessors, family predicates, loopback/unspecified checks. |
| Socket address | Current: `SocketAddr`, `socket_addr`, `localhost`, `ip`, `port`, `with_port`. |
| TCP listener | Current hosted IPv4 slice: `TcpListener::bind`, `try_bind`, `bind_result`, `local_port`, `accept`, `try_accept`, `accept_result`, `descriptor`, `is_open`, `close`. |
| TCP stream | Current hosted IPv4 slice: `TcpStream::connect`, `try_connect`, `connect_result`, `descriptor`, `is_open`, `try_read_byte`, `close`, plus `std::io::Reader`/`Writer` single-byte adapters. |
| DNS lookup | Roadmap: `lookup(host, service)` or `resolve(host, port)` over `getaddrinfo` with owned result storage and error values. |
| UDP socket | Roadmap: `UdpSocket::bind(addr)`, `send_to`, `recv_from`, connected UDP helpers. |
| Unix domain socket | Roadmap: Unix-only `UnixListener`, `UnixStream`, and possibly datagram sockets behind platform docs. |
| socket options | Roadmap: `set_reuse_addr`, `nodelay`, buffer sizes, linger, multicast options where portable. |
| nonblocking socket | Roadmap: `set_nonblocking(socket, enabled)` or per-handle methods after OS handle ownership is settled. |
| timeout | Roadmap: connect/read/write timeout values based on `std::time::Duration`. |
| shutdown | Roadmap: `Shutdown::{Read, Write, Both}` and `stream.shutdown(mode)`. |

## Current Limits

- TCP sockets currently support IPv4 hosted targets only. IPv6 socket handles,
  DNS lookup, UDP, Unix sockets, socket options, stream shutdown, and timeout
  policy remain roadmap work.
- Tests may run on hosts that forbid socket creation. In that case
  `bind_result` should report `PermissionDenied` through `std::error::Error`;
  the loopback test treats that as host policy, not a language failure.
- Runtime-backed networking should use `std::error::Error` for OS error detail
  instead of growing boolean-only APIs for operations where callers need to
  distinguish retryable, timeout, refused, unsupported, or invalid-input
  failures.
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
```

`std-net-addresses.ari` covers IPv4/IPv6 constructors, generic `IpAddr`
family predicates, loopback/unspecified checks, socket-address construction,
port replacement, and associated/module constructor forms.
`std-net-address-validation.ari` covers strict and fallible IPv4 octet and
IPv6 segment accessors.
`std-net-tcp-loopback.ari` covers IPv6 unsupported errors, IPv4 listener bind,
ephemeral local-port lookup, stream connect, accept, `std::io::Reader`/`Writer`
byte transfer, and explicit close. On restricted hosts it verifies that socket
creation reports `PermissionDenied` through the shared error bridge.

## Next Work

- Add address parsing and formatting once `std::string` formatting and parse
  policy can express dotted IPv4 and compressed IPv6 cleanly.
- Add DNS lookup as a runtime-backed slice returning `Option` or `Result`
  values instead of empty/sentinel data.
- Add IPv6 TCP sockets, peer/local address helpers, shutdown, and socket
  options once the first IPv4 handle slice has settled.
- Add UDP handles after owned OS-resource behavior is specified.
- Add Unix domain sockets behind explicit platform documentation.
- Add socket options, nonblocking mode, timeouts, and shutdown once error and
  handle policy are stable.
