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


def build_harness_ready_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
) -> Pipeline:
    repo_root = root or repo_layout().root
    return Pipeline(
        name="harness-ready",
        description="Refresh harness state, reports, and dashboard",
        tasks=harness_refresh_tasks(root=repo_root, executor=executor),
    )


def build_lift_ready_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
) -> Pipeline:
    repo_root = root or repo_layout().root
    return Pipeline(
        name="lift-ready",
        description="Refresh the cheap harness state needed for function lifting",
        tasks=harness_refresh_tasks(root=repo_root, executor=executor),
    )


def harness_refresh_tasks(
    *,
    root: Path,
    executor: CommandExecutor,
) -> list[Task]:
    harness = _bin(root, "harness")
    return [
        _task(
            root=root,
            executor=executor,
            name="harness-refresh",
            description="Refresh harness state, reports, and dashboard",
            command=(harness, "refresh"),
        ),
    ]
