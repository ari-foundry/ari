#!/usr/bin/env python3
"""Extract a small, deterministic symbol inventory from an object or library."""

from __future__ import annotations

from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]


def display_path(path: Path) -> str:
    resolved = path.resolve()
    try:
        return resolved.relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


def collect_symbols(path: Path, dynamic: bool) -> dict[str, str]:
    command = ["nm", "-g", path.as_posix()]
    if dynamic:
        command.insert(1, "-D")
    result = subprocess.run(command, check=True, capture_output=True, text=True)
    symbols: dict[str, str] = {}
    for line in result.stdout.splitlines():
        parts = line.split()
        if len(parts) < 2:
            continue
        if len(parts) == 2:
            kind, name = parts
        else:
            kind, name = parts[-2], parts[-1]
        symbols[name] = "undefined" if kind == "U" else "defined"
    return symbols


def main(argv: list[str]) -> int:
    dynamic = False
    args = argv[1:]
    if args and args[0] == "--dynamic":
        dynamic = True
        args = args[1:]
    if len(args) < 2:
        print(
            "usage: extract_symbol_names.py [--dynamic] object symbol [symbol ...]",
            file=sys.stderr,
        )
        return 2

    path = Path(args[0])
    requested = args[1:]
    symbols = collect_symbols(path, dynamic)
    kind = "dynamic-symbols" if dynamic else "object-symbols"
    print(f"SymbolExtract kind={kind} source={display_path(path)} symbols={len(requested)}")
    for name in requested:
        print(f"  Symbol name={name} state={symbols.get(name, 'missing')}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
