"""Pinned single-function lifter scope and retained mission report validation."""

from __future__ import annotations

from dataclasses import asdict
import json
import math
from pathlib import Path
import re
import yaml

from harness.analysis.mission import mission_brief
from harness.common.digests import digest
from harness.common.directory import validate_repo_path
from harness.common.files import read_file
from harness.common.workspace import workspace_baseline
from harness.context.capabilities import MODE
from harness.domain.claims import resolve_manifest_source_for_address
from harness.domain.ids import parse_function_id
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.psx import (
    is_psx_exe,
    payload_for,
    reviewed_range_digest,
    validate_psx_header,
)
from harness.domain.symbols import map_path
from harness.io import unique_object

from .inventory import capture_inventory, capture_owned, verify_scope
from .execution import capture_context


def validate_request(root: Path, request: object) -> dict:
    if (
        not isinstance(request, dict)
        or set(request)
        != {
            "schema",
            "selector",
            "source",
            "paths",
            "task",
            "adopted_baseline",
            "capabilities",
        }
        or request["schema"] != "bof3.lift-request/v1"
        or request["capabilities"] != MODE
    ):
        raise ValueError("invalid single-function lift request")
    if not isinstance(request["selector"], str):
        raise ValueError("lift selector must be a string")
    function = parse_function_id(request["selector"])
    if request["selector"] != str(function):
        raise ValueError("lift selector must use canonical machine spelling")
    paths = request["paths"]
    if (
        not isinstance(paths, list)
        or not paths
        or len(paths) > 12
        or any(not isinstance(path, str) for path in paths)
        or paths != sorted(set(paths))
    ):
        raise ValueError("lift scope needs sorted unique explicit paths")
    for path in paths:
        validate_repo_path(path)
    source = request["source"]
    if (
        source not in paths
        or not source.startswith("src/")
        or not source.endswith(".c")
    ):
        raise ValueError("lift source must be an explicitly owned C path")
    if (
        not isinstance(request["task"], str)
        or not request["task"].strip()
        or len(request["task"].encode()) > 8192
    ):
        raise ValueError("lift task must contain at most 8 KiB of text")
    if workspace_baseline(root)["digest"] != request["adopted_baseline"]:
        raise ValueError("lift adopted baseline drifted")
    manifests = load_target_manifests(root)
    target = function.target.value
    manifest = manifests.get(target)
    if manifest is None:
        raise ValueError(f"unknown lift target: {target}")
    resolved = resolve_manifest_source_for_address(root, manifest, function.address)
    if resolved is not None and resolved.relative_to(root).as_posix() != source:
        raise ValueError("lift source does not own the selected function")
    allowed = {
        source,
        manifest.splat,
        f"config/targets/{target}/target.toml",
        map_path(root, target).relative_to(root).as_posix(),
        *manifest.support_sources,
        *manifest.headers,
    }
    if set(paths) - allowed:
        raise ValueError("lift paths exceed the selected target's explicit scope")
    shared = {
        path
        for other in manifests.values()
        if other.id != manifest.id
        for path in (*other.sources, *other.support_sources, *other.headers)
    }
    if set(paths) & shared:
        raise ValueError("shared source/header changes need separate ownership")
    brief = mission_brief(root, str(function))
    size = brief["metrics"]["size"]
    binary = read_file(root, manifest.binary)
    if type(size) is not int or size <= 0 or size % 4:
        raise ValueError("lift needs a complete aligned original-byte span")
    if is_psx_exe(binary):
        validate_psx_header(binary, manifest.load_address, binary_name=manifest.binary)
    original = reviewed_range_digest(
        payload_for(binary, manifest.load_address, binary_name=manifest.binary),
        function.address,
        function.address + size,
        binary=binary,
    )
    if original is None:
        raise ValueError("lift original span escapes the target payload")
    return {
        "target": target,
        "existing_source": resolved is not None,
        "manifest": json.loads(json.dumps(asdict(manifest))),
        "splat_options": {
            key: value
            for key, value in yaml.safe_load(read_file(root, manifest.splat)).items()
            if key != "segments"
        },
        "original": {
            "path": manifest.binary,
            "address": function.address,
            "size": size,
            "sha256": original[0],
        },
    }


def capture_mission(root: Path, request: dict) -> dict:
    facts = validate_request(root, request)
    execution = capture_context(root, facts["target"], request["paths"])
    inventory_paths = sorted(set(request["paths"]) | set(execution["inputs"]))
    inventory = capture_inventory(root, inventory_paths)
    owned = capture_owned(root, request["paths"], inventory)
    if validate_request(root, request) != facts:
        raise ValueError("lift request changed during PRE capture")
    verify_scope(inventory, capture_inventory(root, inventory_paths), [])
    record = {
        "schema": "bof3.lift-mission/v1",
        "request": request,
        "facts": facts,
        "inventory_paths": inventory_paths,
        "inventory": inventory,
        "owned_pre": owned,
        "execution": execution,
        "source_accepted": False,
        "restoration_authorized": False,
    }
    return {**record, "digest": digest(record)}


def verify_mission(root: Path, record: dict, expected_digest: str) -> None:
    validate_mission_record(record, expected_digest)
    if capture_mission(root, record["request"]) != record:
        raise ValueError("lift mission or captured inputs drifted")


def validate_mission_record(record: dict, expected_digest: str) -> None:
    if (
        not isinstance(record, dict)
        or set(record)
        != {
            "schema",
            "request",
            "facts",
            "inventory_paths",
            "inventory",
            "owned_pre",
            "execution",
            "source_accepted",
            "restoration_authorized",
            "digest",
        }
        or record["source_accepted"] is not False
        or record["restoration_authorized"] is not False
        or record.get("schema") != "bof3.lift-mission/v1"
        or record.get("digest") != expected_digest
        or digest({key: value for key, value in record.items() if key != "digest"})
        != expected_digest
    ):
        raise ValueError("lift mission pin drifted")


def inspect_candidate(root: Path, record: dict) -> dict:
    request = record["request"]
    execution = capture_context(root, record["facts"]["target"], request["paths"])
    before = record["execution"]
    if {key: value for key, value in execution.items() if key != "inputs"} != {
        key: value for key, value in before.items() if key != "inputs"
    }:
        raise ValueError("lift execution environment or installed tools drifted")
    if set(execution["inputs"]) != set(before["inputs"]):
        raise ValueError(
            "lift native-input membership changed; parent analysis required"
        )
    if any(
        execution["inputs"][name] != state
        for name, state in before["inputs"].items()
        if name not in request["paths"]
    ):
        raise ValueError("lift changed an immutable native execution input")
    observed = capture_inventory(root, record["inventory_paths"])
    changed = verify_scope(record["inventory"], observed, request["paths"])
    function = parse_function_id(request["selector"])
    manifest = load_target_manifests(root)[function.target.value]
    splat_options = {
        key: value
        for key, value in yaml.safe_load(read_file(root, manifest.splat)).items()
        if key != "segments"
    }
    if splat_options != record["facts"]["splat_options"]:
        raise ValueError("lift changed Splat configuration beyond reviewed segments")
    current = json.loads(json.dumps(asdict(manifest)))
    before = record["facts"]["manifest"]
    if {key: value for key, value in current.items() if key != "sources"} != {
        key: value for key, value in before.items() if key != "sources"
    } or current["sources"] not in (
        before["sources"],
        [*before["sources"], request["source"]],
    ):
        raise ValueError("lift changed target configuration beyond source registration")
    resolved = resolve_manifest_source_for_address(root, manifest, function.address)
    if resolved is None or resolved.relative_to(root).as_posix() != request["source"]:
        raise ValueError("retained lift source does not own the selected function")
    if not record["facts"]["existing_source"]:
        boundary = parse_splat_layout(
            root / manifest.splat, manifest.load_address
        ).find_boundary_at(function.address)
        if (
            boundary is None
            or boundary.kind != "c"
            or boundary.source != request["source"]
            or not boundary.behavior
        ):
            raise ValueError(
                "new lift requires a C boundary with source and behavior metadata"
            )
    return {"inventory": observed, "changed_paths": changed}


def parse_result(text: str, selector: str) -> dict:
    reports = re.findall(
        r"(?m)^```(json|acceptance-report)\s*\n(.*?)^```\s*$", text, re.S
    )
    if [kind for kind, _body in reports] != ["json", "acceptance-report"]:
        raise ValueError("lift must return mission JSON followed by acceptance-report")
    mission, acceptance = [
        json.loads(body, object_pairs_hook=unique_object) for _, body in reports
    ]
    fields = {
        "function",
        "status",
        "match_percent",
        "files_changed",
        "matching_aids",
        "notes",
    }
    if not isinstance(mission, dict) or set(mission) != fields:
        raise ValueError("lift mission result fields differ from the protocol")
    if (
        not isinstance(mission["function"], str)
        or str(parse_function_id(mission["function"])) != selector
        or not isinstance(mission["status"], str)
        or mission["status"] not in {"exact", "partial", "escalated"}
        or type(mission["match_percent"]) not in (int, float)
        or not 0 <= mission["match_percent"] <= 100
        or not math.isfinite(mission["match_percent"])
        or not isinstance(mission["notes"], str)
    ):
        raise ValueError("lift mission identity or status is invalid")
    if mission["status"] == "exact" and mission["match_percent"] != 100:
        raise ValueError("exact lift claim must report 100 percent")
    if (
        not isinstance(mission["files_changed"], list)
        or any(not isinstance(path, str) for path in mission["files_changed"])
        or len(set(mission["files_changed"])) != len(mission["files_changed"])
    ):
        raise ValueError("lift changed-file list is invalid")
    for path in mission["files_changed"]:
        validate_repo_path(path)
    if not isinstance(mission["matching_aids"], list):
        raise ValueError("lift matching-aid report must be a list")
    if not isinstance(acceptance, dict) or set(acceptance) != {
        "pre_mission",
        "commands",
        "attempts",
        "risks",
        "retained_candidate",
        "remaining_candidates",
        "snapshot_index_refresh_required",
        "staged_index_changed",
        "parent_restore_required",
        "matching_aid_approvals",
    }:
        raise ValueError("lift acceptance report is incomplete")
    for name in (
        "snapshot_index_refresh_required",
        "staged_index_changed",
        "parent_restore_required",
    ):
        if type(acceptance[name]) is not bool:
            raise ValueError("lift acceptance flags must be booleans")
    for name in (
        "commands",
        "attempts",
        "risks",
        "remaining_candidates",
        "matching_aid_approvals",
    ):
        if not isinstance(acceptance[name], list):
            raise ValueError("lift acceptance lists are invalid")
    if not acceptance["attempts"]:
        raise ValueError("lift acceptance needs an ordered attempt ledger")
    if mission["status"] == "escalated" and not acceptance["parent_restore_required"]:
        raise ValueError("escalated lifts must request parent restoration review")
    for position, attempt in enumerate(acceptance["attempts"], 1):
        if (
            not isinstance(attempt, dict)
            or set(attempt)
            != {
                "order",
                "diagnosis",
                "change",
                "command",
                "result",
                "retained",
                "reason",
            }
            or type(attempt["order"]) is not int
            or attempt["order"] != position
            or type(attempt["retained"]) is not bool
            or any(
                not isinstance(attempt[key], str) or not attempt[key].strip()
                for key in ("diagnosis", "change", "command", "result", "reason")
            )
        ):
            raise ValueError("lift acceptance attempt ledger is invalid")
    return {"mission": mission, "acceptance": acceptance, "source_accepted": False}
