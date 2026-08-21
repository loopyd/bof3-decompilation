"""CLI for type candidate accounting and reviewed application transactions."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from ..analysis.transaction_evidence import evidence_output_path, write_evidence_output
from ..analysis.type_transactions import (
    candidate_account,
    prepare_transaction,
    run_transaction,
    validate_account,
    verify_application,
    workspace_baseline,
)
from ._common import add_root_argument, resolved_root, run_main


def _read(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def _print(value: object) -> None:
    print(json.dumps(value, indent=2, sort_keys=True))


def _account(args: argparse.Namespace) -> int:
    report = candidate_account(resolved_root(args))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    _print(
        {
            key: report[key]
            for key in (
                "schema",
                "complete",
                "candidate_count",
                "safe_application_count",
                "counts",
            )
        }
    )
    return 0


def _validate_account(args: argparse.Namespace) -> int:
    report = validate_account(resolved_root(args), _read(args.report))
    _print(
        {
            key: report[key]
            for key in (
                "schema",
                "complete",
                "candidate_count",
                "safe_application_count",
                "counts",
            )
        }
    )
    return 0


def _baseline(args: argparse.Namespace) -> int:
    _print(workspace_baseline(resolved_root(args)))
    return 0


def _prepare(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    output = evidence_output_path(root, args.output.as_posix())
    manifest = prepare_transaction(root, _read(args.request))
    write_evidence_output(root, output, manifest)
    _print(
        {
            "schema": manifest["schema"],
            "target": manifest["target"],
            "concern": manifest["concern"],
            "output": args.output.as_posix(),
        }
    )
    return 0


def _run(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    output = evidence_output_path(root, args.output.as_posix())
    application = run_transaction(root, _read(args.manifest), _read(args.changes))
    write_evidence_output(root, output, application)
    _print(
        {
            "schema": application["schema"],
            "target": application["target"],
            "concern": application["concern"],
            "applied": application["applied"],
            "application_digest": application["digest"],
            "output": args.output.as_posix(),
        }
    )
    return 0


def _verify(args: argparse.Namespace) -> int:
    _print(
        verify_application(
            resolved_root(args), _read(args.proof), args.expected_application_digest
        )
    )
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="type-audit")
    add_root_argument(parser)
    sub = parser.add_subparsers(dest="command", required=True)
    account = sub.add_parser(
        "account", help="write an exactly-once report for every type candidate"
    )
    account.add_argument("output", type=Path)
    account.set_defaults(handler=_account)
    validate = sub.add_parser(
        "validate-account", help="validate an account against the fresh index"
    )
    validate.add_argument("report", type=Path)
    validate.set_defaults(handler=_validate_account)
    baseline = sub.add_parser(
        "baseline", help="print the exact clean/adopted workspace baseline digest"
    )
    baseline.set_defaults(handler=_baseline)
    prepare = sub.add_parser("prepare", help="capture one reviewed type transaction")
    prepare.add_argument("request", type=Path)
    prepare.add_argument("output", type=Path)
    prepare.set_defaults(handler=_prepare)
    run = sub.add_parser(
        "run", help="atomically apply reviewed changes and execute required checks"
    )
    run.add_argument("manifest", type=Path)
    run.add_argument("changes", type=Path)
    run.add_argument("output", type=Path)
    run.set_defaults(handler=_run)
    verify = sub.add_parser("verify", help="verify the immutable application proof")
    verify.add_argument("proof", type=Path)
    verify.add_argument("--expected-application-digest", required=True)
    verify.set_defaults(handler=_verify)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
