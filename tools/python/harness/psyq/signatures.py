"""Match complete Psy-Q object signatures against target-qualified binaries.

The signature database identifies compiled objects only.  It intentionally does
not import SDK headers or mutate reviewed maps: headers own declarations, while
Rizin snapshots supply call-site evidence in :func:`find_calls`.
"""

from __future__ import annotations

from collections import defaultdict
import json
import os
from pathlib import Path
import tempfile
from typing import Any, Iterable

from ..domain import load_target_manifests
from ..snapshot import read_snapshot, snapshot_path


SIGNATURE_VERSIONS = (
    "3610",
    "3611",
    "370",
    "400",
    "410",
    "420",
    "430",
    "440",
    "450",
    "460",
    "470",
)
INDEX_SCHEMA = "bof3.psyq-signatures/v1"
CALLS_SCHEMA = "bof3.psyq-calls/v1"


def signature_root(root: Path) -> Path:
    return root / "toolchains" / "psx_psyq_signatures"


def index_path(root: Path) -> Path:
    return root / "out" / "psyq" / "index.json"


def calls_path(root: Path) -> Path:
    return root / "out" / "psyq" / "calls.json"


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


def _parse_signature(value: object, *, source: Path) -> tuple[bytes, bytes]:
    if not isinstance(value, str):
        raise ValueError(f"invalid signature in {source}")
    tokens = value.split()
    if not tokens or len(tokens) % 4:
        raise ValueError(f"signature must contain aligned bytes in {source}")
    data = bytearray()
    mask = bytearray()
    for token in tokens:
        if token == "??":
            data.append(0)
            mask.append(0)
            continue
        try:
            data.append(int(token, 16))
        except ValueError as exc:
            raise ValueError(f"invalid signature byte {token!r} in {source}") from exc
        mask.append(0xFF)
    return bytes(data), bytes(mask)


def _symbols(labels: object, *, source: Path) -> tuple[tuple[str, int], ...]:
    if not isinstance(labels, list):
        raise ValueError(f"invalid labels in {source}")
    rows: list[tuple[str, int]] = []
    for label in labels:
        if not isinstance(label, dict):
            raise ValueError(f"invalid label in {source}")
        name, offset = label.get("name"), label.get("offset")
        if not isinstance(name, str) or not isinstance(offset, int) or offset < 0:
            raise ValueError(f"invalid label in {source}")
        # `loc_` and `text_` are analyzer-internal branch labels, not callable
        # Psy-Q symbols.  Keep only the object symbols that can name a call.
        if name.startswith(("loc_", "text_")):
            continue
        rows.append((name, offset))
    return tuple(sorted(set(rows), key=lambda row: (row[1], row[0])))


def _signature_entries(root: Path) -> list[dict[str, Any]]:
    directory = signature_root(root)
    if not directory.is_dir() or not (directory / ".git").exists():
        raise FileNotFoundError(
            "Psy-Q signature submodule is unavailable; run "
            "git submodule update --init toolchains/psx_psyq_signatures"
        )
    grouped: dict[tuple[str, str, bytes, bytes], dict[str, set[Any]]] = {}
    seen_versions: set[str] = set()
    for version in SIGNATURE_VERSIONS:
        version_directory = directory / version
        if not version_directory.is_dir():
            continue
        seen_versions.add(version)
        paths = sorted(
            [*version_directory.glob("*.LIB.json"), *version_directory.glob("*.OBJ.json")]
        )
        for path in paths:
            raw = json.loads(path.read_text(encoding="utf-8"))
            if not isinstance(raw, list):
                raise ValueError(f"signature document is not a list: {path}")
            for row in raw:
                if not isinstance(row, dict) or not isinstance(row.get("name"), str):
                    raise ValueError(f"invalid signature object in {path}")
                # The upstream database retains a few named objects without
                # recovered bytes.  They are useful catalog entries but offer
                # no complete-object signature to scan.
                if not isinstance(row.get("sig"), str) or not row["sig"].split():
                    continue
                data, mask = _parse_signature(row.get("sig"), source=path)
                symbols = _symbols(row.get("labels"), source=path)
                key = (path.name.removesuffix(".json"), row["name"], data, mask)
                merged = grouped.setdefault(key, {"versions": set(), "symbols": set()})
                merged["versions"].add(version)
                merged["symbols"].update(symbols)
    if not seen_versions:
        raise ValueError(f"no requested Psy-Q signature versions under {directory}")
    entries: list[dict[str, Any]] = []
    for (library, object_name, data, mask), merged in grouped.items():
        fixed = _longest_fixed_run(data, mask)
        entries.append(
            {
                "library": library,
                "object": object_name,
                "data": data,
                "mask": mask,
                "symbols": tuple(sorted(merged["symbols"], key=lambda row: (row[1], row[0]))),
                "versions": sorted(merged["versions"]),
                "anchor": fixed,
            }
        )
    return sorted(entries, key=lambda row: (row["library"], row["object"], row["versions"]))


def _longest_fixed_run(data: bytes, mask: bytes) -> tuple[int, bytes]:
    best_start = best_end = 0
    start: int | None = None
    for index, value in enumerate(mask + b"\0"):
        if value and start is None:
            start = index
        elif not value and start is not None:
            if index - start > best_end - best_start:
                best_start, best_end = start, index
            start = None
    return best_start, data[best_start:best_end]


def _matches(payload: bytes, data: bytes, mask: bytes, anchor: tuple[int, bytes]) -> Iterable[int]:
    anchor_offset, needle = anchor
    if len(data) > len(payload):
        return
    if not needle:
        # A completely wildcard object is not evidence and would otherwise
        # produce every aligned offset in every target.
        return
    start = 0
    while True:
        found = payload.find(needle, start)
        if found < 0:
            return
        offset = found - anchor_offset
        start = found + 1
        if offset < 0 or offset % 4 or offset + len(data) > len(payload):
            continue
        candidate = payload[offset : offset + len(data)]
        if all(not required or candidate[index] == data[index] for index, required in enumerate(mask)):
            yield offset


def scan(root: Path) -> dict[str, Any]:
    """Scan every manifest binary and return complete-object signature evidence."""

    manifests = load_target_manifests(root)
    entries = _signature_entries(root)
    results: list[dict[str, Any]] = []
    for target, manifest in sorted(manifests.items()):
        binary = root / manifest.binary
        if not binary.is_file():
            raise FileNotFoundError(f"missing target binary: {manifest.binary}")
        payload = binary.read_bytes()
        for entry in entries:
            for offset in _matches(payload, entry["data"], entry["mask"], entry["anchor"]):
                labels = [
                    {"name": name, "address": f"0x{manifest.load_address + offset + label_offset:08X}"}
                    for name, label_offset in entry["symbols"]
                ]
                results.append(
                    {
                        "target": target,
                        "address": f"0x{manifest.load_address + offset:08X}",
                        "library": entry["library"],
                        "object": entry["object"],
                        "versions": entry["versions"],
                        "symbols": [name for name, _ in entry["symbols"]],
                        "labels": labels,
                    }
                )
    results.sort(
        key=lambda row: (
            row["target"], int(row["address"], 16), row["library"], row["object"], row["versions"]
        )
    )
    return {
        "schema": INDEX_SCHEMA,
        "signature_versions": list(SIGNATURE_VERSIONS),
        "targets": sorted(manifests),
        "matches": results,
    }


def write_index(root: Path) -> dict[str, Any]:
    payload = scan(root)
    _write_json(index_path(root), payload)
    return payload


def _call_addresses(snapshot) -> Iterable[tuple[str, int, int]]:
    for call in snapshot.calls:
        yield call.caller, call.callsite, int(call.callee.rsplit("@", 1)[1], 16)
    for call in snapshot.unresolved_calls:
        yield call.caller, call.callsite, call.target_address


def find_calls(root: Path) -> dict[str, Any]:
    """Join Rizin-derived snapshot call xrefs with generated signature labels."""

    path = index_path(root)
    if not path.is_file():
        raise FileNotFoundError(f"Psy-Q index not found: {path.relative_to(root)}; run bin/harness psyq scan --all")
    index = json.loads(path.read_text(encoding="utf-8"))
    if index.get("schema") != INDEX_SCHEMA or not isinstance(index.get("matches"), list):
        raise ValueError(f"invalid Psy-Q index: {path.relative_to(root)}")
    labels: dict[tuple[str, int], list[dict[str, Any]]] = defaultdict(list)
    for match in index["matches"]:
        if not isinstance(match, dict):
            raise ValueError(f"invalid Psy-Q index match: {path.relative_to(root)}")
        target = match.get("target")
        for label in match.get("labels", []):
            if isinstance(target, str) and isinstance(label, dict) and isinstance(label.get("address"), str):
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
            raise FileNotFoundError(f"missing Rizin snapshot: {snapshot_file.relative_to(root)}")
        snapshot = read_snapshot(snapshot_file)
        if snapshot.target != target or snapshot.engine.get("name") != "rizin":
            raise ValueError(f"snapshot is not target-qualified Rizin evidence: {snapshot_file.relative_to(root)}")
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
    calls.sort(key=lambda row: (row["target"], int(row["callsite"], 16), row["symbol"], row["library"], row["object"]))
    return {"schema": CALLS_SCHEMA, "calls": calls}


def write_calls(root: Path) -> dict[str, Any]:
    payload = find_calls(root)
    _write_json(calls_path(root), payload)
    return payload


__all__ = [
    "CALLS_SCHEMA",
    "INDEX_SCHEMA",
    "SIGNATURE_VERSIONS",
    "calls_path",
    "find_calls",
    "index_path",
    "scan",
    "write_calls",
    "write_index",
]
