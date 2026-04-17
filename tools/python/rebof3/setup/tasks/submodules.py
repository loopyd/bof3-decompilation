from __future__ import annotations

from ...common import run_command
from ..models import SetupContext


def run(context: SetupContext) -> None:
    run_command(
        ["git", "submodule", "update", "--init", "--recursive"],
        cwd=context.layout.root,
    )
