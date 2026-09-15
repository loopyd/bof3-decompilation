"""Prepare exact joint consolidation images without applying or accepting them."""

from __future__ import annotations

import copy
import hashlib
from pathlib import Path

from harness.build.preservation import (
    hash_preservation,
    validate_fingerprint,
    validate_ownership_images,
    validate_preservation_size,
    validate_states,
)
from harness.build.profiles import collect_profile_sources
from harness.combiner.preservation import capture_preservation
from harness.common.deadlines import check_deadline
from harness.common.inputs import InputBatch, relative
from harness.domain.cache import collect_manifest_paths

SCHEMA = "bof3.combiner-transaction/v1"
OWNERS = (
    "tools/python/harness/combiner/transactions.py",
    "tools/python/harness/combiner/cli.py",
)


def _prepare_images(images: object) -> tuple[dict, dict]:
    if not isinstance(images, dict) or not images:
        raise ValueError("transaction requires exact joint file images")
    validate_preservation_size(images)
    changes, post = {}, {}
    for name, image in images.items():
        check_deadline()
        if not isinstance(name, str) or relative(name) != name:
            raise ValueError("transaction images require canonical repository paths")
        if image is None:
            changes[name] = post[name] = None
            continue
        if (
            not isinstance(image, dict)
            or set(image) != {"text", "mode"}
            or not isinstance(image["text"], str)
        ):
            raise ValueError("transaction images require exactly text and mode")
        changes[name] = image["text"]
        post[name] = {
            "sha256": hashlib.sha256(image["text"].encode("utf-8")).hexdigest(),
            "mode": image["mode"],
        }
    return changes, validate_states(post)


def _verify_pre(root: Path, record: dict, owners: dict) -> None:
    with InputBatch(root) as batch:
        for name, expected in {**record["inputs"], **owners}.items():
            if batch.read(root / name)[0] != expected:
                raise ValueError(f"transaction PRE input changed: {name}")
    if (
        collect_profile_sources(root, record["profile"]["destination"])
        != record["source_inventory"]
        or collect_manifest_paths(root) != record["manifest_inventory"]
    ):
        raise ValueError("transaction PRE ownership inventory changed")
    check_deadline()


def prepare_transaction(
    root: Path,
    profile: dict,
    images: dict,
    *,
    expected_profile_fingerprint: str,
) -> dict:
    """Pin complete proposed text, observed PRE and explicit absent POST paths."""
    check_deadline()
    root = root.resolve(strict=True)
    validate_fingerprint(expected_profile_fingerprint)
    profile, images = copy.deepcopy(profile), copy.deepcopy(images)
    changes, post = _prepare_images(images)
    if not isinstance(profile, dict) or not isinstance(
        profile.get("destination_text"), str
    ):
        raise ValueError(
            "transaction requires a profile with the exact destination draft"
        )
    owners = {}
    with InputBatch(root) as batch:
        for name in OWNERS:
            state, _ = batch.read(root / name)
            if state is None:
                raise ValueError(f"transaction owner is absent: {name}")
            owners[name] = state
    record = capture_preservation(
        root,
        profile,
        post,
        expected_profile_fingerprint=expected_profile_fingerprint,
    )
    if any(
        name in record["inputs"] and record["inputs"][name] != state
        for name, state in owners.items()
    ):
        raise ValueError("transaction owner changed during preservation capture")
    pre = {name: record["inputs"][name] for name in post}
    for name, after in post.items():
        before = pre[name]
        if after is None and before is None:
            raise ValueError(f"transaction deletion has no PRE file: {name}")
        if before is not None and after is not None and before["mode"] != after["mode"]:
            raise ValueError(f"transaction must preserve observed file mode: {name}")
    validate_ownership_images(
        record, changes[record["manifest"]], changes[record["layout"]]
    )
    result = {
        "schema": SCHEMA,
        "root": str(root),
        "profile_fingerprint": expected_profile_fingerprint,
        "images": images,
        "changes": changes,
        "allowed_paths": sorted(post),
        "deletions": sorted(name for name, state in post.items() if state is None),
        "pre": pre,
        "post": post,
        "preservation": record,
        "owner_inputs": owners,
        "native_verified": False,
        "coverage_verified": False,
        "write_authorized": False,
        "accepted": False,
    }
    result["fingerprint"] = hash_preservation(result)
    validate_preservation_size(result)
    _verify_pre(root, record, owners)
    return result


def verify_transaction(
    root: Path, transaction: dict, *, expected_fingerprint: str
) -> dict:
    """Re-derive a prepared transaction against live PRE, never approve its edits."""
    check_deadline()
    validate_fingerprint(expected_fingerprint)
    if not isinstance(transaction, dict):
        raise ValueError("transaction must be an object")
    validate_preservation_size(transaction)
    transaction = copy.deepcopy(transaction)
    if (
        transaction.get("schema") != SCHEMA
        or transaction.get("fingerprint") != expected_fingerprint
        or hash_preservation(
            {
                name: value
                for name, value in transaction.items()
                if name != "fingerprint"
            }
        )
        != expected_fingerprint
    ):
        raise ValueError("transaction fingerprint differs from its external pin")
    try:
        current = prepare_transaction(
            root,
            transaction["preservation"]["profile"],
            transaction["images"],
            expected_profile_fingerprint=transaction["profile_fingerprint"],
        )
    except (KeyError, TypeError) as error:
        raise ValueError(
            "transaction is missing its prepared input contract"
        ) from error
    if current != transaction:
        raise ValueError("transaction differs from current PRE preparation")
    return current
