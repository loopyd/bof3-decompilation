"""Generic structural parsing of bof3.naming-audit/v3 report rows.

Pure report-shape validation shared by the row validator and the
post-apply transaction check: rung profiles, receipt-verified commands,
typed observations, corroborators, name terms, reviewed ranges, and
generated-work records.  Imports only domain modules; the analysis layer
keeps its scope/readiness derivation and supplies derived facts
(transaction scope, generated work, canonical storage) to these helpers.
"""

from __future__ import annotations

import re
from typing import Any

from .claims import resolve_source_for_paths
from .naming_debt import address_of
from .receipts import command_records, typed_observation_ids
from .registry import resolve_target
from .symbols import name_terms
from .tags import parse_progress_tags

RUNG_PROFILES = {
    ("function", False): {"selected_range", "selected_call", "one_level_beyond"},
    ("function", True): {
        "selected_call",
        "owner_resolution",
        "owner_body",
        "one_level_beyond",
    },
    ("data", False): {
        "selected_range",
        "selected_access",
        "storage_class",
        "one_level_beyond",
    },
    ("data", True): {
        "selected_access",
        "owner_resolution",
        "owner_data",
        "storage_class",
        "one_level_beyond",
    },
}
_RANGE = re.compile(r"(0x[0-9A-Fa-f]{8})\.\.(0x[0-9A-Fa-f]{8})")


def strings(value: object, field: str, *, minimum: int = 1) -> list[str]:
    if (
        not isinstance(value, list)
        or len(value) < minimum
        or (not all((isinstance(item, str) and item.strip() for item in value)))
    ):
        raise ValueError(f"{field} must contain at least {minimum} non-empty strings")
    return value


def row_rungs(
    row: dict[str, object], kind: str, outside: bool, root, name: str
) -> bool:
    """v3 rung check: typed observations + receipt-verified commands."""
    value = row.get("rungs")
    if not isinstance(value, dict):
        raise ValueError(f"{name}.rungs must be an object")
    required = set(RUNG_PROFILES[kind, outside])
    partial_used = row.get("partial_used")
    if not isinstance(partial_used, bool):
        raise ValueError(f"{name}.partial_used must be boolean")
    if partial_used:
        required.add("partial_baseline")
    missing = required - set(value)
    if missing:
        raise ValueError(f"{name} missing required rungs: {sorted(missing)}")
    failed = False
    for rung_name in sorted(required):
        rung = value[rung_name]
        if not isinstance(rung, dict) or rung.get("status") not in {
            "passed",
            "negative",
            "failed",
            "open",
        }:
            raise ValueError(f"{name}.{rung_name} has invalid status")
        commands = [] if rung["status"] == "open" else command_records(
            rung.get("commands"), f"{name}.{rung_name}.commands", root
        )
        if rung["status"] == "open" and (
            not isinstance(rung.get("next_command"), str)
            or not rung["next_command"].strip()
        ):
            raise ValueError(f"{name}.{rung_name} open status requires next_command")
        observations = rung.get("observations")
        if not isinstance(observations, list) or not observations:
            raise ValueError(f"{name}.{rung_name}.observations required")
        observation_ids = typed_observation_ids(
            observations, f"{name}.{rung_name}.observations"
        )
        if len(set(observation_ids)) != len(observation_ids):
            raise ValueError(f"{name}.{rung_name}.observations have duplicate ids")
        if rung["status"] == "negative" and (
            not isinstance(rung.get("negative_result"), str)
            or not rung["negative_result"].strip()
        ):
            raise ValueError(
                f"{name}.{rung_name} negative status requires negative_result"
            )
        if not isinstance(rung.get("authority"), str) or not rung["authority"].strip():
            raise ValueError(f"{name}.{rung_name} missing authority")
        command_failed = any((command["status"] == "failed" for command in commands))
        if rung["status"] != "open" and (rung["status"] == "failed") != command_failed:
            raise ValueError(f"{name}.{rung_name} status disagrees with commands")
        failed |= command_failed or rung["status"] == "open"
    return failed


def row_observation_ids(row: dict[str, object], name: str) -> set[str]:
    rungs = row.get("rungs")
    if not isinstance(rungs, dict):
        raise ValueError(f"{name}.rungs must be an object")
    ids: set[str] = set()
    for rung_name, rung in rungs.items():
        if isinstance(rung, dict) and isinstance(rung.get("observations"), list):
            ids.update(
                typed_observation_ids(
                    rung["observations"], f"{name}.{rung_name}.observations"
                )
            )
    return ids


def corroborators(
    row: dict[str, object], name: str, observation_ids: set[str]
) -> dict[str, dict[str, Any]]:
    value = row.get("corroborators")
    if not isinstance(value, dict) or len(value) < 2:
        raise ValueError("v3 corroborators must be a typed mapping with 2+ entries")
    parsed: dict[str, dict[str, Any]] = {}
    for label, spec in value.items():
        if not isinstance(spec, dict):
            raise ValueError(f"{name}.corroborators[{label}] must be an object")
        ids = spec.get("observation_ids")
        if (
            not isinstance(ids, list)
            or not ids
            or (not all((isinstance(item, str) for item in ids)))
        ):
            raise ValueError(f"{name}.corroborators[{label}] requires observation_ids")
        unknown = set(ids) - observation_ids
        if unknown:
            raise ValueError(
                f"{name}.corroborators[{label}] cites unknown observations: {sorted(unknown)}"
            )
        mechanism = spec.get("mechanism")
        if not isinstance(mechanism, str) or not mechanism.strip():
            raise ValueError(f"{name}.corroborators[{label}] requires mechanism")
        parsed[str(label)] = {"observation_ids": ids, "mechanism": mechanism}
    labels = sorted(parsed)
    allowed_mechanisms = {
        "selected_original_instructions",
        "independent_caller",
        "independent_callee",
        "reviewed_layout",
        "independent_initializer",
        "independent_consumer",
        "runtime_trace",
    }
    for label in labels:
        if parsed[label]["mechanism"] not in allowed_mechanisms:
            raise ValueError(f"{name}.corroborators[{label}] has unknown mechanism")
    for i, left in enumerate(labels):
        for right in labels[i + 1 :]:
            shared = set(parsed[left]["observation_ids"]) & set(
                parsed[right]["observation_ids"]
            )
            if shared:
                raise ValueError(f"{name}.corroborators {left}/{right} share evidence")
            if parsed[left]["mechanism"] == parsed[right]["mechanism"]:
                raise ValueError(
                    f"{name}.corroborators {left}/{right} duplicate mechanism"
                )
    return parsed


def name_terms_v3(
    row: dict[str, object], new_name: str, corroborators: dict[str, dict[str, Any]]
) -> None:
    terms = row.get("name_terms")
    required = name_terms(new_name)
    if not isinstance(terms, dict):
        raise ValueError("name_terms must cover new_name and cite corroborators")
    cited: set[str] = set()
    for term, references in terms.items():
        if str(term).lower() not in required:
            continue
        labels = references if isinstance(references, list) else [references]
        if not labels or not all(
            (isinstance(label, str) and label in corroborators for label in labels)
        ):
            raise ValueError(f"name_terms[{term}] must cite known corroborators")
        cited.update(labels)
    if {str(term).lower() for term in terms} != required or not cited:
        raise ValueError("name_terms must cover new_name and cite corroborators")


def parse_range(value: object, name: str) -> tuple[int, int]:
    """Parse one proposed ``unchanged_range`` as two hex bounds.

    Strict: exactly two ``0x``-prefixed 8-digit bounds; looser spellings
    cannot be bound to a reviewed Splat range.
    """
    match = _RANGE.fullmatch(str(value))
    if match is None:
        raise ValueError(
            f"identity.unchanged_range must be '0xXXXXXXXX..0xYYYYYYYY' for {name}"
        )
    return int(match.group(1), 16), int(match.group(2), 16)


def range_contains(captured: object, address: int) -> bool:
    """True when a captured strict reviewed range contains the symbol."""
    match = _RANGE.fullmatch(str(captured or ""))
    if match is None:
        return False
    start, end = int(match.group(1), 16), int(match.group(2), 16)
    return start <= address < end


def scope_equality(
    row: dict[str, object], name: str, target: str, scope: dict[str, Any]
) -> None:
    """Proposed identity must equal the derived transaction scope exactly."""
    identity = row.get("identity")
    if not isinstance(identity, dict):
        raise ValueError(f"proposed row requires complete identity for {name}")
    if identity.get("selector") != f"{target}@0x{address_of(name):08X}":
        raise ValueError(f"proposed row requires complete identity for {name}")
    if identity.get("old") != name:
        raise ValueError(f"proposed identity old must be {name}")
    if not isinstance(identity.get("new"), str) or not re.fullmatch(
        r"[a-z][A-Za-z0-9]*", str(identity["new"])
    ):
        raise ValueError(f"new_name must be camelCase for {name}")
    range_start, range_end = parse_range(identity.get("unchanged_range"), name)
    if not range_start <= address_of(name) < range_end:
        raise ValueError(f"proposed identity range must contain symbol for {name}")
    reported_bindings = set(
        strings(identity.get("binding_locations"), f"{name}.binding_locations")
    )
    expected_bindings = set(scope["binding_locations"])
    missing = sorted(expected_bindings - reported_bindings)
    invented = sorted(reported_bindings - expected_bindings)
    if missing or invented:
        raise ValueError(
            f"{name} binding_locations must equal derived scope: missing={missing} invented={invented}"
        )
    reported_sources = set(
        strings(identity.get("source_locations"), f"{name}.source_locations")
    )
    expected_sources = set(scope["source_locations"])
    if scope["definition"]:
        expected_sources.add(scope["definition"])
    missing = sorted(expected_sources - reported_sources)
    invented = sorted(reported_sources - expected_sources)
    if missing or invented:
        raise ValueError(
            f"{name} source_locations must equal derived scope: missing={missing} invented={invented}"
        )


def check_closed_work(record: dict[str, Any], field: str, root) -> None:
    commands = command_records(record.get("commands"), f"{field}.commands", root)
    if any((command["status"] == "failed" for command in commands)):
        raise ValueError(f"{field} completed work contains failed command")
    typed_observation_ids(record.get("observations"), f"{field}.observations")


def required_work(
    row: dict[str, object], name: str, root, generated: list[dict[str, str]]
) -> bool:
    """Validate generated required work; return whether the row is blocked."""
    items = row.get("required_work")
    if not isinstance(items, list):
        raise ValueError(f"{name}.required_work must be an array")
    if {item.get("id") for item in items if isinstance(item, dict)} != {
        item["id"] for item in generated
    }:
        raise ValueError(f"{name}.required_work must match generated work items")
    by_id = {item.get("id"): item for item in items if isinstance(item, dict)}
    generated_by_id = {item["id"]: item for item in generated}
    blocked = False
    for item in generated:
        record = by_id.get(item["id"], {})
        status = record.get("status")
        if status not in {"completed", "duplicate", "open"}:
            raise ValueError(f"{name}.{item['id']} requires a work status")
        if status == "open":
            blocked = True
            continue
        if status == "duplicate":
            duplicate_of = record.get("duplicate_of")
            if not isinstance(duplicate_of, str) or duplicate_of not in by_id:
                raise ValueError(
                    f"{name}.{item['id']} duplicate requires existing duplicate_of"
                )
            source = by_id[duplicate_of]
            source_generated = generated_by_id[duplicate_of]
            if (
                source.get("status") != "completed"
                or source_generated["profile"] != item["profile"]
            ):
                raise ValueError(
                    f"{name}.{item['id']} duplicate_of must be completed same-profile work"
                )
            continue
        check_closed_work(record, f"{name}.{item['id']}", root)
    return blocked


def optional_work(
    row: dict[str, object], name: str, generated: list[dict[str, str]], blocked: bool
) -> None:
    optional = row.get("optional_work")
    if not isinstance(optional, list):
        optional = []
    for item in optional:
        if (
            not isinstance(item, dict)
            or not isinstance(item.get("id"), str)
            or (not item["id"].strip())
        ):
            raise ValueError(f"{name}.optional_work entries require ids")
    if blocked:
        if row.get("rung_status") == "exhausted":
            raise ValueError(f"{name} has open required work and cannot be exhausted")
        return
    if row.get("rung_status") == "exhausted":
        ceiling = row.get("ceiling_next_command")
        if not isinstance(ceiling, str) or not ceiling.strip():
            raise ValueError(f"exhausted row requires ceiling_next_command for {name}")
        cited_optional = [item for item in optional if item["id"] in ceiling]
        if optional and (not cited_optional):
            raise ValueError(
                f"{name}.ceiling_next_command must cite an optional work item"
            )
        open_required = [item for item in generated if item["id"] in ceiling]
        if open_required:
            raise ValueError(
                f"{name}.ceiling_next_command cites required work: {sorted((item['id'] for item in open_required))}"
            )


def status_pair(row: dict[str, object], name: str, blocked: bool) -> None:
    if row.get("rung_status") != "proposed":
        return
    semantic = row.get("semantic_status")
    transaction = row.get("transaction_status")
    blockers = row.get("readiness_blockers")
    if semantic not in {"accepted", "rejected"}:
        raise ValueError(f"{name}.semantic_status must be accepted|rejected")
    allowed = (
        {"not_applicable"}
        if semantic == "rejected"
        else {"ready", "repairable", "blocked"}
    )
    if transaction not in allowed:
        raise ValueError(f"{name}.transaction_status invalid: {transaction}")
    if not isinstance(blockers, list) or not all(
        (isinstance(item, str) for item in blockers)
    ):
        raise ValueError(f"{name}.readiness_blockers must be an array")
    if transaction == "ready" and (blockers or blocked):
        raise ValueError(f"{name} ready transaction requires no blockers")
    if transaction in {"repairable", "blocked"} and (not blockers):
        raise ValueError(f"{name} {transaction} status requires blockers")
    if blocked and transaction == "ready":
        raise ValueError(f"{name} cannot be ready with blocked work")


def metadata_state(root, manifest, name: str) -> tuple[bool, str]:
    """Return (metadata_ok, detail) for the row's metadata-resolved source."""
    source = None
    try:
        resolved = resolve_target(root, manifest.id.value)
        source = resolve_source_for_paths(resolved.source_paths, address_of(name))
    except (FileNotFoundError, ValueError, RuntimeError):
        return (True, "no claimed source")
    if source is None:
        return (True, "no claimed source")
    try:
        parse_progress_tags(source.read_text(encoding="utf-8"))
    except ValueError as error:
        return (False, f"{source}: {error}")
    return (True, "metadata canonical")


__all__ = [
    "RUNG_PROFILES",
    "check_closed_work",
    "corroborators",
    "metadata_state",
    "name_terms_v3",
    "optional_work",
    "parse_range",
    "range_contains",
    "required_work",
    "row_observation_ids",
    "row_rungs",
    "scope_equality",
    "status_pair",
    "strings",
]
