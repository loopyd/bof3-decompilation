from __future__ import annotations

import json
import os
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any


DEFAULT_PSYQ_VERSION = "4.7"


def normalize_psyq_version(version: str | None = None) -> str:
    raw_version = version or DEFAULT_PSYQ_VERSION
    normalized = raw_version.strip()
    if normalized.lower().startswith("psyq"):
        normalized = normalized[4:]
    if not normalized:
        raise ValueError("PsyQ version must not be empty")
    return normalized


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
    harness_dir: Path
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
    harness_disk_src: Path
    emi_ex_src: Path
    harness_disk_bin: Path
    emi_ex_bin: Path
    psn00b_toolchain_root: Path
    psn00b_sdk_root: Path
    gcc272_psx_root: Path
    psyq_version: str
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


def repo_layout(
    root: Path | None = None, *, psyq_version: str | None = None
) -> RepoLayout:
    resolved_root = (root or Path(__file__).resolve().parents[3]).resolve()
    resolved_psyq_version = normalize_psyq_version(psyq_version)
    build_dir = resolved_root / "build"
    out_dir = resolved_root / "out"
    toolchains_dir = resolved_root / "toolchains"
    third_party_dir = resolved_root / "third_party"
    inputs_dir = resolved_root / "inputs"
    extracted_dir = out_dir / "extracted"
    rebuilt_dir = out_dir / "rebuilt"
    raw_emi_dir = out_dir / "extracted"
    ghidra_bootstrap_dir = out_dir / "ghidra-bof3"
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
        harness_dir=resolved_root,
        disc_dir=inputs_dir / "disc",
        external_dir=inputs_dir / "external",
        private_assets_dir=inputs_dir / "external" / "private-assets",
        slus_path=extracted_dir / "SLUS_004.22",
        logo_path=extracted_dir / "LOGO" / "LOGO.EXE",
        emi_root=raw_emi_dir / "BIN",
        extracted_dir=extracted_dir,
        rebuilt_dir=rebuilt_dir,
        raw_emi_dir=raw_emi_dir,
        ghidra_bootstrap_dir=ghidra_bootstrap_dir,
        harness_disk_src=third_party_dir / "bof3-disk-v2",
        emi_ex_src=third_party_dir / "emi-ex-v2",
        harness_disk_bin=build_dir
        / "tools"
        / "rust"
        / "bof3-disk"
        / "release"
        / "bof3-disk",
        emi_ex_bin=third_party_dir / "emi-ex-v2" / "target" / "release" / "emi-ex",
        psn00b_toolchain_root=toolchains_dir / "psn00b_toolchain",
        psn00b_sdk_root=toolchains_dir / "psn00bsdk",
        gcc272_psx_root=toolchains_dir / "gcc-2.7.2-psx",
        psyq_version=resolved_psyq_version,
        psyq_root=toolchains_dir / "psyq" / resolved_psyq_version,
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
    )


def read_json(path: Path) -> dict[str, Any]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError(f"expected a JSON object in {path}")
    return payload


def write_json(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    content = json.dumps(payload, indent=2, sort_keys=True) + "\n"
    descriptor, temporary_name = tempfile.mkstemp(
        dir=path.parent, prefix=f".{path.name}.", suffix=".tmp"
    )
    temporary_path = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8") as temporary:
            temporary.write(content)
            temporary.flush()
            os.fsync(temporary.fileno())
        temporary_path.replace(path)
    except BaseException:
        temporary_path.unlink(missing_ok=True)
        raise


def run_command(
    command: list[str],
    *,
    cwd: Path | None = None,
    env: dict[str, str] | None = None,
) -> None:
    full_env = os.environ.copy()
    if env is not None:
        full_env.update(env)
    print("+", " ".join(command))
    result = subprocess.run(command, cwd=cwd, env=full_env, check=False)
    if result.returncode != 0:
        raise RuntimeError(
            f"command failed with exit code {result.returncode}: {' '.join(command)}"
        )


def ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
