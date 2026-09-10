"""Deterministic lift diagnosis and candidate audit CLI; no agent execution."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import resolved_root
from harness.common.journal import read_record
from harness.decomp.diagnosis import run_diagnosis
from harness.decomp.audit import run_audit


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
    audit.add_argument(
        "--proposal-format", choices=("mission", "unmeasured"), default="mission"
    )
    audit.set_defaults(handler=audit_request)


def audit_request(args: argparse.Namespace) -> int:
    result = run_audit(
        resolved_root(args),
        args.diagnosis,
        args.proposal,
        expected_diagnosis_digest=args.expected_diagnosis_digest,
        expected_proposal_sha256=args.expected_proposal_sha256,
        output=args.output,
        unmeasured=args.proposal_format == "unmeasured",
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
