"""Persist pre-mutation recovery material without granting restoration authority."""

from __future__ import annotations

import base64
import hashlib
import json
import os
import secrets
import stat
from pathlib import Path
from typing import Any

from harness.common.digests import digest
from harness.common.files import atomic_write, read_file
from harness.common.images import prepare_image
from harness.common.git import GitIndexSnapshot
from harness.common.paths import file_state, leaf_stat
from harness.common.quarantine import reserve_quarantine
from harness.common.safeguards import capture_safeguards
from harness.common.submodules import validate_mutations
from harness.common.workspace import WorkspaceSnapshot

LEGACY_SCHEMA = "bof3.transaction-recovery/v1"
IDENTITY_SCHEMA = "bof3.transaction-recovery/v2"
SCHEMA = "bof3.transaction-recovery/v3"
DELETION_SCHEMA = "bof3.transaction-recovery/v4"
RECOVERY_OWNERS = frozenset({"macro", "type", "combiner"})


def capture_recovery(
    root: Path,
    binding: dict[str, Any],
    changes: dict[str, str | None],
    backup: dict[str, bytes | None],
    *,
    workspace: WorkspaceSnapshot | None = None,
    index: GitIndexSnapshot | None = None,
) -> dict[str, dict[str, Any]]:
    if set(binding) != {"owner", "manifest", "implementation_run_id", "output"}:
        raise ValueError("invalid recovery binding")
    if binding["owner"] not in RECOVERY_OWNERS:
        raise ValueError("invalid recovery owner")
    manifest = binding["manifest"]
    if manifest.get("digest") != digest(
        {name: value for name, value in manifest.items() if name != "digest"}
    ):
        raise ValueError("invalid recovery manifest digest")
    if set(changes) != set(backup) or not set(changes) <= set(
        manifest["allowed_paths"]
    ):
        raise ValueError("recovery paths differ from the authorized changes")
    if any(
        (content is not None and not isinstance(content, str))
        or (content is None and backup[name] is None)
        for name, content in changes.items()
    ):
        raise ValueError("recovery requires text or deletion of an existing PRE")
    validate_mutations(root, {*manifest["allowed_paths"], "out/reviews/evidence"})
    if file_state(root, manifest["allowed_paths"]) != manifest["pre_state"]:
        raise ValueError("recovery capture requires the complete owned PRE")
    safeguards = capture_safeguards(root, set(changes), workspace, index)
    files = {}
    for name, content in changes.items():
        validate_mutations(root, {*changes, "out/reviews/evidence"})
        before = backup[name]
        metadata = leaf_stat(root, name)
        if (metadata is None) != (before is None):
            raise ValueError(f"recovery PRE changed: {name}")
        if metadata is not None and metadata.st_nlink != 1:
            raise ValueError(f"recovery PRE must have one link: {name}")
        if read_file(root, name, missing_ok=True) != before:
            raise ValueError(f"recovery PRE changed: {name}")
        destination = reserve_quarantine(name) if before is not None else None
        files[name] = {
            "pre": {
                "content_base64": base64.b64encode(before).decode("ascii")
                if before is not None
                else None,
                "sha256": hashlib.sha256(before).hexdigest()
                if before is not None
                else None,
                "mode": stat.S_IMODE(metadata.st_mode) if metadata else None,
                "device": metadata.st_dev if metadata else None,
                "inode": metadata.st_ino if metadata else None,
            },
            "post": prepare_image(
                root,
                name,
                content.encode("utf-8"),
                stat.S_IMODE(metadata.st_mode) if metadata else None,
            )
            if content is not None
            else None,
            "quarantine": destination,
        }
    identity = root.stat()
    nonce = secrets.token_hex(16)
    record = {
        "schema": DELETION_SCHEMA
        if any(content is None for content in changes.values())
        else SCHEMA,
        "nonce": nonce,
        "root": {
            "path": str(root.resolve()),
            "device": identity.st_dev,
            "inode": identity.st_ino,
        },
        "writer_pid": os.getpid(),
        "binding": binding,
        "files": files,
        "safeguards": safeguards,
        "recovery_scope": "owned source files only; not Git index or unrelated files",
        "restoration_authority": False,
    }
    record["digest"] = digest(record)
    path = f"out/reviews/evidence/{binding['owner']}-recovery-{nonce}.json"
    encoded = (json.dumps(record, indent=2, sort_keys=True) + "\n").encode("utf-8")
    validate_mutations(root, {path})
    atomic_write(root, path, encoded, expected=None, creation_mode=0o600)
    return files
