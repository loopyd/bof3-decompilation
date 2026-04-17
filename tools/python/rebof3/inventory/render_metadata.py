from __future__ import annotations

from collections import defaultdict
from typing import Any


def build_render_metadata(emi_catalog: dict[str, Any]) -> dict[str, Any]:
    archives: dict[str, dict[str, Any]] = {}
    family_entries: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for entry in emi_catalog["entries"]:
        archive_id = str(entry["archive_id"])
        family = str(entry.get("family") or "unknown")
        family_entries[family].append(entry)
        archive = archives.setdefault(
            archive_id,
            {
                "archive_id": archive_id,
                "archive_name": entry["archive_name"],
                "family": family,
                "entry_count": 0,
                "code_candidate_count": 0,
                "image_candidate_count": 0,
                "palette_candidate_count": 0,
                "audio_bundle_ids": [],
            },
        )
        archive["entry_count"] += 1
        archive["code_candidate_count"] += 1 if entry["code_candidate"] else 0
        archive["image_candidate_count"] += 1 if entry["image_candidate"] else 0
        archive["palette_candidate_count"] += 1 if entry["palette_candidate"] else 0
        audio_bundle_id = entry.get("audio_bundle_id")
        if (
            audio_bundle_id is not None
            and audio_bundle_id not in archive["audio_bundle_ids"]
        ):
            archive["audio_bundle_ids"].append(audio_bundle_id)

    families = []
    for family, entries in sorted(family_entries.items()):
        families.append(
            {
                "family": family,
                "archive_count": len({str(entry["archive_id"]) for entry in entries}),
                "entry_count": len(entries),
                "code_candidate_count": sum(
                    1 for entry in entries if entry["code_candidate"]
                ),
                "image_candidate_count": sum(
                    1 for entry in entries if entry["image_candidate"]
                ),
                "palette_candidate_count": sum(
                    1 for entry in entries if entry["palette_candidate"]
                ),
                "audio_bundle_ids": sorted(
                    {
                        int(entry["audio_bundle_id"])
                        for entry in entries
                        if entry.get("audio_bundle_id") is not None
                    }
                ),
            }
        )

    return {
        "schema": "rebof3-simple.inventory-render-metadata/v1",
        "generated_from": emi_catalog.get("generated_from"),
        "family_count": len(families),
        "archive_count": len(archives),
        "families": families,
        "archives": list(
            sorted(archives.values(), key=lambda item: item["archive_id"])
        ),
    }


def render_render_metadata_markdown(metadata: dict[str, Any]) -> str:
    lines = [
        "# Render Metadata",
        "",
        "Machine-generated family and archive summaries derived from the EMI catalog.",
        "",
        f"- Family count: {metadata['family_count']}",
        f"- Archive count: {metadata['archive_count']}",
        "",
        "## Families",
        "",
    ]
    for family in metadata["families"]:
        lines.append(
            f"- `{family['family']}`: archives={family['archive_count']} entries={family['entry_count']}"
        )
    return "\n".join(lines) + "\n"
