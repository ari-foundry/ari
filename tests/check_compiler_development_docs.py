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
    gates_path = "docs/dev/compiler-maturity-gates.md"
    gates = read(gates_path)
    contributor_path = "docs/dev/compiler-contributor-guide.md"
    contributor = read(contributor_path)
    readiness_path = "docs/dev/compiler-readiness-inventory.md"
    readiness = read(readiness_path)
    pass_contracts_path = "docs/dev/compiler-pass-contracts.md"
    pass_contracts = read(pass_contracts_path)
    project_model_path = "docs/dev/compiler-project-model.md"
    project_model = read(project_model_path)
    source_diagnostics_path = "docs/dev/compiler-source-diagnostics.md"
    source_diagnostics = read(source_diagnostics_path)
    artifact_testing_path = "docs/dev/compiler-artifact-testing.md"
    artifact_testing = read(artifact_testing_path)
    manifest_path = "tests/compiler_development_manifest.txt"
    manifest = read(manifest_path)

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
        "Compiler Maturity Gates",
        "Compiler Contributor Guide",
        "Compiler Readiness Inventory",
        "Compiler Pass Contracts",
        "Compiler Project Model",
        "Compiler Source And Diagnostics",
        "Compiler Artifact Testing",
        "stage0 changes",
        "`--emit-tokens`, `--emit-syntax`, `--emit-diagnostics`, and",
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

    for heading in [
        "# Compiler Maturity Gates",
        "## Current Estimate",
        "## How To Read The Gates",
        "## Maturity Gates",
        "## Implementation Order",
        "## Test Classification",
        "## Natural Language Design Rules",
        "## Readiness Formula",
        "## Non-Goals For Now",
    ]:
        require(gates, heading, gates_path)

    for needle in [
        "not a request to implement bootstrapping now",
        "ordinary compiler development",
        "38-42% ready",
        "58-62% remaining",
        "compiler/tooling package",
        "SourceId",
        "Diagnostic",
        "stable golden rendering",
        "file-backed modules",
        "Generic data models",
        "Trait selection",
        "Result[T, E]",
        "Stage comparison",
        "Do not create a `bootstrap/` tree",
    ]:
        require(gates, needle, gates_path)

    require(gates, "Compiler Source And Diagnostics", gates_path)
    require(gates, "Compiler Pass Contracts", gates_path)
    require(gates, "Compiler Contributor Guide", gates_path)
    require(gates, "Compiler Readiness Inventory", gates_path)
    require(gates, "Compiler Project Model", gates_path)
    require(gates, "Compiler Artifact Testing", gates_path)

    if not re.search(r"\|\s*Gate\s*\|\s*Required State\s*\|\s*Test Shape\s*\|\s*Status\s*\|", gates):
        print(f"{gates_path}: missing maturity gate table", file=sys.stderr)
        return 1

    for heading in [
        "# Compiler Contributor Guide",
        "## Start Here",
        "## Edit Map",
        "## Development Loop",
        "## Test Categories",
        "## Natural Design Rule",
        "## What Counts As Progress",
    ]:
        require(contributor, heading, contributor_path)

    for needle in [
        "not a bootstrap implementation plan",
        "Compiler Development Roadmap",
        "Compiler Readiness Inventory",
        "Compiler Pipeline",
        "Compiler Pass Contracts",
        "Feature Test Matrix",
        "Build And Test",
        "src/lexer.cpp",
        "src/parser.cpp",
        "src/sema.cpp",
        "src/ir.hpp",
        "src/llvm_codegen.cpp",
        "make check-compiler-development",
        "tests/cases/compiler-development/ok/model/",
        "bootstrap-readiness",
        "Result[T, E]",
        "38-42% ready",
        "58-62% remaining",
    ]:
        require(contributor, needle, contributor_path)

    for heading in [
        "# Compiler Readiness Inventory",
        "## Current Readiness",
        "## Already Strong",
        "## Blocking Gaps",
        "## Recent Compiler Support",
        "## Development Backlog",
        "## Start Gate",
        "## Natural Syntax Pressure",
        "## Test Inventory",
    ]:
        require(readiness, heading, readiness_path)

    for needle in [
        "not a bootstrap implementation plan",
        "normal compiler development",
        "38-42% ready",
        "58-62% remaining",
        "Hosted LLVM backend",
        "Core executable language",
        "Generic calls and ADTs",
        "Source identity",
        "Diagnostics",
        "File-backed projects",
        "Generic aggregate scale",
        "Trait selection",
        "Pass artifacts",
        "Mixed aggregate enum payload slots",
        "Result[Span, DiagnosticBuildError]",
        "scratch byte storage",
        "Postfix `?` can now propagate residual payloads through aggregate enum layouts",
        "Result[LineColumnRange, E]",
        "SourceFile",
        "SourceId",
        "Span",
        "Result[T, E]",
        "tests/cases/compiler-development/ok/model/compiler-pass-worklist.ari",
        "tests/cases/compiler-development/ok/model/compiler-diagnostic-workflow.ari",
        "tests/cases/compiler-development/ok/model/compiler-source-map-workflow.ari",
        "tests/cases/compiler-development/artifact/ok/token-dump-basic.ari",
        "tests/cases/compiler-development/artifact/ok/syntax-dump-basic.syntax",
        "tests/cases/compiler-development/artifact/ok/typed-ir-basic.ir",
        "tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.diagnostic",
        "--emit-tokens path",
        "--emit-syntax path",
        "--emit-diagnostics path",
        "--emit-typed-ir path",
        "make check-compiler-artifacts",
        "make check-compiler-development",
    ]:
        require(readiness, needle, readiness_path)

    for heading in [
        "# Compiler Pass Contracts",
        "## Goals",
        "## Pass Map",
        "## Boundary Rules",
        "## Data Ownership",
        "## Diagnostic Contract",
        "## Artifact Contract",
        "## Implementation Slices",
        "## Test Layout",
        "## Review Checklist",
        "## Readiness Impact",
    ]:
        require(pass_contracts, heading, pass_contracts_path)

    for needle in [
        "not bootstrap implementation",
        "lexer produces tokens with spans",
        "parser produces source-shaped AST",
        "resolver owns module paths",
        "semantic checking owns types",
        "typed IR carries resolved facts",
        "backend codegen emits target artifacts",
        "codegen stays mechanical",
        "SourceFile",
        "Token",
        "AstNode",
        "SymbolId",
        "HirNode",
        "TypeId",
        "OwnershipFact",
        "IrFunction",
        "LLVM text",
        "module graph dump",
        "typed fact dump",
        "ownership fact dump",
        "src/llvm_codegen.cpp",
        "src/ir.hpp",
        "38-42% ready",
        "58-62%",
    ]:
        require(pass_contracts, needle, pass_contracts_path)

    for heading in [
        "# Compiler Project Model",
        "## Goals",
        "## Current Module Contract",
        "## Recommended Project Layout",
        "## Module Ownership Rules",
        "## Header And Source Policy",
        "## Metadata And Cache Policy",
        "## Build Flow",
        "## Implementation Slices",
        "## Test Layout",
        "## Review Checklist",
        "## Readiness Impact",
    ]:
        require(project_model, heading, project_model_path)

    for needle in [
        "not bootstrap implementation",
        "file-backed modules",
        "package roots",
        "module search paths",
        "name.ari",
        "name.arih",
        "name/mod.ari",
        "name/mod.arih",
        "--module-path",
        "-I",
        ".arih",
        "--emit-module-metadata",
        "--check-module-metadata",
        "--emit-module-cache",
        "--use-module-cache",
        "module graph dump",
        "Makefile",
        "38-42% ready",
    ]:
        require(project_model, needle, project_model_path)

    for heading in [
        "# Compiler Source And Diagnostics",
        "## Goals",
        "## Package Shape",
        "## Core Types",
        "## Diagnostic Values",
        "## Rendering Policy",
        "## Error Code Policy",
        "## Ownership And Allocation",
        "## Implementation Slices",
        "## Test Layout",
        "## Integration With The Current Compiler",
        "## Readiness Impact",
    ]:
        require(source_diagnostics, heading, source_diagnostics_path)

    for needle in [
        "not bootstrap implementation",
        "Do not put these APIs into runtime `std`",
        "SourceId",
        "SourceFile",
        "Span",
        "DiagnosticCode",
        "Label",
        "Diagnostic",
        "stable golden rendering",
        "compiler/source",
        "compiler/report",
        "Span.start",
        "Span.end",
        "diagnostics should be data first",
        "normalize repository-local paths",
        "L0001 lexer",
        "explicit ownership",
        "source-id-stability",
        "report-single-label.ari",
    ]:
        require(source_diagnostics, needle, source_diagnostics_path)

    for heading in [
        "# Compiler Artifact Testing",
        "## Goals",
        "## Artifact Order",
        "## Artifact Formats",
        "## Normalization Rules",
        "## Golden File Policy",
        "## Focused Make Targets",
        "## First Implementation Slices",
        "## Current Seed Implementation",
        "## Current Compiler Integration",
        "## Review Checklist",
        "## Readiness Impact",
    ]:
        require(artifact_testing, heading, artifact_testing_path)

    for needle in [
        "compiler-development infrastructure",
        "Token dump",
        "Diagnostic dump",
        "Syntax dump",
        "HIR dump",
        "Typed IR dump",
        "LLVM text",
        "Object/shared symbols",
        "Executable behavior",
        "normalize repository-local paths",
        "Golden files are committed text outputs",
        "make check-compiler-artifacts",
        "make -C bootstrap check-lex",
        "Text comparator",
        "Path normalizer",
        "Token dump format",
        "ari --emit-tokens path",
        "ari --emit-syntax path",
        "ari --emit-diagnostics path",
        "ari --emit-typed-ir path",
        "`--emit-tokens` writes deterministic lexer output",
        "`--emit-syntax` writes deterministic parser output",
        "`--emit-diagnostics` writes a normalized diagnostic artifact",
        "`--emit-typed-ir` writes deterministic sema-lowered IR",
        "`--emit-tokens` for stable lexer token text",
        "`--emit-syntax` for stable parser tree text",
        "`--emit-diagnostics` for stable expected-failure text",
        "`--emit-typed-ir` for stable sema output",
        "token-dump-basic.ari",
        "token-dump-basic.tokens",
        "syntax-dump-basic.syntax",
        "typed-ir-basic.ir",
        "diagnostic-unexpected-character.ari",
        "diagnostic-unexpected-character.diagnostic",
        "Diagnostic dump format",
        "Syntax dump format",
        "HIR dump format",
        "Typed IR dump format",
        "LLVM normalizer",
        "tests/check_compiler_artifacts.py",
        "tests/cases/compiler-development/artifact/ok/",
        "tests/cases/compiler-development/artifact/errors/",
        "38-42% ready",
    ]:
        require(artifact_testing, needle, artifact_testing_path)

    gate_labels = {
        "frontend-grammar": "Frontend grammar",
        "contributor-guide": "Compiler Contributor Guide",
        "readiness-inventory": "Compiler Readiness Inventory",
        "source-identity": "Source identity",
        "diagnostics": "Diagnostics",
        "pass-contracts": "Compiler Pass Contracts",
        "source-diagnostics-layer": "Compiler Source And Diagnostics",
        "module-projects": "Module projects",
        "project-module-model": "Compiler Project Model",
        "generic-models": "Generic data models",
        "trait-selection": "Trait selection",
        "error-flow": "Error flow",
        "allocation-model": "Allocation model",
        "ir-contract": "IR contract",
        "backend-artifacts": "Backend artifacts",
        "tool-build-flow": "Tool build flow",
        "artifact-testing": "Compiler Artifact Testing",
        "stage-comparison": "Stage comparison",
    }
    for entry, label in gate_labels.items():
        require(manifest, entry, manifest_path)
        if entry == "contributor-guide":
            require(contributor, label, contributor_path)
        elif entry == "readiness-inventory":
            require(readiness, label, readiness_path)
        elif entry == "source-diagnostics-layer":
            require(source_diagnostics, label, source_diagnostics_path)
        elif entry == "pass-contracts":
            require(pass_contracts, label, pass_contracts_path)
        elif entry == "project-module-model":
            require(project_model, label, project_model_path)
        elif entry == "artifact-testing":
            require(artifact_testing, label, artifact_testing_path)
        else:
            require(gates, label, gates_path)

    for index_path in ["docs/README.md", "docs/dev/README.md"]:
        index = read(index_path)
        require(index, "Compiler Development Roadmap", index_path)
        require(index, "Compiler Contributor Guide", index_path)
        require(index, "Compiler Readiness Inventory", index_path)
        require(index, "Compiler Maturity Gates", index_path)
        require(index, "Compiler Pass Contracts", index_path)
        require(index, "Compiler Project Model", index_path)
        require(index, "Compiler Source And Diagnostics", index_path)
        require(index, "Compiler Artifact Testing", index_path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
