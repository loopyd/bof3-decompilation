"""``bin/harness emi archive``: repository-owned EMI archive operations."""

from __future__ import annotations

import os

from harness.common.native import exec_tool
from harness.io import repo_layout

EXAMPLE = "bin/harness emi archive extract -o out/extracted INPUT.EMI"


def main(argv: list[str] | None = None) -> int:
    args = list(argv or [])
    if args == ["--example"]:
        print(EXAMPLE)
        return 0
    layout = repo_layout()
    executable = os.environ.get("PSX_EMI_EX") or str(layout.emi_ex_bin)
    exec_tool(executable, args)
