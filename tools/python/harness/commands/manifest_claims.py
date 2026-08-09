"""Emit target manifest source claims for the CMake configure step.

Prints one ``kind|owner_source_dir|path`` line per explicit claim.  The CMake
grouping pass consumes this so ``bin/build TARGET`` compiles every claimed
lift and header regardless of physical location (semantic ``src/bof3/``
folders may live far outside the manifest ``source_dir``).  Unmigrated
targets emit nothing; CMake keeps the legacy ``source_dir`` inventory
grouping for them.
"""

from __future__ import annotations

import argparse

from ..domain.manifests import load_target_manifests
from ._common import add_root_argument, run_main


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    for manifest in load_target_manifests(root).values():
        owner = manifest.source_dir
        if manifest.has_explicit_sources:
            # Explicit-claim target: CMake must not ancestry-group unclaimed
            # files under its legacy source_dir into its build target.
            print(f"migrated|{owner}|")
        for claimed in manifest.sources + manifest.support_sources:
            print(f"source|{owner}|{claimed}")
        for claimed in manifest.headers:
            print(f"header|{owner}|{claimed}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="manifest-claims")
    add_root_argument(parser)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
