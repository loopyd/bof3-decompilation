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
        description="Refresh harness state, binary maps, reports, and dashboard",
        tasks=harness_refresh_tasks(root=repo_root, executor=executor),
    )


def build_lift_ready_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
) -> Pipeline:
    repo_root = root or repo_layout().root
    harness = _bin(repo_root, "harness")
    return Pipeline(
        name="lift-ready",
        description="Refresh the cheap harness state needed for function lifting",
        tasks=[
            _task(
                root=repo_root,
                executor=executor,
                name="harness-setup",
                description="Initialize harness directories, context, and database",
                command=(harness, "setup"),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="harness-catalog",
                description="Catalog EMI entries and build artifacts",
                command=(harness, "catalog"),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="harness-analyze",
                description="Refresh function targets from existing Ghidra exports",
                command=(harness, "analyze"),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="harness-report",
                description="Render JSON and Markdown harness reports",
                command=(harness, "report"),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="harness-dashboard",
                description="Render the static harness dashboard",
                command=(harness, "dashboard"),
            ),
        ],
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
            name="harness-setup",
            description="Initialize harness directories, context, and database",
            command=(harness, "setup"),
        ),
        _task(
            root=root,
            executor=executor,
            name="harness-catalog",
            description="Catalog EMI entries and build artifacts",
            command=(harness, "catalog"),
        ),
        _task(
            root=root,
            executor=executor,
            name="harness-analyze",
            description="Import available function targets from inventory",
            command=(harness, "analyze"),
        ),
        _task(
            root=root,
            executor=executor,
            name="harness-split",
            description="Record staged source migration targets",
            command=(harness, "split"),
        ),
        _task(
            root=root,
            executor=executor,
            name="harness-binary-map",
            description="Refresh function, symbol, and xref maps for EMI bins",
            command=(harness, "binary", "map", "--all", "--type", "emi"),
        ),
        _task(
            root=root,
            executor=executor,
            name="harness-report",
            description="Render JSON and Markdown harness reports",
            command=(harness, "report"),
        ),
        _task(
            root=root,
            executor=executor,
            name="harness-dashboard",
            description="Render the static harness dashboard",
            command=(harness, "dashboard"),
        ),
    ]


def build_binary_parity_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
) -> Pipeline:
    repo_root = root or repo_layout().root
    harness = _bin(repo_root, "harness")
    return Pipeline(
        name="binary-parity",
        description="Build compiled raw .bin files and diff them against extracted EMI .bin files",
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
                name="build-artifacts",
                description="Build registered raw .bin targets; no EMI archive repacking",
                command=(_bin(repo_root, "build"), "--target", "artifacts"),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="harness-binary-map",
                description="Refresh maps for EMI bins with compiled raw outputs",
                command=(
                    harness,
                    "binary",
                    "map",
                    "--all",
                    "--type",
                    "emi",
                    "--compiled-only",
                ),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="harness-binary-diff",
                description="Diff compiled raw .bin files against extracted EMI .bin files",
                command=(
                    harness,
                    "verify",
                    "binary",
                    "--all",
                    "--type",
                    "emi",
                    "--compiled-only",
                    "--allow-different",
                ),
            ),
        ],
    )
