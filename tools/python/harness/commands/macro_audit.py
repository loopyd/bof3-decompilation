"""CLI for fresh macro opportunity candidate accounting."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from ..analysis.macro_accounting import candidate_account, validate_account
from ..analysis.macro_transactions import (
    prepare_transaction,
    run_transaction,
    verify_application,
)
from ..analysis.transaction_evidence import evidence_output_path, write_evidence_output
from ._common import add_root_argument, resolved_root, run_main


def _read(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def _summary(report: dict[str, Any]) -> dict[str, Any]:
    return {
        key: report[key]
        for key in (
            "schema",
            "complete",
            "fresh",
            "candidate_count",
            "safe_application_count",
            "counts",
            "source_input_fingerprint",
        )
    }


def _account(args: argparse.Namespace) -> int:
    report = candidate_account(resolved_root(args))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    print(json.dumps(_summary(report), indent=2, sort_keys=True))
    return 0


def _validate_account(args: argparse.Namespace) -> int:
    report = validate_account(resolved_root(args), _read(args.report))
    print(json.dumps(_summary(report), indent=2, sort_keys=True))
    return 0


def _prepare(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    output = evidence_output_path(root, args.output.as_posix())
    manifest = prepare_transaction(root, _read(args.request))
    write_evidence_output(root, output, manifest)
    print(json.dumps({"schema": manifest["schema"], "concern": manifest["concern"]}))
    return 0


def _run(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    output = evidence_output_path(root, args.output.as_posix())
    application = run_transaction(root, _read(args.manifest), _read(args.changes))
    write_evidence_output(root, output, application)
    print(
        json.dumps(
            {
                "schema": application["schema"],
                "applied": True,
                "application_digest": application["digest"],
            }
        )
    )
    return 0


def _verify(args: argparse.Namespace) -> int:
    proof = verify_application(
        resolved_root(args), _read(args.proof), args.expected_application_digest
    )
    print(json.dumps(proof, indent=2, sort_keys=True))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="macro-audit")
    add_root_argument(parser)
    sub = parser.add_subparsers(dest="command", required=True)
    account = sub.add_parser(
        "account", help="write exactly-once accounting for fresh macro opportunities"
    )
    account.add_argument("output", type=Path)
    account.set_defaults(handler=_account)
    validate = sub.add_parser(
        "validate-account", help="validate an account against current fresh inputs"
    )
    validate.add_argument("report", type=Path)
    validate.set_defaults(handler=_validate_account)
    prepare = sub.add_parser("prepare", help="capture one reviewed macro transaction")
    prepare.add_argument("request", type=Path)
    prepare.add_argument("output", type=Path)
    prepare.set_defaults(handler=_prepare)
    run = sub.add_parser("run", help="atomically apply reviewed macro changes")
    run.add_argument("manifest", type=Path)
    run.add_argument("changes", type=Path)
    run.add_argument("output", type=Path)
    run.set_defaults(handler=_run)
    verify = sub.add_parser(
        "verify", help="verify an immutable macro application proof"
    )
    verify.add_argument("proof", type=Path)
    verify.add_argument("--expected-application-digest", required=True)
    verify.set_defaults(handler=_verify)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
