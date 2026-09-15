"""Read-only, externally pinned inspection of transaction recovery evidence."""

from __future__ import annotations

import base64
import hashlib
import json
import re
from pathlib import Path
from typing import Any, Callable

from harness.common.digests import digest
from harness.common.files import read_file
from harness.common.directory import validate_repo_path
from harness.common.paths import validate_paths
from harness.common.observation import observe_file
from harness.common.quarantine import validate_quarantine
from harness.common.images import validate_image_path
from harness.common.recovery import (
    DELETION_SCHEMA,
    IDENTITY_SCHEMA,
    LEGACY_SCHEMA,
    RECOVERY_OWNERS,
    SCHEMA,
)
from harness.common.safeguards import _validate_safeguards, inspect_safeguards
from harness.io import unique_object


def _require_fields(value: object, fields: str) -> dict[str, Any]:
    if not isinstance(value, dict) or set(value) != set(fields.split()):
        raise ValueError("invalid recovery record fields")
    return value


def _is_hash(value: object) -> bool:
    return isinstance(value, str) and re.fullmatch(r"[0-9a-f]{64}", value) is not None


def _validate_file(
    name: str,
    value: object,
    manifest: dict[str, Any],
    *,
    legacy: bool,
    deletions: bool = False,
) -> None:
    entry = _require_fields(value, "pre post quarantine")
    pre = _require_fields(entry["pre"], "content_base64 sha256 mode device inode")
    deleted = entry["post"] is None
    if deleted and (not deletions or pre["content_base64"] is None):
        raise ValueError("invalid recovery POST deletion")
    post = (
        _require_fields(
            entry["post"],
            "sha256 mode creation_mode"
            if legacy
            else "sha256 mode device inode staging quarantine",
        )
        if not deleted
        else None
    )
    if not deleted and not _is_hash(post["sha256"]):
        raise ValueError("invalid recovery POST hash")
    if not legacy and not deleted:
        if (
            type(post["mode"]) is not int
            or not 0 <= post["mode"] <= 0o7777
            or any(
                type(post[key]) is not int or post[key] < 0
                for key in ("device", "inode")
            )
        ):
            raise ValueError("invalid recovery POST identity")
        validate_image_path(name, post["staging"])
        validate_quarantine(name, post["quarantine"])
        if post["quarantine"] == entry["quarantine"]:
            raise ValueError("recovery PRE and POST quarantines must differ")
    if pre["sha256"] != manifest["pre_state"][name]:
        raise ValueError("recovery PRE does not match its manifest")
    if pre["content_base64"] is None:
        if (
            any(value is not None for value in pre.values())
            or entry["quarantine"] is not None
        ):
            raise ValueError("invalid absent recovery PRE")
        if legacy and (
            post["mode"] is not None
            or post["creation_mode"] != ("0644 masked by the process umask")
        ):
            raise ValueError("invalid recovery creation mode policy")
        return
    encoded = pre["content_base64"]
    if not isinstance(encoded, str):
        raise ValueError("invalid recovery PRE encoding")
    try:
        content = base64.b64decode(encoded, validate=True)
    except ValueError as error:
        raise ValueError("invalid recovery PRE encoding") from error
    if (
        base64.b64encode(content).decode("ascii") != encoded
        or hashlib.sha256(content).hexdigest() != pre["sha256"]
        or type(pre["mode"]) is not int
        or not 0 <= pre["mode"] <= 0o7777
        or any(type(pre[key]) is not int or pre[key] < 0 for key in ("device", "inode"))
        or (
            not deleted
            and (type(post["mode"]) is not int or post["mode"] != pre["mode"])
        )
        or (legacy and post["creation_mode"] is not None)
    ):
        raise ValueError("invalid recovery PRE content or identity")
    validate_quarantine(name, entry["quarantine"])


def _describe_file(
    root: Path, name: str, entry: dict[str, Any], *, legacy: bool
) -> dict[str, Any]:
    current = observe_file(root, name)
    pre = {key: value for key, value in entry["pre"].items() if key != "content_base64"}
    quarantine = entry["quarantine"]
    displaced = observe_file(root, quarantine) if quarantine is not None else None
    expected = {**pre, "links": 1} if pre["sha256"] is not None else None
    if entry["post"] is None:
        return {
            "state": "pre"
            if current == expected and displaced is None
            else "deleted"
            if current is None and displaced == expected
            else "missing"
            if current is None and displaced is None
            else "drifted",
            "observed": current,
            "quarantine": quarantine,
            "quarantine_state": "absent"
            if displaced is None
            else "original-pre"
            if displaced == expected
            else "drifted",
            "post_kind": "absent",
            "post_identity_bound": False,
        }
    expected_post = {
        **{
            key: value
            for key, value in entry["post"].items()
            if key not in {"staging", "quarantine"}
        },
        "links": 1,
    }
    if current == expected:
        state = "pre"
    elif current is None:
        state = "missing"
    elif not legacy and current == expected_post:
        state = "post"
    elif legacy and (
        current["sha256"] == entry["post"]["sha256"]
        and current["links"] == 1
        and (entry["post"]["mode"] is None or current["mode"] == entry["post"]["mode"])
    ):
        state = "post-content-only"
    else:
        state = "drifted"
    result = {
        "state": state,
        "observed": current,
        "quarantine": quarantine,
        "quarantine_state": "absent"
        if displaced is None
        else "original-pre"
        if displaced == expected
        else "drifted",
        "post_identity_bound": not legacy,
    }
    if not legacy:
        staging = entry["post"]["staging"]
        staged = observe_file(root, staging)
        result["staging"] = staging
        result["staging_state"] = (
            "absent"
            if staged is None
            else "prepared-post"
            if staged == expected_post
            else "drifted"
        )
        post_quarantine = entry["post"]["quarantine"]
        displaced_post = observe_file(root, post_quarantine)
        result["post_quarantine"] = post_quarantine
        result["post_quarantine_state"] = (
            "absent"
            if displaced_post is None
            else "original-post"
            if displaced_post == expected_post
            else "drifted"
        )
    return result


def load_recovery(
    root: Path,
    name: str,
    expected_recovery_digest: str,
    *,
    owner: str,
    manifest_validator: Callable[..., dict[str, Any]],
) -> dict[str, Any]:
    """Load validated, externally pinned backing without restoration authority."""

    name = validate_repo_path(name)
    if (
        owner not in RECOVERY_OWNERS
        or re.fullmatch(
            rf"out/reviews/evidence/{owner}-recovery-[0-9a-f]{{32}}\.json", name
        )
        is None
    ):
        raise ValueError("invalid recovery record path or owner")
    metadata = observe_file(root, name)
    if metadata is None or metadata["links"] != 1:
        raise ValueError("recovery record must be a single-link regular file")
    content = read_file(root, name)
    if hashlib.sha256(content).hexdigest() != metadata["sha256"]:
        raise ValueError("recovery record changed during inspection")
    record = json.loads(content, object_pairs_hook=unique_object)
    fields = "schema nonce root writer_pid binding files recovery_scope restoration_authority digest"
    if isinstance(record, dict) and record.get("schema") in {SCHEMA, DELETION_SCHEMA}:
        fields += " safeguards"
    record = _require_fields(record, fields)
    facts = {key: value for key, value in record.items() if key != "digest"}
    if (
        record["schema"]
        not in (LEGACY_SCHEMA, IDENTITY_SCHEMA, SCHEMA, DELETION_SCHEMA)
        or record["digest"] != expected_recovery_digest
        or record["digest"] != digest(facts)
        or record["restoration_authority"] is not False
        or record["recovery_scope"]
        != "owned source files only; not Git index or unrelated files"
        or type(record["writer_pid"]) is not int
        or record["writer_pid"] <= 0
        or name != f"out/reviews/evidence/{owner}-recovery-{record['nonce']}.json"
    ):
        raise ValueError("recovery record or independent digest drifted")
    identity = root.stat()
    recorded_root = _require_fields(record["root"], "path device inode")
    if any(
        type(recorded_root[key]) is not int for key in ("device", "inode")
    ) or recorded_root != {
        "path": str(root.resolve()),
        "device": identity.st_dev,
        "inode": identity.st_ino,
    }:
        raise ValueError("recovery root identity drifted")
    binding = _require_fields(
        record["binding"], "owner manifest implementation_run_id output"
    )
    if binding["owner"] != owner:
        raise ValueError("recovery record belongs to another owner")
    run_id = binding["implementation_run_id"]
    if run_id is not None and (not isinstance(run_id, str) or not run_id.strip()):
        raise ValueError("invalid recovery implementation run ID")
    manifest = manifest_validator(root, binding["manifest"], rederive=False)
    allowed = validate_paths(root, manifest["allowed_paths"])
    pre_state = manifest.get("pre_state")
    if (
        not isinstance(pre_state, dict)
        or set(pre_state) != allowed
        or any(
            value is not None and not _is_hash(value) for value in pre_state.values()
        )
        or manifest.get("pre_state_digest") != digest(pre_state)
        or not isinstance(record["files"], dict)
        or not record["files"]
        or not set(record["files"]) <= allowed
    ):
        raise ValueError("invalid recovery owned PRE or changed paths")
    legacy = record["schema"] == LEGACY_SCHEMA
    for path, entry in record["files"].items():
        _validate_file(
            path,
            entry,
            manifest,
            legacy=legacy,
            deletions=record["schema"] == DELETION_SCHEMA,
        )
    if record["schema"] == DELETION_SCHEMA and not any(
        entry["post"] is None for entry in record["files"].values()
    ):
        raise ValueError("deletion recovery requires an absent POST")
    output = binding["output"]
    if output is not None:
        output = validate_repo_path(output)
        if not output.startswith("out/reviews/evidence/"):
            raise ValueError("invalid recovery publication path")
    if record.get("safeguards") is not None:
        _validate_safeguards(record["safeguards"], set(record["files"]))
    if (
        read_file(root, name) != content
        or observe_file(root, name) != metadata
        or digest({key: value for key, value in record.items() if key != "digest"})
        != expected_recovery_digest
        or record["digest"] != expected_recovery_digest
    ):
        raise ValueError("recovery record changed during inspection")
    return record


def inspect_recovery(
    root: Path,
    name: str,
    expected_recovery_digest: str,
    *,
    owner: str,
    manifest_validator: Callable[..., dict[str, Any]],
) -> dict[str, Any]:
    name = validate_repo_path(name)
    metadata = observe_file(root, name)
    record = load_recovery(
        root,
        name,
        expected_recovery_digest,
        owner=owner,
        manifest_validator=manifest_validator,
    )
    binding = record["binding"]
    manifest = binding["manifest"]
    allowed = set(manifest["allowed_paths"])
    pre_state = manifest["pre_state"]
    files = {
        path: _describe_file(
            root, path, entry, legacy=record["schema"] == LEGACY_SCHEMA
        )
        for path, entry in record["files"].items()
    }
    unchanged = {}
    for path in sorted(allowed - record["files"].keys()):
        observed = observe_file(root, path)
        actual = observed["sha256"] if observed is not None else None
        unchanged[path] = "pre-content-only" if actual == pre_state[path] else "drifted"
    output = binding["output"]
    publication = "unknown"
    if output is not None:
        publication = (
            "present-unverified" if observe_file(root, output) is not None else "absent"
        )
    if observe_file(root, name) != metadata:
        raise ValueError("recovery record changed during inspection")
    return {
        "schema": "bof3.transaction-recovery-inspection/v1",
        "recovery_digest": expected_recovery_digest,
        "recovery_schema": record["schema"],
        "owner": owner,
        "manifest_digest": manifest["digest"],
        "manifest_rederived": False,
        "implementation_run_id": binding["implementation_run_id"],
        "files": files,
        "unchanged_paths": unchanged,
        "publication": {"path": output, "state": publication},
        "restoration_authority": False,
        "writer_termination_verified": False,
        "writer_exclusion_verified": False,
        "workspace_and_git_verified": False,
        "safeguards": inspect_safeguards(
            root, record.get("safeguards"), set(record["files"])
        ),
        "observation_atomic": False,
    }
