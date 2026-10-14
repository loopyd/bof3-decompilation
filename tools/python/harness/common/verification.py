"""CLI transport for type/macro revalidation and parent-authorized recovery."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.common.cli import resolved_root
from harness.common.continuation import resolve_disposition
from harness.common.evidence import evidence_output_path
from harness.common.files import atomic_write, read_file
from harness.common.inspection import inspect_recovery
from harness.common.restoration import restore_sources
from harness.io import unique_object


def _read(path: Path) -> object:
    return json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=unique_object)


def _inspect_recovery(args: argparse.Namespace) -> int:
    result = inspect_recovery(
        resolved_root(args),
        args.record,
        args.expected_recovery_digest,
        owner=args.recovery_owner,
        manifest_validator=args.recovery_manifest_validator,
    )
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


def _restore_sources(args: argparse.Namespace) -> int:
    result = restore_sources(
        resolved_root(args),
        args.record,
        args.expected_recovery_digest,
        args.authorization,
        args.expected_authorization_digest,
        owner=args.recovery_owner,
        manifest_validator=args.recovery_manifest_validator,
    )
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


def add_recovery_commands(sub: argparse._SubParsersAction, owner, validator) -> None:
    parser = sub.add_parser(
        "inspect-recovery",
        help="inspect pinned recovery evidence without restoring files",
    )
    parser.add_argument("record", help="canonical repo-relative recovery record path")
    parser.add_argument("--expected-recovery-digest", required=True)
    parser.set_defaults(
        handler=_inspect_recovery,
        recovery_owner=owner,
        recovery_manifest_validator=validator,
    )
    restore = sub.add_parser(
        "recover",
        help="restore owned PRE with pinned parent authority and matching guards",
    )
    restore.add_argument("record", help="canonical repo-relative recovery record path")
    restore.add_argument("--expected-recovery-digest", required=True)
    restore.add_argument("--authorization", required=True)
    restore.add_argument("--expected-authorization-digest", required=True)
    restore.set_defaults(
        handler=_restore_sources,
        recovery_owner=owner,
        recovery_manifest_validator=validator,
    )


def _resume(args: argparse.Namespace) -> int:
    result = resolve_disposition(
        resolved_root(args),
        _read(args.manifest),
        args.expected_manifest_digest,
        args.implementation_run_id,
        _read(args.application),
        args.expected_application_digest,
        owner=args.resume_owner,
        manifest_validator=args.resume_manifest_validator,
        verify_application=args.resume_verify_application,
        verify_reviewed=args.resume_verify_reviewed,
        envelope=_read(args.reviewed_envelope)
        if args.reviewed_envelope is not None
        else None,
        expected_envelope_digest=args.expected_envelope_digest,
    )
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


def add_resume_command(
    sub: argparse._SubParsersAction, owner, validator, verify, review
) -> None:
    parser = sub.add_parser(
        "resume", help="inspect request-bound review or accepted-skip disposition"
    )
    parser.add_argument("manifest", type=Path)
    parser.add_argument("application", type=Path)
    parser.add_argument("--expected-manifest-digest", required=True)
    parser.add_argument("--expected-application-digest", required=True)
    parser.add_argument("--implementation-run-id", required=True)
    parser.add_argument("--reviewed-envelope", type=Path)
    parser.add_argument("--expected-envelope-digest")
    parser.set_defaults(
        handler=_resume,
        resume_owner=owner,
        resume_manifest_validator=validator,
        resume_verify_application=verify,
        resume_verify_reviewed=review,
    )


def _revalidate(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    record = args.revalidation_owner.revalidate_application(
        root,
        _read(args.proof),
        args.expected_envelope_digest,
        execution_run_id=args.execution_run_id,
        adopted_baseline=args.adopted_baseline,
        intervening=_read(args.intervening) if args.intervening is not None else None,
        output=evidence_output_path(root, args.output.as_posix()),
    )
    print(
        json.dumps(
            {"schema": record["schema"], "checked": True, "digest": record["digest"]},
            indent=2,
            sort_keys=True,
        )
    )
    return 0


def _verify(args: argparse.Namespace) -> int:
    result = args.verify_record(
        resolved_root(args), _read(args.proof), args.expected_digest
    )
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


def _review(args: argparse.Namespace) -> int:
    root = resolved_root(args)
    output = evidence_output_path(root, args.output.as_posix())
    if read_file(root, output, missing_ok=True) is not None:
        raise FileExistsError(output)
    owner = args.revalidation_owner
    envelope = owner.review_revalidation(
        root,
        _read(args.proof),
        _read(args.parent_attestation),
        args.expected_revalidation_digest,
    )
    atomic_write(
        root,
        output,
        (json.dumps(envelope, indent=2, sort_keys=True) + "\n").encode(),
        exclusive=True,
    )
    result = owner.verify_reviewed_revalidation(root, envelope, envelope["digest"])
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


def add_revalidation_commands(sub: argparse._SubParsersAction, owner) -> None:
    revalidate = sub.add_parser(
        "revalidate",
        help="run fresh checks on a reviewed private result without applying edits",
    )
    revalidate.add_argument("proof", type=Path)
    revalidate.add_argument("output", type=Path)
    revalidate.add_argument("--expected-envelope-digest", required=True)
    revalidate.add_argument("--execution-run-id", required=True)
    revalidate.add_argument("--adopted-baseline", required=True)
    revalidate.add_argument(
        "--intervening",
        type=Path,
        help="JSON list of ordered externally pinned private envelopes",
    )
    revalidate.set_defaults(handler=_revalidate, revalidation_owner=owner)
    review = sub.add_parser(
        "review-revalidation",
        help="ingest independent parent acceptance of fresh checks",
    )
    review.add_argument("proof", type=Path)
    review.add_argument("output", type=Path)
    review.add_argument("--expected-revalidation-digest", required=True)
    review.add_argument("--parent-attestation", type=Path, required=True)
    review.set_defaults(handler=_review, revalidation_owner=owner)
    for command, flag, verify in (
        (
            "verify-revalidation",
            "--expected-revalidation-digest",
            owner.verify_revalidation,
        ),
        (
            "final-verify-revalidation",
            "--expected-envelope-digest",
            owner.verify_reviewed_revalidation,
        ),
    ):
        parser = sub.add_parser(
            command,
            help="replay externally pinned revalidation evidence without new checks",
        )
        parser.add_argument("proof", type=Path)
        parser.add_argument(flag, dest="expected_digest", required=True)
        parser.set_defaults(handler=_verify, verify_record=verify)
