"""CLI for index-independent boundary evidence and isolated textbin probes."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.analysis.boundary import collect_boundaries
from harness.common.cli import add_example_argument, add_root_argument, run_main
from harness.common.directory import validate_repo_path
from harness.common.files import atomic_write
from harness.common.paths import leaf_stat


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    if args.output:
        name = validate_repo_path(args.output)
        if not name.startswith("out/") or Path(name).suffix != ".json":
            raise ValueError("boundary output must be a new out/**/*.json artifact")
        if leaf_stat(root, name) is not None:
            raise ValueError(f"boundary output already exists: {name}")
    report = collect_boundaries(root, args.selectors, probe=args.probe_textbin)
    content = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        atomic_write(root, args.output, content.encode(), exclusive=True)
    if args.json:
        print(content, end="")
    else:
        print(
            f"boundary evidence: targets={len(report['targets'])} conflicts={report['conflicts']}"
        )
        for row in report["rows"]:
            probe = row["probe"]
            print(
                f"{row['target']}@{row['address']} {row['symbol']} conflict={row['conflict']} probe={probe['status'] if probe else 'not-run'}"
            )
            print(f"  {row['resolution']}")
            if probe and probe.get("workspace"):
                print(f"  evidence: {probe['workspace']}")
        if args.output:
            print(f"report: {args.output}")
        print("Research evidence only; no live layout edits or index acceptance.")
    return int(
        any(
            row["probe"] and row["probe"]["status"] != "supported"
            for row in report["rows"]
        )
    )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness analysis boundary")
    add_root_argument(parser)
    parser.add_argument(
        "selectors",
        nargs="*",
        help="TARGET conflicts or TARGET@0xADDRESS evidence; default: all targets",
    )
    parser.add_argument(
        "--probe-textbin",
        action="store_true",
        help="run isolated before/after Splat splits; never apply",
    )
    parser.add_argument("--json", action="store_true")
    parser.add_argument("-o", "--output", help="new report under out/ (no overwrite)")
    add_example_argument(
        parser,
        "bin/harness analysis boundary emi/etc/commu00/00@0x801F24FC --probe-textbin --json",
    )
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)
