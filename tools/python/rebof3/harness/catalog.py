from __future__ import annotations

from pathlib import Path
from typing import Any

from ..inventory.emi_catalog import build_emi_manifest_catalog
from ..jsonio import read_json, write_json
from .classify import classify_emi_entry
from .config import HarnessConfig


def load_or_build_emi_catalog(config: HarnessConfig) -> dict[str, Any]:
    if config.emi_catalog.is_file():
        return read_json(config.emi_catalog)
    if not config.emi_root.is_dir():
        raise FileNotFoundError(
            f"missing EMI catalog and EMI root: {config.emi_catalog}, {config.emi_root}"
        )
    return build_emi_manifest_catalog(config.emi_root)


def symbolic_emi_catalog(catalog: dict[str, Any]) -> dict[str, Any]:
    entries: list[dict[str, Any]] = []
    kind_counts: dict[str, int] = {}
    for raw_entry in catalog.get("entries", []):
        if not isinstance(raw_entry, dict):
            continue
        entry = dict(raw_entry)
        raw_type = int(entry.pop("type", entry.get("raw_type", 0)) or 0)
        entry["raw_type"] = raw_type
        classification = classify_emi_entry(entry)
        entry["emi_kind"] = classification.kind
        entry["classification"] = classification.to_dict()
        kind_counts[classification.kind] = kind_counts.get(classification.kind, 0) + 1
        entries.append(entry)
    return {
        "schema": "rebof3-simple.harness-catalog/v1",
        "archive_count": int(catalog.get("archive_count") or 0),
        "entry_count": len(entries),
        "emi_kind_counts": dict(sorted(kind_counts.items())),
        "entries": sorted(
            entries,
            key=lambda item: (
                str(item.get("archive_id") or ""),
                int(item.get("entry_index") or 0),
            ),
        ),
    }


def artifact_records(config: HarnessConfig) -> list[dict[str, Any]]:
    if not config.artifact_manifest.is_file():
        return []
    payload = read_json(config.artifact_manifest)
    artifacts = payload.get("artifacts", [])
    records: list[dict[str, Any]] = []
    if not isinstance(artifacts, list):
        return records
    for artifact in artifacts:
        if not isinstance(artifact, dict):
            continue
        target = str(artifact.get("target") or "")
        if not target:
            continue
        placeholder = bool(artifact.get("placeholder"))
        priority = 40 if placeholder else 80
        records.append(
            {
                "id": f"artifact:{target}",
                "type": "artifact",
                "status": "queued" if placeholder else "ready",
                "priority": priority,
                "summary": f"{target} ({artifact.get('build_stage') or '?'})",
                "source_hint": artifact.get("source_hint"),
                "program_path": artifact.get("program_path"),
                "payload": dict(artifact),
            }
        )
    return records


def emi_target_records(catalog: dict[str, Any]) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for entry in catalog.get("entries", []):
        if not isinstance(entry, dict):
            continue
        classification = entry.get("classification")
        if not isinstance(classification, dict):
            classification = classify_emi_entry(entry).to_dict()
        archive_id = str(entry.get("archive_id") or "")
        entry_index = int(entry.get("entry_index") or 0)
        target_id = f"emi:{archive_id}#{entry_index}"
        score = int(classification.get("score") or 0)
        status = "queued" if score >= 50 else "cataloged"
        source_hint = f"out/extracted/BIN/{archive_id}.EMI#{entry_index}"
        records.append(
            {
                "id": target_id,
                "type": "emi",
                "status": status,
                "priority": max(1, 100 - score),
                "summary": (
                    f"{archive_id}#{entry_index} {entry.get('emi_kind')} "
                    f"{entry.get('ram_ptr_hex')} size {entry.get('size')}"
                ),
                "source_hint": source_hint,
                "payload": dict(entry),
            }
        )
    return records


def write_harness_catalog(config: HarnessConfig) -> tuple[dict[str, Any], Path]:
    source_catalog = load_or_build_emi_catalog(config)
    catalog = symbolic_emi_catalog(source_catalog)
    output_path = config.out_dir / "catalog.json"
    write_json(output_path, catalog)
    return catalog, output_path
