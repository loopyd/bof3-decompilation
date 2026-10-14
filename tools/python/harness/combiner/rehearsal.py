"""Temporarily consolidate a pinned group, compare it and restore owned PRE."""

from __future__ import annotations

import json
import hashlib
import os
from contextlib import closing
from pathlib import Path

from harness.build.preservation import (
    read_preservation_document,
    validate_preservation_size,
)
from harness.build.routing import select_preservation
from harness.match.grouped import compare_members, resolve_comparison_outputs
from harness.combiner.recovery import (
    prepare_recovery_manifest,
    validate_recovery_manifest,
)
from harness.combiner.transactions import verify_transaction
from harness.common.digests import digest
from harness.common.evidence import evidence_output_path, write_new_evidence_output
from harness.common.files import preflight_existing_replacements, read_file
from harness.common.git import git_index_backup
from harness.common.inputs import InputBatch
from harness.common.lease import exclude_writers, require_writer
from harness.common.observation import PathWatch
from harness.common.paths import require_absent, validate_paths
from harness.common.process import ProcessCleanupError
from harness.common.runtime import apply_changes, rollback
from harness.common.safeguards import capture_safeguards, verify_restored_state
from harness.common.submodules import protect_artifacts
from harness.common.workspace import (
    adopted_baseline,
    verify_workspace,
)
from harness.domain.policy import validate_matching_text

SCHEMA = "bof3.combiner-rehearsal-result/v3"
_LIMIT = 64 * 1024 * 1024


def _capture_inputs(root: Path, names: set[str]) -> dict:
    result = {}
    with InputBatch(root) as batch:
        for name in sorted(names):
            state, _ = batch.read(root / name, max_bytes=_LIMIT, include_metadata=True)
            if state is not None and state["metadata"]["st_nlink"] != 1:
                raise ValueError(f"rehearsal input is hardlinked: {name}")
            result[name] = state
    return result


def _verify_post(root: Path, transaction: dict) -> None:
    with InputBatch(root) as batch:
        for name, expected in transaction["post"].items():
            if batch.read(root / name, max_bytes=_LIMIT)[0] != expected:
                raise ValueError(f"rehearsal POST differs from exact images: {name}")


def _restore_pre(root, manifest, backup, images, index, safeguards) -> None:
    try:
        require_writer(root)
        rollback(root, backup, images)
        verify_restored_state(root, manifest, index, safeguards)
        validate_recovery_manifest(root, manifest, rederive=True)
        require_writer(root)
    except ProcessCleanupError:
        raise
    except BaseException as error:
        raise RuntimeError(
            "combiner rehearsal restoration failed or is unverified; "
            "preserve recovery evidence and external state for parent review"
        ) from error


@protect_artifacts
@exclude_writers
def rehearse_transaction(
    root: Path,
    transaction: dict,
    *,
    expected_fingerprint: str,
    implementation_run_id: str,
    object_path: str,
    output: str,
    adopted_workspace: str | None = None,
    output_limit: int = 2 * 1024 * 1024,
) -> dict:
    """Always restore PRE; a comparison result never accepts a consolidation."""
    require_writer(root)
    if not isinstance(implementation_run_id, str) or not implementation_run_id.strip():
        raise ValueError("rehearsal requires a nonempty implementation run ID")
    if type(output_limit) is not int or not 1 <= output_limit <= _LIMIT:
        raise ValueError("invalid rehearsal native output bound")
    output = evidence_output_path(root, output)
    require_absent(root, output)
    recovery_output = evidence_output_path(root, output + ".recovery.json")
    require_absent(root, recovery_output)
    _, _, artifacts = resolve_comparison_outputs(root, object_path)
    transaction = verify_transaction(
        root, transaction, expected_fingerprint=expected_fingerprint
    )
    allowed = validate_paths(root, transaction["allowed_paths"])
    preflight_existing_replacements(root, allowed)
    for name in allowed:
        parent = (root / name).parent
        if not parent.is_dir() or parent.resolve() != parent:
            raise ValueError("rehearsal requires existing canonical source parents")
        text = transaction["changes"][name]
        if name.endswith(".c") and text is not None:
            validate_matching_text(text)
    record = transaction["preservation"]
    source = root / record["profile"]["destination"]
    with closing(select_preservation(root, [source], [source])) as selection:
        if (
            selection.record is None
            or selection.fingerprint != record["fingerprint"]
            or read_preservation_document(selection.record) != record
        ):
            raise ValueError("rehearsal requires the prepared preservation route")
        protected = {root / name for name in record["inputs"]}
        protected.update(root / name for name in transaction["owner_inputs"])
        protected.update(selection.reserved | {selection.record})
        if selection.routes is not None:
            protected.add(selection.routes)
        if artifacts & protected or {root / output, root / recovery_output} & (
            protected | artifacts
        ):
            raise ValueError("rehearsal outputs collide with retained inputs")
        baseline = adopted_baseline(root, {"adopted_baseline": adopted_workspace})
        manifest = prepare_recovery_manifest(
            root,
            transaction,
            baseline,
            implementation_run_id,
        )
        validate_recovery_manifest(root, manifest)
        index = git_index_backup(root)
        if index is None:
            raise ValueError("rehearsal requires a captured Git index for recovery")
        safeguards = capture_safeguards(root, set(), baseline["state"], index)
        names = set(record["inputs"]) | set(transaction["owner_inputs"])
        names.difference_update(allowed)
        inputs = _capture_inputs(root, names)
        environment = digest(dict(os.environ))
        backup, images = {}, {}
        recovery_receipt = {}

        def retain_recovery(reference):
            require_writer(root)
            if reference["path"] in {output, recovery_output}:
                raise ValueError("recovery record collides with rehearsal publication")
            receipt = {
                "schema": "bof3.combiner-recovery-receipt/v1",
                "transaction_fingerprint": expected_fingerprint,
                "manifest_digest": manifest["digest"],
                "implementation_run_id": implementation_run_id,
                "recovery": reference,
                "restoration_authority": False,
            }
            receipt["digest"] = digest(receipt)
            write_new_evidence_output(root, recovery_output, receipt)
            encoded = (json.dumps(receipt, indent=2, sort_keys=True) + "\n").encode()
            if read_file(root, recovery_output, max_bytes=len(encoded)) != encoded:
                raise ValueError(
                    "rehearsal recovery receipt changed during publication"
                )
            recovery_receipt.update(
                path=recovery_output, sha256=hashlib.sha256(encoded).hexdigest()
            )

        with closing(PathWatch({root / name for name in names})) as watch:

            def guard():
                require_writer(root)
                selection.validate()
                watch.validate()
                if (
                    _capture_inputs(root, names) != inputs
                    or digest(dict(os.environ)) != environment
                    or git_index_backup(root) != index
                ):
                    raise ValueError(
                        "rehearsal inputs, environment or Git index changed"
                    )

            guard()
            verify_transaction(
                root, transaction, expected_fingerprint=expected_fingerprint
            )
            verify_restored_state(root, manifest, index, safeguards)
            try:
                backup, images = apply_changes(
                    root,
                    transaction["changes"],
                    allowed,
                    recovery={
                        "owner": "combiner",
                        "manifest": manifest,
                        "implementation_run_id": implementation_run_id,
                        "output": output,
                    },
                    workspace=baseline["state"],
                    index=index,
                    deletions=set(transaction["deletions"]),
                    recovery_callback=retain_recovery,
                )
                guard()
                _verify_post(root, transaction)
                comparison = compare_members(
                    root,
                    source.relative_to(root).as_posix(),
                    object_path,
                    output_limit=output_limit,
                )
                guard()
                _verify_post(root, transaction)
                verify_workspace(root, baseline["state"], allowed)
            except ProcessCleanupError:
                raise
            except BaseException:
                _restore_pre(root, manifest, backup, images, index, safeguards)
                raise
            else:
                _restore_pre(root, manifest, backup, images, index, safeguards)
            guard()
            require_absent(root, output)
            result = {
                "schema": SCHEMA,
                "transaction_fingerprint": expected_fingerprint,
                "implementation_run_id": implementation_run_id,
                "manifest_digest": manifest["digest"],
                "recovery_receipt": recovery_receipt,
                "comparison": comparison,
                "restored": True,
                "status": "restored",
                "comparison_status": comparison["status"],
                "all_function_bytes_match": comparison["all_function_bytes_match"],
                "all_data_bytes_match": comparison["all_data_bytes_match"],
                "all_native_bytes_match": comparison["all_native_bytes_match"],
                "native_verified": False,
                "coverage_verified": False,
                "accepted": False,
                "reusable": False,
                "write_authorized": False,
            }
            result["digest"] = digest(result)
            validate_preservation_size(result)
            write_new_evidence_output(root, output, result)
            expected_bytes = (
                json.dumps(result, indent=2, sort_keys=True) + "\n"
            ).encode()
            if read_file(root, output, max_bytes=_LIMIT) != expected_bytes:
                raise ValueError("rehearsal report changed during publication")
            guard()
            return result
