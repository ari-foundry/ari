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


def run_raw(*args):
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

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    ok = True

    ok &= require_success(
        run_raw("--help"),
        "usage: ari <input.ari>",
        "ari --list-artifacts",
        "ari --explain-artifact option",
    )

    ok &= require_success(
        run_raw("--list-artifacts"),
        "CompilerArtifacts version=1",
        "option=--emit-capability-inventory",
        "option=--emit-c-header",
        "option=--emit-llvm",
        "option=--emit-llvm-fragment",
        "option=--emit-obj",
        "option=--emit-resolved-index",
        "option=--emit-symbols",
        "option=--shared",
        "option=-o",
        "Rule one_artifact_output=true backend_outputs_separate=true",
    )

    ok &= require_success(
        run_raw("--explain-artifact", "--emit-tokens"),
        "CompilerArtifact version=1 option=--emit-tokens owner=lexer",
        'first_check="make check-compiler-artifacts"',
        'purpose="token kinds, spellings, and byte spans"',
        "Rule earliest_layer=true one_artifact_output=true",
    )

    ok &= require_success(
        run_raw("--explain-artifact", "emit-pass-summary"),
        "CompilerArtifact version=1 option=--emit-pass-summary owner=driver/sema",
    )

    ok &= require_success(
        run_raw("--explain-artifact", "emit-resolved-index"),
        "CompilerArtifact version=1 option=--emit-resolved-index owner=sema",
        'purpose="resolved functions, locals, calls, enum cases, and pattern bindings"',
    )

    ok &= require_success(
        run_raw("--explain-artifact", "--emit-llvm"),
        "CompilerArtifact version=1 option=--emit-llvm owner=llvm-backend",
        'first_check="focused --emit-llvm"',
        "Rule earliest_layer=false one_artifact_output=false backend_output=true",
    )

    ok &= require_success(
        run_raw("--explain-artifact", "--emit-llvm-fragment"),
        "CompilerArtifact version=1 option=--emit-llvm-fragment owner=llvm-backend",
        "Rule earliest_layer=false one_artifact_output=false backend_output=true llvm_fragment=true requested_symbols=true requires_emit_llvm=true",
    )

    ok &= require_success(
        run_raw("--explain-artifact", "--emit-c-header"),
        "CompilerArtifact version=1 option=--emit-c-header owner=abi-header",
        "Rule earliest_layer=false one_artifact_output=false header_output=true",
    )

    ok &= require_success(
        run_raw("--explain-artifact", "--shared"),
        "CompilerArtifact version=1 option=--shared owner=toolchain",
        "Rule earliest_layer=false one_artifact_output=false backend_output=true shared_library=true symbol_inventory=true",
    )

    ok &= require_success(
        run_raw("--explain-artifact", "--emit-symbols"),
        "CompilerArtifact version=1 option=--emit-symbols owner=toolchain",
        "Rule earliest_layer=false one_artifact_output=false backend_output=true symbol_inventory=true requested_symbols=true",
    )

    ok &= require_success(
        run_raw("--explain-artifact", "-o"),
        "CompilerArtifact version=1 option=-o owner=toolchain/runtime",
        "Rule earliest_layer=false one_artifact_output=false runtime_output=true stdout_stderr_capture=true",
    )

    token_output = OUT_DIR / "absolute-output.tokens"
    ok &= require_success(
        run_ari("--emit-tokens", token_output),
        "wrote build/compiler-development/artifact-cli/absolute-output.tokens (token dump)",
    )

    ok &= require_failure(
        run_raw("--explain-artifact", "--emit-bytecode"),
        "unknown compiler artifact option '--emit-bytecode'; use --list-artifacts",
    )

    combined = run_ari(
        "--emit-tokens", OUT_DIR / "tokens.tokens",
        "--emit-syntax", OUT_DIR / "syntax.syntax",
    )
    ok &= require_failure(
        combined,
        "artifact outputs cannot be combined: --emit-tokens, --emit-syntax",
    )

    ok &= require_failure(
        run_ari("--symbol", "_ARNv3add"),
        "--symbol requires --emit-symbols",
    )

    ok &= require_failure(
        run_ari("--emit-symbols", OUT_DIR / "symbols.symbols"),
        "--emit-symbols requires --emit-obj or --shared",
    )

    ok &= require_failure(
        run_ari("--llvm-symbol", "_ARNv4main"),
        "--llvm-symbol requires --emit-llvm-fragment",
    )

    ok &= require_failure(
        run_ari("--emit-llvm-fragment", OUT_DIR / "backend.llvm-frag"),
        "--emit-llvm-fragment requires --emit-llvm",
    )

    ok &= require_failure(
        run_ari(
            "--shared",
            "--emit-llvm", OUT_DIR / "symbols.ll",
            "--emit-symbols", OUT_DIR / "symbols.symbols",
            "--symbol", "_ARNv4main",
        ),
        "--emit-symbols cannot be combined with --emit-llvm",
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
