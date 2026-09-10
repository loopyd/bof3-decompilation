"""Explicit lift diagnosis, native writer, parent audit and retained review CLI."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import resolved_root
from harness.context.journal import read_record
from harness.decomp.diagnosis import run_diagnosis
from harness.decomp.audit import run_audit
from harness.decomp.dispatch import run_lift
from harness.decomp.review import run_review
from harness.context.arguments import add_dispatch_arguments, load_dispatch_inputs
from harness.context.capabilities import MODE


def register_commands(commands: argparse._SubParsersAction) -> None:
    diagnosis = commands.add_parser(
        "diagnose",
        help="cold native diagnosis of an existing lift; no model or source edits",
    )
    diagnosis.add_argument("request")
    diagnosis.add_argument("--expected-request-digest", required=True)
    diagnosis.add_argument("--output", required=True)
    diagnosis.add_argument("--deadline", required=True, type=float)
    diagnosis.set_defaults(handler=diagnose_request)
    audit = commands.add_parser(
        "audit",
        help="check a retained lift proposal against pinned diagnosis; no edits",
    )
    audit.add_argument("diagnosis")
    audit.add_argument("proposal")
    audit.add_argument("--expected-diagnosis-digest", required=True)
    audit.add_argument("--expected-proposal-sha256", required=True)
    audit.add_argument("--output", required=True)
    audit.add_argument("--expected-writer-digest")
    audit.add_argument(
        "--proposal-format", choices=("mission", "unmeasured"), default="mission"
    )
    audit.set_defaults(handler=audit_request)
    lift = commands.add_parser(
        "lift",
        help="one budgeted scoped-directory Codex proposal; stops for parent audit",
    )
    lift.add_argument("diagnosis")
    lift.add_argument("--expected-diagnosis-digest", required=True)
    lift.add_argument("--capabilities", choices=(MODE,), required=True)
    add_dispatch_arguments(lift)
    lift.set_defaults(handler=dispatch_lift)
    review = commands.add_parser(
        "review-lift",
        help="one read-only review of a pinned audited writer; no acceptance",
    )
    review.add_argument("audit")
    review.add_argument("--expected-audit-digest", required=True)
    review.add_argument("--expected-diagnosis-digest", required=True)
    review.add_argument("--expected-writer-digest", required=True)
    add_dispatch_arguments(review)
    review.set_defaults(handler=review_lift)


def review_lift(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    budget, chain, options = load_dispatch_inputs(root, args)
    result = run_review(
        root,
        args.audit,
        budget,
        chain,
        expected_audit_digest=args.expected_audit_digest,
        expected_diagnosis_digest=args.expected_diagnosis_digest,
        expected_writer_digest=args.expected_writer_digest,
        **options,
    )
    print(json.dumps(result), flush=True)
    return 0


def dispatch_lift(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    budget, chain, options = load_dispatch_inputs(root, args)
    result = run_lift(
        root,
        args.diagnosis,
        budget,
        chain,
        capabilities=args.capabilities,
        expected_diagnosis_digest=args.expected_diagnosis_digest,
        **options,
    )
    print(json.dumps(result), flush=True)
    return 0


def audit_request(args: argparse.Namespace) -> int:
    result = run_audit(
        resolved_root(args),
        args.diagnosis,
        args.proposal,
        expected_diagnosis_digest=args.expected_diagnosis_digest,
        expected_proposal_sha256=args.expected_proposal_sha256,
        output=args.output,
        unmeasured=args.proposal_format == "unmeasured",
        expected_writer_digest=args.expected_writer_digest,
    )
    print(json.dumps(result), flush=True)
    return 0


def diagnose_request(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    result = run_diagnosis(
        root,
        read_record(root, args.request),
        expected_request_digest=args.expected_request_digest,
        output=args.output,
        deadline=args.deadline,
    )
    print(json.dumps(result), flush=True)
    return 0
