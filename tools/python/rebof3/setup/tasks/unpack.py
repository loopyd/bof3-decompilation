from __future__ import annotations

from ...emi import emi_unpack
from ..models import SetupContext


def run(context: SetupContext) -> None:
    emi_unpack(
        tool_path=context.layout.emi_ex_bin,
        cwd=context.layout.root,
        extracted_dir=context.layout.extracted_dir,
        raw_emi_dir=context.layout.raw_emi_dir,
    )
