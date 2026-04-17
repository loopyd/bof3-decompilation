from __future__ import annotations

from ...common import run_command
from ...private_assets import list_optional_submodule_paths
from ..models import SetupContext


def run(context: SetupContext) -> None:
    paths = list_optional_submodule_paths(context.layout.root)
    if not paths:
        return
    run_command(
        ["git", "submodule", "update", "--init", "--recursive", "--", *paths],
        cwd=context.layout.root,
    )
