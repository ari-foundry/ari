#!/usr/bin/env python3
"""Focused CLI checks for compiler capability inventory lookup.

The capability inventory is a contributor-facing compiler map, so these checks
stay separate from language and standard-library behavior.
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
        "ari --list-capabilities",
        "ari --explain-capability name",
    )
    ok &= require_success(
        run_ari("--list-capabilities"),
        "CompilerCapabilityInventory version=1",
        "entries=26",
        "capability=functions status=implemented owner=parser/sema/backend",
        "capability=structs-and-field-layout status=implemented",
        "capability=generic-function-calls status=implemented",
        "capability=generic-aggregate-monomorphization status=implemented",
        "capability=resolver-facing-artifact status=implemented",
        "capability=class-keyword status=rejected",
    )
    ok &= require_success(
        run_ari("--target", "x86_64-pc-linux-gnu", "--no-implicit-std", "--list-capabilities"),
        "target=x86_64-pc-linux-gnu implicit_std=false",
    )
    ok &= require_success(
        run_ari("--explain-capability", "trait-resolution"),
        "CompilerCapability version=1 capability=trait-resolution status=implemented",
        'first_check="make check-traits"',
        "Rule status_values=[implemented, partial, planned, rejected] ordinary_compiler_work=true",
    )
    ok &= require_failure(
        run_ari("--explain-capability", "bytecode-backend"),
        "unknown compiler capability 'bytecode-backend'; use --list-capabilities",
    )
    ok &= require_failure(
        run_ari(str(FIXTURE), "--list-capabilities"),
        "--list-capabilities does not take an input file",
    )
    ok &= require_failure(
        run_ari("--target-info", "--list-capabilities"),
        "compiler information commands cannot be combined",
    )
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
