"""CLI adapter for ``bin/naming-audit``."""

from __future__ import annotations

import argparse
import json
import sqlite3
from contextlib import contextmanager
from pathlib import Path
from typing import Any

from harness.common.cli import add_root_argument, resolved_root, run_main
from harness.naming.context import SCHEMA_V3
from harness.naming.opportunities import (
    collect_inventory,
    collect_opportunities,
    describe_opportunity,
)
from harness.naming.readiness import transaction_scope


def add_query_commands(sub: argparse._SubParsersAction) -> None:
    scope = sub.add_parser(
        "transaction-scope",
        help="derive the exact tracked files one spelling rename must touch",
    )
    scope.add_argument("target")
    scope.add_argument("symbol", help="current raw or semantic symbol name")
    scope.set_defaults(query_handler=_query_scope, query_labeled=True)
    inventory = sub.add_parser(
        "inventory", help="list target-local raw function and data naming debt"
    )
    inventory.add_argument("target")
    inventory.set_defaults(query_handler=_query_inventory, query_requires_index=False)


def _query_scope(
    args: argparse.Namespace, connection: sqlite3.Connection
) -> list[dict[str, Any]]:
    return [transaction_scope(resolved_root(args), args.target, args.symbol)]


def _query_inventory(
    args: argparse.Namespace, connection: None
) -> list[dict[str, Any]]:
    return collect_inventory(resolved_root(args), args.target)


def _print(payload: dict[str, Any]) -> None:
    print(json.dumps(payload, indent=2, sort_keys=True))


class _SingleEvidenceRoot(argparse.Action):
    """Preserve one raw evidence-root spelling for trust-boundary validation."""

    def __call__(self, parser, namespace, values, option_string=None):
        if getattr(namespace, self.dest, None) is not None:
            parser.error("--evidence-root may be specified once")
        setattr(namespace, self.dest, values)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="naming-audit")
    add_root_argument(parser)
    sub = parser.add_subparsers(dest="command", required=True)
    opportunities = sub.add_parser(
        "opportunities", help="list read-only target-local raw symbol naming leads"
    )
    opportunities.add_argument("target")
    opportunities.set_defaults(handler=_run_opportunities)
    describe = sub.add_parser(
        "describe-opportunity", help="inspect one exact naming lead without mutation"
    )
    describe.add_argument("target")
    describe.add_argument("id")
    describe.add_argument("--expected-fingerprint")
    describe.set_defaults(handler=_run_opportunities)
    prep = sub.add_parser(
        "prepare", help="readiness preflight; --repair closes proven repairs"
    )
    prep.add_argument("target")
    prep.add_argument("--repair", action="store_true")
    prep.add_argument(
        "--rows",
        help="comma-separated prepared row selectors (KIND:NAME), sorted and "
        "unique; repairs only those rows (empty value repairs nothing)",
    )
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
    reconcile = sub.add_parser(
        "reconcile", help="preview additive canonical target inventory reconciliation"
    )
    reconcile.add_argument("target")
    reconcile.add_argument("--apply", action="store_true")
    reconcile.add_argument(
        "--evidence-root",
        action=_SingleEvidenceRoot,
        help="canonical absolute evidence root used for retained receipts",
    )
    reconcile.set_defaults(handler=_run_reconcile)
    check = sub.add_parser(
        "validate", help="pre-apply report or isolated transaction check"
    )
    check.add_argument("target")
    check.add_argument("report", type=Path)
    check.add_argument("--transaction", help="validate one KIND:NAME transaction")
    check.add_argument(
        "--evidence-root",
        action=_SingleEvidenceRoot,
        help="canonical absolute evidence root used for retained receipts",
    )
    check.set_defaults(handler=_run_validate)
    transaction = sub.add_parser(
        "prepare-transaction",
        help="atomically bind one ready FUNCTION or DATA proposal",
    )
    transaction.add_argument("target")
    transaction.add_argument("report", type=Path)
    transaction.add_argument(
        "--transaction", required=True, help="function:NAME or data:NAME"
    )
    transaction.add_argument(
        "--candidate", type=Path, help="reviewed proposed FUNCTION or DATA row JSON"
    )
    transaction.add_argument(
        "--expected-sha256", help="reviewed current report byte SHA-256"
    )
    transaction.add_argument("--evidence-root", action=_SingleEvidenceRoot)
    transaction.set_defaults(handler=_run_prepare_transaction)
    conclusion = sub.add_parser(
        "conclude", help="import one explicit expert-authored row conclusion"
    )
    conclusion.add_argument("target")
    conclusion.add_argument("report", type=Path)
    conclusion.add_argument("input", type=Path)
    conclusion.add_argument(
        "--evidence-root",
        action=_SingleEvidenceRoot,
        help="one canonically spelled absolute evidence root used during collection",
    )
    conclusion.set_defaults(handler=_run_conclude)
    proof = sub.add_parser(
        "verify", help="prove a captured transaction applied exactly"
    )
    proof.add_argument("target")
    proof.add_argument("report", type=Path)
    proof.add_argument("--transaction", required=True, help="KIND:NAME transaction")
    proof.add_argument("--evidence-root", action=_SingleEvidenceRoot)
    proof.add_argument("--post-apply-receipts", type=Path)
    proof.set_defaults(handler=_run_verify)
    for name in ("postapply-gates", "postapply-review"):
        native = sub.add_parser(name, help="bounded native postapply lifecycle")
        native.add_argument("target")
        native.add_argument("report", type=Path)
        native.add_argument("--transaction", required=True)
        native.add_argument("--evidence-root", action=_SingleEvidenceRoot)
        if name == "postapply-gates":
            native.add_argument("--implementation-run-id", required=True)
        else:
            native.add_argument("--gates", type=Path, required=True)
            native.add_argument("--parent-attestation", type=Path, required=True)
        native.set_defaults(handler=_run_postapply)
    terminal = sub.add_parser(
        "terminal-verify", help="read-only parent-pinned terminal acceptance"
    )
    terminal.add_argument("target")
    terminal.add_argument("report", type=Path)
    terminal.add_argument("--transaction", required=True)
    terminal.add_argument("--parent-attestation", type=Path, required=True)
    terminal.add_argument("--expected-parent-digest", required=True)
    terminal.add_argument("--evidence-root", action=_SingleEvidenceRoot)
    terminal.set_defaults(handler=_run_terminal_verify)
    return parser


def _run_opportunities(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    payload = (
        collect_opportunities(root, args.target)
        if args.command == "opportunities"
        else describe_opportunity(
            root, args.target, args.id, expected_fingerprint=args.expected_fingerprint
        )
    )
    print(json.dumps(payload, indent=2, sort_keys=True))
    return 0


def _report(args: argparse.Namespace) -> dict[str, Any]:
    return json.loads(args.report.read_text(encoding="utf-8"))


def _run_prepare(args: argparse.Namespace) -> int:
    from harness.naming.audit import prepare

    # Omitted --rows stays None (legacy full repair); a supplied value, even
    # empty or commas-only, is an explicit row list that repairs nothing.
    rows = None if args.rows is None else [row for row in args.rows.split(",") if row]
    payload = prepare(resolved_root(args), args.target, repair=args.repair, rows=rows)
    _print(payload)
    return 0 if payload["ready"] else 1


def _run_init(args: argparse.Namespace) -> int:
    from harness.naming.audit import initialize
    from harness.naming.proposal import canonical_report_path

    root = resolved_root(args)
    output = canonical_report_path(root, args.output)
    payload = initialize(root, args.target)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    _print(
        {
            "schema": SCHEMA_V3,
            "target": payload["target"],
            "rows": len(payload["rows"]),
            "output": output.as_posix(),
        }
    )
    return 0


def _run_init_all(args: argparse.Namespace) -> int:
    from harness.naming.audit import initialize_all

    _print(initialize_all(resolved_root(args), args.output))
    return 0


@contextmanager
def _evidence_context(value):
    """Select explicit receipt and namespace roots, restoring both on every exit."""
    from harness.domain.receipts import reset_receipt_root, set_receipt_root
    from harness.naming.namespace import (
        canonical_evidence_root,
        reset_evidence_root,
        set_evidence_root,
    )

    root = None if value is None else canonical_evidence_root(value)
    token = set_evidence_root(root)
    try:
        receipt_token = set_receipt_root(root)
        try:
            yield
        finally:
            reset_receipt_root(receipt_token)
    finally:
        reset_evidence_root(token)


def _run_reconcile(args: argparse.Namespace) -> int:
    from harness.naming.reconciliation import reconcile

    with _evidence_context(args.evidence_root):
        _print(reconcile(resolved_root(args), args.target, apply=args.apply))
    return 0


def _run_validate(args: argparse.Namespace) -> int:
    from harness.naming.audit import validate
    from harness.naming.proposal import canonical_report_path

    root = resolved_root(args)
    args.report = canonical_report_path(root, args.report)
    with _evidence_context(args.evidence_root):
        _print(
            validate(
                root,
                args.target,
                _report(args),
                transaction=args.transaction,
                report_path=args.report,
            )
        )
    return 0


def _run_prepare_transaction(args: argparse.Namespace) -> int:
    from harness.io import unique_object
    from harness.naming.audit import prepare_transaction

    candidate = None
    if args.candidate is not None:
        candidate = json.loads(
            args.candidate.read_text(encoding="utf-8"), object_pairs_hook=unique_object
        )
        if not isinstance(candidate, dict):
            raise ValueError("candidate must be a JSON object")
    with _evidence_context(args.evidence_root):
        _print(
            prepare_transaction(
                resolved_root(args),
                args.target,
                args.report,
                args.transaction,
                candidate=candidate,
                expected_sha256=args.expected_sha256,
            )
        )
    return 0


def _run_conclude(args: argparse.Namespace) -> int:
    from harness.naming.conclusion import import_conclusion
    from harness.naming.namespace import canonical_evidence_root

    evidence_root = (
        None
        if args.evidence_root is None
        else canonical_evidence_root(args.evidence_root)
    )
    _print(
        import_conclusion(
            resolved_root(args),
            args.target,
            args.report,
            args.input.resolve(),
            evidence_root=evidence_root,
        )
    )
    return 0


def _run_verify(args: argparse.Namespace) -> int:
    from harness.naming.audit import verify
    from harness.naming.proposal import canonical_report_path

    root = resolved_root(args)
    args.report = canonical_report_path(root, args.report)
    with _evidence_context(args.evidence_root):
        _print(
            verify(
                root,
                args.target,
                _report(args),
                args.transaction,
                report_path=args.report,
                post_apply_receipts=args.post_apply_receipts,
            )
        )
    return 0


def _run_postapply(args: argparse.Namespace) -> int:
    from harness.naming.application import produce
    from harness.naming.review import ingest

    with _evidence_context(args.evidence_root):
        arguments = (resolved_root(args), args.target, args.report, args.transaction)
        path = (
            produce(*arguments, args.implementation_run_id)
            if args.command == "postapply-gates"
            else ingest(
                *arguments, args.gates.resolve(), args.parent_attestation.resolve()
            )
        )
        _print({"bundle": str(path)})
    return 0


def _run_terminal_verify(args: argparse.Namespace) -> int:
    from harness.naming.proposal import canonical_report_path
    from harness.naming.terminal import verify_terminal

    root = resolved_root(args)
    with _evidence_context(args.evidence_root):
        _print(
            verify_terminal(
                root,
                args.target,
                canonical_report_path(root, args.report),
                args.transaction,
                args.parent_attestation,
                args.expected_parent_digest,
            )
        )
    return 0


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
