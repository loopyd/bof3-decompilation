from __future__ import annotations

from ...toolchain.aspsx import download_aspsx_binaries
from ..models import SetupContext


def run(context: SetupContext) -> None:
    download_aspsx_binaries(
        context.layout,
        force=context.options.force,
    )
