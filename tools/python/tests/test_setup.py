from __future__ import annotations

from rebof3.setup import SetupOptions, plan_setup_tasks


def test_plan_setup_tasks_full_workspace() -> None:
    tasks = plan_setup_tasks(SetupOptions())
    assert [task.name for task in tasks] == [
        "submodules",
        "native-tools",
        "psx-toolchain",
        "psyq",
        "extract",
        "unpack",
    ]
