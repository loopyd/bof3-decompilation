from __future__ import annotations

from ...ghidra.bootstrap import bootstrap_ghidra
from ..models import SetupContext


def run(context: SetupContext) -> None:
    bootstrap_ghidra(
        slus_path=context.layout.slus_path,
        logo_path=context.layout.logo_path,
        emi_root=context.layout.emi_root,
        output_dir=context.layout.ghidra_bootstrap_dir,
        analyze=True,
    )
