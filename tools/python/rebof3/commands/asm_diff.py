from __future__ import annotations

import argparse
from pathlib import Path

from ..match.asm_diff import AsmDiffRequest, parse_int, run_asm_diff_one
from ..paths import repo_layout
from ._common import run_main


def run_one(args: argparse.Namespace) -> int:
    payload = run_asm_diff_one(
        AsmDiffRequest(
            source_path=args.source,
            address=args.address,
            size=args.size,
            binary_path=args.binary,
            load_address=args.load_address,
            output_root=args.output_root,
        )
    )
    outputs = payload["outputs"]
    instruct = payload["instruction_count"]
    print(f"status: {payload['status']}")
    print(f"match: {instruct['match_percent']:.2f}% ({instruct['matching']}/{max(instruct['original'], instruct['current'])} instrs)")
    print(f"function: {payload['function']} {payload['address']}")
    print(f"summary: {outputs['summary_json']}")
    print(f"diff: {outputs['diff']}")
    return 0 if payload["exact_match"] else 1


def build_parser() -> argparse.ArgumentParser:
    layout = repo_layout()
    parser = argparse.ArgumentParser(prog="asm-diff-one")
    parser.add_argument("source", type=Path)
    parser.add_argument(
        "--address",
        type=parse_int,
        help="original function address; inferred from @source or func_XXXXXXXX when omitted",
    )
    parser.add_argument(
        "--size",
        type=parse_int,
        help="original function byte size; inferred from the next sibling source when omitted",
    )
    parser.add_argument(
        "--binary",
        type=Path,
        help="original PS-X EXE or raw overlay binary; core sources default to SLUS",
    )
    parser.add_argument(
        "--load-address",
        type=parse_int,
        help="load address for raw binaries; PS-X EXE headers are read automatically",
    )
    parser.add_argument(
        "--output-root",
        type=Path,
        default=layout.out_dir / "asm-diff",
        help="directory for asm diff outputs",
    )
    parser.set_defaults(handler=run_one)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
