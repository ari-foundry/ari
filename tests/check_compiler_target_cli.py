#!/usr/bin/env python3
"""Focused CLI checks for Ari target information output."""

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
        run_ari("--target-info"),
        "TargetInfo version=1",
        "pointer_bits=",
        "Predicates active=[",
    )
    ok &= require_success(
        run_ari("--target", "x86_64-pc-linux-gnu", "--target-info"),
        "TargetInfo version=1 triple=x86_64-pc-linux-gnu arch=x86_64 os=linux",
        "unix=true pointer_bits=64 long_bits=64",
        "Predicates active=[x86_64, linux, unix, gnu, glibc, elf, dwarf]",
    )
    ok &= require_success(
        run_ari("--target", "i686-unknown-linux-gnu", "--target-info"),
        "TargetInfo version=1 triple=i686-unknown-linux-gnu arch=x86 os=linux",
        "pointer_bits=32 long_bits=32",
    )
    ok &= require_failure(
        run_ari("tests/cases/compiler-development/artifact/ok/token-dump-basic.ari", "--target-info"),
        "--target-info does not take an input file",
    )
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
