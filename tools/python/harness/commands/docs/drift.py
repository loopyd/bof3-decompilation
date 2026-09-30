"""Report documentation drift against code and CLI evidence."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import add_root_argument, run_main
from harness.docs.drift import collect_drift


def run(args: argparse.Namespace) -> int:
    findings = collect_drift(args.root.resolve())
    if args.json:
        print(json.dumps({"findings": findings, "count": len(findings)}, indent=2))
    else:
        for finding in findings:
            print(f"drift: {finding['where']}: {finding['detail']}")
        print(f"docs drift: {len(findings)} finding(s)")
    return 2 if findings else 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness source docs")
    add_root_argument(parser)
    parser.add_argument("--json", action="store_true", help="emit a JSON report")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
