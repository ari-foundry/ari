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
    root_readme_path = "README.md"
    root_readme = read(root_readme_path)
    require(root_readme, "https://ari-foundry.github.io", root_readme_path)

    docs_index_path = "docs/README.md"
    docs_index = read(docs_index_path)
    require(docs_index, "https://ari-foundry.github.io", docs_index_path)
    for needle in [
        "Detailed Ari language, compiler, standard library, and developer docs remain",
        "The [Ari Foundry portal](https://ari-foundry.github.io)",
        "ecosystem entry",
        "New compiler contributor",
        "## Ecosystem Portal",
        "## Language Users",
        "## Standard Library Users",
        "## Compiler Users",
        "## Compiler Contributors",
        "## Bootstrap / Ari-written compiler work",
        "## Bundled Tooling Docs",
        "These docs are currently bundled in `ari`",
        "future stable tooling projects may",
        "[Tooling Split Criteria](notes/tooling-split-criteria.md)",
        "https://github.com/ari-foundry/ari-lint",
        "Ari-language reimplementation",
        "migration is still in progress",
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
        "Ownership note: this developer guide belongs to the Ari compiler project.",
        "published as part of the Ari project docs",
        "`docs/dev` is not being split",
        "into a separate repository at this stage",
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

    notes_index_path = "docs/notes/README.md"
    notes_index = read(notes_index_path)
    require(
        notes_index,
        "[Documentation Ownership](documentation-ownership.md)",
        notes_index_path,
    )
    require(
        notes_index,
        "[Tooling Split Criteria](tooling-split-criteria.md)",
        notes_index_path,
    )
    require(
        notes_index,
        "[ari-lint Boundary Inventory](ari-lint-boundary-inventory.md)",
        notes_index_path,
    )
    require(
        notes_index,
        "[ari-lint CLI and Diagnostic Contract](ari-lint-cli-diagnostic-contract.md)",
        notes_index_path,
    )
    require(
        notes_index,
        "[ari-lint Dependency Model](ari-lint-dependency-model.md)",
        notes_index_path,
    )
    require(
        notes_index,
        "[ari-lint Standalone Test Plan](ari-lint-standalone-test-plan.md)",
        notes_index_path,
    )
    require(
        notes_index,
        "[ari-lint Repository Skeleton](ari-lint-repo-skeleton.md)",
        notes_index_path,
    )

    tooling_split_path = "docs/notes/tooling-split-criteria.md"
    tooling_split = read(tooling_split_path)
    for needle in [
        "# Tooling Split Criteria",
        "ari-lint is currently bundled",
        "ari-lsp is currently bundled",
        "editor integrations currently live under the ari repository",
        "stable command-line or protocol boundary",
        "compatibility with ari releases",
        "Do not split tools/lint in this step",
        "Do not split tools/lsp in this step",
        "Do not create ari-lint, ari-lsp, or ari-vscode repositories in this step",
        "Do not invent future repository links",
    ]:
        require(tooling_split, needle, tooling_split_path)

    lint_boundary_path = "docs/notes/ari-lint-boundary-inventory.md"
    lint_boundary = read(lint_boundary_path)
    for needle in [
        "# ari-lint Boundary Inventory",
        "`ari-lint` is currently bundled in the `ari` repository",
        "## Source Inventory",
        "## Test Inventory",
        "## Documentation Inventory",
        "## Compiler Boundary",
        "## CLI Boundary",
        "## Rule / Severity Boundary",
        "## Split Risks",
        "Do not move `tools/lint` in this step",
        "Do not create `ari-foundry/ari-lint` in this step",
        "Do not invent future repository links",
    ]:
        require(lint_boundary, needle, lint_boundary_path)

    lint_contract_path = "docs/notes/ari-lint-cli-diagnostic-contract.md"
    lint_contract = read(lint_contract_path)
    for needle in [
        "# ari-lint CLI and Diagnostic Contract",
        "## CLI Surface",
        "## Config File Contract",
        "## Rule and Severity Contract",
        "## Diagnostic Output Contract",
        "## Exit Status Contract",
        "## Compiler Boundary",
        "## JSON Compatibility Risks",
        "Do not move tools/lint in this step",
        "Do not create ari-foundry/ari-lint in this step",
        "Do not change ari-lint behavior in this step",
        "Do not invent future repository links",
    ]:
        require(lint_contract, needle, lint_contract_path)

    lint_dependency_path = "docs/notes/ari-lint-dependency-model.md"
    lint_dependency = read(lint_dependency_path)
    for needle in [
        "# ari-lint Dependency Model",
        "## Candidate Models",
        "### Model A: invoke `ari --check`",
        "### Model B: link compiler libraries directly",
        "### Model C: shared checker or diagnostics library",
        "Preferred near-term model: Model A",
        "## Required Boundary For Model A",
        "## Compatibility Implications",
        "## Split Risks",
        "Do not move `tools/lint` in this step",
        "Do not create `ari-foundry/ari-lint` in this step",
        "Do not introduce a shared checker library in this step",
        "Do not invent future repository links",
    ]:
        require(lint_dependency, needle, lint_dependency_path)

    lint_test_plan_path = "docs/notes/ari-lint-standalone-test-plan.md"
    lint_test_plan = read(lint_test_plan_path)
    for needle in [
        "# ari-lint Standalone Test Plan",
        "## Current Test Inventory",
        "## Standalone Fixture Goals",
        "## Compiler Binary Strategy",
        "## Test Categories",
        "## Golden Fixture Policy",
        "## CI Requirements",
        "## Split Risks",
        "Do not move `tools/lint` in this step",
        "Do not create `ari-foundry/ari-lint` in this step",
        "Do not add standalone tests in this step",
        "Do not invent future repository links",
    ]:
        require(lint_test_plan, needle, lint_test_plan_path)

    lint_repo_skeleton_path = "docs/notes/ari-lint-repo-skeleton.md"
    lint_repo_skeleton = read(lint_repo_skeleton_path)
    for needle in [
        "# ari-lint Repository Skeleton",
        "## Proposed Future Repository",
        "## Proposed Directory Layout",
        "## Source Migration Candidates",
        "## Documentation Migration Candidates",
        "## Test And Fixture Layout",
        "## CI Expectations",
        "## Compatibility And Release Metadata",
        "## AGENTS.md Expectations",
        "## Split Readiness Checklist",
        "Do not move `tools/lint` in this step",
        "Do not create `ari-foundry/ari-lint` in this step",
        "Do not add CI in this step",
        "Do not invent future repository links",
    ]:
        require(lint_repo_skeleton, needle, lint_repo_skeleton_path)

    ownership_path = "docs/notes/documentation-ownership.md"
    ownership = read(ownership_path)
    for needle in [
        "# Documentation Ownership",
        "The top-level Ari site is a portal only.",
        "The `ari` repository keeps the Ari language docs, compiler docs, standard",
        "library docs, developer docs",
        "`docs/dev` remains Ari project-owned.",
        "The developer guide belongs to the Ari",
        "compiler project",
        "is published as part of the Ari project docs",
        "is not",
        "being split into a separate repository at this stage",
        "`arix` is the Ari package manager project.",
        "It starts separately",
        "Lint, LSP, and editor docs stay bundled with `ari` until their projects",
        "split.",
    ]:
        require(ownership, needle, ownership_path)

    lint_index_path = "docs/lint/README.md"
    lint_index = read(lint_index_path)
    for needle in [
        "`ari-lint` is currently bundled in the `ari` repository",
        "becomes a separate project",
        "https://github.com/ari-foundry/ari-lint",
        "current bundled\n`tools/lint` reference implementation",
        "future direction is an Ari-language implementation",
        "source is not being moved\nwholesale",
        "compiler and standard library bugs should be filed in `ari-foundry/ari`",
        "stable standalone `ari-lint` release",
    ]:
        require(lint_index, needle, lint_index_path)

    lsp_index_path = "docs/lsp/README.md"
    lsp_index = read(lsp_index_path)
    for needle in [
        "`ari-lsp` is currently bundled in the `ari` repository",
        "becomes a separate project",
    ]:
        require(lsp_index, needle, lsp_index_path)

    editors_index_path = "editors/README.md"
    editors_index = read(editors_index_path)
    for needle in [
        "editor integrations are currently kept in the `ari` repository",
        "may move to dedicated repositories and project sites later",
    ]:
        require(editors_index, needle, editors_index_path)

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
        "Continue lexer parity by adding source-text smoke coverage for the next missing",
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
