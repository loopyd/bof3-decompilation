from __future__ import annotations

from pathlib import Path

from ..core import Pipeline, Task
from ..paths import repo_layout
from ..tasks import CommandExecutor, CommandTaskSpec, build_command_task
from ..tasks import run_workspace_command


def _bin(root: Path, name: str) -> str:
    return str(root / "bin" / name)


def _task(
    *,
    root: Path,
    executor: CommandExecutor,
    name: str,
    description: str,
    command: tuple[str, ...],
) -> Task:
    return build_command_task(
        CommandTaskSpec(
            name=name,
            description=description,
            commands=(command,),
        ),
        root=root,
        executor=executor,
    )


def build_extract_assets_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
) -> Pipeline:
    repo_root = root or repo_layout().root
    return Pipeline(
        name="extract-assets",
        description="Extract the disc and unpack EMI archives into workspace outputs",
        tasks=[
            _task(
                root=repo_root,
                executor=executor,
                name="disk-extract",
                description="Extract BOF3 disc files from inputs/disc",
                command=(_bin(repo_root, "disk-extract"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="emi-unpack",
                description="Unpack extracted EMI archives into out/emi_raw",
                command=(_bin(repo_root, "emi-unpack"),),
            ),
        ],
    )


def build_inventory_refresh_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
) -> Pipeline:
    repo_root = root or repo_layout().root
    return Pipeline(
        name="inventory-refresh",
        description="Refresh maintained inventory artifacts from extracted assets",
        tasks=[
            _task(
                root=repo_root,
                executor=executor,
                name="inventory-build",
                description="Build inventory JSON and Markdown artifacts",
                command=(_bin(repo_root, "inventory-build"),),
            ),
        ],
    )


def build_ghidra_ready_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
) -> Pipeline:
    repo_root = root or repo_layout().root
    return Pipeline(
        name="ghidra-ready",
        description="Prepare extracted assets, inventory, and Ghidra bootstrap outputs",
        tasks=[
            *build_extract_assets_pipeline(root=repo_root, executor=executor).tasks,
            *build_inventory_refresh_pipeline(root=repo_root, executor=executor).tasks,
            _task(
                root=repo_root,
                executor=executor,
                name="ghidra-bootstrap",
                description="Generate Ghidra import inventory, groups, and manifest",
                command=(_bin(repo_root, "ghidra-bootstrap"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="ghidra-import-project",
                description="Import bootstrap manifest binaries into a Ghidra project",
                command=(_bin(repo_root, "ghidra-import-project"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="doctor-ghidra",
                description="Verify Ghidra workflow readiness",
                command=(_bin(repo_root, "doctor"), "--profile", "ghidra"),
            ),
        ],
    )


def build_decomp_ready_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
) -> Pipeline:
    repo_root = root or repo_layout().root
    return Pipeline(
        name="decomp-ready",
        description="Export/import Ghidra symbols and verify decomp/matching readiness",
        tasks=[
            _task(
                root=repo_root,
                executor=executor,
                name="ghidra-export-symbols",
                description="Export raw Ghidra function symbols from the project",
                command=(_bin(repo_root, "ghidra-export-symbols"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="inventory-import-ghidra-symbols",
                description="Import raw Ghidra symbol export into inventory indexes",
                command=(_bin(repo_root, "inventory-import-ghidra-symbols"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="doctor-decomp",
                description="Verify decomp and matching workflow readiness",
                command=(_bin(repo_root, "doctor"), "--profile", "decomp"),
            ),
        ],
    )
