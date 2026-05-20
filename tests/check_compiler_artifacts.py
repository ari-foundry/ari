#!/usr/bin/env python3
"""Seed checks for compiler artifact comparison.

The real Ari compiler artifact runner will grow from this contract: normalize
host-specific text first, then compare small golden files with useful mismatch
reports. Keeping this as a tiny Python check gives compiler-development docs a
working target before token/HIR/typed-IR dump producers exist.
"""

from __future__ import annotations

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
FIXTURES = ROOT / "tests" / "cases" / "compiler-development" / "artifact"


def read_fixture(relative: str) -> str:
    return (FIXTURES / relative).read_text(encoding="utf-8")


def expand_fixture_paths(text: str) -> str:
    return text.replace("$BUILD", str(ROOT / "build")).replace("$ROOT", str(ROOT))


def normalize_artifact_text(text: str) -> str:
    """Normalize paths and host noise while preserving language facts."""

    normalized = text.replace("\\", "/")
    repo = str(ROOT).replace("\\", "/")
    build = str(ROOT / "build").replace("\\", "/")

    # Normalize the more specific build path first because it is inside repo.
    normalized = normalized.replace(build, "<build>")
    normalized = normalized.replace(repo, "<repo>")

    temp_names: dict[str, str] = {}

    def temp_replacement(match: re.Match[str]) -> str:
        value = match.group(0)
        if value not in temp_names:
            temp_names[value] = f"<tmp{len(temp_names) + 1}>"
        return temp_names[value]

    normalized = re.sub(r"(?:<build>/tmp|/tmp)/ari-[A-Za-z0-9._-]+", temp_replacement, normalized)
    normalized = re.sub(r"0x[0-9A-Fa-f]+", "<ptr>", normalized)
    return normalized


def compare_text(expected: str, actual: str, label: str) -> str | None:
    expected_lines = expected.splitlines()
    actual_lines = actual.splitlines()
    count = max(len(expected_lines), len(actual_lines))
    for index in range(count):
        expected_line = expected_lines[index] if index < len(expected_lines) else "<missing>"
        actual_line = actual_lines[index] if index < len(actual_lines) else "<missing>"
        if expected_line != actual_line:
            return (
                f"artifact mismatch: {label}\n"
                f"line {index + 1}:\n"
                f"  expected: {expected_line}\n"
                f"    actual: {actual_line}\n"
            )
    return None


def require_equal(expected: str, actual: str, label: str) -> None:
    report = compare_text(expected, actual, label)
    if report is not None:
        print(report, file=sys.stderr, end="")
        raise SystemExit(1)


def require_report(expected_report: str, expected: str, actual: str, label: str) -> None:
    report = compare_text(expected, actual, label)
    if report is None:
        print(f"{label}: expected a mismatch report", file=sys.stderr)
        raise SystemExit(1)
    require_equal(expected_report, report, label + " report")


def main() -> int:
    require_equal(
        read_fixture("ok/text-equal.expected.txt"),
        read_fixture("ok/text-equal.actual.txt"),
        "tests/cases/compiler-development/artifact/ok/text-equal",
    )

    require_equal(
        read_fixture("ok/normalize-paths.expected.txt"),
        normalize_artifact_text(expand_fixture_paths(read_fixture("ok/normalize-paths.actual.txt"))),
        "tests/cases/compiler-development/artifact/ok/normalize-paths",
    )

    require_report(
        read_fixture("errors/text-line-mismatch.report.txt"),
        read_fixture("errors/text-line-mismatch.expected.txt"),
        read_fixture("errors/text-line-mismatch.actual.txt"),
        "tests/cases/compiler-development/artifact/errors/text-line-mismatch",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
