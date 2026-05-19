# std::net Positive Tests

These cases cover the first `std::net` slice:

- source-only IPv4 and IPv6 address value constructors
- address-family predicates for `IpAddr`
- socket address construction, port access, and port replacement
- loopback and unspecified-address predicates

Keep network tests deterministic. The current positive tests do not open host
sockets or perform DNS lookups; those operations belong to later runtime-backed
`std::net` slices.

Files in this folder:

- `std-net-addresses.ari`: IP address and socket address value helpers.
