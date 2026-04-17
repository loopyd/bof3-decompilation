from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class RepoLayout:
    root: Path
    build_dir: Path
    out_dir: Path
    toolchains_dir: Path
    third_party_dir: Path
    inputs_dir: Path
    docs_dir: Path
    downloads_dir: Path
    bof3_dir: Path
    disc_dir: Path
    external_dir: Path
    private_assets_dir: Path
    slus_path: Path
    logo_path: Path
    emi_root: Path
    extracted_dir: Path
    rebuilt_dir: Path
    raw_emi_dir: Path
    ghidra_bootstrap_dir: Path
    bof3_disk_src: Path
    emi_ex_src: Path
    objdiff_src: Path
    mipsmatch_src: Path
    bof3_disk_bin: Path
    emi_ex_bin: Path
    objdiff_bin: Path
    mipsmatch_bin: Path
    psn00b_toolchain_root: Path
    psn00b_sdk_root: Path
    gcc272_psx_root: Path
    psyq_root: Path
    inventory_path: Path
    groups_path: Path
    ghidra_manifest_path: Path
    inventory_artifacts_dir: Path
    inventory_artifact_index_path: Path
    inventory_disc_lba_path: Path
    inventory_slot_map_path: Path
    inventory_slot_map_md_path: Path
    inventory_emi_catalog_path: Path
    inventory_emi_catalog_md_path: Path
    inventory_overlay_catalog_path: Path
    inventory_overlay_catalog_md_path: Path
    inventory_overlay_clusters_path: Path
    inventory_overlay_clusters_md_path: Path
    inventory_unique_overlay_map_path: Path
    inventory_unique_overlay_map_md_path: Path
    inventory_entry_tables_path: Path
    inventory_entry_tables_md_path: Path
    inventory_project_plan_path: Path
    inventory_project_plan_md_path: Path
    inventory_render_metadata_path: Path
    inventory_render_metadata_md_path: Path
    inventory_ghidra_symbols_index_path: Path
    inventory_ghidra_function_index_path: Path
    inventory_ghidra_function_index_tsv_path: Path
    inventory_ghidra_symbols_md_path: Path
    inventory_ghidra_symbols_program_dir: Path
    disk_checksums_path: Path
    aspsx_psyq_root: Path
    aspsx_psyq_compat_root: Path


def repo_layout(root: Path | None = None) -> RepoLayout:
    resolved_root = (root or Path(__file__).resolve().parents[3]).resolve()
    build_dir = resolved_root / "build"
    out_dir = resolved_root / "out"
    toolchains_dir = resolved_root / "toolchains"
    third_party_dir = resolved_root / "third_party"
    inputs_dir = resolved_root / "inputs"
    extracted_dir = build_dir / "extracted"
    rebuilt_dir = build_dir / "rebuilt"
    raw_emi_dir = out_dir / "emi_raw"
    ghidra_bootstrap_dir = out_dir / "ghidra-bootstrap"
    inventory_artifacts_dir = out_dir / "inventory"
    return RepoLayout(
        root=resolved_root,
        build_dir=build_dir,
        out_dir=out_dir,
        toolchains_dir=toolchains_dir,
        third_party_dir=third_party_dir,
        inputs_dir=inputs_dir,
        docs_dir=resolved_root / "docs",
        downloads_dir=toolchains_dir / "downloads",
        bof3_dir=resolved_root / "bof3",
        disc_dir=inputs_dir / "disc",
        external_dir=inputs_dir / "external",
        private_assets_dir=resolved_root / "external" / "private-assets",
        slus_path=extracted_dir / "SLUS_004.22",
        logo_path=extracted_dir / "LOGO" / "LOGO.EXE",
        emi_root=raw_emi_dir / "BIN",
        extracted_dir=extracted_dir,
        rebuilt_dir=rebuilt_dir,
        raw_emi_dir=raw_emi_dir,
        ghidra_bootstrap_dir=ghidra_bootstrap_dir,
        bof3_disk_src=third_party_dir / "bof3-disk",
        emi_ex_src=third_party_dir / "emi-ex",
        objdiff_src=third_party_dir / "objdiff",
        mipsmatch_src=third_party_dir / "mipsmatch",
        bof3_disk_bin=build_dir / "third_party" / "bof3-disk" / "bof3-disk",
        emi_ex_bin=build_dir / "tools" / "emi-ex-v2" / "cli" / "emi-ex",
        objdiff_bin=build_dir / "third_party" / "objdiff" / "release" / "objdiff-cli",
        mipsmatch_bin=build_dir / "third_party" / "mipsmatch" / "release" / "mipsmatch",
        psn00b_toolchain_root=toolchains_dir / "psn00b_toolchain",
        psn00b_sdk_root=toolchains_dir / "psn00bsdk",
        gcc272_psx_root=toolchains_dir / "gcc-2.7.2-psx",
        psyq_root=toolchains_dir / "psyq" / "4.7",
        inventory_path=ghidra_bootstrap_dir / "inventory.json",
        groups_path=ghidra_bootstrap_dir / "groups.json",
        ghidra_manifest_path=ghidra_bootstrap_dir / "ghidra_import_manifest.json",
        inventory_artifacts_dir=inventory_artifacts_dir,
        inventory_artifact_index_path=inventory_artifacts_dir / "index.json",
        inventory_disc_lba_path=inventory_artifacts_dir / "disc_lba.json",
        inventory_slot_map_path=inventory_artifacts_dir / "slot_map.json",
        inventory_slot_map_md_path=inventory_artifacts_dir / "slot_map.md",
        inventory_emi_catalog_path=inventory_artifacts_dir / "emi_catalog.json",
        inventory_emi_catalog_md_path=inventory_artifacts_dir / "emi_catalog.md",
        inventory_overlay_catalog_path=inventory_artifacts_dir / "overlay_catalog.json",
        inventory_overlay_catalog_md_path=inventory_artifacts_dir
        / "overlay_catalog.md",
        inventory_overlay_clusters_path=inventory_artifacts_dir
        / "overlay_clusters.json",
        inventory_overlay_clusters_md_path=inventory_artifacts_dir
        / "overlay_clusters.md",
        inventory_unique_overlay_map_path=inventory_artifacts_dir
        / "unique_overlay_map.json",
        inventory_unique_overlay_map_md_path=inventory_artifacts_dir
        / "unique_overlay_map.md",
        inventory_entry_tables_path=inventory_artifacts_dir / "entry_tables.json",
        inventory_entry_tables_md_path=inventory_artifacts_dir / "entry_tables.md",
        inventory_project_plan_path=inventory_artifacts_dir / "project_plan.json",
        inventory_project_plan_md_path=inventory_artifacts_dir / "project_plan.md",
        inventory_render_metadata_path=inventory_artifacts_dir / "render_metadata.json",
        inventory_render_metadata_md_path=inventory_artifacts_dir
        / "render_metadata.md",
        inventory_ghidra_symbols_index_path=inventory_artifacts_dir
        / "ghidra_symbols_index.json",
        inventory_ghidra_function_index_path=inventory_artifacts_dir
        / "ghidra_function_index.json",
        inventory_ghidra_function_index_tsv_path=inventory_artifacts_dir
        / "ghidra_function_index.tsv",
        inventory_ghidra_symbols_md_path=inventory_artifacts_dir / "ghidra_symbols.md",
        inventory_ghidra_symbols_program_dir=inventory_artifacts_dir
        / "ghidra_symbols_programs",
        disk_checksums_path=out_dir / "disk_checksums.json",
        aspsx_psyq_root=toolchains_dir / "aspsx-psyq-binaries",
        aspsx_psyq_compat_root=third_party_dir / "maspsx" / "aspsx" / "psyq",
    )
