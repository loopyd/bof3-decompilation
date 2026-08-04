"""Preview or create one conservative EMI analysis target."""

from __future__ import annotations

import argparse
import json

from ..emi.catalog import load_catalog
from ..emi.catalog_bootstrap import apply_bootstrap, bootstrap_plan
from ._common import add_example_argument, add_root_argument, run_main


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
    add_root_argument(parser)
    add_example_argument(
        parser,
        "bin/emi-target BIN/BATTLE/BATL_END.EMI#0\n"
        "bin/emi-target BIN/BATTLE/BATL_END.EMI#0 --apply",
    )
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
