# Compiler Development Tests

These fixtures protect ordinary Ari compiler-development work. They are not a
bootstrap implementation tree.

Use this folder for small programs that prove Ari can model compiler-shaped
data, pass results, artifact behavior, and expected compiler diagnostics with
the same language surface that normal Ari users get.

## Layout

- `ok/model/`: data-model and pass-flow fixtures.
- `ok/artifact/`: backend or artifact fixtures that should compile to LLVM,
  object, executable, or shared-library output.
- `errors/`: focused compiler diagnostics for unsupported or invalid surfaces.

