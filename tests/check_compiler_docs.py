#!/usr/bin/env python3
"""Keep compiler-development docs focused and navigable.

This check is intentionally small. It protects the current C++ hosted compiler
documentation path while keeping the separate Ari-written compiler source root
discoverable.
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
        "[Ari-Written Compiler](notes/ari-written-compiler.md)",
        "[status](notes/ari-written-compiler-status.md)",
        "[roadmap](notes/ari-written-compiler-roadmap.md)",
        "[tasks](notes/ari-written-compiler-tasks.md)",
        "[validation/follow-up](notes/ari-written-compiler-validation.md)",
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
        "source lives directly under `compiler/`",
        "Do not start writing the Ari compiler in Ari here; use the separate",
        "Do not add `bootstrap/`, `stage1/`, `compiler/src/`",
    ]:
        require(dev_index, needle, dev_index_path)

    roadmap_path = "docs/dev/roadmap.md"
    roadmap = read(roadmap_path)
    for needle in [
        "# Compiler Roadmap",
        "This page tracks active compiler work for the current C++ hosted compiler.",
        "## Near-Term Order",
        "Function parameter patterns",
        "Generic thread results and compiler TLS",
        "Variadic/default-zone formatting",
        "## Language Ideas Parked For Later",
        "Capability requirement extensions",
        "`union by` extensions",
        "## What Not To Track Here",
        "Ari-written compiler source root now lives directly under `compiler/`",
        "Ari compiler rewrite tasks inside the hosted compiler roadmap",
        "package manager or cargo-like tool work",
        "standard-library maturity",
    ]:
        require(roadmap, needle, roadmap_path)

    readiness_path = "docs/dev/compiler-readiness-inventory.md"
    readiness = read(readiness_path)
    for needle in [
        "# Compiler Readiness Inventory",
        "It is not the Ari-written compiler source roadmap.",
        "The Ari-written compiler source lives in `compiler/`",
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

    ari_written_path = "docs/notes/ari-written-compiler.md"
    ari_written = read(ari_written_path)
    for needle in [
        "# Ari-Written Compiler",
        "`compiler/` is the Ari-written compiler source root",
        "There is no `compiler/src/`",
        "The existing C++ compiler remains stage0.",
        "## Temporary Split",
        "These split-out bootstrap notes are temporary.",
        "[Status](ari-written-compiler-status.md)",
        "[Roadmap](ari-written-compiler-roadmap.md)",
        "[Tasks](ari-written-compiler-tasks.md)",
        "[Validation And Follow-Ups](ari-written-compiler-validation.md)",
        "## Current Status",
        "## Incremental Roadmap",
        "## Phase Architecture",
        "Do not blindly copy the current C++ hosted compiler architecture.",
        "## Future Package Manager Transition",
        "## Small Task Queue",
        "## Next Recommended Task",
        "## Local Validation",
        "## Known Blockers",
        "## Stage0 Host Compiler Follow-Ups",
        "make check-ari-compiler-bootstrap",
        "`tests/cases/ari-compiler-bootstrap/`",
        "Confirmed host compiler bugs from this bootstrap slice are tracked there",
    ]:
        require(ari_written, needle, ari_written_path)

    ari_status_path = "docs/notes/ari-written-compiler-status.md"
    ari_status = read(ari_status_path)
    for needle in [
        "# Ari-Written Compiler Status",
        "temporary split-out note",
        "## Current Status",
        "`compiler/` has been started as a direct Ari source root.",
        "KeywordTable",
        "make check-ari-compiler-bootstrap",
        "`tests/cases/ari-compiler-bootstrap/`",
    ]:
        require(ari_status, needle, ari_status_path)

    ari_roadmap_path = "docs/notes/ari-written-compiler-roadmap.md"
    ari_roadmap = read(ari_roadmap_path)
    for needle in [
        "# Ari-Written Compiler Roadmap",
        "temporary split-out note",
        "## Incremental Roadmap",
        "## Phase Architecture",
        "Do not blindly copy the current C++ hosted compiler architecture.",
        "## Future Package Manager Transition",
        "package manager",
    ]:
        require(ari_roadmap, needle, ari_roadmap_path)

    ari_tasks_path = "docs/notes/ari-written-compiler-tasks.md"
    ari_tasks = read(ari_tasks_path)
    for needle in [
        "# Ari-Written Compiler Tasks",
        "temporary split-out note",
        "## Completed Tasks",
        "## Small Task Queue",
        "## Next Recommended Task",
        "Add source-text leading-comment operator diagnostics",
    ]:
        require(ari_tasks, needle, ari_tasks_path)

    ari_validation_path = "docs/notes/ari-written-compiler-validation.md"
    ari_validation = read(ari_validation_path)
    for needle in [
        "# Ari-Written Compiler Validation And Follow-Ups",
        "temporary split-out note",
        "## Local Validation",
        "## Known Blockers",
        "## Stage0 Host Compiler Follow-Ups",
        "make check-ari-compiler-bootstrap",
        "`tests/cases/ari-compiler-bootstrap/`",
        "Confirmed host compiler bugs from this bootstrap slice: none.",
    ]:
        require(ari_validation, needle, ari_validation_path)

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
