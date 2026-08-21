"""CLI adapter for ``bin/naming-audit``."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from ..analysis.naming import SCHEMA_V3
from ._common import add_root_argument, resolved_root, run_main


def _print(payload: dict[str, Any]) -> None:
    print(json.dumps(payload, indent=2, sort_keys=True))


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="naming-audit")
    add_root_argument(parser)
    sub = parser.add_subparsers(dest="command", required=True)
    prep = sub.add_parser(
        "prepare", help="readiness preflight; --repair closes proven repairs"
    )
    prep.add_argument("target")
    prep.add_argument("--repair", action="store_true")
    prep.set_defaults(handler=_run_prepare)
    init = sub.add_parser(
        "init", help="write a v3 inventory with explicit evidence gaps"
    )
    init.add_argument("target")
    init.add_argument("output", type=Path)
    init.set_defaults(handler=_run_init)
    init_all = sub.add_parser(
        "init-all", help="write and validate v3 reports for every target"
    )
    init_all.add_argument("output", type=Path)
    init_all.set_defaults(handler=_run_init_all)
    check = sub.add_parser(
        "validate", help="pre-apply report or isolated transaction check"
    )
    check.add_argument("target")
    check.add_argument("report", type=Path)
    check.add_argument("--transaction", help="validate one KIND:NAME transaction")
    check.set_defaults(handler=_run_validate)
    proof = sub.add_parser(
        "verify", help="prove a captured transaction applied exactly"
    )
    proof.add_argument("target")
    proof.add_argument("report", type=Path)
    proof.add_argument("--transaction", required=True, help="KIND:NAME transaction")
    proof.set_defaults(handler=_run_verify)
    return parser


def _report(args: argparse.Namespace) -> dict[str, Any]:
    return json.loads(args.report.read_text(encoding="utf-8"))


def _run_prepare(args: argparse.Namespace) -> int:
    from .naming_audit import prepare

    payload = prepare(resolved_root(args), args.target, repair=args.repair)
    _print(payload)
    return 0 if payload["ready"] else 1


def _run_init(args: argparse.Namespace) -> int:
    from .naming_audit import initialize

    payload = initialize(resolved_root(args), args.target)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    _print(
        {
            "schema": SCHEMA_V3,
            "target": payload["target"],
            "rows": len(payload["rows"]),
            "output": args.output.as_posix(),
        }
    )
    return 0


def _run_init_all(args: argparse.Namespace) -> int:
    from .naming_audit import initialize_all

    _print(initialize_all(resolved_root(args), args.output))
    return 0


def _run_validate(args: argparse.Namespace) -> int:
    from .naming_audit import validate

    _print(
        validate(
            resolved_root(args),
            args.target,
            _report(args),
            transaction=args.transaction,
        )
    )
    return 0


def _run_verify(args: argparse.Namespace) -> int:
    from .naming_audit import verify

    _print(verify(resolved_root(args), args.target, _report(args), args.transaction))
    return 0


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)
