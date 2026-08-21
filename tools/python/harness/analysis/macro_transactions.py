"""Reviewed, atomic macro and template application transactions."""

from __future__ import annotations

import subprocess
from pathlib import Path
from typing import Any, Callable

from ..domain import load_target_manifests, normalize_target_id, resolve_function
from ..domain.receipts import sha256_file
from . import macro_accounting
from .index import connect
from .macro_transaction_review import (
    REVIEW_SCHEMA,
    exact_proofs,
    reviewed_artifact,
)
from .type_candidate_review import digest
from .type_transaction_checks import capture_partial_baselines, required_checks
from .type_transaction_runtime import (
    apply_changes,
    application_record,
    canonical_repo_path,
    changed_paths,
    file_state,
    git_index_backup,
    git_index_state,
    restore_git_index,
    rollback,
    rollback_workspace,
    run_checks,
    validate_attestation,
    validate_paths,
    validate_receipts,
    workspace_backup,
    write_attestation,
)
from .type_transactions import _workspace_state, workspace_baseline

REQUEST_SCHEMA = "bof3.macro-transaction-request/v1"
MANIFEST_SCHEMA = "bof3.macro-transaction/v1"
RECEIPT_SCHEMA = "bof3.macro-command-receipt/v1"
APPLICATION_SCHEMA = "bof3.macro-application/v1"
ATTESTATION_SCHEMA = "bof3.macro-application-attestation/v1"
CONCERNS = frozenset({"constant", "expression", "local_template", "shared_template"})


def _target(value: object, manifests: dict[str, Any]) -> str:
    if not isinstance(value, str):
        raise ValueError("macro transaction target must be canonical")
    try:
        target = normalize_target_id(value).value
    except ValueError as error:
        raise ValueError(
            f"unknown or non-canonical macro transaction target: {value}"
        ) from error
    if target != value or target not in manifests:
        raise ValueError(f"unknown or non-canonical macro transaction target: {value}")
    return target


def _functions(
    root: Path, connection: Any, values: object, targets: set[str]
) -> list[dict[str, str]]:
    if (
        not isinstance(values, list)
        or not values
        or any(not isinstance(item, str) for item in values)
    ):
        raise ValueError("affected_functions must be non-empty strings")
    result = []
    for selector in sorted(set(values)):
        if "@0x" not in selector:
            raise ValueError(f"invalid affected function selector: {selector}")
        target, raw = selector.rsplit("@0x", 1)
        if target not in targets:
            raise ValueError(f"affected function selector has wrong target: {selector}")
        try:
            address = int(raw, 16)
        except ValueError as error:
            raise ValueError(
                f"invalid affected function selector: {selector}"
            ) from error
        try:
            resolved = resolve_function(root, selector)
        except (OSError, RuntimeError, ValueError) as error:
            raise ValueError(
                f"affected function identity is not canonical: {selector}"
            ) from error
        row = connection.execute(
            "SELECT lift_status FROM functions WHERE target_id=? AND address=?",
            (target, address),
        ).fetchone()
        if (
            row is None
            or row[0] not in {"exact", "partial"}
            or resolved.source is None
            or resolved.compiled_symbol is None
        ):
            raise ValueError(
                f"affected function must be exact/partial and manifest-claimed: {selector}"
            )
        result.append(
            {
                "selector": selector,
                "target": target,
                "address": f"0x{address:08X}",
                "function": resolved.compiled_symbol,
                "status": row[0],
                "source": resolved.source.relative_to(root).as_posix(),
            }
        )
    return result


def prepare_transaction(root: Path, request: object) -> dict[str, Any]:
    if not isinstance(request, dict) or request.get("schema") != REQUEST_SCHEMA:
        raise ValueError(f"macro transaction request schema must be {REQUEST_SCHEMA}")
    concern = request.get("concern")
    if concern not in CONCERNS:
        raise ValueError(f"unknown macro transaction concern: {concern}")
    account = macro_accounting.candidate_account(root)
    if account["safe_application_count"] != 0:
        raise ValueError(
            "macro transaction requires zero current automatic applications"
        )
    manifests = load_target_manifests(root)
    target = _target(request.get("target"), manifests)
    targets = {target}
    if concern == "shared_template":
        shared_targets = request.get("shared_targets")
        if not isinstance(shared_targets, list):
            raise ValueError("shared template requires exactly two declared targets")
        targets = {_target(item, manifests) for item in shared_targets}
        if target not in targets or len(targets) != 2:
            raise ValueError("shared template requires exactly two declared targets")
    proof_refs = request.get("exact_function_proofs")
    if concern != "shared_template" and proof_refs not in (None, []):
        raise ValueError("private macro transactions cannot reference shared proofs")
    proofs = (
        exact_proofs(
            root,
            proof_refs,
            manifests,
            normalize_target=_target,
            verify_application=verify_application,
        )
        if concern == "shared_template"
        else []
    )
    if proofs and {item["target"] for item in proofs} != targets:
        raise ValueError("shared template proofs must cover both declared targets")
    reviewed = reviewed_artifact(
        root,
        request.get("candidate_artifact"),
        concern,
        account,
        manifests,
        proofs=proofs,
    )
    if concern != "shared_template" and not targets.issubset(
        set(reviewed["declared_targets"])
    ):
        raise ValueError("macro transaction target does not own reviewed paths")
    connection = connect(root)
    try:
        functions = _functions(
            root, connection, request.get("affected_functions"), targets
        )
    finally:
        connection.close()
    if concern == "local_template" and any(
        item["status"] != "exact" for item in functions
    ):
        raise ValueError("local template wrappers must be independently exact")
    allowed = validate_paths(
        root,
        {
            *(canonical_repo_path(owner) for owner in reviewed["owners"]),
            *(canonical_repo_path(item["source"]) for item in functions),
        },
    )
    pre_state = file_state(root, allowed)
    baseline = workspace_baseline(root)
    adopted = request.get("adopted_baseline")
    if baseline["state"] and adopted != baseline["digest"]:
        raise ValueError(
            "dirty worktree requires adopted_baseline equal to current workspace digest"
        )
    if not baseline["state"] and adopted not in {None, digest({})}:
        raise ValueError("adopted_baseline does not match the clean worktree")
    partial_baselines = capture_partial_baselines(root, functions)
    facts = {
        "schema": MANIFEST_SCHEMA,
        "target": target,
        "targets": sorted(targets),
        "concern": concern,
        "reviewed_opportunity": reviewed,
        "exact_function_proofs": proofs,
        "affected_functions": functions,
        "allowed_paths": sorted(allowed),
        "pre_state": pre_state,
        "pre_state_digest": digest(pre_state),
        "workspace_baseline": baseline,
        "required_checks": required_checks(
            sorted(targets), functions, partial_baselines
        ),
        "request": request,
    }
    return {**facts, "digest": digest(facts)}


def _manifest(root: Path, value: object, *, rederive: bool = False) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ValueError("macro transaction manifest must be an object")
    facts = {key: item for key, item in value.items() if key != "digest"}
    if facts.get("schema") != MANIFEST_SCHEMA or value.get("digest") != digest(facts):
        raise ValueError("macro transaction manifest drifted")
    if rederive:
        try:
            canonical = prepare_transaction(root, value["request"])
        except (KeyError, TypeError, ValueError) as error:
            raise ValueError(
                "macro transaction manifest cannot be re-derived"
            ) from error
        if value != canonical:
            raise ValueError("macro transaction manifest is not canonical")
    try:
        expected = {
            *(
                canonical_repo_path(name)
                for name in value["reviewed_opportunity"]["owners"]
            ),
            *(
                canonical_repo_path(item["source"])
                for item in value["affected_functions"]
            ),
        }
        supplied = set(value["allowed_paths"])
    except (KeyError, TypeError, ValueError) as error:
        raise ValueError("macro transaction manifest paths are invalid") from error
    if supplied != expected or len(value["allowed_paths"]) != len(expected):
        raise ValueError("macro transaction manifest allowed_paths are forged")
    return value


def run_transaction(
    root: Path,
    manifest_value: object,
    changes: object,
    *,
    runner: Callable[..., subprocess.CompletedProcess[str]] = subprocess.run,
) -> dict[str, Any]:
    manifest = _manifest(root, manifest_value, rederive=True)
    allowed = validate_paths(root, manifest["allowed_paths"])
    proofs = manifest["exact_function_proofs"]
    try:
        proof_drifted = any(
            (root / item["path"]).is_symlink()
            or not (root / item["path"]).is_file()
            or sha256_file(root / item["path"]) != item["sha256"]
            for item in proofs
        )
    except (KeyError, TypeError):
        proof_drifted = True
    if proof_drifted:
        raise ValueError("shared template proof drifted")
    if (
        file_state(root, allowed) != manifest["pre_state"]
        or _workspace_state(root) != manifest["workspace_baseline"]["state"]
    ):
        raise ValueError("macro transaction base drifted")
    validate_paths(root, allowed)
    full_backup = workspace_backup(root)
    validate_paths(root, allowed)
    backup: dict[str, bytes | None] = {}
    quarantines: dict[str, tuple[str | None, bytes]] = {}
    index_state = git_index_state(root)
    index_backup = git_index_backup(root)
    expected_index = index_backup
    try:
        backup, quarantines = apply_changes(root, changes, allowed)
        post = file_state(root, allowed)
        changed = changed_paths(manifest["pre_state"], post)
        if not changed or set(changed) != set(backup):
            raise ValueError("macro transaction changed-path set is incomplete")
        post_digest = digest(post)
        receipts = []
        passed = True
        for check in manifest["required_checks"]:
            current, passed = run_checks(
                root,
                [check],
                post_digest,
                runner=runner,
                start_index=len(receipts),
                receipt_schema=RECEIPT_SCHEMA,
                evidence_prefix="macro",
            )
            receipts.extend(current)
            current_index = git_index_backup(root)
            if current_index != index_backup:
                expected_index = current_index
                raise ValueError("macro transaction validation changed the Git index")
            if file_state(root, allowed) != post:
                raise ValueError("macro transaction validation mutated an allowed path")
            if not passed:
                break
        state = _workspace_state(root)
        outside = {name: state[name] for name in state.keys() - allowed}
        baseline = manifest["workspace_baseline"]["state"]
        if outside != {name: baseline[name] for name in baseline.keys() - allowed}:
            raise ValueError("macro transaction changed an unrelated path")
        if not passed:
            raise RuntimeError("macro transaction validation failed")
        application = application_record(
            manifest["digest"],
            manifest["pre_state"],
            post,
            changed,
            receipts,
            schema=APPLICATION_SCHEMA,
        )
        opportunity = manifest["reviewed_opportunity"]
        application.update(
            target=manifest["target"],
            concern=manifest["concern"],
            semantic_guards=opportunity["semantic_guards"],
            observations=opportunity["observations"],
            exact_function_proofs=(
                [
                    item["selector"]
                    for item in manifest["affected_functions"]
                    if item["status"] == "exact"
                ]
                if manifest["concern"] == "local_template"
                else [item["selector"] for item in manifest["exact_function_proofs"]]
            ),
            manifest=manifest,
            receipts=receipts,
            quarantine_paths={
                name: quarantine
                for name, (quarantine, _installed) in quarantines.items()
                if quarantine is not None
            },
            applied=True,
        )
        application["digest"] = digest(
            {key: item for key, item in application.items() if key != "digest"}
        )
        application["attestation"] = write_attestation(
            root,
            application,
            schema=ATTESTATION_SCHEMA,
            prefix="macro",
        )
        current_index = git_index_backup(root)
        if current_index != index_backup:
            expected_index = current_index
            raise ValueError("macro transaction changed the Git index before proof")
        return application
    except BaseException:
        try:
            validate_paths(root, allowed)
            restore_git_index(root, index_backup, expected_index)
            rollback(root, backup, quarantines)
            rollback_workspace(root, full_backup)
        except BaseException as error:
            raise RuntimeError("macro transaction rollback failed") from error
        if (
            file_state(root, allowed) != manifest["pre_state"]
            or _workspace_state(root) != manifest["workspace_baseline"]["state"]
            or git_index_state(root) != index_state
        ):
            raise RuntimeError("macro transaction rollback failed")
        raise


def verify_application(
    root: Path, value: object, expected_application_digest: str
) -> dict[str, Any]:
    validate_attestation(
        root,
        value,
        expected_application_digest,
        schema=ATTESTATION_SCHEMA,
        prefix="macro",
    )
    if (
        not isinstance(value, dict)
        or value.get("schema") != APPLICATION_SCHEMA
        or not value.get("applied")
    ):
        raise ValueError("macro application proof is invalid")
    if value.get("digest") != digest(
        {
            key: item
            for key, item in value.items()
            if key not in {"attestation", "digest"}
        }
    ):
        raise ValueError("macro application proof drifted")
    manifest = _manifest(root, value.get("manifest"))
    if (
        value["manifest_digest"] != manifest["digest"]
        or value["pre_state"] != manifest["pre_state"]
    ):
        raise ValueError("macro application proof does not match manifest")
    allowed = validate_paths(root, manifest["allowed_paths"])
    if file_state(root, allowed) != value["post_state"]:
        raise ValueError("macro application post-state drifted")
    validate_receipts(
        root,
        value.get("receipts"),
        manifest["required_checks"],
        value["post_state_digest"],
        receipt_schema=RECEIPT_SCHEMA,
    )
    if value.get("changed_paths_digest") != digest(value.get("changed_paths")) or value[
        "receipt_digests"
    ] != [item["digest"] for item in value["receipts"]]:
        raise ValueError("macro application receipt or changed-path set drifted")
    return {
        "schema": APPLICATION_SCHEMA,
        "target": value["target"],
        "concern": value["concern"],
        "applied": True,
        "digest": value["digest"],
    }


__all__ = [
    "APPLICATION_SCHEMA",
    "ATTESTATION_SCHEMA",
    "CONCERNS",
    "MANIFEST_SCHEMA",
    "RECEIPT_SCHEMA",
    "REQUEST_SCHEMA",
    "REVIEW_SCHEMA",
    "prepare_transaction",
    "run_transaction",
    "verify_application",
]
