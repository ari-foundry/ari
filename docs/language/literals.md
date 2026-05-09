# Literals

## Numeric Bases

Integer literals can be written in decimal, hexadecimal, octal, or binary:

```ari
let decimal = 42
let hex = 0xffu8
let octal = 0o755
let binary = 0b1010i16
```

The base prefixes are `0x`, `0o`, and `0b`. Uppercase `0X`, `0O`, and `0B`
are accepted too. Exact-width integer suffixes such as `u8`, `i16`, and `i64`
work on every integer base. Non-decimal float literals are not supported.

## String Escapes

String literals support the common C-style escapes:

```ari
println("line\nnext\tcolumn")
println("quote=\" slash=\\ byte=\x21")
```

Supported single-character escapes are `\a`, `\b`, `\e`, `\f`, `\n`, `\r`,
`\t`, `\v`, `\\`, `\"`, `\'`, and `\?`.

Byte escapes:

```ari
"\x21"
"\0123"
```

`\x` consumes hexadecimal digits and must fit in one byte. A backslash followed
by octal digits consumes the full octal run and must also fit in one byte.

Unicode escapes encode a Unicode scalar value as UTF-8:

```ari
"\u0041"
"\U0001F600"
"\u{1F600}"
```

Surrogate code points and values above `U+10FFFF` are rejected.
