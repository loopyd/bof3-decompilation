"""Build a verified hybrid executable or resident EMI payload."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.build.payload import build_payload
from harness.common.cli import (
    SingleValue,
    add_example_argument,
    add_root_argument,
    resolved_root,
    run_main,
)


def run(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    report = build_payload(root, args.target, args.output_dir)
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        report_path = root / report["output"]
        print(
            f"{report['target']}: {report['compiled_bytes']} compiled bytes, "
            f"{report['retained_bytes']} retained bytes\n"
            f"Report: {report_path.parent / 'report.json'}"
        )
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness build payload")
    parser.add_argument("target")
    parser.add_argument("--output-dir", type=Path, action=SingleValue, required=True)
    parser.add_argument(
        "--json", action="store_true", help="emit the payload report as JSON"
    )
    add_root_argument(parser)
    add_example_argument(
        parser,
        "bin/harness build payload exe/logo --output-dir out/payloads/logo --json",
    )
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
