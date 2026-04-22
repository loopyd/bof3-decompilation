from __future__ import annotations

import argparse
from pathlib import Path

from ..inventory import (
    build_emi_manifest_catalog,
    build_entry_tables_catalog,
    build_inventory_artifacts,
    build_overlay_catalog,
    build_overlay_clusters,
    build_project_plan,
    build_render_metadata,
    build_slot_map_artifact,
    build_unique_overlay_map,
    group_exact_duplicates,
    import_ghidra_symbols,
    scan_inventory,
)
from ..inventory.build import write_markdown
from ..inventory.emi_catalog import render_emi_catalog_markdown
from ..inventory.entry_tables import render_entry_tables_markdown
from ..inventory.overlay_catalog import render_overlay_catalog_markdown
from ..inventory.overlay_clusters import render_overlay_clusters_markdown
from ..inventory.project_plan import render_project_plan_markdown
from ..inventory.render_metadata import render_render_metadata_markdown
from ..inventory.slot_map import SLOT_TABLE_VADDR, render_slot_map_markdown
from ..inventory.unique_overlay_map import render_unique_overlay_map_markdown
from ..jsonio import read_json, write_json
from ..models import DuplicateGroups, InventorySnapshot
from ..paths import repo_layout
from ._common import run_main


def run_scan(args: argparse.Namespace) -> int:
    snapshot = scan_inventory(
        slus_path=args.slus,
        logo_path=args.logo,
        emi_root=args.emi_root,
    )
    write_json(args.output, snapshot.to_dict())
    print(f"wrote {len(snapshot.programs)} programs to {args.output}")
    return 0


def run_group(args: argparse.Namespace) -> int:
    snapshot = InventorySnapshot.from_dict(read_json(args.input))
    groups = group_exact_duplicates(snapshot)
    write_json(args.output, groups.to_dict())
    print(f"wrote {len(groups.groups)} duplicate groups to {args.output}")
    return 0


def run_build(args: argparse.Namespace) -> int:
    layout = repo_layout(args.root)
    outputs = build_inventory_artifacts(
        slus_path=args.slus,
        logo_path=args.logo,
        emi_root=args.emi_root,
        layout=layout,
    )
    print(f"inventory: {outputs['inventory']}")
    print(f"groups: {outputs['groups']}")
    print(f"index: {outputs['index']}")
    return 0


def run_slot_map(args: argparse.Namespace) -> int:
    slot_map = build_slot_map_artifact(
        slus_path=args.slus,
        disc_lba_path=args.disc_lba,
        slot_table_address=args.slot_table_address,
        slot_count=args.slot_count,
    )
    write_json(args.json_out, slot_map)
    write_markdown(args.md_out, render_slot_map_markdown(slot_map))
    print(f"wrote slot map to {args.json_out}")
    return 0


def run_emi_catalog(args: argparse.Namespace) -> int:
    catalog = build_emi_manifest_catalog(args.emi_root)
    write_json(args.json_out, catalog)
    write_markdown(args.md_out, render_emi_catalog_markdown(catalog))
    print(f"wrote EMI catalog to {args.json_out}")
    return 0


def load_inventory_snapshot(path: Path) -> InventorySnapshot:
    return InventorySnapshot.from_dict(read_json(path))


def load_duplicate_groups(path: Path | None) -> DuplicateGroups | None:
    if path is None:
        return None
    return DuplicateGroups.from_dict(read_json(path))


def run_overlay_catalog(args: argparse.Namespace) -> int:
    catalog = build_overlay_catalog(
        load_inventory_snapshot(args.inventory),
        load_duplicate_groups(args.groups),
    )
    write_json(args.json_out, catalog)
    write_markdown(args.md_out, render_overlay_catalog_markdown(catalog))
    print(f"wrote overlay catalog to {args.json_out}")
    return 0


def run_overlay_clusters(args: argparse.Namespace) -> int:
    clusters = build_overlay_clusters(read_json(args.catalog))
    write_json(args.json_out, clusters)
    write_markdown(args.md_out, render_overlay_clusters_markdown(clusters))
    print(f"wrote overlay clusters to {args.json_out}")
    return 0


def run_unique_overlay_map(args: argparse.Namespace) -> int:
    unique_map = build_unique_overlay_map(
        read_json(args.catalog),
        read_json(args.clusters),
    )
    write_json(args.json_out, unique_map)
    write_markdown(args.md_out, render_unique_overlay_map_markdown(unique_map))
    print(f"wrote unique overlay map to {args.json_out}")
    return 0


def run_entry_tables(args: argparse.Namespace) -> int:
    catalog = build_entry_tables_catalog(read_json(args.catalog))
    write_json(args.json_out, catalog)
    write_markdown(args.md_out, render_entry_tables_markdown(catalog))
    print(f"wrote entry tables to {args.json_out}")
    return 0


def run_project_plan(args: argparse.Namespace) -> int:
    plan = build_project_plan(
        read_json(args.catalog),
        read_json(args.entry_tables),
        read_json(args.unique_overlay_map),
    )
    write_json(args.json_out, plan)
    write_markdown(args.md_out, render_project_plan_markdown(plan))
    print(f"wrote project plan to {args.json_out}")
    return 0


def run_render_metadata(args: argparse.Namespace) -> int:
    metadata = build_render_metadata(read_json(args.emi_catalog))
    write_json(args.json_out, metadata)
    write_markdown(args.md_out, render_render_metadata_markdown(metadata))
    print(f"wrote render metadata to {args.json_out}")
    return 0


def run_import_ghidra_symbols(args: argparse.Namespace) -> int:
    result = import_ghidra_symbols(
        input_path=args.input,
        index_out=args.index_out,
        function_index_out=args.function_index_out,
        function_index_tsv_out=args.function_index_tsv_out,
        md_out=args.md_out,
        program_output_dir=args.program_output_dir,
    )
    print(
        f"wrote {result['program_count']} programs and {result['function_count']} functions"
    )
    return 0


def configure_scan_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--slus", type=Path, default=layout.slus_path)
    parser.add_argument("--logo", type=Path, default=layout.logo_path)
    parser.add_argument("--emi-root", type=Path, default=layout.emi_root)
    parser.add_argument("--output", type=Path, default=layout.inventory_path)
    parser.set_defaults(handler=run_scan)


def configure_group_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--input", type=Path, default=layout.inventory_path)
    parser.add_argument("--output", type=Path, default=layout.groups_path)
    parser.set_defaults(handler=run_group)


def configure_build_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--root", type=Path, default=layout.root)
    parser.add_argument("--slus", type=Path, default=layout.slus_path)
    parser.add_argument("--logo", type=Path, default=layout.logo_path)
    parser.add_argument("--emi-root", type=Path, default=layout.emi_root)
    parser.set_defaults(handler=run_build)


def configure_slot_map_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--slus", type=Path, default=layout.slus_path)
    parser.add_argument("--disc-lba", type=Path, default=layout.inventory_disc_lba_path)
    parser.add_argument("--json-out", type=Path, default=layout.inventory_slot_map_path)
    parser.add_argument(
        "--md-out", type=Path, default=layout.inventory_slot_map_md_path
    )
    parser.add_argument(
        "--slot-table-address",
        type=lambda text: int(text, 0),
        default=SLOT_TABLE_VADDR,
    )
    parser.add_argument("--slot-count", type=int, default=0)
    parser.set_defaults(handler=run_slot_map)


def configure_emi_catalog_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--emi-root", type=Path, default=layout.emi_root)
    parser.add_argument(
        "--json-out", type=Path, default=layout.inventory_emi_catalog_path
    )
    parser.add_argument(
        "--md-out", type=Path, default=layout.inventory_emi_catalog_md_path
    )
    parser.set_defaults(handler=run_emi_catalog)


def configure_overlay_catalog_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--inventory", type=Path, default=layout.inventory_path)
    parser.add_argument("--groups", type=Path, default=layout.groups_path)
    parser.add_argument(
        "--json-out",
        type=Path,
        default=layout.inventory_overlay_catalog_path,
    )
    parser.add_argument(
        "--md-out",
        type=Path,
        default=layout.inventory_overlay_catalog_md_path,
    )
    parser.set_defaults(handler=run_overlay_catalog)


def configure_overlay_clusters_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument(
        "--catalog", type=Path, default=layout.inventory_overlay_catalog_path
    )
    parser.add_argument(
        "--json-out",
        type=Path,
        default=layout.inventory_overlay_clusters_path,
    )
    parser.add_argument(
        "--md-out",
        type=Path,
        default=layout.inventory_overlay_clusters_md_path,
    )
    parser.set_defaults(handler=run_overlay_clusters)


def configure_unique_overlay_map_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument(
        "--catalog", type=Path, default=layout.inventory_overlay_catalog_path
    )
    parser.add_argument(
        "--clusters",
        type=Path,
        default=layout.inventory_overlay_clusters_path,
    )
    parser.add_argument(
        "--json-out",
        type=Path,
        default=layout.inventory_unique_overlay_map_path,
    )
    parser.add_argument(
        "--md-out",
        type=Path,
        default=layout.inventory_unique_overlay_map_md_path,
    )
    parser.set_defaults(handler=run_unique_overlay_map)


def configure_entry_tables_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument(
        "--catalog", type=Path, default=layout.inventory_overlay_catalog_path
    )
    parser.add_argument(
        "--json-out",
        type=Path,
        default=layout.inventory_entry_tables_path,
    )
    parser.add_argument(
        "--md-out",
        type=Path,
        default=layout.inventory_entry_tables_md_path,
    )
    parser.set_defaults(handler=run_entry_tables)


def configure_project_plan_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument(
        "--catalog", type=Path, default=layout.inventory_overlay_catalog_path
    )
    parser.add_argument(
        "--entry-tables",
        type=Path,
        default=layout.inventory_entry_tables_path,
    )
    parser.add_argument(
        "--unique-overlay-map",
        type=Path,
        default=layout.inventory_unique_overlay_map_path,
    )
    parser.add_argument(
        "--json-out",
        type=Path,
        default=layout.inventory_project_plan_path,
    )
    parser.add_argument(
        "--md-out",
        type=Path,
        default=layout.inventory_project_plan_md_path,
    )
    parser.set_defaults(handler=run_project_plan)


def configure_render_metadata_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument(
        "--emi-catalog",
        type=Path,
        default=layout.inventory_emi_catalog_path,
    )
    parser.add_argument(
        "--json-out",
        type=Path,
        default=layout.inventory_render_metadata_path,
    )
    parser.add_argument(
        "--md-out",
        type=Path,
        default=layout.inventory_render_metadata_md_path,
    )
    parser.set_defaults(handler=run_render_metadata)


def configure_import_ghidra_symbols_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument(
        "input",
        nargs="?",
        type=Path,
        default=layout.inventory_artifacts_dir / "raw_ghidra_export.json",
    )
    parser.add_argument(
        "--index-out",
        type=Path,
        default=layout.inventory_ghidra_symbols_index_path,
    )
    parser.add_argument(
        "--function-index-out",
        type=Path,
        default=layout.inventory_ghidra_function_index_path,
    )
    parser.add_argument(
        "--function-index-tsv-out",
        type=Path,
        default=layout.inventory_ghidra_function_index_tsv_path,
    )
    parser.add_argument(
        "--md-out",
        type=Path,
        default=layout.inventory_ghidra_symbols_md_path,
    )
    parser.add_argument(
        "--program-output-dir",
        type=Path,
        default=layout.inventory_ghidra_symbols_program_dir,
    )
    parser.set_defaults(handler=run_import_ghidra_symbols)


def configure_root_parser(parser: argparse.ArgumentParser) -> None:
    subparsers = parser.add_subparsers(required=True)

    scan = subparsers.add_parser("scan")
    configure_scan_parser(scan)

    group = subparsers.add_parser("group")
    configure_group_parser(group)

    build = subparsers.add_parser("build")
    configure_build_parser(build)

    slot_map = subparsers.add_parser("slot-map")
    configure_slot_map_parser(slot_map)

    emi_catalog = subparsers.add_parser("emi-catalog")
    configure_emi_catalog_parser(emi_catalog)

    overlay_catalog = subparsers.add_parser("overlay-catalog")
    configure_overlay_catalog_parser(overlay_catalog)

    overlay_clusters = subparsers.add_parser("overlay-clusters")
    configure_overlay_clusters_parser(overlay_clusters)

    unique_overlay_map = subparsers.add_parser("unique-overlay-map")
    configure_unique_overlay_map_parser(unique_overlay_map)

    entry_tables = subparsers.add_parser("entry-tables")
    configure_entry_tables_parser(entry_tables)

    project_plan = subparsers.add_parser("project-plan")
    configure_project_plan_parser(project_plan)

    render_metadata = subparsers.add_parser("render-metadata")
    configure_render_metadata_parser(render_metadata)

    ghidra_symbols = subparsers.add_parser("import-ghidra-symbols")
    configure_import_ghidra_symbols_parser(ghidra_symbols)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="inventory")
    configure_root_parser(parser)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
