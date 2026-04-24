from __future__ import annotations

from collections.abc import Callable, Sequence
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Protocol

from ..core import Task, run_process
from ..private_assets import list_required_submodule_paths

TaskRunner = Callable[[Any], Any | None]
Command = Sequence[str]


class CommandExecutor(Protocol):
    def __call__(self, command: Command, *, cwd: Path) -> object:
        """Run one workspace command."""


@dataclass(frozen=True)
class CommandTaskSpec:
    name: str
    description: str
    commands: tuple[Command, ...]


def run_workspace_command(command: Command, *, cwd: Path) -> object:
    return run_process(command, cwd=cwd, stream=True, capture=False)


def build_command_task(
    spec: CommandTaskSpec,
    *,
    root: Path,
    executor: CommandExecutor = run_workspace_command,
) -> Task:
    return Task(
        name=spec.name,
        description=spec.description,
        runner=_commands_runner(spec.commands, root=root, executor=executor),
    )


def build_submodule_task(
    name: str,
    description: str,
    command: Command,
    *,
    root: Path,
    executor: CommandExecutor = run_workspace_command,
) -> Task:
    return build_command_task(
        CommandTaskSpec(
            name=name,
            description=description,
            commands=(_with_required_submodule_paths(command, root=root),),
        ),
        root=root,
        executor=executor,
    )


def _commands_runner(
    commands: Sequence[Command],
    *,
    root: Path,
    executor: CommandExecutor,
) -> TaskRunner:
    def run(_: Any) -> None:
        for command in commands:
            executor(command, cwd=root)

    return run


def _with_required_submodule_paths(command: Command, *, root: Path) -> Command:
    paths = list_required_submodule_paths(root)
    if not paths:
        return command
    return (*command, "--", *paths)


def with_force(command: Command, force: bool) -> Command:
    if not force:
        return command
    return (*command, "--force")
