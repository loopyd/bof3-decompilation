"""PsyQ signature call-site matching, index/calls IO, and promotion proposals."""

from __future__ import annotations

from collections import defaultdict
import json
import os
from pathlib import Path
import tempfile
from typing import Any, Iterable

from ..domain import load_target_manifests
from ..canonical import load_target_symbols
from ..snapshot import read_snapshot, snapshot_path


CALLS_SCHEMA = "bof3.psyq-calls/v1"
INDEX_SCHEMA = "bof3.psyq-signatures/v1"


def signature_index_path(root: Path) -> Path:
    return root / "out" / "psyq" / "index.json"


def calls_path(root: Path) -> Path:
    return root / "out" / "psyq" / "calls.json"


def proposal_path(root: Path) -> Path:
    return root / "out" / "psyq" / "proposal.json"


def _write_json(path: Path, payload: dict[str, Any]) -> None:
    """Atomically replace generated evidence, preserving the prior result on failure."""

    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temp_name = tempfile.mkstemp(
        dir=path.parent, prefix=f".{path.name}.", suffix=".tmp"
    )
    temporary = Path(temp_name)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8") as stream:
            json.dump(payload, stream, indent=2, sort_keys=True)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
        temporary.replace(path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def _call_addresses(snapshot) -> Iterable[tuple[str, int, int]]:
    for call in snapshot.calls:
        yield call.caller, call.callsite, int(call.callee.rsplit("@", 1)[1], 16)
    for call in snapshot.unresolved_calls:
        yield call.caller, call.callsite, call.target_address


def find_calls(root: Path) -> dict[str, Any]:
    """Join Rizin-derived snapshot call xrefs with generated signature labels."""

    path = signature_index_path(root)
    if not path.is_file():
        raise FileNotFoundError(
            f"Psy-Q index not found: {path.relative_to(root)}; run bin/harness psyq scan --all"
        )
    index = json.loads(path.read_text(encoding="utf-8"))
    if index.get("schema") != INDEX_SCHEMA or not isinstance(
        index.get("matches"), list
    ):
        raise ValueError(f"invalid Psy-Q index: {path.relative_to(root)}")
    labels: dict[tuple[str, int], list[dict[str, Any]]] = defaultdict(list)
    for match in index["matches"]:
        if not isinstance(match, dict):
            raise ValueError(f"invalid Psy-Q index match: {path.relative_to(root)}")
        target = match.get("target")
        for label in match.get("labels", []):
            if (
                isinstance(target, str)
                and isinstance(label, dict)
                and isinstance(label.get("address"), str)
            ):
                labels[(target, int(label["address"], 0))].append(
                    {
                        "library": match.get("library"),
                        "object": match.get("object"),
                        "versions": match.get("versions"),
                        "symbol": label.get("name"),
                    }
                )
    calls: list[dict[str, Any]] = []
    for target in sorted(index.get("targets", [])):
        snapshot_file = snapshot_path(root, target)
        if not snapshot_file.is_file():
            raise FileNotFoundError(
                f"missing Rizin snapshot: {snapshot_file.relative_to(root)}"
            )
        snapshot = read_snapshot(snapshot_file)
        if snapshot.target != target or snapshot.engine.get("name") != "rizin":
            raise ValueError(
                f"snapshot is not target-qualified Rizin evidence: {snapshot_file.relative_to(root)}"
            )
        for caller, callsite, destination in _call_addresses(snapshot):
            for symbol in labels.get((target, destination), []):
                calls.append(
                    {
                        "target": target,
                        "caller": caller,
                        "callsite": f"0x{callsite:08X}",
                        "address": f"0x{destination:08X}",
                        **symbol,
                    }
                )
    calls.sort(
        key=lambda row: (
            row["target"],
            int(row["callsite"], 16),
            row["symbol"],
            row["library"],
            row["object"],
        )
    )
    return {"schema": CALLS_SCHEMA, "calls": calls}


def write_calls(root: Path) -> dict[str, Any]:
    payload = find_calls(root)
    _write_json(calls_path(root), payload)
    return payload


def promotion_proposal(root: Path) -> dict[str, Any]:
    """Extract exact, reviewable external function-map candidates.

    This is intentionally an evidence export only. ``bin/symbols import-psyq``
    remains the sole map mutation path and still requires explicit selectors
    plus ``--write``.
    """

    index_file = signature_index_path(root)
    calls_file = calls_path(root)
    if not index_file.is_file() or not calls_file.is_file():
        raise FileNotFoundError("run bin/harness psyq scan --all and calls --all first")
    index = json.loads(index_file.read_text(encoding="utf-8"))
    calls = json.loads(calls_file.read_text(encoding="utf-8"))
    if index.get("schema") != INDEX_SCHEMA or calls.get("schema") != CALLS_SCHEMA:
        raise ValueError("invalid Psy-Q signature evidence")
    names: dict[tuple[str, int], set[str]] = defaultdict(set)
    labels: dict[tuple[str, int, str], list[dict[str, Any]]] = defaultdict(list)
    for match in index.get("matches", []):
        if not isinstance(match, dict) or not isinstance(match.get("target"), str):
            raise ValueError("invalid Psy-Q signature match")
        for label in match.get("labels", []):
            if not isinstance(label, dict) or not isinstance(label.get("name"), str):
                raise ValueError("invalid Psy-Q signature label")
            address = int(str(label["address"]), 0)
            key = (match["target"], address)
            names[key].add(label["name"])
            labels[(*key, label["name"])].append({"match": match, "label": label})
    call_evidence: dict[tuple[str, int, str], list[dict[str, str]]] = defaultdict(list)
    for call in calls.get("calls", []):
        if not isinstance(call, dict) or not all(
            isinstance(call.get(key), str)
            for key in ("target", "address", "symbol", "caller", "callsite")
        ):
            continue
        key = (str(call["target"]), int(str(call["address"]), 0), str(call["symbol"]))
        call_evidence[key].append(
            {"caller": str(call["caller"]), "callsite": str(call["callsite"])}
        )
    candidates: list[dict[str, Any]] = []
    for target, manifest in sorted(load_target_manifests(root).items()):
        current = {
            symbol.address: symbol for symbol in load_target_symbols(root, target)
        }
        current_names = {
            symbol.canonical_name: symbol.address for symbol in current.values()
        }
        for (label_target, address, name), evidence in sorted(labels.items()):
            if label_target != target or names[(target, address)] != {name}:
                continue
            label = evidence[0]["label"]
            declaration = label.get("declaration")
            if (
                not isinstance(declaration, dict)
                or declaration.get("kind") != "function"
            ):
                continue
            existing = current.get(address)
            if existing is not None and not existing.is_raw:
                continue
            if name in current_names and current_names[name] != address:
                continue
            evidence_calls = call_evidence.get((target, address, name), [])
            if not evidence_calls:
                continue
            matches = [row["match"] for row in evidence]
            candidates.append(
                {
                    "target": target,
                    "address": f"0x{address:08X}",
                    "name": name,
                    "confidence": "exact",
                    "external": True,
                    "declaration": declaration,
                    "objects": sorted(
                        {f"{match['library']}:{match['object']}" for match in matches}
                    ),
                    "versions": sorted(
                        {version for match in matches for version in match["versions"]}
                    ),
                    "calls": sorted(
                        evidence_calls, key=lambda row: (row["caller"], row["callsite"])
                    ),
                }
            )
    candidates.sort(key=lambda row: (row["target"], row["address"], row["name"]))
    return {"schema": "bof3.psyq-find/v1", "matches": candidates}


def write_promotion_proposal(root: Path) -> dict[str, Any]:
    """Write the generated exact-map proposal atomically."""

    payload = promotion_proposal(root)
    _write_json(proposal_path(root), payload)
    return payload
