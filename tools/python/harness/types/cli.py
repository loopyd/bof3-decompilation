"""CLI for type candidate accounting and reviewed application transactions."""

from __future__ import annotations

import argparse
import json
import sqlite3
from pathlib import Path
from typing import Any

from harness.common.cli import add_root_argument, resolved_root, run_main
from harness.common.evidence import evidence_output_path, write_evidence_output
from harness.common.verification import (
    add_recovery_commands,
    add_resume_command,
    add_revalidation_commands,
)
from harness.io import unique_object
from harness.output import add_detail_argument, resolve_detail
from harness.types import application as type_application_review
from harness.types.application import review_application, verify_reviewed_application
from harness.types.queries import (
    type_candidates_payload,
    type_usages_payload,
    types_payload,
)
from harness.types.transactions import (
    _manifest,
    candidate_account,
    prepare_transaction,
    run_transaction,
    validate_account,
    verify_application,
    workspace_baseline,
)


def add_query_commands(sub: argparse._SubParsersAction) -> None:
    types = sub.add_parser("types", help="list target-owned type declarations")
    types.add_argument("pattern", nargs="?")
    types.add_argument("--target")
    types.add_argument("--untyped", action="store_true")
    add_detail_argument(types)
    types.set_defaults(query_handler=_query_declarations)
    uses = sub.add_parser("type-uses", help="list indexed type use sites")
    uses.add_argument("pattern", nargs="?")
    uses.add_argument("--target")
    add_detail_argument(uses)
    uses.set_defaults(query_handler=_query_uses)
    candidates = sub.add_parser(
        "type-candidates", help="list evidence-only representation candidates"
    )
    candidates.add_argument("--target")
    candidates.add_argument(
        "--status", choices=("blocked", "proposed", "accepted", "rejected")
    )
    candidates.add_argument("--kind")
    add_detail_argument(candidates)
    candidates.set_defaults(query_handler=_query_candidates)


def _query_declarations(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return types_payload(
        connection,
        target=args.target,
        pattern=args.pattern,
        untyped=args.untyped,
        limit=args.limit,
        detail=resolve_detail(requested=args.detail, json_output=args.json),
    )


def _query_uses(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return type_usages_payload(
        connection, target=args.target, pattern=args.pattern, limit=args.limit
    )


def _query_candidates(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return type_candidates_payload(
        connection,
        target=args.target,
        status=args.status,
        kind=args.kind,
        limit=args.limit,
    )


def _read(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=unique_object)


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
    options = (
        {"implementation_run_id": args.implementation_run_id}
        if getattr(args, "implementation_run_id", None) is not None
        else {}
    )
    if getattr(args, "participating_targets", None) is not None:
        options["participating_targets"] = args.participating_targets
    if getattr(args, "deadline", None) is not None:
        options["deadline"] = args.deadline
    application = run_transaction(
        root, _read(args.manifest), _read(args.changes), output=output, **options
    )
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


def _review(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    output = evidence_output_path(root, args.output.as_posix())
    envelope = review_application(
        root,
        _read(args.proof),
        _read(args.parent_attestation),
        args.expected_application_digest,
    )
    write_evidence_output(root, output, envelope)
    result = verify_reviewed_application(root, envelope, envelope["digest"])
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


def _final_verify(args: argparse.Namespace) -> int:
    print(
        json.dumps(
            verify_reviewed_application(
                resolved_root(args), _read(args.proof), args.expected_envelope_digest
            ),
            indent=2,
            sort_keys=True,
        )
    )
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness types")
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
    run.add_argument("--implementation-run-id")
    run.add_argument(
        "--deadline",
        type=float,
        help="original absolute monotonic work cutoff; not a duration",
    )
    run.add_argument(
        "--participating-targets",
        nargs="+",
        help="sorted one/two-target evidence capture scope; requires implementation ID",
    )
    run.set_defaults(handler=_run)
    verify = sub.add_parser("verify", help="verify the immutable application proof")
    verify.add_argument("proof", type=Path)
    verify.add_argument("--expected-application-digest", required=True)
    verify.set_defaults(handler=_verify)
    review = sub.add_parser(
        "review", help="ingest explicit supervising parent acceptance"
    )
    review.add_argument("proof", type=Path)
    review.add_argument("output", type=Path)
    review.add_argument("--parent-attestation", required=True, type=Path)
    review.add_argument("--expected-application-digest", required=True)
    review.set_defaults(handler=_review)
    final = sub.add_parser(
        "final-verify", help="verify an externally pinned parent envelope"
    )
    final.add_argument("proof", type=Path)
    final.add_argument("--expected-envelope-digest", required=True)
    final.set_defaults(handler=_final_verify)
    add_revalidation_commands(sub, type_application_review)
    add_recovery_commands(sub, "type", _manifest)
    add_resume_command(
        sub, "type", _manifest, verify_application, verify_reviewed_application
    )
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
