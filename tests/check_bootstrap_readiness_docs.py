#!/usr/bin/env python3
"""Keep the bootstrap readiness docs linked and useful.

This is intentionally a small documentation smoke check. It does not prove
bootstrapping works; it prevents the entry-gate document from losing the
sections that contributors need before the first Ari-written compiler component
lands.
"""

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def require(text: str, needle: str, path: str) -> None:
    if needle not in text:
        print(f"{path}: missing {needle!r}", file=sys.stderr)
        raise SystemExit(1)


def main() -> int:
    readiness_path = "docs/dev/bootstrap-readiness.md"
    readiness = read(readiness_path)
    for heading in [
        "# Bootstrap Readiness",
        "## Current Estimate",
        "## Bootstrap Start Gate",
        "## Needed Language Work",
        "## Needed Standard Library Work",
        "## Roadmap To Start",
        "## Test Plan",
        "## Stage Comparison Policy",
        "## What Not To Do Yet",
    ]:
        require(readiness, heading, readiness_path)

    if not re.search(r"32-37% ready", readiness):
        print(f"{readiness_path}: missing ready percentage", file=sys.stderr)
        return 1
    if not re.search(r"63-68% remaining", readiness):
        print(f"{readiness_path}: missing remaining percentage", file=sys.stderr)
        return 1

    self_host_path = "docs/dev/self-host-roadmap.md"
    self_host = read(self_host_path)
    require(self_host, "[Bootstrap Readiness](bootstrap-readiness.md)", self_host_path)

    for index_path in ["docs/README.md", "docs/dev/README.md"]:
        require(read(index_path), "Bootstrap Readiness", index_path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
