#!/usr/bin/env python3
"""Focused CLI checks for compiler artifact option handling.

This script stays separate from broad suite targets so artifact UX changes can
be checked while other library work is in progress.
"""

import os
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ARI = Path(os.environ.get("ARI", ROOT / "build" / "ari"))
FIXTURE = ROOT / "tests" / "cases" / "compiler-development" / "artifact" / "ok" / "token-dump-basic.ari"
OUT_DIR = ROOT / "build" / "compiler-development" / "artifact-cli"


def run_ari(*args):
    return subprocess.run(
        [str(ARI), str(FIXTURE), *map(str, args)],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


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

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    ok = True

    combined = run_ari(
        "--emit-tokens", OUT_DIR / "tokens.tokens",
        "--emit-syntax", OUT_DIR / "syntax.syntax",
    )
    ok &= require_failure(
        combined,
        "artifact outputs cannot be combined: --emit-tokens, --emit-syntax",
    )

    backend_mix = run_ari(
        "--target", "x86_64-pc-linux-gnu",
        "--no-implicit-std",
        "--emit-capability-inventory", OUT_DIR / "capability.inventory",
        "-o", OUT_DIR / "ignored.elf",
    )
    ok &= require_failure(
        backend_mix,
        "compiler artifact outputs cannot be combined with backend, module-cache, or linking options: --emit-capability-inventory",
    )

    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
