from __future__ import annotations

from pathlib import Path

from ..jsonio import write_json
from ..models import DuplicateGroups, InventorySnapshot
from ..paths import RepoLayout
from .emi_catalog import build_emi_manifest_catalog, render_emi_catalog_markdown
from .entry_tables import build_entry_tables_catalog, render_entry_tables_markdown
from .group import group_exact_duplicates
from .overlay_catalog import build_overlay_catalog, render_overlay_catalog_markdown
from .overlay_clusters import build_overlay_clusters, render_overlay_clusters_markdown
from .project_plan import build_project_plan, render_project_plan_markdown
from .render_metadata import build_render_metadata, render_render_metadata_markdown
from .scan import scan_inventory
from .unique_overlay_map import (
    build_unique_overlay_map,
    render_unique_overlay_map_markdown,
)


def write_markdown(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def artifact_record(name: str, path: Path) -> dict[str, str]:
    return {"name": name, "path": str(path)}


def write_inventory_artifacts(
    *,
    snapshot: InventorySnapshot,
    groups: DuplicateGroups,
    emi_catalog: dict,
    overlay_catalog: dict,
    overlay_clusters: dict,
    unique_overlay_map: dict,
    entry_tables: dict,
    project_plan: dict,
    render_metadata: dict,
    layout: RepoLayout,
) -> dict[str, Path]:
    write_json(layout.inventory_path, snapshot.to_dict())
    write_json(layout.groups_path, groups.to_dict())

    write_json(layout.inventory_emi_catalog_path, emi_catalog)
    write_markdown(
        layout.inventory_emi_catalog_md_path, render_emi_catalog_markdown(emi_catalog)
    )

    write_json(layout.inventory_overlay_catalog_path, overlay_catalog)
    write_markdown(
        layout.inventory_overlay_catalog_md_path,
        render_overlay_catalog_markdown(overlay_catalog),
    )

    write_json(layout.inventory_overlay_clusters_path, overlay_clusters)
    write_markdown(
        layout.inventory_overlay_clusters_md_path,
        render_overlay_clusters_markdown(overlay_clusters),
    )

    write_json(layout.inventory_unique_overlay_map_path, unique_overlay_map)
    write_markdown(
        layout.inventory_unique_overlay_map_md_path,
        render_unique_overlay_map_markdown(unique_overlay_map),
    )

    write_json(layout.inventory_entry_tables_path, entry_tables)
    write_markdown(
        layout.inventory_entry_tables_md_path,
        render_entry_tables_markdown(entry_tables),
    )

    write_json(layout.inventory_project_plan_path, project_plan)
    write_markdown(
        layout.inventory_project_plan_md_path,
        render_project_plan_markdown(project_plan),
    )

    write_json(layout.inventory_render_metadata_path, render_metadata)
    write_markdown(
        layout.inventory_render_metadata_md_path,
        render_render_metadata_markdown(render_metadata),
    )

    artifact_index = {
        "schema": "rebof3-simple.inventory-artifacts/v1",
        "artifacts": [
            artifact_record("inventory", layout.inventory_path),
            artifact_record("groups", layout.groups_path),
            artifact_record("emi_catalog_json", layout.inventory_emi_catalog_path),
            artifact_record("emi_catalog_md", layout.inventory_emi_catalog_md_path),
            artifact_record(
                "overlay_catalog_json", layout.inventory_overlay_catalog_path
            ),
            artifact_record(
                "overlay_catalog_md", layout.inventory_overlay_catalog_md_path
            ),
            artifact_record(
                "overlay_clusters_json", layout.inventory_overlay_clusters_path
            ),
            artifact_record(
                "overlay_clusters_md", layout.inventory_overlay_clusters_md_path
            ),
            artifact_record(
                "unique_overlay_map_json", layout.inventory_unique_overlay_map_path
            ),
            artifact_record(
                "unique_overlay_map_md", layout.inventory_unique_overlay_map_md_path
            ),
            artifact_record("entry_tables_json", layout.inventory_entry_tables_path),
            artifact_record("entry_tables_md", layout.inventory_entry_tables_md_path),
            artifact_record("project_plan_json", layout.inventory_project_plan_path),
            artifact_record("project_plan_md", layout.inventory_project_plan_md_path),
            artifact_record(
                "render_metadata_json", layout.inventory_render_metadata_path
            ),
            artifact_record(
                "render_metadata_md", layout.inventory_render_metadata_md_path
            ),
        ],
    }
    write_json(layout.inventory_artifact_index_path, artifact_index)

    return {
        "entry_tables": layout.inventory_entry_tables_path,
        "emi_catalog": layout.inventory_emi_catalog_path,
        "groups": layout.groups_path,
        "index": layout.inventory_artifact_index_path,
        "inventory": layout.inventory_path,
        "overlay_catalog": layout.inventory_overlay_catalog_path,
        "overlay_clusters": layout.inventory_overlay_clusters_path,
        "project_plan": layout.inventory_project_plan_path,
        "render_metadata": layout.inventory_render_metadata_path,
        "unique_overlay_map": layout.inventory_unique_overlay_map_path,
    }


def build_inventory_artifacts(
    *,
    slus_path: Path | None,
    logo_path: Path | None,
    emi_root: Path | None,
    layout: RepoLayout,
) -> dict[str, Path]:
    if emi_root is None:
        raise ValueError("inventory build requires --emi-root")

    snapshot = scan_inventory(
        slus_path=slus_path,
        logo_path=logo_path,
        emi_root=emi_root,
    )
    groups = group_exact_duplicates(snapshot)
    emi_catalog = build_emi_manifest_catalog(emi_root)
    overlay_catalog = build_overlay_catalog(snapshot, groups)
    overlay_clusters = build_overlay_clusters(overlay_catalog)
    unique_overlay_map = build_unique_overlay_map(overlay_catalog, overlay_clusters)
    entry_tables = build_entry_tables_catalog(overlay_catalog)
    project_plan = build_project_plan(overlay_catalog, entry_tables, unique_overlay_map)
    render_metadata = build_render_metadata(emi_catalog)
    return write_inventory_artifacts(
        snapshot=snapshot,
        groups=groups,
        emi_catalog=emi_catalog,
        overlay_catalog=overlay_catalog,
        overlay_clusters=overlay_clusters,
        unique_overlay_map=unique_overlay_map,
        entry_tables=entry_tables,
        project_plan=project_plan,
        render_metadata=render_metadata,
        layout=layout,
    )
