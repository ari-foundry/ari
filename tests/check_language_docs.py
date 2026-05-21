#!/usr/bin/env python3
"""Keep user-facing Ari docs usable as a docs-only entry point.

This check is intentionally small. It does not validate the whole language
reference; it prevents the beginner path, quick reference, cookbook, and test
navigation from losing the sections a first-time Ari user or contributor needs.
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


def main() -> int:
    docs_index_path = "docs/README.md"
    docs_index = read(docs_index_path)
    for needle in [
        "New Ari user",
        "New compiler contributor",
        "[Getting Started](language/getting-started.md)",
        "[Language Tour](language/language-tour.md)",
        "[Quick Reference](language/quick-reference.md)",
        "[Cookbook](language/cookbook.md)",
        "[Feature Status](language/feature-status.md)",
        "[Feature Crosswalk](language/feature-crosswalk.md)",
        "[Examples And Tests](language/examples-and-tests.md)",
        "[Compiler Contributor Guide](dev/compiler-contributor-guide.md)",
    ]:
        require(docs_index, needle, docs_index_path)

    language_index_path = "docs/language/README.md"
    language_index = read(language_index_path)
    for needle in [
        "## First-Hour Path",
        "## Docs-Only Workflow",
        "[Getting Started](getting-started.md)",
        "[Language Tour](language-tour.md)",
        "[Quick Reference](quick-reference.md)",
        "[Cookbook](cookbook.md)",
        "[Feature Status](feature-status.md)",
        "[Feature Crosswalk](feature-crosswalk.md)",
        "[Examples And Tests](examples-and-tests.md)",
        "[Front-End Only Syntax](front-end-only.md)",
    ]:
        require(language_index, needle, language_index_path)

    getting_started_path = "docs/language/getting-started.md"
    getting_started = read(getting_started_path)
    for heading in [
        "# Getting Started",
        "## Mental Model",
        "## Build The Compiler",
        "## Write A First Program",
        "## Use The Fast Check Loop",
        "## Add Values And Control Flow",
        "## Split Code Into A Module",
        "## CLI Reference",
        "## Runtime Artifacts",
        "## Common First Errors",
        "## Next Reading",
    ]:
        require(getting_started, heading, getting_started_path)
    for needle in [
        "fn main() -> i64",
        "There is no `class` or `interface` keyword",
        "./build/ari hello.ari --check",
        "mod math;",
        "--target-info",
        "--explain-artifact",
        "--list-passes",
        "--explain-pass",
        "--list-test-buckets",
        "--explain-test-bucket",
        "--list-work-items",
        "--explain-work-item",
        "--list-capabilities",
        "--explain-capability",
        "--emit-llvm",
        "--emit-obj",
        "--shared",
        "front-end-only",
        "Language Tour",
    ]:
        require(getting_started, needle, getting_started_path)

    language_tour_path = "docs/language/language-tour.md"
    language_tour = read(language_tour_path)
    for heading in [
        "# Language Tour",
        "## One-File Program",
        "## Values And Control Flow",
        "## Functions And Generics",
        "## Structs, Enums, And Match",
        "## Traits And Methods",
        "## Modules And Names",
        "## Ownership And Allocation",
        "## Tests As Examples",
        "## What To Avoid",
        "## Next Steps",
    ]:
        require(language_tour, heading, language_tour_path)
    for needle in [
        "examples/language-tour.ari",
        "./build/ari examples/language-tour.ari --check",
        "fn main() -> i64",
        "No local shadowing",
        "fn identity[T](value: T) -> T",
        "struct Point",
        "enum ScoreResult",
        "match classify(value)",
        "Ari has `trait`; it does not have `class` or `interface`",
        "mod Math",
        "Math::double(base)",
        "std::vec::new<i64>",
        "tests/cases/compiler-development/ok/model/",
        "tests/cases/compiler-development/artifact/ok/",
        "Result[T, E]",
        "bootstrap-only syntax",
    ]:
        require(language_tour, needle, language_tour_path)

    quick_reference_path = "docs/language/quick-reference.md"
    quick_reference = read(quick_reference_path)
    for needle in [
        "## Mental Model",
        "## Common Syntax",
        "## Types At A Glance",
        "## Prelude And Standard Library Choices",
        "## Ownership And Borrowing Checklist",
        "## Current Gotchas",
        "No local shadowing",
        "There is no `class` or `interface` keyword",
    ]:
        require(quick_reference, needle, quick_reference_path)

    cookbook_path = "docs/language/cookbook.md"
    cookbook = read(cookbook_path)
    for needle in [
        "## Minimal Program",
        "## Use Result-Style Early Return",
        "## Iterate Ranges And Local Vectors",
        "## Allocate With A Zone",
        "## Split Code Into Modules",
        "## Write Tests",
        "## Before You Blame The Compiler",
    ]:
        require(cookbook, needle, cookbook_path)

    feature_status_path = "docs/language/feature-status.md"
    feature_status = read(feature_status_path)
    for heading in [
        "# Feature Status",
        "## Executable Core",
        "## Data Modeling",
        "## Names And Modules",
        "## Traits And Methods",
        "## Memory And Runtime Boundaries",
        "## Tooling And Compiler Development",
        "## How To Use This Page",
    ]:
        require(feature_status, heading, feature_status_path)
    for needle in [
        "Feature Crosswalk",
        "implemented",
        "partial",
        "front-end only",
        "planned",
        "tests/cases/functions/",
        "tests/cases/modules/",
        "tests/cases/compiler-development/ok/model/",
        "make check-compiler-development",
        "Bootstrap Readiness",
    ]:
        require(feature_status, needle, feature_status_path)

    feature_crosswalk_path = "docs/language/feature-crosswalk.md"
    feature_crosswalk = read(feature_crosswalk_path)
    for heading in [
        "# Feature Crosswalk",
        "## How To Read A Row",
        "## Executable Program Surface",
        "## Data And Names",
        "## Traits, Memory, And Boundaries",
        "## Compiler Development Surface",
        "## When Docs And Behavior Differ",
    ]:
        require(feature_crosswalk, heading, feature_crosswalk_path)
    for needle in [
        "docs-only usability",
        "Read",
        "Example",
        "Tests",
        "Small check",
        "make check-functions",
        "make check-variables",
        "make check-control-flow",
        "make check-modules",
        "make check-traits",
        "make check-ffi",
        "make check-compiler-development",
        "make check-compiler-artifacts",
        "tests/cases/compiler-development/ok/model/",
        "tests/cases/compiler-development/artifact/ok/",
        "tests/cases/compiler-development/artifact/errors/",
        "compiler-readiness-scorecard.ari",
        "Feature Status",
        "Do not add bootstrap-only syntax",
    ]:
        require(feature_crosswalk, needle, feature_crosswalk_path)

    examples_tests_path = "docs/language/examples-and-tests.md"
    examples_tests = read(examples_tests_path)
    for heading in [
        "# Examples And Tests",
        "## Example Programs",
        "## Test Families",
        "## Reading A Test File",
        "## Small Check Workflow",
    ]:
        require(examples_tests, heading, examples_tests_path)
    for needle in [
        "examples/hello.ari",
        "examples/language-tour.ari",
        "Language Tour",
        "make check-examples",
        "tests/cases/functions/",
        "tests/cases/compiler-development/",
        "make check-compiler-artifacts",
        "Full `make check` is for handoff",
    ]:
        require(examples_tests, needle, examples_tests_path)

    tests_index_path = "tests/README.md"
    tests_index = read(tests_index_path)
    for needle in [
        "## Focused Targets",
        "make check-language-docs",
        "make check-compiler-development",
        "tests/cases/compiler-development/ok/model/",
        "tests/cases/compiler-development/artifact/ok/",
        "compiler-readiness-scorecard",
        "compiler-development-dashboard",
        "compiler-test-classification",
        "tests/check_compiler_capability_cli.py",
        "tests/check_compiler_pass_cli.py",
        "tests/check_compiler_test_bucket_cli.py",
        "tests/check_compiler_work_item_cli.py",
    ]:
        require(tests_index, needle, tests_index_path)

    language_tour_example_path = "examples/language-tour.ari"
    language_tour_example = read(language_tour_example_path)
    for needle in [
        "mod Math",
        "trait Score",
        "struct Point",
        "enum ScoreResult",
        "fn identity[T](value: T) -> T",
        "impl Score for Point",
        "for value in 0..limit",
        "match classify(value)",
        "println(\"tour={}\", total)",
    ]:
        require(language_tour_example, needle, language_tour_example_path)

    cases_index_path = "tests/cases/README.md"
    cases_index = read(cases_index_path)
    for needle in [
        "## Naming Rules",
        "`ok/`",
        "`errors/`",
        "`artifact/ok`",
        "compiler-test-classification.ari",
    ]:
        require(cases_index, needle, cases_index_path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
