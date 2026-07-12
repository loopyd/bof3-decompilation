from __future__ import annotations

import json
import shutil
from collections import defaultdict
from pathlib import Path
from typing import Any

from .archive_extract import extract_archive


def read_emi_catalog(catalog_path: Path) -> dict[str, Any]:
    return json.loads(catalog_path.read_text(encoding="utf-8"))


def build_archive_index(catalog: dict[str, Any]) -> list[dict[str, Any]]:
    archive_index: dict[str, dict[str, Any]] = {}
    for entry in catalog.get("entries", []):
        archive_id = str(entry["archive_id"])
        archive_record = archive_index.setdefault(
            archive_id,
            {
                "archive_id": archive_id,
                "archive_name": entry["archive_name"],
                "family": entry["family"],
                "manifest_path": entry["manifest_path"],
                "entry_count": 0,
                "image_candidate_count": 0,
                "palette_candidate_count": 0,
                "code_candidate_count": 0,
            },
        )
        archive_record["entry_count"] += 1
        archive_record["image_candidate_count"] += 1 if entry["image_candidate"] else 0
        archive_record["palette_candidate_count"] += (
            1 if entry["palette_candidate"] else 0
        )
        archive_record["code_candidate_count"] += 1 if entry["code_candidate"] else 0
    return sorted(archive_index.values(), key=lambda item: item["archive_id"])


def select_archives(
    archives: list[dict[str, Any]],
    *,
    families: list[str] | None,
    archive_substrings: list[str] | None,
) -> list[dict[str, Any]]:
    selected = archives
    if families:
        family_names = set(families)
        selected = [
            archive for archive in selected if archive["family"] in family_names
        ]
    if archive_substrings:
        selected = [
            archive
            for archive in selected
            if any(token in archive["archive_id"] for token in archive_substrings)
        ]
    return selected


def summarize_families(selected_archives: list[dict[str, Any]]) -> list[dict[str, Any]]:
    family_summary: dict[str, dict[str, int | str]] = defaultdict(
        lambda: {
            "family": "",
            "archive_count": 0,
            "entry_count": 0,
            "image_candidate_count": 0,
            "palette_candidate_count": 0,
            "code_candidate_count": 0,
        }
    )
    for archive in selected_archives:
        summary = family_summary[archive["family"]]
        summary["family"] = archive["family"]
        summary["archive_count"] += 1
        summary["entry_count"] += archive["entry_count"]
        summary["image_candidate_count"] += archive["image_candidate_count"]
        summary["palette_candidate_count"] += archive["palette_candidate_count"]
        summary["code_candidate_count"] += archive["code_candidate_count"]
    return [family_summary[name] for name in sorted(family_summary)]


def build_review_packet(
    *,
    catalog_path: Path,
    output_root: Path,
    families: list[str] | None,
    archive_substrings: list[str] | None,
    clean: bool,
    emit_indices: bool,
) -> dict[str, Any]:
    catalog = read_emi_catalog(catalog_path)
    all_archives = build_archive_index(catalog)
    selected_archives = select_archives(
        all_archives,
        families=families,
        archive_substrings=archive_substrings,
    )
    if not selected_archives:
        raise ValueError("no archives matched the requested review filters")

    if clean and output_root.exists():
        shutil.rmtree(output_root)
    output_root.mkdir(parents=True, exist_ok=True)

    extracted_outputs: list[dict[str, Any]] = []
    for archive in selected_archives:
        archive_dir = Path(archive["manifest_path"]).parent
        archive_output_dir = output_root / "assets" / archive["archive_id"]
        written = extract_archive(
            archive_dir,
            archive_output_dir,
            emit_indices=emit_indices,
            emit_palette_previews=True,
        )
        extracted_outputs.append(
            {
                "archive_id": archive["archive_id"],
                "output_dir": str(archive_output_dir),
                "written_count": len(written),
            }
        )

    manifest = {
        "schema": "harness.emi-review/v1",
        "catalog_path": str(catalog_path),
        "filters": {
            "families": families or [],
            "archive_substrings": archive_substrings or [],
        },
        "selected_archive_count": len(selected_archives),
        "families": summarize_families(selected_archives),
        "archives": selected_archives,
        "outputs": extracted_outputs,
    }
    manifest_path = output_root / "review_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return {"manifest_path": manifest_path, "selected_archives": selected_archives}
