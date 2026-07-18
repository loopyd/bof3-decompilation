"""Preview or create one conservative EMI analysis target."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from ..emi.catalog import apply_bootstrap, bootstrap_plan, load_catalog
from ..io import repo_layout
from ._common import run_main


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    catalog = load_catalog(root)
    plan = bootstrap_plan(root, catalog, args.entry)
    if args.apply:
        created = apply_bootstrap(root, catalog, plan)
        result = {
            "schema": plan["schema"],
            "entry": plan["entry"],
            "target": plan["target"],
            "created": [path.relative_to(root).as_posix() for path in created],
        }
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(json.dumps(plan, indent=2, sort_keys=True))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="emi-target",
        description="Preview or create one bin-only EMI reverse-engineering target.",
    )
    parser.add_argument(
        "entry", help="archive slot, for example BIN/BATTLE/BATL_END.EMI#0"
    )
    parser.add_argument(
        "--apply", action="store_true", help="create the previewed files"
    )
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    arguments = sys.argv[1:] if argv is None else argv
    if arguments == ["--example"]:
        print(
            "bin/emi-target BIN/BATTLE/BATL_END.EMI#0\n"
            "bin/emi-target BIN/BATTLE/BATL_END.EMI#0 --apply"
        )
        return 0
    return run_main(build_parser, arguments)


if __name__ == "__main__":
    raise SystemExit(main())
