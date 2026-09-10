"""CLI transport for unchanged macro assessment and disposition evidence."""

from __future__ import annotations

import json
from pathlib import Path

from harness.common.cli import resolved_root
from harness.common.evidence import write_new_evidence_output
from harness.macros.assessment import prepare_assessment
from harness.macros.disposition import (
    account_existing,
    inspect_existing,
    review_existing,
    verify_existing,
)


def add_inspection_commands(sub, read) -> None:
    def account(args):
        print(
            json.dumps(
                account_existing(resolved_root(args), read(args.references)),
                indent=2,
                sort_keys=True,
            )
        )
        return 0

    def prepare(args):
        print(
            json.dumps(
                prepare_assessment(resolved_root(args), read(args.request)),
                indent=2,
                sort_keys=True,
            )
        )
        return 0

    def inspect(args):
        record = inspect_existing(
            resolved_root(args),
            read(args.manifest),
            args.expected_manifest_digest,
            implementation_run_id=args.implementation_run_id,
            output=args.output.as_posix(),
            deadline=args.deadline,
        )
        print(
            json.dumps(
                {
                    "digest": record["digest"],
                    "checked": True,
                    "accepted": False,
                    "safe_application_count": 0,
                },
                indent=2,
            )
        )
        return 0

    def review(args):
        root = resolved_root(args)
        record = review_existing(
            root,
            read(args.inspection),
            read(args.parent_review),
            args.expected_inspection_digest,
        )
        write_new_evidence_output(root, args.output.as_posix(), record)
        print(
            json.dumps(
                verify_existing(root, record, record["digest"]),
                indent=2,
                sort_keys=True,
            )
        )
        return 0

    def verify(args):
        print(
            json.dumps(
                verify_existing(
                    resolved_root(args),
                    read(args.disposition),
                    args.expected_envelope_digest,
                ),
                indent=2,
                sort_keys=True,
            )
        )
        return 0

    command = sub.add_parser(
        "account-existing",
        help="count explicitly pinned existing dispositions without counting applications",
    )
    command.add_argument("references", type=Path)
    command.set_defaults(handler=account)
    command = sub.add_parser(
        "prepare-existing", help="freeze an unchanged existing-abstraction assessment"
    )
    command.add_argument("request", type=Path)
    command.set_defaults(handler=prepare)
    command = sub.add_parser(
        "check-existing", help="check every existing consumer without source edits"
    )
    command.add_argument("manifest", type=Path)
    command.add_argument("--expected-manifest-digest", required=True)
    command.add_argument("--implementation-run-id", required=True)
    command.add_argument("--output", type=Path, required=True)
    command.add_argument("--deadline", type=float)
    command.set_defaults(handler=inspect)
    command = sub.add_parser(
        "review-existing", help="accept independently reviewed unchanged macro evidence"
    )
    command.add_argument("inspection", type=Path)
    command.add_argument("parent_review", type=Path)
    command.add_argument("--expected-inspection-digest", required=True)
    command.add_argument("--output", type=Path, required=True)
    command.set_defaults(handler=review)
    command = sub.add_parser(
        "verify-existing",
        help="verify an externally pinned existing-abstraction disposition",
    )
    command.add_argument("disposition", type=Path)
    command.add_argument("--expected-envelope-digest", required=True)
    command.set_defaults(handler=verify)
