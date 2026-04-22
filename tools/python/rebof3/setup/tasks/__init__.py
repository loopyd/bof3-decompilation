from __future__ import annotations

from ..interfaces import SetupTaskSpec
from ..models import SetupContext, SetupOptions, SetupTask
from . import (
    aspsx_binaries,
    extract,
    ghidra_bootstrap,
    match_tools,
    native_tools,
    private_assets_submodule,
    psyq,
    psx_toolchain,
    submodules,
    unpack,
)


def always_enabled(_: SetupOptions) -> bool:
    return True


def when_aspsx_binaries_enabled(options: SetupOptions) -> bool:
    return options.include_aspsx_binaries


def when_match_tools_enabled(options: SetupOptions) -> bool:
    return options.include_match_tools


def when_psyq_enabled(options: SetupOptions) -> bool:
    return options.include_psyq


def when_extract_enabled(options: SetupOptions) -> bool:
    return options.include_extract


def when_ghidra_plan_enabled(options: SetupOptions) -> bool:
    return options.include_ghidra_plan


def never_enabled(_: SetupOptions) -> bool:
    return False


SETUP_TASK_SPECS: tuple[SetupTaskSpec, ...] = (
    SetupTaskSpec(
        task=SetupTask("submodules", "Initialize git submodules"),
        runner=submodules.run,
        enabled=always_enabled,
    ),
    SetupTaskSpec(
        task=SetupTask(
            "private-assets-submodule",
            "Initialize the optional private-assets import workspace git submodule",
        ),
        runner=private_assets_submodule.run,
        enabled=never_enabled,
    ),
    SetupTaskSpec(
        task=SetupTask(
            "aspsx-binaries",
            "Download the public ASPSX/PsyQ binary bundles used by maspsx tests",
        ),
        runner=aspsx_binaries.run,
        enabled=when_aspsx_binaries_enabled,
    ),
    SetupTaskSpec(
        task=SetupTask("native-tools", "Build bof3-disk and emi-ex"),
        runner=native_tools.run,
        enabled=always_enabled,
    ),
    SetupTaskSpec(
        task=SetupTask(
            "psx-toolchain", "Download and stage the canonical PSX toolchain"
        ),
        runner=psx_toolchain.run,
        enabled=always_enabled,
    ),
    SetupTaskSpec(
        task=SetupTask("psyq", "Stage the configured PsyQ SDK"),
        runner=psyq.run,
        enabled=when_psyq_enabled,
    ),
    SetupTaskSpec(
        task=SetupTask("match-tools", "Build objdiff-cli and mipsmatch"),
        runner=match_tools.run,
        enabled=when_match_tools_enabled,
    ),
    SetupTaskSpec(
        task=SetupTask("extract", "Extract the BOF3 disc"),
        runner=extract.run,
        enabled=when_extract_enabled,
    ),
    SetupTaskSpec(
        task=SetupTask("unpack", "Unpack EMI archives"),
        runner=unpack.run,
        enabled=when_extract_enabled,
    ),
    SetupTaskSpec(
        task=SetupTask(
            "ghidra-bootstrap",
            "Generate inventory, duplicate groups, and a Ghidra import manifest",
        ),
        runner=ghidra_bootstrap.run,
        enabled=when_ghidra_plan_enabled,
    ),
)


def iter_setup_task_specs(options: SetupOptions) -> list[SetupTaskSpec]:
    return [spec for spec in SETUP_TASK_SPECS if spec.enabled(options)]


def iter_setup_tasks(options: SetupOptions) -> list[SetupTask]:
    return [spec.task for spec in iter_setup_task_specs(options)]


def setup_task_names() -> tuple[str, ...]:
    return tuple(spec.task.name for spec in SETUP_TASK_SPECS)


def run_setup_task(context: SetupContext, task_name: str) -> None:
    for spec in SETUP_TASK_SPECS:
        if spec.task.name == task_name:
            spec.runner(context)
            return
    raise KeyError(f"unknown setup task: {task_name}")
