from __future__ import annotations

from pathlib import Path
from typing import Sequence

from rebof3.pipelines.build_match import (
    build_build_ready_pipeline,
    build_match_loop_pipeline,
)


class RecordingExecutor:
    def __init__(self) -> None:
        self.calls: list[tuple[tuple[str, ...], Path]] = []

    def __call__(self, command: Sequence[str], *, cwd: Path) -> None:
        self.calls.append((tuple(command), cwd))


def test_build_ready_pipeline_configures_then_builds() -> None:
    executor = RecordingExecutor()
    root = Path("/repo")
    pipeline = build_build_ready_pipeline(root=root, executor=executor)

    pipeline.run()

    assert [task.name for task in pipeline.plan()] == ["configure", "build"]
    assert executor.calls == [
        ((str(root / "bin" / "configure"),), root),
        ((str(root / "bin" / "build"),), root),
    ]


def test_match_loop_pipeline_builds_diffs_then_reports() -> None:
    executor = RecordingExecutor()
    root = Path("/repo")
    pipeline = build_match_loop_pipeline(root=root, executor=executor)

    pipeline.run()

    assert [task.name for task in pipeline.plan()] == [
        "match-build",
        "match-diff",
        "match-report",
    ]
    assert executor.calls == [
        ((str(root / "bin" / "match-build"),), root),
        ((str(root / "bin" / "match-diff"),), root),
        ((str(root / "bin" / "match-report"),), root),
    ]
