"""Atomic transaction application with confined, symlink-safe filesystem access."""

from __future__ import annotations

import hashlib
import json
import os
import secrets
import subprocess
from pathlib import Path
from typing import Any, Callable

from .transaction_files import (
    atomic_write as _atomic_write,
    canonical_repo_path,
    read_file as _read_file,
    restore_quarantined,
    safe_unlink as _safe_unlink,
)
from .transaction_git import (
    git_index_backup,
    git_index_state,
    restore_git_index,
    rollback_workspace,
    workspace_backup,
)
from .transaction_paths import file_state, validate_paths
from .type_candidate_review import digest
from .type_transaction_checks import check_evidence

RECEIPT_SCHEMA = "bof3.type-command-receipt/v1"
APPLICATION_SCHEMA = "bof3.type-application/v1"
ATTESTATION_SCHEMA = "bof3.type-application-attestation/v1"


def changed_paths(
    before: dict[str, str | None], after: dict[str, str | None]
) -> list[str]:
    return sorted(
        name for name in set(before) | set(after) if before.get(name) != after.get(name)
    )


def _write_receipt(
    root: Path,
    index: int,
    payload: dict[str, Any],
    *,
    schema: str = RECEIPT_SCHEMA,
    prefix: str = "type",
) -> dict[str, Any]:
    facts = {"schema": schema, **payload}
    record = {**facts, "digest": digest(facts)}
    name = f"{prefix}-run-{os.getpid()}-{index}-{secrets.token_hex(8)}.json"
    relative = f"out/reviews/evidence/{name}"
    _atomic_write(
        root,
        relative,
        (json.dumps(record, sort_keys=True) + "\n").encode(),
        expected=None,
        exclusive=True,
    )
    content = _read_file(root, relative)
    assert content is not None
    return {
        "path": relative,
        "sha256": hashlib.sha256(content).hexdigest(),
        "digest": record["digest"],
    }


def run_checks(
    root: Path,
    checks: list[dict[str, Any]],
    post_state_digest: str,
    *,
    runner: Callable[..., subprocess.CompletedProcess[str]] = subprocess.run,
    start_index: int = 0,
    receipt_schema: str = RECEIPT_SCHEMA,
    evidence_prefix: str = "type",
) -> tuple[list[dict[str, Any]], bool]:
    receipts, passed = [], True
    for index, check in enumerate(checks, start_index):
        argv = check["argv"]
        result = runner(argv, cwd=root, text=True, capture_output=True)
        evidence = check_evidence(check, result.returncode, result.stdout, root)
        output = (
            result.stdout
            if check.get("partial_baseline") is not None
            else (result.stdout + result.stderr)[-16000:]
        )
        payload = {
            "argv": argv,
            "command": check["command"],
            "status": "passed" if evidence["passed"] else "failed",
            "exit_code": result.returncode,
            "output": output,
            "metrics": evidence["metrics"],
            "evidence_digest": evidence["evidence_digest"],
            "target": check["target"],
            "selector": check["selector"],
            "post_state_digest": post_state_digest,
        }
        ref = _write_receipt(
            root, index, payload, schema=receipt_schema, prefix=evidence_prefix
        )
        receipts.append({**payload, **ref})
        if not evidence["passed"]:
            passed = False
            break
    return receipts, passed


def validate_receipts(
    root: Path,
    values: object,
    checks: list[dict[str, Any]],
    post_state_digest: str,
    *,
    receipt_schema: str = RECEIPT_SCHEMA,
) -> list[dict[str, Any]]:
    if not isinstance(values, list) or len(values) != len(checks):
        raise ValueError(
            "type transaction receipts do not exactly match required checks"
        )
    result = []
    for receipt, check in zip(values, checks):
        if not isinstance(receipt, dict):
            raise ValueError("invalid type command receipt")
        path_value = canonical_repo_path(receipt.get("path"))
        try:
            content = _read_file(root, path_value)
        except (OSError, ValueError, TypeError) as error:
            raise ValueError("type command receipt replaced") from error
        if content is None or hashlib.sha256(content).hexdigest() != receipt.get(
            "sha256"
        ):
            raise ValueError("type command receipt replaced")
        try:
            record = json.loads(content.decode())
        except (json.JSONDecodeError, UnicodeDecodeError) as error:
            raise ValueError("type command receipt replaced") from error
        if not isinstance(record, dict):
            raise ValueError("type command receipt replaced")
        facts = {key: item for key, item in record.items() if key != "digest"}
        if (
            record.get("schema") != receipt_schema
            or record.get("digest") != digest(facts)
            or receipt.get("digest") != record.get("digest")
        ):
            raise ValueError("type command receipt replaced")
        expected = {
            "argv": check["argv"],
            "command": check["command"],
            "target": check["target"],
            "selector": check["selector"],
            "post_state_digest": post_state_digest,
        }
        evidence = check_evidence(
            check, record.get("exit_code"), record.get("output", ""), root
        )
        if (
            any(record.get(key) != value for key, value in expected.items())
            or record.get("status") != "passed"
            or not evidence["passed"]
            or record.get("metrics") != evidence["metrics"]
            or record.get("evidence_digest") != evidence["evidence_digest"]
        ):
            raise ValueError("type command receipt does not match transaction")
        if any(receipt.get(key) != record.get(key) for key in facts if key != "schema"):
            raise ValueError("type command receipt replaced")
        result.append(receipt)
    return result


def apply_changes(
    root: Path, changes: object, allowed: set[str]
) -> tuple[dict[str, bytes | None], dict[str, tuple[str | None, bytes]]]:
    safe_allowed = validate_paths(root, allowed)
    if not isinstance(changes, dict) or not changes:
        raise ValueError("type application requires non-empty changes")
    if any(
        not isinstance(name, str) or canonical_repo_path(name) not in safe_allowed
        for name in changes
    ):
        raise ValueError("type application attempted an unowned path")
    if any(not isinstance(content, str) for content in changes.values()):
        raise ValueError("type application content must be text")
    backup = {name: _read_file(root, name, missing_ok=True) for name in changes}
    changed: dict[str, bytes | None] = {}
    records: dict[str, tuple[str | None, bytes]] = {}
    try:
        for name, content in changes.items():
            current = backup[name]
            installed = content.encode()
            quarantine = None
            if current is not None:
                quarantine = _safe_unlink(root, name, expected=current)
                assert quarantine is not None
                changed[name] = current
                records[name] = (quarantine, installed)
            _atomic_write(root, name, installed, expected=None)
            changed[name] = current
            records[name] = (quarantine, installed)
    except BaseException:
        rollback(root, changed, records)
        raise
    return backup, records


def rollback(
    root: Path,
    backup: dict[str, bytes | None],
    records: dict[str, tuple[str | None, bytes]] | None = None,
) -> None:
    errors = []
    for name, content in backup.items():
        try:
            current = _read_file(root, name, missing_ok=True)
            if records is None:
                _atomic_write(root, name, content or b"", expected=current)
                if content is None:
                    _safe_unlink(root, name, expected=b"")
            elif name in records:
                quarantine, installed = records[name]
                if current not in {None, installed}:
                    raise ValueError(
                        f"transaction path drifted during rollback: {name}"
                    )
                if current is not None:
                    _safe_unlink(root, name, expected=installed)
                if content is not None:
                    assert quarantine is not None
                    restore_quarantined(root, name, quarantine, expected=content)
        except (OSError, ValueError, RuntimeError) as error:
            errors.append(f"{name}: {error}")
    if errors:
        raise RuntimeError("type transaction rollback failed: " + "; ".join(errors))


def application_record(
    manifest_digest: str,
    pre_state: dict[str, str | None],
    post_state: dict[str, str | None],
    paths: list[str],
    receipts: list[dict[str, Any]],
    *,
    schema: str = APPLICATION_SCHEMA,
) -> dict[str, Any]:
    facts = {
        "schema": schema,
        "manifest_digest": manifest_digest,
        "pre_state": pre_state,
        "pre_state_digest": digest(pre_state),
        "post_state": post_state,
        "post_state_digest": digest(post_state),
        "changed_paths": paths,
        "changed_paths_digest": digest(paths),
        "receipt_digests": [item["digest"] for item in receipts],
    }
    return {**facts, "digest": digest(facts)}


def write_attestation(
    root: Path,
    application: dict[str, Any],
    *,
    schema: str = ATTESTATION_SCHEMA,
    prefix: str = "type",
) -> dict[str, str]:
    attestation_id = secrets.token_hex(16)
    facts = {
        "schema": schema,
        "id": attestation_id,
        "application_digest": application["digest"],
        "manifest_digest": application["manifest_digest"],
        "receipt_digests": application["receipt_digests"],
    }
    record = {**facts, "digest": digest(facts)}
    relative = f"out/reviews/evidence/{prefix}-attestation-{attestation_id}.json"
    _atomic_write(
        root,
        relative,
        (json.dumps(record, sort_keys=True) + "\n").encode(),
        expected=None,
        exclusive=True,
    )
    content = _read_file(root, relative)
    assert content is not None
    return {
        "path": relative,
        "sha256": hashlib.sha256(content).hexdigest(),
        "digest": record["digest"],
    }


def validate_attestation(
    root: Path,
    application: object,
    expected_application_digest: str,
    *,
    schema: str = ATTESTATION_SCHEMA,
    prefix: str = "type",
) -> dict[str, str]:
    if (
        not isinstance(application, dict)
        or not isinstance(expected_application_digest, str)
        or application.get("digest") != expected_application_digest
    ):
        raise ValueError("type application attestation is not trusted")
    value = application.get("attestation")
    if not isinstance(value, dict):
        raise ValueError("type application attestation is invalid")
    path_value = canonical_repo_path(value.get("path"))
    try:
        content = _read_file(root, path_value)
    except (OSError, ValueError, TypeError) as error:
        raise ValueError("type application attestation replaced") from error
    if content is None or hashlib.sha256(content).hexdigest() != value.get("sha256"):
        raise ValueError("type application attestation replaced")
    try:
        record = json.loads(content.decode())
    except (json.JSONDecodeError, UnicodeDecodeError) as error:
        raise ValueError("type application attestation replaced") from error
    facts = {key: item for key, item in record.items() if key != "digest"}
    expected_path = f"out/reviews/evidence/{prefix}-attestation-{record.get('id')}.json"
    if (
        path_value != expected_path
        or record.get("schema") != schema
        or record.get("digest") != digest(facts)
        or value.get("digest") != record.get("digest")
        or record.get("application_digest") != expected_application_digest
        or record.get("manifest_digest") != application["manifest_digest"]
        or record.get("receipt_digests") != application["receipt_digests"]
    ):
        raise ValueError("type application attestation replaced or reused")
    return value


__all__ = [
    "APPLICATION_SCHEMA",
    "ATTESTATION_SCHEMA",
    "apply_changes",
    "application_record",
    "canonical_repo_path",
    "changed_paths",
    "file_state",
    "git_index_backup",
    "git_index_state",
    "restore_git_index",
    "rollback",
    "rollback_workspace",
    "run_checks",
    "validate_attestation",
    "validate_paths",
    "validate_receipts",
    "workspace_backup",
    "write_attestation",
]
