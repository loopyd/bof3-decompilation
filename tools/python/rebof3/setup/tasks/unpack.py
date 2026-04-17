from __future__ import annotations

from ...common import run_command
from ..models import SetupContext


def run(context: SetupContext) -> None:
    run_command(
        [
            str(context.layout.emi_ex_bin),
            "mirror-extract",
            "--quiet",
            "-i",
            str(context.layout.extracted_dir),
            "-o",
            str(context.layout.raw_emi_dir),
        ],
        cwd=context.layout.root,
    )
