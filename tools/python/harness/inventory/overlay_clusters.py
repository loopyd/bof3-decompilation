from __future__ import annotations

from collections import defaultdict
from typing import Any


def build_overlay_clusters(catalog: dict[str, Any]) -> dict[str, Any]:
    by_hash: dict[str, list[dict[str, Any]]] = defaultdict(list)
    by_region: dict[tuple[str, int], list[dict[str, Any]]] = defaultdict(list)

    for candidate in catalog["candidates"]:
        by_hash[str(candidate["sha256"])].append(candidate)
        by_region[(str(candidate["ram_ptr_hex"]), int(candidate["size"]))].append(
            candidate
        )

    exact_groups: list[dict[str, Any]] = []
    for sha256, members in by_hash.items():
        if len(members) < 2:
            continue
        ordered_members = sorted(
            members,
            key=lambda item: (
                str(item.get("family") or ""),
                str(item.get("archive_id") or ""),
                int(item.get("entry_index") or 0),
            ),
        )
        exact_groups.append(
            {
                "sha256": sha256,
                "group_size": len(ordered_members),
                "representative": ordered_members[0]["candidate_name"],
                "families": sorted(
                    {
                        str(member.get("family") or "unknown")
                        for member in ordered_members
                    }
                ),
                "load_addresses": sorted(
                    {str(member["ram_ptr_hex"]) for member in ordered_members}
                ),
                "members": [
                    {
                        "candidate_name": member["candidate_name"],
                        "archive_id": member["archive_id"],
                        "entry_index": member["entry_index"],
                        "payload_path": member["payload_path"],
                        "program_id": member["program_id"],
                        "ram_ptr_hex": member["ram_ptr_hex"],
                        "size": member["size"],
                    }
                    for member in ordered_members
                ],
            }
        )

    exact_groups.sort(key=lambda group: (-group["group_size"], group["representative"]))

    region_clusters: list[dict[str, Any]] = []
    for (ram_ptr_hex, size), members in by_region.items():
        if len(members) < 2:
            continue
        region_clusters.append(
            {
                "ram_ptr_hex": ram_ptr_hex,
                "size": size,
                "member_count": len(members),
                "distinct_hash_count": len(
                    {str(member["sha256"]) for member in members}
                ),
                "families": sorted(
                    {str(member.get("family") or "unknown") for member in members}
                ),
                "representative_candidates": sorted(
                    {str(member["candidate_name"]) for member in members}
                )[:16],
            }
        )

    region_clusters.sort(
        key=lambda group: (-group["member_count"], group["ram_ptr_hex"], group["size"])
    )

    return {
        "schema": "harness.inventory-overlay-clusters/v1",
        "exact_duplicate_group_count": len(exact_groups),
        "region_cluster_count": len(region_clusters),
        "exact_duplicate_groups": exact_groups,
        "region_clusters": region_clusters,
    }


def render_overlay_clusters_markdown(clusters: dict[str, Any]) -> str:
    lines = [
        "# Overlay Clusters",
        "",
        "Machine-generated duplicate and region clustering for overlay candidates.",
        "",
        f"- Exact duplicate groups: {clusters['exact_duplicate_group_count']}",
        f"- Region clusters: {clusters['region_cluster_count']}",
        "",
        "## Largest Exact Duplicate Groups",
        "",
    ]
    for group in clusters["exact_duplicate_groups"][:20]:
        lines.append(
            f"- `{group['representative']}`: {group['group_size']} members across {', '.join(group['families'])}"
        )
    lines.extend(["", "## Largest Region Clusters", ""])
    for group in clusters["region_clusters"][:20]:
        lines.append(
            f"- `{group['ram_ptr_hex']}` size `{group['size']}`: {group['member_count']} members"
        )
    return "\n".join(lines) + "\n"
