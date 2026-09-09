"""Closed frozen-report parsing and derived naming postapply input state."""

from __future__ import annotations

import hashlib
import json
import os
import subprocess
from pathlib import Path

from harness.common.inputs import file_state, input_state, relative
from harness.domain.ids import normalize_target_id
from harness.domain.manifests import load_target_manifests
from harness.domain.symbols import load_target_symbols
from harness.io import unique_object
from harness.naming.annotations import reviewed_scope_digest
from harness.naming.context import READY_STATUS, SCHEMA_V3, TargetContext
from harness.naming.proposal import (
    canonical_report_path,
    require_provenance,
    validate_authored_digests,
)
from harness.naming.reports import _binding_scope_holds


def digest(value: object) -> str:
    return hashlib.sha256(
        json.dumps(value, sort_keys=True, separators=(",", ":")).encode()
    ).hexdigest()


def keys(value, allowed: str, *, exact: bool = True) -> None:
    expected = set(allowed.split())
    if not isinstance(value, dict) or (
        set(value) != expected if exact else set(value) - expected
    ):
        raise ValueError("noncanonical postapply object keys")


def load(path: Path):
    return json.loads(path.read_bytes(), object_pairs_hook=unique_object)


def index_digest(root: Path) -> str:
    result = subprocess.run(
        ["git", "rev-parse", "--path-format=absolute", "--git-path", "index"],
        cwd=root,
        capture_output=True,
        check=True,
        timeout=120,
    )
    state = file_state(Path(os.fsdecode(result.stdout.rstrip(b"\n"))))
    return digest(state)


# ponytail: closed v3 record vocabulary, not a second semantic validator; extend
# only alongside an owning schema change. Dynamic labels are explicit below.
_SHAPES = {
    "report": "schema target complete rows",
    "row": "authority ceiling_next_command conclusion_provenance corroborators identity initializer_state interpretation kind manifest missing_fact name name_terms new_name optional_work outside_payload partial_used pre_apply readiness_blockers required_work rung_status rungs semantic_status smallest_repair transaction_status storage",
    "provenance": "kind report report_sha256 row_sha256 pre_apply manifest evidence evidence_sha256 evidence_namespace initializer_report_sha256",
    "namespace": "mode namespace root",
    "binding": "version digest facts",
    "facts": "address destination kind new_name old_name reviewed reviewed_digest scope selector status unchanged_range work storage",
    "scope": "address binding_locations cross_target_locations definition kind manifest map old_name source_locations splat target",
    "status": "rung_status semantic_status transaction_status",
    "identity": "binding_locations new old selector source_locations unchanged_range",
    "manifest": "baseline collision inventory pre_apply_digest range required_checks reviewed reviewed_digest schema scope storage transaction work",
    "inventory": "address kind new_name old_name selector",
    "work": "commands description id observations profile status next_command reason",
    "rung": "authority commands next_command observations status negative_result",
    "command": "command item operation output receipt selector sha256 status supplemental target",
    "observation": "id producer source_id text",
    "corroborator": "mechanism observation_ids source_id",
    "storage": "kind start end file_offset present_in_binary authority",
}
_CHILDREN = {
    "rows": "row",
    "conclusion_provenance": "provenance",
    "evidence_namespace": "namespace",
    "pre_apply": "binding",
    "facts": "facts",
    "scope": "scope",
    "status": "status",
    "identity": "identity",
    "manifest": "manifest",
    "baseline": "status",
    "inventory": "inventory",
    "work": "work",
    "required_work": "work",
    "optional_work": "work",
    "commands": "command",
    "observations": "observation",
    "storage": "storage",
}


def _shape(value, kind: str) -> None:
    if isinstance(value, list):
        for item in value:
            _shape(item, kind)
    elif isinstance(value, dict):
        keys(value, _SHAPES[kind], exact=False)
        for key, item in value.items():
            if key in {"rungs", "corroborators", "name_terms"}:
                if not isinstance(item, dict):
                    raise ValueError("invalid labeled report record")
                if key == "rungs":
                    keys(
                        item,
                        "one_level_beyond owner_body owner_data owner_resolution partial_baseline selected_access selected_call selected_range storage_class",
                        exact=False,
                    )
                for child in item.values():
                    if key == "name_terms":
                        if not isinstance(child, list) or not all(
                            isinstance(x, str) for x in child
                        ):
                            raise ValueError("invalid name-term references")
                    else:
                        _shape(child, "rung" if key == "rungs" else "corroborator")
            elif isinstance(item, (dict, list)):
                if key in _CHILDREN:
                    _shape(item, _CHILDREN[key])
                elif isinstance(item, dict) or any(
                    isinstance(x, (dict, list)) for x in item
                ):
                    raise ValueError(f"unsupported nested report field: {key}")


def frozen(root: Path, target: str, report_path: Path, transaction: str):
    path = canonical_report_path(root, report_path)
    before = file_state(path)
    report = load(path)
    _shape(report, "report")
    keys(report, _SHAPES["report"])
    if (
        report["schema"] != SCHEMA_V3
        or report["target"] != target
        or normalize_target_id(target).value != target
    ):
        raise ValueError("postapply requires canonical v3 target")
    selected = [
        r for r in report["rows"] if f"{r.get('kind')}:{r.get('name')}" == transaction
    ]
    if len(selected) != 1:
        raise ValueError("postapply requires exactly one transaction")
    row = selected[0]
    provenance = require_provenance(row, transaction)
    validate_authored_digests(report, row, provenance)
    if provenance["report"] != path.relative_to(root).as_posix() or any(
        row.get(k) != provenance[k] for k in ("pre_apply", "manifest")
    ):
        raise ValueError("frozen proposal provenance mismatch")
    facts = row["pre_apply"]["facts"]
    if facts.get("status") != READY_STATUS or any(
        row.get(k) != v for k, v in READY_STATUS.items()
    ):
        raise ValueError("native gates require accepted ready proposal")
    if row["pre_apply"]["digest"] != "v1:" + digest(facts):
        raise ValueError("captured facts digest mismatch")
    if (
        row["kind"] != "function"
        or row.get("partial_used") is not False
        or row.get("outside_payload") is not False
        or facts["scope"]["cross_target_locations"]
    ):
        raise ValueError(
            "native postapply supports exact, in-payload, target-local FUNCTION transactions only"
        )
    if (
        facts["selector"] != row["identity"]["selector"]
        or facts["new_name"] != row["new_name"]
        or facts["old_name"] != row["name"]
        or facts["kind"] != "function"
        or not facts["selector"].startswith(target + "@0x")
    ):
        raise ValueError("captured identity mismatch")
    if file_state(path) != before:
        raise ValueError("frozen report changed while reading")
    return (
        report,
        row,
        {
            "report": path.relative_to(root).as_posix(),
            "report_state": before,
            "target": target,
            "transaction": transaction,
            "pre_apply_digest": row["pre_apply"]["digest"],
        },
    )


def transaction_paths(row) -> set[str]:
    facts = row["pre_apply"]["facts"]
    scope = facts["scope"]
    return {
        relative(p)
        for p in [
            *scope["binding_locations"],
            *scope["source_locations"],
            facts["destination"],
            scope["definition"],
        ]
    }


def state(root: Path, target: str, row) -> dict:
    return input_state(root, target, transaction_paths(row))


def applied_scope(root: Path, target: str, row) -> None:
    facts = row["pre_apply"]["facts"]
    old, new = row["name"], row["new_name"]
    address = int(facts["address"], 0)
    ctx = TargetContext(root, target, load_target_manifests(root)[target])
    _binding_scope_holds(facts["scope"], old, ctx)
    _binding_scope_holds({"source_locations": [facts["destination"]]}, old, ctx)
    actual = ctx.scope(new, address=address, kind="function")
    expected = set(facts["scope"]["source_locations"]) - {
        facts["scope"]["definition"]
    } | {facts["destination"]}
    if (
        set(actual["source_locations"]) != expected
        or set(actual["binding_locations"]) != set(facts["scope"]["binding_locations"])
        or actual["definition"] != facts["destination"]
    ):
        raise ValueError("applied naming scope mismatch")
    if (
        facts["scope"]["definition"] != facts["destination"]
        and (root / facts["scope"]["definition"]).exists()
    ):
        raise ValueError("superseded source still exists")
    mapped = [s.address for s in load_target_symbols(root, target) if s.name == new]
    if (
        mapped != [address]
        or reviewed_scope_digest(root, target) != facts["reviewed_digest"]
    ):
        raise ValueError("map address or reviewed annotations changed")
    if facts["destination"] not in ctx.manifest.sources:
        raise ValueError("destination is not a live manifest source")
