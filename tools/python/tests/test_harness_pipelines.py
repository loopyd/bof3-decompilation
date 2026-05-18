from __future__ import annotations

from pathlib import Path
from typing import Sequence

from rebof3.pipelines.harness import (
    build_harness_ready_pipeline,
    build_lift_ready_pipeline,
)
from rebof3.pipelines.reverse import build_decomp_full_ready_pipeline


class RecordingExecutor:
    def __init__(self) -> None:
        self.calls: list[tuple[tuple[str, ...], Path]] = []

    def __call__(self, command: Sequence[str], *, cwd: Path) -> None:
        self.calls.append((tuple(command), cwd))


def test_harness_ready_pipeline_refreshes_state_maps_and_reports() -> None:
    executor = RecordingExecutor()
    root = Path("/repo")
    pipeline = build_harness_ready_pipeline(root=root, executor=executor)

    pipeline.run()

    assert [task.name for task in pipeline.plan()] == ["harness-refresh"]
    assert executor.calls == [
        ((str(root / "bin" / "harness"), "refresh"), root),
    ]


def test_lift_ready_pipeline_runs_only_cheap_harness_refresh() -> None:
    executor = RecordingExecutor()
    root = Path("/repo")
    pipeline = build_lift_ready_pipeline(root=root, executor=executor)

    pipeline.run()

    assert [task.name for task in pipeline.plan()] == ["harness-refresh"]
    assert executor.calls == [
        ((str(root / "bin" / "harness"), "refresh"), root),
    ]


def test_decomp_full_ready_pipeline_refreshes_one_project_and_harness() -> None:
    executor = RecordingExecutor()
    root = Path("/repo")
    pipeline = build_decomp_full_ready_pipeline(root=root, executor=executor)

    pipeline.run()

    assert [task.name for task in pipeline.plan()] == [
        "disk-extract",
        "emi-unpack",
        "inventory-build",
        "ghidra-bootstrap",
        "ghidra-import-project",
        "ghidra-analyze",
        "ghidra-export-symbols",
        "inventory-import-ghidra-symbols",
        "ghidra-coverage",
        "harness-refresh",
    ]
    assert executor.calls == [
        ((str(root / "bin" / "disk-extract"),), root),
        ((str(root / "bin" / "emi-unpack"),), root),
        ((str(root / "bin" / "inventory-build"),), root),
        ((str(root / "bin" / "ghidra-bootstrap"),), root),
        (
            (
                str(root / "bin" / "harness"),
                "ghidra",
                "import-project",
                "--no-analysis",
            ),
            root,
        ),
        ((str(root / "bin" / "harness"), "ghidra", "analyze"), root),
        ((str(root / "bin" / "harness"), "ghidra", "export"), root),
        ((str(root / "bin" / "inventory-import-ghidra-symbols"),), root),
        (
            (
                str(root / "bin" / "harness"),
                "ghidra",
                "coverage",
                "--allow-partial",
            ),
            root,
        ),
        ((str(root / "bin" / "harness"), "refresh"), root),
    ]
