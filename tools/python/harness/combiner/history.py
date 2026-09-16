"""Validate versioned retained consolidation history without live admission."""

from __future__ import annotations

import copy

from harness.build.preservation import (
    hash_preservation,
    validate_fingerprint,
    validate_ownership_images,
    validate_preservation_size,
    validate_profile_history,
    validate_states,
)
from harness.combiner.images import prepare_images
from harness.common.deadlines import check_deadline

_OWNERS_V1 = (
    "tools/python/harness/combiner/transactions.py",
    "tools/python/harness/combiner/cli.py",
    "tools/python/harness/combiner/rehearsal.py",
    "tools/python/harness/combiner/recovery.py",
    "tools/python/harness/common/runtime.py",
    "tools/python/harness/common/recovery.py",
    "tools/python/harness/common/inspection.py",
    "tools/python/harness/common/authorization.py",
    "tools/python/harness/common/restoration.py",
    "tools/python/harness/common/safeguards.py",
    "tools/python/harness/common/workspace.py",
    "tools/python/harness/common/verification.py",
    "tools/python/harness/domain/policy.py",
)
_OWNERS_V2 = (
    *_OWNERS_V1,
    "tools/python/harness/combiner/history.py",
    "tools/python/harness/combiner/images.py",
)


def collect_transaction_owners(schema: str) -> tuple[str, ...]:
    """Return the exact owner set attached to one supported historical version."""
    if schema == "bof3.combiner-transaction/v1":
        return _OWNERS_V1
    if schema == "bof3.combiner-transaction/v2":
        return _OWNERS_V2
    raise ValueError("unsupported retained transaction schema")


def validate_transaction_history(
    transaction: dict, *, expected_fingerprint: str
) -> dict:
    """Validate retained images and bindings without asserting current PRE or POST."""
    check_deadline()
    validate_fingerprint(expected_fingerprint)
    if not isinstance(transaction, dict):
        raise ValueError("transaction must be an object")
    required_owners = collect_transaction_owners(transaction.get("schema"))
    validate_preservation_size(transaction)
    transaction = copy.deepcopy(transaction)
    if (
        transaction.get("fingerprint") != expected_fingerprint
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
    if set(transaction) != {
        "schema",
        "root",
        "profile_fingerprint",
        "images",
        "changes",
        "allowed_paths",
        "deletions",
        "pre",
        "post",
        "preservation",
        "owner_inputs",
        "native_verified",
        "coverage_verified",
        "write_authorized",
        "accepted",
        "fingerprint",
    }:
        raise ValueError("transaction fields differ from preparation")
    changes, post = prepare_images(transaction["images"])
    pre = validate_states(transaction["pre"])
    owners = validate_states(transaction["owner_inputs"])
    record = transaction["preservation"]
    if (
        changes != transaction["changes"]
        or post != transaction["post"]
        or set(pre) != set(post)
        or transaction["allowed_paths"] != sorted(post)
        or transaction["deletions"]
        != sorted(name for name, state in post.items() if state is None)
        or set(owners) != set(required_owners)
        or any(state is None for state in owners.values())
        or not isinstance(record, dict)
        or record.get("schema") != "bof3.combiner-preservation/v5"
        or record.get("root") != transaction["root"]
        or record.get("fingerprint")
        != hash_preservation(
            {name: value for name, value in record.items() if name != "fingerprint"}
        )
        or any(
            transaction[name] is not False
            for name in (
                "native_verified",
                "coverage_verified",
                "write_authorized",
                "accepted",
            )
        )
    ):
        raise ValueError("transaction images or retained bindings differ")
    try:
        manifest, layout, _ = validate_profile_history(record["profile"], post)
        if (
            record["profile"]["fingerprint"] != transaction["profile_fingerprint"]
            or hash_preservation(
                {
                    name: value
                    for name, value in record["profile"].items()
                    if name != "fingerprint"
                }
            )
            != transaction["profile_fingerprint"]
            or record["manifest"] != manifest
            or record["layout"] != layout
            or record["post"] != post
            or {name: record["inputs"][name] for name in post} != pre
            or any(
                (after is None and pre[name] is None)
                or (
                    after is not None
                    and pre[name] is not None
                    and after["mode"] != pre[name]["mode"]
                )
                for name, after in post.items()
            )
        ):
            raise ValueError("transaction preservation differs from joint states")
        validate_ownership_images(
            record, changes[record["manifest"]], changes[record["layout"]]
        )
    except (KeyError, TypeError) as error:
        raise ValueError("transaction lacks retained preparation bindings") from error
    return transaction
