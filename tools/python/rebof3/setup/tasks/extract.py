from __future__ import annotations

from ...common import run_command
from ..models import SetupContext
from .helpers import detect_disk_inputs


def run(context: SetupContext) -> None:
    if not detect_disk_inputs(context):
        raise RuntimeError(f"no disc image found under {context.layout.disc_dir}")
    run_command([str(context.layout.bof3_disk_bin), "extract"], cwd=context.layout.root)
