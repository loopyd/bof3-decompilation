"""``bin/rz-project`` command surface."""

from __future__ import annotations

import argparse
import json
import os
import subprocess

from ..analysis.engine import find_engine
from ..analysis.project import analyze_project, prepare_target, rizin_argv, status
from ._common import add_example_argument, add_root_argument, resolved_root, run_main


_root = resolved_root


def run_open(args: argparse.Namespace) -> int:
    project = prepare_target(_root(args), args.target)
    engine = find_engine("rizin", root=_root(args))
    os.execv(str(engine.executable), rizin_argv(project, engine))
    return 2


def run_query(args: argparse.Namespace) -> int:
    root = _root(args)
    project = prepare_target(root, args.target)
    engine = find_engine("rizin", root=root)
    completed = subprocess.run(
        rizin_argv(project, engine, commands=tuple(args.execute), quiet=True),
        cwd=root,
        timeout=args.timeout,
        check=False,
    )
    return completed.returncode


def run_status(args: argparse.Namespace) -> int:
    payload = status(_root(args), args.target)
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        print(f"{payload['target']}: {'fresh' if payload['fresh'] else 'stale'}")
        print(f"snapshot: {payload['snapshot']}")
    return 0 if payload["fresh"] else 1


def run_analyze(args: argparse.Namespace) -> int:
    project = analyze_project(_root(args), args.target, timeout=args.timeout)
    print(project.snapshot.relative_to(_root(args)))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="rz-project")
    add_root_argument(parser)
    add_example_argument(parser, "bin/rz-project analyze exe/logo")
    sub = parser.add_subparsers(dest="command", required=True)
    for name, handler, help_text in (
        ("open", run_open, "open a target-isolated interactive Rizin session"),
        ("status", run_status, "report snapshot freshness"),
    ):
        command = sub.add_parser(name, help=help_text)
        command.add_argument("target")
        command.set_defaults(handler=handler)
    status_parser = sub.choices["status"]
    status_parser.add_argument("--json", action="store_true")
    analyze = sub.add_parser("analyze", help="rebuild generated snapshot using Rizin")
    analyze.add_argument("target")
    analyze.add_argument("--timeout", type=int, default=120)
    analyze.set_defaults(handler=run_analyze)
    query = sub.add_parser("query", help="run bounded commands on a target and exit")
    query.add_argument("target")
    query.add_argument(
        "-c",
        "--execute",
        action="append",
        required=True,
        help="Rizin command to execute; repeat for multiple commands",
    )
    query.add_argument("--timeout", type=int, default=120)
    query.set_defaults(handler=run_query)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
