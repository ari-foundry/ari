#!/usr/bin/env python3
"""Generate the browsable standard-library API index from the API manifest."""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "tests" / "std_api_manifest.txt"
OUTPUT = ROOT / "docs" / "stdlib" / "generated" / "api-index.md"
STD_MODULE_DIR = ROOT / "lib" / "std"
PUBLIC_STD_MODULES = {path.stem for path in STD_MODULE_DIR.glob("*.arih")}


MODULE_TIERS = {
    "std": "core",
    "std::algo": "alloc",
    "std::allocator": "alloc",
    "std::ascii": "core",
    "std::bits": "core",
    "std::boxed": "alloc",
    "std::c": "platform",
    "std::cell": "alloc",
    "std::cmp": "core",
    "std::collections": "alloc",
    "std::context": "hosted",
    "std::convert": "core",
    "std::encoding": "core",
    "std::env": "hosted",
    "std::error": "core",
    "std::fmt": "core",
    "std::fs": "hosted",
    "std::hash": "alloc",
    "std::input": "hosted",
    "std::io": "hosted",
    "std::iter": "alloc",
    "std::log": "hosted",
    "std::math": "core",
    "std::mem": "core",
    "std::net": "hosted",
    "std::option": "core",
    "std::os": "platform",
    "std::parse": "core",
    "std::path": "core",
    "std::process": "hosted",
    "std::random": "alloc/hosted",
    "std::rc": "alloc",
    "std::result": "core",
    "std::string": "alloc",
    "std::sync": "hosted",
    "std::target": "platform",
    "std::test": "hosted",
    "std::thread": "hosted",
    "std::time": "hosted",
    "std::vec": "alloc",
    "std::zone": "alloc",
}


STATUS_BY_TIER = {
    "core": "stable candidate",
    "alloc": "usable",
    "alloc/hosted": "usable with hosted entropy APIs",
    "hosted": "platform-backed",
    "platform": "platform-specific",
}


def read_manifest() -> list[tuple[str, str]]:
    entries: list[tuple[str, str]] = []
    for raw_line in MANIFEST.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        api, note = [part.strip() for part in line.split("|", 1)]
        entries.append((api, note))
    return entries


def strip_generics(value: str) -> str:
    return re.sub(r"\[.*", "", value)


def module_for(api: str) -> str:
    try:
        kind, path = api.split(" ", 1)
    except ValueError:
        return "std"

    if kind == "module":
        return path if path.startswith("std::") else "std"

    if not path.startswith("std::"):
        return "std"

    pieces = path.split("::")
    if len(pieces) >= 2:
        candidate = strip_generics(pieces[1])
        if candidate in PUBLIC_STD_MODULES:
            return f"std::{candidate}"
    return "std"


def kind_for(api: str) -> str:
    return api.split(" ", 1)[0]


def md(value: str) -> str:
    return value.replace("|", r"\|")


def render(entries: list[tuple[str, str]]) -> str:
    grouped: dict[str, list[tuple[str, str]]] = defaultdict(list)
    kind_counts = Counter()
    tier_counts = Counter()

    for api, note in entries:
        module = module_for(api)
        grouped[module].append((api, note))
        kind_counts[kind_for(api)] += 1
        tier_counts[MODULE_TIERS.get(module, "experimental")] += 1

    modules = sorted(grouped)
    lines: list[str] = [
        "# Generated Standard Library API Index",
        "",
        "This file is generated from `tests/std_api_manifest.txt` by",
        "`python3 tools/generate_std_api_docs.py`. Do not hand-edit this file;",
        "edit the manifest coverage note or the source API, then regenerate it.",
        "",
        "Use this index when you need the exact public `std` spellings. Use the",
        "hand-written module guides for purpose, examples, ownership rules, and",
        "platform notes.",
        "",
        "## How To Read This File",
        "",
        "- `API` is the public declaration shape extracted from `lib/std.arih` and",
        "  `lib/std/*.arih`.",
        "- `Coverage note` names the focused test or docs coverage that intentionally",
        "  tracks the API.",
        "- `Tier` comes from the standard-library stability policy: core APIs should",
        "  be portable, alloc APIs require explicit zones or owned buffers, hosted",
        "  APIs require OS/runtime support, and platform APIs expose target or ABI",
        "  facts.",
        "- Generated entries describe current APIs, not future roadmap items. Future",
        "  or experimental APIs should stay in roadmap docs until they have source,",
        "  tests, and manifest coverage.",
        "",
        "## Summary",
        "",
        f"- API entries: `{len(entries)}`",
        f"- Modules: `{len(modules)}`",
        "",
        "| Tier | Entries | Stability reading |",
        "| --- | ---: | --- |",
    ]

    for tier in sorted(tier_counts):
        lines.append(
            f"| `{tier}` | {tier_counts[tier]} | {STATUS_BY_TIER.get(tier, 'experimental or unclassified')} |"
        )

    lines.extend(["", "| Kind | Entries |", "| --- | ---: |"])
    for kind in sorted(kind_counts):
        lines.append(f"| `{kind}` | {kind_counts[kind]} |")

    lines.extend(["", "## Modules", ""])
    lines.extend(["| Module | Tier | Entries |", "| --- | --- | ---: |"])
    for module in modules:
        tier = MODULE_TIERS.get(module, "experimental")
        lines.append(f"| `{module}` | `{tier}` | {len(grouped[module])} |")

    lines.append("")
    for module in modules:
        tier = MODULE_TIERS.get(module, "experimental")
        status = STATUS_BY_TIER.get(tier, "experimental or unclassified")
        lines.extend(
            [
                f"## `{module}`",
                "",
                f"Tier: `{tier}`. Stability reading: {status}.",
                "",
            ]
        )
        by_kind: dict[str, list[tuple[str, str]]] = defaultdict(list)
        for api, note in grouped[module]:
            by_kind[kind_for(api)].append((api, note))
        for kind in sorted(by_kind):
            lines.extend(
                [
                    f"### {kind}",
                    "",
                    "| API | Coverage note |",
                    "| --- | --- |",
                ]
            )
            for api, note in by_kind[kind]:
                lines.append(f"| `{md(api)}` | {md(note)} |")
            lines.append("")

    return "\n".join(lines).rstrip() + "\n"


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check",
        action="store_true",
        help="verify that the generated API index is up to date",
    )
    args = parser.parse_args(argv)

    output = render(read_manifest())
    if args.check:
        current = OUTPUT.read_text(encoding="utf-8") if OUTPUT.exists() else ""
        if current == output:
            return 0
        print(f"{OUTPUT.relative_to(ROOT)} is stale; run tools/generate_std_api_docs.py", file=sys.stderr)
        return 1

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(output, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
