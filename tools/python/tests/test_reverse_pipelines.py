from __future__ import annotations

from pathlib import Path
from typing import Sequence

from rebof3.pipelines.reverse import (
    build_decomp_ready_pipeline,
    build_extract_assets_pipeline,
    build_ghidra_ready_pipeline,
    build_inventory_refresh_pipeline,
)


class RecordingExecutor:
    def __init__(self) -> None:
        self.calls: list[tuple[tuple[str, ...], Path]] = []

    def __call__(self, command: Sequence[str], *, cwd: Path) -> None:
        self.calls.append((tuple(command), cwd))


def test_extract_assets_pipeline_wires_disk_and_emi_commands() -> None:
    executor = RecordingExecutor()
    root = Path("/repo")

    build_extract_assets_pipeline(root=root, executor=executor).run()

    assert executor.calls == [
        ((str(root / "bin" / "disk-extract"),), root),
        ((str(root / "bin" / "emi-unpack"),), root),
    ]


def test_inventory_refresh_pipeline_wires_inventory_build() -> None:
    executor = RecordingExecutor()
    root = Path("/repo")

    build_inventory_refresh_pipeline(root=root, executor=executor).run()

    assert executor.calls == [((str(root / "bin" / "inventory-build"),), root)]


def test_ghidra_ready_pipeline_runs_extract_inventory_bootstrap_and_doctor() -> None:
    executor = RecordingExecutor()
    root = Path("/repo")
    pipeline = build_ghidra_ready_pipeline(root=root, executor=executor)

    pipeline.run()

    assert [task.name for task in pipeline.plan()] == [
        "disk-extract",
        "emi-unpack",
        "inventory-build",
        "ghidra-bootstrap",
        "ghidra-import-project",
        "doctor-ghidra",
    ]
    assert executor.calls[-2] == (
        (str(root / "bin" / "harness"), "ghidra", "import-project"),
        root,
    )
    assert executor.calls[-1] == (
        (str(root / "bin" / "doctor"), "--profile", "ghidra"),
        root,
    )


def test_decomp_ready_pipeline_imports_symbols_then_verifies_decomp_profile() -> None:
    executor = RecordingExecutor()
    root = Path("/repo")

    build_decomp_ready_pipeline(root=root, executor=executor).run()

    assert executor.calls == [
        ((str(root / "bin" / "harness"), "ghidra", "export-symbols"), root),
        ((str(root / "bin" / "inventory-import-ghidra-symbols"),), root),
        ((str(root / "bin" / "doctor"), "--profile", "decomp"), root),
    ]
