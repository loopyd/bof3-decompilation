#!/usr/bin/env python3
"""Smoke-test every project Pi skill script against the prepared workspace."""

from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[4]
TARGET = "emi/battle/battle/15@0x80096E90"
CASES = (
    (
        ROOT / ".pi/skills/bof3-re/scripts/function-brief.py",
        (TARGET,),
        "bof3.skill-function-brief/v1",
    ),
    (
        ROOT / ".pi/skills/bof3-lift-loop/scripts/loop-status.py",
        ("--selection", "hotspots", "--limit", "1"),
        "bof3.skill-lift-loop-status/v1",
    ),
    (
        ROOT / ".pi/skills/psx-rizin/scripts/snapshot-status.py",
        ("emi/world00/area030/04",),
        "bof3.skill-rizin-snapshot-status/v1",
    ),
)


def main() -> int:
    for script, args, schema in CASES:
        result = subprocess.run(
            (sys.executable, str(script), *args),
            cwd=ROOT,
            text=True,
            capture_output=True,
            check=True,
        )
        assert json.loads(result.stdout)["schema"] == schema, script
        print(f"ok {script.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
