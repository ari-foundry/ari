#!/usr/bin/env python3
"""Extract selected LLVM function bodies as review-sized backend artifacts."""

from __future__ import annotations

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]


def display_path(path: Path) -> str:
    resolved = path.resolve()
    try:
        return resolved.relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


def extract_function(lines: list[str], symbol: str) -> list[str]:
    needle = f'@"{symbol}"'
    for index, line in enumerate(lines):
        if line.startswith("define ") and needle in line:
            body: list[str] = []
            for current in lines[index:]:
                body.append(current)
                if current == "}":
                    return body
            raise SystemExit(f"unterminated LLVM function for {symbol}")
    raise SystemExit(f"LLVM function not found: {symbol}")


def main(argv: list[str]) -> int:
    if len(argv) < 3:
        print(
            "usage: extract_llvm_functions.py input.ll symbol [symbol ...]",
            file=sys.stderr,
        )
        return 2

    path = Path(argv[1])
    lines = path.read_text(encoding="utf-8").splitlines()
    print(f"LLVMFunctionExtract source={display_path(path)} functions={len(argv) - 2}")
    for symbol in argv[2:]:
        print(f"  Function symbol={symbol}")
        for line in extract_function(lines, symbol):
            print(f"    {line}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
