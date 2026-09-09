"""Type application acceptance and revalidation entry points."""

from __future__ import annotations

from pathlib import Path

from harness.common.process import run_command
from harness.common.review import _review, _verify_reviewed
from harness.types import transactions as type_transactions


def review_application(
    root: Path, application: object, parent: object, expected_application_digest: str
) -> dict:
    return _review(
        root,
        application,
        parent,
        expected_application_digest,
        "type",
        type_transactions.verify_application,
        manifest_validator=type_transactions._manifest,
    )


def verify_reviewed_application(
    root: Path, value: object, expected_envelope_digest: str
) -> dict:
    return _verify_reviewed(
        root,
        value,
        expected_envelope_digest,
        "type",
        type_transactions.verify_application,
        manifest_validator=type_transactions._manifest,
    )


def revalidate_application(
    root: Path,
    envelope: object,
    expected_envelope_digest: str,
    *,
    execution_run_id: str,
    adopted_baseline: str,
    runner=run_command,
    output: str | None = None,
    intervening: list | None = None,
) -> dict:
    from harness.common.revalidation import revalidate

    return revalidate(
        root,
        envelope,
        expected_envelope_digest,
        execution_run_id=execution_run_id,
        adopted_baseline=adopted_baseline,
        owner="type",
        verify=verify_reviewed_application,
        manifest_validator=type_transactions._manifest,
        runner=runner,
        output=output,
        intervening=intervening,
    )


def verify_revalidation(
    root: Path, value: object, expected_revalidation_digest: str
) -> dict:
    from harness.common.revalidation import verify_revalidation as verify

    return verify(
        root,
        value,
        expected_revalidation_digest,
        owner="type",
        verify=verify_reviewed_application,
        manifest_validator=type_transactions._manifest,
    )


def review_revalidation(
    root: Path, value: object, parent: object, expected_revalidation_digest: str
) -> dict:
    from harness.common.revalidation import review_revalidation as review

    return review(
        root,
        value,
        parent,
        expected_revalidation_digest,
        owner="type",
        verify=verify_reviewed_application,
        manifest_validator=type_transactions._manifest,
    )


def verify_reviewed_revalidation(
    root: Path, value: object, expected_envelope_digest: str
) -> dict:
    from harness.common.revalidation import verify_reviewed_revalidation as verify

    return verify(
        root,
        value,
        expected_envelope_digest,
        owner="type",
        verify=verify_reviewed_application,
        manifest_validator=type_transactions._manifest,
    )
