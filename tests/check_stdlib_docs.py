#!/usr/bin/env python3
"""Keep standard library production-readiness docs actionable."""

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
MODULE_GUIDE_EXCEPTIONS = {
    "option": "option-result.md",
    "result": "option-result.md",
}


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def require(text: str, needle: str, path: str) -> None:
    if needle not in text:
        print(f"{path}: missing {needle!r}", file=sys.stderr)
        raise SystemExit(1)


def reject(text: str, needle: str, path: str) -> None:
    if needle in text:
        print(f"{path}: unexpected {needle!r}", file=sys.stderr)
        raise SystemExit(1)


def main() -> int:
    readiness_path = "docs/stdlib/production-readiness.md"
    readiness = read(readiness_path)
    development_path = "docs/stdlib/library-development.md"
    development = read(development_path)
    testing_path = "docs/stdlib/testing.md"
    testing = read(testing_path)
    roadmap_path = "docs/stdlib/roadmap.md"
    roadmap = read(roadmap_path)
    readme_path = "docs/stdlib/README.md"
    readme = read(readme_path)
    tests_makefile_path = "tests/Makefile"
    tests_makefile = read(tests_makefile_path)
    lib_makefile_path = "lib/Makefile"
    lib_makefile = read(lib_makefile_path)
    modules_readme_path = "docs/stdlib/modules/README.md"
    modules_readme = read(modules_readme_path)

    for heading in [
        "# Standard Library Production Readiness",
        "## Release Bar",
        "## Stability Levels",
        "## Module Tier Policy",
        "## Failure Policy",
        "## Platform Policy",
        "## API Acceptance Checklist",
        "## Compatibility And Deprecation",
        "## Non-Goals For Runtime Std",
        "## Current Priorities",
        "## Module Status Snapshot",
    ]:
        require(readiness, heading, readiness_path)

    for needle in [
        "natural API",
        "Option",
        "Result",
        "std::error::Error",
        "Zone",
        "core",
        "alloc",
        "hosted",
        "platform",
        "experimental",
        "make check-std-api",
        "make check-stdlib-docs",
        "compiler source maps",
        "source locations",
        "bootstrap-only helpers",
        "hidden global heaps",
        "raw syscall collections",
        "Compatibility wrappers",
    ]:
        require(readiness, needle, readiness_path)

    if not re.search(r"\|\s*Tier\s*\|\s*Modules\s*\|\s*Stability Expectation\s*\|", readiness):
        print(f"{readiness_path}: missing module tier table", file=sys.stderr)
        return 1
    if not re.search(r"\|\s*Family\s*\|\s*Current Level\s*\|\s*Next Hardening Slice\s*\|", readiness):
        print(f"{readiness_path}: missing module status table", file=sys.stderr)
        return 1

    for needle in [
        "make check-stdlib-docs",
        "Stability Review",
        "source maps",
        "source locations",
        "runtime `std`",
        "only a future compiler would use",
    ]:
        require(development, needle, development_path)

    require(testing, "Docs readiness", testing_path)
    require(testing, "make check-stdlib-docs", testing_path)
    require(readme, "module tiers", readme_path)
    require(readme, "failure policy", readme_path)
    require(readme, "non-goals", readme_path)
    require(modules_readme, "Guide Shape", modules_readme_path)

    require(tests_makefile, "check-stdlib-docs", tests_makefile_path)
    require(tests_makefile, "python3 tests/check_stdlib_docs.py", tests_makefile_path)
    require(lib_makefile, "check-docs", lib_makefile_path)
    require(lib_makefile, "tests/check_stdlib_docs.py", lib_makefile_path)

    std_modules = sorted(path.stem for path in (ROOT / "lib" / "std").glob("*.arih"))
    for module in std_modules:
        guide_name = MODULE_GUIDE_EXCEPTIONS.get(module, f"{module}.md")
        guide_path = ROOT / "docs" / "stdlib" / "modules" / guide_name
        if not guide_path.exists():
            print(
                f"docs/stdlib/modules: missing guide for std::{module} "
                f"(expected {guide_name})",
                file=sys.stderr,
            )
            return 1
        require(modules_readme, guide_name, modules_readme_path)

    boxed_guide_path = "docs/stdlib/modules/boxed.md"
    boxed_guide = read(boxed_guide_path)
    for needle in [
        "# std::boxed",
        "zone-backed owner",
        "Box[T]",
        "try_get",
        "try_take",
        "Option[T]",
        "copy_to",
        "put_in",
        "zone::destroy",
        "std-boxed-box.ari",
        "std-boxed-try-get.ari",
        "std-boxed-try-take.ari",
        "std-boxed-put-in-different-zone.ari",
    ]:
        require(boxed_guide, needle, boxed_guide_path)

    # Runtime std should have log/error/test helpers, but no dedicated
    # compiler-diagnostic std module. That belongs in compiler/tooling packages.
    for path in [
        "docs/stdlib/README.md",
        "docs/stdlib/overview.md",
        "docs/stdlib/roadmap.md",
        "docs/stdlib/api-reference.md",
        "tests/std_api_manifest.txt",
    ]:
        text = read(path)
        reject(text, "std::diag", path)
        reject(text, "`diag`", path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
