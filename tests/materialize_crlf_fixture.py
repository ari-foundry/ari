#!/usr/bin/env python3
"""Write a CRLF copy of a text fixture into build/.

Committed test inputs stay LF-normalized so `git diff --check` remains useful.
Artifact tests that need real CRLF bytes materialize those bytes in build/.
"""

from __future__ import annotations

from pathlib import Path
import sys


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        print("usage: materialize_crlf_fixture.py source destination", file=sys.stderr)
        return 2

    source = Path(argv[1])
    destination = Path(argv[2])
    text = source.read_text(encoding="utf-8")
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(text.replace("\n", "\r\n").encode("utf-8"))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
