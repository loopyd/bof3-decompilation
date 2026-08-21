"""Live-bound analysis candidates, command receipts, and observations."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from .ids import normalize_target_id
from .manifests import load_target_manifests
from .sources import local_include_files

CANDIDATE_SCHEMA = "bof3.analysis-candidate/v1"
TRANSACTION_SCHEMA = "bof3.analysis-transaction/v1"
CANDIDATE_KINDS = frozenset(
    {
        "rename",
        "typedef",
        "aggregate",
        "field",
        "enum",
        "prototype",
        "macro",
        "shared_template",
    }
)
CANDIDATE_STATUSES = frozenset(
    {"lead", "blocked", "proposed", "accepted", "rejected", "stale"}
)
AUTHORITY_CLASSES = frozenset({"original", "reviewed", "authored", "generated"})
PROVENANCE_KINDS = frozenset(
    {
        "original_bytes",
        "reviewed_splat",
        "reviewed_rizin",
        "target_manifest",
        "target_map",
        "source_metadata",
        "sdk_header",
        "caller",
        "callee",
        "initializer",
        "consumer",
    }
)


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _receipt_payload(command: dict[str, object]) -> dict[str, object]:
    return {
        "command": command.get("command"),
        "status": command.get("status"),
        "target": command.get("target"),
        "selector": command.get("selector"),
        "output": command.get("output"),
    }


def command_records(value: object, field: str, root: Path) -> list[dict[str, Any]]:
    """Validate receipts binding command/status/target/selector/output."""
    if not isinstance(value, list) or not value:
        raise ValueError(f"{field} must contain executed command records")
    result: list[dict[str, Any]] = []
    for command in value:
        if (
            not isinstance(command, dict)
            or not isinstance(command.get("command"), str)
            or not command["command"].strip()
            or command.get("status") not in {"passed", "failed"}
            or not isinstance(command.get("target"), str)
            or not command["target"].strip()
            or (
                command.get("selector") is not None
                and not isinstance(command.get("selector"), str)
            )
            or not isinstance(command.get("output"), str)
        ):
            raise ValueError(f"{field} contains an invalid command record")
        receipt = command.get("receipt")
        digest = command.get("sha256")
        if not isinstance(receipt, str) or not isinstance(digest, str):
            raise ValueError(f"{field} command requires receipt and sha256")
        if Path(receipt).is_absolute():
            raise ValueError(f"{field} receipt must be repo-relative")
        evidence_root = (root / "out/reviews/evidence").resolve()
        path = (root / receipt).resolve()
        if evidence_root not in path.parents or not path.is_file():
            raise ValueError(f"{field} command receipt missing or stale: {receipt}")
        try:
            recorded = json.loads(path.read_text(encoding="utf-8"))
        except (json.JSONDecodeError, UnicodeDecodeError) as error:
            raise ValueError(
                f"{field} command receipt is not structured: {receipt}"
            ) from error
        if recorded != _receipt_payload(command) or sha256_file(path) != digest:
            raise ValueError(f"{field} command receipt missing or stale: {receipt}")
        result.append(command)
    return result


def _nonempty_strings(value: object, field: str) -> list[str]:
    if (
        not isinstance(value, list)
        or not value
        or any(not isinstance(item, str) or not item.strip() for item in value)
    ):
        raise ValueError(f"{field} must be a non-empty string array")
    return sorted(set(value))


def _owned_paths(value: object, field: str, root: Path, allowed: set[str]) -> list[str]:
    paths = _nonempty_strings(value, field)
    for name in paths:
        path = Path(name)
        if path.is_absolute() or name not in allowed or not (root / path).is_file():
            raise ValueError(f"{field} contains an unowned path: {name}")
    return paths


def _target_owned_paths(root: Path, target: str) -> set[str]:
    manifests = load_target_manifests(root)
    manifest = manifests.get(target)
    if manifest is None:
        raise ValueError(f"unknown candidate target: {target}")
    claimed = {
        *manifest.sources,
        *manifest.support_sources,
        *manifest.headers,
        manifest.splat,
        f"config/targets/{target}/target.toml",
        f"config/targets/{target}/symbols.txt",
        f"config/targets/{target}/reviewed.rz",
    }
    seeds = [root / name for name in claimed if name]
    claimed.update(
        path.relative_to(root).as_posix()
        for path in local_include_files(root.resolve(), seeds)
    )
    return {name for name in claimed if name and (root / name).is_file()}


def _fingerprints(value: object, root: Path, owned: set[str]) -> dict[str, str]:
    if not isinstance(value, dict) or set(value) != owned:
        raise ValueError("fingerprints must cover every owner and location exactly")
    result: dict[str, str] = {}
    for name, digest in value.items():
        if not isinstance(digest, str) or len(digest) != 64:
            raise ValueError("fingerprints require SHA-256 values")
        path = root / name
        actual = sha256_file(path)
        if digest.lower() != actual:
            raise ValueError(f"stale fingerprint: {name}")
        result[name] = actual
    return dict(sorted(result.items()))


def _observations(value: object) -> list[dict[str, str]]:
    if not isinstance(value, list) or not value:
        raise ValueError("observations must contain independent evidence")
    result: list[dict[str, str]] = []
    for item in value:
        if (
            not isinstance(item, dict)
            or not isinstance(item.get("id"), str)
            or not item["id"].strip()
            or not isinstance(item.get("text"), str)
            or not item["text"].strip()
        ):
            raise ValueError("observations contain an invalid record")
        result.append({"id": item["id"], "text": item["text"]})
    ids = [item["id"].casefold() for item in result]
    if len(ids) != len(set(ids)):
        raise ValueError("observation ids must be case-insensitively unique")
    return sorted(result, key=lambda item: item["id"].casefold())


def _candidate_id(kind: str, target: str, address: int) -> str:
    return f"{kind}:{target}@{address:08X}"


def validate_candidate(value: object, root: Path) -> dict[str, Any]:
    """Validate one versioned candidate against its live repository inputs."""
    if not isinstance(value, dict) or value.get("schema") != CANDIDATE_SCHEMA:
        raise ValueError(f"candidate schema must be {CANDIDATE_SCHEMA}")
    kind = value.get("kind")
    status = value.get("status")
    if kind not in CANDIDATE_KINDS:
        raise ValueError(f"unknown candidate kind: {kind}")
    if status not in CANDIDATE_STATUSES:
        raise ValueError(f"unknown candidate status: {status}")
    target_value = value.get("target")
    if not isinstance(target_value, str):
        raise ValueError("candidate target must be canonical")
    target = normalize_target_id(target_value)
    if (
        target.value != target_value
        or ".emi" in target_value.lower()
        or "#" in target_value
    ):
        raise ValueError(
            "candidate target must be canonical and not an archive identity"
        )
    start = value.get("address")
    end = value.get("end")
    if (
        not isinstance(start, int)
        or isinstance(start, bool)
        or not 0 <= start <= 0xFFFFFFFF
    ):
        raise ValueError("candidate address must be a 32-bit integer")
    if end is not None and (
        not isinstance(end, int)
        or isinstance(end, bool)
        or not start < end <= 0x100000000
    ):
        raise ValueError("candidate range must be a non-empty half-open range")
    expected_id = _candidate_id(str(kind), target.value, start)
    if value.get("id") != expected_id:
        raise ValueError(f"candidate id must equal canonical identity {expected_id}")
    allowed_paths = _target_owned_paths(root, target.value)
    owners = _owned_paths(value.get("owners"), "owners", root, allowed_paths)
    locations = _owned_paths(value.get("locations"), "locations", root, allowed_paths)
    authority = value.get("authority")
    if authority not in AUTHORITY_CLASSES:
        raise ValueError(f"unknown authority class: {authority}")
    provenance = _nonempty_strings(value.get("provenance"), "provenance")
    if any(item not in PROVENANCE_KINDS for item in provenance):
        raise ValueError("candidate contains invented provenance")
    missing_facts = value.get("missing_facts")
    if not isinstance(missing_facts, list) or any(
        not isinstance(item, str) or not item.strip() for item in missing_facts
    ):
        raise ValueError("missing_facts must be a string array")
    receipts = value.get("receipts")
    if not isinstance(receipts, list):
        raise ValueError("receipts must be an array")
    normalized_receipts = (
        command_records(receipts, "candidate.receipts", root) if receipts else []
    )
    owned = set(owners) | set(locations)
    return {
        "schema": CANDIDATE_SCHEMA,
        "id": expected_id,
        "kind": kind,
        "status": status,
        "target": target.value,
        "address": start,
        "end": end,
        "owners": owners,
        "locations": locations,
        "fingerprints": _fingerprints(value.get("fingerprints"), root, owned),
        "provenance": provenance,
        "authority": authority,
        "observations": _observations(value.get("observations")),
        "missing_facts": sorted(set(missing_facts)),
        "receipts": normalized_receipts,
    }


def validate_transaction(value: object, root: Path) -> dict[str, Any]:
    """Validate a concern-isolated transaction with unique identities."""
    if not isinstance(value, dict) or value.get("schema") != TRANSACTION_SCHEMA:
        raise ValueError(f"transaction schema must be {TRANSACTION_SCHEMA}")
    concern = value.get("concern")
    if concern not in CANDIDATE_KINDS:
        raise ValueError(f"unknown transaction concern: {concern}")
    candidates = value.get("candidates")
    if not isinstance(candidates, list) or not candidates:
        raise ValueError("transaction candidates must be non-empty")
    normalized = [validate_candidate(candidate, root) for candidate in candidates]
    if any(candidate["kind"] != concern for candidate in normalized):
        raise ValueError("transaction cannot mix candidate concerns")
    ids = [candidate["id"].casefold() for candidate in normalized]
    identities = [
        (candidate["kind"], candidate["target"], candidate["address"])
        for candidate in normalized
    ]
    if len(ids) != len(set(ids)) or len(identities) != len(set(identities)):
        raise ValueError("transaction candidate identities must be unique")
    return {
        "schema": TRANSACTION_SCHEMA,
        "concern": concern,
        "candidates": sorted(
            normalized, key=lambda candidate: candidate["id"].casefold()
        ),
    }


def canonical_json(value: object, root: Path, *, transaction: bool = False) -> str:
    """Serialize a live-validated candidate or transaction deterministically."""
    normalized = (
        validate_transaction(value, root)
        if transaction
        else validate_candidate(value, root)
    )
    return json.dumps(normalized, sort_keys=True, separators=(",", ":")) + "\n"


def typed_observation_ids(observations: object, field: str) -> list[str]:
    """Typed observation records: collect their observation ids."""
    if not isinstance(observations, list) or not observations:
        raise ValueError(f"{field} must contain observation records")
    ids: list[str] = []
    for observation in observations:
        if isinstance(observation, str):
            ids.append(observation)
        elif isinstance(observation, dict):
            if (
                not isinstance(observation.get("id"), str)
                or not observation["id"].strip()
            ):
                raise ValueError(f"{field} observation requires an id")
            if (
                not isinstance(observation.get("text"), str)
                or not observation["text"].strip()
            ):
                raise ValueError(f"{field} observation requires text")
            ids.append(observation["id"])
        else:
            raise ValueError(f"{field} entries must be strings or records")
    return ids


__all__ = [
    "AUTHORITY_CLASSES",
    "CANDIDATE_KINDS",
    "CANDIDATE_SCHEMA",
    "CANDIDATE_STATUSES",
    "PROVENANCE_KINDS",
    "TRANSACTION_SCHEMA",
    "canonical_json",
    "command_records",
    "sha256_file",
    "typed_observation_ids",
    "validate_candidate",
    "validate_transaction",
]
