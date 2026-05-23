#!/usr/bin/env python3
"""Focused CLI checks for compiler test-bucket guidance.

The bucket catalog is compiler-development metadata. It helps contributors put
new fixtures in the closest behavior bucket and choose the smallest first
check, without treating broad executable behavior as the default signal.
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
        "ari --list-test-buckets",
        "ari --explain-test-bucket name",
    )
    ok &= require_success(
        run_ari("--list-test-buckets"),
        "CompilerTestBucketCatalog version=1 entries=6",
        'bucket=feature-ok path="tests/cases/<feature>/ok/" kind=positive-language',
        'bucket=compiler-artifact-ok path="tests/cases/compiler-development/artifact/ok/"',
        "Rule closest_behavior_bucket=true artifact_before_executable=true docs_checked=true",
    )
    ok &= require_success(
        run_ari("--explain-test-bucket", "compiler-artifact-errors"),
        "CompilerTestBucket version=1 bucket=compiler-artifact-errors",
        'path="tests/cases/compiler-development/artifact/errors/"',
        "kind=golden-error-artifact",
        'first_check="make check-compiler-artifacts"',
    )
    ok &= require_failure(
        run_ari("--explain-test-bucket", "bootstrap-only"),
        "unknown compiler test bucket 'bootstrap-only'; use --list-test-buckets",
    )
    ok &= require_failure(
        run_ari(str(FIXTURE), "--list-test-buckets"),
        "--list-test-buckets does not take an input file",
    )
    ok &= require_failure(
        run_ari("--list-test-buckets", "--list-passes"),
        "compiler information commands cannot be combined",
    )
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
