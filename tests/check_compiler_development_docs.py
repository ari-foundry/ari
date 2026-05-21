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
    dashboard_path = "docs/dev/compiler-development-dashboard.md"
    dashboard = read(dashboard_path)
    gates_path = "docs/dev/compiler-maturity-gates.md"
    gates = read(gates_path)
    contributor_path = "docs/dev/compiler-contributor-guide.md"
    contributor = read(contributor_path)
    glossary_path = "docs/dev/compiler-concepts-glossary.md"
    glossary = read(glossary_path)
    layer_map_path = "docs/dev/compiler-layer-map.md"
    layer_map = read(layer_map_path)
    triage_path = "docs/dev/compiler-triage-guide.md"
    triage = read(triage_path)
    source_identity_path = "docs/dev/compiler-source-identity.md"
    source_identity = read(source_identity_path)
    module_project_authoring_path = "docs/dev/compiler-module-project-authoring.md"
    module_project_authoring = read(module_project_authoring_path)
    artifact_authoring_path = "docs/dev/compiler-artifact-authoring.md"
    artifact_authoring = read(artifact_authoring_path)
    diagnostic_authoring_path = "docs/dev/compiler-diagnostic-authoring.md"
    diagnostic_authoring = read(diagnostic_authoring_path)
    test_authoring_path = "docs/dev/compiler-test-authoring.md"
    test_authoring = read(test_authoring_path)
    playbook_path = "docs/dev/compiler-implementation-playbook.md"
    playbook = read(playbook_path)
    next_slices_path = "docs/dev/compiler-next-slices.md"
    next_slices = read(next_slices_path)
    change_checklist_path = "docs/dev/compiler-change-checklist.md"
    change_checklist = read(change_checklist_path)
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
        "# Compiler Development Dashboard",
        "## Current Status",
        "## Read First",
        "## Do Now",
        "## Do Next",
        "## Not Yet",
        "## Small Checks",
        "## Test Classification",
        "## Compiler Development Gates",
        "## Handoff Note",
    ]:
        require(dashboard, heading, dashboard_path)

    for needle in [
        "not a bootstrap implementation plan",
        "hosted compiler and the public Ari language",
        "38-42% ready",
        "58-62% remaining",
        "40% ready",
        "Compiler Contributor Guide",
        "Compiler Development Roadmap",
        "Compiler Concepts Glossary",
        "Compiler Layer Map",
        "Compiler Triage Guide",
        "Compiler Source Identity",
        "Compiler Module Project Authoring",
        "Compiler Artifact Authoring",
        "Compiler Diagnostic Authoring",
        "Compiler Test Authoring",
        "Compiler Implementation Playbook",
        "Compiler Next Slices",
        "Compiler Change Checklist",
        "Compiler Readiness Inventory",
        "Getting Started",
        "Feature Status",
        "Feature Crosswalk",
        "Source identity",
        "Diagnostic data",
        "Artifact comparison",
        "File-backed project flow",
        "Generic compiler models",
        "Do not create a real `bootstrap/` tree yet",
        "Result[T, E]",
        "make check-language-docs",
        "make check-compiler-dev-docs",
        "make check-compiler-development",
        "make check-compiler-artifacts",
        "make check-bootstrap-docs",
        "build/ari path/to/file.ari --check",
        "Full `make check` belongs at handoff",
        "Sanitizer",
        "checks are intentionally separate",
        "tests/cases/compiler-development/ok/model/",
        "tests/cases/compiler-development/artifact/ok/",
        "tests/cases/compiler-development/artifact/errors/",
        "tests/cases/compiler-development/errors/",
        "Name each file by the behavior it protects",
        "Project flow",
        "Compiler data models",
        "Artifact order",
        "Developer loop",
    ]:
        require(dashboard, needle, dashboard_path)

    for heading in [
        "# Compiler Concepts Glossary",
        "## Reading Rule",
        "## Frontend Terms",
        "## Middle-End Terms",
        "## Backend Terms",
        "## Testing Terms",
        "## Layer Ownership",
        "## Review Vocabulary",
        "## Example Change Shape",
    ]:
        require(glossary, heading, glossary_path)

    for needle in [
        "not a bootstrap implementation plan",
        "hosted",
        "C++ compiler today",
        "Which compiler layer owns this value",
        "Which file or artifact proves it changed correctly",
        "Source file",
        "SourceId",
        "Byte span",
        "Token",
        "AST",
        "Declaration",
        "Name resolution",
        "Sema",
        "HIR",
        "Type fact",
        "Ownership fact",
        "Diagnostic",
        "Typed IR",
        "LLVM IR",
        "Object file",
        "Executable",
        "Shared library",
        "Symbol mangling",
        "Golden file",
        "Model fixture",
        "Focused check",
        "make check-compiler-development",
        "make check-compiler-artifacts",
        "Sanitizer checks",
        "intentionally separate",
        "source-level name lookup",
        "Artifact order",
        "Public language pressure",
        "self-hosting readiness as a secondary metric",
    ]:
        require(glossary, needle, glossary_path)

    for heading in [
        "# Compiler Layer Map",
        "## How To Use This Map",
        "## Source Files At A Glance",
        "## Layer Contracts",
        "## First Checks By Layer",
        "## Documentation Updates By Layer",
        "## Adding A New Layer",
        "## Review Checklist",
    ]:
        require(layer_map, heading, layer_map_path)

    for needle in [
        "ordinary hosted compiler development",
        "not bootstrap implementation",
        "earliest layer that can know the behavior",
        "Do not make LLVM codegen re-resolve Ari source-level names",
        "src/driver.cpp",
        "src/toolchain.cpp",
        "src/lexer.cpp",
        "src/token.hpp",
        "src/parser.cpp",
        "src/ast.hpp",
        "src/module_loader.cpp",
        "src/module_metadata.cpp",
        "src/sema.cpp",
        "src/type_semantics.cpp",
        "src/trait_semantics.cpp",
        "src/ownership_semantics.cpp",
        "src/borrow_semantics.cpp",
        "src/ir.hpp",
        "src/llvm_codegen.cpp",
        "src/symbol_mangle.cpp",
        "token dump",
        "syntax dump",
        "module graph",
        "declaration index",
        "typed IR",
        "future ownership fact dump",
        "LLVM, object, executable",
        "make check-cli",
        "make check-modules",
        "make check-generics",
        "make check-traits",
        "make check-compiler-development",
        "make check-compiler-artifacts",
        "make check-compiler-dev-docs",
        "Full `make check` belongs at handoff",
        "Sanitizer checks",
        "Feature Crosswalk",
        "Compiler Pass Contracts",
        "Compiler Module Project Authoring",
        "Compiler Artifact Authoring",
        "Compiler Diagnostic Authoring",
        "Compiler Test Authoring",
        "Compiler Readiness Inventory",
        "Good new layers make reviews smaller",
        "normal Ari compiler development rather than bootstrap-only",
    ]:
        require(layer_map, needle, layer_map_path)

    for heading in [
        "# Compiler Triage Guide",
        "## Triage Rule",
        "## Symptom Map",
        "## Error Placement",
        "## Triage Note Template",
        "## Escalation Rules",
        "## Review Checklist",
    ]:
        require(triage, heading, triage_path)

    for needle in [
        "ordinary hosted compiler development",
        "not bootstrap implementation",
        "Start with the earliest layer that can explain the symptom",
        "Executable output is the last signal",
        "source path -> tokens -> syntax -> module graph -> declaration index",
        "src/driver.cpp",
        "src/toolchain.cpp",
        "src/lexer.cpp",
        "src/token.hpp",
        "src/parser.cpp",
        "src/ast.hpp",
        "src/module_loader.cpp",
        "src/module_path.cpp",
        "src/module_metadata.cpp",
        "src/type_semantics.cpp",
        "src/type_inference.cpp",
        "src/trait_semantics.cpp",
        "src/ownership_semantics.cpp",
        "src/borrow_semantics.cpp",
        "src/move_semantics.cpp",
        "src/control_flow_semantics.cpp",
        "src/ir.hpp",
        "src/ir_builders.cpp",
        "src/llvm_codegen.cpp",
        "src/symbol_mangle.cpp",
        "make check-cli",
        "make check-modules",
        "make check-generics",
        "make check-traits",
        "make check-language-docs",
        "make check-compiler-dev-docs",
        "tests/cases/compiler-development/errors/",
        "tests/cases/compiler-development/artifact/ok/",
        "tests/cases/compiler-development/artifact/errors/",
        "tests/cases/compiler-development/ok/model/",
        "Non-goal",
        "bootstrap-only shortcut",
        "Sanitizer checks",
        "intentionally outside this triage loop",
    ]:
        require(triage, needle, triage_path)

    for heading in [
        "# Compiler Source Identity",
        "## Goals",
        "## Source Kinds",
        "## Stable SourceId Rules",
        "## Span Rules",
        "## Line And Column Lookup",
        "## Artifact Policy",
        "## Implementation Slices",
        "## Review Checklist",
    ]:
        require(source_identity, heading, source_identity_path)

    for needle in [
        "ordinary",
        "hosted-compiler development",
        "not bootstrap implementation",
        "Source identity is the base layer",
        "which files were loaded",
        "which `SourceId` belongs to each loaded source",
        "byte range a token, AST node, diagnostic label, or declaration covers",
        "File source",
        "Generated source",
        "Virtual source",
        "Every kind still receives a `SourceId`",
        "assign ids in deterministic load order",
        "keep the entry file first",
        "do not recycle ids during one invocation",
        "normalize repository-local paths",
        "Do not promise that a `SourceId` is stable across separate builds",
        "Span.start",
        "Span.end",
        "start <= end",
        "empty spans are allowed",
        "byte ranges, not Unicode scalar",
        "line starts at `1`",
        "column starts at `1`",
        "EOF spans",
        "--emit-source-map",
        "--emit-tokens",
        "--emit-syntax",
        "--emit-declaration-index",
        "--emit-module-graph",
        "--emit-diagnostics",
        "tests/cases/compiler-development/artifact/ok/",
        "tests/cases/compiler-development/artifact/errors/",
        "Source table",
        "Span validation",
        "Line table",
        "Snippet extraction",
        "Artifact normalization",
        "Multi-file coverage",
        "Diagnostic integration",
        "Which source kind does the change affect",
        "runtime `std`",
        "all Ari tools",
    ]:
        require(source_identity, needle, source_identity_path)

    for heading in [
        "# Compiler Module Project Authoring",
        "## Current Contract",
        "## Project Layout Rule",
        "## Module Ownership",
        "## Diagnostics",
        "## Metadata And Cache Inputs",
        "## Artifact Policy",
        "## Focused Checks",
        "## Test Placement",
        "## Review Checklist",
    ]:
        require(module_project_authoring, heading, module_project_authoring_path)

    for needle in [
        "ordinary hosted-compiler development",
        "not bootstrap implementation",
        "file-backed module and project behavior",
        "`mod name;` loads a file-backed child module",
        "The importing file's directory is searched first",
        "`-I path` and `--module-path path`",
        "name.ari",
        "name.arih",
        "name/mod.ari",
        "name/mod.arih",
        ".arih",
        "not yet an automatic pair",
        "Compiler Project Model",
        "Module Kind",
        "`source`",
        "`report`",
        "`lex`",
        "`parse`",
        "`resolve`",
        "`hir`",
        "`ir`",
        "`emit`",
        "importing source file",
        "ordered module search paths",
        "candidate files that were checked",
        "visibility boundary",
        "cache format version",
        "compiler format version",
        "target triple and ABI facts",
        "active cfg features",
        "source hashes",
        "Silent",
        "cache reuse is worse than no cache",
        "--emit-module-graph",
        "--emit-declaration-index",
        "--emit-module-metadata",
        "--emit-module-cache",
        "--use-module-cache",
        "--emit-diagnostics",
        "make check-modules",
        "make check-compiler-artifacts",
        "Full `make check` belongs at handoff",
        "Sanitizer checks",
        "intentionally separate",
        "tests/cases/modules/ok/",
        "tests/cases/modules/errors/",
        "tests/packages/",
        "tests/cases/compiler-development/artifact/ok/",
        "tests/cases/compiler-development/artifact/errors/",
        "tests/cases/compiler-development/ok/model/",
        "module-missing-candidates",
        "module-private-item",
        "module-graph-file-module",
        "module-cache-source-hash",
        "module-metadata-visibility",
        "Does the change help ordinary Ari projects",
        "stale cache behavior fail closed",
        "future self-hosting",
    ]:
        require(module_project_authoring, needle, module_project_authoring_path)

    for heading in [
        "# Compiler Artifact Authoring",
        "## Artifact Design Rule",
        "## Artifact Order",
        "## Format Rules",
        "## Normalization Policy",
        "## Golden Update Rule",
        "## Test Placement",
        "## Focused Checks",
        "## Review Checklist",
    ]:
        require(artifact_authoring, heading, artifact_authoring_path)

    for needle in [
        "ordinary hosted-compiler development",
        "not bootstrap implementation",
        "Artifacts are the compiler's review trail",
        "What is the earliest compiler layer",
        "not rely only on a linked executable",
        "--emit-stage-plan",
        "--emit-capability-inventory",
        "--emit-source-map",
        "--emit-tokens",
        "--emit-diagnostics",
        "--emit-diagnostic-catalog",
        "--emit-syntax",
        "--emit-module-graph",
        "--emit-declaration-index",
        "Future HIR dump",
        "--emit-typed-ir",
        "--emit-pass-summary",
        "--emit-llvm",
        "Object/shared symbol checks",
        "Executable behavior",
        "line-oriented",
        "deterministic",
        "small enough to review",
        "Repository root",
        "<repo>",
        "Build directory",
        "<build>",
        "Temporary names",
        "<tmpN>",
        "Do not normalize meaningful language facts",
        "Golden files are committed expected outputs",
        "Do not auto-update goldens",
        "Does CLI misuse name the conflicting `--emit-*` flags",
        "tests/cases/compiler-development/artifact/ok/",
        "tests/cases/compiler-development/artifact/errors/",
        "tests/cases/compiler-development/ok/model/",
        "source-map-file-module.map",
        "token-dump-basic.tokens",
        "syntax-dump-basic.syntax",
        "module-graph-file-module.graph",
        "declaration-index-basic.decls",
        "capability-inventory.inventory",
        "diagnostic-catalog.catalog",
        "stage-plan-basic.plan",
        "typed-ir-basic.ir",
        "diagnostic-parser-expected.diagnostic",
        "python3 tests/check_compiler_artifacts.py expected actual",
        "make check-compiler-artifacts",
        "make check-compiler-dev-docs",
        "python3 tests/check_compiler_artifact_cli.py",
        "Full `make check` belongs at handoff",
        "Sanitizer checks",
        "intentionally separate",
        "Is this the earliest artifact that can prove the behavior",
        "executable behavior used only after earlier artifacts are stable",
        "ordinary compiler development",
        "not a bootstrap-only path",
    ]:
        require(artifact_authoring, needle, artifact_authoring_path)

    for heading in [
        "# Compiler Diagnostic Authoring",
        "## When To Add A Diagnostic",
        "## Diagnostic Shape",
        "## Message Style",
        "## Source Spans And Labels",
        "## Notes And Help",
        "## Codes And Families",
        "## Golden Tests",
        "## Review Checklist",
    ]:
        require(diagnostic_authoring, heading, diagnostic_authoring_path)

    for needle in [
        "ordinary hosted-compiler development",
        "not bootstrap implementation",
        "Diagnostics are part of the language design",
        "unsupported syntax is currently accepted too far into the pipeline",
        "ari/compiler",
        "Code",
        "Family",
        "Severity",
        "Source",
        "Line and column",
        "Primary label",
        "Secondary labels",
        "Notes",
        "Help",
        "expected expression after '='",
        "cannot move 'value' while it is borrowed",
        "module 'math' was not found on the module path",
        "trait 'Score' is not implemented for Point",
        "Span.start",
        "Span.end",
        "one primary label",
        "Parser expected expression",
        "Unknown module",
        "Borrow conflict",
        "Trait method missing",
        "Backend artifact failure",
        "try adding ';' after this statement",
        "pass -I path or --module-path path",
        "L0001",
        "P0001",
        "M0001",
        "T0001",
        "O0001",
        "I0001",
        "B0001",
        "code=P0001 family=parser source=\"source.ari\" line=3 column=1",
        "stable search key",
        "which compiler layer",
        "--emit-diagnostic-catalog",
        "compiler-owned list of current codes",
        "do not reuse it for a different",
        "rule",
        "--emit-diagnostics",
        "--emit-module-graph",
        "--emit-typed-ir",
        "--emit-llvm",
        "tests/cases/compiler-development/artifact/errors/",
        "make check-compiler-artifacts",
        "make check-compiler-dev-docs",
        "Full `make check` belongs at handoff",
        "Sanitizer checks",
        "intentionally separate",
        "Which layer detected the error first",
        "Does the message describe the Ari rule in user language",
        "Good diagnostics make Ari easier to use today",
        "without introducing bootstrap-only behavior",
    ]:
        require(diagnostic_authoring, needle, diagnostic_authoring_path)

    for heading in [
        "# Compiler Test Authoring",
        "## Choose The Test Bucket",
        "## Name The File",
        "## Pick The Small Check",
        "## Expected Results",
        "## Comments In Tests",
        "## Artifact Update Rule",
        "## Review Checklist",
    ]:
        require(test_authoring, heading, test_authoring_path)

    for needle in [
        "not a bootstrap implementation plan",
        "ordinary",
        "compiler development testable",
        "Pick the bucket by behavior",
        "tests/cases/<feature>/ok/",
        "tests/cases/<feature>/errors/",
        "tests/cases/compiler-development/ok/model/",
        "tests/cases/compiler-development/artifact/ok/",
        "tests/cases/compiler-development/artifact/errors/",
        "tests/cases/compiler-development/errors/",
        "choose the one closest to the first layer",
        "can fail",
        "Use behavior names",
        "compiler-test-classification.ari",
        "bootstrap-class-keyword.ari",
        "diagnostic-parser-expected.diagnostic",
        "module-graph-file-module.graph",
        "typed-ir-basic.ir",
        "build/ari path/to/case.ari --check",
        "make check-compiler-development",
        "make check-compiler-artifacts",
        "make check-compiler-dev-docs",
        "make check-language-docs",
        "Full `make check` belongs at handoff",
        "Sanitizer checks",
        "intentionally separate",
        "expected exit code in `tests/Makefile`",
        "not return a magic constant",
        "Artifact tests should make compiler stages reviewable",
        "Which compiler layer owns this behavior",
        "Does the filename name the behavior",
        "later bootstrap",
        "start gate",
    ]:
        require(test_authoring, needle, test_authoring_path)

    for heading in [
        "# Compiler Development Roadmap",
        "## Current Position",
        "## Development Principles",
        "## Immediate Compiler Work Queue",
        "## Concrete Implementation Backlog",
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
        "Compiler Development Dashboard",
        "Compiler Maturity Gates",
        "Compiler Contributor Guide",
        "Compiler Concepts Glossary",
        "Compiler Source Identity",
        "Compiler Module Project Authoring",
        "Compiler Artifact Authoring",
        "Compiler Diagnostic Authoring",
        "Compiler Test Authoring",
        "Compiler Implementation Playbook",
        "Compiler Next Slices",
        "Compiler Change Checklist",
        "Compiler Readiness Inventory",
        "Compiler Pass Contracts",
        "Compiler Project Model",
        "Compiler Source And Diagnostics",
        "Compiler Artifact Testing",
        "hosted-compiler changes",
        "Source identity | Stable source ownership",
        "Test classification",
        "Readiness scorecard",
        "tests/cases/compiler-development/ok/model/compiler-stage-gates.ari",
        "tests/cases/compiler-development/ok/model/compiler-readiness-scorecard.ari",
        "tests/cases/compiler-development/ok/model/compiler-development-dashboard.ari",
        "tests/cases/compiler-development/ok/model/compiler-concepts-glossary.ari",
        "tests/cases/compiler-development/ok/model/compiler-layer-map.ari",
        "tests/cases/compiler-development/ok/model/compiler-triage-guide.ari",
        "tests/cases/compiler-development/ok/model/compiler-source-identity.ari",
        "tests/cases/compiler-development/ok/model/compiler-module-project-authoring.ari",
        "tests/cases/compiler-development/ok/model/compiler-artifact-authoring.ari",
        "tests/cases/compiler-development/ok/model/compiler-diagnostic-authoring.ari",
        "tests/cases/compiler-development/ok/model/compiler-test-authoring.ari",
        "Do not solve those problems with",
        "`--emit-capability-inventory`, `--emit-source-map`, `--emit-tokens`,",
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
        "# Compiler Implementation Playbook",
        "## Work Selection",
        "## Implementation Slices",
        "## Ticket Template",
        "## Test Placement",
        "## Review Checklist",
        "## Readiness Impact",
    ]:
        require(playbook, heading, playbook_path)

    for needle in [
        "not a bootstrap plan",
        "which source files should I read first",
        "Source identity",
        "Diagnostics",
        "Lexer/parser surface",
        "Module flow",
        "Sema boundary",
        "Generic data models",
        "IR contract",
        "Backend artifact",
        "Problem:",
        "Non-goals:",
        "tests/cases/compiler-development/ok/model/",
        "tests/cases/compiler-development/artifact/ok/",
        "tests/cases/compiler-development/artifact/errors/",
        "No bootstrap-only keyword",
        "38-42% ready",
    ]:
        require(playbook, needle, playbook_path)

    for heading in [
        "# Compiler Next Slices",
        "## Selection Rules",
        "## Next Slices",
        "## Ticket Breakdown",
        "## Not Ready Yet",
        "## Readiness Impact",
    ]:
        require(next_slices, heading, next_slices_path)

    for needle in [
        "not about implementing bootstrapping",
        "Source span edge cases",
        "Diagnostic code expansion",
        "Module metadata reviewability",
        "Compiler-shaped aggregate pressure",
        "HIR artifact sketch",
        "IR metadata audit",
        "make check-compiler-artifacts",
        "compiler-source-diagnostics.md",
        "a real `bootstrap/` tree",
        "38-42% ready",
    ]:
        require(next_slices, needle, next_slices_path)

    for heading in [
        "# Compiler Change Checklist",
        "## Fast Checklist",
        "## Test Choice",
        "## Documentation Update Rules",
        "## Review Notes Template",
        "## Readiness Impact",
    ]:
        require(change_checklist, heading, change_checklist_path)

    for needle in [
        "not a bootstrap checklist",
        "Scope",
        "Docs",
        "Tests",
        "Diagnostics",
        "Sema",
        "IR",
        "Non-goals",
        "make check-language-docs",
        "make check-compiler-development",
        "make check-compiler-artifacts",
        "Full `make check` belongs at handoff",
        "No runtime `std` API",
        "38-42% ready",
    ]:
        require(change_checklist, needle, change_checklist_path)

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
        "make check-language-docs",
        "tests/cases/compiler-development/ok/model/",
        "tests/cases/compiler-development/artifact/ok/",
        "bootstrap-readiness",
        "Result[T, E]",
        "38-42% ready",
        "58-62% remaining",
    ]:
        require(contributor, needle, contributor_path)

    for heading in [
        "# Compiler Readiness Inventory",
        "## Current Readiness",
        "## Readiness Scorecard",
        "## Already Strong",
        "## Blocking Gaps",
        "## Recent Compiler Support",
        "## Development Backlog",
        "## Compiler Development Gates",
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
        "weighted engineering score",
        "tests/cases/compiler-development/ok/model/compiler-readiness-scorecard.ari",
        "Weighted together, this lands at roughly 40%",
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
        "tests/cases/compiler-development/ok/model/compiler-diagnostic-authoring.ari",
        "tests/cases/compiler-development/ok/model/compiler-development-dashboard.ari",
        "tests/cases/compiler-development/ok/model/compiler-concepts-glossary.ari",
        "tests/cases/compiler-development/ok/model/compiler-source-identity.ari",
        "tests/cases/compiler-development/ok/model/compiler-module-project-authoring.ari",
        "tests/cases/compiler-development/ok/model/compiler-artifact-authoring.ari",
        "tests/cases/compiler-development/ok/model/compiler-test-authoring.ari",
        "tests/cases/compiler-development/ok/model/compiler-change-checklist.ari",
        "tests/cases/compiler-development/ok/model/compiler-source-map-workflow.ari",
        "tests/cases/compiler-development/ok/model/compiler-implementation-slices.ari",
        "tests/cases/compiler-development/ok/model/compiler-next-slices.ari",
        "tests/cases/compiler-development/ok/model/compiler-stage-gates.ari",
        "tests/cases/compiler-development/ok/model/compiler-readiness-scorecard.ari",
        "tests/cases/compiler-development/ok/model/compiler-test-classification.ari",
        "tests/cases/compiler-development/ok/model/compiler-doc-crosswalk.ari",
        "tests/cases/compiler-development/errors/bootstrap-class-keyword.ari",
        "tests/cases/compiler-development/errors/bootstrap-interface-keyword.ari",
        "tests/cases/compiler-development/artifact/ok/source-map-file-module.map",
        "tests/cases/compiler-development/artifact/ok/token-dump-basic.ari",
        "tests/cases/compiler-development/artifact/ok/declaration-index-basic.decls",
        "tests/cases/compiler-development/artifact/ok/syntax-dump-basic.syntax",
        "tests/cases/compiler-development/artifact/ok/module-graph-file-module.graph",
        "tests/cases/compiler-development/artifact/ok/stage-plan-basic.plan",
        "tests/cases/compiler-development/artifact/ok/capability-inventory.inventory",
        "tests/cases/compiler-development/artifact/ok/diagnostic-catalog.catalog",
        "tests/cases/compiler-development/artifact/ok/typed-ir-basic.ir",
        "tests/cases/compiler-development/artifact/ok/pass-summary-basic.summary",
        "tests/cases/compiler-development/artifact/errors/diagnostic-parser-expected.diagnostic",
        "tests/cases/compiler-development/artifact/errors/diagnostic-missing-module.diagnostic",
        "tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.diagnostic",
        "tests/cases/compiler-development/artifact/errors/diagnostic-unknown-trait.diagnostic",
        "tests/cases/compiler-development/artifact/errors/diagnostic-borrow-conflict.diagnostic",
        "--emit-source-map path",
        "--emit-tokens path",
        "--emit-syntax path",
        "--emit-diagnostics path",
        "--emit-capability-inventory path",
        "--emit-diagnostic-catalog path",
        "--emit-module-graph path",
        "--emit-declaration-index path",
        "--emit-stage-plan path",
        "--emit-typed-ir path",
        "--emit-pass-summary path",
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
        "Declaration index dump",
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
        "Compiler Module Project Authoring",
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
        "Compiler Source Identity",
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
        "source-map-file-module.map",
        "report-single-label.ari",
        "classify_diagnostic_code",
        "diagnostic_code_family",
        "family=parser",
        "--emit-diagnostic-catalog",
        "current code, family, owner",
        "source=",
        "line=",
        "column=",
        "--emit-source-map",
        "Unknown messages keep the fallback `ari/compiler`",
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
        "Compiler Artifact Authoring",
        "Source map dump",
        "Stage plan",
        "Capability inventory",
        "Token dump",
        "Diagnostic dump",
        "Diagnostic catalog",
        "Syntax dump",
        "Declaration index dump",
        "HIR dump",
        "Typed IR dump",
        "Pass summary",
        "LLVM text",
        "Object/shared symbols",
        "Executable behavior",
        "normalize repository-local paths",
        "Golden files are committed text outputs",
        "make check-compiler-artifacts",
        "make -C bootstrap check-lex",
        "Text comparator",
        "Path normalizer",
        "Source map dump format",
        "Token dump format",
        "Declaration index dump format",
        "ari --emit-source-map path",
        "ari --emit-capability-inventory path",
        "ari --emit-tokens path",
        "ari --emit-syntax path",
        "ari --emit-diagnostics path",
        "ari --emit-diagnostic-catalog path",
        "ari --emit-module-graph path",
        "ari --emit-declaration-index path",
        "ari --emit-stage-plan path",
        "ari --emit-pass-summary path",
        "ari --emit-typed-ir path",
        "`--emit-source-map` writes deterministic source file",
        "`--emit-stage-plan` writes deterministic artifact order",
        "`--emit-capability-inventory` writes the compiler's implemented",
        "artifact CLI misuse names the exact conflicting artifact options",
        "`--emit-tokens, --emit-syntax`",
        "`--emit-tokens` writes deterministic lexer output",
        "`--emit-syntax` writes deterministic parser output",
        "`--emit-diagnostics` writes a normalized diagnostic artifact",
        "`--emit-diagnostics` classifies representative lexer, parser, module, type",
        "`family=...` layer names",
        "source=`, `line=`, and `column=` fields",
        "`--emit-diagnostic-catalog` writes the current diagnostic code table",
        "`--emit-module-graph` writes deterministic file-backed source",
        "`--emit-declaration-index` writes deterministic declaration signatures",
        "`--emit-typed-ir` writes deterministic sema-lowered IR",
        "`--emit-pass-summary` writes deterministic stage counts",
        "`--emit-source-map` for stable byte offset",
        "`--emit-stage-plan` for stable stage-order",
        "`--emit-capability-inventory` for stable public compiler feature status",
        "`--emit-tokens` for stable lexer token text",
        "`--emit-syntax` for stable parser tree text",
        "`--emit-diagnostics` for stable expected-failure text",
        "`--emit-diagnostic-catalog` for stable diagnostic code ownership",
        "`--emit-module-graph` for stable file-backed source",
        "`--emit-declaration-index` for stable declaration signatures",
        "`--emit-typed-ir` for stable sema output",
        "`--emit-pass-summary` for quick stage-boundary counts",
        "source-map-file-module.map",
        "declaration-index-basic.decls",
        "token-dump-basic.ari",
        "token-dump-basic.tokens",
        "module-graph-file-module.graph",
        "capability-inventory.inventory",
        "diagnostic-catalog.catalog",
        "stage-plan-basic.plan",
        "pass-summary-basic.summary",
        "syntax-dump-basic.syntax",
        "typed-ir-basic.ir",
        "diagnostic-unexpected-character.ari",
        "diagnostic-unexpected-character.diagnostic",
        "diagnostic-parser-expected.diagnostic",
        "diagnostic-missing-module.diagnostic",
        "diagnostic-unknown-trait.diagnostic",
        "diagnostic-borrow-conflict.diagnostic",
        "Diagnostic dump format",
        "Syntax dump format",
        "HIR dump format",
        "Typed IR dump format",
        "LLVM normalizer",
        "tests/check_compiler_artifacts.py",
        "tests/check_compiler_artifact_cli.py",
        "tests/cases/compiler-development/artifact/ok/",
        "tests/cases/compiler-development/artifact/errors/",
        "38-42% ready",
    ]:
        require(artifact_testing, needle, artifact_testing_path)

    gate_labels = {
        "frontend-grammar": "Frontend grammar",
        "development-dashboard": "Compiler Development Dashboard",
        "contributor-guide": "Compiler Contributor Guide",
        "concepts-glossary": "Compiler Concepts Glossary",
        "layer-map": "Compiler Layer Map",
        "triage-guide": "Compiler Triage Guide",
        "source-identity-authoring": "Compiler Source Identity",
        "module-project-authoring": "Compiler Module Project Authoring",
        "artifact-authoring": "Compiler Artifact Authoring",
        "diagnostic-authoring": "Compiler Diagnostic Authoring",
        "test-authoring": "Compiler Test Authoring",
        "implementation-playbook": "Compiler Implementation Playbook",
        "next-slices": "Compiler Next Slices",
        "change-checklist": "Compiler Change Checklist",
        "readiness-inventory": "Compiler Readiness Inventory",
        "readiness-scorecard": "Readiness Scorecard",
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
        "test-classification": "Test Classification",
        "doc-crosswalk": "language docs-to-tests navigation",
        "policy-errors": "class/interface bootstrap-only syntax stays rejected",
        "stage-comparison": "Stage comparison",
    }
    for entry, label in gate_labels.items():
        require(manifest, entry, manifest_path)
        if entry == "development-dashboard":
            require(dashboard, label, dashboard_path)
        elif entry == "contributor-guide":
            require(contributor, label, contributor_path)
        elif entry == "concepts-glossary":
            require(glossary, label, glossary_path)
        elif entry == "layer-map":
            require(layer_map, label, layer_map_path)
        elif entry == "triage-guide":
            require(triage, label, triage_path)
        elif entry == "source-identity-authoring":
            require(source_identity, label, source_identity_path)
        elif entry == "module-project-authoring":
            require(module_project_authoring, label, module_project_authoring_path)
        elif entry == "artifact-authoring":
            require(artifact_authoring, label, artifact_authoring_path)
        elif entry == "diagnostic-authoring":
            require(diagnostic_authoring, label, diagnostic_authoring_path)
        elif entry == "test-authoring":
            require(test_authoring, label, test_authoring_path)
        elif entry == "implementation-playbook":
            require(playbook, label, playbook_path)
        elif entry == "next-slices":
            require(next_slices, label, next_slices_path)
        elif entry == "change-checklist":
            require(change_checklist, label, change_checklist_path)
        elif entry == "readiness-inventory":
            require(readiness, label, readiness_path)
        elif entry == "readiness-scorecard":
            require(readiness, label, readiness_path)
        elif entry == "source-diagnostics-layer":
            require(source_diagnostics, label, source_diagnostics_path)
        elif entry == "pass-contracts":
            require(pass_contracts, label, pass_contracts_path)
        elif entry == "project-module-model":
            require(project_model, label, project_model_path)
        elif entry == "artifact-testing":
            require(artifact_testing, label, artifact_testing_path)
        elif entry == "doc-crosswalk":
            require(readiness, label, readiness_path)
        elif entry == "policy-errors":
            require(readiness, label, readiness_path)
        else:
            require(gates, label, gates_path)

    for index_path in ["docs/README.md", "docs/dev/README.md"]:
        index = read(index_path)
        require(index, "Compiler Development Dashboard", index_path)
        require(index, "Compiler Development Roadmap", index_path)
        require(index, "Compiler Contributor Guide", index_path)
        require(index, "Compiler Concepts Glossary", index_path)
        require(index, "Compiler Layer Map", index_path)
        require(index, "Compiler Triage Guide", index_path)
        require(index, "Compiler Source Identity", index_path)
        require(index, "Compiler Module Project Authoring", index_path)
        require(index, "Compiler Artifact Authoring", index_path)
        require(index, "Compiler Diagnostic Authoring", index_path)
        require(index, "Compiler Test Authoring", index_path)
        require(index, "Compiler Implementation Playbook", index_path)
        require(index, "Compiler Next Slices", index_path)
        require(index, "Compiler Change Checklist", index_path)
        require(index, "Compiler Readiness Inventory", index_path)
        require(index, "Compiler Maturity Gates", index_path)
        require(index, "Compiler Pass Contracts", index_path)
        require(index, "Compiler Project Model", index_path)
        require(index, "Compiler Source And Diagnostics", index_path)
        require(index, "Compiler Artifact Testing", index_path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
