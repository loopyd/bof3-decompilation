from __future__ import annotations

from rebof3.setup import SetupOptions, plan_setup_tasks, setup_task_names


def test_plan_setup_tasks_full_workspace() -> None:
    tasks = plan_setup_tasks(SetupOptions())
    assert [task.name for task in tasks] == [
        "submodules",
        "aspsx-binaries",
        "native-tools",
        "psx-toolchain",
        "psyq",
        "match-tools",
        "extract",
        "unpack",
        "ghidra-bootstrap",
    ]


def test_plan_setup_tasks_without_optional_steps() -> None:
    tasks = plan_setup_tasks(
        SetupOptions(
            include_aspsx_binaries=False,
            include_match_tools=False,
            include_extract=False,
            include_ghidra_plan=False,
        )
    )
    assert [task.name for task in tasks] == [
        "submodules",
        "native-tools",
        "psx-toolchain",
        "psyq",
    ]


def test_setup_task_names_are_stable() -> None:
    assert setup_task_names() == (
        "submodules",
        "aspsx-binaries",
        "native-tools",
        "psx-toolchain",
        "psyq",
        "match-tools",
        "extract",
        "unpack",
        "ghidra-bootstrap",
    )
