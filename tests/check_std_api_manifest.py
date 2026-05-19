#!/usr/bin/env python3
"""Check that the source std public API surface is intentionally tracked."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
STD_FILES = [ROOT / "lib" / "std.arih", *sorted((ROOT / "lib" / "std").glob("*.arih"))]
MANIFEST = ROOT / "tests" / "std_api_manifest.txt"

IDENT = r"[A-Za-z_][A-Za-z0-9_]*"
GENERIC = r"(\[[^\]]+\])?"

MODULE_RE = re.compile(rf"^pub\s+mod\s+({IDENT})\s*;")
TYPE_RE = re.compile(rf"^pub\s+(struct|enum|trait|type)\s+({IDENT}){GENERIC}")
CONST_RE = re.compile(rf"^pub\s+const\s+({IDENT})")
FN_RE = re.compile(rf"^pub\s+(?:extern\s+\"[^\"]+\"\s+)?fn\s+({IDENT}){GENERIC}")
IMPL_RE = re.compile(r"^pub\s+impl(?:\[[^\]]+\])?\s+([^{]+)\{")
METHOD_RE = re.compile(rf"^\s*(?:pub\s+)?fn\s+({IDENT}){GENERIC}")
USE_GROUP_RE = re.compile(rf"^pub\s+use\s+({IDENT}(?:::{IDENT})*)::\{{([^}}]+)\}}\s*;")
USE_SINGLE_RE = re.compile(
    rf"^pub\s+use\s+({IDENT}(?:::{IDENT})*)::({IDENT})(?:\s+as\s+({IDENT}))?\s*;"
)


def module_for(path: Path) -> str:
    if path.name == "std.arih":
        return "std"
    return f"std::{path.stem}"


def normalize_type_ref(value: str) -> str:
    return re.sub(r"\s+", "", value.strip())


def strip_line_comment(line: str) -> str:
    return line.split("//", 1)[0].rstrip()


def brace_delta(line: str) -> int:
    return line.count("{") - line.count("}")


def exported_alias(item: str) -> str:
    parts = item.strip().split()
    if len(parts) == 3 and parts[1] == "as":
        return parts[2]
    return parts[0]


def api_surface() -> list[str]:
    apis: set[str] = set()

    for path in STD_FILES:
        module = module_for(path)
        context_kind: str | None = None
        context_name = ""
        depth = 0

        for raw_line in path.read_text(encoding="utf-8").splitlines():
            line = strip_line_comment(raw_line)
            if not line.strip():
                continue

            if context_kind:
                method = METHOD_RE.match(line)
                if method:
                    name, generic = method.groups()
                    kind = "method" if context_kind == "impl" else "trait-method"
                    apis.add(f"{kind} {context_name}::{name}{generic or ''}")

                depth += brace_delta(line)
                if depth <= 0:
                    context_kind = None
                    context_name = ""
                continue

            module_match = MODULE_RE.match(line)
            if module_match:
                apis.add(f"module {module}::{module_match.group(1)}")
                continue

            group_use = USE_GROUP_RE.match(line)
            if group_use:
                for item in group_use.group(2).split(","):
                    alias = exported_alias(item)
                    if alias:
                        apis.add(f"use {module}::{alias}")
                continue

            single_use = USE_SINGLE_RE.match(line)
            if single_use:
                alias = single_use.group(3) or single_use.group(2)
                apis.add(f"use {module}::{alias}")
                continue

            type_match = TYPE_RE.match(line)
            if type_match:
                kind, name, generic = type_match.groups()
                full_name = f"{module}::{name}{generic or ''}"
                apis.add(f"{kind} {full_name}")
                if kind == "trait" and "{" in line:
                    context_kind = "trait"
                    context_name = full_name
                    depth = brace_delta(line)
                    if depth <= 0:
                        context_kind = None
                        context_name = ""
                continue

            const_match = CONST_RE.match(line)
            if const_match:
                apis.add(f"const {module}::{const_match.group(1)}")
                continue

            impl_match = IMPL_RE.match(line)
            if impl_match:
                context_kind = "impl"
                context_name = normalize_type_ref(impl_match.group(1))
                depth = brace_delta(line)
                if depth <= 0:
                    context_kind = None
                    context_name = ""
                continue

            fn_match = FN_RE.match(line)
            if fn_match:
                name, generic = fn_match.groups()
                apis.add(f"fn {module}::{name}{generic or ''}")

    return sorted(apis)


def read_manifest(path: Path) -> list[str]:
    entries: list[str] = []
    errors: list[str] = []
    seen: dict[str, int] = {}

    for lineno, raw_line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        if "|" not in line:
            errors.append(f"{path}:{lineno}: missing coverage note separator '|'")
            continue
        api, note = [part.strip() for part in line.split("|", 1)]
        if not api:
            errors.append(f"{path}:{lineno}: empty API entry")
            continue
        if not note:
            errors.append(f"{path}:{lineno}: empty coverage note")
            continue
        if api in seen:
            errors.append(f"{path}:{lineno}: duplicate API entry also listed on line {seen[api]}")
            continue
        seen[api] = lineno
        entries.append(api)

    if entries != sorted(entries):
        errors.append(f"{path}: API entries must stay sorted")

    if errors:
        raise SystemExit("\n".join(errors))

    return entries


def print_manifest() -> None:
    for api in api_surface():
        print(f"{api}|TODO: add focused tests and docs/test-matrix coverage")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--print", action="store_true", help="print the current extracted API surface")
    args = parser.parse_args(argv)

    actual = api_surface()
    if args.print:
        print_manifest()
        return 0

    expected = read_manifest(MANIFEST)
    expected_set = set(expected)
    actual_set = set(actual)

    missing = sorted(expected_set - actual_set)
    added = sorted(actual_set - expected_set)
    if not missing and not added:
        return 0

    print("source std API manifest is stale", file=sys.stderr)
    if added:
        print("\nNew public API entries need manifest notes:", file=sys.stderr)
        for api in added:
            print(f"  + {api}", file=sys.stderr)
    if missing:
        print("\nManifest entries no longer exist in lib/std:", file=sys.stderr)
        for api in missing:
            print(f"  - {api}", file=sys.stderr)
    print(
        "\nUpdate tests/std_api_manifest.txt with focused test/docs coverage notes before committing.",
        file=sys.stderr,
    )
    return 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
