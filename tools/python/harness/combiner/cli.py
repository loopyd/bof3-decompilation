"""CLI adapters for read-only consolidation inspection and transaction preparation."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.common.cli import (
    add_root_argument,
    add_work_deadline_argument,
    resolved_root,
    run_main,
)
from harness.combiner.inspection import inspect_source
from harness.combiner.profiles import inspect_profiles
from harness.common.deadlines import check_deadline
from harness.build.preservation import (
    read_destination_source,
    read_preservation_document,
    verify_preservation,
)
from harness.combiner.preservation import capture_preservation
from harness.combiner.transactions import prepare_transaction, verify_transaction


def _inspect_source(args: argparse.Namespace) -> int:
    print(json.dumps(inspect_source(resolved_root(args), args.source), indent=2))
    return 0


def _inspect_profiles(args: argparse.Namespace) -> int:
    payload = inspect_profiles(
        resolved_root(args),
        args.target,
        args.destination,
        args.sources,
        expected_fingerprint=args.expected_fingerprint,
        destination_text=read_destination_source(
            args.destination_source,
            expected_sha256=args.expected_destination_sha256,
        ),
        migrate_configuration=args.migrate_configuration,
    )
    rendered = json.dumps(payload, indent=2)
    check_deadline()
    print(rendered)
    return 0


def _run_preservation(args: argparse.Namespace) -> int:
    if args.preservation_command == "capture":
        payload = capture_preservation(
            resolved_root(args),
            read_preservation_document(args.profile),
            read_preservation_document(
                args.post, expected_sha256=args.expected_post_sha256
            ),
            expected_profile_fingerprint=args.expected_profile_fingerprint,
        )
    else:
        payload = verify_preservation(
            resolved_root(args),
            read_preservation_document(args.record),
            expected_fingerprint=args.expected_fingerprint,
        )
    rendered = json.dumps(payload, indent=2, sort_keys=True)
    check_deadline()
    print(rendered)
    return 0


def _run_transaction(args: argparse.Namespace) -> int:
    if args.transaction_command == "prepare":
        payload = prepare_transaction(
            resolved_root(args),
            read_preservation_document(args.profile),
            read_preservation_document(
                args.images, expected_sha256=args.expected_images_sha256
            ),
            expected_profile_fingerprint=args.expected_profile_fingerprint,
        )
    else:
        payload = verify_transaction(
            resolved_root(args),
            read_preservation_document(args.record),
            expected_fingerprint=args.expected_fingerprint,
        )
    rendered = json.dumps(payload, indent=2, sort_keys=True)
    check_deadline()
    print(rendered)
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="combiner")
    add_root_argument(parser)
    commands = parser.add_subparsers(dest="command", required=True)
    inspect = commands.add_parser(
        "inspect-source", help="inspect leading function metadata"
    )
    inspect.add_argument("source")
    inspect.set_defaults(handler=_inspect_source)
    profile = commands.add_parser(
        "profile", help="check configured compiler profiles without building"
    )
    profile.add_argument("target")
    profile.add_argument("destination")
    profile.add_argument("sources", nargs="+")
    profile.add_argument("--expected-fingerprint")
    profile.add_argument("--destination-source", type=Path)
    profile.add_argument("--expected-destination-sha256")
    profile.add_argument("--migrate-configuration", action="store_true")
    add_work_deadline_argument(profile)
    profile.set_defaults(handler=_inspect_profiles)
    preservation = commands.add_parser(
        "preservation", help="freeze or verify a pinned PRE-to-POST profile transition"
    )
    stages = preservation.add_subparsers(dest="preservation_command", required=True)
    capture = stages.add_parser(
        "capture", help="pin POST states while PRE still exists"
    )
    capture.add_argument("profile", type=Path)
    capture.add_argument("post", type=Path)
    capture.add_argument("--expected-profile-fingerprint", required=True)
    capture.add_argument("--expected-post-sha256", required=True)
    add_work_deadline_argument(capture)
    capture.set_defaults(handler=_run_preservation)
    verify = stages.add_parser(
        "verify", help="verify live POST against an external PRE pin"
    )
    verify.add_argument("record", type=Path)
    verify.add_argument("--expected-fingerprint", required=True)
    add_work_deadline_argument(verify)
    verify.set_defaults(handler=_run_preservation)
    transaction = commands.add_parser(
        "transaction", help="prepare or verify exact joint images without applying"
    )
    stages = transaction.add_subparsers(dest="transaction_command", required=True)
    prepare = stages.add_parser("prepare", help="bind proposed images to live PRE")
    prepare.add_argument("profile", type=Path)
    prepare.add_argument("images", type=Path)
    prepare.add_argument("--expected-profile-fingerprint", required=True)
    prepare.add_argument("--expected-images-sha256", required=True)
    add_work_deadline_argument(prepare)
    prepare.set_defaults(handler=_run_transaction)
    verify = stages.add_parser(
        "verify", help="recheck a prepared record against live PRE"
    )
    verify.add_argument("record", type=Path)
    verify.add_argument("--expected-fingerprint", required=True)
    add_work_deadline_argument(verify)
    verify.set_defaults(handler=_run_transaction)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
