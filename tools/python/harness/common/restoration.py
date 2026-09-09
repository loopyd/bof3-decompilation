"""Parent-authorized, fail-stop restoration of identity-bound owned source PRE."""

from __future__ import annotations

import base64
import hashlib
import json
from pathlib import Path
from typing import Any, Callable

from harness.common.authorization import load_recovery_authorization
from harness.common.digests import digest
from harness.common.files import atomic_write, read_file
from harness.common.images import classify_restoration
from harness.common.inspection import load_recovery
from harness.common.lease import acquire_writer, verify_writer
from harness.common.observation import observe_file
from harness.common.paths import file_state
from harness.common.recovery import SCHEMA
from harness.common.runtime import rollback
from harness.common.safeguards import inspect_safeguards

RESTORATION_SCHEMA = "bof3.source-restoration/v1"


def _check_restoration_guards(root: Path, record: dict[str, Any]) -> None:
    verify_writer(root)
    output = record["binding"]["output"]
    if output is None or observe_file(root, output) is not None:
        raise ValueError("source recovery requires known absent publication")
    guards = inspect_safeguards(root, record.get("safeguards"), set(record["files"]))
    if not guards["available"] or not guards["matches"]:
        raise ValueError(
            "source recovery workspace or Git guards are unavailable or drifted"
        )
    manifest = record["binding"]["manifest"]
    unchanged = set(manifest["allowed_paths"]) - record["files"].keys()
    if file_state(root, unchanged) != {
        name: manifest["pre_state"][name] for name in unchanged
    }:
        raise ValueError("source recovery unchanged owned paths drifted")


def _load_restoration_images(
    root: Path, record: dict[str, Any]
) -> tuple[dict[str, bytes | None], dict[str, dict[str, Any]]]:
    backup, images = {}, {}
    for name, entry in record["files"].items():
        encoded = entry["pre"]["content_base64"]
        backup[name] = base64.b64decode(encoded) if encoded is not None else None
        post = entry["post"]
        expected = {key: post[key] for key in ("sha256", "mode", "device", "inode")}
        expected["links"] = 1
        locations = [
            path
            for path in (name, post["staging"], post["quarantine"])
            if observe_file(root, path) == expected
        ]
        if len(locations) != 1:
            raise ValueError(
                f"source recovery requires one original POST image: {name}"
            )
        installed = read_file(root, locations[0])
        if hashlib.sha256(installed).hexdigest() != post["sha256"]:
            raise ValueError(f"source recovery POST changed while reading: {name}")
        images[name] = {**entry, "installed": installed}
        classify_restoration(root, name, backup[name], images[name])
    return backup, images


def _verify_restoration_states(
    root: Path,
    backup: dict[str, bytes | None],
    images: dict[str, dict[str, Any]],
    completed: set[str],
) -> None:
    for path, image in images.items():
        state = classify_restoration(root, path, backup[path], image)
        if path in completed and state != "pre":
            raise ValueError(f"source recovery restored PRE drifted: {path}")


def restore_sources(
    root: Path,
    name: str,
    expected_recovery_digest: str,
    authorization_name: str,
    expected_authorization_digest: str,
    *,
    owner: str,
    manifest_validator: Callable[..., dict[str, Any]],
) -> dict[str, Any]:
    with acquire_writer(root):
        record = load_recovery(
            root,
            name,
            expected_recovery_digest,
            owner=owner,
            manifest_validator=manifest_validator,
        )
        if record["schema"] != SCHEMA:
            raise ValueError("source recovery requires v3 identity and guard backing")
        authorization = load_recovery_authorization(
            root, authorization_name, expected_authorization_digest, name, record
        )
        _check_restoration_guards(root, record)
        backup, images = _load_restoration_images(root, record)
        output = authorization["binding"]["completion"]
        facts = {
            "schema": RESTORATION_SCHEMA,
            "owner": owner,
            "recovery_digest": expected_recovery_digest,
            "authorization_digest": expected_authorization_digest,
            "binding": authorization["binding"],
            "termination": authorization["termination"],
            "termination_basis": "parent-attested; not independently authenticated by CLI",
            "restored": {path: entry["pre"] for path, entry in record["files"].items()},
            "source_accepted": False,
            "retry_authorized": False,
            "power_loss_durability_verified": False,
        }
        facts["restored"] = {
            path: {
                key: value for key, value in entry.items() if key != "content_base64"
            }
            for path, entry in facts["restored"].items()
        }
        result = {**facts, "digest": digest(facts)}
        encoded = (json.dumps(result, indent=2, sort_keys=True) + "\n").encode()
        existing = read_file(root, output, missing_ok=True)
        metadata = observe_file(root, output)
        if metadata is not None and (
            metadata["mode"] != 0o600 or metadata["links"] != 1
        ):
            raise ValueError(
                "source restoration receipt must be private and single-link"
            )
        if existing is not None and existing != encoded:
            raise ValueError("source restoration completion receipt drifted")
        if existing is not None and any(
            classify_restoration(root, path, backup[path], image) != "pre"
            for path, image in images.items()
        ):
            raise ValueError("completed source restoration no longer matches PRE")
        completed = set()
        for path in sorted(images):
            load_recovery_authorization(
                root, authorization_name, expected_authorization_digest, name, record
            )
            load_recovery(
                root,
                name,
                expected_recovery_digest,
                owner=owner,
                manifest_validator=manifest_validator,
            )
            _check_restoration_guards(root, record)
            _verify_restoration_states(root, backup, images, completed)
            rollback(root, {path: backup[path]}, {path: images[path]})
            completed.add(path)
        load_recovery_authorization(
            root, authorization_name, expected_authorization_digest, name, record
        )
        load_recovery(
            root,
            name,
            expected_recovery_digest,
            owner=owner,
            manifest_validator=manifest_validator,
        )
        _check_restoration_guards(root, record)
        _verify_restoration_states(root, backup, images, set(images))
        verify_writer(root)
        if existing is None:
            atomic_write(root, output, encoded, expected=None, mode=0o600)
        if read_file(root, output) != encoded:
            raise ValueError("source restoration receipt changed during publication")
        published = observe_file(root, output)
        if (
            published is None
            or published["mode"] != 0o600
            or published["links"] != 1
            or published["sha256"] != hashlib.sha256(encoded).hexdigest()
        ):
            raise ValueError("source restoration receipt identity drifted")
        load_recovery_authorization(
            root, authorization_name, expected_authorization_digest, name, record
        )
        load_recovery(
            root,
            name,
            expected_recovery_digest,
            owner=owner,
            manifest_validator=manifest_validator,
        )
        _check_restoration_guards(root, record)
        _verify_restoration_states(root, backup, images, set(images))
        return result
