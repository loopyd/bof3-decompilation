from __future__ import annotations

from typing import Any


def make_entry_point_label(
    row: dict[str, Any],
    *,
    table_index: int,
    address_text: str,
) -> dict[str, str]:
    candidate_name = str(row.get("candidate_name") or "overlay")
    return {
        "address": address_text,
        "name": f"{candidate_name}__tbl_{table_index:03d}",
        "comment": (
            f"Source: {row.get('ram_ptr_hex')} {candidate_name} entry-table idx={table_index}"
        ),
    }


def build_project_plan(
    candidates_catalog: dict[str, Any],
    entry_tables_catalog: dict[str, Any],
    representatives_catalog: dict[str, Any],
) -> dict[str, Any]:
    candidates = list(candidates_catalog.get("candidates", []))
    entry_rows = list(entry_tables_catalog.get("candidates", []))
    alias_map = dict(representatives_catalog.get("alias_to_representative", {}))

    candidate_by_program_id = {
        str(candidate["program_id"]): candidate for candidate in candidates
    }
    imports: list[dict[str, Any]] = []
    representative_imports: list[dict[str, Any]] = []
    entry_point_labels: list[dict[str, Any]] = []
    seen_representatives: set[str] = set()

    for row in sorted(
        entry_rows,
        key=lambda item: (
            str(item.get("family") or ""),
            str(item.get("archive_id") or ""),
            int(item.get("entry_index") or 0),
        ),
    ):
        candidate = candidate_by_program_id[str(row["program_id"])]
        candidate_name = str(candidate["candidate_name"])
        representative_name = str(alias_map.get(candidate_name) or candidate_name)
        import_row = {
            "archive_id": row["archive_id"],
            "candidate_name": candidate_name,
            "entry_index": row["entry_index"],
            "family": candidate["family"],
            "payload_path": row["payload_path"],
            "program_id": row["program_id"],
            "project_folder_path": candidate["project_folder_path"],
            "ram_ptr_hex": row["ram_ptr_hex"],
            "representative_name": representative_name,
            "entry_table": {
                "confidence": row["confidence"],
                "entry_addresses": row["entry_addresses"],
                "entry_count": row["entry_count"],
                "entry_in_range_count": row["entry_in_range_count"],
            },
        }
        imports.append(import_row)

        if representative_name not in seen_representatives:
            representative_imports.append(
                {
                    "archive_id": row["archive_id"],
                    "candidate_name": candidate_name,
                    "entry_index": row["entry_index"],
                    "ram_ptr_hex": row["ram_ptr_hex"],
                    "representative_name": representative_name,
                }
            )
            seen_representatives.add(representative_name)

        for table_index, address_text in enumerate(row["entry_addresses"]):
            entry_point_labels.append(
                {
                    "archive_id": row["archive_id"],
                    "candidate_name": candidate_name,
                    "confidence": row["confidence"],
                    "entry_index": row["entry_index"],
                    "label": make_entry_point_label(
                        row,
                        table_index=table_index,
                        address_text=str(address_text),
                    ),
                    "representative_name": representative_name,
                    "table_index": table_index,
                }
            )

    families = sorted(
        {
            str(import_row["family"])
            for import_row in imports
            if import_row.get("family")
        }
    )
    labels_by_confidence = {
        "high": sum(1 for row in entry_point_labels if row["confidence"] == "high"),
        "medium": sum(1 for row in entry_point_labels if row["confidence"] == "medium"),
        "low": sum(1 for row in entry_point_labels if row["confidence"] == "low"),
    }

    presets: dict[str, dict[str, Any]] = {
        "broad": {
            "mode": "all_function_candidates",
            "candidate_count": len(imports),
        },
        "minimal": {
            "mode": "representative_function_candidates",
            "candidate_count": len(representative_imports),
            "candidate_names": [
                row["representative_name"] for row in representative_imports
            ],
        },
    }
    for family in families:
        family_imports = [row for row in imports if row.get("family") == family]
        presets[f"focused:{family.lower()}"] = {
            "mode": "family_function_candidates",
            "family": family,
            "candidate_count": len(family_imports),
            "archive_substr": [f"/bins/{family}/"],
        }

    return {
        "schema": "rebof3-simple.inventory-project-plan/v1",
        "function_candidate_count": len(imports),
        "representative_function_candidate_count": len(representative_imports),
        "entry_point_label_count": len(entry_point_labels),
        "entry_point_labels_by_confidence": labels_by_confidence,
        "families": families,
        "presets": presets,
        "imports": imports,
        "representative_imports": representative_imports,
        "entry_point_labels": entry_point_labels,
    }


def render_project_plan_markdown(plan: dict[str, Any]) -> str:
    lines = [
        "# Inventory Project Plan",
        "",
        "Machine-generated import presets and label plan for overlay entry-table candidates.",
        "",
        f"- Function candidates: {plan['function_candidate_count']}",
        f"- Representative function candidates: {plan['representative_function_candidate_count']}",
        f"- Entry-point labels: {plan['entry_point_label_count']}",
        "",
        "## Presets",
        "",
    ]
    for name, preset in sorted(plan["presets"].items()):
        lines.append(
            f"- `{name}`: mode `{preset['mode']}`, candidates `{preset['candidate_count']}`"
        )
    return "\n".join(lines) + "\n"
