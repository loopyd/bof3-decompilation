from __future__ import annotations

from pathlib import Path

from ..core import Pipeline
from ..paths import repo_layout
from ..tasks import (
    CommandExecutor,
    CommandTaskSpec,
    build_command_task,
    build_submodule_task,
    run_workspace_command,
    with_force,
)


def build_setup_open_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
    force: bool = False,
) -> Pipeline:
    repo_root = root or repo_layout().root
    bin_dir = repo_root / "bin"
    return Pipeline(
        name="setup-open",
        description="Prepare a fresh clone for open-source workspace usage",
        tasks=[
            build_submodule_task(
                "sync-submodules",
                "Synchronize git submodule URLs",
                ("git", "submodule", "sync", "--recursive"),
                root=repo_root,
                executor=executor,
            ),
            build_submodule_task(
                "update-submodules",
                "Initialize and update git submodules",
                ("git", "submodule", "update", "--init", "--recursive"),
                root=repo_root,
                executor=executor,
            ),
            build_submodule_task(
                "check-submodules",
                "Inspect git submodule checkout status",
                ("git", "submodule", "status", "--recursive"),
                root=repo_root,
                executor=executor,
            ),
            build_command_task(
                CommandTaskSpec(
                    name="setup-native-tools",
                    description="Build native open-source workspace tools",
                    commands=((str(bin_dir / "setup-native-tools"),),),
                ),
                root=repo_root,
                executor=executor,
            ),
            build_command_task(
                CommandTaskSpec(
                    name="setup-match-tools",
                    description="Build open-source matching tools",
                    commands=((str(bin_dir / "setup-match-tools"),),),
                ),
                root=repo_root,
                executor=executor,
            ),
            build_command_task(
                CommandTaskSpec(
                    name="setup-open-toolchains",
                    description="Install open PSX toolchains and public ASPSX binaries",
                    commands=(
                        with_force((str(bin_dir / "setup-psx-toolchain"),), force),
                        with_force((str(bin_dir / "setup-aspsx"),), force),
                    ),
                ),
                root=repo_root,
                executor=executor,
            ),
            build_command_task(
                CommandTaskSpec(
                    name="verify-open-doctor",
                    description="Verify open workspace readiness with the doctor profile",
                    commands=((str(bin_dir / "doctor"), "--profile", "open"),),
                ),
                root=repo_root,
                executor=executor,
            ),
        ],
    )
