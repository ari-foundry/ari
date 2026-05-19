# Compiler Sources

The C++17 compiler implementation lives here. The main flow is lexer -> parser -> semantic lowering -> typed IR -> LLVM codegen.

Use `make` after local edits and `make check` for compiler-facing behavior changes. Sanitizer checks are intentionally separate.
