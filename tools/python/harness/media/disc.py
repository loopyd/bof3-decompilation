"""``bin/harness media disc``: repository-owned disc inspection and extraction."""

from __future__ import annotations

import os

from harness.common.native import exec_tool
from harness.io import repo_layout

EXAMPLE = "bin/harness media disc extract -i inputs/external/disc.cue -o out/extracted"


def main(argv: list[str] | None = None) -> int:
    args = list(argv or [])
    if args == ["--example"]:
        print(EXAMPLE)
        return 0
    layout = repo_layout()
    executable = os.environ.get("PSX_BOF3_DISK") or str(layout.harness_disk_bin)
    exec_tool(executable, args)
