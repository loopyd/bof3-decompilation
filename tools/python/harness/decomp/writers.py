"""Validate externally pinned completion of the diagnosis-owned native writer."""

from __future__ import annotations

from pathlib import Path
import re

from harness.common.digests import digest
from harness.common.paths import leaf_stat
from harness.context.capabilities import MODE
from harness.context.journal import hash_artifacts, read_record


def validate_writer_slot(
    slot: dict, directory: str, diagnosis: dict, mission: dict
) -> dict:
    fields = {
        "schema",
        "directory",
        "digest",
        "budget_digest",
        "checkpoint_digest",
        "sequence",
        "identity",
        "request_digest",
        "prompt_sha256",
        "diagnosis",
        "clock",
        "capabilities",
    }
    if (
        set(slot) != fields
        or slot.get("schema") != "bof3.lift-writer-slot/v1"
        or slot.get("digest")
        != digest({key: value for key, value in slot.items() if key != "digest"})
        or any(
            not isinstance(slot.get(key), str)
            or re.fullmatch(r"v1:[0-9a-f]{64}", slot[key]) is None
            for key in ("budget_digest", "checkpoint_digest", "request_digest")
        )
        or not isinstance(slot.get("prompt_sha256"), str)
        or re.fullmatch(r"[0-9a-f]{64}", slot["prompt_sha256"]) is None
        or type(slot.get("sequence")) is not int
        or slot["sequence"] < 1
    ):
        raise ValueError("lift writer slot is invalid")
    binding = {
        key: value
        for key, value in slot.items()
        if key not in {"schema", "directory", "digest"}
    }
    if (
        binding["diagnosis"] != {"directory": directory, "digest": diagnosis["digest"]}
        or binding["request_digest"] != digest(mission["request"])
        or binding["identity"] != "lift:" + mission["request"]["selector"]
        or binding["clock"] != diagnosis["clock"]
        or binding["capabilities"] != MODE
    ):
        raise ValueError("writer slot differs from the retained diagnosis")
    return binding


def verify_writer_result(
    root: Path,
    directory: str,
    diagnosis: dict,
    mission: dict,
    expected_digest: str | None,
) -> dict | None:
    if leaf_stat(root, directory + "/writer.json") is None:
        if expected_digest is not None:
            raise ValueError("writer pin supplied without a diagnosis-owned dispatch")
        return None
    if expected_digest is None:
        raise ValueError("managed lift needs an external writer completion pin")
    slot = read_record(root, directory + "/writer.json")
    binding = validate_writer_slot(slot, directory, diagnosis, mission)
    expected_directory = (
        f"out/reviews/dispatch/{binding['budget_digest'][3:]}/{binding['sequence']}"
    )
    if slot.get("directory") != expected_directory:
        raise ValueError("writer dispatch directory differs from its original debit")
    consumption = read_record(root, expected_directory + "/consumption.json")
    if (
        consumption.get("digest") != binding["checkpoint_digest"]
        or digest({key: value for key, value in consumption.items() if key != "digest"})
        != binding["checkpoint_digest"]
        or consumption.get("schema") != "bof3.execution-consumption/v1"
        or consumption.get("budget_digest") != binding["budget_digest"]
        or consumption.get("sequence") != binding["sequence"]
    ):
        raise ValueError("writer consumption differs from its original debit")
    if leaf_stat(root, expected_directory + "/failure.json") is not None:
        raise ValueError(
            "managed writer failed; parent termination/recovery inspection required"
        )
    result = read_record(root, expected_directory + "/result.json")
    if (
        result.get("schema") != "bof3.codex-lift/v1"
        or result.get("digest") != expected_digest
        or digest({key: value for key, value in result.items() if key != "digest"})
        != expected_digest
        or any(result.get(key) != value for key, value in binding.items())
        or any(
            result.get(key) is not True
            for key in (
                "transport_completed",
                "original_owner_confirmed_cleanup",
                "parent_audit_required",
            )
        )
        or any(
            result.get(key) is not False
            for key in ("source_accepted", "restoration_authorized", "retry_authorized")
        )
        or result.get("artifacts") != hash_artifacts(root, expected_directory)
    ):
        raise ValueError("writer completion or retained artifacts drifted")
    return result
