#!/usr/bin/env python3
"""Seed checks for compiler artifact comparison.

The Ari artifact runner normalizes host-specific text first, then compares
small golden files with useful mismatch reports. It also exposes an explicit
update mode so golden regeneration stays reviewable instead of automatic.
"""

from __future__ import annotations

from pathlib import Path
import re
import sys
import tempfile


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
                f"summary: expected_lines={len(expected_lines)} actual_lines={len(actual_lines)}\n"
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


def require_update_mode() -> None:
    with tempfile.TemporaryDirectory(prefix="ari-artifact-") as temp:
        directory = Path(temp)
        expected = directory / "expected.txt"
        actual = directory / "actual.txt"
        expected.write_text("old\n", encoding="utf-8")
        actual.write_text("$ROOT/path 0x1234\n", encoding="utf-8")
        update_artifact_file(str(expected), str(actual), normalize=True)
        require_equal(
            "<repo>/path <ptr>\n",
            expected.read_text(encoding="utf-8"),
            "tests/check_compiler_artifacts.py --update",
        )


def resolve_user_path(value: str) -> Path:
    path = Path(value)
    if path.is_absolute():
        return path
    return ROOT / path


def compare_artifact_files(expected_path: str, actual_path: str, normalize: bool) -> int:
    expected_file = resolve_user_path(expected_path)
    actual_file = resolve_user_path(actual_path)
    expected = expected_file.read_text(encoding="utf-8")
    actual = actual_file.read_text(encoding="utf-8")
    if normalize:
        expected = normalize_artifact_text(expand_fixture_paths(expected))
        actual = normalize_artifact_text(expand_fixture_paths(actual))
    try:
        label = str(expected_file.relative_to(ROOT))
    except ValueError:
        label = str(expected_file)
    require_equal(expected, actual, label)
    return 0


def update_artifact_file(expected_path: str, actual_path: str, normalize: bool) -> int:
    expected_file = resolve_user_path(expected_path)
    actual_file = resolve_user_path(actual_path)
    actual = actual_file.read_text(encoding="utf-8")
    if normalize:
        actual = normalize_artifact_text(expand_fixture_paths(actual))
    expected_file.write_text(actual, encoding="utf-8")
    return 0


def artifact_kind(path: Path) -> str:
    name = path.name
    suffix = path.suffix
    if name.endswith(".llvm-frag"):
        return "llvm-fragment"
    if suffix == ".ari":
        return "source"
    if suffix == ".catalog":
        return "diagnostic-catalog"
    if suffix == ".decls":
        return "declaration-index"
    if suffix == ".diagnostic":
        return "diagnostic"
    if suffix == ".graph":
        return "module-graph"
    if suffix == ".h":
        return "c-header"
    if suffix == ".inventory":
        return "inventory"
    if suffix == ".ir":
        return "typed-ir"
    if suffix == ".map":
        return "source-map"
    if suffix == ".plan":
        return "stage-plan"
    if suffix == ".resolved":
        return "resolved-index"
    if suffix == ".stdout":
        return "runtime-output"
    if suffix == ".summary":
        return "pass-summary"
    if suffix == ".symbols":
        return "symbols"
    if suffix == ".syntax":
        return "syntax"
    if suffix == ".tokens":
        return "tokens"
    if suffix == ".txt":
        return "text"
    return "unknown"


def iter_fixture_entries(group: str) -> list[tuple[str, str, str]]:
    groups = ["ok", "errors"] if group == "all" else [group]
    entries: list[tuple[str, str, str]] = []
    for fixture_group in groups:
        directory = FIXTURES / fixture_group
        for path in sorted(directory.rglob("*")):
            if not path.is_file():
                continue
            relative = path.relative_to(FIXTURES).as_posix()
            entries.append((fixture_group, artifact_kind(path), relative))
    return sorted(entries, key=lambda entry: entry[2])


def list_fixtures(group: str) -> int:
    if group not in {"all", "ok", "errors"}:
        print("fixture group must be one of: all, ok, errors", file=sys.stderr)
        return 2
    entries = iter_fixture_entries(group)
    print(f"CompilerArtifactFixtures version=1 group={group} entries={len(entries)}")
    for fixture_group, kind, relative in entries:
        print(f"  Fixture group={fixture_group} kind={kind} path={relative}")
    return 0


def run_seed_checks() -> int:
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
    require_update_mode()
    return 0


def main(argv: list[str]) -> int:
    if len(argv) == 1:
        return run_seed_checks()
    if argv[1] == "--list-fixtures":
        group = argv[2] if len(argv) == 3 else "all"
        if len(argv) > 3:
            print("usage: check_compiler_artifacts.py --list-fixtures [all|ok|errors]", file=sys.stderr)
            return 2
        return list_fixtures(group)
    normalize = False
    update = False
    args = argv[1:]
    while args and args[0] in {"--normalize", "--update"}:
        if args[0] == "--normalize":
            normalize = True
        elif args[0] == "--update":
            update = True
        args = args[1:]
    if len(args) == 2:
        if update:
            return update_artifact_file(args[0], args[1], normalize)
        return compare_artifact_files(args[0], args[1], normalize)
    print(
        "usage: check_compiler_artifacts.py [--normalize] [--update] expected actual",
        file=sys.stderr,
    )
    return 2


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
