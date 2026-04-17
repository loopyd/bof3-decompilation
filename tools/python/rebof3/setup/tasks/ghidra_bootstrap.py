from __future__ import annotations

from ...pipelines import run_ghidra_bootstrap_pipeline
from ..models import SetupContext


def run(context: SetupContext) -> None:
    run_ghidra_bootstrap_pipeline(
        slus_path=context.layout.slus_path,
        logo_path=context.layout.logo_path,
        emi_root=context.layout.emi_root,
        output_dir=context.layout.ghidra_bootstrap_dir,
        analyze=True,
    )
