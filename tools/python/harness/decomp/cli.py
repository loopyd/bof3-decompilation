"""Parent-only transport for fresh, source-read-only lift diagnosis."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import resolved_root
from harness.context.journal import read_record
from harness.decomp.diagnosis import run_diagnosis


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
