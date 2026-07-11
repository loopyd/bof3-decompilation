from __future__ import annotations

from pathlib import Path

from ..inventory import group_exact_duplicates, scan_inventory
from ..jsonio import write_json
from ..planning import build_ghidra_manifest


def bootstrap_ghidra(
    *,
    slus_path: Path | None,
    logo_path: Path | None,
    emi_root: Path | None,
    output_dir: Path,
    analyze: bool = True,
) -> dict[str, Path]:
    """Generate the inventory and import manifest consumed by Ghidra."""
    inventory_path = output_dir / "inventory.json"
    groups_path = output_dir / "groups.json"
    manifest_path = output_dir / "ghidra_import_manifest.json"

    snapshot = scan_inventory(
        slus_path=slus_path,
        logo_path=logo_path,
        emi_root=emi_root,
    )
    groups = group_exact_duplicates(snapshot)
    manifest = build_ghidra_manifest(snapshot, groups, analyze=analyze)

    write_json(inventory_path, snapshot.to_dict())
    write_json(groups_path, groups.to_dict())
    write_json(manifest_path, manifest.to_dict())
    return {
        "groups": groups_path,
        "inventory": inventory_path,
        "manifest": manifest_path,
    }
