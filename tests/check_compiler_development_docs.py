#!/usr/bin/env python3
"""Keep the general compiler development roadmap useful.

This check is deliberately separate from the bootstrap readiness check. Ari is
developing the compiler first; bootstrapping is a later readiness milestone.
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
    roadmap_path = "docs/dev/compiler-development-roadmap.md"
    roadmap = read(roadmap_path)

    for heading in [
        "# Compiler Development Roadmap",
        "## Current Position",
        "## Development Principles",
        "## Compiler Areas",
        "## Roadmap",
        "### Phase 1: Frontend Reliability",
        "### Phase 2: Sema Boundaries",
        "### Phase 3: Generic Aggregate And Trait Maturity",
        "### Phase 4: Module And Project Ergonomics",
        "### Phase 5: Diagnostic Infrastructure",
        "### Phase 6: IR And Backend Artifacts",
        "### Phase 7: Bootstrap Start Readiness",
        "## Test Layout Policy",
        "## What Not To Do",
        "## Bootstrap Readiness Estimate",
    ]:
        require(roadmap, heading, roadmap_path)

    for needle in [
        "not a plan to start bootstrapping today",
        "readiness signal",
        "Improve Ari as a general language",
        "stage0 changes",
        "Sema",
        "LLVM backend",
        "diagnostic codes",
        "build/ari path/to/test.ari --check",
        "make check-compiler-dev-docs",
        "Do not implement bootstrapping as the current task",
        "38-42% ready",
        "58-62% remaining",
    ]:
        require(roadmap, needle, roadmap_path)

    if not re.search(r"\|\s*Area\s*\|\s*Goal\s*\|\s*Current Direction\s*\|", roadmap):
        print(f"{roadmap_path}: missing compiler area table", file=sys.stderr)
        return 1

    for index_path in ["docs/README.md", "docs/dev/README.md"]:
        index = read(index_path)
        require(index, "Compiler Development Roadmap", index_path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
