#!/usr/bin/env python3
"""Focused CLI checks for compiler roadmap work-item lookup.

The work-item catalog is an implementation aid for ordinary Ari compiler work.
It keeps near-term compiler tasks attached to first files, first artifacts, and
small checks so contributors do not need to start from broad suite runs.
"""

import os
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ARI = Path(os.environ.get("ARI", ROOT / "build" / "ari"))
FIXTURE = ROOT / "tests" / "cases" / "compiler-development" / "artifact" / "ok" / "token-dump-basic.ari"


def run_ari(*args):
    return subprocess.run(
        [str(ARI), *map(str, args)],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


def require_success(result, *needles):
    if result.returncode != 0:
        print(f"expected ari command to succeed, got {result.returncode}", file=sys.stderr)
        print(result.stderr, file=sys.stderr)
        return False
    for needle in needles:
        if needle not in result.stdout:
            print("missing expected stdout text", file=sys.stderr)
            print(f"expected: {needle}", file=sys.stderr)
            print(f"stdout: {result.stdout}", file=sys.stderr)
            return False
    return True


def require_failure(result, expected):
    if result.returncode == 0:
        print("expected ari command to fail, but it succeeded", file=sys.stderr)
        print(result.stdout, file=sys.stderr)
        return False
    if expected not in result.stderr:
        print("missing expected diagnostic text", file=sys.stderr)
        print(f"expected: {expected}", file=sys.stderr)
        print(f"stderr: {result.stderr}", file=sys.stderr)
        return False
    return True


def main():
    if not ARI.exists():
        print(f"{ARI}: compiler binary not found; run `make` first", file=sys.stderr)
        return 1

    ok = True
    ok &= require_success(
        run_ari("--help"),
        "ari --list-work-items",
        "ari --explain-work-item name",
    )
    ok &= require_success(
        run_ari("--list-work-items"),
        "CompilerWorkItemCatalog version=1 entries=10",
        "work_item=source-identity-hardening priority=P0 area=source/diagnostics",
        "work_item=generic-aggregate-stress priority=P1 area=types/generics",
        "work_item=backend-artifact-normalization priority=P2 area=backend",
        "Rule ordinary_compiler_work=true smallest_artifact_first=true roadmap_is_not_bootstrap=true",
    )
    ok &= require_success(
        run_ari("--explain-work-item", "generic-aggregate-stress"),
        "CompilerWorkItem version=1 work_item=generic-aggregate-stress priority=P1 area=types/generics",
        'first_artifact="--emit-typed-ir"',
        'first_check="make check-generics"',
        "generic structs, enums, aliases, and nested payloads lower without one-off escapes",
    )
    ok &= require_failure(
        run_ari("--explain-work-item", "bootstrap-stage1"),
        "unknown compiler work item 'bootstrap-stage1'; use --list-work-items",
    )
    ok &= require_failure(
        run_ari(str(FIXTURE), "--list-work-items"),
        "--list-work-items does not take an input file",
    )
    ok &= require_failure(
        run_ari("--list-work-items", "--list-test-buckets"),
        "compiler information commands cannot be combined",
    )
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
