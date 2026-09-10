"""Manifest-bound participation limits and shared target cardinality."""

from __future__ import annotations


def resolve_limit(manifest: dict) -> int:
    limit = manifest.get("participation_limit", 2)
    if type(limit) is not int or limit < 1:
        raise ValueError("invalid manifest participation limit")
    return limit


def validate_targets(manifest: dict) -> int:
    targets = manifest["targets"]
    if (
        not isinstance(targets, list)
        or any(not isinstance(target, str) for target in targets)
        or targets != sorted(set(targets))
        or not 1 <= len(targets) <= resolve_limit(manifest)
        or manifest["target"] not in targets
    ):
        raise ValueError("invalid manifest targets")
    return len(targets)
