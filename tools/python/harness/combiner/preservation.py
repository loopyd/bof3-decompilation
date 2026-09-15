"""Capture consolidation policy and frozen PRE-to-POST compiler history."""

from __future__ import annotations

import copy
import hashlib
import json
import tomllib
from pathlib import Path

from harness.build.profiles import ProfileContext, collect_profile_sources
from harness.build.preservation import (
    PRESERVATION_OWNERS,
    PRESERVATION_SCHEMA,
    decode_layout,
    hash_preservation,
    validate_fingerprint,
    validate_preservation_size,
    validate_states,
    validate_transition,
)
from harness.combiner.profiles import inspect_profiles
from harness.common.deadlines import check_deadline
from harness.domain.cache import collect_manifest_paths


def _relocate_layout(layout: object, sources: list[str], destination: str) -> object:
    result = copy.deepcopy(layout)
    if not isinstance(result, dict) or not isinstance(result.get("segments"), list):
        raise ValueError("preservation requires explicit Splat segments")
    counts = dict.fromkeys(sources, 0)
    for segment in result["segments"]:
        if not isinstance(segment, dict):
            continue
        for row in segment.get("subsegments", []):
            check_deadline()
            if not isinstance(row, list) or len(row) < 3 or row[1] != "c":
                continue
            markers = [
                (index, value.removeprefix("@source:").strip())
                for index, value in enumerate(row[3:], 3)
                if isinstance(value, str) and value.startswith("@source:")
            ]
            if any(source in counts for index, source in markers):
                if len(markers) != 1:
                    raise ValueError(
                        "preservation requires one source marker per C boundary"
                    )
                index, source = markers[0]
                row[index] = "@source: " + destination
                counts[source] += 1
    if any(count == 0 for count in counts.values()):
        raise ValueError(
            "preservation requires explicit Splat source markers for every member"
        )
    return result


def capture_preservation(
    root: Path,
    profile: dict,
    post: dict,
    *,
    expected_profile_fingerprint: str,
) -> dict:
    """Freeze reviewed PRE and proposed POST images without performing any edits."""
    root = root.resolve()
    profile = copy.deepcopy(profile)
    post = validate_states(copy.deepcopy(post))
    validate_fingerprint(expected_profile_fingerprint)
    manifest, layout, sources = validate_transition(profile, post)
    current = inspect_profiles(
        root,
        profile["target"],
        profile["destination"],
        sources,
        expected_fingerprint=expected_profile_fingerprint,
        destination_text=profile["destination_text"],
    )
    if profile != current:
        raise ValueError("supplied PRE profile differs from current configured inputs")
    context = ProfileContext(root)
    for path, state in profile["inputs"].items():
        context.read(path, required=state is not None)
        if context.inputs[path] != state:
            raise ValueError(f"PRE input drifted: {path}")
    for path in PRESERVATION_OWNERS:
        context.read(path)
    manifest_text = context.read(manifest).decode("utf-8")
    layout_text = context.read(layout).decode("utf-8")
    expected_layout = _relocate_layout(
        decode_layout(layout_text), sources, profile["destination"]
    )
    inventory = collect_profile_sources(root, profile["destination"])
    if profile["source_inventory"] != {
        "count": len(inventory),
        "sha256": hashlib.sha256(
            json.dumps(inventory, separators=(",", ":")).encode()
        ).hexdigest(),
    }:
        raise ValueError("source inventory drifted after PRE inspection")
    paths = collect_manifest_paths(root)
    source_pre = {source: context.read(source).decode("utf-8") for source in sources}
    if paths != sorted(
        path
        for path in profile["inputs"]
        if path.startswith("config/targets/") and path.endswith(".toml")
    ):
        raise ValueError("manifest inventory drifted after PRE inspection")
    context.verify()
    if inventory != collect_profile_sources(
        root, profile["destination"]
    ) or paths != collect_manifest_paths(root):
        raise ValueError("PRE inventory changed during preservation capture")
    result = {
        "schema": PRESERVATION_SCHEMA,
        "root": str(root),
        "profile": profile,
        "inputs": context.inputs,
        "post": post,
        "manifest": manifest,
        "layout": layout,
        "manifest_pre": tomllib.loads(manifest_text),
        "source_pre": source_pre,
        "layout_post": expected_layout,
        "source_inventory": inventory,
        "manifest_inventory": paths,
        "native_verified": False,
        "producer_freshness_verified": False,
        "write_authorized": False,
    }
    fingerprint = hash_preservation(result)
    result = {**result, "fingerprint": fingerprint}
    validate_preservation_size(result)
    check_deadline()
    return result
