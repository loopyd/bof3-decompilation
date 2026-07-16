"""``bin/rz-project`` command surface."""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import sys

from ..analyzer import find_engine
from ..io import repo_layout
from ..rizin_project import analyze_project, export_patch, prepare_project, rizin_argv, status
from ._common import run_main


def _root(args: argparse.Namespace) -> Path:
    return args.root.resolve()


def run_open(args: argparse.Namespace) -> int:
    project = prepare_project(_root(args), args.target)
    engine = find_engine("rizin")
    os.execv(str(engine.executable), rizin_argv(project, engine))
    return 2


def run_status(args: argparse.Namespace) -> int:
    payload = status(_root(args), args.target)
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        print(f"{payload['target']}: {'fresh' if payload['fresh'] else 'stale'}")
        print(f"project: {payload['project']}")
        print(f"snapshot: {payload['snapshot']}")
    return 0 if payload["fresh"] else 1


def run_export(args: argparse.Namespace) -> int:
    print(export_patch(_root(args), args.target, write=args.write), end="")
    return 0


def run_analyze(args: argparse.Namespace) -> int:
    project = analyze_project(_root(args), args.target, deep=args.deep, timeout=args.timeout)
    print(project.snapshot.relative_to(_root(args)))
    return 0


def run_rebuild(args: argparse.Namespace) -> int:
    project = prepare_project(_root(args), args.target)
    print(project.project_path.relative_to(_root(args)))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="rz-project")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    sub = parser.add_subparsers(dest="command", required=True)
    for name, handler, help_text in (
        ("open", run_open, "open a target-isolated interactive Rizin session"),
        ("status", run_status, "report generated project/snapshot freshness"),
        ("rebuild", run_rebuild, "rebuild the generated Rizin replay project"),
    ):
        command = sub.add_parser(name, help=help_text)
        command.add_argument("target")
        command.set_defaults(handler=handler)
    status_parser = sub.choices["status"]
    status_parser.add_argument("--json", action="store_true")
    export = sub.add_parser("export", help="print deterministic reviewed replay patch")
    export.add_argument("target")
    export.add_argument("--write", action="store_true", help="write reviewed replay after validation")
    export.set_defaults(handler=run_export)
    analyze = sub.add_parser("analyze", help="rebuild generated snapshot using Rizin")
    analyze.add_argument("target")
    analyze.add_argument("--deep", action="store_true", help="allow a longer bounded analysis pass")
    analyze.add_argument("--timeout", type=int, default=120)
    analyze.set_defaults(handler=run_analyze)
    return parser


def main(argv: list[str] | None = None) -> int:
    arguments = sys.argv[1:] if argv is None else argv
    if arguments == ["--example"]:
        print("bin/rz-project analyze exe/logo")
        return 0
    return run_main(build_parser, arguments)


if __name__ == "__main__":
    raise SystemExit(main())
