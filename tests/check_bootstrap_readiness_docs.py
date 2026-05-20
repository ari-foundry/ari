#!/usr/bin/env python3
"""Keep the compiler bootstrap design docs linked and useful.

This is intentionally a small documentation smoke check. It does not prove
bootstrapping works; it prevents the production-design and entry-gate documents
from losing the sections that contributors need before the first Ari-written
compiler component lands.
"""

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
FIXTURE_GROUPS = [
    "modules",
    "source",
    "model",
    "generics",
    "traits",
    "errors",
    "zones",
    "formatting",
    "artifacts",
]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def require(text: str, needle: str, path: str) -> None:
    if needle not in text:
        print(f"{path}: missing {needle!r}", file=sys.stderr)
        raise SystemExit(1)


def main() -> int:
    design_path = "docs/dev/production-compiler-design.md"
    design = read(design_path)
    for heading in [
        "# Production Compiler Design",
        "## Current Bootstrap Readiness",
        "## Readiness Scorecard",
        "## Design Goal",
        "## Bootstrap Start Bar",
        "## Production Language Contract",
        "## General Language Design Work",
        "## Compiler Tooling Layer",
        "## Implementation Roadmap",
        "## First Implementation Slices",
        "## Test Strategy",
        "## What To Avoid",
    ]:
        require(design, heading, design_path)
    for needle in [
        "35-40% ready",
        "60-65% remaining",
        "not a bootstrap-only checklist",
        "ordinary production Ari program",
        "[Compiler Bootstrap Fixture Plan](bootstrap-fixture-plan.md)",
        "general language",
        "compiler/tooling package",
        "SourceId",
        "Stage artifact runner",
        "not in runtime `std`",
        "stage0",
        "stage1",
        "stage2",
    ]:
        require(design, needle, design_path)

    fixture_plan_path = "docs/dev/bootstrap-fixture-plan.md"
    fixture_plan = read(fixture_plan_path)
    for heading in [
        "# Compiler Bootstrap Fixture Plan",
        "## Goal",
        "## Placement",
        "## Fixture Groups",
        "## Naming Rules",
        "## Focused Targets",
        "## Artifact Policy",
        "## Start Gate",
    ]:
        require(fixture_plan, heading, fixture_plan_path)
    for needle in [
        "[Production Compiler Design](production-compiler-design.md)",
        "normal Ari programs",
        "tests/cases/bootstrap-readiness/",
        "token dumps",
        "syntax dumps",
        "diagnostic reports",
        "make check-bootstrap-readiness",
        "source line/column lookup",
        "named-capture artifact formatting",
        "Result[Token, LexError]",
        "not in runtime `std`",
    ]:
        require(fixture_plan, needle, fixture_plan_path)
    for group in FIXTURE_GROUPS:
        require(fixture_plan, f"| `{group}` |", fixture_plan_path)

    manifest_path = "tests/bootstrap_readiness_manifest.txt"
    manifest = read(manifest_path)
    for group in FIXTURE_GROUPS:
        require(manifest, f"{group}:", manifest_path)

    readiness_path = "docs/dev/bootstrap-readiness.md"
    readiness = read(readiness_path)
    require(
        readiness,
        "[Production Compiler Design](production-compiler-design.md)",
        readiness_path,
    )
    require(
        readiness,
        "[Compiler Bootstrap Fixture Plan](bootstrap-fixture-plan.md)",
        readiness_path,
    )
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

    if not re.search(r"35-40% ready", readiness):
        print(f"{readiness_path}: missing ready percentage", file=sys.stderr)
        return 1
    if not re.search(r"60-65% remaining", readiness):
        print(f"{readiness_path}: missing remaining percentage", file=sys.stderr)
        return 1

    self_host_path = "docs/dev/self-host-roadmap.md"
    self_host = read(self_host_path)
    require(self_host, "[Production Compiler Design](production-compiler-design.md)", self_host_path)
    require(self_host, "[Compiler Bootstrap Fixture Plan](bootstrap-fixture-plan.md)", self_host_path)
    require(self_host, "[Bootstrap Readiness](bootstrap-readiness.md)", self_host_path)

    for index_path in ["docs/README.md", "docs/dev/README.md"]:
        index = read(index_path)
        require(index, "Production Compiler Design", index_path)
        require(index, "Compiler Bootstrap Fixture Plan", index_path)
        require(index, "Bootstrap Readiness", index_path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
