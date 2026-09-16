"""Prepare exact joint consolidation images without applying or accepting them."""

from __future__ import annotations

import copy
from pathlib import Path

from harness.build.preservation import (
    hash_preservation,
    validate_fingerprint,
    validate_ownership_images,
    validate_preservation_size,
)
from harness.build.profiles import collect_profile_sources
from harness.combiner.history import (
    collect_transaction_owners,
    validate_transaction_history,
)
from harness.combiner.images import prepare_images
from harness.combiner.preservation import capture_preservation
from harness.common.deadlines import check_deadline
from harness.common.inputs import InputBatch
from harness.domain.cache import collect_manifest_paths

SCHEMA = "bof3.combiner-transaction/v3"
OWNERS = collect_transaction_owners(SCHEMA)


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
    changes, post = prepare_images(images)
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
    transaction = validate_transaction_history(
        transaction, expected_fingerprint=expected_fingerprint
    )
    if transaction["schema"] != SCHEMA:
        raise ValueError("historical transaction is not current PRE admission")
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
