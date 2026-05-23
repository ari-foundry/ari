#!/usr/bin/env python3
"""Focused CLI checks for the compiler pass catalog.

The pass catalog is compiler-development metadata: it tells contributors which
compiler layer owns a pass, what that pass consumes, what it produces, and the
smallest check to run first.
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
        "ari --list-passes",
        "ari --explain-pass name",
    )
    ok &= require_success(
        run_ari("--list-passes"),
        "CompilerPassCatalog version=1 entries=11",
        "pass=lexer layer=frontend owner=lexer",
        'input="source bytes" output="tokens with byte spans"',
        "pass=resolver-index layer=frontend owner=declaration-collector",
        'artifact="--emit-declaration-index"',
        "pass=sema layer=middle owner=sema",
        "pass=llvm-backend layer=backend owner=llvm-backend",
        "Rule one_pass_owner=true earliest_artifact_first=true executable_last=true",
    )
    ok &= require_success(
        run_ari("--explain-pass", "sema"),
        "CompilerPass version=1 pass=sema layer=middle owner=sema",
        'input="AST, modules, declarations, and cfg features"',
        'artifact="--emit-typed-ir"',
        'first_check="make check-compiler-artifacts"',
    )
    ok &= require_failure(
        run_ari("--explain-pass", "bytecode-backend"),
        "unknown compiler pass 'bytecode-backend'; use --list-passes",
    )
    ok &= require_failure(
        run_ari(str(FIXTURE), "--list-passes"),
        "--list-passes does not take an input file",
    )
    ok &= require_failure(
        run_ari("--list-passes", "--target-info"),
        "compiler information commands cannot be combined",
    )
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
