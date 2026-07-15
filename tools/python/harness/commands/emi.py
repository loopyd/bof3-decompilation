from __future__ import annotations

import argparse
import sys
from pathlib import Path

from ..emi import emi_pack, emi_unpack
from ..io import repo_layout
from ._common import run_main


def run_unpack(args: argparse.Namespace) -> int:
    archive_count = emi_unpack(
        tool_path=args.tool,
        cwd=args.cwd,
        extracted_dir=args.input_dir,
        raw_emi_dir=args.output_dir,
    )
    print(f"unpacked {archive_count} EMI archives into {args.output_dir}")
    return 0


def run_pack(args: argparse.Namespace) -> int:
    archive_count = emi_pack(
        tool_path=args.tool,
        cwd=args.cwd,
        raw_emi_dir=args.input_dir,
        extracted_dir=args.output_dir,
    )
    print(f"packed {archive_count} EMI archives into {args.output_dir}")
    return 0


def configure_unpack_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--input-dir", type=Path, default=layout.extracted_dir)
    parser.add_argument("--output-dir", type=Path, default=layout.raw_emi_dir)
    parser.add_argument("--tool", type=Path, default=layout.emi_ex_bin)
    parser.add_argument("--cwd", type=Path, default=layout.root)
    parser.set_defaults(handler=run_unpack)


def configure_pack_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--input-dir", type=Path, default=layout.raw_emi_dir)
    parser.add_argument("--output-dir", type=Path, default=layout.extracted_dir)
    parser.add_argument("--tool", type=Path, default=layout.emi_ex_bin)
    parser.add_argument("--cwd", type=Path, default=layout.root)
    parser.set_defaults(handler=run_pack)


def build_parser(command_name: str) -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog=command_name)
    if command_name == "emi-unpack":
        configure_unpack_parser(parser)
        return parser
    if command_name == "emi-pack":
        configure_pack_parser(parser)
        return parser
    raise ValueError(f"unsupported EMI command: {command_name}")


def main(argv: list[str] | None = None) -> int:
    if not argv:
        raise RuntimeError("missing EMI command name")

    command_name, *command_argv = argv
    return run_main(lambda: build_parser(command_name), command_argv)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
