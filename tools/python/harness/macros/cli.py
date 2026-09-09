"""CLI for macro candidate inspection, accounting, and reviewed transactions."""

from __future__ import annotations

import argparse
import json
import sqlite3
from pathlib import Path
from typing import Any

from harness.common.cli import add_root_argument, resolved_root, run_main
from harness.common.evidence import evidence_output_path, write_evidence_output
from harness.common.verification import add_recovery_commands, add_revalidation_commands
from harness.io import unique_object
from harness.macros import application as macro_application_review
from harness.macros.accounting import candidate_account, validate_account
from harness.macros.application import review_application, verify_reviewed_application
from harness.macros.blocks import block_report, validate_minimum
from harness.macros.impact import describe_definition_impact
from harness.macros.opportunities import macro_opportunities_payload
from harness.macros.queries import macro_uses_payload, macros_payload
from harness.macros.resolution import describe_candidate
from harness.macros.selection import add_ranking_commands
from harness.macros.similarity import near_duplicates_payload
from harness.macros.transactions import (
    _manifest,
    prepare_transaction,
    run_transaction,
    verify_application,
)


def add_query_commands(sub: argparse._SubParsersAction) -> None:
    macros = sub.add_parser("macros", help="list indexed existing macros and templates")
    macros.add_argument("pattern", nargs="?")
    macros.add_argument("--target")
    macros.add_argument("--classification")
    macros.set_defaults(query_handler=_query_definitions)
    uses = sub.add_parser("macro-uses", help="list indexed macro expansion sites")
    uses.add_argument("pattern", nargs="?")
    uses.add_argument("--target")
    uses.set_defaults(query_handler=_query_uses)
    opportunities = sub.add_parser(
        "macro-opportunities",
        help="rank blocked repetitive-source and exact-group leads",
    )
    opportunities.add_argument("--target")
    opportunities.add_argument(
        "--kind",
        choices=("constant", "expression_accessor", "statement_window", "exact_group"),
    )
    opportunities.set_defaults(query_handler=_query_opportunities)
    similarity = sub.add_parser(
        "near-duplicates", help="rank blocked immediate-only function similarity leads"
    )
    similarity.add_argument("--target")
    similarity.set_defaults(query_handler=_query_similarity)


def _query_definitions(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return macros_payload(
        connection,
        target=args.target,
        pattern=args.pattern,
        classification=args.classification,
        limit=args.limit,
    )


def _query_uses(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return macro_uses_payload(
        connection, target=args.target, pattern=args.pattern, limit=args.limit
    )


def _query_opportunities(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return macro_opportunities_payload(
        connection,
        resolved_root(args),
        target=args.target,
        kind=args.kind,
        limit=args.limit,
    )


def _query_similarity(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return near_duplicates_payload(
        connection, resolved_root(args), target=args.target, limit=args.limit
    )


def _read(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=unique_object)


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
    options = (
        {"block_min_instructions": args.min_instructions}
        if args.min_instructions is not None
        else {}
    )
    report = candidate_account(resolved_root(args), **options)
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


def _positive(value: str) -> int:
    try:
        return validate_minimum(int(value))
    except ValueError as error:
        raise argparse.ArgumentTypeError(str(error)) from error


def _blocks(args: argparse.Namespace) -> int:
    report = block_report(
        resolved_root(args),
        min_instructions=args.min_instructions,
        target=args.target,
        limit=args.limit,
    )
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


def _impact(args: argparse.Namespace) -> int:
    report = describe_definition_impact(resolved_root(args), args.definition)
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


def _describe(args: argparse.Namespace) -> int:
    report = describe_candidate(
        resolved_root(args),
        args.candidate,
        target=args.target,
        min_instructions=args.min_instructions,
    )
    print(json.dumps(report, indent=2, sort_keys=True))
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
    options = (
        {"implementation_run_id": args.implementation_run_id}
        if getattr(args, "implementation_run_id", None) is not None
        else {}
    )
    if getattr(args, "participating_targets", None) is not None:
        options["participating_targets"] = args.participating_targets
    application = run_transaction(
        root, _read(args.manifest), _read(args.changes), output=output, **options
    )
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
    parser = argparse.ArgumentParser(prog="macro-audit")
    add_root_argument(parser)
    sub = parser.add_subparsers(dest="command", required=True)
    add_ranking_commands(sub, _read)
    blocks = sub.add_parser(
        "blocks", help="rank four-use assembly blocks, largest first"
    )
    blocks.add_argument("--min-instructions", type=_positive, required=True)
    blocks.add_argument(
        "--target", help="focus results without dropping global members"
    )
    blocks.add_argument(
        "--limit",
        type=int,
        default=20,
        help="output rows; zero means all, not an attempt budget",
    )
    blocks.set_defaults(handler=_blocks)
    impact = sub.add_parser(
        "impact", help="inspect global lexical consumers of an exact definition ID"
    )
    impact.add_argument("definition")
    impact.set_defaults(handler=_impact)
    describe = sub.add_parser(
        "describe", help="inspect a global candidate ID and its unresolved review gates"
    )
    describe.add_argument("candidate")
    describe.add_argument(
        "--min-instructions", type=_positive, help="required for assembly_block IDs"
    )
    describe.add_argument(
        "--target", help="focus the view without narrowing its fingerprint"
    )
    describe.set_defaults(handler=_describe)
    account = sub.add_parser(
        "account", help="write exactly-once accounting for fresh macro opportunities"
    )
    account.add_argument("output", type=Path)
    account.add_argument(
        "--min-instructions",
        type=_positive,
        help="include four-use assembly blocks with this explicit floor",
    )
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
    run.add_argument("--implementation-run-id")
    run.add_argument(
        "--participating-targets",
        nargs="+",
        help="sorted one/two-target evidence capture scope; requires implementation ID",
    )
    run.set_defaults(handler=_run)
    verify = sub.add_parser(
        "verify", help="verify an immutable macro application proof"
    )
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
    add_revalidation_commands(sub, macro_application_review)
    add_recovery_commands(sub, "macro", _manifest)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
