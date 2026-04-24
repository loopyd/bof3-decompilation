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


def build_build_ready_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
) -> Pipeline:
    repo_root = root or repo_layout().root
    return Pipeline(
        name="build-ready",
        description="Configure and build the workspace",
        tasks=[
            _task(
                root=repo_root,
                executor=executor,
                name="configure",
                description="Configure the BOF3 PSX CMake preset",
                command=(_bin(repo_root, "configure"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="build",
                description="Build configured targets",
                command=(_bin(repo_root, "build"),),
            ),
        ],
    )


def build_match_loop_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
) -> Pipeline:
    repo_root = root or repo_layout().root
    return Pipeline(
        name="match-loop",
        description="Build, diff, and report matching status",
        tasks=[
            _task(
                root=repo_root,
                executor=executor,
                name="match-build",
                description="Run the configured matching build command",
                command=(_bin(repo_root, "match-build"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="match-diff",
                description="Diff configured expected and actual matching artifacts",
                command=(_bin(repo_root, "match-diff"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="match-report",
                description="Render the matching status report",
                command=(_bin(repo_root, "match-report"),),
            ),
        ],
    )
