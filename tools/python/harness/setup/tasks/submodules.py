from __future__ import annotations

from ...io import run_command
from ..models import SetupContext


def run(context: SetupContext) -> None:
    """Initialize the repository's pinned tool submodules before building them."""

    run_command(
        ["git", "submodule", "update", "--init", "--recursive"],
        cwd=context.layout.root,
    )
