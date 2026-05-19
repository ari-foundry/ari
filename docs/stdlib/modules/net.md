# std::net

`std::net` is Ari's portable networking module. It exists so programs can name
network addresses and, later, open sockets without declaring raw C networking
ABIs at every call site.

The first slice is deliberately source-only and value-oriented. It adds IPv4,
IPv6, generic IP, and socket-address values that are easy to test without
touching the host network. Runtime-backed DNS and socket handles are listed in
the roadmap below so their API shape can grow deliberately around ownership,
timeouts, nonblocking behavior, and platform error values.

## API

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
addr.octet(index)
addr.is_unspecified()
addr.is_loopback()
addr.as_ip()

Ipv6Addr::new(s0, s1, s2, s3, s4, s5, s6, s7)
Ipv6Addr::any()
Ipv6Addr::localhost()
addr.segment(index)
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
```

`Ipv4Addr` stores four `u8` octets. `Ipv4Addr::any()` returns `0.0.0.0`,
`Ipv4Addr::localhost()` returns `127.0.0.1`, and `octet(index)` reads octets
`0..3`.

`Ipv6Addr` stores eight `u16` segments. `Ipv6Addr::any()` returns `::`,
`Ipv6Addr::localhost()` returns `::1`, and `segment(index)` reads segments
`0..7`.

`IpAddr` is the generic address value. It is represented as a small tagged
struct instead of an enum today because Ari's current aggregate-enum storage
cannot yet mix differently-shaped `Ipv4Addr` and `Ipv6Addr` payloads. The
public API still reads naturally: convert concrete addresses with `as_ip()`,
then call `is_v4`, `is_v6`, `is_unspecified`, or `is_loopback`.

`SocketAddr` pairs an `IpAddr` with a `u16` port. Use `socket_addr(ip, port)`
or `SocketAddr::new(ip, port)` when the IP is already known. Use
`SocketAddr::localhost(port)` or `localhost(port)` for `127.0.0.1:port`.

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

Change only the port:

```ari
let local = net::SocketAddr::localhost(3000 as u16);
let https = local.with_port(443 as u16);
```

## Feature Status

| Need | Status |
| --- | --- |
| IP address | Current: `Ipv4Addr`, `Ipv6Addr`, `IpAddr`, constructors, family predicates, loopback/unspecified checks. |
| Socket address | Current: `SocketAddr`, `socket_addr`, `localhost`, `ip`, `port`, `with_port`. |
| DNS lookup | Roadmap: `lookup(host, service)` or `resolve(host, port)` over `getaddrinfo` with owned result storage and error values. |
| TCP listener | Roadmap: `TcpListener::bind(addr)`, `accept`, local address, nonblocking and timeout hooks. |
| TCP stream | Roadmap: `TcpStream::connect(addr)`, read/write adapters, peer/local address, shutdown. |
| UDP socket | Roadmap: `UdpSocket::bind(addr)`, `send_to`, `recv_from`, connected UDP helpers. |
| Unix domain socket | Roadmap: Unix-only `UnixListener`, `UnixStream`, and possibly datagram sockets behind platform docs. |
| socket options | Roadmap: `set_reuse_addr`, `nodelay`, buffer sizes, linger, multicast options where portable. |
| nonblocking socket | Roadmap: `set_nonblocking(socket, enabled)` or per-handle methods after OS handle ownership is settled. |
| timeout | Roadmap: connect/read/write timeout values based on `std::time::Duration`. |
| shutdown | Roadmap: `Shutdown::{Read, Write, Both}` and `stream.shutdown(mode)`. |

## Current Limits

- This slice does not open sockets, perform DNS lookup, or touch the host
  network. It is safe to run in deterministic compiler tests.
- There is no socket error type yet. Runtime-backed networking should not grow
  boolean-only APIs for operations where callers need OS error detail.
- Socket handles should be owned resources, unlike the copyable address values
  in this file. That needs the same ownership/drop policy being developed for
  `std::fs::File`.
- Text parsing and formatting of addresses are not implemented yet. The
  current constructors use numeric octets/segments so behavior is precise.

## Tests

```text
tests/cases/standard-library/ok/net/std-net-addresses.ari
```

`std-net-addresses.ari` covers IPv4/IPv6 constructors, generic `IpAddr`
family predicates, loopback/unspecified checks, socket-address construction,
port replacement, and associated/module constructor forms.

## Next Work

- Add address parsing and formatting once `std::string` formatting and parse
  policy can express dotted IPv4 and compressed IPv6 cleanly.
- Add DNS lookup as a runtime-backed slice returning `Option` or `Result`
  values instead of empty/sentinel data.
- Add TCP and UDP handles after owned OS-resource behavior is specified.
- Add Unix domain sockets behind explicit platform documentation.
- Add socket options, nonblocking mode, timeouts, and shutdown once error and
  handle policy are stable.
