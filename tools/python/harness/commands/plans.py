"""Parse persistent Markdown plans and dispatch reviewed consolidation."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.common.cli import add_root_argument, resolved_root, run_main

from . import _plans


class Once(argparse.Action):
    """Reject repeated singular options rather than accepting the last value."""

    def __call__(self, parser, namespace, values, option_string=None):
        seen = getattr(namespace, "_seen", set())
        if self.dest in seen:
            parser.error(f"duplicate option: {option_string}")
        setattr(namespace, "_seen", seen | {self.dest})
        setattr(namespace, self.dest, values if self.nargs != 0 else True)


def dispatch(args: argparse.Namespace) -> int:
    _plans.check_path(args.root)
    root = resolved_root(args)
    try:
        if args.command == "consolidate":
            return _plans.consolidate(root, args.review, args.apply, args.backup_dir)
        sources = _plans.inventory(root)
        plans = {
            name: _plans.parse_plan(raw.decode("utf-8"))
            for name, raw in sources.items()
        }
        if args.command == "list":
            if not plans:
                print("No plans (not a completion verdict).")
            for name, plan in plans.items():
                counts = {
                    kind: {
                        state: sum(
                            i.phase == phase and i.state == state for i in plan.items
                        )
                        for state in _plans.STATES
                    }
                    for kind, phase in (("phases", True), ("steps", False))
                }
                print(name, json.dumps(counts, sort_keys=True))
        else:
            selected = args.plan
            if selected is None:
                if len(plans) != 1:
                    raise ValueError(
                        "status requires exactly one plan or an explicit filename"
                    )
                selected = next(iter(plans))
            _plans.plan_name(selected)
            if selected not in plans:
                raise ValueError(f"plan not found: {selected}")
            print(selected)
            by_id = {item.id: item for item in plans[selected].items}
            for item in plans[selected].items:
                print(f"{item.id} ({item.state}) {item.title}")
                if item.parent:
                    parent = by_id[item.parent]
                    print(f"  Inherited Depends: {parent.fields['Depends']}")
                    print(f"  Inherited Blocker: {parent.fields['Blocker']}")
                for key, value in item.fields.items():
                    print(f"  {key}: {value}")
        return 0
    except OSError as exc:
        raise ValueError(str(exc)) from exc


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bin/harness plans", description=__doc__, allow_abbrev=False
    )
    add_root_argument(parser)
    # Shared root declaration, with the same singular-option safety as local flags.
    action = parser._option_string_actions["--root"]
    action.__class__ = Once
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("list", allow_abbrev=False)
    status = commands.add_parser("status", allow_abbrev=False)
    status.add_argument("plan", nargs="?")
    merge = commands.add_parser("consolidate", allow_abbrev=False)
    merge.add_argument("review", type=Path)
    merge.add_argument("--apply", action=Once, nargs=0, default=False)
    merge.add_argument("--backup-dir", type=Path, action=Once)
    parser.set_defaults(handler=dispatch)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
