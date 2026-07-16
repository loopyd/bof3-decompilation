"""Run Splat for exactly one manifest-owned target."""

from __future__ import annotations

import argparse
from pathlib import Path
import subprocess
import sys

from ..domain import load_target_manifests, normalize_target_id
from ..io import repo_layout
from ._common import run_main


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    target = normalize_target_id(args.target).value
    manifest = load_target_manifests(root).get(target)
    if manifest is None:
        raise ValueError(f"unknown target: {args.target}")
    executable = root / ".venv" / "bin" / "splat"
    if not executable.is_file():
        raise FileNotFoundError(f"missing Splat executable: {executable}; run just setup")
    result = subprocess.run(
        [str(executable), "split", "--make-full-disasm-for-code", str(root / manifest.splat)],
        cwd=root,
    )
    return result.returncode


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="splat")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.add_argument("target", help="target id, for example exe/logo")
    parser.add_argument("--example", action="store_true")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    arguments = sys.argv[1:] if argv is None else argv
    if "--example" in arguments:
        print("bin/splat exe/logo")
        return 0
    return run_main(build_parser, arguments)


if __name__ == "__main__":
    raise SystemExit(main())
