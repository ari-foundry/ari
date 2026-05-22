#!/usr/bin/env python3
"""Focused CLI checks for compiler diagnostic catalog discovery."""

import os
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ARI = Path(os.environ.get("ARI", ROOT / "build" / "ari"))


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


def main():
    if not ARI.exists():
        print(f"{ARI}: compiler binary not found; run `make` first", file=sys.stderr)
        return 1

    ok = True
    ok &= require_success(
        run_ari("--list-diagnostics"),
        "DiagnosticCatalog version=1",
        "code=P0001 family=parser owner=src/parser.cpp",
        "code=A0001 family=abi owner=src/aggregate_abi.cpp",
        "code=ari/compiler family=general owner=src/driver.cpp",
    )
    ok &= require_success(
        run_ari("--explain-diagnostic", "P0001"),
        "DiagnosticExplanation version=1 code=P0001 status=known",
        "family=parser",
        "owner=src/parser.cpp",
        "artifact=\"--emit-diagnostics\"",
    )
    ok &= require_success(
        run_ari("--explain-diagnostic", "Z9999"),
        "DiagnosticExplanation version=1 code=Z9999 status=unknown",
        "owner=<none>",
        "code is not in the current diagnostic catalog",
    )
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
