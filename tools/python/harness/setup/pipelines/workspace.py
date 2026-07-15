from __future__ import annotations

from ...io import RepoLayout, repo_layout
from ..models import SetupContext, SetupOptions
from ..tasks import iter_setup_task_specs, run_setup_task


def run_setup_workspace(
    options: SetupOptions,
    *,
    layout: RepoLayout | None = None,
) -> None:
    context = SetupContext(layout=layout or repo_layout(), options=options)
    for spec in iter_setup_task_specs(options):
        print(f"==> {spec.task.name}: {spec.task.description}")
        spec.runner(context)


def run_named_setup_task(
    task_name: str,
    options: SetupOptions,
    *,
    layout: RepoLayout | None = None,
) -> None:
    context = SetupContext(layout=layout or repo_layout(), options=options)
    run_setup_task(context, task_name)
