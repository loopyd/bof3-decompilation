from __future__ import annotations

from collections import defaultdict
from typing import Any


def candidate_sort_key(candidate: dict[str, Any]) -> tuple[str, str, int, str]:
    return (
        str(candidate.get("family") or ""),
        str(candidate.get("archive_id") or ""),
        int(candidate.get("entry_index") or 0),
        str(candidate.get("candidate_name") or ""),
    )


def member_record(candidate: dict[str, Any]) -> dict[str, Any]:
    return {
        "archive_id": candidate["archive_id"],
        "candidate_name": candidate["candidate_name"],
        "entry_index": candidate["entry_index"],
        "family": candidate["family"],
        "payload_path": candidate["payload_path"],
        "program_id": candidate["program_id"],
        "ram_ptr_hex": candidate["ram_ptr_hex"],
        "sha256": candidate["sha256"],
        "size": candidate["size"],
    }


def build_unique_overlay_map(
    candidates_catalog: dict[str, Any],
    clusters_catalog: dict[str, Any],
) -> dict[str, Any]:
    candidates = list(candidates_catalog["candidates"])
    by_sha: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for candidate in candidates:
        by_sha[str(candidate["sha256"])].append(candidate)

    representatives: list[dict[str, Any]] = []
    alias_to_representative: dict[str, str] = {}
    seen_sha: set[str] = set()

    for group in clusters_catalog["exact_duplicate_groups"]:
        sha256 = str(group["sha256"])
        members = sorted(by_sha[sha256], key=candidate_sort_key)
        representative = members[0]
        seen_sha.add(sha256)
        representatives.append(
            {
                "representative_name": representative["candidate_name"],
                "representative": member_record(representative),
                "sha256": sha256,
                "group_size": len(members),
                "families": sorted(
                    {str(member.get("family") or "unknown") for member in members}
                ),
                "load_addresses": sorted(
                    {str(member["ram_ptr_hex"]) for member in members}
                ),
                "members": [member_record(member) for member in members],
            }
        )
        for member in members:
            alias_to_representative[str(member["candidate_name"])] = representative[
                "candidate_name"
            ]

    for sha256, members in by_sha.items():
        if sha256 in seen_sha:
            continue
        ordered_members = sorted(members, key=candidate_sort_key)
        representative = ordered_members[0]
        representatives.append(
            {
                "representative_name": representative["candidate_name"],
                "representative": member_record(representative),
                "sha256": sha256,
                "group_size": 1,
                "families": [str(representative.get("family") or "unknown")],
                "load_addresses": [str(representative["ram_ptr_hex"])],
                "members": [member_record(representative)],
            }
        )
        alias_to_representative[representative["candidate_name"]] = representative[
            "candidate_name"
        ]

    representatives.sort(
        key=lambda group: (
            -group["group_size"],
            str(group["representative"].get("family") or ""),
            str(group["representative"].get("archive_id") or ""),
            int(group["representative"].get("entry_index") or 0),
        )
    )

    return {
        "schema": "harness.inventory-unique-overlay-map/v1",
        "candidate_count": len(candidates),
        "representative_count": len(representatives),
        "exact_duplicate_representative_count": sum(
            1 for group in representatives if group["group_size"] > 1
        ),
        "singleton_representative_count": sum(
            1 for group in representatives if group["group_size"] == 1
        ),
        "representatives": representatives,
        "alias_to_representative": alias_to_representative,
        "unmapped_candidate_count": sum(
            1
            for candidate in candidates
            if candidate["candidate_name"] not in alias_to_representative
        ),
    }


def render_unique_overlay_map_markdown(unique_map: dict[str, Any]) -> str:
    lines = [
        "# Unique Overlay Map",
        "",
        "Machine-generated mapping from overlay candidates to representative payloads.",
        "",
        f"- Candidate count: {unique_map['candidate_count']}",
        f"- Representative count: {unique_map['representative_count']}",
        "",
        "## Largest Representative Groups",
        "",
    ]
    for group in unique_map["representatives"][:30]:
        lines.append(
            f"- `{group['representative_name']}`: {group['group_size']} members"
        )
    return "\n".join(lines) + "\n"
