"""Build every lift or one target-qualified function with CMake."""

from __future__ import annotations

import argparse
import shutil
import sys

from ..build import build, cmake_target_for_directory, cmake_target_for_source
from ..domain import lookup_target_manifest, normalize_target_id
from ..io import repo_layout
from ._common import run_main
from .lift import resolve_function


def run(args: argparse.Namespace) -> int:
    root = repo_layout().root
    if args.example:
        print("bin/build exe/logo@0x801CE758")
        return 0
    if args.selector == "clean":
        shutil.rmtree(root / "build" / "src", ignore_errors=True)
        return 0
    if shutil.which("cmake") is None:
        raise FileNotFoundError("cmake executable not found in PATH")

    target = "lifts"
    if args.selector != "all" and "@" in args.selector:
        _function, _manifest, source = resolve_function(args.selector)
        if not source.is_file():
            raise FileNotFoundError(
                f"lift source not found: {source.relative_to(root)}"
            )
        target = cmake_target_for_source(root, source)
    elif args.selector != "all":
        manifest = lookup_target_manifest(root, args.selector)
        if manifest is None:
            raise ValueError(
                f"unknown target: {normalize_target_id(args.selector).value}"
            )
        source_directory = root / manifest.source_dir
        if not any(
            path.suffix in {".c", ".s", ".S"} for path in source_directory.glob("*")
        ):
            print(f"{manifest.id.value}: no authored sources")
            return 0
        target = cmake_target_for_directory(manifest.source_dir)

    result = build(root, target)
    sys.stdout.write(result.stdout)
    sys.stderr.write(result.stderr)
    return result.returncode


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bin/build",
        description="build all lifts, one TARGET, or one TARGET@0xADDRESS",
    )
    parser.add_argument("selector", nargs="?", default="all")
    parser.add_argument("--example", action="store_true")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
