from __future__ import annotations

from ...toolchain.psx import install_canonical_psx_toolchain
from ..models import SetupContext


def run(context: SetupContext) -> None:
    install_canonical_psx_toolchain(
        context.layout,
        force=context.options.force,
    )
