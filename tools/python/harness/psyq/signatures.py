"""Match complete Psy-Q object signatures against target-qualified binaries.

The signature database identifies compiled objects only.  It intentionally does
not import SDK headers or mutate reviewed maps: headers own declarations, while
Rizin snapshots supply call-site evidence in :func:`find_calls`.
"""

from __future__ import annotations

from collections import defaultdict
import json
from pathlib import Path
from typing import Any, Iterable

from ..domain import load_target_manifests
from .headers import (
    HEADER_SCHEMA,
    declaration_from_index,
    declarations_by_name,
    index_headers,
    parse_headers,
)

from .signature_calls import INDEX_SCHEMA, _write_json, signature_index_path

SIGNATURE_VERSIONS = (
    "260",
    "300",
    "330",
    "340",
    "350",
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
# BOF3's production window makes these database releases the useful first
# comparison set.  This is a review prior, not a claim that every object came
# from one SDK release; later byte-compatible signatures remain in the index.
HISTORICAL_PRIMARY_VERSIONS = ("3610", "3611", "370", "400")
REGIONAL_REBUILD_VERSIONS = ("410",)

def signature_root(root: Path) -> Path:
    return root / "toolchains" / "psx_psyq_signatures"

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
            [
                *version_directory.glob("*.LIB.json"),
                *version_directory.glob("*.OBJ.json"),
            ]
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
                "symbols": tuple(
                    sorted(merged["symbols"], key=lambda row: (row[1], row[0]))
                ),
                "versions": sorted(merged["versions"]),
                "anchor": fixed,
            }
        )
    return sorted(
        entries, key=lambda row: (row["library"], row["object"], row["versions"])
    )

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

def _matches(
    payload: bytes, data: bytes, mask: bytes, anchor: tuple[int, bytes]
) -> Iterable[int]:
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
        if all(
            not required or candidate[index] == data[index]
            for index, required in enumerate(mask)
        ):
            yield offset

def scan(root: Path) -> dict[str, Any]:
    """Scan every manifest binary and return complete-object signature evidence."""

    manifests = load_target_manifests(root)
    entries = _signature_entries(root)
    include_root = root / "toolchains" / "psyq" / "4.7" / "include"
    header_catalog = (
        parse_headers(include_root)
        if include_root.is_dir()
        else {"schema": HEADER_SCHEMA, "declarations": []}
    )
    header_index = declarations_by_name(header_catalog)
    results: list[dict[str, Any]] = []
    for target, manifest in sorted(manifests.items()):
        binary = root / manifest.binary
        if not binary.is_file():
            raise FileNotFoundError(f"missing target binary: {manifest.binary}")
        payload = binary.read_bytes()
        for entry in entries:
            for offset in _matches(
                payload, entry["data"], entry["mask"], entry["anchor"]
            ):
                labels = []
                for name, label_offset in entry["symbols"]:
                    label: dict[str, Any] = {
                        "name": name,
                        "address": f"0x{manifest.load_address + offset + label_offset:08X}",
                    }
                    declaration = declaration_from_index(header_index, name)
                    if declaration is not None:
                        label["declaration"] = declaration
                    labels.append(label)
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
            row["target"],
            int(row["address"], 16),
            row["library"],
            row["object"],
            row["versions"],
        )
    )
    version_evidence = _version_evidence(sorted(manifests), results)
    return {
        "schema": INDEX_SCHEMA,
        "signature_versions": list(SIGNATURE_VERSIONS),
        "targets": sorted(manifests),
        "matches": results,
        # A signature can be byte-compatible with several SDK releases.  This
        # ranks that compatibility per binary, while retaining counterexamples
        # so callers cannot mistake a best fit for provenance.
        "version_evidence": version_evidence,
    }

def _version_evidence(
    targets: list[str], matches: list[dict[str, Any]]
) -> list[dict[str, Any]]:
    """Summarize compatible SDK versions and their per-target disagreements."""

    by_target: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for match in matches:
        by_target[str(match["target"])].append(match)
    evidence: list[dict[str, Any]] = []
    for target in targets:
        target_matches = by_target[target]
        counts = {
            version: sum(version in match["versions"] for match in target_matches)
            for version in SIGNATURE_VERSIONS
        }
        alignment_scores = {
            version: round(
                sum(
                    1.0 / len(match["versions"])
                    for match in target_matches
                    if version in match["versions"]
                ),
                6,
            )
            for version in SIGNATURE_VERSIONS
        }
        maximum = max(counts.values(), default=0)
        best = [
            version
            for version in SIGNATURE_VERSIONS
            if maximum and counts[version] == maximum
        ]
        alignment_maximum = max(alignment_scores.values(), default=0.0)
        alignment_best = [
            version
            for version in SIGNATURE_VERSIONS
            if alignment_maximum and alignment_scores[version] == alignment_maximum
        ]
        historical_maximum = max(
            (counts[version] for version in HISTORICAL_PRIMARY_VERSIONS), default=0
        )
        historical_best = [
            version
            for version in HISTORICAL_PRIMARY_VERSIONS
            if historical_maximum and counts[version] == historical_maximum
        ]
        disagreements = [
            {key: match[key] for key in ("address", "library", "object", "versions")}
            for match in target_matches
            if best and not any(version in match["versions"] for version in best)
        ]
        evidence.append(
            {
                "target": target,
                "match_count": len(target_matches),
                # This is raw database compatibility.  It can be dominated by
                # objects unchanged in later SDK releases, so it is not a
                # provenance conclusion.
                "best_versions": best,
                # A complete object shared by N versions contributes 1/N to
                # each.  This avoids letting unchanged common objects drown
                # out version-specific evidence.
                "alignment_best_versions": alignment_best,
                "version_alignment_scores": [
                    {"version": version, "score": alignment_scores[version]}
                    for version in SIGNATURE_VERSIONS
                ],
                "historical_primary_versions": list(HISTORICAL_PRIMARY_VERSIONS),
                "historical_best_versions": historical_best,
                "regional_rebuild_versions": list(REGIONAL_REBUILD_VERSIONS),
                "version_match_counts": [
                    {"version": version, "matches": counts[version]}
                    for version in SIGNATURE_VERSIONS
                ],
                "disagreement_count": len(disagreements),
                "disagreements": disagreements,
            }
        )
    return evidence

def write_index(root: Path) -> dict[str, Any]:
    payload = scan(root)
    # The generated catalog is the durable lookup for official Psy-Q macros,
    # types, variables, and prototypes. It never supplies local-game meanings.
    index_headers(root, "4.7")
    _write_json(signature_index_path(root), payload)
    return payload

__all__ = [
    "SIGNATURE_VERSIONS",
    "scan",
    "write_index",
]
