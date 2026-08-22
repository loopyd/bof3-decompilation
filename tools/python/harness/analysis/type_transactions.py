"""Evidence-gated, atomic type application transactions and candidate accounting."""

from __future__ import annotations

import json
import subprocess
from pathlib import Path
from typing import Any, Callable

from ..domain import load_target_manifests, normalize_target_id, resolve_function
from .index import connect
from .type_candidate_review import (
    artifact_paths,
    candidate_account as _candidate_account,
    digest,
    validate_reviewed_candidate,
)
from .transaction_files import preflight_existing_replacements
from .type_shared_proofs import private_proofs, proof_dependencies
from .type_transaction_checks import capture_partial_baselines, required_checks
from .transaction_workspace import (
    adopted_baseline as _adopted_baseline,
    workspace_baseline as _workspace_baseline,
    workspace_state as _workspace_state,
)
from .type_transaction_runtime import (
    APPLICATION_SCHEMA,
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

ACCOUNT_SCHEMA = "bof3.type-candidate-account/v1"
REQUEST_SCHEMA = "bof3.type-transaction-request/v2"
MANIFEST_SCHEMA = "bof3.type-transaction/v2"
CONCERNS = frozenset({"alias", "layout", "field", "prototype", "shared"})
workspace_baseline = _workspace_baseline


def _target(value: object, manifests: dict[str, Any]) -> str:
    if not isinstance(value, str):
        raise ValueError("type transaction target must be canonical")
    target = normalize_target_id(value).value
    if target != value or target not in manifests:
        raise ValueError(f"unknown or non-canonical type transaction target: {value}")
    return target


def _header(root: Path, manifest: Any, concern: str, value: object) -> str:
    try:
        header = canonical_repo_path(value)
        validate_paths(root, {header})
        exists = file_state(root, {header})[header] is not None
    except (OSError, ValueError):
        header = ""
        exists = False
    if not exists:
        raise ValueError("header must be an existing repo-relative file")
    if concern == "shared":
        relative = Path(header)
        private_headers = set(manifest.headers)
        if (
            not relative.is_relative_to("include")
            or relative.suffix != ".h"
            or relative.name.endswith("_internal.h")
            or header in private_headers
        ):
            raise ValueError(
                "shared type transaction header is not a sanctioned public shared path"
            )
    elif header not in manifest.headers:
        raise ValueError(f"type transaction header is not claimed by target: {header}")
    return header


def _validate_shared_header(
    root: Path,
    header: str,
    reviewed: list[dict[str, Any]],
    proofs: list[dict[str, Any]],
) -> None:
    if any(header not in item["candidate"]["locations"] for item in reviewed):
        raise ValueError(
            "shared type header must be a reviewed candidate location for every owner"
        )
    if any(header not in proof_dependencies(root, proof) for proof in proofs):
        raise ValueError(
            "shared type header must be a private proof dependency for every owner"
        )


def _index_rows(
    connection: Any, ids: object, targets: set[str]
) -> list[dict[str, Any]]:
    if (
        not isinstance(ids, list)
        or not ids
        or any(not isinstance(item, str) for item in ids)
    ):
        raise ValueError("index_candidate_ids must be non-empty strings")
    if len({item.casefold() for item in ids}) != len(ids):
        raise ValueError("index_candidate_ids must be unique")
    rows = []
    for candidate_id in ids:
        row = connection.execute(
            "SELECT id,target_id,address,end,kind,evidence_class,width,signedness,status,"
            "representation_status,semantic_status,evidence,blocker FROM type_candidates WHERE id=?",
            (candidate_id,),
        ).fetchone()
        if row is None:
            raise ValueError(f"unknown type candidate lead: {candidate_id}")
        item = dict(row)
        item["evidence"] = json.loads(item["evidence"])
        if item["target_id"] not in targets:
            raise ValueError(
                f"type candidate belongs to another target: {candidate_id}"
            )
        rows.append(item)
    return sorted(rows, key=lambda item: item["id"])


def _functions(
    root: Path, connection: Any, values: object, targets: set[str]
) -> list[dict[str, str]]:
    if (
        not isinstance(values, list)
        or not values
        or any(not isinstance(item, str) for item in values)
    ):
        raise ValueError("affected_functions must be a non-empty array")
    result = []
    for selector in sorted(set(values)):
        if "@0x" not in selector:
            raise ValueError(f"invalid affected function selector: {selector}")
        target, raw = selector.rsplit("@0x", 1)
        if target not in targets:
            raise ValueError(f"affected function selector has wrong target: {selector}")
        try:
            address = int(raw, 16)
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


def _private_proofs(
    root: Path, values: object, manifests: dict[str, Any]
) -> list[dict[str, Any]]:
    return private_proofs(
        root,
        values,
        manifests,
        normalize_target=_target,
        verify_application=verify_application,
    )


def prepare_transaction(root: Path, request: object) -> dict[str, Any]:
    if not isinstance(request, dict) or request.get("schema") != REQUEST_SCHEMA:
        raise ValueError(f"type transaction request schema must be {REQUEST_SCHEMA}")
    concern = request.get("concern")
    if concern not in CONCERNS:
        raise ValueError(f"unknown type transaction concern: {concern}")
    manifests = load_target_manifests(root)
    target = _target(request.get("target"), manifests)
    header = _header(root, manifests[target], concern, request.get("header"))
    if concern == "shared" and any(header in m.headers for m in manifests.values()):
        raise ValueError(
            "shared type transaction header is not a sanctioned public shared path"
        )
    targets = {target}
    if concern == "shared":
        values = request.get("shared_targets")
        if not isinstance(values, list) or len(values) < 2:
            raise ValueError("shared transaction requires at least two owners")
        targets = {_target(item, manifests) for item in values}
        if target not in targets or len(targets) < 2:
            raise ValueError(
                "shared transaction owners must include the primary target"
            )
    connection = connect(root)
    try:
        rows = _index_rows(connection, request.get("index_candidate_ids"), targets)
        functions = _functions(
            root, connection, request.get("affected_functions"), targets
        )
    finally:
        connection.close()
    paths = [
        canonical_repo_path(path)
        for path in artifact_paths(request.get("candidate_artifacts"))
    ]
    validate_paths(root, paths)
    if len(paths) != len(rows):
        raise ValueError("every index lead requires one reviewed candidate artifact")
    reviewed = [
        validate_reviewed_candidate(root, path, concern, row)
        for path, row in zip(paths, rows)
    ]
    proofs = (
        _private_proofs(root, request.get("private_transaction_proofs"), manifests)
        if concern == "shared"
        else []
    )
    if concern == "shared":
        candidate_targets = {item["candidate"]["target"] for item in reviewed}
        proof_targets = {item["application"]["target"] for item in proofs}
        if candidate_targets != targets or proof_targets != targets:
            raise ValueError("shared promotion must prove every declared owner")
        _validate_shared_header(root, header, reviewed, proofs)
    allowed = validate_paths(
        root, {header, *(canonical_repo_path(item["source"]) for item in functions)}
    )
    preflight_existing_replacements(root, allowed)
    pre_state = file_state(root, allowed)
    partial_baselines = capture_partial_baselines(root, functions)
    facts = {
        "schema": MANIFEST_SCHEMA,
        "target": target,
        "targets": sorted(targets),
        "concern": concern,
        "header": header,
        "reviewed_candidates": reviewed,
        "private_transaction_proofs": proofs,
        "affected_functions": functions,
        "allowed_paths": sorted(allowed),
        "pre_state": pre_state,
        "pre_state_digest": digest(pre_state),
        "workspace_baseline": _adopted_baseline(root, request),
        "required_checks": required_checks(
            sorted(targets), functions, partial_baselines
        ),
        "request": request,
    }
    return {**facts, "digest": digest(facts)}


def _manifest(root: Path, value: object, *, rederive: bool = False) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ValueError("type transaction manifest must be an object")
    facts = {key: item for key, item in value.items() if key != "digest"}
    if facts.get("schema") != MANIFEST_SCHEMA or value.get("digest") != digest(facts):
        raise ValueError("type transaction manifest drifted")
    if rederive:
        try:
            canonical = prepare_transaction(root, value["request"])
        except (KeyError, TypeError, ValueError) as error:
            raise ValueError(
                "type transaction manifest cannot be re-derived"
            ) from error
        if value != canonical:
            raise ValueError("type transaction manifest is not canonical")
    try:
        expected = {
            canonical_repo_path(value["header"]),
            *(
                canonical_repo_path(item["source"])
                for item in value["affected_functions"]
            ),
        }
        supplied = set(value["allowed_paths"])
    except (KeyError, TypeError, ValueError) as error:
        raise ValueError("type transaction manifest paths are invalid") from error
    if supplied != expected or len(value["allowed_paths"]) != len(expected):
        raise ValueError("type transaction manifest allowed_paths are forged")
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
    if (
        file_state(root, allowed) != manifest["pre_state"]
        or _workspace_state(root) != manifest["workspace_baseline"]["state"]
    ):
        raise ValueError("type transaction base drifted")
    validate_paths(root, allowed)
    full_backup = workspace_backup(root)
    validate_paths(root, allowed)
    backup: dict[str, bytes | None] = {}
    quarantines = {}
    index_state, index_backup = git_index_state(root), git_index_backup(root)
    expected_index = index_backup
    try:
        backup, quarantines = apply_changes(root, changes, allowed)
        post = file_state(root, allowed)
        changed = changed_paths(manifest["pre_state"], post)
        if not changed or set(changed) != set(backup):
            raise ValueError("type transaction changed-path set is incomplete")
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
            )
            receipts.extend(current)
            if (expected_index := git_index_backup(root)) != index_backup:
                raise ValueError("type transaction validation changed the Git index")
            if file_state(root, allowed) != post:
                raise ValueError("type transaction validation mutated an allowed path")
            if not passed:
                break
        state = _workspace_state(root)
        baseline = manifest["workspace_baseline"]["state"]
        outside = {name: state[name] for name in state.keys() - allowed}
        if outside != {name: baseline[name] for name in baseline.keys() - allowed}:
            raise ValueError("type transaction changed an unrelated path")
        if not passed:
            raise RuntimeError("type transaction validation failed")
        application = application_record(
            manifest["digest"], manifest["pre_state"], post, changed, receipts
        )
        candidate = manifest["reviewed_candidates"][0]
        application.update(
            target=manifest["target"],
            concern=manifest["concern"],
            representation=candidate["representation"],
            semantics=candidate["semantics"],
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
        application["attestation"] = write_attestation(root, application)
        if (expected_index := git_index_backup(root)) != index_backup:
            raise ValueError("type transaction changed the Git index before proof")
        return application
    except BaseException:
        try:
            validate_paths(root, allowed)
            restore_git_index(root, index_backup, expected_index)
            rollback(root, backup, quarantines)
            rollback_workspace(root, full_backup)
        except BaseException as error:
            raise RuntimeError("type transaction rollback failed") from error
        if (
            file_state(root, allowed) != manifest["pre_state"]
            or _workspace_state(root) != manifest["workspace_baseline"]["state"]
            or git_index_state(root) != index_state
        ):
            raise RuntimeError("type transaction rollback failed")
        raise


def verify_application(
    root: Path, value: object, expected_application_digest: str
) -> dict[str, Any]:
    validate_attestation(root, value, expected_application_digest)
    if (
        not isinstance(value, dict)
        or value.get("schema") != APPLICATION_SCHEMA
        or not value.get("applied")
    ):
        raise ValueError("type application proof is invalid")
    facts = {key: value[key] for key in value.keys() - {"attestation", "digest"}}
    if value.get("digest") != digest(facts):
        raise ValueError("type application proof drifted")
    manifest = _manifest(root, value.get("manifest"))
    if (
        value["manifest_digest"] != manifest["digest"]
        or value["pre_state"] != manifest["pre_state"]
    ):
        raise ValueError("type application proof does not match manifest")
    allowed = validate_paths(root, manifest["allowed_paths"])
    if file_state(root, allowed) != value["post_state"]:
        raise ValueError("type application post-state drifted")
    validate_receipts(
        root,
        value.get("receipts"),
        manifest["required_checks"],
        value["post_state_digest"],
    )
    if value.get("changed_paths_digest") != digest(value.get("changed_paths")) or value[
        "receipt_digests"
    ] != [item["digest"] for item in value["receipts"]]:
        raise ValueError("type application receipt or changed-path set drifted")
    return {
        "schema": APPLICATION_SCHEMA,
        "target": value["target"],
        "concern": value["concern"],
        "applied": True,
        "digest": value["digest"],
    }


def candidate_account(root: Path) -> dict[str, Any]:
    return _candidate_account(root, connect)


def validate_account(root: Path, report: object) -> dict[str, Any]:
    current = candidate_account(root)
    if report != current:
        raise ValueError("type candidate account is stale, incomplete, or duplicated")
    return current
