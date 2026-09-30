"""Isolated ``bin -> asm`` carve probes for mis-split code ranges (evidence only).

A Splat segment start can cut a jump-table arm off its owning function, and a
leading ``bin`` blob can hold code that was never given a boundary at all; the
range then cannot be lifted as written.  This owner re-groups the containing
``bin`` row into head/asm/tail at unchanged offsets inside an isolated workspace
and reports whether the carve is shape-valid and layout-preserving.

It writes no live file, authors no C boundary and runs no assembler/linker, so a
``supported`` result is a candidate for a reviewed layout transaction — never
permission to apply one.  Class A blockers that sit in a *bare* segment row
(for example ``emi/etc/shop/00``'s ``[0, bin, header]`` before the ``main``
segment at ``0x2D78``) are refused here with that exact reason: they need a
segment-level regroup, not a row split.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.common.cli import add_example_argument, add_root_argument, run_main
from harness.common.directory import validate_repo_path
from harness.common.files import atomic_write
from harness.common.paths import leaf_stat
from harness.domain.manifests import load_target_manifests

from .boundary import BoundaryInputs
from .probe import probe_carve

EXAMPLE = "bin/harness analysis carve emi/etc/shop/00@0x801D3828 0x2C28:0x2D78 --json"


def _parse_range(text: str) -> tuple[int, int]:
    parts = text.split(":", 1)
    if len(parts) != 2:
        raise ValueError("carve range must be START:END in hex or decimal offsets")
    start, end = (int(value, 0) for value in parts)
    if start >= end:
        raise ValueError("carve range must be non-empty")
    return start, end


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    target = args.selector.split("@", 1)[0]
    manifests = load_target_manifests(root)
    if target not in manifests:
        raise ValueError(f"unknown target: {target}")
    start, end = _parse_range(args.range)
    name = args.name or f"func_{manifests[target].load_address + start:08X}"
    if not name.replace("_", "").isalnum() or name[0].isdigit():
        raise ValueError("carve name must be a C identifier")
    inputs = BoundaryInputs(root)
    report = probe_carve(root, manifests[target], inputs, start, end, name)
    report["selector"] = args.selector
    report["target"] = target
    content = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        path = validate_repo_path(args.output)
        if not path.startswith("out/") or Path(path).suffix != ".json":
            raise ValueError("carve output must be a new out/**/*.json artifact")
        if leaf_stat(root, path) is not None:
            raise ValueError(f"carve output already exists: {path}")
        atomic_write(root, args.output, content.encode(), exclusive=True)
    if args.json:
        print(content, end="")
    else:
        print(f"carve {target} [{start:#x},{end:#x}) name={name}")
        print(f"status: {report['status']}")
        for run_row in report["runs"]:
            print(f"  {run_row['phase']}: rc={run_row.get('returncode')}")
        print(f"  workspace: {report['workspace']}")
        print(report["limits"])
    return 0 if report["status"] == "supported" else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bin/harness analysis carve", description=__doc__
    )
    parser.add_argument("selector", help="TARGET@0xADDRESS (target-qualified)")
    parser.add_argument("range", help="START:END file offsets of the carve")
    parser.add_argument("--name", help="C identifier for the carved asm range")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("-o", "--output", help="new out/**/*.json report")
    add_root_argument(parser)
    add_example_argument(parser, EXAMPLE)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)
