# Ari Examples

Build every example:

```sh
make examples
```

Check the examples without linking:

```sh
make check-examples
```

Build one example:

```sh
make example EXAMPLE=fibonacci
```

Run one example:

```sh
make run-example EXAMPLE=hello
```

`count` is kept as the small exit-code example used by the quick start. Most
other examples print their result and exit with status `0`.
