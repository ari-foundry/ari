#!/usr/bin/env python3
"""Keep compiler-development docs focused and navigable.

This check is intentionally small. It protects the current C++ hosted compiler
documentation path, not a future self-host implementation plan.
"""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def require(text: str, needle: str, path: str) -> None:
    if needle not in text:
        print(f"{path}: missing {needle!r}", file=sys.stderr)
        raise SystemExit(1)


def reject(text: str, needle: str, path: str) -> None:
    if needle in text:
        print(f"{path}: stale reference {needle!r}", file=sys.stderr)
        raise SystemExit(1)


def main() -> int:
    docs_index_path = "docs/README.md"
    docs_index = read(docs_index_path)
    for needle in [
        "New compiler contributor",
        "[Developer Overview](dev/README.md)",
        "[Architecture](dev/architecture.md)",
        "[Compiler Pipeline](dev/compiler-pipeline.md)",
        "[Compiler Layer Map](dev/compiler-layer-map.md)",
        "[Compiler Source Identity](dev/compiler-source-identity.md)",
        "[Compiler Diagnostic Authoring](dev/compiler-diagnostic-authoring.md)",
        "[Compiler Artifact Authoring](dev/compiler-artifact-authoring.md)",
        "[Compiler Test Authoring](dev/compiler-test-authoring.md)",
        "[Build And Test](dev/build-test.md)",
        "[Compiler Readiness Inventory](dev/compiler-readiness-inventory.md)",
        "[Roadmap](dev/roadmap.md)",
    ]:
        require(docs_index, needle, docs_index_path)

    dev_index_path = "docs/dev/README.md"
    dev_index = read(dev_index_path)
    for needle in [
        "# Ari Compiler Developer Overview",
        "This directory documents the current C++ hosted Ari compiler.",
        "## First-Hour Path",
        "## Where To Edit",
        "## What To Read By Goal",
        "## Focused Checks",
        "## Non-Goals",
        "make check-compiler-docs",
        "Do not start writing the Ari compiler in Ari here.",
        "Do not add `bootstrap/`, `stage1/`, or self-host implementation scaffolding.",
    ]:
        require(dev_index, needle, dev_index_path)

    roadmap_path = "docs/dev/roadmap.md"
    roadmap = read(roadmap_path)
    for needle in [
        "# Compiler Roadmap",
        "This page tracks active compiler work for the current C++ hosted compiler.",
        "## Near-Term Order",
        "## Language Ideas Parked For Later",
        "fn save(x: has serialize() -> i64) -> i64",
        "Structural capability parameters now have an implemented static method-only",
        "## What Not To Track Here",
        "Ari compiler rewrite tasks",
        "package manager or cargo-like tool work",
        "standard-library maturity",
    ]:
        require(roadmap, needle, roadmap_path)

    readiness_path = "docs/dev/compiler-readiness-inventory.md"
    readiness = read(readiness_path)
    for needle in [
        "# Compiler Readiness Inventory",
        "It is not a self-host plan.",
        "Source identity / source-map / span",
        "Diagnostics",
        "Module/project flow",
        "Frontend reliability",
        "Trait/generic readiness",
        "Artifact comparison",
        "Tool build flow",
        "make check-compiler-docs",
    ]:
        require(readiness, needle, readiness_path)

    for path in [
        docs_index_path,
        dev_index_path,
        roadmap_path,
        readiness_path,
        "docs/dev/test-matrix.md",
        "tests/README.md",
    ]:
        text = read(path)
        for stale in [
            "compiler-onboarding.md",
            "compiler-contributor-guide.md",
            "compiler-development-roadmap.md",
            "compiler-implementation-playbook.md",
            "compiler-next-slices.md",
            "compiler-change-checklist.md",
            "compiler-maturity-gates.md",
            "compiler-project-model.md",
            "compiler-source-diagnostics.md",
            "compiler-artifact-testing.md",
            "production-compiler-design.md",
            "bootstrap-fixture-plan.md",
            "bootstrap-readiness.md",
            "self-host-roadmap.md",
            "completed-milestones.md",
            "library-testing.md",
            "standard-library-roadmap.md",
            "make check-bootstrap-docs",
            "tests/check_bootstrap_readiness_docs.py",
        ]:
            reject(text, stale, path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
