from __future__ import annotations

import argparse
import json
from pathlib import Path

from ..match.asm_diff import AsmDiffRequest, parse_int, run_asm_diff_one
from ..match.asm_differ import write_bundle
from ..paths import repo_layout
from ._common import run_main
from ._asm_diff_output import format_asm_diff_summary


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
    write_bundle(repo_layout().root, payload, html_output=args.html)
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
        return 0 if payload["exact_match"] else 1
    outputs = payload["outputs"]
    print(format_asm_diff_summary(payload, root=repo_layout().root))
    if args.show_diff:
        diff_path = Path(outputs["diff"])
        if diff_path.is_file():
            print(diff_path.read_text(encoding="utf-8"))
    return 0 if payload["exact_match"] else 1


def build_parser() -> argparse.ArgumentParser:
    layout = repo_layout()
    parser = argparse.ArgumentParser(prog="asm-diff-one")
    parser.add_argument("source", type=Path)
    parser.add_argument(
        "--address",
        type=parse_int,
        help="original function address; read from @source or func_XXXXXXXX when omitted",
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
        default=layout.out_dir / "matching",
        help="directory for asm diff outputs",
    )
    parser.add_argument("--json", action="store_true", help="print the result as JSON")
    parser.add_argument(
        "--show-diff",
        action="store_true",
        help="print the unified diff of normalized instructions",
    )
    parser.add_argument("--html", action="store_true")
    parser.set_defaults(handler=run_one)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
